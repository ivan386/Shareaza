#define version GetFileVersion("..\builds\TorrentWizard.exe")

[Setup]
AppName=TorrentAid
AppVerName=TorrentAid {#version}
AppPublisher=Shareaza Development Team
DefaultDirName={reg:HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Shareaza_is1,Inno Setup: App Path|{reg:HKLM\SOFTWARE\Shareaza,|{pf}\Shareaza}}
DisableDirPage=yes
DefaultGroupName=TorrentAid
DisableProgramGroupPage=yes
Compression=lzma/ultra
InternalCompressLevel=Ultra
SolidCompression=yes
OutputBaseFilename=TorrentAid_{#version}
OutputDir=setup\builds
VersionInfoCompany=Shareaza Development Team
VersionInfoDescription=TorrentAid
VersionInfoVersion={#version}
AppId=TorrentAid
AppVersion={#version}
DirExistsWarning=no
DisableFinishedPage=yes
DisableReadyPage=yes
PrivilegesRequired=admin
LanguageDetectionMethod=locale
ShowLanguageDialog=auto
UninstallDisplayIcon={app}\TorrentWizard.exe
UninstallDisplayName={cm:NameAndVersion,TorrentAid,{#version}}
UninstallFilesDir={app}\Uninstall
LicenseFile=setup\license\default.rtf

; Set the CVS root as source dir (up 2 levels)
SourceDir=..\..

; links to website for software panel
AppPublisherURL=http://www.torrentaid.com/default.htm
AppSupportURL=http://www.torrentaid.com/tutorial.htm
AppUpdatesURL=http://www.torrentaid.com/download.htm

[Files]
Source: "setup\builds\TorrentWizard.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

[Icons]
Name: "{userprograms}\TorrentAid"; Filename: "{app}\TorrentWizard.exe"; WorkingDir: "{app}"; Comment: "TorrentAid"

[Registry]
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\BitTorrent"; ValueType: string; ValueName: "TorrentCreatorPath"; ValueData: "{app}\TorrentWizard.exe"; Flags: deletevalue
Root: HKCU; Subkey: "Software\TorrentAid\"; Flags: dontcreatekey uninsdeletekey
