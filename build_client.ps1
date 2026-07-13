$vcvars = "C:\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
$cmake = "C:\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

Set-Location -Path "C:\Users\truonghieu\windows-client"
$cmd = "`"$vcvars`" x64 && `"$cmake`" --build build --config Release"
cmd.exe /c $cmd
