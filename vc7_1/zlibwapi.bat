@echo off
IF "%4" == "" GOTO missing_parameter
SET OLDDIR=%CD%
SET ACTION=%1
SET TARGETDIR=%2
SET CONFIGNAME=%3
SET PLATFORMNAME=%4

IF %CONFIGNAME% == "Debugx64" SET PLATFORMNAME="x64"
IF %CONFIGNAME% == "Releasex64" SET PLATFORMNAME="x64"

SET FILENAME="zlibwapi.dll"
IF %PLATFORMNAME% == "x64" SET FILENAME="zlibwapi_x64.dll"

IF %ACTION% == "Copy generated" GOTO generated
IF %ACTION% == "Copy downloaded" GOTO downloaded

echo ERROR: Wrong action selected
GOTO error

:generated
  echo Copying generated %FILENAME%
  echo Configuration = %CONFIGNAME%
  echo Platform = %PLATFORMNAME%

  copy %TARGETDIR%%FILENAME% %TARGETDIR%..\  /b /y
  IF NOT EXIST %TARGETDIR%..\plugins\ md %TARGETDIR%..\plugins\
  copy %TARGETDIR%..\zlibwapi.dll %TARGETDIR%..\plugins\ /b /y

  GOTO exit

:downloaded
  echo Copying downloaded %FILENAME%
  echo Configuration = %CONFIGNAME%
  echo Platform = %PLATFORMNAME%

  copy ..\setup\builds\%FILENAME% %TARGETDIR%..\  /b /y
  IF %PLATFORMNAME% == "x64" ren %TARGETDIR%..\%FILENAME% zlibwapi.dll
  IF NOT EXIST %TARGETDIR%..\plugins\ md %TARGETDIR%..\plugins\
  copy %TARGETDIR%..\zlibwapi.dll %TARGETDIR%..\plugins\ /b /y

  GOTO exit

:missing_parameter
  echo ERROR: Missing parameter
  pause > nul
  GOTO error

:error
  exit 1

:exit