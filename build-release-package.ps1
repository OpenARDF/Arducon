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

function Assert-ReleaseVersion {
    param([Parameter(Mandatory = $true)][string]$Version)

    if($Version -notmatch '^\d+\.\d+\.\d+$')
    {
        throw "ARDUCON_FIRMWARE_VERSION must be a plain x.y.z release version. Got '$Version'."
    }
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
$manifestFile = "Arducon-Release-Info-$friendlyVersion-ATmega328P.json"
$checksumsFile = "Arducon-Checksums-$friendlyVersion-ATmega328P.txt"
$readmeFile = "README-Arducon-$friendlyVersion-ATmega328P.txt"

$updatePath = Join-Path $OutputDir $updateFile
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
$files += Copy-PackageFile -SourcePath $BootloaderHexPath -DestinationPath $bootloaderPath -Kind 'bootloader' -Purpose 'Reviewed Optiboot-compatible ATmega328P bootloader for ISP first install.'
$bootloaderImage = Get-IntelHexBytes -Path $bootloaderPath
$bootloaderSummary = Get-HexAddressSummary -Image $bootloaderImage
$bootloaderSourceFile = 'Arducon-Bootloader-Optiboot-ATmega328P-Source.zip'
$bootloaderSourcePath = Join-Path $OutputDir $bootloaderSourceFile
Compress-Archive -Path (Join-Path $bootloaderSourceDir '*') -DestinationPath $bootloaderSourcePath -Force
$files += Copy-PackageFile -SourcePath $bootloaderSourcePath -DestinationPath $bootloaderSourcePath -Kind 'bootloader-source' -Purpose 'Corresponding source and notices for the bundled Optiboot bootloader.'

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
    bootloader = [pscustomobject]@{
        fileName = $bootloaderFile
        sourceArchiveFileName = $bootloaderSourceFile
        sourcePackage = 'arduino:avr@1.8.6'
        sourceFile = 'optiboot/optiboot_atmega328.hex'
        protocol = 'stk500v1'
        baud = 115200
        highFuseTarget = '0xDE'
        highFuseTargetPreserveEeprom = '0xD6'
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
    files = $files
}

$manifestPath = Join-Path $OutputDir $manifestFile
$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding ASCII
$files += Copy-PackageFile -SourcePath $manifestPath -DestinationPath $manifestPath -Kind 'release-info' -Purpose 'Information updater tools read to choose and verify files.'

$checksumsPath = Join-Path $OutputDir $checksumsFile
$files | ForEach-Object { "$($_.sha256)  $($_.fileName)" } | Set-Content -LiteralPath $checksumsPath -Encoding ASCII

$readmePath = Join-Path $OutputDir $readmeFile
@"
Arducon $friendlyVersion ATmega328P firmware package

Files:
- ${updateFile}: application HEX for normal bootloader updates.
- ${bootloaderFile}: reviewed Optiboot-compatible ATmega328P bootloader for ISP first install.
- ${bootloaderSourceFile}: corresponding source and notices for the bundled Optiboot bootloader.
- ${manifestFile}: machine-readable update metadata.
- ${checksumsFile}: SHA-256 checksums.

Address range in update HEX: 0x$("{0:X4}" -f $first)..0x$("{0:X4}" -f $last)
Address range in bootloader HEX: 0x$("{0:X4}" -f $bootloaderSummary.First)..0x$("{0:X4}" -f $bootloaderSummary.Last)
Bootloader-safe app limit: 0x7DFF when reserving 512 bytes at top of flash.
High fuse target: 0xDE, or 0xD6 when programming EESAVE to preserve EEPROM across chip erase.
"@ | Set-Content -LiteralPath $readmePath -Encoding ASCII

Compress-Archive -Path (Join-Path $OutputDir '*') -DestinationPath (Join-Path $OutputDir "Arducon-$friendlyVersion-ATmega328P-Release-Files.zip") -Force

Write-Host "Release package written to $OutputDir"
Write-Host "Manifest: $manifestPath"
