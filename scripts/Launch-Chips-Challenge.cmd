@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Launch-Chips-Challenge.ps1"
if errorlevel 1 pause
