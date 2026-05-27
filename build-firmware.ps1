[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',

    [string]$MakePath = '',

    [switch]$SkipSizeCheck
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = $PSScriptRoot
$projectRoot = Join-Path $repoRoot 'Software/AtmelStudio7/Arducon/Arducon'
$buildDir = Join-Path $projectRoot $Configuration
$makefilePath = Join-Path $buildDir 'Makefile'
$elfPath = Join-Path $buildDir 'Arducon.elf'

function Assert-PathExists {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,

        [Parameter(Mandatory = $true)]
        [string]$Description
    )

    if(-not (Test-Path -LiteralPath $Path))
    {
        throw "$Description not found: $Path"
    }
}

function Resolve-MakePath {
    param([string]$RequestedPath)

    if(-not [string]::IsNullOrWhiteSpace($RequestedPath))
    {
        return $RequestedPath
    }

    $atmelMake = 'C:\Program Files (x86)\Atmel\Studio\7.0\shellutils\make.exe'
    if(Test-Path -LiteralPath $atmelMake)
    {
        return $atmelMake
    }

    $command = Get-Command make -ErrorAction SilentlyContinue
    if($command)
    {
        return $command.Source
    }

    throw 'make was not found. Install Atmel Studio 7 on Windows or pass -MakePath.'
}

Assert-PathExists -Path $projectRoot -Description 'Atmel Studio project directory'
Assert-PathExists -Path $buildDir -Description "$Configuration build directory"
Assert-PathExists -Path $makefilePath -Description "$Configuration Makefile. Open Atmel Studio 7 and build this configuration once so it generates the Makefile."

$resolvedMake = Resolve-MakePath -RequestedPath $MakePath
Write-Host "Building Arducon $Configuration firmware with $resolvedMake..."
& $resolvedMake -C $buildDir all
if($LASTEXITCODE -ne 0)
{
    throw "Firmware build failed with exit code $LASTEXITCODE."
}

Assert-PathExists -Path $elfPath -Description "$Configuration ELF image"
Write-Host "Firmware build completed: $elfPath"

if(-not $SkipSizeCheck)
{
    & (Join-Path $repoRoot 'check-firmware-size.ps1') -Configuration $Configuration
}
