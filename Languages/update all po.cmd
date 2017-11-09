setlocal
set msgmerge="%ProgramFiles(x86)%\Poedit\GettextTools\bin\msgmerge.exe"
if exist %msgmerge% goto msgmerge
set msgmerge="%ProgramFiles%\Poedit\GettextTools\bin\msgmerge.exe"
if exist %msgmerge% goto msgmerge
echo The msgmerge utility is missing. Please go to https://www.poedit.net/ and install PoEdit.
exit /b 1
:msgmerge
for %%i in (*.po) do %msgmerge% --no-escape --quiet --no-wrap --update --backup=off --no-fuzzy-matching "%%i" default-en.pot