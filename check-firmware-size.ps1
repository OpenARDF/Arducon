[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',

    [string]$HexPath = '',

    [string]$ElfPath = '',

    [string]$EepPath = '',

    [string]$AvrSizePath = '',

    [int]$FlashBytes = 32768,

    [int]$BootloaderBytes = 512
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if([string]::IsNullOrWhiteSpace($HexPath))
{
    $HexPath = Join-Path $PSScriptRoot "Software/AtmelStudio7/Arducon/Arducon/$Configuration/Arducon.hex"
}
if([string]::IsNullOrWhiteSpace($ElfPath))
{
    $ElfPath = [System.IO.Path]::ChangeExtension($HexPath, '.elf')
}
if([string]::IsNullOrWhiteSpace($EepPath))
{
    $EepPath = [System.IO.Path]::ChangeExtension($HexPath, '.eep')
}

function Resolve-AvrSize {
    param([string]$RequestedPath)

    $candidates = @()
    if(-not [string]::IsNullOrWhiteSpace($RequestedPath)) { $candidates += $RequestedPath }
    if($env:ARDUCON_AVR_TOOLCHAIN_BIN) { $candidates += Join-Path $env:ARDUCON_AVR_TOOLCHAIN_BIN 'avr-size' }
    $candidates += 'C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-size'
    $candidates += Join-Path "$HOME/Library/Arduino15/packages/arduino/tools/avr-gcc/7.3.0-atmel3.6.1-arduino7/bin" 'avr-size'
    $candidates += Join-Path '/Applications/microchip/xc8/v3.10/avr/bin' 'avr-size'
    foreach($candidate in $candidates)
    {
        if(Test-Path -LiteralPath $candidate) { return $candidate }
        if($IsWindows -and (Test-Path -LiteralPath "$candidate.exe")) { return "$candidate.exe" }
    }
    $command = Get-Command avr-size -ErrorAction SilentlyContinue
    if($command) { return $command.Source }
    return ''
}

function Get-IntelHexBytes {
    param([Parameter(Mandatory = $true)][string]$Path)

    if(-not (Test-Path -LiteralPath $Path))
    {
        throw "HEX file not found: $Path"
    }

    $bytes = @{}
    $upper = 0
    $lineNumber = 0

    foreach($line in Get-Content -LiteralPath $Path)
    {
        $lineNumber++
        if([string]::IsNullOrWhiteSpace($line))
        {
            continue
        }
        if(-not $line.StartsWith(':'))
        {
            throw "Invalid Intel HEX line $lineNumber in $Path."
        }

        $count = [Convert]::ToInt32($line.Substring(1, 2), 16)
        $address = [Convert]::ToInt32($line.Substring(3, 4), 16)
        $recordType = [Convert]::ToInt32($line.Substring(7, 2), 16)
        $expectedLength = 11 + ($count * 2)
        if($line.Length -lt $expectedLength)
        {
            throw "Invalid Intel HEX line length at $lineNumber in $Path."
        }

        $sum = $count + (($address -shr 8) -band 0xFF) + ($address -band 0xFF) + $recordType
        $data = New-Object byte[] $count
        for($i = 0; $i -lt $count; $i++)
        {
            $value = [Convert]::ToByte($line.Substring(9 + ($i * 2), 2), 16)
            $data[$i] = $value
            $sum += $value
        }
        $checksum = [Convert]::ToInt32($line.Substring(9 + ($count * 2), 2), 16)
        if((($sum + $checksum) -band 0xFF) -ne 0)
        {
            throw "Intel HEX checksum failed at line $lineNumber in $Path."
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
            2 {
                if($count -ne 2) { throw "Invalid extended segment address record at line $lineNumber." }
                $upper = ((([int]$data[0] -shl 8) -bor [int]$data[1]) -shl 4)
            }
            4 {
                if($count -ne 2) { throw "Invalid extended linear address record at line $lineNumber." }
                $upper = ((([int]$data[0] -shl 8) -bor [int]$data[1]) -shl 16)
            }
            default {
                throw "Unsupported Intel HEX record type $recordType at line $lineNumber."
            }
        }
    }

    return $bytes
}

$bytes = Get-IntelHexBytes -Path $HexPath
if($bytes.Count -eq 0)
{
    throw "HEX file contains no data: $HexPath"
}

$addresses = @($bytes.Keys | Sort-Object { [int]$_ })
$first = [int]($addresses | Select-Object -First 1)
$last = [int]($addresses | Select-Object -Last 1)
$appLimit = $FlashBytes - $BootloaderBytes
$remaining = $appLimit - ($last + 1)

Write-Host ("HEX: {0}" -f $HexPath)
Write-Host ("Programmed range: 0x{0:X4}..0x{1:X4} ({2} data bytes)" -f $first, $last, $bytes.Count)
Write-Host ("Bootloader reservation: {0} bytes; app limit: 0x{1:X4}" -f $BootloaderBytes, $appLimit)
Write-Host ("Remaining below app limit: {0} bytes" -f $remaining)

if(Test-Path -LiteralPath $ElfPath)
{
    $resolvedAvrSize = Resolve-AvrSize -RequestedPath $AvrSizePath
    if(-not [string]::IsNullOrWhiteSpace($resolvedAvrSize))
    {
        $sizeOutput = & $resolvedAvrSize $ElfPath
        if($LASTEXITCODE -eq 0)
        {
            $lines = @($sizeOutput | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
            if($lines.Count -ge 2)
            {
                $parts = ($lines[-1] -split '\s+') | Where-Object { $_ -ne '' }
                if($parts.Count -ge 3)
                {
                    $textBytes = [int]$parts[0]
                    $dataBytes = [int]$parts[1]
                    $bssBytes = [int]$parts[2]
                    Write-Host ("avr-size: text={0}, data={1}, bss={2}, SRAM={3}" -f $textBytes, $dataBytes, $bssBytes, ($dataBytes + $bssBytes))
                }
            }
        }
    }
}

if(Test-Path -LiteralPath $EepPath)
{
    $eepromBytes = Get-IntelHexBytes -Path $EepPath
    Write-Host ("EEPROM image data bytes: {0}" -f $eepromBytes.Count)
}

if($first -ne 0)
{
    throw ("Application HEX starts at 0x{0:X4}; expected 0x0000 for ATmega328P app image." -f $first)
}

if(($last + 1) -gt $appLimit)
{
    throw ("Application reaches 0x{0:X4}, beyond bootloader-safe limit 0x{1:X4}." -f $last, ($appLimit - 1))
}
