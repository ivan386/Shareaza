; WARNING: Compile repair.iss first!

#define version GetFileVersion("..\builds\Shareaza.exe")

[Setup]
AppComments=Shareaza Ultimate File Sharing
AppId=Shareaza
;AppMutex=Shareaza,Global\Shareaza
AppName=Shareaza
AppPublisher=Shareaza Development Team
AppReadmeFile={app}\Uninstall\readme.txt
AppVersion={#version}
AppVerName=Shareaza {#version}
VersionInfoVersion={#version}
DefaultDirName={reg:HKLM\SOFTWARE\Shareaza,|{pf}\Shareaza}
DirExistsWarning=no
DefaultGroupName=Shareaza
DisableReadyPage=yes
OutputDir=setup\builds
OutputBaseFilename=Shareaza {#version}
SolidCompression=yes
Compression=lzma/ultra
InternalCompressLevel=Ultra
VersionInfoCompany=Shareaza Development Team
VersionInfoDescription=Shareaza Ultimate File Sharing
PrivilegesRequired=admin
ShowLanguageDialog=auto
LanguageDetectionMethod=locale
UninstallDisplayIcon={app}\Uninstall\uninstall.ico
UninstallDisplayName={cm:NameAndVersion,Shareaza,{#version}}
UninstallFilesDir={app}\Uninstall
SetupIconFile=setup\misc\install.ico
ShowComponentSizes=no
WizardImageFile=setup\misc\sidebar.bmp
WizardSmallImageFile=setup\misc\corner.bmp
AppModifyPath="{app}\Uninstall\repair.exe"

; Set the CVS root as source dir (up 2 levels)
SourceDir=..\..

; links to website for software panel
AppPublisherURL=http://www.shareaza.com/?id=home
AppSupportURL=http://www.shareaza.com/?id=support
AppUpdatesURL=http://www.shareaza.com/?id=download

[Components]
; Ask user wich components to install
Name: "plugins"; Description: "{cm:components_plugins}"; Types: full compact; Flags: disablenouninstallwarning
Name: "skins"; Description: "{cm:components_skins}"; Types: full; Flags: disablenouninstallwarning
Name: "language"; Description: "{cm:components_languages}"; Types: full; Flags: disablenouninstallwarning

[Tasks]
; Ask user to setup Shareaza for all users or just current user
; Note: Tasks disabled by functions are not unchecked, they DISSAPPEAR
Name: allusers; Description: "{cm:tasks_allusers}"; GroupDescription: "{cm:tasks_selectusers,Shareaza}"; Flags: exclusive; Check: not ShareazaInstalled; MinVersion: 0,4.0
Name: currentuser; Description: "{cm:tasks_currentuser,{username}}"; GroupDescription: "{cm:tasks_selectusers,Shareaza}"; Flags: exclusive; Check: not ShareazaInstalled; MinVersion: 0,4.0

[Files]
; Need zlib.dll in {sys} or regserver will crash
; Place this entry before other entries using regserver
Source: "setup\builds\zlib.dll"; DestDir: "{sys}"; Flags: regserver noregerror overwritereadonly replacesameversion restartreplace sharedfile uninsneveruninstall sortfilesbyextension

; Install unicows.dll on Win 9X
Source: "setup\builds\unicows.dll"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension regserver noregerror; MinVersion: 4.0,0
Source: "setup\builds\unicows.dll"; DestDir: "{sys}"; Flags: regserver noregerror overwritereadonly replacesameversion restartreplace sharedfile uninsneveruninstall sortfilesbyextension; MinVersion: 4.0,0

; Main files
Source: "setup\builds\zlib.dll"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "setup\builds\zlib.dll"; DestDir: "{app}\Skins"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "setup\builds\Shareaza.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "setup\builds\skin.exe"; DestDir: "{app}\Skins"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Data\DefaultAvatar.png"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension;
Source: "Data\Emoticons.bmp"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Data\*.xml"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Data\WorldGPS.dat"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Schemas\*.ico"; DestDir: "{app}\Schemas"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Schemas\*.xml"; DestDir: "{app}\Schemas"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Schemas\*.xsd"; DestDir: "{app}\Schemas"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

; Copy repair installer
Source: "setup\builds\repair.exe"; DestDir: "{app}\Uninstall"; Flags: uninsremovereadonly sortfilesbyextension onlyifdoesntexist

; Plugins
; Don't register RazaWebHook.dll since it will setup Shareaza as download manager
; Always install ImageServices.dll to enable Shareaza splash screen
Source: "setup\plugins\*.dll"; DestDir: "{app}\Plugins"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension regserver noregerror; Components: plugins; Excludes: "RazaWebHook.dll,ImageServices.dll"
Source: "setup\plugins\RazaWebHook.dll"; DestDir: "{app}\Plugins"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: plugins
Source: "setup\plugins\ImageServices.dll"; DestDir: "{app}\Plugins"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension regserver noregerror

; Uninstall icon for software panel
Source: "setup\misc\uninstall.ico"; DestDir: "{app}\Uninstall"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

; Skins
Source: "Skins\BlueStreak\*.xml"; DestDir: "{app}\Skins\BlueStreak"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: skins
Source: "Skins\BlueStreak\*.bmp"; DestDir: "{app}\Skins\BlueStreak"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: skins
Source: "Skins\CleanBlue\*.xml"; DestDir: "{app}\Skins\CleanBlue"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: skins
Source: "Skins\CleanBlue\*.bmp"; DestDir: "{app}\Skins\CleanBlue"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: skins
Source: "Skins\Corona\*.xml"; DestDir: "{app}\Skins\Corona"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: skins
Source: "Skins\Corona\*.bmp"; DestDir: "{app}\Skins\Corona"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: skins
Source: "Skins\Shareaza2\*.xml"; DestDir: "{app}\Skins\Shareaza2"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: skins
Source: "Skins\Shareaza2\*.bmp"; DestDir: "{app}\Skins\Shareaza2"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: skins
Source: "Skins\ShareazaOS\*.xml"; DestDir: "{app}\Skins\ShareazaOS"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: skins
Source: "Skins\ShareazaOS\*.bmp"; DestDir: "{app}\Skins\ShareazaOS"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: skins

; Languages: English gets installed by default
Source: "Languages\default-en.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Languages\*.bmp"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Components: "language"
Source: "Languages\*.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Components: "language"
Source: "Languages\*.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: "language"

; Old versions of Shareaza stored xml and dat files under {app}
; These need to be copied to {app}\Data
Source: "{app}\*.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{app}\*.xml"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist

; Copy installer into download and uninstall dir
Source: "{srcexe}"; DestDir: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{userappdata}\Shareaza\Downloads}}"; DestName: "Shareaza {#version}.exe"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension external; Tasks: currentuser
Source: "{srcexe}"; DestDir: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{commonappdata}\Shareaza\Downloads}}"; DestName: "Shareaza {#version}.exe"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension external; Tasks: allusers
Source: "{srcexe}"; DestDir: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{app}\Downloads}}"; DestName: "Shareaza {#version}.exe"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension external; MinVersion: 4.0,0
Source: "{srcexe}"; DestDir: "{app}\Uninstall"; DestName: "setup.exe"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension external
Source: "{srcexe}"; DestDir: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{userappdata}\Shareaza\Downloads}}"; DestName: "Shareaza {#version}.exe"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension external; Check: ShareazaInstalled

[Icons]
; Shareaza icons
Name: "{userprograms}\{groupname}\Shareaza"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "Shareaza Ultimate File Sharing"; Tasks: currentuser
Name: "{commonprograms}\{groupname}\Shareaza"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "Shareaza Ultimate File Sharing"; Tasks: allusers
Name: "{userprograms}\{groupname}\Shareaza"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "Shareaza Ultimate File Sharing"; MinVersion: 4.0,0
Name: "{userdesktop}\Shareaza"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "Shareaza Ultimate File Sharing"; Tasks: currentuser
Name: "{commondesktop}\Shareaza"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "Shareaza Ultimate File Sharing"; Tasks: allusers
Name: "{userdesktop}\Shareaza"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "Shareaza Ultimate File Sharing"; MinVersion: 4.0,0

; License and uninstall icon in user language
Name: "{userprograms}\{groupname}\{cm:icons_license}"; Filename: "{app}\Uninstall\license.rtf"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:icons_license}"; Tasks: currentuser
Name: "{userprograms}\{groupname}\{cm:icons_uninstall}"; Filename: "{uninstallexe}"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:UninstallProgram,Shareaza}"; Tasks: currentuser; IconFilename: "{app}\Uninstall\uninstall.ico"
Name: "{commonprograms}\{groupname}\{cm:icons_license}"; Filename: "{app}\Uninstall\license.rtf"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:icons_license}"; Tasks: allusers
Name: "{commonprograms}\{groupname}\{cm:icons_uninstall}"; Filename: "{uninstallexe}"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:UninstallProgram,Shareaza}"; Tasks: allusers; IconFilename: "{app}\Uninstall\uninstall.ico"
Name: "{userprograms}\{groupname}\{cm:icons_license}"; Filename: "{app}\Uninstall\license.rtf"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:icons_license}"; MinVersion: 4.0,0
Name: "{userprograms}\{groupname}\{cm:icons_uninstall}"; Filename: "{uninstallexe}"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:UninstallProgram,Shareaza}"; MinVersion: 4.0,0; IconFilename: "{app}\Uninstall\uninstall.ico"

