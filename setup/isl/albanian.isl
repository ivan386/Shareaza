; ******************************************************
; ***                                                ***
; *** Inno Setup version 5.1.11+ Albanian messages   ***
; ***                                                ***
; *** Original Author:                               ***
; ***                                                ***
; ***   Besmir Godole (bgodole@gmail.com)            ***
; ***                                                ***
; ******************************************************
;
; To download user-contributed translations of this file, go to:
;   http://www.jrsoftware.org/is3rdparty.php
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).

[LangOptions]
; The following three entries are very important. Be sure to read and 
; understand the '[LangOptions] section' topic in the help file.
LanguageName=Albanian
LanguageID=$041C
LanguageCodePage=1250
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
;DialogFontName=
;DialogFontSize=8
;WelcomeFontName=Verdana
;WelcomeFontSize=12
;TitleFontName=Arial
;TitleFontSize=29
;CopyrightFontName=Arial
;CopyrightFontSize=8

[Messages]

; *** Application titles
SetupAppTitle=Ndërtoni
SetupWindowTitle=Ndërtoni - %1
UninstallAppTitle=Çinstaloni
UninstallAppFullTitle=Çinstaloni %1

; *** Misc. common
InformationTitle=Informacion
ConfirmTitle=Miratimi
ErrorTitle=Gabim

; *** SetupLdr messages
SetupLdrStartupMessage=Kjo do të instalojë %1. A doni të vijoni?
LdrCannotCreateTemp=Nuk mund të krijoj një skedë të përkohshme. Ndërtimi u ndërpre
LdrCannotExecTemp=Nuk mund të ekzekutoj skedën në direktorinë e përkohshme. Ndërtimi u ndërpre

; *** Startup error messages
LastErrorMessage=%1.%n%nGabim %2: %3
SetupFileMissing=Skeda %1 mungon nga direktori e instalimit. Ju lutemi korrigjoni problemin ose merrni një kopje të re të programit.
SetupFileCorrupt=Skedat e ndërtimit janë korruptuar. Ju lutemi merrni një kopje të re të programit.
SetupFileCorruptOrWrongVer=Skedat e ndërtimit janë korruptuar, ose nuk pajtohen me këtë version të Ndërtimit. Ju lutemi korrigjoni problemin ose merrni një kopje të re të programit.
NotOnThisPlatform=Ky program nuk do të veprojë në %1.
OnlyOnThisPlatform=Ky program duhet të veprojë në %1.
OnlyOnTheseArchitectures=Ky program mund të instalohet vetëm në versionet e Windows të modeluar me arkitekturën e procesorëve më poshtë:%n%n%1
MissingWOW64APIs=Versioni i Windows ku po punoni nuk e përfshin funksionin e kërkuar nga Ndërtimi për të kryer një instalim 64-bit. Që të korrigjoni problemin, ju lutemi instaloni Service Pack %1.
WinVersionTooLowError=Ky program kërkon %1 version %2 e më tej.
WinVersionTooHighError=Ky program nuk mund të instalohet në %1 version %2 e më tej.
AdminPrivilegesRequired=Ju duhet të hyni si administrues kur instaloni këtë program.
PowerUserPrivilegesRequired=Ju duhet të hyni si administrues ose si anëtar i grupit Përdorues në Fuqi kur instaloni këtë program.
SetupAppRunningError=Ndërtimi diktoi se %1 po vepron aktualisht.%n%nJu lutemi mbyllni tani gjithë rastet e tij, pastaj klikoni OK për të vijuar, ose Anulo që të dilni.
UninstallAppRunningError=Çinstalimi diktoi se %1 po vepron aktualisht.%n%nJu lutem mbyllni tani gjithë rastet e tij, pastaj klikoni OK për të vijuar, ose Anulo që të dilni.

; *** Misc. errors
ErrorCreatingDir=Ndërtimi nuk arriti të krijojë direktorinë "%1"
ErrorTooManyFilesInDir=Nuk mund të krijoj një skedë në direktorinë "%1" sepse përmban shumë skeda

