cd ..\..\
for %%i in ( *.xml ) do tools\SkinTranslate\Win32\Release\SkinTranslate.exe .\default-en.xml .\%%i .\%%i.po
@pause