; Set icons if ShareazaInstalled=1 and InnoSetupUsed=0
Name: "{userprograms}\{groupname}\Shareaza"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "Shareaza Ultimate File Sharing"; Check: ShareazaInstalled and not InnoSetupUsed
Name: "{userdesktop}\Shareaza"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "Shareaza Ultimate File Sharing"; Check: ShareazaInstalled and not InnoSetupUsed
Name: "{userprograms}\{groupname}\{cm:icons_license}"; Filename: "{app}\Uninstall\license.rtf"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:icons_license}"; Check: ShareazaInstalled and not InnoSetupUsed
Name: "{userprograms}\{groupname}\{cm:icons_uninstall}"; Filename: "{uninstallexe}"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:UninstallProgram,Shareaza}"; Check: ShareazaInstalled and not InnoSetupUsed; IconFilename: "{app}\Uninstall\uninstall.ico"

[Messages]
; Overwrite standard ISL entries
; DO NOT use for localized messages
BeveledLabel=Shareaza Development Team

[Run]
; Run the skin installer at end of installation
Filename: "{app}\Skins\skin.exe"; Parameters: "/installsilent"; WorkingDir: "{app}\Skins"; StatusMsg: "{cm:run_skinexe}"
; Run Shareaza at end of installation
Filename: "{app}\Shareaza.exe"; Description: "{cm:LaunchProgram,Shareaza}"; WorkingDir: "{app}"; Flags: postinstall skipifsilent nowait

