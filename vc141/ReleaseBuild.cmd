@echo off
setlocal

if "%1"=="" echo Usage: %0 "[Win32|x64]" && goto errors

cd ..
for /F "tokens=3" %%a in ( 'findstr ProductVersion shareaza\Shareaza.rc' ) do set VERSION=%%~a

set PLATFORM=%1
set CONFIG=Release
set COMPILER=vc141
set BUILDDIR=%COMPILER%\%PLATFORM%\%CONFIG%
set BUILDPRP=%COMPILER%\ReleaseBuild.props
set BUILDTMP=%BUILDDIR%\ReleaseBuild.tmp.cmd
set BUILDRUN=%BUILDDIR%\ReleaseBuild.run.cmd
set BUILDLNK=..\Releases\%VERSION%\
set BUILDOUT=..\Releases\Shareaza-%VERSION%\

echo Builing Shareaza %VERSION% %PLATFORM% %CONFIG%...

set SubWCRev=%ProgramFiles%\TortoiseSVN\bin\SubWCRev.exe
if not exist "%SubWCRev%" set SubWCRev=%ProgramFiles(x86)%\TortoiseSVN\bin\SubWCRev.exe
if not exist "%SubWCRev%" set SubWCRev=%ProgramW6432%\TortoiseSVN\bin\SubWCRev.exe
if not exist "%SubWCRev%" echo The SubWCRev utility is missing. Please go to https://sourceforge.net/projects/tortoisesvn/ and install TortoiseSVN. && goto errors
echo Using TortoiseSVN at %SubWCRev%...

for /d %%i in ("%ProgramFiles(x86)%\Microsoft Visual Studio\2017\*") do	if exist "%%i\MSBuild\15.0\Bin\MSBuild.exe" set MSBUILD=%%i\MSBuild\15.0\Bin\MSBuild.exe
if "%MSBUILD%"=="" echo The MSBuild utility is missing. && goto errors
echo Using MSBuild at %MSBUILD%...

echo Cleaning...
call clean.cmd
md %BUILDDIR% 2>nul
del /q %BUILDTMP% 2>nul
del /q %BUILDRUN% 2>nul
del /q setup\builds\Preprocessed.iss 2>nul

echo Creating build script...
echo @echo off > %BUILDTMP%
echo setlocal >> %BUILDTMP%
echo rmdir %BUILDLNK% >> %BUILDTMP%
echo mklink /D /J %BUILDLNK% . >> %BUILDTMP%
echo pushd %BUILDLNK% >> %BUILDTMP%
echo set ISCCOPTIONS=/dRELEASE_BUILD=1 /q >> %BUILDTMP%
echo "%MSBUILD%" %COMPILER%\Shareaza.sln /nologo /v:m /t:Rebuild /p:ForceImportBeforeCppTargets="%CD%\%BUILDPRP%" /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /fl /flp:Summary;Verbosity=normal;LogFile=%BUILDDIR%\Shareaza_r$WCREV$_$WCNOW=%%Y-%%m-%%d$.log >> %BUILDTMP%
echo popd >> %BUILDTMP%
echo rmdir %BUILDLNK% >> %BUILDTMP%
echo md %BUILDOUT% >> %BUILDTMP%
echo move /y setup\builds\Shareaza_%VERSION%_%PLATFORM%.exe %BUILDOUT% >> %BUILDTMP%
echo if errorlevel 1 exit /b 1 >> %BUILDTMP%
echo move /y setup\builds\Shareaza_%VERSION%_%PLATFORM%_Symbols.7z %BUILDOUT% >> %BUILDTMP%
echo if errorlevel 1 exit /b 1 >> %BUILDTMP%
echo move /y setup\builds\Shareaza_%VERSION%_Source.7z %BUILDOUT% >> %BUILDTMP%
echo if errorlevel 1 exit /b 1 >> %BUILDTMP%
"%SubWCRev%" . %BUILDTMP% %BUILDRUN%
del /q %BUILDTMP% 2>nul

echo Building...
call %BUILDRUN%
if errorlevel 1 goto errors
del /q %BUILDRUN% 2>nul

echo Saving symbols locally...
md ..\Symbols\Shareaza_%VERSION%_%PLATFORM%_Symbols 2>nul
copy /b /y %BUILDDIR%\*.exe ..\Symbols\Shareaza_%VERSION%_%PLATFORM%_Symbols 2>nul
copy /b /y %BUILDDIR%\*.dll ..\Symbols\Shareaza_%VERSION%_%PLATFORM%_Symbols 2>nul
copy /b /y %BUILDDIR%\*.pdb ..\Symbols\Shareaza_%VERSION%_%PLATFORM%_Symbols 2>nul

echo Build succeeded!
exit /b 0

:errors
echo Build failed!
exit /b 1