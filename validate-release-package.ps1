[CmdletBinding()]
param(
    [string]$PackageDir = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if([string]::IsNullOrWhiteSpace($PackageDir))
{
    $root = Join-Path $PSScriptRoot 'release-packages'
    if(-not (Test-Path -LiteralPath $root))
    {
        throw "Release package directory not found: $root"
    }
    $PackageDir = Get-ChildItem -LiteralPath $root -Directory |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1 -ExpandProperty FullName
}

function ConvertFrom-HexAddress {
    param([Parameter(Mandatory = $true)][string]$Text)
    if($Text -match '^0x([0-9A-Fa-f]+)$') { return [Convert]::ToInt32($matches[1], 16) }
    if($Text -match '^\d+$') { return [int]$Text }
    throw "Invalid address: $Text"
}

function Get-IntelHexBytes {
    param([Parameter(Mandatory = $true)][string]$Path)

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

function Test-ZipContainsEntries {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string[]]$RequiredEntries
    )

    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [System.IO.Compression.ZipFile]::OpenRead($Path)
    try
    {
        $entryNames = @($zip.Entries | ForEach-Object { $_.FullName })
        foreach($entry in $RequiredEntries)
        {
            if($entryNames -notcontains $entry)
            {
                throw "Source archive $Path is missing $entry."
            }
        }
    }
    finally
    {
        $zip.Dispose()
    }
}

function Assert-ReleaseVersion {
    param([Parameter(Mandatory = $true)][string]$Version)

    if($Version -notmatch '^\d+\.\d+\.\d+$')
    {
        throw "Release version must be a plain x.y.z version. Got '$Version'."
    }
}

if(-not (Test-Path -LiteralPath $PackageDir))
{
    throw "Release package directory not found: $PackageDir"
}

$manifestPath = Get-ChildItem -LiteralPath $PackageDir -Filter 'Arducon-Release-Info-*.json' |
    Select-Object -First 1 -ExpandProperty FullName
if(-not $manifestPath)
{
    throw "No Arducon release-info manifest found in $PackageDir."
}

$manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
if($manifest.format -ne 'arducon-release-info-v1')
{
    throw "Unsupported manifest format: $($manifest.format)"
}
if($manifest.product -ne 'Arducon')
{
    throw "Unexpected product: $($manifest.product)"
}
Assert-ReleaseVersion -Version $manifest.version

$checksumsPath = @(Get-ChildItem -LiteralPath $PackageDir -Filter 'Arducon-Checksums-*.txt' -File)
if($checksumsPath.Count -ne 1)
{
    throw "Expected exactly one Arducon checksums file, found $($checksumsPath.Count)."
}
$checksumsPath = $checksumsPath[0].FullName

$releaseZipPath = @(Get-ChildItem -LiteralPath $PackageDir -Filter 'Arducon-*-Release-Files.zip' -File)
if($releaseZipPath.Count -ne 1)
{
    throw "Expected exactly one Arducon release zip file, found $($releaseZipPath.Count)."
}
$releaseZipPath = $releaseZipPath[0].FullName

$settings = $manifest.firmwareUpdate
$appStart = ConvertFrom-HexAddress $settings.appStartAddress
$appLimit = ConvertFrom-HexAddress $settings.appLimitAddress
if($appStart -ne 0)
{
    throw ("Unexpected app start 0x{0:X}; ATmega328P app should start at 0x0000." -f $appStart)
}
if($settings.appUpdateCommand -ne 'UPD')
{
    throw "Unexpected app update command: $($settings.appUpdateCommand)"
}
if($settings.appInfoCommand -ne 'INF')
{
    throw "Unexpected app info command: $($settings.appInfoCommand)"
}

$updatePath = Join-Path $PackageDir $manifest.update.fileName
if(-not (Test-Path -LiteralPath $updatePath))
{
    throw "Update HEX not found: $updatePath"
}

$hexBytes = Get-IntelHexBytes -Path $updatePath
$addresses = @($hexBytes.Keys | Sort-Object { [int]$_ })
$first = [int]($addresses | Select-Object -First 1)
$last = [int]($addresses | Select-Object -Last 1)
if($first -ne $appStart)
{
    throw ("Update HEX starts at 0x{0:X}; expected 0x{1:X}." -f $first, $appStart)
}
if($last -ge $appLimit)
{
    throw ("Update HEX reaches 0x{0:X}; app limit is 0x{1:X}." -f $last, ($appLimit - 1))
}
if($hexBytes.Count -ne [int]$manifest.update.bytesInImage)
{
    throw "Update HEX byte count mismatch."
}