[UninstallRun]
; Run the skin installer at start of uninstallation and make sure it only runs once
Filename: "{app}\Skins\skin.exe"; Parameters: "/uninstallsilent"; WorkingDir: "{app}\Skins"; StatusMsg: "{cm:run_skinexe}"; RunOnceId: "uninstallskinexe"

[Registry]
; Write installation path to registry
Root: HKLM; Subkey: "SOFTWARE\Shareaza"; ValueType: string; ValueName: ; ValueData: "{app}"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\Shareaza.exe"; ValueType: string; ValueName: ; ValueData: "{app}\Shareaza.exe"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\Shareaza.exe"; ValueType: string; ValueName: "Path"; ValueData: "{app}"; Flags: uninsdeletekey

; Install chat notify sound
Root: HKCU; Subkey: "AppEvents\EventLabels\RAZA_IncomingChat"; ValueType: string; ValueName: ; ValueData: "{cm:reg_incomingchat}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Shareaza"; ValueType: string; ValueName: ; ValueData: "{cm:reg_apptitle}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Shareaza\RAZA_IncomingChat\.current"; ValueType: string; ValueName: ; ValueData: "%SystemRoot%\media\notify.wav"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Shareaza\RAZA_IncomingChat\.default"; ValueType: string; ValueName: ; ValueData: "%SystemRoot%\media\notify.wav"; Flags: uninsdeletekey

