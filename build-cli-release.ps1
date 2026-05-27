[CmdletBinding()]
param(
    [string]$ToolchainBin = '',

    [string]$ProjectDir = '',

    [string]$OutputDir = '',

    [switch]$Clean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = $PSScriptRoot
if([string]::IsNullOrWhiteSpace($ProjectDir))
{
    $ProjectDir = Join-Path $repoRoot 'Software/AtmelStudio7/Arducon/Arducon'
}
if([string]::IsNullOrWhiteSpace($OutputDir))
{
    $OutputDir = Join-Path $repoRoot 'tmp/cli-release'
}

function Resolve-Tool {
    param(
        [Parameter(Mandatory = $true)][string]$ToolName,
        [string]$RequestedBin
    )

    $candidates = @()
    if(-not [string]::IsNullOrWhiteSpace($RequestedBin))
    {
        $candidates += Join-Path $RequestedBin $ToolName
    }
    if($env:ARDUCON_AVR_TOOLCHAIN_BIN)
    {
        $candidates += Join-Path $env:ARDUCON_AVR_TOOLCHAIN_BIN $ToolName
    }
    $candidates += "C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\$ToolName"
    $candidates += Join-Path "$HOME/Library/Arduino15/packages/arduino/tools/avr-gcc/7.3.0-atmel3.6.1-arduino7/bin" $ToolName
    $candidates += Join-Path '/Applications/microchip/xc8/v3.10/avr/bin' $ToolName

    foreach($candidate in $candidates)
    {
        if(Test-Path -LiteralPath $candidate) { return $candidate }
    }

    $command = Get-Command $ToolName -ErrorAction SilentlyContinue
    if($command) { return $command.Source }

    throw "Could not find $ToolName. Pass -ToolchainBin or set ARDUCON_AVR_TOOLCHAIN_BIN."
}

function Invoke-Tool {
    param(
        [Parameter(Mandatory = $true)][string]$ToolPath,
        [Parameter(Mandatory = $true)][string[]]$Arguments
    )

    Write-Host ("{0} {1}" -f ([System.IO.Path]::GetFileName($ToolPath)), ($Arguments -join ' '))
    & $ToolPath @Arguments
    if($LASTEXITCODE -ne 0)
    {
        throw "$([System.IO.Path]::GetFileName($ToolPath)) failed with exit code $LASTEXITCODE."
    }
}

function Get-ProjectSources {
    param([Parameter(Mandatory = $true)][string]$ProjectPath)

    [xml]$project = Get-Content -LiteralPath $ProjectPath -Raw
    $sources = New-Object System.Collections.Generic.List[string]
    foreach($node in $project.GetElementsByTagName('Compile'))
    {
        $include = $node.GetAttribute('Include')
        if($include -and $include.EndsWith('.cpp', [StringComparison]::OrdinalIgnoreCase))
        {
            $sources.Add($include)
        }
    }
    return $sources.ToArray()
}

function Get-SizeSummary {
    param(
        [Parameter(Mandatory = $true)][string]$AvrSize,
        [Parameter(Mandatory = $true)][string]$ElfPath
    )

    $output = & $AvrSize $ElfPath
    if($LASTEXITCODE -ne 0)
    {
        throw "avr-size failed with exit code $LASTEXITCODE."
    }
    $lines = @($output | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
    if($lines.Count -lt 2)
    {
        throw "Unexpected avr-size output: $($output -join ' ')"
    }
    $parts = ($lines[-1] -split '\s+') | Where-Object { $_ -ne '' }
    if($parts.Count -lt 6)
    {
        throw "Unexpected avr-size data row: $($lines[-1])"
    }
    [pscustomobject]@{
        text = [int]$parts[0]
        data = [int]$parts[1]
        bss = [int]$parts[2]
        dec = [int]$parts[3]
        hex = $parts[4]
        file = $parts[5]
        sramBytes = [int]$parts[1] + [int]$parts[2]
        raw = $output
    }
}

function Get-IntelHexSummary {
    param([Parameter(Mandatory = $true)][string]$Path)

    if(-not (Test-Path -LiteralPath $Path))
    {
        throw "HEX file not found: $Path"
    }

    $addresses = New-Object System.Collections.Generic.List[int]
    $upper = 0
    foreach($line in Get-Content -LiteralPath $Path)
    {
        if([string]::IsNullOrWhiteSpace($line)) { continue }
        if(-not $line.StartsWith(':')) { throw "Invalid Intel HEX line in $Path." }
        $count = [Convert]::ToInt32($line.Substring(1, 2), 16)
        $address = [Convert]::ToInt32($line.Substring(3, 4), 16)
        $recordType = [Convert]::ToInt32($line.Substring(7, 2), 16)
        switch($recordType)
        {
            0 {
                for($i = 0; $i -lt $count; $i++)
                {
                    $addresses.Add($upper + $address + $i)
                }
            }
            1 { break }
            2 {
                $data = [Convert]::ToInt32($line.Substring(9, 4), 16)
                $upper = $data -shl 4
            }
            4 {
                $data = [Convert]::ToInt32($line.Substring(9, 4), 16)
                $upper = $data -shl 16
            }
            default { throw "Unsupported Intel HEX record type $recordType in $Path." }
        }
    }

    if($addresses.Count -eq 0)
    {
        return [pscustomobject]@{ firstAddress = $null; lastAddress = $null; byteCount = 0 }
    }
    $sorted = @($addresses | Sort-Object)
    [pscustomobject]@{
        firstAddress = [int]($sorted | Select-Object -First 1)
        lastAddress = [int]($sorted | Select-Object -Last 1)
        byteCount = $addresses.Count
    }
}

$cppproj = Join-Path $ProjectDir 'Arducon.cppproj'
if(-not (Test-Path -LiteralPath $cppproj))
{
    throw "Atmel Studio project file not found: $cppproj"
}

$avrGpp = Resolve-Tool -ToolName 'avr-g++' -RequestedBin $ToolchainBin
$avrObjcopy = Resolve-Tool -ToolName 'avr-objcopy' -RequestedBin $ToolchainBin
$avrObjdump = Resolve-Tool -ToolName 'avr-objdump' -RequestedBin $ToolchainBin
$avrSize = Resolve-Tool -ToolName 'avr-size' -RequestedBin $ToolchainBin

if($Clean -and (Test-Path -LiteralPath $OutputDir))
{
    Remove-Item -LiteralPath $OutputDir -Recurse -Force
}
New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null

$sources = Get-ProjectSources -ProjectPath $cppproj
if($sources.Count -eq 0)
{
    throw "No C++ source files found in $cppproj."
}

$objectPaths = New-Object System.Collections.Generic.List[string]
$compileFlags = @(
    '-funsigned-char',
    '-funsigned-bitfields',
    '-DATMEL_STUDIO_7',
    '-DNDEBUG',
    '-Os',
    '-ffunction-sections',
    '-fdata-sections',
    '-fpack-struct',
    '-fshort-enums',
    '-Wall',
    '-mmcu=atmega328p',
    '-c'
)

foreach($source in $sources)
{
    $sourcePath = Join-Path $ProjectDir $source
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($source)
    $objectPath = Join-Path $OutputDir "$baseName.o"
    $depPath = Join-Path $OutputDir "$baseName.d"
    $objectPaths.Add($objectPath)
    Invoke-Tool -ToolPath $avrGpp -Arguments ($compileFlags + @(
        '-MD',
        '-MP',
        '-MF', $depPath,
        '-MT', $depPath,
        '-MT', $objectPath,
        '-o', $objectPath,
        $sourcePath
    ))
}

$elfPath = Join-Path $OutputDir 'Arducon.elf'
$hexPath = Join-Path $OutputDir 'Arducon.hex'
$eepPath = Join-Path $OutputDir 'Arducon.eep'
$lssPath = Join-Path $OutputDir 'Arducon.lss'
$srecPath = Join-Path $OutputDir 'Arducon.srec'
$mapPath = Join-Path $OutputDir 'Arducon.map'

Invoke-Tool -ToolPath $avrGpp -Arguments (@(
    "-o$elfPath"
) + $objectPaths.ToArray() + @(
    "-Wl,-Map=$mapPath",
    '-Wl,--start-group',
    '-Wl,-lm',
    '-Wl,--end-group',
    '-Wl,--gc-sections',
    '-mmcu=atmega328p'
))

Invoke-Tool -ToolPath $avrObjcopy -Arguments @('-O', 'ihex', '-R', '.eeprom', '-R', '.fuse', '-R', '.lock', '-R', '.signature', '-R', '.user_signatures', $elfPath, $hexPath)
Invoke-Tool -ToolPath $avrObjcopy -Arguments @('-j', '.eeprom', '--set-section-flags=.eeprom=alloc,load', '--change-section-lma', '.eeprom=0', '--no-change-warnings', '-O', 'ihex', $elfPath, $eepPath)
& $avrObjdump -h -S $elfPath | Set-Content -LiteralPath $lssPath -Encoding ASCII
if($LASTEXITCODE -ne 0)
{
    throw "avr-objdump failed with exit code $LASTEXITCODE."
}
Invoke-Tool -ToolPath $avrObjcopy -Arguments @('-O', 'srec', '-R', '.eeprom', '-R', '.fuse', '-R', '.lock', '-R', '.signature', '-R', '.user_signatures', $elfPath, $srecPath)

$size = Get-SizeSummary -AvrSize $avrSize -ElfPath $elfPath
$hexSummary = Get-IntelHexSummary -Path $hexPath
$eepSummary = Get-IntelHexSummary -Path $eepPath
$size.raw | ForEach-Object { Write-Host $_ }

& (Join-Path $repoRoot 'check-firmware-size.ps1') -Configuration Release -HexPath $hexPath

$summary = [pscustomobject]@{
    build = 'cli-release'
    toolchainBin = Split-Path -Parent $avrGpp
    outputDir = $OutputDir
    elf = $elfPath
    hex = $hexPath
    map = $mapPath
    eep = $eepPath
    lss = $lssPath
    srec = $srecPath
    elfBytes = (Get-Item -LiteralPath $elfPath).Length
    hexBytes = (Get-Item -LiteralPath $hexPath).Length
    mapBytes = (Get-Item -LiteralPath $mapPath).Length
    textBytes = $size.text
    dataBytes = $size.data
    bssBytes = $size.bss
    programBytes = $size.text + $size.data
    sramBytes = $size.sramBytes
    hexFirstAddress = if($null -eq $hexSummary.firstAddress) { $null } else { '0x{0:X4}' -f $hexSummary.firstAddress }
    hexLastAddress = if($null -eq $hexSummary.lastAddress) { $null } else { '0x{0:X4}' -f $hexSummary.lastAddress }
    hexDataBytes = $hexSummary.byteCount
    eepromBytes = $eepSummary.byteCount
}
$summaryPath = Join-Path $OutputDir 'Arducon-cli-release-summary.json'
$summary | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $summaryPath -Encoding ASCII
Write-Host "CLI Release build summary written to $summaryPath"
