[CmdletBinding()]
param(
    [string]$CliSummaryPath = '',

    [string]$AtmelReleaseDir = '',

    [int]$BaselineHexDataBytes = 29014,

    [string]$BaselineHexLastAddress = '0x7155',

    [int]$BaselineSramBytes = 1629,

    [int]$BaselineEepromBytes = 203,

    [int]$AllowedSramDelta = 8
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = $PSScriptRoot
if([string]::IsNullOrWhiteSpace($CliSummaryPath))
{
    $CliSummaryPath = Join-Path $repoRoot 'tmp/cli-release/Arducon-cli-release-summary.json'
}
if([string]::IsNullOrWhiteSpace($AtmelReleaseDir))
{
    $AtmelReleaseDir = Join-Path $repoRoot 'Software/AtmelStudio7/Arducon/Arducon/Release'
}

if(-not (Test-Path -LiteralPath $CliSummaryPath))
{
    throw "CLI Release summary not found: $CliSummaryPath. Run build-cli-release.ps1 first."
}

$summary = Get-Content -LiteralPath $CliSummaryPath -Raw | ConvertFrom-Json
$failures = New-Object System.Collections.Generic.List[string]

function Add-Comparison {
    param(
        [System.Collections.Generic.List[object]]$Rows,
        [string]$Name,
        [object]$Cli,
        [object]$Baseline,
        [bool]$Pass
    )

    $Rows.Add([pscustomobject]@{
        Metric = $Name
        CLI = $Cli
        Baseline = $Baseline
        Pass = $Pass
    })
}

$rows = New-Object System.Collections.Generic.List[object]
Add-Comparison -Rows $rows -Name 'HEX data bytes' -Cli $summary.hexDataBytes -Baseline $BaselineHexDataBytes -Pass ([int]$summary.hexDataBytes -eq $BaselineHexDataBytes)
Add-Comparison -Rows $rows -Name 'HEX last address' -Cli $summary.hexLastAddress -Baseline $BaselineHexLastAddress -Pass ($summary.hexLastAddress -eq $BaselineHexLastAddress)
Add-Comparison -Rows $rows -Name 'SRAM bytes' -Cli $summary.sramBytes -Baseline "$BaselineSramBytes (+/- $AllowedSramDelta)" -Pass ([Math]::Abs([int]$summary.sramBytes - $BaselineSramBytes) -le $AllowedSramDelta)
Add-Comparison -Rows $rows -Name 'EEPROM bytes' -Cli $summary.eepromBytes -Baseline $BaselineEepromBytes -Pass ([int]$summary.eepromBytes -eq $BaselineEepromBytes)

foreach($row in $rows)
{
    if(-not $row.Pass)
    {
        $failures.Add("$($row.Metric): CLI=$($row.CLI), baseline=$($row.Baseline)")
    }
}

$atmelElf = Join-Path $AtmelReleaseDir 'Arducon.elf'
$atmelHex = Join-Path $AtmelReleaseDir 'Arducon.hex'
$atmelMap = Join-Path $AtmelReleaseDir 'Arducon.map'
$artifactRows = New-Object System.Collections.Generic.List[object]
foreach($pair in @(
    @{ Name = 'ELF'; Cli = $summary.elf; Atmel = $atmelElf },
    @{ Name = 'HEX'; Cli = $summary.hex; Atmel = $atmelHex },
    @{ Name = 'MAP'; Cli = $summary.map; Atmel = $atmelMap }
))
{
    $atmelExists = Test-Path -LiteralPath $pair.Atmel
    $artifactRows.Add([pscustomobject]@{
        Artifact = $pair.Name
        CliPath = $pair.Cli
        AtmelPath = $pair.Atmel
        AtmelPresent = $atmelExists
        CliSha256 = if(Test-Path -LiteralPath $pair.Cli) { (Get-FileHash -LiteralPath $pair.Cli -Algorithm SHA256).Hash.ToLowerInvariant() } else { '' }
        AtmelSha256 = if($atmelExists) { (Get-FileHash -LiteralPath $pair.Atmel -Algorithm SHA256).Hash.ToLowerInvariant() } else { '' }
    })
}

Write-Host 'Microchip Studio baseline from CODEX_MAILBOX.md:'
Write-Host ("  HEX range: 0x0000..{0}; bytes: {1}" -f $BaselineHexLastAddress.Substring(2), $BaselineHexDataBytes)
Write-Host ("  SRAM/data report: {0} bytes; EEPROM: {1} bytes" -f $BaselineSramBytes, $BaselineEepromBytes)
Write-Host ''
Write-Host 'CLI Release comparison:'
$rows | Format-Table -AutoSize
Write-Host ''
Write-Host 'Artifact comparison paths and hashes:'
$artifactRows | Format-Table -AutoSize

if($failures.Count)
{
    throw "CLI Release comparison failed: $($failures -join '; ')"
}

Write-Host 'CLI Release comparison passed.'
