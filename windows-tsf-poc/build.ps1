$vcvars = "C:\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
if (-Not (Test-Path $vcvars)) {
    Write-Error "Could not find vcvarsall.bat. MSVC Build Tools may not be installed."
    exit 1
}

$cmd = "`"$vcvars`" x64 && cl.exe /LD /MD /EHsc /W4 /Fe:TsfPoC.dll TsfPoC.cpp TsfPoC.def ole32.lib oleaut32.lib user32.lib advapi32.lib"
cmd.exe /c $cmd

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed!"
    exit $LASTEXITCODE
}
Write-Output "Build succeeded: TsfPoC.dll"
