[CmdletBinding()]
param(
    [string]$Port = '',

    [string]$ApplicationHexPath = '',

    [string]$BootloaderHexPath = '',

    [string]$CombinedHexPath = '',

    [ValidateSet('Auto', 'Avrdude', 'Atprogram')]
    [string]$Backend = 'Auto',

    [string]$AvrdudePath = 'avrdude',

    [string]$AvrdudeProgrammer = 'atmelice_isp',

    [string]$AvrdudePort = '',

    [string]$AvrdudeBitClock = '',

    [string]$HighFuseValue = '',

    [string]$AtprogramPath = 'C:\Program Files (x86)\Atmel\Studio\7.0\atbackend\atprogram.exe',

    [string]$Tool = 'atmelice',

    [string]$Interface = 'isp',

    [switch]$CheckPrereqs,

    [switch]$CheckProgrammer,

    [switch]$ProgramFuses,

    [switch]$ConfirmFuseWrite,

    [switch]$ReadFusesOnly,

    [switch]$ChipErase,

    [switch]$PreserveEeprom,

    [switch]$SkipFlash,

    [switch]$SkipSerialValidation,

    [switch]$DryRun
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = $PSScriptRoot
$device = 'atmega328p'
$desiredHighFuseBootBits = 0x06
$highFuseEesaveBit = 0x08
$exitCodes = @{
    Prereqs = 2
    Programmer = 3
    Args = 4
    Flash = 5
    Fuses = 6
    Serial = 7
    Unexpected = 20
}

if([string]::IsNullOrWhiteSpace($ApplicationHexPath))
{
    $cliHexPath = Join-Path $repoRoot 'tmp/cli-release/Arducon.hex'
    if(Test-Path -LiteralPath $cliHexPath)
    {
        $ApplicationHexPath = $cliHexPath
    }
    else
    {
        $ApplicationHexPath = Join-Path $repoRoot 'Software/AtmelStudio7/Arducon/Arducon/Release/Arducon.hex'
    }
}
if([string]::IsNullOrWhiteSpace($BootloaderHexPath))
{
    $BootloaderHexPath = Join-Path $repoRoot 'Bootloaders/optiboot-atmega328p-arduino-1.8.6/optiboot_atmega328.hex'
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

function Resolve-CommandOrPath {
    param([string]$Path)
    if(Test-Path -LiteralPath $Path) { return $Path }
    $command = Get-Command $Path -ErrorAction SilentlyContinue
    if($command) { return $command.Source }
    return $Path
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

function Convert-StatusValue {
    param([object]$Value)
    if($null -eq $Value) { return '' }
    return ([string]$Value) -replace '\s+', '_'
}

function Write-SetupOk {
    param(
        [Parameter(Mandatory = $true)][string]$Step,
        [hashtable]$Fields = @{}
    )
    $parts = [System.Collections.Generic.List[string]]::new()
    $parts.Add("SS_SETUP_OK step=$(Convert-StatusValue $Step)")
    foreach($key in @($Fields.Keys | Sort-Object))
    {
        $parts.Add("$key=$(Convert-StatusValue $Fields[$key])")
    }
    Write-Host ($parts -join ' ')
}

function Write-SetupError {
    param(
        [Parameter(Mandatory = $true)][string]$Step,
        [Parameter(Mandatory = $true)][string]$Code,
        [Parameter(Mandatory = $true)][string]$Detail
    )
    Write-Host "SS_SETUP_ERROR step=$(Convert-StatusValue $Step) code=$(Convert-StatusValue $Code) detail=$(Convert-StatusValue $Detail)"
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
            3 { }
            4 { $upper = ((([int]$data[0] -shl 8) -bor [int]$data[1]) -shl 16) }
            5 { }
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
            $upperData = [byte[]]@(
                [byte](($upper -shr 8) -band 0xFF),
                [byte]($upper -band 0xFF)
            )
            $lines.Add((Write-IntelHexRecord -Address 0 -RecordType 4 -Data $upperData))
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

function Get-AvrdudeBaseArguments {
    $arguments = @('-p', 'm328p', '-c', $AvrdudeProgrammer)
    if(-not [string]::IsNullOrWhiteSpace($AvrdudePort))
    {
        $arguments += @('-P', $AvrdudePort)
    }
    if(-not [string]::IsNullOrWhiteSpace($AvrdudeBitClock))
    {
        $arguments += @('-B', $AvrdudeBitClock)
    }
    return $arguments
}

function Invoke-Avrdude {
    param([string[]]$Arguments)
    $toolPath = Resolve-CommandOrPath $AvrdudePath
    Write-Host ("avrdude {0}" -f ($Arguments -join ' '))
    if(-not $DryRun)
    {
        & $toolPath @Arguments
        if($LASTEXITCODE -ne 0)
        {
            throw "avrdude failed with exit code $LASTEXITCODE."
        }
    }
}

function Invoke-AvrdudeCapture {
    param([string[]]$Arguments)
    $toolPath = Resolve-CommandOrPath $AvrdudePath
    Write-Host ("avrdude {0}" -f ($Arguments -join ' '))
    if($DryRun)
    {
        return ''
    }

    $output = & $toolPath @Arguments 2>&1 | ForEach-Object { "$_" }
    $exitCode = $LASTEXITCODE
    $text = $output -join [Environment]::NewLine
    if($text) { Write-Host $text }
    if($exitCode -ne 0)
    {
        throw "avrdude failed with exit code $exitCode."
    }
    return $text
}

function Test-ProgrammerAvrdude {
    $output = Invoke-AvrdudeCapture -Arguments (Get-AvrdudeBaseArguments)
    if($output -match 'Device signature\s*=\s*(0x[0-9a-fA-F]+)')
    {
        return $Matches[1]
    }
    return 'unknown'
}

function Test-ProgrammerAtprogram {
    Invoke-Atprogram -Arguments @('-t', $Tool, '-i', $Interface, '-d', $device, 'info')
    return 'unknown'
}

function Read-HighFuseAvrdude {
    $output = Invoke-AvrdudeCapture -Arguments ((Get-AvrdudeBaseArguments) + @('-U', 'hfuse:r:-:h'))
    $match = [regex]::Matches($output, '0x[0-9a-fA-F]{2}|(?m)^[0-9a-fA-F]{2}$') | Select-Object -Last 1
    if(-not $match)
    {
        throw 'Could not parse high fuse from avrdude output.'
    }
    return [Convert]::ToInt32($match.Value, 16)
}

function Convert-HexByte {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Value,

        [string]$Name = 'value'
    )
    $trimmed = $Value.Trim()
    if($trimmed -notmatch '^(0x)?[0-9a-fA-F]{1,2}$')
    {
        throw "$Name must be a one-byte hexadecimal value such as 0xDA."
    }
    return [Convert]::ToInt32(($trimmed -replace '^0x', ''), 16)
}

function Get-SelectedBackend {
    if($Backend -ne 'Auto') { return $Backend }
    if(Test-CommandOrPath $AvrdudePath) { return 'Avrdude' }
    return 'Atprogram'
}

try
{
    $selectedBackend = Get-SelectedBackend
    $prereqs = [System.Collections.Generic.List[object]]::new()
    Add-Prereq -Results $prereqs -Name 'PowerShell' -Present ($PSVersionTable.PSVersion.Major -ge 5) -Required $true -Install 'Windows: included or install PowerShell 7. macOS: brew install --cask powershell.'
    Add-Prereq -Results $prereqs -Name 'avrdude' -Present (Test-CommandOrPath $AvrdudePath) -Required ($selectedBackend -eq 'Avrdude') -Install 'Install avrdude or pass -AvrdudePath.'
    Add-Prereq -Results $prereqs -Name 'atprogram' -Present (Test-CommandOrPath $AtprogramPath) -Required ($selectedBackend -eq 'Atprogram') -Install 'Install Atmel Studio 7 or pass -AtprogramPath.'
    Add-Prereq -Results $prereqs -Name 'Application HEX' -Present (Test-Path -LiteralPath $ApplicationHexPath) -Required (-not $SkipFlash -and -not $CheckProgrammer) -Install 'Build Release firmware first.'
    Add-Prereq -Results $prereqs -Name 'Bootloader HEX' -Present ((-not [string]::IsNullOrWhiteSpace($BootloaderHexPath)) -and (Test-Path -LiteralPath $BootloaderHexPath)) -Required (-not $SkipFlash -and -not $CheckProgrammer) -Install 'Provide an ATmega328P Optiboot-compatible bootloader HEX with -BootloaderHexPath.'

    if($CheckPrereqs)
    {
        $prereqs | Format-Table -AutoSize
        $missing = @($prereqs | Where-Object { $_.Required -and -not $_.Present })
        if($missing.Count)
        {
            Write-SetupError -Step 'check-prereqs' -Code 'missing_prereq' -Detail ($missing[0].Name)
            exit $exitCodes.Prereqs
        }
        Write-SetupOk -Step 'check-prereqs' -Fields @{ backend = $selectedBackend }
        exit 0
    }

    foreach($item in $prereqs)
    {
        if($item.Required -and -not $item.Present)
        {
            Write-SetupError -Step 'check-prereqs' -Code 'missing_prereq' -Detail "$($item.Name): $($item.Install)"
            exit $exitCodes.Prereqs
        }
    }

    if($CheckProgrammer)
    {
        $signature = if($selectedBackend -eq 'Avrdude') { Test-ProgrammerAvrdude } else { Test-ProgrammerAtprogram }
        Write-SetupOk -Step 'check-programmer' -Fields @{ backend = $selectedBackend; signature = $signature }
        exit 0
    }

    if($ProgramFuses -and -not $ConfirmFuseWrite)
    {
        Write-SetupError -Step 'program-fuses' -Code 'confirm_required' -Detail 'Fuse writes require both -ProgramFuses and -ConfirmFuseWrite.'
        exit $exitCodes.Args
    }

    if($ReadFusesOnly -and $selectedBackend -ne 'Avrdude')
    {
        Write-SetupError -Step 'read-fuses' -Code 'unsupported_backend' -Detail '-ReadFusesOnly currently requires -Backend Avrdude.'
        exit $exitCodes.Args
    }

    if($ProgramFuses -and $selectedBackend -ne 'Avrdude')
    {
        Write-SetupError -Step 'program-fuses' -Code 'unsupported_backend' -Detail 'Automatic high-fuse rewriting currently requires -Backend Avrdude.'
        exit $exitCodes.Args
    }

    if($selectedBackend -eq 'Avrdude')
    {
        $oldHighFuse = $null
        $newHighFuse = $null
        if(-not [string]::IsNullOrWhiteSpace($HighFuseValue))
        {
            $oldHighFuse = Convert-HexByte -Value $HighFuseValue -Name 'HighFuseValue'
        }
        elseif(-not $DryRun)
        {
            $oldHighFuse = Read-HighFuseAvrdude
        }
        elseif($ProgramFuses -or $ReadFusesOnly)
        {
            Write-SetupError -Step 'read-fuses' -Code 'high_fuse_required' -Detail 'Dry-run fuse operations require -HighFuseValue because hardware is not read in dry-run mode.'
            exit $exitCodes.Args
        }

        if($null -ne $oldHighFuse)
        {
            $newHighFuse = ($oldHighFuse -band 0xF8) -bor $desiredHighFuseBootBits
            if($PreserveEeprom)
            {
                $newHighFuse = $newHighFuse -band 0xF7
            }

            Write-Host ("ATmega328P high fuse: current=0x{0:X2}; bootloader target=0x{1:X2}" -f $oldHighFuse, $newHighFuse)
            if($PreserveEeprom)
            {
                Write-Host ("Transform: newHigh = ((oldHigh & 0xF8) | 0x{0:X2}) & 0xF7  # 512-byte bootloader, BOOTRST, EESAVE" -f $desiredHighFuseBootBits)
            }
            else
            {
                Write-Host ("Transform: newHigh = (oldHigh & 0xF8) | 0x{0:X2}" -f $desiredHighFuseBootBits)
            }
            Write-SetupOk -Step 'read-fuses' -Fields @{ highFuse = ('0x{0:X2}' -f $oldHighFuse); targetHighFuse = ('0x{0:X2}' -f $newHighFuse) }
        }
        else
        {
            Write-Host ("Dry run: high fuse not read. Pass -HighFuseValue to preview the exact bootloader high fuse.")
            Write-SetupOk -Step 'read-fuses' -Fields @{ highFuse = 'skipped' }
        }

        if($ReadFusesOnly)
        {
            exit 0
        }
    }
    elseif($ReadFusesOnly)
    {
        exit 0
    }

    if(-not $SkipFlash)
    {
        $summary = Merge-IntelHex -BootloaderPath $BootloaderHexPath -ApplicationPath $ApplicationHexPath -OutputPath $CombinedHexPath
        Write-Host ("Combined HEX: {0} bytes ({1} bootloader + {2} app) -> {3}" -f $summary.CombinedBytes, $summary.BootloaderBytes, $summary.ApplicationBytes, $CombinedHexPath)
        if($selectedBackend -eq 'Avrdude')
        {
            $flashArgs = Get-AvrdudeBaseArguments
            if($ChipErase)
            {
                $effectiveHighFuse = if($ProgramFuses -and ($null -ne $newHighFuse)) { $newHighFuse } else { $oldHighFuse }
                if(($null -ne $effectiveHighFuse) -and (($effectiveHighFuse -band $highFuseEesaveBit) -eq 0))
                {
                    Write-Host ("Chip erase requested; EESAVE is programmed in the effective high fuse 0x{0:X2}, so EEPROM should be preserved." -f $effectiveHighFuse)
                }
                else
                {
                    Write-Warning 'Chip erase can erase EEPROM unless the EESAVE fuse is programmed. Use -PreserveEeprom with -ProgramFuses -ConfirmFuseWrite to program EESAVE.'
                }
                $flashArgs += '-e'
            }
            else
            {
                $flashArgs += '-D'
            }
            $flashArgs += @('-U', "flash:w:${CombinedHexPath}:i")
            Invoke-Avrdude -Arguments $flashArgs
        }
        else
        {
            Invoke-Atprogram -Arguments @('-t', $Tool, '-i', $Interface, '-d', $device, 'program', '-fl', '--verify', '-f', $CombinedHexPath)
        }
        Write-SetupOk -Step 'program-flash' -Fields @{ backend = $selectedBackend; bytes = $summary.CombinedBytes }
    }
    else
    {
        Write-SetupOk -Step 'program-flash' -Fields @{ status = 'skipped' }
    }

    if($ProgramFuses)
    {
        if($null -eq $newHighFuse)
        {
            Write-SetupError -Step 'program-fuses' -Code 'high_fuse_unavailable' -Detail 'No high-fuse value is available. Read the fuse from hardware or pass -HighFuseValue.'
            exit $exitCodes.Fuses
        }
        $fuseArgs = (Get-AvrdudeBaseArguments) + @('-U', ('hfuse:w:0x{0:X2}:m' -f $newHighFuse))
        Write-Warning ("Writing ATmega328P high fuse from 0x{0:X2} to 0x{1:X2}. Unrelated high-fuse bits are preserved." -f $oldHighFuse, $newHighFuse)
        if($PreserveEeprom)
        {
            Write-Warning 'EESAVE will be programmed so future chip erase operations preserve EEPROM.'
        }
        Invoke-Avrdude -Arguments $fuseArgs
        if($DryRun)
        {
            Write-Host 'Dry run: high fuse verify skipped.'
        }
        else
        {
            $verifiedHighFuse = Read-HighFuseAvrdude
            if($verifiedHighFuse -ne $newHighFuse)
            {
                Write-SetupError -Step 'program-fuses' -Code 'verify_failed' -Detail ("Expected 0x{0:X2}, read 0x{1:X2}." -f $newHighFuse, $verifiedHighFuse)
                exit $exitCodes.Fuses
            }
            Write-Host ("High fuse verified: 0x{0:X2}" -f $verifiedHighFuse)
        }
        Write-SetupOk -Step 'program-fuses' -Fields @{ highFuse = ('0x{0:X2}' -f $newHighFuse) }
    }
    else
    {
        Write-Host 'Fuse writes skipped.'
        if($PreserveEeprom)
        {
            Write-Host ("Desired high-fuse boot bits plus EESAVE: ((oldHigh & 0xF8) | 0x{0:X2}) & 0xF7" -f $desiredHighFuseBootBits)
            Write-Warning '-PreserveEeprom only takes effect when fuse writes are enabled with -ProgramFuses -ConfirmFuseWrite.'
        }
        else
        {
            Write-Host ("Desired high-fuse boot bits: (oldHigh & 0xF8) | 0x{0:X2}" -f $desiredHighFuseBootBits)
        }
        Write-SetupOk -Step 'program-fuses' -Fields @{ status = 'skipped' }
    }

    if($SkipSerialValidation)
    {
        Write-SetupOk -Step 'serial-validation' -Fields @{ status = 'skipped' }
    }
}
catch
{
    $step = 'setup'
    $code = 'unexpected'
    $exitCode = $exitCodes.Unexpected
    $message = $_.Exception.Message
    if($message -match 'avrdude|atprogram|programmer|Device signature')
    {
        $step = 'check-programmer'
        $code = 'programmer_failed'
        $exitCode = $exitCodes.Programmer
    }
    elseif($message -match 'HEX|flash|program')
    {
        $step = 'program-flash'
        $code = 'flash_failed'
        $exitCode = $exitCodes.Flash
    }
    elseif($message -match 'fuse')
    {
        $step = 'program-fuses'
        $code = 'fuse_failed'
        $exitCode = $exitCodes.Fuses
    }
    Write-SetupError -Step $step -Code $code -Detail $message
    exit $exitCode
}
