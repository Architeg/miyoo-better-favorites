@echo off
setlocal
if /i "%~nx0"=="entry.cmd" goto staged
if not exist "%~dp0computer\transport.json" goto advanced
set "BF_CARD_SOURCE=%~dp0."
:stage
set "BF_EXEC_STAGE=%TEMP%\better-favorites-bootstrap-%RANDOM%-%RANDOM%"
if exist "%BF_EXEC_STAGE%" goto stage
mkdir "%BF_EXEC_STAGE%" || exit /b 1
copy /b "%~f0" "%BF_EXEC_STAGE%\entry.cmd" >nul || goto failed
copy /b "%~dp0computer\better-favorites-dispatch-windows-386.exe" "%BF_EXEC_STAGE%\dispatch.exe" >nul || goto failed
cd /d "%BF_EXEC_STAGE%" || goto failed
rem No CALL: transfer the batch context off-card before complete uninstall.
"%BF_EXEC_STAGE%\entry.cmd" %*
exit /b 1
:staged
if "%BF_CARD_SOURCE%"=="" exit /b 1
"%~dp0dispatch.exe" --card-launcher "%BF_CARD_SOURCE%" %*
set "result=%errorlevel%"
if "%~1"=="" pause
cd /d "%TEMP%" || exit /b 1
rem This block is parsed before deleting its own private host copy.
(
 del "%BF_EXEC_STAGE%\dispatch.exe" >nul 2>&1
 del "%BF_EXEC_STAGE%\entry.cmd" >nul 2>&1
 rmdir "%BF_EXEC_STAGE%" >nul 2>&1
 exit /b %result%
)
:failed
if exist "%BF_EXEC_STAGE%\dispatch.exe" del "%BF_EXEC_STAGE%\dispatch.exe" >nul 2>&1
if exist "%BF_EXEC_STAGE%\entry.cmd" del "%BF_EXEC_STAGE%\entry.cmd" >nul 2>&1
rmdir "%BF_EXEC_STAGE%" >nul 2>&1
exit /b 1
:advanced
cd /d "%~dp0" || exit /b 1
"%~dp0better-favorites-dispatch-windows-386.exe" %*
set "result=%errorlevel%"
if "%~1"=="" pause
exit /b %result%
