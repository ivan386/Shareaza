cd ..
cd ..
cd ..
echo %CD%
"%ProgramFiles%\Microsoft SDKs\Windows\v6.0A\bin\tlbimp.exe" "..\..\shareaza\win32\%1\Shareaza.tlb" /out:Shareaza.Interop.dll /namespace:Shareaza /asmversion:1.1.0.0 /keyfile:Shareaza.snk /nologo /primary /sysarray /machine:x86 /transform:DispRet