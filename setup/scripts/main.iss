; WARNING: Compile repair.iss first!

#define version GetFileVersion("..\builds\Shareaza.exe")

[Setup]
AppComments=Shareaza Ultimate File Sharing
AppId=Shareaza
AppName=Shareaza
AppPublisher=Shareaza Development Team
AppVersion={#version}
AppVerName=Shareaza {#version}
VersionInfoVersion={#version}
DefaultDirName={reg:HKLM\SOFTWARE\Shareaza,|{pf}\Shareaza}
DirExistsWarning=no
DefaultGroupName=Shareaza
AllowNoIcons=yes
OutputDir=setup\builds
OutputBaseFilename=Shareaza_{#version}
SolidCompression=yes
Compression=lzma/ultra
InternalCompressLevel=Ultra
VersionInfoCompany=Shareaza Development Team
VersionInfoDescription=Shareaza Ultimate File Sharing
PrivilegesRequired=poweruser
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
ChangesAssociations=yes

; Set the CVS root as source dir (up 2 levels)
SourceDir=..\..

; links to website for software panel
AppPublisherURL=http://www.shareaza.com/?id=home
AppSupportURL=http://www.shareaza.com/?id=support
AppUpdatesURL=http://www.shareaza.com/?id=download

[Components]
; Ask user wich components to install
Name: "language"; Description: "{cm:components_languages}"; Types: full; Flags: disablenouninstallwarning

[Tasks]
Name: "multiuser"; Description: "{cm:tasks_multisetup}"
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"
Name: "quicklaunch"; Description: "{cm:CreateQuickLaunchIcon}"

[Files]
; Install unicows.dll on Win 9X
Source: "setup\builds\unicows.dll"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension regserver noregerror; MinVersion: 4.0,0
Source: "setup\builds\unicows.dll"; DestDir: "{sys}"; Flags: regserver noregerror overwritereadonly replacesameversion restartreplace sharedfile uninsneveruninstall sortfilesbyextension; MinVersion: 4.0,0

; Main files
Source: "setup\builds\1.dll"; DestDir: "{app}"; DestName: "zlib.dll"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "setup\builds\1.dll"; DestDir: "{app}\Plugins"; DestName: "zlib.dll"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "setup\builds\Shareaza.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "setup\builds\skin.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Schemas\*"; DestDir: "{app}\Schemas"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

; Set up data dir in {app}
Source: "Data\Emoticons.bmp"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Data\Emoticons.xml"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Data\gwebcache.txt"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist
Source: "Data\Vendors.xml"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Data\WorldGPS.dat"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Data\DefaultAvatar.png"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

; Copy repair installer
Source: "setup\builds\repair.exe"; DestDir: "{app}\Uninstall"; Flags: uninsremovereadonly sortfilesbyextension onlyifdoesntexist

; Plugins
; Don't register RazaWebHook.dll since it will setup Shareaza as download manager
Source: "setup\plugins\*.dll"; DestDir: "{app}\Plugins"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension regserver; Excludes: "RazaWebHook.dll"
Source: "setup\plugins\RazaWebHook.dll"; DestDir: "{app}\Plugins"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

; Uninstall icon for software panel
Source: "setup\misc\uninstall.ico"; DestDir: "{app}\Uninstall"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

; Skins
Source: "Skins\BlueStreak\*"; DestDir: "{app}\Skins\BlueStreak"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Skins\CleanBlue\*"; DestDir: "{app}\Skins\CleanBlue"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Skins\Corona\*"; DestDir: "{app}\Skins\Corona"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Skins\Shareaza2\*"; DestDir: "{app}\Skins\Shareaza2"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Skins\ShareazaOS\*"; DestDir: "{app}\Skins\ShareazaOS"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

; Languages: English gets installed by default
Source: "Languages\default-en.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension
Source: "Languages\*"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: "language"; Excludes: "default-en.xml"

; Copy skins back from {userappdata}\Shareaza\Skins
Source: "{userappdata}\Shareaza\Skins\*"; DestDir: "{app}\Skins"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist recursesubdirs; AfterInstall: DeleteMultiSkinDir

; Switch user data between locations
Source: "{app}\Data\Library1.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{app}\Data\Library2.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{app}\Data\DownloadGroups.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{app}\Data\TigerTree.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{app}\Data\Security.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{app}\Data\UploadQueues.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{app}\Data\Searches.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{app}\Data\Profile.xml"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: multiuser
Source: "{userappdata}\Shareaza\Data\Library1.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{userappdata}\Shareaza\Data\Library2.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{userappdata}\Shareaza\Data\DownloadGroups.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{userappdata}\Shareaza\Data\TigerTree.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{userappdata}\Shareaza\Data\Security.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{userappdata}\Shareaza\Data\UploadQueues.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{userappdata}\Shareaza\Data\Searches.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; Tasks: not multiuser
Source: "{userappdata}\Shareaza\Data\Profile.xml"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist; AfterInstall: DeleteMultiDataDir; Tasks: not multiuser

; Copy installer into download and uninstall dir
Source: "{srcexe}"; DestDir: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{userappdata}\Shareaza\Downloads}}"; DestName: "Shareaza_{#version}.exe"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension external onlyifdoesntexist; Tasks: multiuser
Source: "{srcexe}"; DestDir: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{app}\Downloads}}"; DestName: "Shareaza_{#version}.exe"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension external onlyifdoesntexist; Tasks: not multiuser
Source: "{srcexe}"; DestDir: "{app}\Uninstall"; DestName: "setup.exe"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension external

[Icons]
; Shareaza icons
Name: "{userprograms}\{groupname}\Shareaza"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"
Name: "{userdesktop}\Shareaza"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Shareaza"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; Tasks: quicklaunch

; Other icons in user language
Name: "{userprograms}\{groupname}\{cm:icons_license}"; Filename: "{app}\Uninstall\license.rtf"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:icons_license}"
Name: "{userprograms}\{groupname}\{cm:icons_uninstall}"; Filename: "{uninstallexe}"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:UninstallProgram,Shareaza}"; IconFilename: "{app}\Uninstall\uninstall.ico"
Name: "{userprograms}\{groupname}\{cm:icons_downloads}"; Filename: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{userappdata}\Shareaza\Downloads}}"; WorkingDir: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{userappdata}\Shareaza\Downloads}}"; Comment: "{cm:icons_downloads}"; Tasks: multiuser
Name: "{userprograms}\{groupname}\{cm:icons_downloads}"; Filename: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{app}\Downloads}}"; WorkingDir: "{ini:{param:SETTINGS|},Locations,CompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{app}\Downloads}}"; Comment: "{cm:icons_downloads}"; Tasks: not multiuser

