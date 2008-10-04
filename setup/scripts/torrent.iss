; Select file source root
#ifexist "..\..\vc7_1\ReleaseWin32\TorrentWizard.exe"
  #define root "vc7_1\ReleaseWin32"
  #define version GetFileVersion("..\..\vc7_1\ReleaseWin32\TorrentWizard.exe")
#endif
#ifexist "..\..\vc8_0\ReleaseWin32\TorrentWizard.exe"
  #define root "vc8_0\ReleaseWin32"
  #define version GetFileVersion("..\..\vc8_0\ReleaseWin32\TorrentWizard.exe")
#endif

#ifndef root
  #error You must compile TorrentWizard before compile the setup
#endif

[Setup]
AppName=TorrentWizard
AppVerName=TorrentWizard {#version}
AppPublisher=Shareaza Development Team
DefaultDirName={pf}\TorrentWizard
DefaultGroupName=TorrentWizard
DisableProgramGroupPage=yes
Compression=lzma/max
InternalCompressLevel=max
SolidCompression=yes
OutputBaseFilename=TorrentWizard_{#version}
OutputDir=setup\builds
VersionInfoCompany=Shareaza Development Team
VersionInfoDescription=TorrentWizard
VersionInfoVersion={#version}
AppId=TorrentWizard
AppVersion={#version}
DirExistsWarning=no
PrivilegesRequired=poweruser
LanguageDetectionMethod=locale
ShowLanguageDialog=auto
UninstallDisplayIcon={app}\TorrentWizard.exe
UninstallDisplayName={cm:NameAndVersion,TorrentWizard,{#version}}
UninstallFilesDir={app}\Uninstall
LicenseFile=setup\license\default.rtf
SetupIconFile=TorrentWizard\Res\TorrentWizard.ico
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
Source: "{#root}\TorrentWizard.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

; Uninstall icon for software panel
Source: "TorrentWizard\Res\uninstall.ico"; DestDir: "{app}\Uninstall"; Flags: ignoreversion sortfilesbyextension

[Icons]
Name: "{group}\Torrent Wizard"; Filename: "{app}\TorrentWizard.exe"; WorkingDir: "{app}"; Comment: "Shareaza Torrent Wizard"
Name: "{group}\Uninstall"; Filename: "{uninstallexe}"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:UninstallProgram,TorrentWizard}"; IconFilename: "{app}\Uninstall\uninstall.ico"

[Registry]
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\BitTorrent"; ValueType: string; ValueName: "TorrentCreatorPath"; ValueData: "{app}\TorrentWizard.exe"; Flags: deletevalue uninsdeletevalue
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\BitTorrent"; ValueType: string; ValueName: "TorrentCreatorPath"; ValueData: "{app}\TorrentWizard.exe"; Flags: deletevalue uninsdeletevalue
Root: HKCU; Subkey: "Software\Shareaza\TorrentWizard\"; Flags: dontcreatekey uninsdeletekey

[InstallDelete]
Type: files; Name: "{userprograms}\TorrentWizard.*"
Type: filesandordirs; Name: "{group}"

