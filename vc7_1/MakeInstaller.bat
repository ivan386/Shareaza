set OLDDIR=%CD%
cd "..\setup\scripts"
"%ProgramFiles%\Inno Setup 5\ISCC.exe" main.iss /d%1 /dPlatformName=%2 /o"%OLDDIR%\Installer\"