[Messages]
; Overwrite standard ISL entries
; DO NOT use for localized messages
BeveledLabel=Shareaza Development Team

[Run]
; Run the skin installer at end of installation
Filename: "{app}\skin.exe"; Parameters: "/installsilent"; WorkingDir: "{app}"; StatusMsg: "{cm:run_skinexe}"
; Run Shareaza at end of installation
Filename: "{app}\Shareaza.exe"; Description: "{cm:LaunchProgram,Shareaza}"; WorkingDir: "{app}"; Flags: postinstall skipifsilent nowait

[UninstallRun]
; Run the skin installer at start of uninstallation and make sure it only runs once
Filename: "{app}\skin.exe"; Parameters: "/uninstallsilent"; WorkingDir: "{app}"; StatusMsg: "{cm:run_skinexe}"; RunOnceId: "uninstallskinexe"

[Registry]
; Write installation path to registry
Root: HKLM; Subkey: "SOFTWARE\Shareaza"; ValueType: string; ValueName: ; ValueData: "{app}"; Flags: uninsdeletekey deletevalue
Root: HKCU; Subkey: "Software\Shareaza\Shareaza"; ValueType: string; ValueName: "Path" ; ValueData: "{app}"; Flags: uninsdeletekey deletevalue
Root: HKCU; Subkey: "Software\Shareaza\Shareaza"; ValueType: string; ValueName: "UserPath" ; ValueData: "{userappdata}\Shareaza"; Flags: uninsdeletekey deletevalue; Tasks: multiuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza"; ValueType: string; ValueName: "UserPath" ; ValueData: "{app}"; Flags: uninsdeletekey deletevalue; Tasks: not multiuser
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\Shareaza.exe"; ValueType: string; ValueName: ; ValueData: "{app}\Shareaza.exe"; Flags: uninsdeletekey deletevalue
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\Shareaza.exe"; ValueType: string; ValueName: "Path"; ValueData: "{app}"; Flags: uninsdeletekey deletevalue

