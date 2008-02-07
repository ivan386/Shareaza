@echo off
rem    This batch file saves Shareaza settings from the registry
rem    to files ShareazaSave1.reg and ShareazaSave2.reg

regedit /ea ShareazaSave1.reg HKEY_CURRENT_USER\Software\Shareaza
regedit /ea ShareazaSave2.reg HKEY_LOCAL_MACHINE\Software\Shareaza
