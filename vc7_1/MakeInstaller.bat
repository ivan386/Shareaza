set OLDDIR=%CD%
IF EXIST "..\setup\builds\Release" del "..\setup\builds\Release" /F
IF EXIST "..\setup\builds\Debug" del "..\setup\builds\Debug" /F
echo Type > "..\setup\builds\%1"
cd "..\setup\scripts"
"%ProgramFiles%\Inno Setup 5\ISCC.exe" main.iss /Q
