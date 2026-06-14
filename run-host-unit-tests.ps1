[CmdletBinding()]
param(
    [string]$Cxx = '',

    [string]$BuildDir = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = $PSScriptRoot
if([string]::IsNullOrWhiteSpace($BuildDir))
{
    $BuildDir = Join-Path $repoRoot 'tmp/host-tests'
}

function Resolve-Cxx {
    param([string]$RequestedCxx)

    if(-not [string]::IsNullOrWhiteSpace($RequestedCxx))
    {
        return $RequestedCxx
    }

    if($env:CXX)
    {
        return $env:CXX
    }

    foreach($candidate in @('clang++', 'g++', 'c++'))
    {
        $command = Get-Command $candidate -ErrorAction SilentlyContinue
        if($command)
        {
            return $command.Source
        }
    }

    throw 'Could not find a host C++ compiler. Install clang++ or g++, or pass -Cxx.'
}

$cxxPath = Resolve-Cxx -RequestedCxx $Cxx
New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null

$exeExtension = if($IsWindows) { '.exe' } else { '' }
$testExe = Join-Path $BuildDir "scheduler_logic_tests$exeExtension"
$testSource = Join-Path $repoRoot 'tests/scheduler_logic_tests.cpp'

$compileArgs = @(
    '-std=c++17',
    '-Wall',
    '-Wextra',
    '-pedantic',
    '-O0',
    '-g',
    '-o', $testExe,
    $testSource
)

Write-Host ("{0} {1}" -f ([System.IO.Path]::GetFileName($cxxPath)), ($compileArgs -join ' '))
& $cxxPath @compileArgs
if($LASTEXITCODE -ne 0)
{
    throw "Host unit-test build failed with exit code $LASTEXITCODE."
}

Write-Host $testExe
& $testExe
if($LASTEXITCODE -ne 0)
{
    throw "Host unit tests failed with exit code $LASTEXITCODE."
}
