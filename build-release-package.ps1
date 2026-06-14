[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',

    [string]$HexPath = '',

    [string]$BootloaderHexPath = '',

    [string]$OutputDir = '',

    [switch]$SkipBuild,

    [switch]$UseMicrochipStudioBuild
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = $PSScriptRoot
$defsPath = Join-Path $repoRoot 'Software/AtmelStudio7/Arducon/Arducon/EepromManager.h'
if([string]::IsNullOrWhiteSpace($BootloaderHexPath))
{
    $BootloaderHexPath = Join-Path $repoRoot 'Bootloaders/optiboot-atmega328p-arduino-1.8.6/optiboot_atmega328.hex'
}
$bootloaderSourceDir = Join-Path $repoRoot 'Bootloaders/optiboot-atmega328p-arduino-1.8.6'

function Get-DefineString {
    param(
        [Parameter(Mandatory = $true)][string]$Text,
        [Parameter(Mandatory = $true)][string]$Name
    )

    $match = [regex]::Match($Text, "(?m)^\s*#define\s+$Name\s+`"([^`"]+)`"")
    if(-not $match.Success)
    {
        throw "Could not find string define $Name."
    }
    return $match.Groups[1].Value
}

function Get-IntelHexBytes {
    param([Parameter(Mandatory = $true)][string]$Path)

    if(-not (Test-Path -LiteralPath $Path))
    {
        throw "HEX file not found: $Path"
    }

    $bytes = @{}
    $upper = 0
    foreach($line in Get-Content -LiteralPath $Path)
    {
        if([string]::IsNullOrWhiteSpace($line)) { continue }
        if(-not $line.StartsWith(':')) { throw "Invalid Intel HEX line in $Path." }
        $count = [Convert]::ToInt32($line.Substring(1, 2), 16)
        $address = [Convert]::ToInt32($line.Substring(3, 4), 16)
        $recordType = [Convert]::ToInt32($line.Substring(7, 2), 16)
        $sum = $count + (($address -shr 8) -band 0xFF) + ($address -band 0xFF) + $recordType
        $data = New-Object byte[] $count
        for($i = 0; $i -lt $count; $i++)
        {
            $data[$i] = [Convert]::ToByte($line.Substring(9 + ($i * 2), 2), 16)
            $sum += $data[$i]
        }
        $checksum = [Convert]::ToInt32($line.Substring(9 + ($count * 2), 2), 16)
        if((($sum + $checksum) -band 0xFF) -ne 0) { throw "Intel HEX checksum failed in $Path." }
        switch($recordType)
        {
            0 {
                for($i = 0; $i -lt $count; $i++)
                {
                    $bytes[$upper + $address + $i] = $data[$i]
                }
            }
            1 { break }
            2 { $upper = ((([int]$data[0] -shl 8) -bor [int]$data[1]) -shl 4) }
            3 { }
            4 { $upper = ((([int]$data[0] -shl 8) -bor [int]$data[1]) -shl 16) }
            5 { }
            default { throw "Unsupported Intel HEX record type $recordType in $Path." }
        }
    }
    return $bytes
}

function Copy-PackageFile {
    param(
        [Parameter(Mandatory = $true)][string]$SourcePath,
        [Parameter(Mandatory = $true)][string]$DestinationPath,
        [Parameter(Mandatory = $true)][string]$Kind,
        [string]$Purpose = ''
    )

    if([System.IO.Path]::GetFullPath($SourcePath) -ne [System.IO.Path]::GetFullPath($DestinationPath))
    {
        Copy-Item -LiteralPath $SourcePath -Destination $DestinationPath -Force
    }
    $bytes = [System.IO.File]::ReadAllBytes($DestinationPath)
    [pscustomobject]@{
        fileName = [System.IO.Path]::GetFileName($DestinationPath)
        kind = $Kind
        purpose = $Purpose
        sizeBytes = $bytes.Length
        sha256 = (Get-FileHash -LiteralPath $DestinationPath -Algorithm SHA256).Hash.ToLowerInvariant()
    }
}

function Get-HexAddressSummary {
    param([Parameter(Mandatory = $true)][hashtable]$Image)

    $addresses = @($Image.Keys | Sort-Object { [int]$_ })
    if($addresses.Count -eq 0)
    {
        throw 'HEX image contains no data records.'
    }
    [pscustomobject]@{
        First = [int]($addresses | Select-Object -First 1)
        Last = [int]($addresses | Select-Object -Last 1)
        Count = $Image.Count
    }
}

function New-IntelHexRecord {
    param(
        [Parameter(Mandatory = $true)][byte]$RecordType,
        [Parameter(Mandatory = $true)][UInt16]$Address,
        [byte[]]$Data = [byte[]]::new(0)
    )

    $sum = $Data.Length + (($Address -shr 8) -band 0xFF) + ($Address -band 0xFF) + $RecordType
    $hex = ':' + ('{0:X2}{1:X4}{2:X2}' -f $Data.Length, $Address, $RecordType)
    foreach($byte in $Data)
    {
        $sum += $byte
        $hex += ('{0:X2}' -f $byte)
    }
    $checksum = ((-$sum) -band 0xFF)
    return $hex + ('{0:X2}' -f $checksum)
}

function Write-IntelHex {
    param(
        [Parameter(Mandatory = $true)][hashtable]$BytesByAddress,
        [Parameter(Mandatory = $true)][string]$Path
    )

    $parent = Split-Path -Parent $Path
    if(-not [string]::IsNullOrWhiteSpace($parent))
    {
        New-Item -ItemType Directory -Force -Path $parent | Out-Null
    }

    $lines = [System.Collections.Generic.List[string]]::new()
    $currentUpper = -1
    $addresses = @($BytesByAddress.Keys | Sort-Object { [int]$_ })
    $index = 0

    while($index -lt $addresses.Count)
    {
        $startAddress = [int]$addresses[$index]
        $upper = ($startAddress -shr 16) -band 0xFFFF
        if($upper -ne $currentUpper)
        {
            $lines.Add((New-IntelHexRecord -RecordType 4 -Address 0 -Data ([byte[]]@([byte](($upper -shr 8) -band 0xFF), [byte]($upper -band 0xFF)))))
            $currentUpper = $upper
        }

        $chunkStart = $startAddress
        $chunk = [System.Collections.Generic.List[byte]]::new()
        while($index -lt $addresses.Count -and $chunk.Count -lt 16)
        {
            $address = [int]$addresses[$index]
            if($address -ne ($chunkStart + $chunk.Count) -or (($address -shr 16) -band 0xFFFF) -ne $currentUpper)
            {
                break
            }
            $chunk.Add([byte]$BytesByAddress[$address])
            $index++
        }

        $lines.Add((New-IntelHexRecord -RecordType 0 -Address ([UInt16]($chunkStart -band 0xFFFF)) -Data $chunk.ToArray()))
    }

    $lines.Add(':00000001FF')
    Set-Content -LiteralPath $Path -Value $lines -Encoding ASCII
}

function Merge-HexFiles {
    param(
        [Parameter(Mandatory = $true)][string]$BootloaderPath,
        [Parameter(Mandatory = $true)][string]$ApplicationPath,
        [Parameter(Mandatory = $true)][string]$OutputPath
    )

    $bootBytes = Get-IntelHexBytes -Path $BootloaderPath
    $appBytes = Get-IntelHexBytes -Path $ApplicationPath
    $combined = @{}

    foreach($address in $bootBytes.Keys)
    {
        $combined[[int]$address] = [byte]$bootBytes[$address]
    }

    foreach($address in $appBytes.Keys)
    {
        $intAddress = [int]$address
        if($combined.ContainsKey($intAddress) -and [byte]$combined[$intAddress] -ne [byte]$appBytes[$address])
        {
            throw ("HEX overlap at 0x{0:X} between bootloader and app." -f $intAddress)
        }
        $combined[$intAddress] = [byte]$appBytes[$address]
    }

    Write-IntelHex -BytesByAddress $combined -Path $OutputPath
    return [pscustomobject]@{
        BootloaderBytes = $bootBytes.Count
        ApplicationBytes = $appBytes.Count
        CombinedBytes = $combined.Count
    }
}

function Assert-ReleaseVersion {
    param([Parameter(Mandatory = $true)][string]$Version)

    if($Version -notmatch '^\d+\.\d+\.\d+$')
    {
        throw "ARDUCON_FIRMWARE_VERSION must be a plain x.y.z release version. Got '$Version'."
    }
}

function New-WorkshopSetupScript {
    param(
        [Parameter(Mandatory = $true)][string]$OutputPath,
        [Parameter(Mandatory = $true)][string]$Version,
        [Parameter(Mandatory = $true)][string]$BootloaderFileName,
        [Parameter(Mandatory = $true)][string]$ApplicationFileName,
        [Parameter(Mandatory = $true)][string]$FirstInstallFileName
    )

    $script = @"
[CmdletBinding()]
param(
    [string]`$Port = '',

    [ValidateSet('Auto', 'Avrdude', 'Atprogram')]
    [string]`$Backend = 'Auto',

    [switch]`$CheckPrereqs,

    [switch]`$CheckProgrammer,

    [switch]`$ProgramFuses,

    [switch]`$ConfirmFuseWrite,

    [switch]`$SkipSerialValidation,

    [switch]`$PreserveEeprom,

    [switch]`$SkipFlash,

    [switch]`$DryRun,

    [string]`$AvrdudePath = 'avrdude',

    [string]`$AvrdudeProgrammer = 'atmelice_isp',

    [string]`$AvrdudePort = '',

    [string]`$AvrdudeBitClock = '',

    [string]`$HighFuseValue = '',

    [string]`$ExtendedFuseValue = '',

    [string]`$AtprogramPath = 'C:\Program Files (x86)\Atmel\Studio\7.0\atbackend\atprogram.exe',

    [string]`$Tool = 'atmelice',

    [string]`$Interface = 'isp'
)

Set-StrictMode -Version Latest
`$ErrorActionPreference = 'Stop'

`$scriptRoot = `$PSScriptRoot
`$provisionScript = Join-Path `$scriptRoot 'provision-bootloader.ps1'
`$serialTestScript = Join-Path `$scriptRoot 'test-bootloader-serial.ps1'
`$bootloaderHex = Join-Path `$scriptRoot '$BootloaderFileName'
`$applicationHex = Join-Path `$scriptRoot '$ApplicationFileName'
`$combinedHex = Join-Path `$scriptRoot '$FirstInstallFileName'
`$exitCodes = @{
    Package = 8
    Prereqs = 2
    Programmer = 3
    Args = 4
    Serial = 7
    Unexpected = 20
}

function Convert-StatusValue {
    param([object]`$Value)
    if(`$null -eq `$Value) { return '' }
    return ([string]`$Value) -replace '\s+', '_'
}

function Write-SetupOk {
    param(
        [Parameter(Mandatory = `$true)][string]`$Step,
        [hashtable]`$Fields = @{}
    )
    `$parts = [System.Collections.Generic.List[string]]::new()
    `$parts.Add("SS_SETUP_OK step=`$(Convert-StatusValue `$Step)")
    foreach(`$key in @(`$Fields.Keys | Sort-Object))
    {
        `$parts.Add("`$key=`$(Convert-StatusValue `$Fields[`$key])")
    }
    Write-Host (`$parts -join ' ')
}

function Write-SetupError {
    param(
        [Parameter(Mandatory = `$true)][string]`$Step,
        [Parameter(Mandatory = `$true)][string]`$Code,
        [Parameter(Mandatory = `$true)][string]`$Detail
    )
    Write-Host "SS_SETUP_ERROR step=`$(Convert-StatusValue `$Step) code=`$(Convert-StatusValue `$Code) detail=`$(Convert-StatusValue `$Detail)"
}

function Get-LastExitCodeOrZero {
    `$lastExitCodeVariable = Get-Variable -Name LASTEXITCODE -Scope Global -ErrorAction SilentlyContinue
    if(`$lastExitCodeVariable)
    {
        return [int]`$lastExitCodeVariable.Value
    }
    return 0
}

foreach(`$required in @(`$provisionScript, `$serialTestScript, `$bootloaderHex, `$applicationHex, `$combinedHex))
{
    if(-not (Test-Path -LiteralPath `$required))
    {
        Write-SetupError -Step 'package' -Code 'missing_file' -Detail "Required setup file not found: `$required"
        exit `$exitCodes.Package
    }
}

Write-Host 'Arducon update-support setup'
Write-Host 'Board: ATmega328P'
Write-Host 'Firmware: $Version'
Write-Host ''

if(-not `$CheckPrereqs -and -not (`$ProgramFuses -and `$ConfirmFuseWrite))
{
    Write-Warning 'Bootloader installation requires both -ProgramFuses and -ConfirmFuseWrite so BOOTRST, boot size, and BODLEVEL=2.7V are set.'
    Write-Host 'To check this computer without touching the Arducon, run with -CheckPrereqs.'
    Write-Host 'To prepare a connected Arducon, run with -ProgramFuses -ConfirmFuseWrite.'
    Write-Host ''
}

if(`$CheckProgrammer -and `$CheckPrereqs)
{
    Write-SetupError -Step 'arguments' -Code 'exclusive_modes' -Detail '-CheckPrereqs and -CheckProgrammer cannot be combined.'
    exit `$exitCodes.Args
}

`$provisionArgs = @{
    ApplicationHexPath = `$applicationHex
    BootloaderHexPath = `$bootloaderHex
    CombinedHexPath = `$combinedHex
    Backend = `$Backend
    AvrdudePath = `$AvrdudePath
    AvrdudeProgrammer = `$AvrdudeProgrammer
    AvrdudePort = `$AvrdudePort
    AvrdudeBitClock = `$AvrdudeBitClock
    HighFuseValue = `$HighFuseValue
    ExtendedFuseValue = `$ExtendedFuseValue
    AtprogramPath = `$AtprogramPath
    Tool = `$Tool
    Interface = `$Interface
}

if(-not [string]::IsNullOrWhiteSpace(`$Port))
{
    `$provisionArgs.Port = `$Port
}
if(`$CheckPrereqs) { `$provisionArgs.CheckPrereqs = `$true }
if(`$CheckProgrammer) { `$provisionArgs.CheckProgrammer = `$true }
if(`$ProgramFuses) { `$provisionArgs.ProgramFuses = `$true }
if(`$ConfirmFuseWrite) { `$provisionArgs.ConfirmFuseWrite = `$true }
if(`$SkipSerialValidation) { `$provisionArgs.SkipSerialValidation = `$true }
if(`$PreserveEeprom) { `$provisionArgs.PreserveEeprom = `$true }
if(`$SkipFlash) { `$provisionArgs.SkipFlash = `$true }
if(`$DryRun) { `$provisionArgs.DryRun = `$true }

try
{
    & `$provisionScript @provisionArgs
    `$provisionExitCode = Get-LastExitCodeOrZero
    if(`$provisionExitCode -ne 0)
    {
        exit `$provisionExitCode
    }

    if(`$CheckPrereqs -or `$CheckProgrammer)
    {
        exit 0
    }

    if(`$SkipSerialValidation)
    {
        Write-SetupOk -Step 'setup-complete' -Fields @{ serialValidation = 'skipped' }
        exit 0
    }

    if([string]::IsNullOrWhiteSpace(`$Port))
    {
        Write-SetupError -Step 'serial-validation' -Code 'missing_port' -Detail 'Pass -Port for serial validation, or pass -SkipSerialValidation.'
        exit `$exitCodes.Serial
    }

    & `$serialTestScript -Port `$Port -RequestBootloaderFromApp
    `$serialExitCode = Get-LastExitCodeOrZero
    if(`$serialExitCode -ne 0)
    {
        Write-SetupError -Step 'serial-validation' -Code 'validation_failed' -Detail "Serial validation failed with exit code `$serialExitCode."
        exit `$exitCodes.Serial
    }

    Write-SetupOk -Step 'serial-validation' -Fields @{ port = `$Port }
    Write-SetupOk -Step 'setup-complete' -Fields @{ serialValidation = 'ok' }
    exit 0
}
catch
{
    Write-SetupError -Step 'setup' -Code 'unexpected' -Detail `$_.Exception.Message
    exit `$exitCodes.Unexpected
}
"@

    $script | Set-Content -LiteralPath $OutputPath -Encoding UTF8
}

if(-not $SkipBuild)
{
    if($UseMicrochipStudioBuild)
    {
        & (Join-Path $repoRoot 'build-firmware.ps1') -Configuration $Configuration
    }
    else
    {
        if($Configuration -ne 'Release')
        {
            throw 'The repo-owned CLI build currently supports Release only. Use -UseMicrochipStudioBuild for Debug packaging.'
        }
        & (Join-Path $repoRoot 'build-cli-release.ps1') -Clean
    }
}

if([string]::IsNullOrWhiteSpace($HexPath))
{
    if($UseMicrochipStudioBuild)
    {
        $HexPath = Join-Path $repoRoot "Software/AtmelStudio7/Arducon/Arducon/$Configuration/Arducon.hex"
    }
    else
    {
        $HexPath = Join-Path $repoRoot 'tmp/cli-release/Arducon.hex'
    }
}

& (Join-Path $repoRoot 'check-firmware-size.ps1') -Configuration $Configuration -HexPath $HexPath

$defsText = Get-Content -LiteralPath $defsPath -Raw
$version = Get-DefineString -Text $defsText -Name 'ARDUCON_FIRMWARE_VERSION'
Assert-ReleaseVersion -Version $version
$friendlyVersion = "v$version"

if([string]::IsNullOrWhiteSpace($OutputDir))
{
    $OutputDir = Join-Path $repoRoot "release-packages/Arducon-$friendlyVersion"
}

if(Test-Path -LiteralPath $OutputDir)
{
    Remove-Item -LiteralPath $OutputDir -Recurse -Force
}
New-Item -ItemType Directory -Path $OutputDir | Out-Null

$updateFile = "Arducon-Update-$friendlyVersion-ATmega328P.hex"
$firstInstallFile = "Arducon-First-Install-$friendlyVersion-ATmega328P.hex"
$manifestFile = "Arducon-Release-Info-$friendlyVersion-ATmega328P.json"
$checksumsFile = "Arducon-Checksums-$friendlyVersion-ATmega328P.txt"
$readmeFile = "README-Arducon-$friendlyVersion-ATmega328P.txt"
$releaseZipFile = "Arducon-$friendlyVersion-ATmega328P-Release-Files.zip"
$setupLauncherFile = "Prepare-Arducon-Updates-$friendlyVersion-ATmega328P.ps1"

$updatePath = Join-Path $OutputDir $updateFile
$firstInstallPath = Join-Path $OutputDir $firstInstallFile
$setupLauncherPath = Join-Path $OutputDir $setupLauncherFile
$provisionScriptPath = Join-Path $OutputDir 'provision-bootloader.ps1'
$serialTestScriptPath = Join-Path $OutputDir 'test-bootloader-serial.ps1'
$releaseZipPath = Join-Path $OutputDir $releaseZipFile
$files = @()
$files += Copy-PackageFile -SourcePath $HexPath -DestinationPath $updatePath -Kind 'update' -Purpose 'Application HEX for normal bootloader updates.'

$bootloaderFile = ''
$bootloaderSourceFile = ''
$bootloaderSummary = $null
if(-not (Test-Path -LiteralPath $BootloaderHexPath))
{
    throw "Bootloader HEX not found: $BootloaderHexPath"
}
if(-not (Test-Path -LiteralPath $bootloaderSourceDir))
{
    throw "Bootloader source directory not found: $bootloaderSourceDir"
}
$bootloaderFile = 'Arducon-Bootloader-Optiboot-ATmega328P.hex'
$bootloaderPath = Join-Path $OutputDir $bootloaderFile
$files += Copy-PackageFile -SourcePath $BootloaderHexPath -DestinationPath $bootloaderPath -Kind 'bootloader' -Purpose 'Arducon-patched Optiboot-compatible ATmega328P bootloader for ISP first install.'
$bootloaderImage = Get-IntelHexBytes -Path $bootloaderPath
$bootloaderSummary = Get-HexAddressSummary -Image $bootloaderImage
$mergeSummary = Merge-HexFiles -BootloaderPath $bootloaderPath -ApplicationPath $updatePath -OutputPath $firstInstallPath
$files += Copy-PackageFile -SourcePath $firstInstallPath -DestinationPath $firstInstallPath -Kind 'first-install' -Purpose 'Combined application and bootloader HEX for programming a new board with an ISP programmer.'
$bootloaderSourceFile = 'Arducon-Bootloader-Optiboot-ATmega328P-Source.zip'
$bootloaderSourcePath = Join-Path $OutputDir $bootloaderSourceFile
Compress-Archive -Path (Join-Path $bootloaderSourceDir '*') -DestinationPath $bootloaderSourcePath -Force
$files += Copy-PackageFile -SourcePath $bootloaderSourcePath -DestinationPath $bootloaderSourcePath -Kind 'bootloader-source' -Purpose 'Corresponding source and notices for the bundled Optiboot bootloader.'
New-WorkshopSetupScript -OutputPath $setupLauncherPath -Version $friendlyVersion -BootloaderFileName $bootloaderFile -ApplicationFileName $updateFile -FirstInstallFileName $firstInstallFile
$files += Copy-PackageFile -SourcePath $setupLauncherPath -DestinationPath $setupLauncherPath -Kind 'workshop-setup-launcher' -Purpose 'Friendly setup launcher for adding software-update support with a programmer.'
$files += Copy-PackageFile -SourcePath (Join-Path $repoRoot 'provision-bootloader.ps1') -DestinationPath $provisionScriptPath -Kind 'workshop-setup-tool' -Purpose 'Advanced setup tool used by the friendly setup launcher.'
$files += Copy-PackageFile -SourcePath (Join-Path $repoRoot 'test-bootloader-serial.ps1') -DestinationPath $serialTestScriptPath -Kind 'workshop-setup-tool' -Purpose 'Serial verification tool used after adding software-update support.'

$image = Get-IntelHexBytes -Path $updatePath
$updateSummary = Get-HexAddressSummary -Image $image
$first = $updateSummary.First
$last = $updateSummary.Last

$manifest = [pscustomobject]@{
    format = 'arducon-release-info-v1'
    product = 'Arducon'
    version = $version
    board = 'ATmega328P'
    update = [pscustomobject]@{
        fileName = $updateFile
        startAddress = ('0x{0:X4}' -f $first)
        bytesInImage = $image.Count
    }
    firstInstall = [pscustomobject]@{
        fileName = $firstInstallFile
        bytesInImage = $mergeSummary.CombinedBytes
    }
    bootloader = [pscustomobject]@{
        fileName = $bootloaderFile
        sourceArchiveFileName = $bootloaderSourceFile
        sourcePackage = 'arduino:avr@1.8.6 with Arducon SRAM handoff-marker patch'
        sourceFile = 'optiboot/optiboot_atmega328.hex plus local Arducon source changes'
        sourcePatch = 'Arducon app writes a one-byte SRAM marker at 0x0100 before UPD watchdog reset; Optiboot clears that marker when accepting app-requested update mode; later watchdog resets, including STK500 LEAVE_PROGMODE, jump to the application when the marker is absent; LED_START_FLASHES=0 keeps the bootloader inside 512 bytes.'
        protocol = 'stk500v1'
        baud = 115200
        highFuseTarget = '0xDE'
        highFuseTargetPreserveEeprom = '0xD6'
        extendedFuseBodLevelTarget = '0x05'
        extendedFuseBodLevelDescription = 'BODLEVEL=2.7V; preserve extended-fuse bits outside BODLEVEL[2:0]'
        startAddress = ('0x{0:X4}' -f $bootloaderSummary.First)
        endAddress = ('0x{0:X4}' -f $bootloaderSummary.Last)
        bytesInImage = $bootloaderSummary.Count
    }
    firmwareUpdate = [pscustomobject]@{
        appBaud = 57600
        updateBaud = 115200
        appInfoCommand = 'INF'
        appUpdateCommand = 'UPD'
        bootloaderEntryCommand = ''
        pageBytes = 128
        protocolVersion = 1
        bootloaderProtocol = 'stk500v1'
        bootloaderVersion = 'unknown'
        appStartAddress = '0x0000'
        flashBytes = 32768
        appLimitAddress = '0x7E00'
    }
    workshopSetup = [pscustomobject]@{
        setupLauncherFileName = $setupLauncherFile
        provisioningScriptFileName = 'provision-bootloader.ps1'
        serialValidationScriptFileName = 'test-bootloader-serial.ps1'
        highFuseTarget = '0xDE'
        highFuseTargetPreserveEeprom = '0xD6'
        extendedFuseBodLevelTarget = '0x05'
        extendedFuseBodLevelDescription = 'BODLEVEL=2.7V; preserve extended-fuse bits outside BODLEVEL[2:0]'
        supportedProgrammers = @('atmelice_isp', 'atmelice', 'avrisp2', 'usbasp')
    }
    files = $files
}

$manifestPath = Join-Path $OutputDir $manifestFile
$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding ASCII
$files += Copy-PackageFile -SourcePath $manifestPath -DestinationPath $manifestPath -Kind 'release-info' -Purpose 'Information updater tools read to choose and verify files.'

$checksumsPath = Join-Path $OutputDir $checksumsFile

$readmePath = Join-Path $OutputDir $readmeFile
@"
Arducon $friendlyVersion ATmega328P firmware package

Files:
- ${updateFile}: application HEX for normal bootloader updates.
- ${firstInstallFile}: combined application and bootloader HEX for programming a new board with an ISP programmer.
- ${bootloaderFile}: Arducon-patched Optiboot-compatible ATmega328P bootloader for ISP first install.
- ${bootloaderSourceFile}: corresponding source and notices for the bundled Optiboot bootloader.
- ${setupLauncherFile}: friendly setup launcher for adding software-update support with a programmer.
- provision-bootloader.ps1: advanced setup tool used by the friendly setup launcher.
- test-bootloader-serial.ps1: serial verification tool used after adding software-update support.
- ${manifestFile}: machine-readable update metadata.
- ${checksumsFile}: SHA-256 checksums.

Address range in update HEX: 0x$("{0:X4}" -f $first)..0x$("{0:X4}" -f $last)
Address range in bootloader HEX: 0x$("{0:X4}" -f $bootloaderSummary.First)..0x$("{0:X4}" -f $bootloaderSummary.Last)
Bootloader-safe app limit: 0x7DFF when reserving 512 bytes at top of flash.
High fuse target: 0xDE, or 0xD6 when programming EESAVE to preserve EEPROM across chip erase.
Extended fuse BODLEVEL target: low bits 0x05 for brown-out detection at VCC=2.7 V; bits outside BODLEVEL[2:0] are preserved.
Automatic bootloader fuse programming currently requires avrdude. With -Backend Auto, the setup tool selects avrdude for bootloader installation and fails prereq checks if avrdude is unavailable. Install avrdude on Windows too, or use an already-reviewed manual fuse programming process.

Updating from Arducon 1.x to $friendlyVersion requires a programming device, such as an Atmel-ICE or compatible ISP programmer, because 1.x units do not already have the new Optiboot update path installed.
"@ | Set-Content -LiteralPath $readmePath -Encoding ASCII
$files += Copy-PackageFile -SourcePath $readmePath -DestinationPath $readmePath -Kind 'readme' -Purpose 'Plain-language notes for the release folder.'

$files | ForEach-Object { "$($_.sha256)  $($_.fileName)" } | Set-Content -LiteralPath $checksumsPath -Encoding ASCII

$zipSourcePaths = @(
    $updatePath,
    $firstInstallPath,
    $bootloaderPath,
    $bootloaderSourcePath,
    $setupLauncherPath,
    $provisionScriptPath,
    $serialTestScriptPath,
    $manifestPath,
    $checksumsPath,
    $readmePath
)
Compress-Archive -LiteralPath $zipSourcePaths -DestinationPath $releaseZipPath -Force

Write-Host "Release package written to $OutputDir"
Write-Host "Manifest: $manifestPath"
Write-Host "Update file: $updateFile"
Write-Host "Complete release zip: $releaseZipFile"
Write-Host "First-install file: $firstInstallFile"
