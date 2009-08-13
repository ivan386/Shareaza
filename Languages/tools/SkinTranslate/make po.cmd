@echo off
md ..\..\new_po
for %%i in ( ..\..\*.xml ) do Win32\Release\SkinTranslate.exe ..\..\default-en.xml %%i ..\..\new_po\#.po
pause