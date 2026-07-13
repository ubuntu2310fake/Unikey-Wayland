$vcvars = "C:\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
$cmake = "C:\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

Write-Output "=== Copying bamboo.dll ==="
Copy-Item "C:\Users\truonghieu\bamboo.dll" -Destination "C:\Users\truonghieu\windows-engine\"
Copy-Item "C:\Users\truonghieu\bamboo.dll" -Destination "C:\Users\truonghieu\windows-client\"

Write-Output "=== Building Windows TSF Engine ==="
Set-Location -Path "C:\Users\truonghieu\windows-engine"
$cmd1 = "`"$vcvars`" x64 && `"$cmake`" -S . -B build && `"$cmake`" --build build --config Release"
cmd.exe /c $cmd1

Write-Output "=== Building Qt6 Client ==="
Set-Location -Path "C:\Users\truonghieu\windows-client"
$cmd2 = "`"$vcvars`" x64 && `"$cmake`" -S . -B build -DCMAKE_PREFIX_PATH=`"C:\Qt\6.5.3\msvc2019_64`" && `"$cmake`" --build build --config Release"
cmd.exe /c $cmd2

Write-Output "=== Registering TSF COM Server ==="
$dllPath = "C:\Users\truonghieu\windows-engine\build\Release\TsfEngine.dll"
Start-Process -FilePath "regsvr32.exe" -ArgumentList "/s `"$dllPath`"" -Wait -NoNewWindow

Write-Output "=== DONE! ==="
