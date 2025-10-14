@echo off
setlocal enabledelayedexpansion

rem
for /f "tokens=2*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\QQ" /v "UninstallString"') do (
    rem 使用 %%~b 可以直接移除 %%b 内容两端的引号
    set "UninstallFullPath=%%~b"
    goto :found_path
)

echo QQ installation not found
pause
exit

:found_path
rem 直接从净化后的完整路径中提取目录
for %%a in ("!UninstallFullPath!") do (
    set "pathWithoutUninstall=%%~dpa"
)

if not exist "%pathWithoutUninstall%dbghelp.dll" (
    if not exist %~dp0SignerServer.dll (
        echo SignerServer.dll not found
        pause
        exit
    )
    goto copydll
) else (
    set /p Choice="dbghelp.dll already exists, do you want to replace it?(Y/N):"
    if /i "!Choice!"=="Y" goto copydll
    if /i "!Choice!"=="y" goto copydll
    goto launch
)

:copydll
copy /y "%~dp0SignerServer.dll" "%pathWithoutUninstall%dbghelp.dll"
if errorlevel 1 (
    set /p Choice="Copy error, do you want to attempt to running as administrator?(Y/N):"
    if /i "!Choice!"=="Y" goto restart
    if /i "!Choice!"=="y" goto restart
    exit
)
goto launch

:restart
powershell Start-Process -FilePath cmd.exe -ArgumentList """/c pushd %~dp0 && %~s0 %*""" -Verb RunAs
exit

:launch
set "QQPath=!pathWithoutUninstall!QQ.exe"
set ELECTRON_RUN_AS_NODE=1

echo Launching QQ
"!QQPath!" "%~dp0load.js" %*