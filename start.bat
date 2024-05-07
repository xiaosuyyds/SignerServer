@echo off
setlocal enabledelayedexpansion

for /f "tokens=2*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\QQ" /v "UninstallString"') do (
    set "RetString=%%b"
    goto :boot
)

:boot
for %%a in ("!RetString!") do (
    set "pathWithoutUninstall=%%~dpa"
)

if not exist %pathWithoutUninstall%version.dll (
    if not exist %~dp0SignerServer.dll (
        echo SignerServer.dll not found
        exit
    )
    copy /y "%~dp0SignerServer.dll" "%pathWithoutUninstall%version.dll" 1>NUL 2>NUL || powershell Start-Process -FilePath cmd.exe -ArgumentList """/c pushd %~dp0 && %~s0 %*""" -Verb RunAs && exit
)

set "QQPath=!pathWithoutUninstall!QQ.exe"
set ELECTRON_RUN_AS_NODE=1
echo !QQPath!

"!QQPath!" %~dp0load.js %*
