@echo off
md ..\..\new_xml
for %%i in ( ..\..\new_po\*.po ) do Win32\Release\SkinTranslate.exe ..\..\default-en.xml %%i ..\..\new_xml\#.xml
@pause