; Set directory locations
; Note: These do not have to be set if ShareazaInstalled=1
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CompletePath"; ValueData: "{userappdata}\Shareaza\Downloads"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: currentuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CompletePath"; ValueData: "{commonappdata}\Shareaza\Downloads"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: allusers
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CompletePath"; ValueData: "{app}\Downloads"; Flags: uninsdeletekey createvalueifdoesntexist; MinVersion: 4.0,0
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "IncompletePath"; ValueData: "{userappdata}\Shareaza\Incomplete"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: currentuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "IncompletePath"; ValueData: "{commonappdata}\Shareaza\Incomplete"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: allusers
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "IncompletePath"; ValueData: "{app}\Incomplete"; Flags: uninsdeletekey createvalueifdoesntexist; MinVersion: 4.0,0
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "TorrentPath"; ValueData: "{userappdata}\Shareaza\Torrents"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: currentuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "TorrentPath"; ValueData: "{commonappdata}\Shareaza\Torrents"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: allusers
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "TorrentPath"; ValueData: "{app}\Torrents"; Flags: uninsdeletekey createvalueifdoesntexist; MinVersion: 4.0,0
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CollectionPath"; ValueData: "{userappdata}\Shareaza\Collections"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: currentuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CollectionPath"; ValueData: "{commonappdata}\Shareaza\Collections"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: allusers
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CollectionPath"; ValueData: "{app}\Collections"; Flags: uninsdeletekey createvalueifdoesntexist; MinVersion: 4.0,0

; Delete keys at uninstall
Root: HKLM; Subkey: "SOFTWARE\Shareaza"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Shareaza"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "Shareaza"; Flags: dontcreatekey uninsdeletevalue

; Delete NSIS entry on software panel
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Shareaza"; Flags: dontcreatekey deletekey

; Create TorrentAid default dir locations
Root: HKCU; Subkey: "Software\TorrentAid\TorrentWizard\Folders"; ValueType: string; ValueName: "001.Path"; ValueData: "{userappdata}\Shareaza\Torrents"; Flags: createvalueifdoesntexist; Tasks: currentuser
Root: HKCU; Subkey: "Software\TorrentAid\TorrentWizard\Folders"; ValueType: string; ValueName: "Last"; ValueData: "{userappdata}\Shareaza\Torrents"; Flags: createvalueifdoesntexist; Tasks: currentuser
Root: HKCU; Subkey: "Software\TorrentAid\TorrentWizard\Folders"; ValueType: string; ValueName: "001.Path"; ValueData: "{commonappdata}\Shareaza\Torrents"; Flags: createvalueifdoesntexist; Tasks: allusers
Root: HKCU; Subkey: "Software\TorrentAid\TorrentWizard\Folders"; ValueType: string; ValueName: "Last"; ValueData: "{commonappdata}\Shareaza\Torrents"; Flags: createvalueifdoesntexist; Tasks: allusers
Root: HKCU; Subkey: "Software\TorrentAid\TorrentWizard\Folders"; ValueType: string; ValueName: "001.Path"; ValueData: "{app}\Torrents"; Flags: createvalueifdoesntexist; MinVersion: 4.0,0
Root: HKCU; Subkey: "Software\TorrentAid\TorrentWizard\Folders"; ValueType: string; ValueName: "Last"; ValueData: "{app}\Torrents"; Flags: createvalueifdoesntexist; MinVersion: 4.0,0

