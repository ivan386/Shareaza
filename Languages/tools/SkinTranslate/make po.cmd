cd ..\..\
for %%i in ( *.xml ) do tools\SkinTranslate\Release\SkinTranslate.exe .\default-en.xml .\%%i .\%%i.po
@pause