; WARNING: Compile repair.iss first!
; If it exists, I didn't finish that yet. :-)

[Setup]
AppComments=Shareaza Ultimate File Sharing
AppId=Shareaza
AppMutex=Shareaza
AppName=Shareaza
AppPublisher=Shareaza Development Team
AppReadmeFile={app}\Uninstall\readme.txt
DefaultDirName={pf}\Shareaza
DefaultGroupName=Shareaza
DisableFinishedPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes
OutputDir=setup\builds
SolidCompression=yes
Compression=lzma/ultra
VersionInfoCompany=Shareaza Development Team
VersionInfoDescription=Shareaza Ultimate File Sharing
PrivilegesRequired=admin
ShowLanguageDialog=auto
UninstallDisplayIcon={app}\Uninstall\uninstall.ico
UninstallDisplayName=Shareaza
UninstallFilesDir={app}\Uninstall
UninstallIconFile=setup\misc\uninstall.ico
SetupIconFile=setup\misc\install.ico
ShowComponentSizes=no
WizardImageFile=setup\misc\sidebar.bmp
WizardSmallImageFile=setup\misc\corner.bmp

; Set the CVS root as source dir (up 2 levels)
SourceDir=..\..

[Components]
; Ask user wich components to install
Name: "mainfiles"; Description: "{cm:components_mainfiles}"; Types: full compact custom; Flags: fixed
Name: "plugins"; Description: "{cm:components_plugins}"; Types: full compact; Flags: disablenouninstallwarning
Name: "skins"; Description: "{cm:components_skins}"; Types: full; Flags: disablenouninstallwarning
Name: "languages"; Description: "{cm:components_languages}"; Types: full; Flags: disablenouninstallwarning

[Tasks]
; Ask user to setup Shareaza for all users or just current user
Name: allusers; Description: "{cm:tasks_allusers}"; GroupDescription: "{cm:tasks_selectusers,Shareaza}"; Flags: exclusive
Name: currentuser; Description: "{cm:tasks_currentuser,{username}}"; GroupDescription: "{cm:tasks_selectusers,Shareaza}"; Flags: exclusive

[Files]
; Need zlib.dll in {sys} or regserver will crash
; Place this entriy before other entries using regserver
Source: "setup\builds\zlib.dll"; DestDir: "{sys}"; Flags: regserver noregerror overwritereadonly replacesameversion restartreplace sharedfile uninsneveruninstall sortfilesbyextension; Components: mainfiles

; Install unicows.dll on Win 9X
Source: "setup\builds\unicows.dll"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension regserver noregerror; Components: mainfiles; MinVersion: 4.0,0

; Main files
Source: "setup\builds\zlib.dll"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles
Source: "setup\builds\zlib.dll"; DestDir: "{app}\Skins"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles
Source: "setup\builds\Shareaza.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles
Source: "setup\builds\skin.exe"; DestDir: "{app}\Skins"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles
Source: "Data\DefaultAvatar.png"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles
Source: "Data\Emoticons.bmp"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles
Source: "Data\*.xml"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles
Source: "Data\WorldGPS.dat"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles
Source: "Schemas\*.ico"; DestDir: "{app}\Schemas"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles
Source: "Schemas\*.xml"; DestDir: "{app}\Schemas"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles
Source: "Schemas\*.xsd"; DestDir: "{app}\Schemas"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles

; Plugins
Source: "setup\plugins\*.dll"; DestDir: "{app}\Plugins"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension regserver noregerror; Components: plugins

; Uninstall icon for software panel
Source: "setup\misc\uninstall.ico"; DestDir: "{app}\Uninstall"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles

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
Source: "Languages\default-en.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles
Source: "Languages\*.bmp"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Components: "languages"
Source: "Languages\*.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Components: "languages"
Source: "Languages\*.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: "languages"

; Old versions of Shareaza stored xml and dat files under {app}
; These need to be copied to {app}\Data
Source: "{app}\*.dat"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{app}\*.xml"; DestDir: "{app}\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist

[Icons]
; Shareaza icon
Name: "{userprograms}\{groupname}\Shareaza"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "Shareaza Ultimate File Sharing"; Tasks: currentuser
Name: "{commonprograms}\{groupname}\Shareaza"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "Shareaza Ultimate File Sharing"; Tasks: allusers
; License and uninstall icon in user language
Name: "{userprograms}\{groupname}\{cm:icons_license}"; Filename: "{app}\Uninstall\license.rtf"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:icons_license}"; Tasks: currentuser
Name: "{userprograms}\{groupname}\{cm:icons_uninstall}"; Filename: "{uninstallexe}"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:UninstallProgram,Shareaza}"; Tasks: currentuser
Name: "{commonprograms}\{groupname}\{cm:icons_license}"; Filename: "{app}\Uninstall\license.rtf"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:icons_license}"; Tasks: allusers
Name: "{commonprograms}\{groupname}\{cm:icons_uninstall}"; Filename: "{uninstallexe}"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:UninstallProgram,Shareaza}"; Tasks: allusers

