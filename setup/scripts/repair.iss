#define version GetFileVersion("..\builds\Shareaza.exe")

[Setup]
AppName=Shareaza
AppVerName=Shareaza {#version}
AppPublisher=Shareaza Development Team
AppId=Shareaza
AppMutex=Shareaza
Compression=lzma/ultra
InternalCompressLevel=Ultra
SolidCompression=yes
OutputBaseFilename=repair
OutputDir=setup\builds
VersionInfoCompany=Shareaza Development Team
VersionInfoDescription=Shareaza Ultimate File Sharing
VersionInfoVersion={#version}
DefaultDirName={reg:HKLM\SOFTWARE\Shareaza,|{pf}\Shareaza}
DefaultGroupName=Shareaza
DisableDirPage=yes
DisableFinishedPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes
PrivilegesRequired=admin
ShowLanguageDialog=auto
Uninstallable=no
SetupIconFile=setup\misc\install.ico
WizardImageFile=setup\misc\sidebar.bmp
WizardSmallImageFile=setup\misc\corner.bmp

; Set the CVS root as source dir (up 2 levels)
SourceDir=..\..

[INI]
; Write important settings to safe location before erasing them from registry
Filename: "{param:SETTINGS|{tmp}\settings.ini}"; Section: "Locations"; key: "CompletePath"; String: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|}"; Flags: createkeyifdoesntexist
Filename: "{param:SETTINGS|{tmp}\settings.ini}"; Section: "Locations"; key: "IncompletePath"; String: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,IncompletePath|}"; Flags: createkeyifdoesntexist
Filename: "{param:SETTINGS|{tmp}\settings.ini}"; Section: "Locations"; key: "TorrentPath"; String: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,TorrentPath|}"; Flags: createkeyifdoesntexist
Filename: "{param:SETTINGS|{tmp}\settings.ini}"; Section: "Locations"; key: "CollectionPath"; String: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CollectionPath|}"; Flags: createkeyifdoesntexist
Filename: "{param:SETTINGS|{tmp}\settings.ini}"; Section: "Skins"; key: "ShareazaOS"; String: "{reg:HKCU\Software\Shareaza\Shareaza\Skins,ShareazaOS\ShareazaOS.xml|}"; Flags: createkeyifdoesntexist

[Registry]
; Delete all settings before starting reparation
Root: HKLM; Subkey: "SOFTWARE\Shareaza"; Flags: dontcreatekey deletekey
Root: HKCU; Subkey: "Software\Shareaza"; Flags: dontcreatekey deletekey

[InstallDelete]
; Delete all files exept uninstall dir
Type: filesandordirs; Name: "{app}\Skins"
Type: filesandordirs; Name: "{app}\Plugins"
Type: filesandordirs; Name: "{app}\Remote"
Type: filesandordirs; Name: "{app}\Data"
Type: filesandordirs; Name: "{app}\Schemas"
Type: files; Name: "{app}\unicows.dll"
Type: files; Name: "{app}\zlib.dll"
Type: files; Name: "{app}\Shareaza.exe"

[Files]
; Copy installer stored in uninstall dir to temp location to avoid "in-use" error
Source: "{app}\Uninstall\setup.exe"; DestDir: "{tmp}"; DestName: "temp.exe"; Flags: ignoreversion overwritereadonly sortfilesbyextension external

[Run]
; Run installer in silent mode and pass all settings.
Filename: "{tmp}\temp.exe"; WorkingDir: "{app}"; Flags: hidewizard; Parameters: "/SILENT /NOCANCEL /NORESTART /LANG={language} /DIR=""{app}"" /GROUP=""{groupname}"" /SETTINGS=""{param:SETTINGS|{tmp}\settings.ini}"""

[Messages]
WelcomeLabel1=Welcome to the [name] Repair Wizard
WelcomeLabel2=This will repair [name/ver] on your computer.%n%nIt is recommended that you close all other applications before continuing.
