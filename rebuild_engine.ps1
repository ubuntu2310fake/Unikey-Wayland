$dllPath = "C:\Users\truonghieu\windows-engine\build\Release\TsfEngine.dll"
$backupPath = "C:\Users\truonghieu\windows-engine\build\Release\TsfEngine_$((Get-Date).Ticks).dll"
Move-Item $dllPath $backupPath -Force -ErrorAction SilentlyContinue

$vcvars = "C:\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
$cmake = "C:\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
Set-Location -Path "C:\Users\truonghieu\windows-engine"
$cmd = "`"$vcvars`" x64 && `"$cmake`" --build build --config Release"
cmd.exe /c $cmd

Start-Process -FilePath "regsvr32.exe" -ArgumentList "/s `"$dllPath`"" -Wait -NoNewWindow
