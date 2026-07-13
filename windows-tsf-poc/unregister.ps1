$dllPath = Join-Path $PWD "TsfPoC.dll"
Write-Output "Unregistering COM Server and TSF Profile..."
Start-Process -FilePath "regsvr32.exe" -ArgumentList "/s /u `"$dllPath`"" -Wait -NoNewWindow
Write-Output "Unregistration complete."
