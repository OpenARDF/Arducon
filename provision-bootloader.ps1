[CmdletBinding()]
param(
    [string]$Port = '',

    [string]$ApplicationHexPath = '',

    [string]$BootloaderHexPath = '',

    [string]$CombinedHexPath = '',

    [string]$AtprogramPath = 'C:\Program Files (x86)\Atmel\Studio\7.0\atbackend\atprogram.exe',

    [string]$Tool = 'atmelice',

    [string]$Interface = 'isp',

    [switch]$CheckPrereqs,

    [switch]$ProgramFuses,

    [switch]$ConfirmFuseWrite,

    [switch]$SkipFlash,

    [switch]$DryRun
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = $PSScriptRoot
$device = 'atmega328p'
$desiredHighFuseMask = 0x07
$desiredHighFuseBootBits = 0x06

if([string]::IsNullOrWhiteSpace($ApplicationHexPath))
{
    $ApplicationHexPath = Join-Path $repoRoot 'Software/AtmelStudio7/Arducon/Arducon/Release/Arducon.hex'
}
if([string]::IsNullOrWhiteSpace($CombinedHexPath))
{
    $CombinedHexPath = Join-Path $repoRoot 'tmp/Arducon-bootloader-combined.hex'
}

function Test-CommandOrPath {
    param([string]$Path)
    if(Test-Path -LiteralPath $Path) { return $true }
    return [bool](Get-Command $Path -ErrorAction SilentlyContinue)
}

function Add-Prereq {
    param(
        [System.Collections.Generic.List[object]]$Results,
        [string]$Name,
        [bool]$Present,
        [bool]$Required,
        [string]$Install
    )
    $Results.Add([pscustomobject]@{
        Name = $Name
        Present = $Present
        Required = $Required
        Install = $Install
    })
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
        $data = New-Object byte[] $count
        for($i = 0; $i -lt $count; $i++)
        {
            $data[$i] = [Convert]::ToByte($line.Substring(9 + ($i * 2), 2), 16)
        }
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
            4 { $upper = ((([int]$data[0] -shl 8) -bor [int]$data[1]) -shl 16) }
            default { throw "Unsupported Intel HEX record type $recordType in $Path." }
        }
    }
    return $bytes
}

function Write-IntelHexRecord {
    param(
        [int]$Address,
        [int]$RecordType,
        [byte[]]$Data
    )
    $sum = $Data.Length + (($Address -shr 8) -band 0xFF) + ($Address -band 0xFF) + $RecordType
    $hex = ':' + ('{0:X2}{1:X4}{2:X2}' -f $Data.Length, $Address, $RecordType)
    foreach($byte in $Data)
    {
        $sum += $byte
        $hex += ('{0:X2}' -f $byte)
    }
    $checksum = ((- $sum) -band 0xFF)
    return $hex + ('{0:X2}' -f $checksum)
}

function Write-IntelHex {
    param(
        [hashtable]$BytesByAddress,
        [string]$Path
    )
    $parent = Split-Path -Parent $Path
    if($parent) { New-Item -ItemType Directory -Path $parent -Force | Out-Null }
    $lines = New-Object System.Collections.Generic.List[string]
    $currentUpper = -1
    $addresses = @($BytesByAddress.Keys | Sort-Object { [int]$_ })
    $index = 0
    while($index -lt $addresses.Count)
    {
        $absolute = [int]$addresses[$index]
        $upper = ($absolute -shr 16) -band 0xFFFF
        if($upper -ne $currentUpper)
        {
            $currentUpper = $upper
            $lines.Add((Write-IntelHexRecord -Address 0 -RecordType 4 -Data ([byte[]]@(($upper -shr 8) -band 0xFF, $upper -band 0xFF))))
        }
        $chunkStart = $absolute -band 0xFFFF
        $chunk = New-Object System.Collections.Generic.List[byte]
        while($index -lt $addresses.Count -and $chunk.Count -lt 16)
        {
            $address = [int]$addresses[$index]
            if((($address -shr 16) -band 0xFFFF) -ne $currentUpper -or (($address -band 0xFFFF) -ne ($chunkStart + $chunk.Count)))
            {
                break
            }
            $chunk.Add([byte]$BytesByAddress[$address])
            $index++
        }
        $lines.Add((Write-IntelHexRecord -Address $chunkStart -RecordType 0 -Data $chunk.ToArray()))
    }
    $lines.Add(':00000001FF')
    Set-Content -LiteralPath $Path -Value $lines -Encoding ASCII
}

