$vcvars = "C:\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
$cmake = "C:\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$qt_path = "C:\Qt\6.5.3\msvc2019_64"

Set-Location -Path "C:\Users\truonghieu\windows-client"
$cmd = "`"$vcvars`" x64 && `"$cmake`" -B build -DCMAKE_PREFIX_PATH=`"$qt_path`" && `"$cmake`" --build build --config Release"
cmd.exe /c $cmd

# Copy bamboo.dll to the Release folder so it can be loaded
Copy-Item "C:\Users\truonghieu\bamboo.dll" -Destination "C:\Users\truonghieu\windows-client\build\Release\" -Force
