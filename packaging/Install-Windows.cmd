@echo off
setlocal
cd /d "%~dp0" || exit /b 1
"%~dp0better-favorites-dispatch-windows-386.exe" %*
set "result=%errorlevel%"
if "%~1"=="" pause
exit /b %result%
