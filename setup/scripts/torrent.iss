; Select file source root
#ifexist "..\..\vc7_1\ReleaseWin32\TorrentWizard.exe"
  #define root "vc7_1\ReleaseWin32"
  #define version GetFileVersion("..\..\vc7_1\ReleaseWin32\TorrentWizard.exe")
#endif
#ifexist "..\..\vc8_0\ReleaseWin32\TorrentWizard.exe"
  #define root "vc8_0\ReleaseWin32"
  #define version GetFileVersion("..\..\vc8_0\ReleaseWin32\TorrentWizard.exe")
#endif
#ifexist "..\..\vc9\Win32\Release\TorrentWizard.exe"
  #define root "vc9\Win32\Release"
  #define version GetFileVersion("..\..\vc9\Win32\Release\TorrentWizard.exe")
#endif
#ifexist "..\..\vc10\Win32\Release\TorrentWizard.exe"
  #define root "vc10\Win32\Release"
  #define version GetFileVersion("..\..\vc10\Win32\Release\TorrentWizard.exe")
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
SolidCompression=yes
Compression=lzma2/max
InternalCompressLevel=max
LZMAUseSeparateProcess=yes
OutputBaseFilename=TorrentWizard_{#version}
OutputDir=setup\builds
VersionInfoCompany=Shareaza Development Team
VersionInfoDescription=TorrentWizard
VersionInfoVersion={#version}
AppId=TorrentWizard
AppVersion={#version}
DirExistsWarning=no
PrivilegesRequired=poweruser
ShowLanguageDialog=yes
LanguageDetectionMethod=locale
UninstallDisplayIcon={app}\TorrentWizard.exe
UninstallDisplayName=TorrentWizard {#version}
UninstallFilesDir={app}\Uninstall
LicenseFile=setup\license\default.rtf
SetupIconFile=TorrentWizard\Res\TorrentWizard.ico
ShowComponentSizes=no
WizardImageFile=setup\misc\sidebar.bmp
WizardSmallImageFile=setup\misc\corner.bmp
MinVersion=5.0

; Set the CVS root as source dir (up 2 levels)
SourceDir=..\..

; links to website for software panel
AppPublisherURL=http://shareaza.sourceforge.net/TorrentAid/
AppSupportURL=http://shareaza.sourceforge.net/?id=support
AppUpdatesURL=http://shareaza.sourceforge.net/TorrentAid/?id=download

[Files]
; Main file
Source: "{#root}\TorrentWizard.exe"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
; HashLib
Source: "{#root}\HashLib.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

[Icons]
Name: "{group}\Torrent Wizard"; Filename: "{app}\TorrentWizard.exe"; WorkingDir: "{app}"; Comment: "Shareaza Torrent Wizard"

[Registry]
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\BitTorrent"; ValueType: string; ValueName: "TorrentCreatorPath"; ValueData: "{app}\TorrentWizard.exe"; Flags: noerror deletevalue uninsdeletevalue
Root: HKCU; Subkey: "Software\Shareaza\TorrentWizard\"; Flags: noerror dontcreatekey uninsdeletekey
