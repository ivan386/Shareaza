#define version GetFileVersion("..\builds\Shareaza.exe")

[Setup]
AppName=Shareaza
AppVerName=Shareaza {#version}
AppPublisher=Shareaza Development Team
AppId=Shareaza
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
PrivilegesRequired=poweruser
LanguageDetectionMethod=locale
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
Filename: "{param:SETTINGS|{tmp}\settings.ini}"; Section: "Locations"; key: "UserPath"; String: "{reg:HKCU\Software\Shareaza\Shareaza,UserPath|}"; Flags: createkeyifdoesntexist


[Registry]
; Delete all settings before starting reparation
Root: HKLM; Subkey: "SOFTWARE\Shareaza"; Flags: dontcreatekey deletekey
Root: HKCU; Subkey: "Software\Shareaza"; Flags: dontcreatekey deletekey
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "Shareaza"; Flags: dontcreatekey deletevalue

[InstallDelete]
; Delete all files exept uninstall dir
Type: filesandordirs; Name: "{app}\Skins"
Type: filesandordirs; Name: "{app}\Data"
Type: filesandordirs; Name: "{app}\Schemas"
Type: filesandordirs; Name: "{app}\Remote"
Type: filesandordirs; Name: "{app}\Plugins"
Type: filesandordirs; Name: "{userappdata}\Shareaza\Skins"
Type: filesandordirs; Name: "{userappdata}\Shareaza\Data"
Type: filesandordirs; Name: "{userappdata}\Shareaza\Schemas"
Type: filesandordirs; Name: "{userappdata}\Shareaza\Remote"
Type: filesandordirs; Name: "{userappdata}\Shareaza\Plugins"
Type: files; Name: "{app}\unicows.dll"
Type: files; Name: "{app}\zlib.dll"
Type: files; Name: "{app}\Shareaza.exe"
Type: files; Name: "{app}\skin.exe"

[Files]
; Copy installer stored in uninstall dir to temp location to avoid "in-use" error
Source: "{app}\Uninstall\setup.exe"; DestDir: "{tmp}"; DestName: "temp.exe"; Flags: ignoreversion overwritereadonly sortfilesbyextension external

[Run]
; Run installer in silent mode and pass all settings.
Filename: "{tmp}\temp.exe"; WorkingDir: "{app}"; Flags: hidewizard; Parameters: "/SILENT /NOCANCEL /DIR=""{app}"" /GROUP=""{groupname}"" /SETTINGS=""{param:SETTINGS|{tmp}\settings.ini}"""

[Messages]
; This section overrides phrases and words from the ISL files
en.WelcomeLabel1=Welcome to the [name] Repair Wizard
en.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.
nl.WelcomeLabel1=Welkom bij de [name] reparatiewizard
nl.WelcomeLabel2=Hiermee wordt [name] hersteld op deze computer.%n%nU wordt aanbevolen alle actieve programma's af te sluiten voordat u verder gaat. Dit helpt conflicten tijdens de installatie voorkomen.
lt.WelcomeLabel1=Sveiki! Èia „[name]“ taisymo vedlys.
lt.WelcomeLabel2=Jis pataisys „[name]“, ádiegtà Jûsø kompiuteryje.%n%nPatariama baigti darbà su visomis programomis prieð tæsiant.
de.WelcomeLabel1=Welcome to the [name] Repair Wizard
de.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.
pt.WelcomeLabel1=Welcome to the [name] Repair Wizard
pt.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.
it.WelcomeLabel1=Welcome to the [name] Repair Wizard
it.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.
no.WelcomeLabel1=Welcome to the [name] Repair Wizard
no.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.
af.WelcomeLabel1=Welcome to the [name] Repair Wizard
af.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.
br.WelcomeLabel1=Welcome to the [name] Repair Wizard
br.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.
fr.WelcomeLabel1=Welcome to the [name] Repair Wizard
fr.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.
es.WelcomeLabel1=Welcome to the [name] Repair Wizard
es.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.
ru.WelcomeLabel1=Äîáðî ïîæàëîâàòü â ìàñòåð èñïðàâëåíèé [name]
ru.WelcomeLabel2=Îí èñïðàâèò óñòàíîâêó [name] â Âàøåì êîìïüþòåðå.%n%nÐåêîìåíäóåòñÿ çàâåðøèòü ðàáîòó âñåõ ïðîãðàìì ïðåæäå ÷åì ïðèñòóïàòü ê äàëíåéøèì äåéñòâèÿì.
gr.WelcomeLabel1=Welcome to the [name] Repair Wizard
gr.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.
hu.WelcomeLabel1=Welcome to the [name] Repair Wizard
hu.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.
chs.WelcomeLabel1=»¶Ó­Ê¹ÓÃ [name] ÐÞ¸´Ïòµ¼
chs.WelcomeLabel2=´Ë²Ù×÷½«ÐÞ¸´ÄúµçÄÔÉÏµÄ [name]¡£%n%n½¨ÒéÄúÔÚ¼ÌÐø´Ë²Ù×÷Ç°¹Ø±ÕËùÓÐÆäËû³ÌÐò¡£
sv.WelcomeLabel1=Welcome to the [name] Repair Wizard
sv.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.

[Languages]
; Use compiler's built in ISL file to patch up holes in ISL collection and specify localized license files
; Note: first language specified is default > English

Name: "en"; MessagesFile: "compiler:Default.isl,setup\isl\default.isl"
Name: "nl"; MessagesFile: "compiler:Default.isl,setup\isl\dutch.isl"
Name: "lt"; MessagesFile: "compiler:Default.isl,setup\isl\lithuanian.isl"
Name: "de"; MessagesFile: "compiler:Default.isl,setup\isl\german.isl"
Name: "pt"; MessagesFile: "compiler:Default.isl,setup\isl\portuguese-std.isl"
Name: "it"; MessagesFile: "compiler:Default.isl,setup\isl\italian.isl"
Name: "no"; MessagesFile: "compiler:Default.isl,setup\isl\norwegian.isl"
Name: "af"; MessagesFile: "compiler:Default.isl,setup\isl\afrikaans.isl"
Name: "br"; MessagesFile: "compiler:Default.isl,setup\isl\portuguese-braz.isl"
Name: "fr"; MessagesFile: "compiler:Default.isl,setup\isl\french.isl"
Name: "es"; MessagesFile: "compiler:Default.isl,setup\isl\spanish.isl"
Name: "ru"; MessagesFile: "compiler:Default.isl,setup\isl\russian.isl"
Name: "gr"; MessagesFile: "compiler:Default.isl,setup\isl\greek.isl"
Name: "hu"; MessagesFile: "compiler:Default.isl,setup\isl\hungarian.isl"
Name: "chs"; MessagesFile: "compiler:Default.isl,setup\isl\chinese-simp.isl"
Name: "sv"; MessagesFile: "compiler:Default.isl,setup\isl\swedish.isl"

[Code]
const
  WM_CLOSE = $0010;

procedure CurStepChanged(CurStep: TSetupStep);
var
  Wnd: HWND;
begin
  if CurStep = ssInstall then
    Wnd := FindWindowByClassName('ShareazaMainWnd');
    if Wnd <> 0 then
      SendMessage(Wnd, WM_CLOSE, 0, 0);
      while Wnd <> 0 do
        begin
          Sleep(100);
          Wnd := FindWindowByClassName('ShareazaMainWnd');
        End
end;
