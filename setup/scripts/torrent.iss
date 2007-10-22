#define version GetFileVersion("..\builds\TorrentWizard.exe")

[Setup]
AppName=TorrentAid
AppVerName=TorrentAid {#version}
AppPublisher=Shareaza Development Team
DefaultDirName={pf}\TorrentAid
DefaultGroupName=TorrentAid
DisableProgramGroupPage=yes
Compression=lzma/max
InternalCompressLevel=max
SolidCompression=yes
OutputBaseFilename=TorrentAid_{#version}
OutputDir=setup\builds
VersionInfoCompany=Shareaza Development Team
VersionInfoDescription=TorrentAid
VersionInfoVersion={#version}
AppId=TorrentAid
AppVersion={#version}
DirExistsWarning=no
PrivilegesRequired=poweruser
LanguageDetectionMethod=locale
ShowLanguageDialog=auto
UninstallDisplayIcon={app}\TorrentWizard.exe
UninstallDisplayName={cm:NameAndVersion,TorrentAid,{#version}}
UninstallFilesDir={app}\Uninstall
LicenseFile=setup\license\default.rtf
SetupIconFile=torrentaid\Res\TorrentWizard.ico
WizardImageFile=setup\misc\WizardImages\SetupModern16.bmp
WizardSmallImageFile=setup\misc\WizardImages\Small\OtherModernSmall01.bmp

; Set the CVS root as source dir (up 2 levels)
SourceDir=..\..

; links to website for software panel
AppPublisherURL=http://shareaza.sourceforge.net/TorrentAid/
AppSupportURL=http://shareaza.sourceforge.net/?id=support
AppUpdatesURL=http://shareaza.sourceforge.net/TorrentAid/?id=download

[Files]
; Install unicows.dll on Win 9X
Source: "setup\builds\unicows.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension; MinVersion: 4.0,0

; Main file
Source: "setup\builds\TorrentWizard.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

; Uninstall icon for software panel
Source: "torrentaid\Res\uninstall.ico"; DestDir: "{app}\Uninstall"; Flags: ignoreversion sortfilesbyextension

[Icons]
Name: "{group}\Torrent Wizard"; Filename: "{app}\TorrentWizard.exe"; WorkingDir: "{app}"; Comment: "TorrentAid Torrent Wizard"
Name: "{group}\Uninstall"; Filename: "{uninstallexe}"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:UninstallProgram,TorrentAid}"; IconFilename: "{app}\Uninstall\uninstall.ico"

[Registry]
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\BitTorrent"; ValueType: string; ValueName: "TorrentCreatorPath"; ValueData: "{app}\TorrentWizard.exe"; Flags: deletevalue uninsdeletevalue
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\BitTorrent"; ValueType: string; ValueName: "TorrentCreatorPath"; ValueData: "{app}\TorrentWizard.exe"; Flags: deletevalue uninsdeletevalue
Root: HKCU; Subkey: "Software\TorrentAid\"; Flags: dontcreatekey uninsdeletekey

[InstallDelete]
Type: files; Name: "{userprograms}\TorrentAid.*"
Type: filesandordirs; Name: "{group}"