$firstInstallPath = $null
$firstInstallBytes = $null
$firstInstallSummary = $null
$manifestPropertyNames = @($manifest.PSObject.Properties.Name)
if($manifestPropertyNames -contains 'firstInstall')
{
    $firstInstallFileName = $manifest.firstInstall.fileName
    if([string]::IsNullOrWhiteSpace($firstInstallFileName))
    {
        throw 'First-install manifest is missing fileName.'
    }
    $firstInstallFileEntry = $manifest.files | Where-Object { $_.kind -eq 'first-install' -and $_.fileName -eq $firstInstallFileName } | Select-Object -First 1
    if(-not $firstInstallFileEntry)
    {
        throw "Manifest does not list first-install file $firstInstallFileName."
    }
    $firstInstallPath = Join-Path $PackageDir $firstInstallFileName
    if(-not (Test-Path -LiteralPath $firstInstallPath))
    {
        throw "First-install HEX not found: $firstInstallPath"
    }
    $firstInstallBytes = Get-IntelHexBytes -Path $firstInstallPath
    $firstInstallSummary = Get-HexAddressSummary -Image $firstInstallBytes
    if($firstInstallSummary.First -ne 0)
    {
        throw ("First-install HEX starts at 0x{0:X}; expected 0x0000." -f $firstInstallSummary.First)
    }
    if($firstInstallSummary.Last -lt $last)
    {
        throw 'First-install HEX does not include the full update image.'
    }
    if($firstInstallBytes.Count -ne [int]$manifest.firstInstall.bytesInImage)
    {
        throw 'First-install HEX byte count mismatch.'
    }
}
else
{
    throw 'Manifest is missing firstInstall metadata.'
}

if($manifestPropertyNames -contains 'bootloader')
{
    $bootloader = $manifest.bootloader
    $bootloaderFileEntry = $manifest.files | Where-Object { $_.kind -eq 'bootloader' -and $_.fileName -eq $bootloader.fileName } | Select-Object -First 1
    if(-not $bootloaderFileEntry)
    {
        throw "Manifest does not list bootloader file $($bootloader.fileName)."
    }

    $bootloaderPath = Join-Path $PackageDir $bootloader.fileName
    if(-not (Test-Path -LiteralPath $bootloaderPath))
    {
        throw "Bootloader HEX not found: $bootloaderPath"
    }

    $sourceArchiveFileName = $bootloader.sourceArchiveFileName
    if([string]::IsNullOrWhiteSpace($sourceArchiveFileName))
    {
        throw 'Bootloader manifest is missing sourceArchiveFileName.'
    }
    $sourceArchiveFileEntry = $manifest.files | Where-Object { $_.kind -eq 'bootloader-source' -and $_.fileName -eq $sourceArchiveFileName } | Select-Object -First 1
    if(-not $sourceArchiveFileEntry)
    {
        throw "Manifest does not list bootloader source archive $sourceArchiveFileName."
    }
    $sourceArchivePath = Join-Path $PackageDir $sourceArchiveFileName
    if(-not (Test-Path -LiteralPath $sourceArchivePath))
    {
        throw "Bootloader source archive not found: $sourceArchivePath"
    }
    Test-ZipContainsEntries -Path $sourceArchivePath -RequiredEntries @('optiboot.c', 'README.TXT', 'Makefile')

    if($bootloader.protocol -ne 'stk500v1')
    {
        throw "Unexpected bootloader protocol: $($bootloader.protocol)"
    }
    if([int]$bootloader.baud -ne [int]$settings.updateBaud)
    {
        throw "Bootloader baud does not match update baud."
    }
    if($bootloader.highFuseTarget -ne '0xDE')
    {
        throw "Unexpected bootloader high-fuse target: $($bootloader.highFuseTarget)"
    }
    $bootloaderPropertyNames = @($bootloader.PSObject.Properties.Name)
    if($bootloaderPropertyNames -contains 'highFuseTargetPreserveEeprom')
    {
        if($bootloader.highFuseTargetPreserveEeprom -ne '0xD6')
        {
            throw "Unexpected EEPROM-preserving bootloader high-fuse target: $($bootloader.highFuseTargetPreserveEeprom)"
        }
    }

    $bootloaderBytes = Get-IntelHexBytes -Path $bootloaderPath
    $bootloaderAddresses = @($bootloaderBytes.Keys | Sort-Object { [int]$_ })
    $bootloaderFirst = [int]($bootloaderAddresses | Select-Object -First 1)
    $bootloaderLast = [int]($bootloaderAddresses | Select-Object -Last 1)
    $manifestBootloaderStart = ConvertFrom-HexAddress $bootloader.startAddress
    $manifestBootloaderEnd = ConvertFrom-HexAddress $bootloader.endAddress
    $expectedBootStart = $appLimit
    $flashLast = [int]$settings.flashBytes - 1
    if($bootloaderFirst -ne $expectedBootStart)
    {
        throw ("Bootloader HEX starts at 0x{0:X}; expected 0x{1:X}." -f $bootloaderFirst, $expectedBootStart)
    }
    if($bootloaderLast -gt $flashLast)
    {
        throw ("Bootloader HEX reaches 0x{0:X}; flash ends at 0x{1:X}." -f $bootloaderLast, $flashLast)
    }
    if($bootloaderFirst -ne $manifestBootloaderStart -or $bootloaderLast -ne $manifestBootloaderEnd)
    {
        throw "Bootloader HEX address range does not match manifest."
    }
    if($bootloaderBytes.Count -ne [int]$bootloader.bytesInImage)
    {
        throw "Bootloader HEX byte count mismatch."
    }

    if($null -ne $firstInstallBytes)
    {
        foreach($address in $hexBytes.Keys)
        {
            $intAddress = [int]$address
            if(-not $firstInstallBytes.ContainsKey($intAddress) -or [byte]$firstInstallBytes[$intAddress] -ne [byte]$hexBytes[$address])
            {
                throw ("First-install HEX does not match update HEX at 0x{0:X}." -f $intAddress)
            }
        }
        foreach($address in $bootloaderBytes.Keys)
        {
            $intAddress = [int]$address
            if(-not $firstInstallBytes.ContainsKey($intAddress) -or [byte]$firstInstallBytes[$intAddress] -ne [byte]$bootloaderBytes[$address])
            {
                throw ("First-install HEX does not match bootloader HEX at 0x{0:X}." -f $intAddress)
            }
        }
    }
}
else
{
    throw 'Manifest is missing bootloader metadata.'
}

