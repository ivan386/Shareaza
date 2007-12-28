@echo off
set OLDDIR=%CD%
CD /D "%MSSdk%"
call setenv.cmd /X64 /RETAIL
set include=%include%;C:\Boost\include\boost-1_33_1
CD /D "%VS71COMNTOOLS%..\IDE\"
start devenv.exe "%OLDDIR%\Shareaza.sln" /useenv