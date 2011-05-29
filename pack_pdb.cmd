set dst="Shareaza_{version}_Symbols.7z"

set zip="%ProgramFiles(x86)%\7-Zip\7z.exe"
if exist %zip% goto zip
set zip="%ProgramFiles%\7-Zip\7z.exe"
if exist %zip% goto zip
set zip=".\7z.exe"
:zip

set src=.\vc10\Win32\Release
if exist %src%\Shareaza.pdb goto work
set src=.\vc10\x64\Release
if exist %src%\Shareaza.pdb goto work
set src=.\vc10\Win32\Debug
if exist %src%\Shareaza.pdb goto work
set src=.\vc10\x64\Debug
if exist %src%\Shareaza.pdb goto work
set src=.\vc9\Win32\Release
if exist %src%\Shareaza.pdb goto work
set src=.\vc9\x64\Release
if exist %src%\Shareaza.pdb goto work
set src=.\vc9\Win32\Debug
if exist %src%\Shareaza.pdb goto work
set src=.\vc9\x64\Debug
if exist %src%\Shareaza.pdb goto work
:work

del %dst%
%zip% a -y -mx=9 %dst% %src%\Shareaza.pdb %src%\BugTrap.pdb