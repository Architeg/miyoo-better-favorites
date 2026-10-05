#!/usr/bin/env python3
"""Generate a transparent literal PowerShell selector; no encoded/script-policy bypass."""
from pathlib import Path
import argparse
ROOT=Path(__file__).resolve().parents[1]
def rendered():
 lines=[s.strip() for s in (ROOT/'packaging/windows-select.ps1').read_text().splitlines() if s.strip() and not s.lstrip().startswith('#')]
 command=' '.join(lines)+' Invoke-BetterFavoritesSelection;'
 assert not any(c in command for c in ('"','%','`')), 'Keep CMD literal quoting unambiguous'
 assert len(command)<7000, 'Windows CMD command-length limit'
 template='''@echo off
setlocal DisableDelayedExpansion
rem Built-in PowerShell selects the existing native installer. No compiled bootstrap.
if /i "%~nx0"=="entry.cmd" goto staged
set "BF_ENTRY_MODE=direct"
set "BF_TOOL_ROOT=%~dp0."
set "BF_CARD_SOURCE="
if exist "%~dp0computer\\transport.json" (
 set "BF_ENTRY_MODE=card"
 set "BF_CARD_SOURCE=%~dp0."
 set "BF_TOOL_ROOT=%~dp0computer"
)
:reserve
set "BF_EXEC_STAGE=%TEMP%\\better-favorites-entry-%RANDOM%-%RANDOM%"
if exist "%BF_EXEC_STAGE%" goto reserve
mkdir "%BF_EXEC_STAGE%" || exit /b 2
copy /b "%~f0" "%BF_EXEC_STAGE%\\entry.cmd" >nul || goto failed
"%SystemRoot%\\System32\\fc.exe" /b "%~f0" "%BF_EXEC_STAGE%\\entry.cmd" >nul || goto failed
cd /d "%BF_EXEC_STAGE%" || goto failed
rem Transfer the batch context off-card before complete uninstall can remove it.
"%BF_EXEC_STAGE%\\entry.cmd" %*
exit /b 2
:staged
set "BF_POWERSHELL=%SystemRoot%\\System32\\WindowsPowerShell\\v1.0\\powershell.exe"
if exist "%SystemRoot%\\Sysnative\\WindowsPowerShell\\v1.0\\powershell.exe" set "BF_POWERSHELL=%SystemRoot%\\Sysnative\\WindowsPowerShell\\v1.0\\powershell.exe"
"%BF_POWERSHELL%" -NoLogo -NoProfile -Command "@SELECTOR@"
set "result=%errorlevel%"
if not "%result%"=="0" goto finish
set /p BF_NATIVE_NAME=<"%BF_EXEC_STAGE%\\native-name.txt"
if "%BF_ENTRY_MODE%"=="card" goto card
"%BF_TOOL_ROOT%\\%BF_NATIVE_NAME%" %*
set "result=%errorlevel%"
goto finish
:card
"%BF_EXEC_STAGE%\\%BF_NATIVE_NAME%" --card-launcher "%BF_CARD_SOURCE%" %*
set "result=%errorlevel%"
:finish
if "%~1"=="" pause
cd /d "%TEMP%" || exit /b 2
rem Only exact files created in this invocation's private stage are candidates.
for %%F in (better-favorites-installer-windows7-386.exe better-favorites-installer-windows7-amd64.exe better-favorites-installer-windows-386.exe better-favorites-installer-windows-amd64.exe better-favorites-installer-windows-arm64.exe native-name.txt) do if exist "%BF_EXEC_STAGE%\\%%F" del "%BF_EXEC_STAGE%\\%%F" >nul 2>&1
(
 del "%BF_EXEC_STAGE%\\entry.cmd" >nul 2>&1
 rmdir "%BF_EXEC_STAGE%" >nul 2>&1
 exit /b %result%
)
:failed
set "result=2"
goto finish
'''
 return template.replace('@SELECTOR@',command).replace('\n','\r\n').encode('ascii')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');a=p.parse_args();path=ROOT/'packaging/Install-Windows.cmd';data=rendered()
 if a.check:assert path.read_bytes()==data, 'Regenerate Windows entry'
 else:path.write_bytes(data)
