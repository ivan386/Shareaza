@echo off
setlocal
cd ..
set BUILDDIR=vc10\Win32\Debug\

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
del /q %BUILDDIR%DailyBuild.tmp 2>nul
del /q %BUILDDIR%DailyBuild.run.cmd 2>nul
del /q setup\builds\Preprocessed.iss 2>nul
del /q Shareaza_*.7z 2>nul

echo Creating build script...
echo setlocal > %BUILDDIR%DailyBuild.tmp
echo md ..\Snapshots\ >> %BUILDDIR%DailyBuild.tmp
echo md ..\Snapshots\r$WCREV$\ >> %BUILDDIR%DailyBuild.tmp
echo rmdir ..\r$WCREV$ >> %BUILDDIR%DailyBuild.tmp
echo mklink /D /J ..\r$WCREV$ . >> %BUILDDIR%DailyBuild.tmp
echo pushd ..\r$WCREV$ >> %BUILDDIR%DailyBuild.tmp
echo call "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86 >> %BUILDDIR%DailyBuild.tmp
echo set ISCCOPTIONS=/dRELEASE_BUILD=0 /dREVISION=r$WCREV$ /q >> %BUILDDIR%DailyBuild.tmp
echo msbuild vc10\Shareaza.sln /nologo /v:m /t:Rebuild /p:ForceImportBeforeCppTargets="%CD%\vc10\DailyBuild.props" /p:Configuration=Debug /p:Platform=Win32 /fl /flp:Summary;Verbosity=normal;LogFile=%BUILDDIR%Shareaza_r$WCREV$_$WCNOW=%%Y-%%m-%%d$.log >> %BUILDDIR%DailyBuild.tmp
echo popd >> %BUILDDIR%DailyBuild.tmp
echo rmdir ..\r$WCREV$ >> %BUILDDIR%DailyBuild.tmp
echo move /y setup\builds\*.exe ..\Snapshots\r$WCREV$\ >> %BUILDDIR%DailyBuild.tmp
echo if errorlevel 1 exit /b 1 >> %BUILDDIR%DailyBuild.tmp
echo call pack_pdb.cmd >> %BUILDDIR%DailyBuild.tmp
echo if errorlevel 1 exit /b 1 >> %BUILDDIR%DailyBuild.tmp
echo call pack_src.cmd >> %BUILDDIR%DailyBuild.tmp
echo if errorlevel 1 exit /b 1 >> %BUILDDIR%DailyBuild.tmp
echo move /y Shareaza_{version}_Symbols.7z ..\Snapshots\r$WCREV$\Shareaza_Win32_Debug_r$WCREV$_$WCNOW=%%Y-%%m-%%d$_Symbols.7z >> %BUILDDIR%DailyBuild.tmp
echo if errorlevel 1 exit /b 1 >> %BUILDDIR%DailyBuild.tmp
echo move /y Shareaza_{version}_Source.7z ..\Snapshots\r$WCREV$\Shareaza_r$WCREV$_Source.7z >> %BUILDDIR%DailyBuild.tmp
echo if errorlevel 1 exit /b 1 >> %BUILDDIR%DailyBuild.tmp
%SubWCRev% . %BUILDDIR%DailyBuild.tmp %BUILDDIR%DailyBuild.run.cmd
del /q %BUILDDIR%DailyBuild.tmp 2>nul

echo Building...
call %BUILDDIR%DailyBuild.run.cmd
if errorlevel 1 goto errors
del /q %BUILDDIR%DailyBuild.run.cmd 2>nul
exit /b 0

:errors
echo Build failed!
pause