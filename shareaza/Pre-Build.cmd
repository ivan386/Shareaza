rem %1 - $(PlatformName)
rem %2 - $(ConfigurationName)

if exist "..\sqlite3\%1\%2\sqlite3.dll" goto sqlite1
copy /b /y "..\sqlite3\%1\Release\sqlite3.dll" "%1\%2\"
goto sqlite2
:sqlite1
copy /b /y "..\sqlite3\%1\%2\sqlite3.dll" "%1\%2\"
:sqlite2

if exist "..\zlib\%1\%2\zlibwapi.dll" goto zlib1
copy /b /y "..\zlib\%1\Release\zlibwapi.dll" "%1\%2\"
goto zlib2
:zlib1
copy /b /y "..\zlib\%1\%2\zlibwapi.dll" "%1\%2\"
:zlib2

copy /b /y "..\HashLib\%1\%2\HashLib.dll" "%1\%2\"

copy /b /y "..\RegExp\%1\%2\RegExp.dll" "%1\%2\"

copy /b /y "..\GeoIP\%1\%2\GeoIP.dll" "%1\%2\"
copy /b /y "..\GeoIP\GeoIP.dat" "%1\%2\"

if exist "..\BugTrap\%1\%2\BugTrap.dll" goto BugTrap1
copy /b /y "..\BugTrap\%1\Release\BugTrap.dll" "%1\%2\"
goto BugTrap2
:BugTrap1
copy /b /y "..\BugTrap\%1\%2\BugTrap.dll" "%1\%2\"
:BugTrap2
if "%1" == "x64"   copy /b /y "..\BugTrap\dbghelp-x64.dll" "%1\%2\dbghelp.dll"
if "%1" == "Win32" copy /b /y "..\BugTrap\dbghelp.dll"     "%1\%2\dbghelp.dll"

cscript.exe //E:jscript //nologo revision.js