; *** Setup common messages
ExitSetupTitle=Dilni nga Ndërtimi
ExitSetupMessage=Ndërtimi nuk ka përfunduar. Nëse dilni tani, programi nuk do të instalohet.%n%nJu mund ta ngarkoni përsëri një herë tjetër që instalimi të përfundojë.%n%nDoni të dilni nga Ndërtimi?
AboutSetupMenuItem=&Mbi Ndërtimin...
AboutSetupTitle=Mbi Ndërtimin
AboutSetupMessage=%1 version %2%n%3%n%n%1 faqe zyrtare:%n%4
AboutSetupNote=
TranslatorNote=Albanian translation by Besmir Godole (bgodole@gmail.com)

; *** Buttons
ButtonBack=< &Pas
ButtonNext=&Tjetër >
ButtonInstall=&Instalo
ButtonOK=OK
ButtonCancel=Anulo
ButtonYes=&Po
ButtonYesToAll=Po, &Gjithçka
ButtonNo=&Jo
ButtonNoToAll=J&o, Asgjë
ButtonFinish=&Përfundo
ButtonBrowse=&Shfleto...
ButtonWizardBrowse=S&hfleto...
ButtonNewFolder=&Krijo Dosje të Re

; *** "Select Language" dialog messages
SelectLanguageTitle=Zgjidhni Gjuhën e Ndërtimit
SelectLanguageLabel=Zgjidhni gjuhën që do të përdorni gjatë instalimit:

; *** Common wizard text
ClickNext=Klikoni Tjetër për të vijuar, ose Anulo që të dilni nga Ndërtimi.
BeveledLabel=
BrowseDialogTitle=Shfletoni Dosjen
BrowseDialogLabel=Zgjidhni një dosje në këtë listë, pastaj klikoni OK.
NewFolderName=Dosje e Re

; *** "Welcome" wizard page
WelcomeLabel1=Mirë se erdhët në Uizardin Ndërtues të [name]
WelcomeLabel2=[name/ver] do të instalohet në kompjuter.%n%nPara se të vijoni rekomandohet të mbyllni aplikimet e tjera.

; *** "Password" wizard page
WizardPassword=Fjalëkalimi
PasswordLabel1=Ky instalim mbrohet me fjalëkalim.
PasswordLabel3=Ju lutemi shkruani fjalëkalimin, pastaj klikoni Tjetër për të vijuar. Fjalëkalimet janë rast i ndjeshëm.
PasswordEditLabel=&Fjalëkalimi:
IncorrectPassword=Fjalëkalimi që shkruat nuk është korrekt. Ju lutemi provoni përsëri.

; *** "License Agreement" wizard page
WizardLicense=Marrëveshje License
LicenseLabel=Ju lutemi lexoni informacionin e rëndësishëm më poshtë para se të vijoni.
LicenseLabel3=Ju lutemi lexoni Marrëveshjen e Licensës më poshtë. Para se të vijoni, duhet të pranoni termat e kësaj marrëveshjeje.
LicenseAccepted=Unë &pranoj marrëveshjen
LicenseNotAccepted=Unë &nuk pranoj marrëveshjen

; *** "Information" wizard pages
WizardInfoBefore=Informacion
InfoBeforeLabel=Ju lutemi lexoni informacionin e rëndësishëm më poshtë para se të vijoni.
InfoBeforeClickLabel=Kur të jeni gati për të vijuar me Ndërtimin, klikoni Tjetër.
WizardInfoAfter=Informacion
InfoAfterLabel=Ju lutemi lexoni informacionin e rëndësishëm më poshtë para se të vijoni.
InfoAfterClickLabel=Kur të jeni gati për të vijuar me Ndërtimin, klikoni Tjetër.

; *** "User Information" wizard page
WizardUserInfo=Informacion Përdoruesi
UserInfoDesc=Ju lutemi shkruani informacionin tuaj.
UserInfoName=&Përdoruesi:
UserInfoOrg=&Organizata:
UserInfoSerial=Numri i &Serisë:
UserInfoNameRequired=Duhet të shkruani një emër.