function Merge-IntelHex {
    param(
        [string]$BootloaderPath,
        [string]$ApplicationPath,
        [string]$OutputPath
    )
    if([string]::IsNullOrWhiteSpace($BootloaderPath))
    {
        throw 'Pass -BootloaderHexPath to create a first-install combined image.'
    }
    $boot = Get-IntelHexBytes -Path $BootloaderPath
    $app = Get-IntelHexBytes -Path $ApplicationPath
    $merged = @{}
    foreach($address in $boot.Keys) { $merged[[int]$address] = [byte]$boot[$address] }
    foreach($address in $app.Keys)
    {
        $intAddress = [int]$address
        if($merged.ContainsKey($intAddress) -and [byte]$merged[$intAddress] -ne [byte]$app[$address])
        {
            throw ("HEX overlap at 0x{0:X}." -f $intAddress)
        }
        $merged[$intAddress] = [byte]$app[$address]
    }
    Write-IntelHex -BytesByAddress $merged -Path $OutputPath
    [pscustomobject]@{ BootloaderBytes = $boot.Count; ApplicationBytes = $app.Count; CombinedBytes = $merged.Count }
}

function Invoke-Atprogram {
    param([string[]]$Arguments)
    Write-Host ("atprogram {0}" -f ($Arguments -join ' '))
    if(-not $DryRun)
    {
        & $AtprogramPath @Arguments
        if($LASTEXITCODE -ne 0)
        {
            throw "atprogram failed with exit code $LASTEXITCODE."
        }
    }
}

$prereqs = [System.Collections.Generic.List[object]]::new()
Add-Prereq -Results $prereqs -Name 'PowerShell' -Present ($PSVersionTable.PSVersion.Major -ge 5) -Required $true -Install 'Windows: included or install PowerShell 7. macOS: brew install --cask powershell.'
Add-Prereq -Results $prereqs -Name 'atprogram' -Present (Test-CommandOrPath $AtprogramPath) -Required $true -Install 'Install Atmel Studio 7 or pass -AtprogramPath.'
Add-Prereq -Results $prereqs -Name 'Application HEX' -Present (Test-Path -LiteralPath $ApplicationHexPath) -Required (-not $SkipFlash) -Install 'Build Release firmware first.'
Add-Prereq -Results $prereqs -Name 'Bootloader HEX' -Present ((-not [string]::IsNullOrWhiteSpace($BootloaderHexPath)) -and (Test-Path -LiteralPath $BootloaderHexPath)) -Required (-not $SkipFlash) -Install 'Provide an ATmega328P Optiboot-compatible bootloader HEX with -BootloaderHexPath.'

if($CheckPrereqs)
{
    $prereqs | Format-Table -AutoSize
    $missing = @($prereqs | Where-Object { $_.Required -and -not $_.Present })
    if($missing.Count) { throw 'Required prerequisites are missing.' }
    return
}

foreach($item in $prereqs)
{
    if($item.Required -and -not $item.Present)
    {
        throw "$($item.Name) missing. $($item.Install)"
    }
}

if($ProgramFuses -and -not $ConfirmFuseWrite)
{
    throw 'Fuse writes require both -ProgramFuses and -ConfirmFuseWrite.'
}

if(-not $SkipFlash)
{
    $summary = Merge-IntelHex -BootloaderPath $BootloaderHexPath -ApplicationPath $ApplicationHexPath -OutputPath $CombinedHexPath
    Write-Host ("Combined HEX: {0} bytes ({1} bootloader + {2} app) -> {3}" -f $summary.CombinedBytes, $summary.BootloaderBytes, $summary.ApplicationBytes, $CombinedHexPath)
    Invoke-Atprogram -Arguments @('-t', $Tool, '-i', $Interface, '-d', $device, 'program', '-fl', '--verify', '-f', $CombinedHexPath)
}

if($ProgramFuses)
{
    Write-Warning 'About to set ATmega328P high-fuse boot bits for a 512-byte bootloader with BOOTRST.'
    Write-Warning 'This preserves all high-fuse bits except BOOTSZ/BOOTRST only if you supply the already-reviewed high-fuse value manually later.'
    throw 'Automatic high-fuse rewriting is intentionally not implemented yet. Read current fuses first and add a reviewed exact-write step.'
}
else
{
    Write-Host 'Fuse writes skipped. Read and verify ATmega328P fuses before production provisioning.'
    Write-Host ("Desired high-fuse boot bits: (oldHigh & 0xF8) | 0x{0:X2}" -f $desiredHighFuseBootBits)
}