[Dirs]
; Make incomplete, torrent and collection dir
; Note: download dir will be created when installer is copied
; Note: These do not have to be set if ShareazaInstalled=1
Name: "{ini:{param:SETTINGS|},Locations,IncompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,IncompletePath|{userappdata}\Shareaza\Incomplete}}"; Flags: uninsalwaysuninstall; Tasks: currentuser
Name: "{ini:{param:SETTINGS|},Locations,IncompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,IncompletePath|{commonappdata}\Shareaza\Incomplete}}"; Flags: uninsalwaysuninstall; Tasks: allusers
Name: "{ini:{param:SETTINGS|},Locations,IncompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,IncompletePath|{app}\Incomplete}}"; Flags: uninsalwaysuninstall; MinVersion: 4.0,0
Name: "{ini:{param:SETTINGS|},Locations,TorrentPath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,TorrentPath|{userappdata}\Shareaza\Torrents}}"; Flags: uninsalwaysuninstall; Tasks: currentuser
Name: "{ini:{param:SETTINGS|},Locations,TorrentPath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,TorrentPath|{commonappdata}\Shareaza\Torrents}}"; Flags: uninsalwaysuninstall; Tasks: allusers
Name: "{ini:{param:SETTINGS|},Locations,TorrentPath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,TorrentPath|{app}\Torrents}}"; Flags: uninsalwaysuninstall; MinVersion: 4.0,0
Name: "{ini:{param:SETTINGS|},Locations,CollectionPath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CollectionPath|{userappdata}\Shareaza\Collections}}"; Flags: uninsalwaysuninstall; Tasks: currentuser
Name: "{ini:{param:SETTINGS|},Locations,CollectionPath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CollectionPath|{commonappdata}\Shareaza\Collections}}"; Flags: uninsalwaysuninstall; Tasks: allusers
Name: "{ini:{param:SETTINGS|},Locations,CollectionPath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CollectionPath|{app}\Collections}}"; Flags: uninsalwaysuninstall; MinVersion: 4.0,0

[InstallDelete]
; Clean up old files from Shareaza
Type: files; Name: "{app}\zlib1.dll"
Type: files; Name: "{app}\LICENSE.txt"
Type: files; Name: "{app}\uninstall.exe"
Type: files; Name: "{app}\Plugins\DivFix.dll"

; Clean up old Shareaza icons
Type: files; Name: "{userdesktop}\Start Shareaza.lnk"
Type: filesandordirs; Name: "{userprograms}\Shareaza"

; Delete extra components so installer can "uninstall" them
Type: filesandordirs; Name: "{app}\Skins"
Type: filesandordirs; Name: "{app}\Plugins"
Type: filesandordirs; Name: "{app}\Remote"

[UninstallDelete]
Type: files; Name: "{app}\*.dat"
Type: files; Name: "{app}\*.xml"
Type: files; Name: "{app}\Data\*.dat"
Type: files; Name: "{app}\Data\*.xml"

; Pull in languages and localized files
#include "languages.iss"
; Pull in Shareaza settings to write to registry
#include "settings.iss"

; Code sections need to be the last section in a script or the compiler will get confused
[Code]
Function ShareazaInstalled(): boolean;
Begin
    Result := RegKeyExists(HKEY_LOCAL_MACHINE, 'SOFTWARE\Shareaza');
End;

Function InnoSetupUsed(): boolean;
Begin
    Result := RegKeyExists(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Shareaza_is1');
End;

const
  WM_CLOSE = $0010;

procedure CurStepChanged(CurStep: Integer);
var
  Wnd: HWND;
begin
  if CurStep = csCopy then
    Wnd := FindWindowByClassName('ShareazaMainWnd');
    if Wnd <> 0 then
      SendMessage(Wnd, WM_CLOSE, 0, 0);
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Wnd: HWND;
begin
  if CurUninstallStep = usUninstall then
    Wnd := FindWindowByClassName('ShareazaMainWnd');
    if Wnd <> 0 then
      SendMessage(Wnd, WM_CLOSE, 0, 0);
end;
