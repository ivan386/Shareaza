; This sub-script defines all languages to be compiled
; WARNING: Do not change this file's encoding

[Languages]
; Use compiler's built in ISL file to patch up holes in ISL collection and specify localized license files
; Note: first language specified is default > English

Name: "en"; MessagesFile: "compiler:Default.isl"; LicenseFile: "setup/license/default.rtf"
Name: "nl"; MessagesFile: "compiler:Languages\dutch.isl"; LicenseFile: "setup/license/dutch.rtf"
Name: "lt"; MessagesFile: "setup\isl\lithuanian.isl"; LicenseFile: "setup/license/lithuanian.rtf"
Name: "de"; MessagesFile: "compiler:Languages\german.isl"; LicenseFile: "setup/license/German.rtf"
Name: "pt"; MessagesFile: "compiler:Languages\Portuguese.isl"; LicenseFile: "setup/license/portuguese-braz.rtf"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl"; LicenseFile: "setup/license/italian.rtf"
Name: "no"; MessagesFile: "compiler:Languages\norwegian.isl"; LicenseFile: "setup/license/default.rtf"
Name: "af"; MessagesFile: "setup\isl\afrikaans.isl"; LicenseFile: "setup/license/afrikaans.rtf"
Name: "br"; MessagesFile: "setup\isl\portuguese-braz.isl"; LicenseFile: "setup/license/portuguese-braz.rtf"
Name: "fr"; MessagesFile: "compiler:Languages\french.isl"; LicenseFile: "setup/license/default.rtf"
Name: "es"; MessagesFile: "setup\isl\spanish.isl"; LicenseFile: "setup/license/spanish.rtf"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"; LicenseFile: "setup/license/russian.rtf"
Name: "gr"; MessagesFile: "setup\isl\greek.isl"; LicenseFile: "setup/license/greek.rtf"
Name: "hu"; MessagesFile: "compiler:Languages\hungarian.isl"; LicenseFile: "setup/license/hungarian.rtf"
Name: "chs"; MessagesFile: "setup\isl\chinese-simp.isl"; LicenseFile: "setup/license/chinese.rtf"
Name: "sv"; MessagesFile: "setup\isl\swedish.isl"; LicenseFile: "setup/license/swedish.rtf"
Name: "fi"; MessagesFile: "compiler:Languages\Finnish.isl"; LicenseFile: "setup/license/finnish.rtf"
Name: "heb"; MessagesFile: "setup\isl\hebrew.isl"; LicenseFile: "setup/license/hebrew.rtf"
Name: "pl"; MessagesFile: "compiler:Languages\Polish.isl"; LicenseFile: "setup/license/polish.rtf"
Name: "sr"; MessagesFile: "setup\isl\Serbian.isl"; LicenseFile: "setup/license/serbian.rtf"
Name: "tr"; MessagesFile: "setup\isl\turkish.isl"; LicenseFile: "setup/license/turkish.rtf"
Name: "jp"; MessagesFile: "setup\isl\japanese.isl"; LicenseFile: "setup/license/japanese.rtf"
Name: "ar"; MessagesFile: "setup\isl\arabic.isl"; LicenseFile: "setup/license/default.rtf"
;Name: "cz"; MessagesFile: "compiler:Languages\Czech.isl"; LicenseFile: "setup/license/czech.rtf"

[Files]
#ifndef debug
; Install default remote
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: not language
; Install localized remote
; English
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: en; Components: language
; Dutch
Source: "Remote\nl\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: nl; Components: language
; Lithuanian
Source: "Remote\lt\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: lt; Components: language
; German
Source: "Remote\de\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: de; Components: language
; Portuguese std
Source: "Remote\pt\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt; Components: language
; Italian
Source: "Remote\it\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: it; Components: language
; Norwegian
Source: "Remote\no\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: no; Components: language
; Afrikaans
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: af; Components: language
; Portuguese braz
Source: "Remote\pt\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: br; Components: language
; French
Source: "Remote\fr\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fr; Components: language
; Spanish
Source: "Remote\es\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es; Components: language
; Russian
Source: "Remote\ru\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ru; Components: language
; Greek
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: gr; Components: language
; Hungarian
Source: "Remote\hu\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: hu; Components: language
; Chinese Simp
Source: "Remote\chs\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: chs; Components: language
; Swedish
Source: "Remote\sv\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sv; Components: language
; Finnish
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fi; Components: language
; Hebrew
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: heb; Components: language
; Polish
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pl; Components: language
; Czech
;Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: cz; Components: language
; Serbian
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sr; Components: language
; Turkish
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tr; Components: language
; Japanese
Source: "Remote\jp\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: jp; Components: language
; Arabic
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ar; Components: language

; Install default license
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: not language
; Install localized license
; English
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: en; Components: language
; Dutch
Source: "setup\license\dutch.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: nl; Components: language
; Lithuanian
Source: "setup\license\lithuanian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: lt; Components: language
; German
Source: "setup\license\german.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: de; Components: language
; Portuguese std
Source: "setup\license\portuguese-braz.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt; Components: language
; Italian
Source: "setup\license\italian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: it; Components: language
; Norwegian
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: no; Components: language
; Afrikaans
Source: "setup\license\afrikaans.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: af; Components: language
; Portuguese braz
Source: "setup\license\portuguese-braz.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: br; Components: language
; French
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fr; Components: language
; Spanish
Source: "setup\license\spanish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es; Components: language
; Russian
Source: "setup\license\russian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ru; Components: language
; Greek
Source: "setup\license\greek.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: gr; Components: language
; Hungarian
Source: "setup\license\hungarian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: hu; Components: language
; Chinese Simp
Source: "setup\license\chinese.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: chs; Components: language
; Swedish
Source: "setup\license\swedish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sv; Components: language
; Finnish
Source: "setup\license\finnish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fi; Components: language
; Hebrew
Source: "setup\license\hebrew.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: heb; Components: language
; Polish
Source: "setup\license\polish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pl; Components: language
; Czech
;Source: "setup\license\czech.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: cz; Components: language
; Serbian
Source: "setup\license\serbian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sr; Components: language
; Turkish
Source: "setup\license\turkish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tr; Components: language
; Japanese
Source: "setup\license\japanese.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: jp; Components: language
; Arabic
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ar; Components: language

; Install default filter
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: not language
; Install localized filter
; English
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: en; Components: language
; Dutch
Source: "setup\filter\dutch.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: nl; Components: language
; Lithuanian
Source: "setup\filter\lithuanian.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: lt; Components: language
; German
Source: "setup\filter\german.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: de; Components: language
; Portuguese std
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt; Components: language
; Italian
Source: "setup\filter\italian.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: it; Components: language
; Norwegian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: no; Components: language
; Afrikaans
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: af; Components: language
; Portuguese braz
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: br; Components: language
; French
Source: "setup\filter\french.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fr; Components: language
; Spanish
Source: "setup\filter\spanish.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es; Components: language
; Russian
Source: "setup\filter\russian.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ru; Components: language
; Greek
Source: "setup\filter\greek.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: gr; Components: language
; Hungarian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: hu; Components: language
; Chinese Simp
Source: "setup\filter\chinese-simpl.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: chs; Components: language
; Swedish
Source: "setup\filter\swedish.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sv; Components: language
; Finnish
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fi; Components: language
; Hebrew
Source: "setup\filter\hebrew.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: heb; Components: language
; Polish
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pl; Components: language
; Czech
;Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: cz; Components: language
; Serbian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sr; Components: language
; Turkish
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tr; Components: language
; Japanese
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: jp; Components: language
; Arabic
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ar; Components: language
#endif

