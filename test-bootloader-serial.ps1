[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Port,

    [int]$AppBaud = 57600,

    [int]$BootBaud = 115200,

    [switch]$RequestBootloaderFromApp,

    [switch]$NoReset
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Open-SerialPort {
    param(
        [string]$PortName,
        [int]$BaudRate
    )
    $serial = [System.IO.Ports.SerialPort]::new($PortName, $BaudRate, [System.IO.Ports.Parity]::None, 8, [System.IO.Ports.StopBits]::One)
    $serial.ReadTimeout = 100
    $serial.WriteTimeout = 2000
    $serial.DtrEnable = $false
    $serial.RtsEnable = $false
    $serial.Open()
    return $serial
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

if($RequestBootloaderFromApp -and $NoReset)
{
    throw '-RequestBootloaderFromApp and -NoReset cannot be combined.'
}

$entryText = ''
if($RequestBootloaderFromApp)
{
    Write-Host "Requesting Arducon bootloader from app on $Port at $AppBaud baud..."
    $app = Open-SerialPort -PortName $Port -BaudRate $AppBaud
    try
    {
        $app.DiscardInBuffer()
        $app.Write("`r")
        $entryText += Read-SerialText -SerialPort $app -Milliseconds 500
        $app.Write("INF`r")
        $entryText += Read-SerialText -SerialPort $app -Milliseconds 1000
        if($entryText -notmatch 'INF product=Arducon')
        {
            throw "Arducon app did not respond to INF. Received: $entryText"
        }
        $app.Write("UPD`r")
        $entryText += Read-SerialText -SerialPort $app -Milliseconds 1500
        if($entryText -notmatch 'Bootloader update mode')
        {
            throw "Arducon app did not acknowledge UPD. Received: $entryText"
        }
    }
    finally
    {
        $app.Close()
    }
    Start-Sleep -Milliseconds 1200
}

Write-Host "Checking bootloader on $Port at $BootBaud baud..."
$boot = Open-SerialPort -PortName $Port -BaudRate $BootBaud
try
{
    $boot.DiscardInBuffer()
    if(-not $NoReset)
    {
        $boot.Write('0 ')
        Start-Sleep -Milliseconds 100
    }
    $boot.Write('?')
    $text = $entryText + (Read-SerialText -SerialPort $boot -Milliseconds 1500)
    if($text -notmatch 'Optiboot|BOOT|STK|AVR')
    {
        Write-Warning "Bootloader did not return a recognizable banner. Raw response follows."
        Write-Host $text
    }
    else
    {
        Write-Host 'Bootloader serial smoke test received a recognizable response.'
        Write-Host $text
    }
}
finally
{
    $boot.Close()
}
