set OLDDIR=%CD%
if exist "%ProgramFiles%\Inno Setup 5\ISCC.exe" (
	cd "..\setup"
	"%ProgramFiles%\Inno Setup 5\ISCC.exe" scripts\main.iss /dConfigurationName=%1 /dPlatformName=%2 /o"%OLDDIR%\Installer\"
) else (
	echo You must have Inno Setup 5 installed before you can create the installer.
)