; *** "Select Destination Location" wizard page
WizardSelectDir=Zgjidhni Vend-Destinacionin
SelectDirDesc=Ku doni të instaloni [name]?
SelectDirLabel3=Ndërtimi do të instalojë [name] në këtë dosje.
SelectDirBrowseLabel=Për të vijuar, klikoni Tjetër. Nëse doni të zgjidhni një dosje ndryshe, klikoni Shfleto.
DiskSpaceMBLabel=Kërkohet jo më pak se [mb] MB hapësirë e lirë e diskut.
InvalidPath=Ju duhet të shkruani shtegun e plotë me shkronjën e diskut; për shembull:%n%nC:\APP%n%nose një shteg UNC sipas formës:%n%n\\server\share
InvalidDrive=Disku ose shpërndarësi UNC që zgjodhët nuk ekziston ose nuk mund të hapet. Ju lutemi zgjidhni një tjetër.
DiskSpaceWarningTitle=Hapësirë Disku e Pamjaftueshme
DiskSpaceWarning=Ndërtimi kërkon jo më pak se %1 KB hapësirë të lirë që të instalojë, por disku i zgjedhur ka vetëm %2 KB të vlefshëm.%n%nGjithsesi, a doni të vijoni?
DirNameTooLong=Emri ose shtegu i dosjes është tepër i gjatë.
InvalidDirName=Emri i dosjes nuk është i vlefshëm.
BadDirName32=Emrat e dosjes nuk mund të përmbajnë asnjë nga këto shkronja:%n%n%1
DirExistsTitle=Dosja Ekziston
DirExists=Dosja:%n%n%1%n%nekziston që më parë. Gjithsesi, a doni të instaloni në këtë dosje?
DirDoesntExistTitle=Dosja Nuk Ekziston
DirDoesntExist=Dosja:%n%n%1%n%nnuk ekziston. A doni të krijoni dosjen?

; *** "Select Components" wizard page
WizardSelectComponents=Zgjidhni Përbërësit
SelectComponentsDesc=Cilët përbërës doni të instaloni?
SelectComponentsLabel2=Zgjidhni përbërësit që doni të instaloni; pastroni përbërësit që nuk doni të instaloni. Klikoni Tjetër kur të jeni gati të vijoni.
FullInstallation=Instalim i Plotë
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=Instalim Kompakt
CustomInstallation=Instalim Porositës
NoUninstallWarningTitle=Përbërësit Ekzistojnë
NoUninstallWarning=Ndërtimi diktoi se këta përbërës janë instaluar që më parë në kompjuter:%n%n%1%n%nLëshimi i këtyre përbërësve nuk do të çinstalojë ata.%n%nGjithsesi, a doni të vijoni?
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceMBLabel=Zgjedhja aktuale kërkon jo më pak se [mb] MB hapësirë disku.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=Zgjidhni Detyrat Shtesë
SelectTasksDesc=Cilat detyra shtesë doni të kryeni?
SelectTasksLabel2=Zgjidhni detyrat shtesë që doni të kryejë Ndërtimi gjatë instalimit të [name], pastaj klikoni Tjetër.

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=Zgjidhni Dosjen në Menunë Start
SelectStartMenuFolderDesc=Ku duhet të vendosë Ndërtimi shkurtesat e programit?
SelectStartMenuFolderLabel3=Ndërtimi do të krijojë shkurtesat e programit në këtë dosje të Menusë Start.
SelectStartMenuFolderBrowseLabel=Për të vijuar, klikoni Tjetër. Nëse doni të zgjidhni një dosje ndryshe, klikoni Shfleto.
MustEnterGroupName=Duhet të shkruani emrin e dosjes.
GroupNameTooLong=Emri ose shtegu i dosjes është tepër i gjatë.
InvalidGroupName=Emri i dosjes është i pavlefshëm.
BadGroupName=Emri i dosjes nuk mund të përmbajë asnjë nga këto shkronja:%n%n%1
NoProgramGroupCheck2=&Mos krijo Dosje në Menunë Start

