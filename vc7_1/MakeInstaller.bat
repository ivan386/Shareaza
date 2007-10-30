set OLDDIR=%CD%
xcopy "%1\shareaza.exe" ..\setup\builds /y /q
cd ..\setup\scripts
"%ProgramFiles%\Inno Setup 5\ISCC.exe" main.iss /Q