; Install chat notify sound
Root: HKCU; Subkey: "AppEvents\EventLabels\RAZA_IncomingChat"; ValueType: string; ValueName: ; ValueData: "{cm:reg_incomingchat}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Shareaza"; ValueType: string; ValueName: ; ValueData: "{cm:reg_apptitle}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Shareaza\RAZA_IncomingChat\.current"; ValueType: string; ValueName: ; ValueData: "%SystemRoot%\media\notify.wav"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Shareaza\RAZA_IncomingChat\.default"; ValueType: string; ValueName: ; ValueData: "%SystemRoot%\media\notify.wav"; Flags: uninsdeletekey

; Set directory locations
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CompletePath"; ValueData: "{userappdata}\Shareaza\Downloads"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: multiuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "IncompletePath"; ValueData: "{userappdata}\Shareaza\Incomplete"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: multiuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "TorrentPath"; ValueData: "{userappdata}\Shareaza\Torrents"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: multiuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CollectionPath"; ValueData: "{userappdata}\Shareaza\Collections"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: multiuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CompletePath"; ValueData: "{app}\Downloads"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: not multiuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "IncompletePath"; ValueData: "{app}\Incomplete"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: not multiuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "TorrentPath"; ValueData: "{app}\Torrents"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: not multiuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CollectionPath"; ValueData: "{app}\Collections"; Flags: uninsdeletekey createvalueifdoesntexist; Tasks: not multiuser

; Delete keys at uninstall
Root: HKLM; Subkey: "SOFTWARE\Shareaza"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Shareaza"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "Shareaza"; Flags: dontcreatekey uninsdeletevalue

; Delete NSIS entry on software panel
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Shareaza"; Flags: dontcreatekey deletekey

; Create TorrentAid default dir locations
Root: HKCU; Subkey: "Software\TorrentAid\TorrentWizard\Folders"; ValueType: string; ValueName: "001.Path"; ValueData: "{userappdata}\Shareaza\Torrents"; Flags: createvalueifdoesntexist; Tasks: multiuser
Root: HKCU; Subkey: "Software\TorrentAid\TorrentWizard\Folders"; ValueType: string; ValueName: "Last"; ValueData: "{userappdata}\Shareaza\Torrents"; Flags: createvalueifdoesntexist; Tasks: multiuser
Root: HKCU; Subkey: "Software\TorrentAid\TorrentWizard\Folders"; ValueType: string; ValueName: "001.Path"; ValueData: "{app}\Torrents"; Flags: createvalueifdoesntexist; Tasks: not multiuser
Root: HKCU; Subkey: "Software\TorrentAid\TorrentWizard\Folders"; ValueType: string; ValueName: "Last"; ValueData: "{app}\Torrents"; Flags: createvalueifdoesntexist; Tasks: not multiuser

[Dirs]
; Make incomplete, torrent and collection dir
; Note: download dir will be created when installer is copied
Name: "{ini:{param:SETTINGS|},Locations,IncompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,IncompletePath|{userappdata}\Shareaza\Incomplete}}"; Flags: uninsalwaysuninstall; Tasks: multiuser
Name: "{ini:{param:SETTINGS|},Locations,TorrentPath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,TorrentPath|{userappdata}\Shareaza\Torrents}}"; Flags: uninsalwaysuninstall; Tasks: multiuser
Name: "{ini:{param:SETTINGS|},Locations,CollectionPath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CollectionPath|{userappdata}\Shareaza\Collections}}"; Flags: uninsalwaysuninstall; Tasks: multiuser
Name: "{ini:{param:SETTINGS|},Locations,IncompletePath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,IncompletePath|{app}\Incomplete}}"; Flags: uninsalwaysuninstall; Tasks: not multiuser
Name: "{ini:{param:SETTINGS|},Locations,TorrentPath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,TorrentPath|{app}\Torrents}}"; Flags: uninsalwaysuninstall; Tasks: not multiuser
Name: "{ini:{param:SETTINGS|},Locations,CollectionPath|{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CollectionPath|{app}\Collections}}"; Flags: uninsalwaysuninstall; Tasks: not multiuser
Name: "{userappdata}\Shareaza\Data"; Flags: uninsalwaysuninstall; Tasks: multiuser


