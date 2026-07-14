@echo off
title Unikey-Wayland Installer
setlocal enabledelayedexpansion

:: 1. Kiem tra va nang quyen Administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [Setup] Dang yeu cau quyen Administrator...
    powershell -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    exit /b
)

:: Thiet lap duong dan nguon va dich
set "SRC_DIR=%~dp0"
set "DEST_DIR=C:\Program Files\UnikeyWayland"
set "ZIP_FILE=%SRC_DIR%UnikeyWayland.7z"
set "ZIP_EXE=%SRC_DIR%7z.exe"

echo ========================================================
echo   Cai dat Unikey-Wayland (Windows Edition)
echo ========================================================
echo.

:: 2. Kiem tra cac file can thiet
if not exist "%ZIP_FILE%" (
    echo [Loi] Khong tim thay file "%ZIP_FILE%"
    echo Vui long dam bao UnikeyWayland.7z nam cung thu muc voi file setup.bat nay.
    pause
    exit /b 1
)

if not exist "%ZIP_EXE%" (
    echo [Loi] Khong tim thay file "%ZIP_EXE%"
    echo Vui long dam bao 7z.exe nam cung thu muc voi file setup.bat nay.
    pause
    exit /b 1
)

:: 3. Tao thu muc cai dat
echo [1/4] Dang tao thu muc cai dat: %DEST_DIR%...
if not exist "%DEST_DIR%" (
    mkdir "%DEST_DIR%"
)

:: 4. Giai nen ung dung
echo [2/4] Dang giai nen UnikeyWayland.7z vao thu muc cai dat...
"%ZIP_EXE%" x -y "%ZIP_FILE%" -o"%DEST_DIR%"
if %errorLevel% neq 0 (
    echo [Loi] Giai nen that bai! Co the thu muc dang bi khoa.
    pause
    exit /b 1
)

:: 5. Tao Shortcuts
echo [3/4] Dang tao Shortcut ngoai Desktop va Start Menu...
set "TARGET_EXE=%DEST_DIR%\UnikeyWayland.exe"

:: Desktop Shortcut
powershell -Command "$WshShell = New-Object -ComObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut([System.IO.Path]::Combine([System.Environment]::GetFolderPath('CommonDesktopDirectory'), 'Unikey-Wayland.lnk')); $Shortcut.TargetPath = '%TARGET_EXE%'; $Shortcut.WorkingDirectory = '%DEST_DIR%'; $Shortcut.Save()"

:: Start Menu Shortcut
powershell -Command "$WshShell = New-Object -ComObject WScript.Shell; $StartMenu = [System.IO.Path]::Combine([System.Environment]::GetFolderPath('CommonPrograms'), 'Unikey-Wayland'); if (-not (Test-Path $StartMenu)) { New-Item -ItemType Directory -Path $StartMenu | Out-Null }; $Shortcut = $WshShell.CreateShortcut([System.IO.Path]::Combine($StartMenu, 'Unikey-Wayland.lnk')); $Shortcut.TargetPath = '%TARGET_EXE%'; $Shortcut.WorkingDirectory = '%DEST_DIR%'; $Shortcut.Save()"

:: 6. Chay ung dung luon
echo [4/4] Dang khoi chay bo go Unikey-Wayland...
start "" "%TARGET_EXE%"

echo.
echo ========================================================
echo   Cai dat Unikey-Wayland hoan tat thanh cong!
echo ========================================================
echo.
pause