; *** "Ready to Install" wizard page
WizardReady=Gati të Instaloni
ReadyLabel1=Ndërtimi tani është gati të nisë të instalojë [name] në kompjuter.
ReadyLabel2a=Klikoni Instalo për të vijuar me instalimin, ose klikoni Pas që të rishikoni apo ndryshoni ndonjë kuadro.
ReadyLabel2b=Klikoni Instalo për të vijuar me instalimin.
ReadyMemoUserInfo=Informacion i Përdoruesit:
ReadyMemoDir=Vend-Destinacioni:
ReadyMemoType=Lloji i Ndërtimit:
ReadyMemoComponents=Përbërësit e Zgjedhur:
ReadyMemoGroup=Dosja në Menunë Start:
ReadyMemoTasks=Detyra shtesë:

; *** "Preparing to Install" wizard page
WizardPreparing=Përgatitje për Instalim
PreparingDesc=Ndërtimi po përgatitet të instalojë [name] në kompjuter.
PreviousInstallNotCompleted=Instalimi/heqja e një programi të mëparshëm nuk përfundoi. Ju duhet të rinisni kompjuterin që të plotësoni instalimin.%n%nPas rinisjes së kompjuterit, ngarkoni përsëri Ndërtimin që të plotësoni instalimin e [name].
CannotContinue=Ndërtimi nuk mund të vijojë. Ju lutemi klikoni Anulo që të dilni.

; *** "Installing" wizard page
WizardInstalling=Po Instaloj
InstallingLabel=Ju lutemi prisni ndërkohë që Ndërtimi instalon [name] në kompjuter.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=Po Plotësoj Uizardin Ndërtues të [name]
FinishedLabelNoIcons=Ndërtimi përfundoi së instaluari [name] në kompjuter.
FinishedLabel=Ndërtimi përfundoi së instaluari [name] në kompjuter. Aplikimi mund të lëshohet duke zgjedhur ikonat e instaluara.
ClickFinish=Klikoni Përfundo për të dalë nga Ndërtimi.
FinishedRestartLabel=Për të plotësuar instalimin e [name], Ndërtimi duhet të rinisë kompjuterin. A doni të rinisni tani?
FinishedRestartMessage=Për të plotësuar instalimin e [name], Ndërtimi duhet të rinisë kompjuterin.%n%nA doni të rinisni tani?
ShowReadmeCheck=Po, dua të shikoj skedën README
YesRadio=&Po, rinise kompjuterin tani
NoRadio=&Jo, do e nis kompjuterin më vonë
; used for example as 'Run MyProg.exe'
RunEntryExec=Vepro %1
; used for example as 'View Readme.txt'
RunEntryShellExec=Shiko %1

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=Ndërtimit i Nevojitet Disku Tjetër
SelectDiskLabel2=Ju lutemi vendosni Diskun %1 dhe klikoni OK.%n%nNëse skedat në këtë disk mund të gjenden në një dosje ndryshe nga ajo që shfaqet më poshtë, shkruani shtegun korrekt ose klikoni Shfleto.
PathLabel=&Shtegu:
FileNotInDir2=Skeda "%1" nuk mund të gjendej në "%2". Ju lutemi vendosni diskun korrekt ose zgjidhni një dosje tjetër.
SelectDirectoryLabel=Ju lutemi përcaktoni vendin e diskut tjetër.

; *** Installation phase messages
SetupAborted=Ndërtimi nuk përfundoi.%n%nJu lutemi korrigjoni problemin dhe ngarkoni përsëri Ndërtimin.
EntryAbortRetryIgnore=Klikoni Riprovo për të provuar përsëri, Përbuze që gjithsesi të vijoni, ose Ndërprite që të anuloni instalimin.

; *** Installation status messages
StatusCreateDirs=Po krijoj direktoritë...
StatusExtractFiles=Po nxjerr skedat...
StatusCreateIcons=Po krijoj shkurtesat...
StatusCreateIniEntries=Po krijoj hyrjet INI...
StatusCreateRegistryEntries=Po krijoj hyrjet e regjistrit...
StatusRegisterFiles=Po regjistroj skedat...
StatusSavingUninstall=Po ruaj informacionin e çinstalimit...
StatusRunProgram=Po përfundoj instalimin...
StatusRollback=Po kthej ndryshimet e bëra...

