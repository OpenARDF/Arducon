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

$manifestPropertyNames = @($manifest.PSObject.Properties.Name)
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

Write-Host "Release package validated: $PackageDir"
Write-Host ("Update HEX range: 0x{0:X4}..0x{1:X4}; bytes: {2}" -f $first, $last, $hexBytes.Count)
if($manifestPropertyNames -contains 'bootloader')
{
    Write-Host ("Bootloader HEX range: 0x{0:X4}..0x{1:X4}; bytes: {2}" -f $bootloaderFirst, $bootloaderLast, $bootloaderBytes.Count)
}
