@echo off
rem    This batch file restores Shareaza settings from previously saved
rem    file ShareazaSave.reg to the registry

if not exist ShareazaSave.reg exit

echo REGEDIT4> "%TEMP%\ShareazaDelOld.reg"
echo [-HKEY_CURRENT_USER\Software\Shareaza]>> "%TEMP%\ShareazaDelOld.reg"

start/wait regedit -s "%TEMP%\ShareazaDelOld.reg"
del "%TEMP%\ShareazaDelOld.reg" > nul

:import
start/wait regedit -s ShareazaSave.reg