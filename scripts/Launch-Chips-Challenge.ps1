$ErrorActionPreference = 'Stop'

$packageDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path
$python = Get-Command python3.exe, py.exe, python.exe -ErrorAction SilentlyContinue |
    Select-Object -First 1

if (-not $python) {
    Write-Host 'Python 3 is required to serve the WebAssembly game.' -ForegroundColor Red
    Write-Host 'Install Python 3, then double-click Launch-Chips-Challenge.cmd again.'
    exit 1
}

$listener = [System.Net.Sockets.TcpListener]::new([System.Net.IPAddress]::Loopback, 0)
$listener.Start()
$port = $listener.LocalEndpoint.Port
$listener.Stop()

$arguments = @()
if ($python.Name -eq 'py.exe') {
    $arguments += '-3'
}
$arguments += @('-m', 'http.server', "$port", '--bind', '127.0.0.1')

$server = $null
try {
    $server = Start-Process -FilePath $python.Source -ArgumentList $arguments `
        -WorkingDirectory $packageDirectory -WindowStyle Minimized -PassThru
    $url = "http://127.0.0.1:$port/"
    $ready = $false

    for ($attempt = 0; $attempt -lt 40; $attempt++) {
        if ($server.HasExited) {
            throw 'The local web server exited before it was ready.'
        }
        try {
            $response = Invoke-WebRequest -Uri $url -UseBasicParsing -TimeoutSec 2
            if ($response.StatusCode -eq 200 -and $response.Content -match '<canvas|chips-native\.js') {
                $ready = $true
                break
            }
        } catch {
            Start-Sleep -Milliseconds 250
        }
    }

    if (-not $ready) {
        throw 'The local web server did not become ready in time.'
    }

    Start-Process $url
    Write-Host 'Chip’s Challenge is open in your default browser.'
    Write-Host 'To stop the local server, close its minimized console window.'
} catch {
    if ($server -and -not $server.HasExited) {
        Stop-Process -Id $server.Id -Force -ErrorAction SilentlyContinue
    }
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
