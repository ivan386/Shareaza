@echo off
IF "%2" == "" GOTO missing_parameter
SET OLDDIR=%CD%
SET CONFIGNAME=%1
SET PLATFORMNAME=%2

IF %CONFIGNAME% == "Debug x64" SET PLATFORMNAME="x64"
IF %CONFIGNAME% == "Release x64" SET PLATFORMNAME="x64"

IF %CONFIGNAME% == "Debug Win32" SET CONFIGNAME="Debug"
IF %CONFIGNAME% == "Debug x64" SET CONFIGNAME="Debug"
IF %CONFIGNAME% == "Release Win32" SET CONFIGNAME="Release"
IF %CONFIGNAME% == "Release x64" SET CONFIGNAME="Release"

IF %CONFIGNAME% == "Debug" GOTO compile
IF %CONFIGNAME% == "Release" GOTO compile

echo ERROR: Wrong configuration selected
GOTO error

:compile
  echo Configuration = %CONFIGNAME%
  echo Platform = %PLATFORMNAME%

  if exist "%ProgramFiles%\Inno Setup 5\ISCC.exe" (
    cd "..\setup\scripts"
    "%ProgramFiles%\Inno Setup 5\ISCC.exe" main.iss /d%CONFIGNAME% /dPlatformName=%PLATFORMNAME% /o"%OLDDIR%\%CONFIGNAME% %PLATFORMNAME%\Installer\"
  ) else (
    echo ERROR: You must have Inno Setup 5 QuickStart Pack installed before you can create the installer.
    GOTO error
  )
  GOTO exit

:missing_parameter
  echo ERROR: Missing parameter
  pause > nul
  GOTO error

:error
  exit 1

:exit