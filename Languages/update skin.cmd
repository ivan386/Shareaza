@echo off
setlocal
set src="%VS110COMNTOOLS%\..\..\VC\atlmfc\include\afxres.h"
if exist %src% goto ok
set src="%VS120COMNTOOLS%\..\..\VC\atlmfc\include\afxres.h"
if exist %src% goto ok
echo File afxres.h missed!
exit /b 1
:ok
SkinUpdate.exe ..\shareaza\resource.h  ..\shareaza\shareaza.rc default-en-updated.xml > SkinUpdate.log
start notepad SkinUpdate.log