[InstallDelete]
; Clean up old files from Shareaza
Type: files; Name: "{app}\zlib1.dll"
Type: files; Name: "{app}\LICENSE.txt"
Type: files; Name: "{app}\uninstall.exe"
Type: files; Name: "{app}\Plugins\DivFix.dll"
Type: files; Name: "{app}\Skins\skin.exe"
Type: files; Name: "{app}\*.dat"
Type: files; Name: "{app}\*.xml"
Type: files; Name: "{app}\*.png"
Type: files; Name: "{app}\*.bmp"
Type: filesandordirs; Name: "{userappdata}\Shareaza\Remote"
Type: filesandordirs; Name: "{userappdata}\Shareaza\Schemas"
Type: files; Name: "{userappdata}\Shareaza\Data\DefaultAvatar.png"

; Clean up old Shareaza icons
Type: files; Name: "{userdesktop}\Start Shareaza.lnk"
Type: files; Name: "{userdesktop}\Shareaza.lnk"
Type: filesandordirs; Name: "{userprograms}\Shareaza"
Type: files; Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Shareaza.lnk"

; Delete extra components so installer can "uninstall" them
Type: filesandordirs; Name: "{app}\Remote"
Type: filesandordirs; Name: "{app}\Skins\Languages"; Components: not language
Type: filesandordirs; Name: "{userappdata}\Shareaza\Skins\Languages"; Components: not language

[UninstallDelete]
; Clean up files created after installation
Type: filesandordirs; Name: "{app}\Data"
Type: filesandordirs; Name: "{userappdata}\Shareaza\Data"
Type: filesandordirs; Name: "{app}\Skins"

; Pull in languages and localized files
#include "languages.iss"
; Pull in Shareaza settings to write to registry
#include "settings.iss"

; Code sections need to be the last section in a script or the compiler will get confused

[Code]
const
  WM_CLOSE = $0010;
  KeyLoc1 = 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Shareaza_is1';
  KeyLoc2 = 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Shareaza';
  KeyName = 'UninstallString';
var
  Installed: Boolean;

Function ShareazaInstalled(): boolean;
Begin
    Result := RegKeyExists(HKEY_LOCAL_MACHINE, 'SOFTWARE\Shareaza');
End;

Function InnoSetupUsed(): boolean;
Begin
    Result := RegKeyExists(HKEY_LOCAL_MACHINE, KeyLoc1);
End;

Function NSISUsed(): boolean;
Begin
    Result := RegKeyExists(HKEY_LOCAL_MACHINE, KeyLoc2);
End;

Procedure CurStepChanged(CurStep: TSetupStep);
var
  Wnd: HWND;
Begin
  if CurStep = ssInstall then
    Wnd := FindWindowByClassName('ShareazaMainWnd');
  if Wnd <> 0 then
    SendMessage(Wnd, WM_CLOSE, 0, 0);
End;

Function InitializeSetup: Boolean;
Begin
  Result := True;
  Installed := RegValueExists(HKEY_LOCAL_MACHINE, KeyLoc1, KeyName) or RegValueExists(HKEY_LOCAL_MACHINE, KeyLoc2, KeyName);
End;

Function ShouldSkipPage(PageID: Integer): Boolean;
Begin
  Result := False;
  if PageID = wpSelectDir then Result := Installed;
  if PageID = wpSelectProgramGroup then Result := Installed;
End;

Procedure DeleteSingleDataDir();
var
  SingleDataDir: string;
Begin
  SingleDataDir := ExpandConstant('{app}\Data');
  DelTree(SingleDataDir, True, True, True);
End;

Procedure DeleteSingleSkinDir();
var
  SingleSkinDir: string;
Begin
  SingleSkinDir := ExpandConstant('{app}\Skins');
  DelTree(SingleSkinDir, True, True, True);
End;

Procedure DeleteMultiDataDir();
var
  MultiDataDir: string;
Begin
  MultiDataDir := ExpandConstant('{userappdata}\Shareaza\Data');
  DelTree(MultiDataDir, True, True, True);
End;

Procedure DeleteMultiSkinDir();
var
  MultiSkinDir: string;
Begin
  MultiSkinDir := ExpandConstant('{userappdata}\Shareaza\Skins');
  DelTree(MultiSkinDir, True, True, True);
End;

Procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Wnd: HWND;
Begin
  if CurUninstallStep = usUninstall then
    Wnd := FindWindowByClassName('ShareazaMainWnd');
    if Wnd <> 0 then
      SendMessage(Wnd, WM_CLOSE, 0, 0);
      Sleep(1000)
End;


