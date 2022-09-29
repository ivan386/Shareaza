@echo off
setlocal

set app="..\vc143\packages\7-Zip.StandaloneConsole.x64.22.1.0\tools\7za.exe"

%app% a -tgzip -mx=9 -- %2 %1
if %errorlevel% equ 9009 echo The 7-Zip utility is missing. Please go to http://www.7-zip.org/ and install 7-Zip.
exit /b %errorlevel%