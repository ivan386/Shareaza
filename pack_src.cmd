set dst="Shareaza_{version}_Source.7z"

set zip="%ProgramFiles(x86)%\7-Zip\7z.exe"
if exist %zip% goto zip
set zip="%ProgramFiles%\7-Zip\7z.exe"
if exist %zip% goto zip
set zip=".\7z.exe"
:zip

del %dst%
%zip% a -y -mx=9 -r -x!.svn -x!setup\builds\*.exe -x!setup\builds\*.txt -x!setup\builds\*.iss -x!Win32 -x!x64 -x!ipch -x!*.7z -x!*.sdf -x!*.suo -x!*.ncb -x!*.user -x!*.opensdf %dst% .\