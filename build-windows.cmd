@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-windows.ps1" %*
set "twinleap_exit=%errorlevel%"
if not "%twinleap_exit%"=="0" pause
exit /b %twinleap_exit%
