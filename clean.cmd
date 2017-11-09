@echo off
del /q "Languages\*.log" 2>nul:
del /q "setup\builds\*.exe" 2>nul:
del /q "setup\builds\*.7z" 2>nul:
del /q "setup\builds\*.iss" 2>nul:
del /q "setup\builds\*.txt" 2>nul:
del /q "shareaza\ShareazaOM.h" 2>nul:
del /q "shareaza\revision.h" 2>nul:
del /q "shareaza\Res\*.gz" 2>nul:
for /r %%i in (*_i.c) do del /q "%%i" && echo Cleaning %%i...
for /r %%i in (*.aps) do del /q "%%i" && echo Cleaning %%i...
for /r %%i in (*.ncb) do del /q "%%i" && echo Cleaning %%i...
for /r %%i in (*.sdf) do del /q "%%i" && echo Cleaning %%i...
for /r %%i in (*.vc.db) do del /q "%%i" && echo Cleaning %%i...
call :clean Win32
call :clean x64
call :clean_vc Win32
call :clean_vc x64
call :clean_vc Debug
call :clean_vc Release
for /d /r %%i in (*.*) do if exist "%%i\ipch\" rd /s /q "%%i\ipch\"  && echo Cleaning %%i\ipch\...
exit /b 0
:clean
for /d /r %%i in (*.*) do (
	if exist "%%i\%1\Debug\" rd /s /q "%%i\%1\" && echo Cleaning %%i\%1\...
	if exist "%%i\%1\Release\" rd /s /q "%%i\%1\" && echo Cleaning %%i\%1\...
)
exit /b 0
:clean_vc
for /d /r %%i in (vc*) do if exist "%%i\%1\" rd /s /q "%%i\%1\" && echo Cleaning %%i\%1\...
exit /b 0