if($manifestPropertyNames -notcontains 'workshopSetup')
{
    throw 'Manifest is missing workshopSetup metadata.'
}

$workshopSetup = $manifest.workshopSetup
$setupLauncherFileName = $workshopSetup.setupLauncherFileName
$provisioningScriptFileName = $workshopSetup.provisioningScriptFileName
$serialValidationScriptFileName = $workshopSetup.serialValidationScriptFileName
foreach($scriptFileName in @($setupLauncherFileName, $provisioningScriptFileName, $serialValidationScriptFileName))
{
    if([string]::IsNullOrWhiteSpace($scriptFileName))
    {
        throw 'Workshop setup script metadata is missing a file name.'
    }
    $scriptPath = Join-Path $PackageDir $scriptFileName
    if(-not (Test-Path -LiteralPath $scriptPath))
    {
        throw "Workshop setup script not found: $scriptFileName"
    }
}

$setupLauncherEntry = $manifest.files | Where-Object { $_.kind -eq 'workshop-setup-launcher' -and $_.fileName -eq $setupLauncherFileName } | Select-Object -First 1
if(-not $setupLauncherEntry)
{
    throw "Manifest does not list setup launcher file $setupLauncherFileName."
}

$provisioningEntry = $manifest.files | Where-Object { $_.fileName -eq $provisioningScriptFileName } | Select-Object -First 1
if(-not $provisioningEntry)
{
    throw "Manifest does not list provisioning script $provisioningScriptFileName."
}

$serialValidationEntry = $manifest.files | Where-Object { $_.fileName -eq $serialValidationScriptFileName } | Select-Object -First 1
if(-not $serialValidationEntry)
{
    throw "Manifest does not list serial validation script $serialValidationScriptFileName."
}

if($workshopSetup.highFuseTarget -ne '0xDE')
{
    throw "Unexpected workshop high-fuse target: $($workshopSetup.highFuseTarget)"
}
if($workshopSetup.highFuseTargetPreserveEeprom -ne '0xD6')
{
    throw "Unexpected workshop EEPROM-preserving high-fuse target: $($workshopSetup.highFuseTargetPreserveEeprom)"
}

