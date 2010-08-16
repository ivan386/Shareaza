rem %1 - $(PlatformName)
rem %2 - $(ConfigurationName)
copy /b /y "..\sqlite3\%1\%2\sqlite3.dll" "%1\%2\"
copy /b /y "..\zlib\%1\%2\zlibwapi.dll" "%1\%2\"
copy /b /y "..\bzlib\%1\%2\bzlib.dll" "%1\%2\"
copy /b /y "..\HashLib\%1\%2\HashLib.dll" "%1\%2\"
copy /b /y "..\RegExp\%1\%2\RegExp.dll" "%1\%2\"
copy /b /y "..\GeoIP\%1\%2\GeoIP.dll" "%1\%2\"
copy /b /y "..\GeoIP\GeoIP.dat" "%1\%2\"
copy /b /y "..\BugTrap\%1\%2\BugTrap.dll" "%1\%2\"
if "%1" == "x64"   copy /b /y "..\BugTrap\dbghelp-x64.dll" "%1\%2\dbghelp.dll"
if "%1" == "Win32" copy /b /y "..\BugTrap\dbghelp.dll"     "%1\%2\dbghelp.dll"
cscript.exe //E:jscript //nologo revision.js