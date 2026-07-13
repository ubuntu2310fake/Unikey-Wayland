$ErrorActionPreference = "Stop"

$workspace = "C:\Users\truonghieu\windows-client"
$deployDir = "C:\Users\truonghieu\deploy"
$distDir = "$deployDir\dist"

Write-Host "=== 1. Don dep thu muc dong goi cu ==="
if (Test-Path $deployDir) {
    Remove-Item -Recurse -Force $deployDir
}
New-Item -ItemType Directory -Path $distDir | Out-Null

Write-Host "=== 2. Sao chep file thuc thi va Bamboo Core ==="
Copy-Item "$workspace\build\Release\UnikeyWayland.exe" -Destination $distDir -Force
Copy-Item "C:\Users\truonghieu\bamboo.dll" -Destination $distDir -Force

Write-Host "=== 3. Quet va thu thap toan bo thu vien Qt6 ==="
$windeployqt = "C:\Qt\6.5.3\msvc2019_64\bin\windeployqt.exe"
& $windeployqt --dir $distDir "$distDir\UnikeyWayland.exe" --no-translations --no-system-d3d-compiler

Write-Host "=== 4. Bien dich trinh cai dat Setup.exe ==="
$vcvars = "C:\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
$compileCmd = "`"$vcvars`" x64 && cl.exe /EHsc /O2 `"$workspace\Setup.cpp`" /link /MANIFESTUAC:`"level='requireAdministrator' uiAccess='false'`" /SUBSYSTEM:WINDOWS Shell32.lib Ole32.lib User32.lib uuid.lib shlwapi.lib /out:`"$deployDir\Setup.exe`""
cmd.exe /c $compileCmd

Write-Host "=== 5. Nen toan bo thu muc cai dat thanh setup.7z ==="
$7z = "C:\Program Files\7-Zip\7z.exe"
# Nen tat ca cac file trong dist vao file setup.7z
Set-Location -Path $distDir
& $7z a -t7z "$deployDir\setup.7z" "*"

Write-Host "=== 6. Don dep bo nho dem ==="
Set-Location -Path "C:\Users\truonghieu"
Remove-Item -Recurse -Force $distDir

Write-Host "====== DONG GOI HOAN TAT ======"
Write-Host "File Setup.exe va setup.7z dang nam tai C:\Users\truonghieu\deploy\"
