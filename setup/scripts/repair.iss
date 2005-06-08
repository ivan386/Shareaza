#if VER < 0x05010000
  #error Inno Setup version 5.1.4 or higher is needed for this script
#endif
#if PREPROCVER < 0x05010000
  #error PreProcessor version 5.1.2 or higher is needed for this script
#endif

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
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Windows"; ValueType: dword; ValueName: "RunLanguage"; ValueData: "0"; Flags: deletevalue
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Windows"; ValueType: dword; ValueName: "RunWizard"; ValueData: "0"; Flags: deletevalue

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
Type: files; Name: "{app}\zlib1.dll"
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
lt.WelcomeLabel1=Sveiki! Èia „[name]“ atkûrimo vedlys.
lt.WelcomeLabel2=Jis pataisys „[name]“, ádiegtà Jûsø kompiuteryje.%n%nPatariama baigti darbà su visomis programomis prieð tæsiant.
de.WelcomeLabel1=Wilkommen beim [name] Reparaturassistent
de.WelcomeLabel2=Dieser Assistent wird [name] auf ihrem Computer reparieren.%n%nEs wird empfohlen, dass Sie alle anderen Programme beenden, bevor Sie fortfahren.
pt.WelcomeLabel1=Welcome to the [name] Repair Wizard
pt.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.
it.WelcomeLabel1=Benvenuto nella procedura guidata Riparazione Installazione di [name]
it.WelcomeLabel2=Questo riparerà [name] sul tuo computer.%n%nSi consiglia di chiudere tutte le applicazioni attive prima di procedere.
no.WelcomeLabel1=Velkommen til [name] sin reparasjonsveiviser
no.WelcomeLabel2=Dette vil reparere [name] på din datamaskin.%n%nDet er anbefalt at du lukker alle andre programmer f¸r du fortsetter.
af.WelcomeLabel1=Welkom by die [name] Herstel Assistent
af.WelcomeLabel2=Hierdie sal die [name] installasie op jou rekenaar herstel.%n%Dit word aanbeveel dat jy alle ander programme sluit voor jy voortgaan.
br.WelcomeLabel1=Bem-vindo ao Assistente de Reparo [name]
br.WelcomeLabel2=Isto vai corrigir os problemas com [name] no seu computador.%n%nÉ recomendado que você feche todas as outras aplicações antes de continuar.
fr.WelcomeLabel1=Bienvenue dans l'assistant de réparation [name]
fr.WelcomeLabel2=Ceci réparera [name] sur votre ordinateur.%n%nIl est recommandé que vous fermiez toutes les autres applications avant de continuer.
es.WelcomeLabel1=Bievenido al asistente de reparación de [name].
es.WelcomeLabel2=Esto va a reparar [name] en tu computadora.%n%nIt es recomendado que cierres todas las aplicaciones antes de continuar.
ru.WelcomeLabel1=Äîáðî ïîæàëîâàòü â ìàñòåð âîññòàíîâëåíèÿ [name]
ru.WelcomeLabel2=Îí èñïðàâèò óñòàíîâêó [name] â Âàøåì êîìïüþòåðå.%n%nÐåêîìåíäóåòñÿ çàâåðøèòü ðàáîòó âñåõ ïðîãðàìì ïðåæäå ÷åì ïðèñòóïàòü ê äàëíåéøèì äåéñòâèÿì.
gr.WelcomeLabel1=Êáëþò Þëèáôå óôï âïçèü åðéäéüñèùóçò ôïõ [name]
gr.WelcomeLabel2=Áõôü èá åðéäéïñèþóåé ôï [name] óôïí õðïëïãéóôÞ óáò.%n%nÐñïôåßíåôå íá êëåßóåôå üëåò ôéò Üëëåò åöáñìïãÝò ðïõ ôñÝ÷ïõí, ðñïôïõ óõíå÷ßóåôå
hu.WelcomeLabel1=Üdvözöl a [name] helyreállító varázslója
hu.WelcomeLabel2=Ez a varázsló a [name] esetleges hibáit fogja kijavítani.%n%nAjánlott mindem más futó programot bezárni a mûvelet folytatása elõtt.
chs.WelcomeLabel1=»¶Ó­Ê¹ÓÃ [name] ÐÞ¸´Ïòµ¼
chs.WelcomeLabel2=´Ë²Ù×÷½«ÐÞ¸´ÄúµçÄÔÉÏµÄ [name]¡£%n%n½¨ÒéÄúÔÚ¼ÌÐø´Ë²Ù×÷Ç°¹Ø±ÕËùÓÐÆäËû³ÌÐò¡£
sv.WelcomeLabel1=Welcome to the [name] Repair Wizard
sv.WelcomeLabel2=This will repair [name] on your computer.%n%nIt is recommended that you close all other applications before continuing.
fi.WelcomeLabel1=Tervetuloa [name] korjaustoimintoon
fi.WelcomeLabel2=Tämä korjaa [name] asennuksen tietokoneellasi.%n%nOn suositeltavaa että suljet ylimääräiset ohjelmat ennen kuin jatkat asennusta.
heb.WelcomeLabel1=ìàùó äúé÷åï [name] áøåê äáà
heb.WelcomeLabel2=òì äîçùá ùìê [name] äàùó éú÷ï àú.%n%nîåîìõ ùúñâåø àú ëì äúåëðéåú äàçøåú ìôðé ùúîùéê

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
Name: "fi"; MessagesFile: "compiler:Default.isl,setup\isl\finnish.isl"
Name: "heb"; MessagesFile: "compiler:Default.isl,setup\isl\hebrew.isl"

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
