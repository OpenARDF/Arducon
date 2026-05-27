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

function Read-SerialBytes {
    param(
        [System.IO.Ports.SerialPort]$SerialPort,
        [int]$Milliseconds
    )
    $deadline = [DateTime]::UtcNow.AddMilliseconds($Milliseconds)
    $bytes = [System.Collections.Generic.List[byte]]::new()
    while([DateTime]::UtcNow -lt $deadline)
    {
        while($SerialPort.BytesToRead -gt 0)
        {
            $bytes.Add([byte]$SerialPort.ReadByte())
        }
        Start-Sleep -Milliseconds 10
    }
    return $bytes.ToArray()
}

function Format-ByteList {
    param([byte[]]$Bytes)
    if(-not $Bytes -or $Bytes.Count -eq 0) { return '<none>' }
    return (($Bytes | ForEach-Object { '0x{0:X2}' -f $_ }) -join ' ')
}

function Invoke-Stk500Command {
    param(
        [System.IO.Ports.SerialPort]$SerialPort,
        [byte[]]$Command,
        [int]$ReadMilliseconds = 500
    )
    $SerialPort.DiscardInBuffer()
    $SerialPort.Write($Command, 0, $Command.Length)
    return Read-SerialBytes -SerialPort $SerialPort -Milliseconds $ReadMilliseconds
}

function Test-Stk500Sync {
    param([System.IO.Ports.SerialPort]$SerialPort)
    $response = Invoke-Stk500Command -SerialPort $SerialPort -Command ([byte[]]@(0x30, 0x20))
    if($response.Length -lt 2 -or $response[0] -ne 0x14 -or $response[$response.Length - 1] -ne 0x10)
    {
        throw "Bootloader did not answer STK500 GET_SYNC. Response: $(Format-ByteList $response)"
    }
    return $response
}

function Read-Stk500Signature {
    param([System.IO.Ports.SerialPort]$SerialPort)
    $response = Invoke-Stk500Command -SerialPort $SerialPort -Command ([byte[]]@(0x75, 0x20))
    if($response.Length -lt 5 -or $response[0] -ne 0x14 -or $response[$response.Length - 1] -ne 0x10)
    {
        throw "Bootloader did not answer STK500 READ_SIGN. Response: $(Format-ByteList $response)"
    }
    return [byte[]]@($response[1], $response[2], $response[3])
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
    if($NoReset)
    {
        Test-Stk500Sync -SerialPort $boot | Out-Null
        $signature = Read-Stk500Signature -SerialPort $boot
        Write-Host "Bootloader STK500v1 sync OK. Signature: $(Format-ByteList $signature)"
    }
    else
    {
        $lastFailure = $null
        for($attempt = 1; $attempt -le 8; $attempt++)
        {
            try
            {
                Test-Stk500Sync -SerialPort $boot | Out-Null
                $signature = Read-Stk500Signature -SerialPort $boot
                Write-Host "Bootloader STK500v1 sync OK. Signature: $(Format-ByteList $signature)"
                return
            }
            catch
            {
                $lastFailure = $_
                Start-Sleep -Milliseconds 100
            }
        }
        throw $lastFailure
    }
}
finally
{
    $boot.Close()
}