$supportedProgrammers = @($workshopSetup.supportedProgrammers)
foreach($programmer in @('atmelice_isp', 'atmelice', 'avrisp2', 'usbasp'))
{
    if($supportedProgrammers -notcontains $programmer)
    {
        throw "Workshop setup metadata is missing supported programmer $programmer."
    }
}

$setupLauncherText = Get-Content -LiteralPath (Join-Path $PackageDir $setupLauncherFileName) -Raw
foreach($requiredOption in @('CheckPrereqs', 'CheckProgrammer', 'ProgramFuses', 'ConfirmFuseWrite', 'SkipSerialValidation', 'Backend', 'Port'))
{
    if($setupLauncherText -notmatch [regex]::Escape($requiredOption))
    {
        throw "Setup launcher is missing option $requiredOption."
    }
}
foreach($requiredStatus in @('SS_SETUP_OK', 'SS_SETUP_ERROR'))
{
    if($setupLauncherText -notmatch $requiredStatus)
    {
        throw "Setup launcher is missing status marker $requiredStatus."
    }
}

foreach($file in $manifest.files)
{
    $path = Join-Path $PackageDir $file.fileName
    if(-not (Test-Path -LiteralPath $path))
    {
        throw "Manifest file missing: $($file.fileName)"
    }
    $bytes = [System.IO.File]::ReadAllBytes($path)
    if($file.sizeBytes -and $bytes.Length -ne [int]$file.sizeBytes)
    {
        throw "Size mismatch for $($file.fileName)."
    }
    $actualHash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    if($actualHash -ne $file.sha256.ToLowerInvariant())
    {
        throw "SHA-256 mismatch for $($file.fileName)."
    }
}

$checksums = @{}
foreach($line in Get-Content -LiteralPath $checksumsPath)
{
    if([string]::IsNullOrWhiteSpace($line)) { continue }
    if($line -notmatch '^([0-9A-Fa-f]{64})\s+(.+)$')
    {
        throw "Invalid checksum line: $line"
    }
    $checksums[$Matches[2]] = $Matches[1].ToLowerInvariant()
}

foreach($name in $checksums.Keys)
{
    $path = Join-Path $PackageDir $name
    if(-not (Test-Path -LiteralPath $path))
    {
        throw "Checksum-listed file missing: $name"
    }
    $actualHash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    if($actualHash -ne $checksums[$name])
    {
        throw "Checksum file hash mismatch for $name."
    }
}

foreach($file in $manifest.files)
{
    if(-not $checksums.ContainsKey($file.fileName))
    {
        throw "Checksum file does not list manifest file $($file.fileName)."
    }
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
$zipArchive = [System.IO.Compression.ZipFile]::OpenRead($releaseZipPath)
try
{
    $zipEntryNames = @($zipArchive.Entries | ForEach-Object { $_.Name })
    foreach($entry in $manifest.files)
    {
        if($zipEntryNames -notcontains $entry.fileName)
        {
            throw "Release zip does not contain $($entry.fileName)."
        }
    }
    foreach($name in $checksums.Keys)
    {
        if($zipEntryNames -notcontains $name)
        {
            throw "Release zip does not contain checksum-listed file $name."
        }
    }
    foreach($requiredFile in @((Split-Path -Leaf $manifestPath), (Split-Path -Leaf $checksumsPath)))
    {
        if($zipEntryNames -notcontains $requiredFile)
        {
            throw "Release zip does not contain $requiredFile."
        }
    }
}
finally
{
    if($null -ne $zipArchive)
    {
        $zipArchive.Dispose()
    }
}

Write-Host "Release package validated: $PackageDir"
Write-Host ("Update HEX range: 0x{0:X4}..0x{1:X4}; bytes: {2}" -f $first, $last, $hexBytes.Count)
if($null -ne $firstInstallSummary)
{
    Write-Host ("First-install HEX range: 0x{0:X4}..0x{1:X4}; bytes: {2}" -f $firstInstallSummary.First, $firstInstallSummary.Last, $firstInstallBytes.Count)
}
if($manifestPropertyNames -contains 'bootloader')
{
    Write-Host ("Bootloader HEX range: 0x{0:X4}..0x{1:X4}; bytes: {2}" -f $bootloaderFirst, $bootloaderLast, $bootloaderBytes.Count)
}
Write-Host ("Release zip OK: {0}" -f (Split-Path -Leaf $releaseZipPath))
