#define version GetFileVersion("..\builds\TorrentWizard.exe")

[Setup]
AppName=Torrent Wizard
AppVerName=Torrent Wizard {#version}
AppPublisher=TorrentAid
DefaultDirName={reg:HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Shareaza_is1,Inno Setup: App Path|{reg:HKLM\SOFTWARE\Shareaza,|{pf}\Shareaza}}
DisableDirPage=yes
DefaultGroupName=Torrent Wizard
DisableProgramGroupPage=yes
Compression=lzma/ultra
InternalCompressLevel=Ultra
SolidCompression=yes
OutputBaseFilename=Torrent Wizard {#version}
OutputDir=setup\builds
VersionInfoCompany=TorrentAid
VersionInfoDescription=Torrent Wizard
VersionInfoVersion={#version}
AppId=TorrentAid
AppVersion={#version}
DirExistsWarning=no
DisableFinishedPage=yes
DisableReadyPage=yes
PrivilegesRequired=admin
ShowLanguageDialog=auto
UninstallDisplayIcon={app}\TorrentWizard.exe
UninstallDisplayName={cm:NameAndVersion,Torrent Wizard,{#version}}
UninstallFilesDir={app}\Uninstall

; Set the CVS root as source dir (up 2 levels)
SourceDir=..\..

; links to website for software panel
AppPublisherURL=http://www.torrentaid.com/default.htm
AppSupportURL=http://www.torrentaid.com/tutorial.htm
AppUpdatesURL=http://www.torrentaid.com/download.htm

[Files]
Source: "setup\builds\TorrentWizard.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

[Icons]
Name: "{userprograms}\Torrent Wizard"; Filename: "{app}\TorrentWizard.exe"; WorkingDir: "{app}"; Comment: "Torrent Wizard"

[Registry]
Root: HKCU; Subkey: "Software\TorrentAid\"; Flags: dontcreatekey uninsdeletekey
