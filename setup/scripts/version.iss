; This sub-script include keys wich need to be changed at each release

[Setup]
; Version branding keys
AppVersion=2.0.0.5
AppVerName=Shareaza 2.0.0.5
VersionInfoVersion=2.0.0.5

; links to website for software panel
AppPublisherURL=http://www.shareaza.com/?id=home
AppSupportURL=http://www.shareaza.com/?id=support
AppUpdatesURL=http://www.shareaza.com/?id=download

[Files]
; Copy installer into download dir
Source: "{srcexe}"; DestDir: "{userappdata}\Shareaza\Downloads"; DestName: "Shareaza 2.0.0.5.exe"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension external; Components: mainfiles; Tasks: currentuser
Source: "{srcexe}"; DestDir: "{commonappdata}\Shareaza\Downloads"; DestName: "Shareaza 2.0.0.5.exe"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension external; Components: mainfiles; Tasks: allusers