; *** Misc. errors
ErrorInternal2=Gabim i brendshëm: %1
ErrorFunctionFailedNoCode=%1 dështoi
ErrorFunctionFailed=%1 dështoi; kodi %2
ErrorFunctionFailedWithMessage=%1 dështoi; kodi %2.%n%3
ErrorExecutingProgram=Nuk mund të ekzekutoj skedën:%n%1

; *** Registry errors
ErrorRegOpenKey=Gabim gjatë hapjes së kodit në regjistër:%n%1\%2
ErrorRegCreateKey=Gabim gjatë krijimit të kodit në regjistër:%n%1\%2
ErrorRegWriteKey=Gabim gjatë krijimit të kodit në regjistër:%n%1\%2

; *** INI errors
ErrorIniEntry=Gabim gjatë krijimit të hyrjes INI në skedën "%1".

; *** File copying errors
FileAbortRetryIgnore=Klikoni Riprovo që të provoni përsëri, Përbuze që të kaloni skedën (nuk rekomandohet), ose Ndërprite që të anuloni instalimin.
FileAbortRetryIgnore2=Klikoni Riprovo për të provuar përsëri, Përbuze që gjithsesi të vijoni (nuk rekomandohet), ose Ndërprite që të anuloni instalimin.
SourceIsCorrupted=Skeda burimore është korruptuar
SourceDoesntExist=Skeda burimore "%1" nuk ekziston
ExistingFileReadOnly=Skeda ekzistuese është e shenjuar vetëm për lexim.%n%nKlikoni Riprovo që të hiqni atributin vetëm për lexim dhe provoni përsëri, Përbuze që të kaloni skedën, ose Ndërprite që të anuloni instalimin.
ErrorReadingExistingDest=Ndodhi një gabim gjatë provës për të lexuar skedën ekzistuese:
FileExists=Skeda ekziston që më parë.%n%nA doni që Ndërtimi të mbishkruajë atë?
ExistingFileNewer=Skeda ekzistuese është më e re sesa ajo që Ndërtimi po provon të instalojë. Rekomandohet që të mbani skedën ekzistuese.%n%nA doni të mbani skedën ekzistuese?
ErrorChangingAttr=Ndodhi një gabim gjatë provës për të ndryshuar atributet e skedës ekzistuese:
ErrorCreatingTemp=Ndodhi një gabim gjatë provës për të krijuar një skedë në direktorinë destinacion:
ErrorReadingSource=Ndodhi një gabim gjatë provës për të lexuar skedën burimore:
ErrorCopying=Ndodhi një gabim gjatë provës për të kopjuar skedën:
ErrorReplacingExistingFile=Ndodhi një gabim gjatë provës për të zëvendësuar skedën ekzistuese:
ErrorRestartReplace=RinisjeZëvendësimo dështoi:
ErrorRenamingTemp=Ndodhi një gabim gjatë provës për të riemërtuar skedën në direktorinë destinacion:
ErrorRegisterServer=Nuk mund të regjistroj DLL/OCX: %1
ErrorRegSvr32Failed=RegSvr32 dështoi me kod dalje %1
ErrorRegisterTypeLib=Nuk mund të regjistroj librarinë e llojit: %1

; *** Post-installation errors
ErrorOpeningReadme=Ndodhi një gabim gjatë provës për të hapur skedën README.
ErrorRestartingComputer=Ndërtimi nuk mundi të rinisë kompjuterin. Ju lutemi bëjeni vetë.

