set OLDDIR=%CD%
xcopy ..\GeoIP\geoip.dll %1 /y /q
xcopy ..\GeoIP\data\GeoIP.dat %1 /y /q
"%OLDDIR%\%1\Shareaza\skin.exe" /install