[Messages]
; Overwrite standard ISL entries
; DO NOT use for localized messages
BeveledLabel=Shareaza Development Team

[Run]
; Run the skin installer at end of installation
Filename: "{app}\Skins\skin.exe"; Parameters: "/installsilent"; WorkingDir: "{app}\Skins"; StatusMsg: "{cm:run_skinexe}"

[UninstallRun]
; Run the skin installer at start of uninstallation and make sure it only runs once
Filename: "{app}\Skins\skin.exe"; Parameters: "/uninstallsilent"; WorkingDir: "{app}\Skins"; StatusMsg: "{cm:run_skinexe}"; RunOnceId: "uninstallskinexe"

[Registry]
; Write installation path to registry
Root: HKLM; Subkey: "SOFTWARE\Shareaza"; ValueType: string; ValueName: ; ValueData: "{app}"; Flags: uninsdeletekey

; Install chat notify sound
Root: HKCU; Subkey: "AppEvents\EventLabels\RAZA_IncomingChat"; ValueType: string; ValueName: ; ValueData: "{cm:reg_incomingchat}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Shareaza"; ValueType: string; ValueName: ; ValueData: "{cm:reg_apptitle}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Shareaza\RAZA_IncomingChat\.current"; ValueType: string; ValueName: ; ValueData: "%SystemRoot%\media\notify.wav"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Shareaza\RAZA_IncomingChat\.default"; ValueType: string; ValueName: ; ValueData: "%SystemRoot%\media\notify.wav"; Flags: uninsdeletekey

; Set directory locations
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CompletePath"; ValueData: "{userappdata}\Shareaza\Downloads"; Flags: uninsdeletekey; Tasks: currentuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CompletePath"; ValueData: "{commonappdata}\Shareaza\Downloads"; Flags: uninsdeletekey; Tasks: allusers
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "IncompletePath"; ValueData: "{userappdata}\Shareaza\Incomplete"; Flags: uninsdeletekey; Tasks: currentuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "IncompletePath"; ValueData: "{commonappdata}\Shareaza\Incomplete"; Flags: uninsdeletekey; Tasks: allusers
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "TorrentPath"; ValueData: "{userappdata}\Shareaza\Torrents"; Flags: uninsdeletekey; Tasks: currentuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "TorrentPath"; ValueData: "{commonappdata}\Shareaza\Torrents"; Flags: uninsdeletekey; Tasks: allusers
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CollectionPath"; ValueData: "{userappdata}\Shareaza\Collections"; Flags: uninsdeletekey; Tasks: currentuser
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: string; ValueName: "CollectionPath"; ValueData: "{commonappdata}\Shareaza\Collections"; Flags: uninsdeletekey; Tasks: allusers

; Delete keys at uninstall
Root: HKLM; Subkey: "SOFTWARE\Shareaza"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Shareaza"; Flags: dontcreatekey uninsdeletekey

[Dirs]
; Make incomplete, torrent and collection dir
; Note: download dir will be created when installer is copied
Name: "{userappdata}\Shareaza\Incomplete"; Flags: uninsalwaysuninstall; Tasks: currentuser
Name: "{commonappdata}\Shareaza\Incomplete"; Flags: uninsalwaysuninstall; Tasks: allusers
Name: "{userappdata}\Shareaza\Torrents"; Flags: uninsalwaysuninstall; Tasks: currentuser
Name: "{commonappdata}\Shareaza\Torrents"; Flags: uninsalwaysuninstall; Tasks: allusers
Name: "{userappdata}\Shareaza\Collections"; Flags: uninsalwaysuninstall; Tasks: currentuser
Name: "{commonappdata}\Shareaza\Collections"; Flags: uninsalwaysuninstall; Tasks: allusers

[InstallDelete]
; Clean up old files from Shareaza
Type: files; Name: "{app}\zlib1.dll"
Type: files; Name: "{app}\Plugins\DivFix.dll"

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
; Pull in version-specific keys
#include "version.iss"
; Pull in keys common to main and repair installer
; #include "common.iss" Note: Not finished yet