; *** Uninstaller messages
UninstallNotFound=Skeda "%1" nuk ekziston. Nuk mund të çinstaloj.
UninstallOpenError=Skeda "%1" nuk mund të hapet. Nuk mund të çinstaloj
UninstallUnsupportedVer=Skedë-ditari i çinstalimit "%1" është në format të panjohur nga ky version i çinstaluesit. Nuk mund të çinstaloj
UninstallUnknownEntry=Një hyrje e panjohur (%1) u ndesh në ditarin e çinstalimit
ConfirmUninstall=A jeni i sigurt se doni të hiqni plotësisht %1 dhe gjithë përbërësit e tij?
UninstallOnlyOnWin64=Ky instalim mund të çinstalohet vetëm në Windows 64-bit.
OnlyAdminCanUninstall=Ky instalim mund të çinstalohet vetëm nga një përdorues me privilegje administrimi.
UninstallStatusLabel=Ju lutemi prisni ndërkohë që %1 hiqet nga kompjuteri.
UninstalledAll=%1 u hoq me sukses nga kompjuteri.
UninstalledMost=%1 përfundoi së çinstaluari.%n%nDisa elemente nuk mund të hiqeshin. Këto mund të hiqen nga ju.
UninstalledAndNeedsRestart=Që të plotësoni çinstalimin e %1, kompjuteri duhet të rinisë.%n%nA doni të rinisni tani?
UninstallDataCorrupted="%1" është skedë e korruptuar. Nuk mund të çinstaloj

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=Doni të Hiqni Skedat e Shpërndara?
ConfirmDeleteSharedFile2=Sistemi tregon se kjo skedë e shpërndarë nuk përdoret më nga asnjë program. A doni që Çinstalimi të heqë këtë skedë të shpërndarë?%n%nNëse ndonjë program ende po përdor skedën, kur ajo të hiqet programi mund të mos punojë siç duhet. Nëse jeni i pasigurt, zgjidhni Jo. Lënia e skedën në sistem nuk do të shkaktojë dëme.
SharedFileNameLabel=Emri i skedës:
SharedFileLocationLabel=Vendi:
WizardUninstalling=Statusi i Çinstalimit
StatusUninstalling=Po çinstaloj %1...

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.

[CustomMessages]

NameAndVersion=%1 version %2
AdditionalIcons=Ikona shtesë:
CreateDesktopIcon=Krijo ikonë në &desktop
CreateQuickLaunchIcon=Krijo ikonë në &Quick Launch
ProgramOnTheWeb=%1 në Rrjet
UninstallProgram=Çinstalo %1
LaunchProgram=Lësho %1
AssocFileExtension=&Shoqëro %1 me shtojcën e skedës %2
AssocingFileExtension=Po shoqëroj %1 me shtojcën e skedës %2...

components_plugins=Spina
components_skins=Veshje
tasks_languages=Shumë gjuhë
tasks_allusers=Tërë përdoruesit
tasks_selectusers=Instalo %1 për:
tasks_currentuser=%1 vetëm
tasks_multisetup=Lejo përkrahjen më shumë përdorues
tasks_firewall=Shto një përjashtim te Fajruolli i Windows-it
tasks_upnp=Lejo zbulimin e pajisjeve UPnP
tasks_deleteoldsetup=Fshiji instaluesit e vjetër
tasks_resetdiscoveryhostcache=Rivendos Zbulimin dhe Depon e Hosteve
run_skinexe=Vepron instalimi i veshjes...
reg_incomingchat=Mesazh hyrës në chat
reg_apptitle=Shpërndarja më e Lartë e Skedave me Shareaza
icons_license=Licencë
icons_uninstall=Çinstalo
icons_downloads=Shkarkesa
icons_basicmode=Mënyrë Normale
icons_tabbedmode=Mënyrë me Fusha
icons_windowedmode=Mënyrë me Dritare
dialog_shutdown=%1 aktualisht po vepron. A do e mbyllësh %1, që të vijosh me instalimin?
dialog_firewall=Sistemimi dështoi ta shtojë Shareaza-n te Fajruolli i Windows-it.%nTë lutem shtoje vetë Shareaza-n te lista e përjashtimeve.
dialog_malwaredetected=Te sistemi yt në %1 diktohet një keqbërës. Të lutem fshije atë me një skanues virusi/keqbërësi para se të instalohet Shareaza. Do e mbyllësh tani?
page_viruswarning_text=Kur përdor internetin duhet të sigurohesh që ke një skanues virusi të ridatuar që të mbrohesh nga trojanët, krimbat dhe programet e tjera keqbërëse. Nëse ndjek këtë link mund të gjesh një listë me skanuesa virusi të mirë dhe këshilla të tjera sigurie se si mbrohet kompjuteri:
page_viruswarning_title=Lajmërim Virusi
page_viruswarning_subtitle=A ke instaluar një program AntiVirus?
PathNotExist=Gabim, shtegu i dosjes së %1 nuk ekziston!
