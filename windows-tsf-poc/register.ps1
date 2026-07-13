$dllPath = Join-Path $PWD "TsfPoC.dll"
if (-Not (Test-Path $dllPath)) {
    Write-Error "TsfPoC.dll not found. Please build first."
    exit 1
}

Write-Output "Registering COM Server and TSF Profile..."
Start-Process -FilePath "regsvr32.exe" -ArgumentList "/s `"$dllPath`"" -Wait -NoNewWindow
Write-Output "Registration complete."