[CustomMessages]
; This section specifies phrazes and words not specified in the ISL files
; Avoid customizing the ISL files since they will change with each version of Inno Setup.
; English
components_plugins=Plugins
components_skins=Skins
components_languages=Languages
tasks_allusers=All users
tasks_selectusers=Install %1 for:
tasks_currentuser=%1 only
tasks_multisetup=Enable multi-user support
tasks_firewall=Add an exception to the Windows Firewall
tasks_deleteoldsetup=Delete old installers
run_skinexe=Running skin installation...
reg_incomingchat=Incoming chat message
reg_apptitle=Shareaza Ultimate File Sharing
icons_license=License
icons_uninstall=Uninstall
icons_downloads=Downloads
icons_basicmode=Normal Mode
icons_tabbedmode=Tabbed Mode
icons_windowedmode=Windowed Mode
dialog_shutdown=%1 is currently running. Would you like %1 to be shutdown so the installation can continue?
dialog_firewall=Setup failed to add Shareaza to the Windows Firewall.%nPlease add Shareaza to the exception list manually.
page_viruswarning_text=When using the internet, you should always ensure you have an up-to-date virus scanner to protect you from trojans, worms, and other malicious programs. You can find list of good virus scanners and other security tips to protect your computer by following this link:
page_viruswarning_title=Virus Warning
page_viruswarning_subtitle=Do you have an AntiVirus program installed?
CreateDesktopIcon=Display a &desktop icon
CreateQuickLaunchIcon=Display a &Quick Launch icon
page_viruswarning_link=http://www.shareaza.com/securityhelp
page_viruswarning_destination=http://www.shareaza.com/securityhelp/
; Don't copy these last 2 messages, they are just links.
; Dutch
nl.components_plugins=Plugins
nl.components_skins=Skins
nl.components_languages=Talen
nl.tasks_allusers=Alle gebruikers
nl.tasks_selectusers=Installeer %1 voor:
nl.tasks_currentuser=Aleen %1
nl.tasks_multisetup=Ondersteuning voor meerdere gebruikers inschakelen
nl.tasks_firewall=Een uitzondering aan de Windows Firewall toevoegen
nl.tasks_deleteoldsetup=Oude installatieprogramma's wissen
nl.run_skinexe=Skin installatie uitvoeren...
nl.reg_incomingchat=Nieuw chat bericht
nl.reg_apptitle=Shareaza Ultimate File Sharing
nl.icons_license=Gebruiksovereenkomst
nl.icons_uninstall=Verwijderen
nl.icons_downloads=Downloads
nl.dialog_shutdown=%1 is momenteel open. Wil je %1 afsluiten zodat de installatie verder kan gaan?
nl.dialog_firewall=Het installatieprogramma kon Shareaza niet toevoegen aan de Windows Firewall.%nVoeg Shareaza alstublieft manueel toe aan de uitzonderingenlijst.
nl.page_viruswarning_text=Als u het internet gebruikt moet u een recente virusscanner gebruiken om u te beschermen tegen virussen, wormen en andere kwaadaardige programma's. U kan een lijst van virusscanners en andere veiligheidstips vinden om uw computer the beschermen door deze link te volgen:
nl.page_viruswarning_title=Virus Waarschuwing
nl.page_viruswarning_subtitle=Heeft u een antivirus programma geïnstalleerd?
nl.CreateDesktopIcon=Toon een snelkoppeling op het &bureaublad
nl.CreateQuickLaunchIcon=Toon een snelkoppeling op de &Snel Starten werkbalk
; Lithuanian
lt.components_plugins=Papildiniai
lt.components_skins=Apvalkalai
lt.components_languages=Kalbos
lt.tasks_allusers=Visiems vartotojams
lt.tasks_selectusers=Ádiegti „%1“ ğiems vartotojams:
lt.tasks_currentuser=Tik vartotojui %1
lt.tasks_multisetup=Ágalinti keliø vartotojø palaikymà
lt.tasks_firewall=Pridëti prie Windows ugniasienës iğimèiø sàrağo
lt.tasks_deleteoldsetup=Trinti senas diegimo programas
lt.run_skinexe=Vykdoma apvalkalo sàranka...
lt.reg_incomingchat=Gaunama şinutë pokalbiui
lt.reg_apptitle=Shareaza — geriausia programa bylø mainams
lt.icons_license=Licencinë sutartis
lt.icons_uninstall=Pağalinti
lt.icons_downloads=Atsisiuntimai
lt.dialog_shutdown=„%1“ ğiuo metu dirba. Ar norite nutraukti „%1“ darbà, tam kad bûtø galima tæsti ádiegimà?
lt.dialog_firewall=Ádiegimo programai nepavyko pridëti „Shareaza“ prie Windows ugniasienës iğimèiø sàrağo.%nPridëkite jà á ğá sàrağà rankiniu bûdu.
lt.page_viruswarning_text=Visada, kai naudojatës Internetu, ásitikinkite, jog turite naujausià virusø skenerá, tam kad bûtumëte apsaugoti nuo trojanø, kirminø ir kitokiø kenkëjiğkø programø. Jûs galite rasti gerø virusø skeneriø sàrağà ir kitokius kompiuterio apsaugojimo patarimus nuëjæ ğiuo adresu:
lt.page_viruswarning_title=Áspëjimas apie virusus
lt.page_viruswarning_subtitle=Ar Jûs turite ásidiegæ antivirusinæ programà?
lt.CreateDesktopIcon=Rodyti piktogramà &Darbalaukyje
lt.CreateQuickLaunchIcon=Rodyti Sparèiosios &Paleisties piktogramà
; German
de.components_plugins=Plugins
de.components_skins=Skins
de.components_languages=Sprachen
de.tasks_allusers=Alle Benutzer
de.tasks_selectusers=Installieren %1 für:
de.tasks_currentuser=Nur für %1
de.tasks_multisetup=Mehrbenutzerunterstützung aktivieren
de.tasks_firewall=Eine Ausnahme zur Windows Firewall hinzufügen
de.tasks_deleteoldsetup=Alte Installer löschen
de.run_skinexe=Skin Installer einrichten...
de.reg_incomingchat=Eingehende Chat Nachricht
de.reg_apptitle=Shareaza Ultimate File Sharing
de.icons_license=Lizenz
de.icons_uninstall=Shareaza deinstallieren
de.icons_downloads=Downloads
de.dialog_shutdown=%1 wird zur Zeit ausgeführt. Wollen Sie %1 beenden, um mit der Installation fortzufahren?
de.dialog_firewall=Setup konnte Shareaza nicht zur Windows Firewall hinzufügen.%nBitte tragen Sie Shareaza manuell in die Ausnahmenliste ein.
de.page_viruswarning_text=Wenn Sie das Internet benutzen, sollten Sie stets einen aktuellen Virenscanner installiert haben, der ihren Computer vor Trojanern, Würmern und anderen bösartigen Programmen beschützt. Sie finden eine Liste guter Virenscanner und andere Tipps, wie Sie ihren Computer schützen können, unter folgendem Link:
de.page_viruswarning_title=Virenwarnung
de.page_viruswarning_subtitle=Haben Sie ein Antivirenprogramm installiert?
de.CreateDesktopIcon=&Symbol auf dem Desktop anzeigen
de.CreateQuickLaunchIcon=&Quick Launch Symbol anzeigen
; Portuguese std
pt.components_plugins=Plugins
pt.components_skins=Peles
pt.components_languages=Linguagens
pt.tasks_allusers=Todos os usuários
pt.tasks_selectusers=Instalar %1 para:
pt.tasks_currentuser=somente %1
pt.tasks_multisetup=Acionar o suporte a vários usuários
pt.tasks_firewall=Adicionar exceção ao Firewall do Windows
pt.tasks_deleteoldsetup=Apagar instaladores antigos
pt.run_skinexe=Instalando Pele...
pt.reg_incomingchat=Mensagem de chat
pt.reg_apptitle=Shareaza Compartilhamento de Arquivos Incomparável
pt.icons_license=Licença
pt.icons_uninstall=Desintalar
pt.icons_downloads=Downloads
pt.dialog_shutdown=O %1 está sendo executado. Você gostaria que o %1 fosse fechado para que a instalação continue?
pt.dialog_firewall=Setup failed to add Shareaza to the Windows Firewall.%nPlease add Shareaza to the exception list manually.
pt.page_viruswarning_text=When using the internet, you should always ensure you have an up-to-date virus scanner to protect you from trojans, worms, and other malicious programs. You can find list of good virus scanners and other security tips to protect your computer by following this link:
pt.page_viruswarning_title=Virus Warning
pt.page_viruswarning_subtitle=Do you have an AntiVirus program installed?
pt.CreateDesktopIcon=Criar ícone no &Ambiente de Trabalho
pt.CreateQuickLaunchIcon=Criar ícone na barra de Iniciação &Rápida
; Italian
it.components_plugins=Plugins
it.components_skins=Skins
it.components_languages=Lingue
it.tasks_allusers=Tutti gli utenti
it.tasks_selectusers=Installa %1 per:
it.tasks_currentuser=Solo %1
it.tasks_multisetup=Abilita supporto multi utente
it.tasks_firewall=Aggiungi un'eccezione a Windows Firewall
it.tasks_deleteoldsetup=Cancella installer vecchi
it.run_skinexe=Sto installando le skin...
it.reg_incomingchat=Messaggio di chat in arrivo
it.reg_apptitle=Shareaza, il programma definitivo di P2P
it.icons_license=Licenza
it.icons_uninstall=Disinstalla
it.icons_downloads=Downloads
it.dialog_shutdown=%1 è attualmente in esecuzione. Vuoi che %1 sia chiuso così l'installazione può continuare?
it.dialog_firewall=Impossibile aggiungere Shareaza a Windows Firewall.%nAggiungi Shareaza alla lista delle eccezioni manualmente.
it.page_viruswarning_text=Quando usi internet, dovresti sempre assicurarti di aver un antivirus aggiornato per proteggerti dai trojan, worm e dagli altri programmi malevoli. Puoi trovare una lista di buoni antivirus e altri suggerimenti di sicurezza per proteggere il tuo computer seguendo questo link:
it.page_viruswarning_title=Attenzione ai virus
it.page_viruswarning_subtitle=Hai installato un programma antivirus?
it.CreateDesktopIcon=Visualizza un'icona sul &desktop
it.CreateQuickLaunchIcon=Visualizza un'icona in &Avvio veloce
; Norwegian
no.components_plugins=Plugins
no.components_skins=Skins
no.components_languages=Språk
no.tasks_allusers=Alle brukere
no.tasks_selectusers=Installer %1 for:
no.tasks_currentuser=Kun %1
no.tasks_multisetup=Flere brukere
no.tasks_firewall=Lag nytt unntak i Windows brannmur
no.tasks_deleteoldsetup=Slett eldre installasjonsfiler
no.run_skinexe=Kjører skin installasjon...
no.reg_incomingchat=Innkommende melding
no.reg_apptitle=Shareaza Ultimate File Sharing
no.icons_license=Lisens
no.icons_uninstall=Uninstall
no.icons_downloads=Downloads
no.dialog_shutdown=%1 kjører. Ønsker du at %1 avsluttes slik at installasjonen kan fortsette?
no.dialog_firewall=Installasjonen klarte ikke å lage unntak for Shareaza i Windows Brannmuren. %nVennligst legg til shareaza i brannmurens unntak manuelt.
no.page_viruswarning_text=Når du bruker internett bør du alltid ha et oppdatert antivirus-program, for å beskytte deg fra trojaner, ormer, og annen skadelig programvare. Du kan finne en liste over gode antivirus-prgrammer og andre sikkerhetstips, for å beskytte din datamaskin, ved å følge denne linken:
no.page_viruswarning_title=Virusadvarsel
no.page_viruswarning_subtitle=Har du et antivirus-program installert?
no.CreateDesktopIcon=Vis ikon på &skrivebordet
no.CreateQuickLaunchIcon=Vis et &Hurtigstarts-ikon
; Afrikaans
af.components_plugins=Inpropprogramme
af.components_skins=Omslagte
af.components_languages=Tale
af.tasks_allusers=Alle gebruikers van hierdie rekenaar
af.tasks_selectusers=Installeer %1 vir die volgende gebruikers:
af.tasks_currentuser=Vir %1 alleenlik
af.tasks_multisetup=Skakel ondersteuning vir veelvuldige gebruikers aan
af.tasks_firewall=Voeg 'n uitsondering by die Windows Netskans
af.tasks_deleteoldsetup=Skrap ou opstellerslêers
af.run_skinexe=Hardloop omslagte installasie...
af.reg_incomingchat=Inkomende Geselsie-boodskap
af.reg_apptitle=Shareaza Ultimate File Sharing
af.icons_license=Lisensie-ooreenkoms
af.icons_uninstall=Deïnstalleer
af.icons_downloads=Aflaaie
af.dialog_shutdown=%1 is op die oomblik besig om te loop. Wil jy graag %1 sluit sodat die installasie kan voortgaan?
af.dialog_firewall=Die Opsteller kon nie Shareaza by die Windows netskans uitsonderings voeg nie.%nVoeg Shareaza asseblief met die hand op die uitsonderingslys.
af.page_viruswarning_text=Maak altyd seker dat jy 'n opgedateerde anti-virus program geïnstalleer het wanneer jy die internet gebruik, om jou rekenaar te beskerm teen virusse, wurms, en ander ongewenste programme. Jy kan 'n lys van goeie anti-virus programme en ander sekuriteitswenke oor hoe om jou rekenaar te beskerm verkry deur die volgende skakel te volg:
af.page_viruswarning_title=Virus Waarskuwing
af.page_viruswarning_subtitle=Het jy 'n Anti-Virus program geïnstalleer?
af.CreateDesktopIcon=Vertoon 'n &werkskerm ikoon
af.CreateQuickLaunchIcon=Vertoon 'n &Quick Launch ikoon
; Portuguese braz
br.components_plugins=Plugins
br.components_skins=Peles
br.components_languages=Linguagens
br.tasks_allusers=Todos os Usuários
br.tasks_selectusers=Instalar %1 para:
br.tasks_currentuser=%1 apenas
br.tasks_multisetup=Ativar suporte para vários usuários
br.tasks_firewall=Adicionar exceção ao Firewall do Windows
br.tasks_deleteoldsetup=Apagar instaladores antigos
br.run_skinexe=Instalando as Peles...
br.reg_incomingchat=Nova mensagem no chat
br.reg_apptitle=Shareaza o Compartilhador de Arquivos Definitivo
br.icons_license=Licença
br.icons_uninstall=Desinstalar
br.icons_downloads=Downloads
br.dialog_shutdown=Você deseja fechar o %1?
br.dialog_firewall=A instalação falhou ao tentar adicionar o Shareaza à lista de exceções do Firewall do Windows.%nPor favor adicione manualmente o Shareaza à lista.
br.page_viruswarning_text=Ao usar a Internet você deve sempre manter seu Anti-Vírus atualizado, para proteger contra vírus, worms, cavalos-de-tróia e outros programas perigosos. Você encontra uma lista de bons anti-vírus e dicas de segurança entrando no seguinte endereço:
br.page_viruswarning_title=Aviso sobre Vírus
br.page_viruswarning_subtitle=Você tem um programa anti-vírus instalado?
br.CreateDesktopIcon=Mostrar um ícone na &área de trabalho
br.CreateQuickLaunchIcon=Mostrar um ícone na barra de &Inicialização Rápida
; French
fr.components_plugins=Plugins
fr.components_skins=Skins
fr.components_languages=Langues
fr.tasks_allusers=Tous les utilisateurs
fr.tasks_selectusers=Installer %1 pour:
fr.tasks_currentuser=%1 seulement
fr.tasks_multisetup=Activer le support multi-utilisateurs
fr.tasks_firewall=Ajouter une exception au Pare-feu Windows
fr.tasks_deleteoldsetup=Delete old installers
fr.run_skinexe=Installation de la skin en cours...
fr.reg_incomingchat=Réception d'un message chat
fr.reg_apptitle=Shareaza Ultimate File Sharing
fr.icons_license=Licence
fr.icons_uninstall=Désinstaller
fr.icons_downloads=Téléchargements
fr.dialog_shutdown=%1 est en cours d'exécution. Voulez-vous quitter %1 pour que l'installation puisse se poursuivre?
fr.dialog_firewall=L'installation n'a pas pu ajouter Shareaza au Pare-feu Windows.%nVeuillez ajouter Shareaza manuellement à la liste des exceptions.
fr.page_viruswarning_text=Lorsque vous utilisez internet, vous devriez toujours vous assurer que vous avez un scanner de virus à jour pour vous protéger des troyens (trojans), vers (worms), et autres programmes malveillants. Vous pouvez trouver une liste de bons antivirus et conseils de sécurité pour protéger votre ordinateur en suivant ce lien:
fr.page_viruswarning_title=Avertissement sur les virus
fr.page_viruswarning_subtitle=Avez-vous un antivirus installé?
fr.CreateDesktopIcon=Afficher un raccourci sur le &Bureau
fr.CreateQuickLaunchIcon=Afficher un raccouri dans la barre de &Lancement rapide
; Spanish
es.components_plugins=Plugins
es.components_skins=Skins
es.components_languages=Lenguages
es.tasks_allusers=Todos los usuarios
es.tasks_selectusers=Instalar %1 para:
es.tasks_currentuser=%1 solamente
es.tasks_multisetup=Habilitar soporte multi-usuario
es.tasks_firewall=Agregar una excepción al Firewall de Windows
es.tasks_deleteoldsetup=Borrar archivos de instaladores viejos
es.run_skinexe=Instalando Skin...
es.reg_incomingchat=Hay un mensaje de chat entrante
es.reg_apptitle=Shareaza Ultimate File Sharing
es.icons_license=Licencia
es.icons_uninstall=Desinstalar
es.icons_downloads=Descargas
es.dialog_shutdown=%1 se encuentra ejecutándose. ¿Deseas que %1 sea cerrado para que la instalación pueda continuar?
es.dialog_firewall=La instalación fallo al agregar la excepción de Shareaza al cortafuego Firewall.%n Por favor hágalo manualmente.
es.page_viruswarning_text=Cuando estas usando Internet, debes siempre asegurarte que tienes un antivirus actualizado hasta la fecha para protegerte de troyanos, gusanos, y otros programas maliciosos. Puedes encontrar una lista de buenos antivirus y sugerencias de seguridad para proteger tu computadora en la siguiente dirección:
es.page_viruswarning_title=Peligro de Virus
es.page_viruswarning_subtitle=¿Tienes un programa antivirus instalado?
es.CreateDesktopIcon=Mostrar/Quitar icono de &Escritorio
es.CreateQuickLaunchIcon=Mostrar/Quitar icono de &Inicio Rápido
; Russian
ru.components_plugins=Ìîäóëè
ru.components_skins=Øêóğêè
ru.components_languages=ßçûêè
ru.tasks_allusers=Âñåì ïîëüçîâàòåëÿì
ru.tasks_selectusers=Óñòàíîâèòü %1 äëÿ:
ru.tasks_currentuser=Òîëüêî äëÿ %1
ru.tasks_multisetup=Ğàçğåøèòü ïîääåğæêó íåñêîëüêèõ ïîëüçîâàòåëåé
ru.tasks_firewall=Äîáàâèòü â ñïèñîê èñêëş÷åíèé áğàíìàóığà Windows
ru.tasks_deleteoldsetup=Óäàëèòü ñòàğûå èíñòàëëÿòîğû
ru.run_skinexe=Èä¸ò óñòàíîâêà øêóğîê...
ru.reg_incomingchat=Âõîäÿùåå ñîîáùåíèå äëÿ ÷àòà
ru.reg_apptitle=Shareaza - ïğîãğàììà äëÿ îáìåíà ôàéëàìè
ru.icons_license=Ëèöåíçèÿ
ru.icons_uninstall=Äåèíñòàëëÿöèÿ
ru.icons_downloads=Çàãğóçêè
ru.dialog_shutdown=%1 â äàííûé ìîìåíò ğàáîòàåò. Õîòèòå ëè çàâåğøèòü ğàáîòó %1, ÷òîáû ïğîäîëæèòü óñòàíîâêó?
ru.dialog_firewall=Ïğîãğàììà óñòàíîâêè íå ñìîãëà äîáàâèòü Shareaza â ñïèñîê èñêëş÷åíèé áğàíäìàóığà Windows.%nÏîæàëóéñòà, äîáàâüòå åå â ıòîò ñïèñîê âğó÷íóş.
ru.page_viruswarning_text=Âñåãäà, êîãäà ïîëüçóåòåñü èíòåğíåòîì, óäîñòîâåğüòåñü, ÷òî ó Âàñ åñòü íîâåéøèé ñêàíåğ äëÿ âèğóñîâ, ÷òîáû çàùèòèòü êîìïüşòåğ îò òğîÿíîâ, ÷åğâåé è äğóãèõ âğåäèòåëüñêèõ ïğîãğàìì. Âû ìîæåòå íàéòè ñïèñîê õîğîøèõ ñêàíåğîâ äëÿ âèğóñîâ è äğóãèå ñîâåòû î çàùèòå êîìïüşòåğà ïî ıòîìó àäğåñó:
ru.page_viruswarning_title=Ïğåäîñòåğåæåíèå î âèğóñàõ
ru.page_viruswarning_subtitle=Èìååòå ëè Âû óñòàíîâëåííóş àíòèâèğóñíóş ïğîãğàììó?
ru.CreateDesktopIcon=Ïîêàçûâàòü çíà÷îê íà &Ğàáî÷åì ñòîëå
ru.CreateQuickLaunchIcon=Ïîêàçûâàòü çíà÷îê â &Ïàíåëè áûñòğîãî çàïóñêà
; Greek
gr.components_plugins=Plugins
gr.components_skins=Skins
gr.components_languages=Ãëşóóåò
gr.tasks_allusers=¼ëïé ïé ÷ñŞóôåò
gr.tasks_selectusers=ÅãêáôÜóôáóç %1 ãéá:
gr.tasks_currentuser=%1 ìüíï
gr.tasks_multisetup=Åíåñãïğïßçóç ôçò âïŞèåéáò ğïëëáğëşí ÷ñçóôşí
gr.tasks_firewall=ÂÜëå ìéá åîáßñåóç óôï ôåß÷ïò ğñïóôáóßáò ôùí Windows
gr.tasks_deleteoldsetup=Äéİãñáøå ôçí ğáëéÜ åãêáôÜóôáóç
gr.run_skinexe=Running ÅãêáôÜóôáóç ôïõ skin...
gr.reg_incomingchat=Íİï ìŞíõìá chat
gr.reg_apptitle=Shareaza Ultimate File Sharing
gr.icons_license=¢äåéá
gr.icons_uninstall=ÁğåãêáôÜóôáóç
gr.icons_downloads=Êáôİâáóìá
gr.dialog_shutdown=Ôï %1 áêüìá ôñİ÷åé. Èİëåôå íá ôåñìáôßóåôå ôçí ëåéôïõñãåßá ôïõ %1 ãéá íá óõíå÷éóôåß ç åãêáôÜóôáóç?
gr.dialog_firewall=Ç åãêáôÜóôáóç ôïõ ğñïãñÜììáôïò áğİôõ÷å íá ğñïóèİóåé ôï Shareaza óôï Windows Firewall. % Ğáñáêáëş ğñïóèİóôå ôï Shareaza óôçí exception ëßóôá ÷åéñïêßíçôá
gr.page_viruswarning_text=¼ôáí ÷ñçóéìïğïéåßôå ôï internet, èá ğñİğåé ğÜíôá íá İ÷åôå İíá ğñüãñáììá ğñïóôáóßáò ãéá éïıò åíçìåñùìİíï ãéá íá óáò ğñïóôáôåıåé áğï áõôïıò êáé Üëëá åğéêßíäõíá ğñïãñÜììáôá. Ìğïñåßôå íá âñåßôå ìéá ëßóôá ìå êáëÜ ğñïãñÜììáôá ğñïóôáóßáò ãéá éïıò êáé Üëëá âïçèŞìáôá ãéá íá ğñïóôáôİøåôå ôïí õğïëïãéóôŞ óáò áêïëïõèşíôáò áõôüí ôïí óıíäåóìï:
gr.page_viruswarning_title=Ğñïåéäïğïßçóç ãéá éü
gr.page_viruswarning_subtitle=¸÷åôå İíá ğñüãñáììá ğñïóôáóßáò ãéá éïıò åãêáôåóôçìİíï?
gr.CreateDesktopIcon=ÅìöÜíéóå ôï &åéêïíßäéï óôçí åğéöÜíåéá åñãáóßáò
gr.CreateQuickLaunchIcon=ÅìöÜíéóå İíá å&éêïíßäéï ÃñŞãïñçò Åêêßíçóçò
; Hungarian
hu.components_plugins=Pluginek
hu.components_skins=Kinézetek
hu.components_languages=Nyelvek
hu.tasks_allusers=Minden felhasználó
hu.tasks_selectusers=Megadott felhaszáló:
hu.tasks_currentuser=Jelenlegi felhasználó
hu.tasks_multisetup=Többfelhasználós mód engedélyezése
hu.tasks_firewall=Felvétel a Windows tûzfal kivételeihez
hu.tasks_deleteoldsetup=Régi telepítõk törlése
hu.run_skinexe=Kinézet telepítése folyamatban...
hu.reg_incomingchat=Bejövõ chat üzenet
hu.reg_apptitle=Shareaza fájlmegosztó
hu.icons_license=Licensz
hu.icons_uninstall=Törlés
hu.icons_downloads=Letöltések
hu.dialog_shutdown=A %1 jelenleg fut. Be akarod zárni a programot, hogy a telepítés folytatódhasson?
hu.dialog_firewall=A telepíto nem tudta hozzáadni a Shareazát a Windows tuzfal kivételeihez.%nManuálisan kell hozzáadni a kivételekhez.
hu.page_viruswarning_text=Ha az internetet használod, mindig legyen föltelepítve egy, a legfrissebb vírusadatbázissal rendelkezõ antivírus program, ami megvéd a férgektõl, trójai és egyéb kártékony programoktól. Ha követed ezt a linket, sok jó víruskeresõt találhatsz és hasznos tippeket kaphatsz a számítógép védelmérõl:
hu.page_viruswarning_title=Vírusveszély
hu.page_viruswarning_subtitle=Van feltelepített antivírus programod?
hu.CreateDesktopIcon=Ikon létrehozása az &Asztalon
hu.CreateQuickLaunchIcon=Ikon létrehozása a &Gyorsindítás eszköztáron
; Chinese Simp
chs.components_plugins=²å¼ş
chs.components_skins=Æ¤·ô
chs.components_languages=ÓïÑÔ
chs.tasks_allusers=ËùÓĞÓÃ»§
chs.tasks_selectusers=°²×° %1 Îª:
chs.tasks_currentuser=½ö %1
chs.tasks_multisetup=ÆôÓÃ¶àÓÃ»§Ö§³Ö
chs.tasks_firewall=Ìí¼ÓÒ»¸öÀıÍâµ½ Windows ·À»ğÇ½
chs.tasks_deleteoldsetup=É¾³ı¾ÉµÄ°²×°ÎÄ¼ş
chs.run_skinexe=°²×°Æ¤·ô...
chs.reg_incomingchat=´«ÈëÁÄÌìÏûÏ¢
chs.reg_apptitle=Shareaza ÖÕ¼«ÎÄ¼ş¹²Ïí
chs.icons_license=Ğí¿É
chs.icons_uninstall=Ğ¶ÔØ
chs.icons_downloads=ÏÂÔØ
chs.dialog_shutdown=%1 ÕıÔÚÔËĞĞ¡£ÄúÏ£Íû¹Ø±Õ %1 ÒÔ±ã¼ÌĞø°²×°Âğ£¿
chs.dialog_firewall=°²×°Ìí¼Ó Shareaza µ½ Windows ·À»ğÇ½Ê§°Ü¡£%nÇë½« Shareaza ÊÖ¶¯Ìí¼ÓÖÁ³ıÍâÁĞ±í¡£
chs.page_viruswarning_text=ÔÚÊÔÓÃ»¥ÁªÍøÊ±£¬ÄúĞèÒªÈ·¶¨ÄúÓµÓĞ×îĞÂµÄ²¡¶¾É¨ÃèÈí¼şÒÔ±£»¤ÄúÃâÊÜÄ¾Âí¡¢Èä³æºÍÆäËû¶ñÒâ³ÌĞòµÄÇÖº¦¡£Äú¿ÉÒÔÔÚÒÔÏÂÁ´½ÓÖĞÕÒµ½½ÏºÃµÄ²¡¶¾É¨ÃèÈí¼şµÄÁĞ±íÒÔ¼°ÆäËû±£»¤ÄúµÄ¼ÆËã»úµÄ°²È«½¨Òé:
chs.page_viruswarning_title=²¡¶¾¾¯¸æ
chs.page_viruswarning_subtitle=Äú°²×°ÁË·À²¡¶¾Èí¼şÁËÂğ£¿
chs.CreateDesktopIcon=ÏÔÊ¾×ÀÃæÍ¼±ê(&D)
chs.CreateQuickLaunchIcon=ÏÔÊ¾¿ìËÙÆô¶¯À¸Í¼±ê(&Q)
; Swedish
sv.components_skins=Skinn
sv.components_languages=Språk
sv.tasks_allusers=Alla användare
sv.tasks_selectusers=Installera %1 för:
sv.tasks_currentuser=%1 endast
sv.tasks_multisetup=Aktivera stöd för flera användare
sv.tasks_firewall=Lägg till ett undantag till Windows brandvägg
sv.tasks_deleteoldsetup=Delete old installers
sv.run_skinexe=Kör skinninstallation...
sv.reg_incomingchat=Inkommande chattmeddelande
sv.reg_apptitle=Shareaza ultimat fildelning
sv.icons_license=Licens
sv.icons_uninstall=Avinstallera
sv.icons_downloads=Nedladdningar
sv.dialog_shutdown=%1 körs för tillfället. Vill du att %1 ska stängas ned så att installationen kan fortsätta?
sv.dialog_firewall=Setup failed to add Shareaza to the Windows Firewall.%nPlease add Shareaza to the exception list manually.
sv.page_viruswarning_text=När du använder internet ska du alltid försäkra dig om att du har ett uppdaterat antivirusprogram som skyddar dig mot trojaner, maskar och andra skadliga program. Här finns en lista på bra antivirusprogram och andra säkerhetstips för att skydda din dator:
sv.page_viruswarning_title=Virusvarning
sv.page_viruswarning_subtitle=Har du ett antivirusprogram installerat?
sv.CreateDesktopIcon=Skapa en ikon på srivbordet
sv.CreateQuickLaunchIcon=Skapa en ikon i Snabbstartfältet
; Finnish
fi.components_plugins=Laajennukset
fi.components_skins=Ulkoasut
fi.components_languages=Kielet
fi.tasks_allusers=Kaikille käyttäjille
fi.tasks_selectusers=Asenna %1 käyttäjille:
fi.tasks_currentuser=%1 vain
fi.tasks_multisetup=Asenna kaikille koneen käyttäjille
fi.tasks_firewall=Lisää poikkeus Windowsin palomuuriin
fi.tasks_deleteoldsetup=Poista vanhat asennusohjelmat
fi.run_skinexe=Käynnissä ulkoasujen asennus...
fi.reg_incomingchat=Tuleva keskusteluviesti
fi.reg_apptitle=Shareaza jako-ohjelma
fi.icons_license=Lisenssi
fi.icons_uninstall=Poista
fi.icons_downloads=Lataukset
fi.dialog_shutdown=%1 on tällä hetkellä käynnissä. Haluatko että %1 suljetaan jotta asennus voisi jatkua?
fi.dialog_firewall=Asentaja epäonnistui lisätessään Shareazaa Windowsiin Firewall.%nOle hyvä ja lisää Shareaza poikkeuslistaan manuaalisesti.
fi.page_viruswarning_text=Kun käytät internetiä, sinun tulee aina varmistaa että sinulla on viimeisimmät päivitykset virusohjelmissasi jotka suojaavat sinua troijalaisilta, madoilta, ja muilta haittaohjelmilta. Löydät hyvän listan hyvistä virusohjelmista ja turvallisuusvinkkejä seuraavista linkeistä:
fi.page_viruswarning_title=Virus Varoitus
fi.page_viruswarning_subtitle=Onko sinulla AntiVirus ohjelmaa asennettuna?
fi.CreateDesktopIcon=Luo kuvake työpöydälle
fi.CreateQuickLaunchIcon=Luo kuvake pikakäynnistyspalkkiin
; Hebrew
heb.components_plugins=úåñôéí
heb.components_skins=îòèôåú
heb.components_languages=ùôåú
heb.tasks_allusers=ëì äîùúîùéí
heb.tasks_selectusers=äú÷ï àú %1 òáåø
heb.tasks_currentuser=%1 ø÷
heb.tasks_multisetup=àôùø úîéëä ìîùúîùéí îøåáéí
heb.tasks_firewall=äåñó ëìì ìçåîú äàù ìàéôùåø ùøæä
heb.tasks_deleteoldsetup=îç÷ äú÷ğåú éùğåú
heb.run_skinexe=îøéõ äú÷ğú îòèôåú...
heb.reg_incomingchat=äåãòú ö'àè ğëğñú
heb.reg_apptitle=ùéúåó ä÷áöéí äàåìèéîèéáé ùì ùøæä
heb.icons_license=øùéåï
heb.icons_uninstall=äñø äú÷ğä
heb.icons_downloads=äåøãåú
heb.dialog_shutdown=?ëøâò %1 ôåòì äàí àú øåöä ìñâåø àú %1 ëê ùääú÷ğä úåëì ìäîùê
heb.dialog_firewall=ääú÷ğä ğëùìä ìäåñéó àú ùøæä àì çåîú äàù%nàğà äåñó àú ùøæä ìøùéîú äçøéâéí áçåîú äàù áàåôï éãğé
heb.page_viruswarning_text=ëùàú/ä îùúîù áàéğèøğè òìééê úîéã ìååãà ùîåú÷ï àöìê àğèé-åéøåñ îòåãëï ìäâğä îôğé åéøåñéí/úåìòéí/øåâìåú, ìøùéîä ùì àğèé-åéøåñéí åèéôéí áğåùà àáèçä ôğä ì÷éùåø äáà:
heb.page_viruswarning_title=äúøàú åéøåñ
heb.page_viruswarning_subtitle=?äàí éù ìê úåëğú àğèé-åéøåñ îåú÷ğú
heb.CreateDesktopIcon= äöâ ñîì òì ù&åìçï òáåãä
heb.CreateQuickLaunchIcon=äöâ ñîì òì ä&ôòìä îäéøä
; Polish
pl.components_plugins=Wtyczki
pl.components_skins=Skórki
pl.components_languages=Jêzyki
pl.tasks_allusers=Dla wszystkich u¿ytkowników
pl.tasks_selectusers=Instaluj dla %1:
pl.tasks_currentuser=tylko %1
pl.tasks_multisetup=W³¹cz obs³ugê wielu u¿ytkowników
pl.tasks_firewall=Dodaj wyj¹tek do Firewall'a Windows'a
pl.tasks_deleteoldsetup=Usuñ stare instalatory
pl.run_skinexe=Instalowanie skórek...
pl.reg_incomingchat=Przychodz¹ca wiadomoœæ chatowa
pl.reg_apptitle=Shareaza Ultimate File Sharing
pl.icons_license=Licencja
pl.icons_uninstall=Odinstaluj
pl.icons_downloads=Pobierania
pl.dialog_shutdown=%1 obecnie dzia³a. Czy chcia³byœ wy³¹czyæ %1 aby kontynuowaæ instalacjê?
pl.dialog_firewall=Instalator nie potrafi³ dodaæ Shareazy do Firewall'a Windows'a.%nProszê dodaæ Shareazê do listy wyj¹tków rêcznie.
pl.page_viruswarning_text=Podczas u¿ywania internetu zawsze powinieneœ byæ pewny, ¿e masz program antywirusowy z aktualn¹ baz¹ wirusów, który ciê ochroni przed trojanami, robakami i innym niebezpiecznym oprogramowaniem. Mo¿esz znaleŸæ listê dobrych programów antywirusowych i porady jak zabezpieczyæ komputer pod nastêpuj¹cymi adresami:
pl.page_viruswarning_title=Ostrze¿enie przed wirusem
pl.page_viruswarning_subtitle=Czy masz zainstalowany jakiœ program antywirusowy?
pl.CreateDesktopIcon=Wyœwietl ikonê na pulpicie
pl.CreateQuickLaunchIcon=Wyœwietl ikonê na pasku szybkiego uruchamiania
; Serbian
sr.components_plugins=Pluginovi
sr.components_skins=Skinovi
sr.components_languages=Jezici
sr.tasks_allusers=Svi korisnici
sr.tasks_selectusers=Instaliraj %1 za:
sr.tasks_currentuser=%1 samo
sr.tasks_multisetup=Omoguæi više-korisnièku podršku
sr.tasks_firewall=Dodaj izuzetak u Windows Vatrozid
sr.tasks_deleteoldsetup=Ukloni stare instalere
sr.run_skinexe=Instalacija skina u toku...
sr.reg_incomingchat=Dolazeæe cet poruke
sr.reg_apptitle=Shareaza Najbolji P2P za Deljenje Datoteka
sr.icons_license=Licenca
sr.icons_uninstall=Ukloni Program
sr.icons_downloads=Skinutno
sr.dialog_shutdown=%1 je ukljuèena. Da li bi eleli da %1 bude ugašena da bi se instalacija nastavila?
sr.dialog_firewall=Instalacija nije uspla da doda Shareaza-u u Windows Vatrozid.%nMolimo dodajte Shareaza-u na listu izuzetaka ruèno.
sr.page_viruswarning_text=Kada koristite internet, trebali bi uvek da budete sigurni da imate auriaran virus skener koji Vas štiti od trojanaca, crva, i drugih zlonamernih programa. Moete pronaæi listu dobrih anti-virus programa i drugih sigurnosnih saveta kako da zaštitite Vaš raèunar prateæi ovaj link:
sr.page_viruswarning_title=Virus Uopzorenje
sr.page_viruswarning_subtitle=Da li imate AntiVirus program instaliran?
sr.CreateDesktopIcon=Napravi &desktop ikonu
sr.CreateQuickLaunchIcon=Napravi &Brzo Pokretanje(QL) ikonu
;Turkish
tr.components_plugins=Pluginler
tr.components_skins=Ara Yüzler
tr.components_languages=Diller
tr.tasks_allusers=Tüm Kullanıcılar
tr.tasks_selectusers=%1 Kuruldu:
tr.tasks_currentuser=Sadece %1
tr.tasks_multisetup=Çoklu-kullanıcı desteğini etkinleştir
tr.tasks_firewall=Windows Güvenlik Duvarına bir istisna ekle
tr.tasks_deleteoldsetup=Eski kurulumları sil
tr.run_skinexe=Arayüz kurulumu çalışıyor...
tr.reg_incomingchat=Gelen sohbet mesajı
tr.reg_apptitle=Shareaza En iyi Dosya Paylaşımı
tr.icons_license=Lisans
tr.icons_uninstall=Kurulumu Kaldır
tr.icons_downloads=Downloadlar
tr.dialog_shutdown=Şuan %1 çalışıyor.Kurulumun devam edebilmesi için %1'in kapalı olmasını istiyor musunuz?
tr.dialog_firewall=Windows güvenlik duvarına Shareaza kurulumunu eklemek başarısız oldu.%n Lütfen Shareaza'yı el ile istisna listesine ekle
tr.page_viruswarning_text=İnternet kullanıyorken, trojanlardan, wormlardan ve diğer kötü niyetli programlardan sizi koruyan güncel bir virüs tarayıcısına sahip olduğunuzdan emin olmalısınız. Bu bağlantı takibiyle bilgisayarınızı koruyan iyi virüs tarayıcılarının ve diğer güvenlik tiplerinin listesini bulacaksınız:
tr.page_viruswarning_title=Virüs Uyarısı
tr.page_viruswarning_subtitle=Bir AntiVirus programı yükledin mi?
tr.CreateDesktopIcon=Bir &Masaüstü ikonu görüntüle
tr.CreateQuickLaunchIcon=Bir &Hızlı Başlat ikonu görüntüle
; Czech
;cz.components_plugins=Plugins
;cz.components_skins=Skins
;cz.components_languages=Languages
;cz.tasks_allusers=All users
;cz.tasks_selectusers=Install %1 for:
;cz.tasks_currentuser=%1 only
;cz.tasks_multisetup=Enable multi-user support
;cz.tasks_firewall=Add an exception to the Windows Firewall
;cz.tasks_deleteoldsetup=Delete old installers
;cz.run_skinexe=Running skin installation...
;cz.reg_incomingchat=Incoming chat message
;cz.reg_apptitle=Shareaza Ultimate File Sharing
;cz.icons_license=License
;cz.icons_uninstall=Uninstall
;cz.icons_downloads=Downloads
;cz.dialog_shutdown=%1 is currently running. Would you like %1 to be shutdown so the installation can continue?
;cz.dialog_firewall=Setup failed to add Shareaza to the Windows Firewall.%nPlease add Shareaza to the exception list manually.
;cz.page_viruswarning_text=When using the internet, you should always ensure you have an up-to-date virus scanner to protect you from trojans, worms, and other malicious programs. You can find list of good virus scanners and other security tips to protect your computer by following this link:
;cz.page_viruswarning_title=Virus Warning
;cz.page_viruswarning_subtitle=Do you have an AntiVirus program installed?
;cz.CreateDesktopIcon=Display a &desktop icon
;cz.CreateQuickLaunchIcon=Display a &Quick Launch icon
; japanese
jp.components_plugins=ƒvƒ‰ƒOƒCƒ“
jp.components_skins=ƒXƒLƒ“
jp.components_languages=Œ¾Œêƒtƒ@ƒCƒ‹
jp.tasks_allusers=‚·‚×‚Ä‚Ìƒ†[ƒU[
jp.tasks_selectusers=%1‚ğ‚µ‚æ‚¤‚·‚éƒ†[ƒU[:
jp.tasks_currentuser=%1‚Ì‚İ
jp.tasks_multisetup=ƒ}ƒ‹ƒ`ƒ†[ƒU[ƒTƒ|[ƒg
jp.tasks_firewall=Windowsƒtƒ@ƒCƒ„[ƒEƒH[ƒ‹‚Ì—áŠO‚Éİ’è
jp.tasks_deleteoldsetup=ŒÃ‚¢ƒCƒ“ƒXƒg[ƒ‰[‚Ìíœ
jp.run_skinexe=ƒXƒLƒ“ƒCƒ“ƒXƒg[ƒ‰[‚ğÀs‚µ‚Ä‚¢‚Ü‚·...
jp.reg_incomingchat=ƒ`ƒƒƒbƒgƒƒbƒZ[ƒW‚ğó‚¯“ü‚ê‚é
jp.reg_apptitle=Shareazaƒtƒ@ƒCƒ‹‹¤—Lƒ\ƒtƒg
jp.icons_license=ƒ‰ƒCƒZƒ“ƒX
jp.icons_uninstall=ƒAƒ“ƒCƒ“ƒXƒg[ƒ‹
jp.icons_downloads=ƒ_ƒEƒ“ƒ[ƒh
jp.dialog_shutdown=%1 ‚ªŠˆ“®’†‚Å‚·. %1‚ğI—¹‚µ‚ÄƒCƒ“ƒXƒg[ƒ‹‚ğ‘±‚¯‚Ü‚·‚©?
jp.dialog_firewall=WindowsXPƒtƒ@ƒCƒ„[ƒEƒH[ƒ‹‚Ì“o˜^‚É¸”s‚µ‚Ü‚µ‚½.%nè“®‚Å“o˜^‚µ‚Ä‚­‚¾‚³‚¢.
jp.page_viruswarning_text=‚ ‚È‚½‚ªƒCƒ“ƒ^[ƒlƒbƒg‚ÉÚ‘±‚·‚é‚Æ‚«‚ÍAƒgƒƒC‚âƒ[ƒ€–”‚ÍA‚»‚êˆÈŠO‚ÌŠëŒ¯‚Èƒtƒ@ƒCƒ‹‚©‚çPC‚ğ•ÛŒì‚·‚é‚½‚ß‚ÉA•K‚¸ƒEƒCƒ‹ƒX’è‹`ƒtƒ@ƒCƒ‹‚ğÅV‚Ì‚à‚Ì‚É‚µ‚Ü‚µ‚å‚¤BƒEƒCƒ‹ƒXƒXƒLƒƒƒi[‚âƒZƒLƒ…ƒŠƒeƒB-‚ÉŠÖ‚·‚éî•ñ‚ª‰º‹L‚ÌƒŠƒ“ƒN‚É‚ ‚è‚Ü‚·B
jp.page_viruswarning_title=ƒEƒCƒ‹ƒX‚ÌŒx
jp.page_viruswarning_subtitle=ƒAƒ“ƒ`EƒEƒCƒ‹ƒXEƒ\ƒtƒg‚Í“ü‚Á‚Ä‚¢‚Ü‚·‚©?
jp.CreateDesktopIcon=ƒfƒXƒNƒgƒbƒv‚ÉƒAƒCƒRƒ“‚ğ•\¦(&d)
jp.CreateQuickLaunchIcon=ƒNƒCƒbƒNƒ‰ƒ“ƒ`‚ÉƒAƒCƒRƒ“‚ğ•\¦(&Q)
; arabic
ar.components_plugins=ÇáæÙÇÆİ ÇáÅÖÇİíÉ
ar.components_skins=ÇáÛáÇİÇÊ
ar.components_languages=ÇááÛÇÊ
ar.tasks_allusers=ßÇİÉ ÇáãÓÊÎÏãíä
ar.tasks_selectusers=ãä ÃÌá %1 ÊÑßíÈ:
ar.tasks_currentuser=%1 İŞØ
ar.tasks_multisetup=Êãßíä ÇáÏÚã áÚÏÉ ãÓÊÎÏãíä
ar.tasks_firewall=ÅÖÇİÉ ÅÓÊËäÇÁ Åáì ÌÏÇÑ ÇáæíäÏæÒ ÇáäÇÑí
ar.tasks_deleteoldsetup=ÍĞİ ãáİÇÊ ÇáÊÑßíÈ ÇáŞÏíãÉ
ar.run_skinexe=...íÊã ÊÔÛíá ÊÑßíÈ ÇáÛáÇİ
ar.reg_incomingchat=ÑÓÇáÉ ãÍÇÏËÉ æÇÑÏÉ
ar.reg_apptitle=ÔíÑ-ÃÒÇ ááãÔÇÑßÉ ÇáÛíÑ ãÍÏæÏÉ ÈÇáãáİÇÊ
ar.icons_license=ÇáÑÎÕÉ
ar.icons_uninstall=ÅáÛÇÁ ÇáÊËÈíÊ
ar.icons_downloads=ÇáÊÍãíáÇÊ
ar.dialog_shutdown=%1 íÚãá ÍÇáíÇ . åá ÊÑíÏ ÅÛáÇŞ %1 áíÓÊãÑ ÇáÊÑßíÈ ¿
ardialog_firewall=İÔá ÇáÅÚÏÇÏ İí ÅÖÇİÉ ÔíÑ-ÃÒÇ Åáì ÈÑäÇãÌ ÍãÇíÉ ÇáæíäÏæÒ %nÇáÑÌÇÁ ÅÖÇİÉ ÔíÑ-ÃÒÇ Åáì ŞÇÆãÉ ÇáÅÓÊËäÇÁÇÊ íÏæíÇ
ar.page_viruswarning_text=ÚäÏãÇ ÅÓÊÚãÇá ÇáÅäÊÑäÊ ¡ íÌÈ Ãä ÊÊÃßÏ ãä æÌæÏ ÃÍÏË ÈÑäÇãÌ ááİíÑæÓ áÏíß . íãßäß ÇáÍÕæá Úáì ÈÑÇãÌ ááİíÑæÓÇÊ æ äÕÇÆÍ ÃãäíÉ ÃÎÑì áÍãÇíÉ ßãÈíæÊÑß ãä åĞå ÇáæÕáÉ:
ar.page_viruswarning_title=ÊÍĞíÑ ãä ÇáİíÑæÓÇÊ
ar.page_viruswarning_subtitle=åá ÚäÏß ÈÑäÇãÌ ááİíÑæÓÇÊ ¿
ar.CreateDesktopIcon=ÅÙåÇÑ &ÃíŞæäÉ ÓØÍ ÇáãßÊÈ
ar.CreateQuickLaunchIcon=ÅÙåÇÑ &ÃíŞæäÉ ÇáÊÔÛíá ÇáÓÑíÚ
