@echo off
rem    This batch file restores Shareaza settings from previously saved
rem    files ShareazaSave1.reg and ShareazaSave2.reg to the registry

if not exist ShareazaSave?.reg exit

echo REGEDIT4> "%TEMP%\ShareazaDelOld.reg"
echo [-HKEY_CURRENT_USER\Software\Shareaza]>> "%TEMP%\ShareazaDelOld.reg"
echo [-HKEY_LOCAL_MACHINE\Software\Shareaza]>> "%TEMP%\ShareazaDelOld.reg"

start/wait regedit -s "%TEMP%\ShareazaDelOld.reg"
del "%TEMP%\ShareazaDelOld.reg" > nul

:import
start/wait regedit -s ShareazaSave1.reg ShareazaSave2.reg
