@echo off
setlocal
cd ..
set BUILDDIR=vc12\Win32\Release\

set SubWCRev="%ProgramFiles(x86)%\TortoiseSVN\bin\SubWCRev.exe"
if exist %SubWCRev% goto ok
set SubWCRev="%ProgramFiles%\TortoiseSVN\bin\SubWCRev.exe"
if exist %SubWCRev% goto ok
echo The SubWCRev utility is missing. Please go to https://sourceforge.net/projects/tortoisesvn/ and install TortoiseSVN.
goto errors
:ok

echo Cleaning...
call clean.cmd
md %BUILDDIR% 2>nul
del /q %BUILDDIR%LanMode.tmp 2>nul
del /q %BUILDDIR%LanMode.run.cmd 2>nul
del /q setup\builds\Preprocessed.iss 2>nul
del /q Shareaza_*.7z 2>nul

echo Creating build script...
echo setlocal > %BUILDDIR%LanMode.tmp
echo md ..\LAN_MODE\ >> %BUILDDIR%LanMode.tmp
echo md ..\LAN_MODE\r$WCREV$\ >> %BUILDDIR%LanMode.tmp
echo rmdir ..\r$WCREV$ >> %BUILDDIR%LanMode.tmp
echo mklink /D /J ..\r$WCREV$ . >> %BUILDDIR%LanMode.tmp
echo pushd ..\r$WCREV$ >> %BUILDDIR%LanMode.tmp
echo call "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86 >> %BUILDDIR%LanMode.tmp
echo set ISCCOPTIONS=/dRELEASE_BUILD=1 /dLAN_MODE /dREVISION=r$WCREV$ /q >> %BUILDDIR%LanMode.tmp
echo msbuild vc12\Shareaza.sln /nologo /v:m /t:Rebuild /p:ForceImportBeforeCppTargets="%CD%\vc12\LanMode.props" /p:Configuration=Release /p:Platform=Win32 /fl /flp:Summary;Verbosity=normal;LogFile=%BUILDDIR%Shareaza_r$WCREV$_$WCNOW=%%Y-%%m-%%d$.log >> %BUILDDIR%LanMode.tmp
echo popd >> %BUILDDIR%LanMode.tmp
echo rmdir ..\r$WCREV$ >> %BUILDDIR%LanMode.tmp
echo del /q ..\LAN_MODE\r$WCREV$\*.exe >> %BUILDDIR%LanMode.tmp
echo move /y setup\builds\*.exe ..\LAN_MODE\r$WCREV$ >> %BUILDDIR%LanMode.tmp
echo for %%%%i in (..\LAN_MODE\r$WCREV$\*.exe) do ren %%%%i %%%%~ni_LAN.exe >> %BUILDDIR%LanMode.tmp
echo if errorlevel 1 exit /b 1 >> %BUILDDIR%LanMode.tmp
echo call pack_pdb.cmd >> %BUILDDIR%LanMode.tmp
echo if errorlevel 1 exit /b 1 >> %BUILDDIR%LanMode.tmp
echo move /y Shareaza_{version}_Symbols.7z ..\LAN_MODE\r$WCREV$\Shareaza_Win32_LAN_r$WCREV$_$WCNOW=%%Y-%%m-%%d$_Symbols.7z >> %BUILDDIR%LanMode.tmp
echo if errorlevel 1 exit /b 1 >> %BUILDDIR%LanMode.tmp
%SubWCRev% . %BUILDDIR%LanMode.tmp %BUILDDIR%LanMode.run.cmd
del /q %BUILDDIR%LanMode.tmp 2>nul

echo Building...
call %BUILDDIR%LanMode.run.cmd
if errorlevel 1 goto errors
del /q %BUILDDIR%LanMode.run.cmd 2>nul
exit /b 0

:errors
echo Build failed!
pause