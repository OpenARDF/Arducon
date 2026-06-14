[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Port,

    [string]$ManifestPath,

    [string]$JavaPath,

    [string]$SerialSlingerClasspath,

    [int]$AppBaud = 57600,

    [int]$StopSettleMilliseconds = 2500,

    [switch]$SkipStop,

    [switch]$SkipUpdate
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-RepoRoot {
    return Split-Path -Parent $PSCommandPath
}

function Find-ManifestPath {
    $repoRoot = Get-RepoRoot
    $manifests = @(Get-ChildItem -LiteralPath (Join-Path $repoRoot 'release-packages') -Filter 'Arducon-Release-Info-*-ATmega328P.json' -Recurse -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTimeUtc -Descending)

    if($manifests.Count -eq 0)
    {
        throw 'No Arducon release manifest found. Run `just package-skip-build` first.'
    }

    return $manifests[0].FullName
}

function Resolve-JavaPath {
    param([string]$RequestedPath)

    if(-not [string]::IsNullOrWhiteSpace($RequestedPath))
    {
        return $RequestedPath
    }

    if($env:ARDUCON_FLASH_JAVA)
    {
        return $env:ARDUCON_FLASH_JAVA
    }

    $javaHome = (& /usr/libexec/java_home -v 17 2>$null)
    if($LASTEXITCODE -eq 0 -and -not [string]::IsNullOrWhiteSpace($javaHome))
    {
        return (Join-Path $javaHome 'bin/java')
    }

    return 'java'
}

function Resolve-SerialSlingerClasspath {
    param([string]$RequestedClasspath)

    if(-not [string]::IsNullOrWhiteSpace($RequestedClasspath))
    {
        return $RequestedClasspath
    }

    if($env:SERIALSLINGER_CLASSPATH)
    {
        return $env:SERIALSLINGER_CLASSPATH
    }

    $candidate = '/Users/charlesscharlau/Documents/GitHub/SerialSlinger/shared/build/packaging/input/*'
    if(Test-Path -LiteralPath (Split-Path -Parent $candidate))
    {
        return $candidate
    }

    $installed = '/Applications/SerialSlinger.app/Contents/app/*'
    if(Test-Path -LiteralPath (Split-Path -Parent $installed))
    {
        return $installed
    }

    throw 'SerialSlinger classpath not found. Pass -SerialSlingerClasspath or set SERIALSLINGER_CLASSPATH.'
}

function Read-SerialText {
    param(
        [System.IO.Ports.SerialPort]$SerialPort,
        [int]$Milliseconds
    )

    $deadline = [DateTime]::UtcNow.AddMilliseconds($Milliseconds)
    $text = ''
    while([DateTime]::UtcNow -lt $deadline)
    {
        try
        {
            $text += $SerialPort.ReadExisting()
        }
        catch
        {
        }
        Start-Sleep -Milliseconds 25
    }

    return $text
}

function Stop-ArduconEvent {
    param(
        [string]$PortName,
        [int]$Baud,
        [int]$SettleMilliseconds
    )

    Write-Host "Stopping any running Arducon event on $PortName..."
    $serial = [System.IO.Ports.SerialPort]::new($PortName, $Baud, [System.IO.Ports.Parity]::None, 8, [System.IO.Ports.StopBits]::Two)
    $serial.ReadTimeout = 100
    $serial.WriteTimeout = 1000
    $serial.DtrEnable = $false
    $serial.RtsEnable = $false
    $serial.Open()

    try
    {
        Start-Sleep -Milliseconds 750
        for($attempt = 1; $attempt -le 3; $attempt++)
        {
            $serial.DiscardInBuffer()
            $serial.Write("SYN 0`r")
            [void](Read-SerialText -SerialPort $serial -Milliseconds 500)
            Start-Sleep -Milliseconds 250
        }

        Start-Sleep -Milliseconds $SettleMilliseconds
        $serial.DiscardInBuffer()
        $serial.Write("INF`r")
        $info = Read-SerialText -SerialPort $serial -Milliseconds 1500
        if($info -notmatch 'INF product=Arducon')
        {
            Write-Warning "Arducon did not answer INF after stop command. SerialSlinger will still attempt the update."
        }
    }
    finally
    {
        $serial.Close()
    }
}

$manifest = if([string]::IsNullOrWhiteSpace($ManifestPath)) { Find-ManifestPath } else { (Resolve-Path -LiteralPath $ManifestPath).Path }
$java = Resolve-JavaPath -RequestedPath $JavaPath
$classpath = Resolve-SerialSlingerClasspath -RequestedClasspath $SerialSlingerClasspath

if(-not $SkipStop)
{
    Stop-ArduconEvent -PortName $Port -Baud $AppBaud -SettleMilliseconds $StopSettleMilliseconds
}
else
{
    Write-Host 'Skipping Arducon stop step.'
}

if($SkipUpdate)
{
    Write-Host "Skipping update. Manifest: $manifest"
    exit 0
}

Write-Host "Flashing Arducon with SerialSlinger..."
& $java -cp $classpath com.openardf.serialslinger.cli.DesktopSmokeCliKt arducon-update $Port $manifest
exit $LASTEXITCODE
