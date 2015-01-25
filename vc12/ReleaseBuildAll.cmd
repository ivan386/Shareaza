@echo off
call ReleaseBuild.cmd Win32
call ReleaseBuild.cmd x64
exit /b 0