Edit GPS data in Excel file.
Import data from Excel spreadsheet into MS Access database, name the table "WorldGPS"
and export to WorldGPS.xml using transform.xsl to get the file which can be converted
to WorldGPS.dat file.

To make WorldGPS.dat place WorldGPS.xml file in Data folder, create the following 
DWORD value in the registry:
[HKEY_CURRENT_USER\Software\Shareaza\Shareaza]
"ImportWorldGPS"=dword:00000001

Then start Shareaza, open Profile, select Identity->Profile page and a new .dat file
will be created.

Note: MS Access 2003 is required.
