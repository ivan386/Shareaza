; This sub-script defines all languages to be compiled
; WARNING: Do not change this file's encoding

[Languages]
; Use compiler's built in ISL file to patch up holes in ISL collection and specify localized license files
; Note: first language specified is default > English

Name: "en"; MessagesFile: "compiler:Default.isl"; LicenseFile: "setup/license/default.rtf"
Name: "nl"; MessagesFile: "compiler:Languages\dutch.isl"; LicenseFile: "setup/license/dutch.rtf"
Name: "lt"; MessagesFile: "setup\isl\lithuanian.isl"; LicenseFile: "setup/license/lithuanian.rtf"
Name: "de"; MessagesFile: "compiler:Languages\German.isl"; LicenseFile: "setup/license/German.rtf"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl"; LicenseFile: "setup/license/italian.rtf"
Name: "no"; MessagesFile: "compiler:Languages\Norwegian.isl"; LicenseFile: "setup/license/default.rtf"
Name: "af"; MessagesFile: "setup\isl\afrikaans.isl"; LicenseFile: "setup/license/afrikaans.rtf"
Name: "pt"; MessagesFile: "compiler:Languages\Portuguese.isl"; LicenseFile: "setup/license/portuguese-braz.rtf"
Name: "pt_br"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"; LicenseFile: "setup/license/portuguese-braz.rtf"
Name: "fr"; MessagesFile: "compiler:Languages\french.isl"; LicenseFile: "setup/license/default.rtf"
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl"; LicenseFile: "setup/license/spanish-mexican.rtf"
Name: "es_mx"; MessagesFile: "setup\isl\spanish-mexican.isl"; LicenseFile: "setup/license/spanish-mexican.rtf"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"; LicenseFile: "setup/license/russian.rtf"
Name: "gr"; MessagesFile: "setup\isl\greek.isl"; LicenseFile: "setup/license/greek.rtf"
Name: "hu"; MessagesFile: "compiler:Languages\Hungarian.isl"; LicenseFile: "setup/license/hungarian.rtf"
Name: "chs"; MessagesFile: "setup\isl\chinese-simp.isl"; LicenseFile: "setup/license/chinese.rtf"
Name: "sv"; MessagesFile: "setup\isl\swedish.isl"; LicenseFile: "setup/license/swedish.rtf"
Name: "fi"; MessagesFile: "compiler:Languages\Finnish.isl"; LicenseFile: "setup/license/finnish.rtf"
Name: "heb"; MessagesFile: "compiler:Languages\Hebrew.isl"; LicenseFile: "setup/license/hebrew.rtf"
Name: "pl"; MessagesFile: "compiler:Languages\Polish.isl"; LicenseFile: "setup/license/polish.rtf"
Name: "sr"; MessagesFile: "setup\isl\Serbian.isl"; LicenseFile: "setup/license/serbian.rtf"
Name: "tr"; MessagesFile: "setup\isl\turkish.isl"; LicenseFile: "setup/license/turkish.rtf"
Name: "ja"; MessagesFile: "setup\isl\japanese.isl"; LicenseFile: "setup/license/japanese.rtf"
Name: "ar"; MessagesFile: "setup\isl\arabic.isl"; LicenseFile: "setup/license/default.rtf"
Name: "ee"; MessagesFile: "setup\isl\estonian.isl"; LicenseFile: "setup/license/estonian.rtf"
Name: "tw"; MessagesFile: "setup\isl\chinese-trad.isl"; LicenseFile: "setup/license/chinese-trad.rtf"
Name: "cz"; MessagesFile: "compiler:Languages\Czech.isl"; LicenseFile: "setup/license/czech.rtf"
Name: "sl"; MessagesFile: "compiler:Languages\Slovenian.isl"; LicenseFile: "setup/license/default.rtf"
Name: "ca"; MessagesFile: "compiler:Languages\Catalan.isl"; LicenseFile: "setup/license/catalan.rtf"
Name: "sq"; MessagesFile: "setup\isl\albanian.isl"; LicenseFile: "setup/license/albanian.rtf"

[Files]
; Common files
Source: "Remote\Common\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"

; Install default remote
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Tasks: not language
; Install localized remote
; English
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: en; Tasks: language
; Dutch
Source: "Remote\nl\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: nl; Tasks: language
; Lithuanian
Source: "Remote\lt\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: lt; Tasks: language
; German
Source: "Remote\de\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: de; Tasks: language
; Italian
Source: "Remote\it\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: it; Tasks: language
; Norwegian
Source: "Remote\no\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: no; Tasks: language
; Afrikaans
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: af; Tasks: language
; Portuguese std
Source: "Remote\pt-br\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: pt; Tasks: language
; Portuguese Braz
Source: "Remote\pt-br\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: pt_br; Tasks: language
; French
Source: "Remote\fr\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: fr; Tasks: language
; Spanish std
Source: "Remote\es-mx\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: es; Tasks: language
; Spanish Mexican
Source: "Remote\es-mx\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: es_mx; Tasks: language
; Russian
Source: "Remote\ru\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: ru; Tasks: language
; Greek
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: gr; Tasks: language
; Hungarian
Source: "Remote\hu\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: hu; Tasks: language
; Chinese Simp
Source: "Remote\chs\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: chs; Tasks: language
; Swedish
Source: "Remote\sv\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: sv; Tasks: language
; Finnish
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: fi; Tasks: language
; Hebrew
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: heb; Tasks: language
; Polish
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: pl; Tasks: language
; Czech
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: cz; Tasks: language
; Serbian
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: sr; Tasks: language
; Turkish
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: tr; Tasks: language
; Japanese
Source: "Remote\ja\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: ja; Tasks: language
; Arabic
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: ar; Tasks: language
; Estonian
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: ee; Tasks: language
; Chinese Trad
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: tw; Tasks: language
; Slovenian
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: sl; Tasks: language
; Catalan
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: ca; Tasks: language
; Albanian
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: sq; Tasks: language

; Install default license
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Tasks: not language
; Install localized license
; English
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: en; Tasks: language
; Dutch
Source: "setup\license\dutch.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: nl; Tasks: language
; Lithuanian
Source: "setup\license\lithuanian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: lt; Tasks: language
; German
Source: "setup\license\german.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: de; Tasks: language
; Italian
Source: "setup\license\italian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: it; Tasks: language
; Norwegian
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: no; Tasks: language
; Afrikaans
Source: "setup\license\afrikaans.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: af; Tasks: language
; Portuguese std
Source: "setup\license\portuguese-braz.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt; Tasks: language
; Portuguese Braz
Source: "setup\license\portuguese-braz.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt_br; Tasks: language
; French
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fr; Tasks: language
; Spanish std
Source: "setup\license\spanish-mexican.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es; Tasks: language
; Spanish Mexican
Source: "setup\license\spanish-mexican.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es_mx; Tasks: language
; Russian
Source: "setup\license\russian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ru; Tasks: language
; Greek
Source: "setup\license\greek.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: gr; Tasks: language
; Hungarian
Source: "setup\license\hungarian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: hu; Tasks: language
; Chinese Simp
Source: "setup\license\chinese.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: chs; Tasks: language
; Swedish
Source: "setup\license\swedish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sv; Tasks: language
; Finnish
Source: "setup\license\finnish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fi; Tasks: language
; Hebrew
Source: "setup\license\hebrew.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: heb; Tasks: language
; Polish
Source: "setup\license\polish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pl; Tasks: language
; Czech
Source: "setup\license\czech.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: cz; Tasks: language
; Serbian
Source: "setup\license\serbian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sr; Tasks: language
; Turkish
Source: "setup\license\turkish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tr; Tasks: language
; Japanese
Source: "setup\license\japanese.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ja; Tasks: language
; Arabic
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ar; Tasks: language
; Estonian
Source: "setup\license\estonian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ee; Tasks: language
; Chinese Trad
Source: "setup\license\chinese-trad.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tw; Tasks: language
; Slovenian
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sl; Tasks: language
; Catalan
Source: "setup\license\catalan.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ca; Tasks: language
; Catalan
Source: "setup\license\albanian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sq; Tasks: language

; Install default filter
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Tasks: not language
; Install localized filter
; English
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: en; Tasks: language
; Dutch
Source: "setup\filter\dutch.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: nl; Tasks: language
; Lithuanian
Source: "setup\filter\lithuanian.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: lt; Tasks: language
; German
Source: "setup\filter\german.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: de; Tasks: language
; Italian
Source: "setup\filter\italian.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: it; Tasks: language
; Norwegian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: no; Tasks: language
; Afrikaans
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: af; Tasks: language
; Portuguese std
Source: "setup\filter\portuguese-br.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt; Tasks: language
; Portuguese Braz
Source: "setup\filter\portuguese-br.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt_br; Tasks: language
; French
Source: "setup\filter\french.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fr; Tasks: language
; Spanish std
Source: "setup\filter\spanish-mexican.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es; Tasks: language
; Spanish Mexican
Source: "setup\filter\spanish-mexican.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es_mx; Tasks: language
; Russian
Source: "setup\filter\russian.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ru; Tasks: language
; Greek
Source: "setup\filter\greek.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: gr; Tasks: language
; Hungarian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: hu; Tasks: language
; Chinese Simp
Source: "setup\filter\chinese-simpl.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: chs; Tasks: language
; Swedish
Source: "setup\filter\swedish.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sv; Tasks: language
; Finnish
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fi; Tasks: language
; Hebrew
Source: "setup\filter\hebrew.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: heb; Tasks: language
; Polish
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pl; Tasks: language
; Czech
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: cz; Tasks: language
; Serbian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sr; Tasks: language
; Turkish
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tr; Tasks: language
; Japanese
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ja; Tasks: language
; Arabic
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ar; Tasks: language
; Estonian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ee; Tasks: language
; Chinese Trad
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tw; Tasks: language
; Slovenian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sl; Tasks: language
; Catalan
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ca; Tasks: language
; Albanian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sq; Tasks: language

[CustomMessages]
; This section specifies phrazes and words not specified in the ISL files
; Avoid customizing the ISL files since they will change with each version of Inno Setup.
; English
components_plugins=Plugins
components_skins=Skins
tasks_languages=Multi-language
tasks_allusers=All users
tasks_selectusers=Install %1 for:
tasks_currentuser=%1 only
tasks_multisetup=Enable multi-user support
tasks_firewall=Add an exception to the Windows Firewall
tasks_upnp=Enable discovery of UPnP devices
tasks_deleteoldsetup=Delete old installers
tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
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
dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
page_viruswarning_text=When using the internet, you should always ensure you have an up-to-date virus scanner to protect you from trojans, worms, and other malicious programs. You can find list of good virus scanners and other security tips to protect your computer by following this link:
page_viruswarning_title=Virus Warning
page_viruswarning_subtitle=Do you have an AntiVirus program installed?
CreateDesktopIcon=Display a &desktop icon
CreateQuickLaunchIcon=Display a &Quick Launch icon
PathNotExist=Error, the path of the %1 folder doesn't exist
; Don't copy these last 2 messages, they are just links.
page_viruswarning_link=http://shareaza.sourceforge.net/securityhelp
page_viruswarning_destination=http://shareaza.sourceforge.net/securityhelp/

; Dutch
nl.components_plugins=Plugins
nl.components_skins=Skins
nl.tasks_languages=Talen
nl.tasks_allusers=Alle gebruikers
nl.tasks_selectusers=Installeer %1 voor:
nl.tasks_currentuser=Alleen %1
nl.tasks_multisetup=Ondersteuning voor meerdere gebruikers inschakelen
nl.tasks_firewall=Een uitzondering aan de Windows Firewall toevoegen
nl.tasks_upnp=Configureer automatisch mijn router
nl.tasks_deleteoldsetup=Oude installatieprogramma's wissen
nl.tasks_resetdiscoveryhostcache=Herstel de Discovery- en Hostcachelist
nl.run_skinexe=Skin installatie uitvoeren...
nl.reg_incomingchat=Nieuw chat bericht
nl.reg_apptitle=Shareaza: De Ultieme FileSharer
nl.icons_license=Gebruiksovereenkomst
nl.icons_uninstall=Verwijderen
nl.icons_downloads=Downloads
nl.icons_basicmode=Normale Stijl
nl.icons_tabbedmode=Tabblad Stijl
nl.icons_windowedmode=Venster Stijl
nl.dialog_shutdown=%1 is momenteel open. Wil je %1 afsluiten zodat de installatie verder kan gaan?
nl.dialog_firewall=Het installatieprogramma kon Shareaza niet toevoegen aan de Windows Firewall.%nVoeg Shareaza alstublieft manueel toe aan de uitzonderingenlijst.
nl.dialog_malwaredetected=Er werd malware gevonden op %1, verwijder het alstublieft met een virus/malware scanner vooraleer Shareaza verder te installeren. Wil je nu sluiten?
nl.page_viruswarning_text=Als u het internet gebruikt moet u een recente virusscanner gebruiken om u te beschermen tegen virussen, wormen en andere kwaadaardige programma's. U kan een lijst van virusscanners en andere veiligheidstips vinden om uw computer the beschermen door deze link te volgen:
nl.page_viruswarning_title=Virus Waarschuwing
nl.page_viruswarning_subtitle=Heeft u een antivirus programma geïnstalleerd?
nl.CreateDesktopIcon=Toon een snelkoppeling op het &bureaublad
nl.CreateQuickLaunchIcon=Toon een snelkoppeling op de &Snel Starten werkbalk
; Lithuanian
lt.components_plugins=Papildiniai
lt.components_skins=Apvalkalai
lt.tasks_languages=Kalbos
lt.tasks_allusers=Visiems vartotojams
lt.tasks_selectusers=Ádiegti „%1“ ğiems vartotojams:
lt.tasks_currentuser=Tik vartotojui %1
lt.tasks_multisetup=Ágalinti keliø vartotojø palaikymà
lt.tasks_firewall=Pridëti prie Windows ugniasienës iğimèiø sàrağo
lt.tasks_upnp=Ágalinti UPnP árenginiø aptikimà
lt.tasks_deleteoldsetup=Trinti senas diegimo programas
lt.tasks_resetdiscoveryhostcache=Apnulinti tarnybas ir adresatø talpyklas
lt.run_skinexe=Vykdoma apvalkalo sàranka...
lt.reg_incomingchat=Gaunama şinutë pokalbiui
lt.reg_apptitle=Shareaza — geriausia programa bylø mainams
lt.icons_license=Licencinë sutartis
lt.icons_uninstall=Pağalinti
lt.icons_downloads=Atsisiuntimai
lt.icons_basicmode=Áprastinë veiksena
lt.icons_tabbedmode=Korteliø veiksena
lt.icons_windowedmode=Langø veiksena
lt.dialog_shutdown=„%1“ ğiuo metu dirba. Ar norite nutraukti „%1“ darbà, tam kad bûtø galima tæsti ádiegimà?
lt.dialog_firewall=Ádiegimo programai nepavyko pridëti „Shareaza“ prie Windows ugniasienës iğimèiø sàrağo.%nPridëkite jà á ğá sàrağà rankiniu bûdu.
lt.dialog_malwaredetected=Jûsø sistemoje aptiktos kenkëjiğkos programos (%1). Prieğ diegdami „Shareaza“, pağalinkite jas antivirusine programa ar skeneriu. Nutraukti diegimà dabar?
lt.page_viruswarning_text=Visada, kai naudojatës Internetu, ásitikinkite, jog turite naujausià virusø skenerá, tam kad bûtumëte apsaugoti nuo trojanø, kirminø ir kitokiø kenkëjiğkø programø. Jûs galite rasti gerø virusø skeneriø sàrağà ir kitokius kompiuterio apsaugojimo patarimus nuëjæ ğiuo adresu:
lt.page_viruswarning_title=Áspëjimas apie virusus
lt.page_viruswarning_subtitle=Ar Jûs turite ásidiegæ antivirusinæ programà?
lt.CreateDesktopIcon=Rodyti piktogramà &Darbalaukyje
lt.CreateQuickLaunchIcon=Rodyti Sparèiosios &Paleisties piktogramà
lt.PathNotExist=Klaida, katalogo kelias „%1“ neegzistuoja
; German
de.components_plugins=Plugins
de.components_skins=Skins
de.tasks_languages=Sprachen
de.tasks_allusers=Alle Benutzer
de.tasks_selectusers=Installieren %1 für:
de.tasks_currentuser=Nur für %1
de.tasks_multisetup=Mehrbenutzerunterstützung aktivieren
de.tasks_firewall=Eine Ausnahme zur Windows Firewall hinzufügen
de.tasks_upnp=Automatische Routerkonfiguration benutzen ( UPnP )
de.tasks_deleteoldsetup=Alte Installer löschen
de.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
de.run_skinexe=Skin Installer einrichten...
de.reg_incomingchat=Eingehende Chat Nachricht
de.reg_apptitle=Shareaza Ultimate File Sharing
de.icons_license=Lizenz
de.icons_uninstall=Shareaza deinstallieren
de.icons_downloads=Downloads
de.icons_basicmode=Normal Modus
de.icons_tabbedmode=Tab Modus
de.icons_windowedmode=Fenster Modus
de.dialog_shutdown=%1 wird zur Zeit ausgeführt. Wollen Sie %1 beenden, um mit der Installation fortzufahren?
de.dialog_firewall=Setup konnte Shareaza nicht zur Windows Firewall hinzufügen.%nBitte tragen Sie Shareaza manuell in die Ausnahmenliste ein.
de.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
de.page_viruswarning_text=Wenn Sie das Internet benutzen, sollten Sie stets einen aktuellen Virenscanner installiert haben, der ihren Computer vor Trojanern, Würmern und anderen bösartigen Programmen beschützt. Sie finden eine Liste guter Virenscanner und andere Tipps, wie Sie ihren Computer schützen können, unter folgendem Link:
de.page_viruswarning_title=Virenwarnung
de.page_viruswarning_subtitle=Haben Sie ein Antivirenprogramm installiert?
de.CreateDesktopIcon=&Symbol auf dem Desktop anzeigen
de.CreateQuickLaunchIcon=&Quick Launch Symbol anzeigen
; Italian
it.components_plugins=Plugin
it.components_skins=Skin
it.tasks_languages=Multi-lingua
it.tasks_allusers=Tutti gli utenti
it.tasks_selectusers=Installa %1 per:
it.tasks_currentuser=Solo %1
it.tasks_multisetup=Abilita il supporto multi-utente
it.tasks_firewall=Aggiungi un'eccezione a Windows Firewall
it.tasks_upnp=Abilita il rilevamento dei dispositivi UPnP
it.tasks_deleteoldsetup=Cancella installer vecchi
it.tasks_resetdiscoveryhostcache=Resetta i servizi di connessione e la cache host
it.run_skinexe=Installazione delle skin in corso...
it.reg_incomingchat=Messaggio di chat in arrivo
it.reg_apptitle=Shareaza, il programma definitivo di P2P
it.icons_license=Licenza
it.icons_uninstall=Disinstalla
it.icons_downloads=Download
it.icons_basicmode=Modalità normale
it.icons_tabbedmode=Modalità a tabelle
it.icons_windowedmode=Modalità a finestre
it.dialog_shutdown=%1 è attualmente in esecuzione. Vuoi che %1 sia chiuso così l'installazione può continuare?
it.dialog_firewall=Impossibile aggiungere Shareaza a Windows Firewall.%nAggiungi Shareaza alla lista delle eccezioni manualmente.
it.dialog_malwaredetected=Un programma malevolo (malware) è stato rilevato sul tuo sistema in %1, rimuovilo con un programma anti-virus/anti-malware prima di installare Shareaza. Vuoi uscire adesso?
it.page_viruswarning_text=Quando usi internet, dovresti sempre assicurarti di aver un antivirus aggiornato per proteggerti dai trojan, worm e dagli altri programmi malevoli. Puoi trovare una lista di buoni antivirus e altri suggerimenti di sicurezza per proteggere il tuo computer seguendo questo link:
it.page_viruswarning_title=Attenzione ai virus
it.page_viruswarning_subtitle=Hai installato un programma antivirus?
it.CreateDesktopIcon=Visualizza un'icona sul &desktop
it.CreateQuickLaunchIcon=Visualizza un'icona in &Avvio veloce
it.PathNotExist=Errore, il percorso della cartella %1 non esiste
; Norwegian
no.components_plugins=Plugins
no.components_skins=Skins
no.tasks_languages=Språk
no.tasks_allusers=Alle brukere
no.tasks_selectusers=Installer %1 for:
no.tasks_currentuser=Kun %1
no.tasks_multisetup=Flere brukere
no.tasks_firewall=Lag nytt unntak i Windows brannmur
no.tasks_upnp=Enable discovery of UPnP devices
no.tasks_deleteoldsetup=Slett eldre installasjonsfiler
no.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
no.run_skinexe=Kjører skin installasjon...
no.reg_incomingchat=Innkommende melding
no.reg_apptitle=Shareaza Ultimate File Sharing
no.icons_license=Lisens
no.icons_uninstall=Uninstall
no.icons_downloads=Downloads
no.icons_basicmode=Normalmodus
no.icons_tabbedmode=Fanemodus
no.icons_windowedmode=Vindumodus
no.dialog_shutdown=%1 kjører. Ønsker du at %1 avsluttes slik at installasjonen kan fortsette?
no.dialog_firewall=Installasjonen klarte ikke å lage unntak for Shareaza i Windows Brannmuren. %nVennligst legg til shareaza i brannmurens unntak manuelt.
no.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
no.page_viruswarning_text=Når du bruker internett bør du alltid ha et oppdatert antivirus-program, for å beskytte deg fra trojaner, ormer, og annen skadelig programvare. Du kan finne en liste over gode antivirus-prgrammer og andre sikkerhetstips, for å beskytte din datamaskin, ved å følge denne linken:
no.page_viruswarning_title=Virusadvarsel
no.page_viruswarning_subtitle=Har du et antivirus-program installert?
no.CreateDesktopIcon=Vis ikon på &skrivebordet
no.CreateQuickLaunchIcon=Vis et &Hurtigstarts-ikon
; Afrikaans
af.components_plugins=Inpropprogramme
af.components_skins=Omslagte
af.tasks_languages=Tale
af.tasks_allusers=Alle gebruikers van hierdie rekenaar
af.tasks_selectusers=Installeer %1 vir die volgende gebruikers:
af.tasks_currentuser=Vir %1 alleenlik
af.tasks_multisetup=Skakel ondersteuning vir veelvuldige gebruikers aan
af.tasks_firewall=Voeg 'n uitsondering by die Windows Netskans
af.tasks_upnp=Enable discovery of UPnP devices
af.tasks_deleteoldsetup=Skrap ou opstellerslêers
af.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
af.run_skinexe=Hardloop omslagte installasie...
af.reg_incomingchat=Inkomende Geselsie-boodskap
af.reg_apptitle=Shareaza Ultimate File Sharing
af.icons_license=Lisensie-ooreenkoms
af.icons_uninstall=Deïnstalleer
af.icons_downloads=Aflaaie
af.icons_basicmode=Normale Modus
af.icons_tabbedmode=Tabelmodus
af.icons_windowedmode=Venstermodus
af.dialog_shutdown=%1 is op die oomblik besig om te loop. Wil jy graag %1 sluit sodat die installasie kan voortgaan?
af.dialog_firewall=Die Opsteller kon nie Shareaza by die Windows netskans uitsonderings voeg nie.%nVoeg Shareaza asseblief met die hand op die uitsonderingslys.
af.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
af.page_viruswarning_text=Maak altyd seker dat jy 'n opgedateerde anti-virus program geïnstalleer het wanneer jy die internet gebruik, om jou rekenaar te beskerm teen virusse, wurms, en ander ongewenste programme. Jy kan 'n lys van goeie anti-virus programme en ander sekuriteitswenke oor hoe om jou rekenaar te beskerm verkry deur die volgende skakel te volg:
af.page_viruswarning_title=Virus Waarskuwing
af.page_viruswarning_subtitle=Het jy 'n Anti-Virus program geïnstalleer?
af.CreateDesktopIcon=Vertoon 'n &werkskerm ikoon
af.CreateQuickLaunchIcon=Vertoon 'n &Quick Launch ikoon
; Portuguese std
pt.components_plugins=Plugins
pt.components_skins=Skins
pt.tasks_languages=Linguagens
pt.tasks_allusers=Todos os usuários
pt.tasks_selectusers=Instalar %1 para:
pt.tasks_currentuser=somente %1
pt.tasks_multisetup=Ativar o suporte a vários usuários
pt.tasks_firewall=Adicionar exceção ao Firewall do Windows
pt.tasks_upnp=Ativar a descoberta de dispositivos UPnP
pt.tasks_deleteoldsetup=Apagar os instaladores antigos
pt.tasks_resetdiscoveryhostcache=Resetar o Cache de Armazenamento e Descoberta
pt.run_skinexe=Instalando a Skin...
pt.reg_incomingchat=Mensagem de chat
pt.reg_apptitle=Shareaza Compartilhamento de Arquivos Incomparável
pt.icons_license=Licença
pt.icons_uninstall=Desinstalar
pt.icons_downloads=Downloads
pt.icons_basicmode=Modo Normal
pt.icons_tabbedmode=Modo de Abas
pt.icons_windowedmode=Modo de Janelas
pt.dialog_shutdown=O %1 está sendo executado. Você gostaria que o %1 fosse fechado para que a instalação continue?
pt.dialog_firewall=O Setup falhou em adicionar o Shareaza no Firewall do Windows.%nPor favor adicione o Shareaza a lista de exceções manualmente.
pt.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
pt.page_viruswarning_text=Quando usando a internet, você deve sempre garantir que você tenha um scanner contra vírus atualizado para proteger você de trojans, worms e outros programas maliciosos. Você pode achar uma lista de bons scanners contra vírus e outras dicas de segurança para proteger o seu computador seguindo este link:
pt.page_viruswarning_title=Alerta contra Vírus
pt.page_viruswarning_subtitle=Você tem um programa Anti-Vírus instalado?
pt.CreateDesktopIcon=Criar ícone no &Ambiente de Trabalho
pt.CreateQuickLaunchIcon=Criar ícone na barra de Inicialização &Rápida
; Portuguese Braz
pt_br.components_plugins=Plugins
pt_br.components_skins=Skins
pt_br.tasks_languages=Linguagens
pt_br.tasks_allusers=Todos os Usuários
pt_br.tasks_selectusers=Instalar %1 para:
pt_br.tasks_currentuser=%1 apenas
pt_br.tasks_multisetup=Ativar suporte para vários usuários
pt_br.tasks_firewall=Adicionar exceção ao Firewall do Windows
pt_br.tasks_upnp=Ativar descopt_brimento de dispositivos UPnP
pt_br.tasks_deleteoldsetup=Apagar os instaladores antigos
pt_br.tasks_resetdiscoveryhostcache=Resetar Descopt_brimento e Cache de Hosts
pt_br.run_skinexe=Instalando as Skins...
pt_br.reg_incomingchat=Nova mensagem no chat
pt_br.reg_apptitle=Shareaza o Compartilhador de Arquivos Definitivo
pt_br.icons_license=Licença
pt_br.icons_uninstall=Desinstalar
pt_br.icons_downloads=Downloads
pt_br.icons_basicmode=Modo Simples
pt_br.icons_tabbedmode=Modo Avançado
pt_br.icons_windowedmode=Modo de Janelas
pt_br.dialog_shutdown=Você quer fechar o %1?
pt_br.dialog_firewall=A instalação falhou ao tentar adicionar o Shareaza à lista de exceções do Firewall do Windows.%nPor favor adicione manualmente o Shareaza a lista.
pt_br.dialog_malwaredetected=Um malware foi detectado no seu sistema as %1, por favor remova-o com um scanner contra vírus/malware antes de instalar o Shareaza. Você quer sair agora?
pt_br.page_viruswarning_text=Ao usar a Internet você deve sempre manter seu Anti-Vírus atualizado, para proteger contra vírus, worms, cavalos-de-tróia e outros programas perigosos. Você encontra uma lista de bons anti-vírus e dicas de segurança entrando no seguinte endereço:
pt_br.page_viruswarning_title=Aviso sopt_bre Vírus
pt_br.page_viruswarning_subtitle=Você tem um programa anti-vírus instalado?
pt_br.CreateDesktopIcon=Mostrar um ícone na &área de trabalho
pt_br.CreateQuickLaunchIcon=Mostrar um ícone na barra de &Inicialização Rápida
; French
fr.components_plugins=Plugins
fr.components_skins=Skins
fr.tasks_languages=Langues
fr.tasks_allusers=Tous les utilisateurs
fr.tasks_selectusers=Installer %1 pour:
fr.tasks_currentuser=%1 seulement
fr.tasks_multisetup=Activer le support multi-utilisateurs
fr.tasks_firewall=Ajouter une exception au Pare-feu Windows
fr.tasks_upnp=Activer UPnP pour essayer decouvrir les pare-feu/routeurs.
fr.tasks_deleteoldsetup=Voulez-vous supprimer les anciens programs d'installation Shareaza?
fr.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
fr.run_skinexe=Installation de la skin en cours...
fr.reg_incomingchat=Réception d'un message chat
fr.reg_apptitle=Shareaza Ultimate File Sharing
fr.icons_license=Licence
fr.icons_uninstall=Désinstaller
fr.icons_downloads=Téléchargements
fr.icons_basicmode=Mode normal
fr.icons_tabbedmode=Mode tabulé
fr.icons_windowedmode=Mode fenêtré
fr.dialog_shutdown=%1 est en cours d'exécution. Voulez-vous quitter %1 pour que l'installation puisse se poursuivre?
fr.dialog_firewall=L'installation n'a pas pu ajouter Shareaza au Pare-feu Windows.%nVeuillez ajouter Shareaza manuellement à la liste des exceptions.
fr.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
fr.page_viruswarning_text=Lorsque vous utilisez internet, vous devriez toujours vous assurer que vous avez un scanner de virus à jour pour vous protéger des troyens (trojans), vers (worms), et autres programmes malveillants. Vous pouvez trouver une liste de bons antivirus et conseils de sécurité pour protéger votre ordinateur en suivant ce lien:
fr.page_viruswarning_title=Avertissement sur les virus
fr.page_viruswarning_subtitle=Avez-vous un antivirus installé?
fr.CreateDesktopIcon=Afficher un raccourci sur le &Bureau
fr.CreateQuickLaunchIcon=Afficher un raccouri dans la barre de &Lancement rapide
; Spanish std
es.components_plugins=Plugins
es.components_skins=Skins
es.tasks_languages=Idiomas
es.tasks_allusers=Todos los usuarios
es.tasks_selectusers=Instalar %1 para:
es.tasks_currentuser=%1 solamente
es.tasks_multisetup=Habilitar soporte multi-usuario
es.tasks_firewall=Agregar una excepción al Firewall de Windows
es.tasks_upnp=Enable discovery of UPnP devices
es.tasks_deleteoldsetup=Borrar archivos de instaladores viejos
es.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
es.run_skinexe=Instalando Skin...
es.reg_incomingchat=Hay un mensaje de chat entrante
es.reg_apptitle=Shareaza Ultimate File Sharing
es.icons_license=Licencia
es.icons_uninstall=Desinstalar
es.icons_downloads=Descargas
es.icons_basicmode=Modo Normal
es.icons_tabbedmode=Modo Avanzado
es.icons_windowedmode=Modo Ventanas
es.dialog_shutdown=%1 se encuentra ejecutándose. ¿Deseas que %1 sea cerrado para que la instalación pueda continuar?
es.dialog_firewall=La instalación fallo al agregar la excepción de Shareaza al cortafuego Firewall.%n Por favor hágalo manualmente.
es.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
es.page_viruswarning_text=Cuando estas usando Internet, debes siempre asegurarte que tienes un antivirus actualizado hasta la fecha para protegerte de troyanos, gusanos, y otros programas maliciosos. Puedes encontrar una lista de buenos antivirus y sugerencias de seguridad para proteger tu computadora en la siguiente dirección:
es.page_viruswarning_title=Peligro de Virus
es.page_viruswarning_subtitle=¿Tienes un programa antivirus instalado?
es.CreateDesktopIcon=Mostrar/Quitar icono de &Escritorio
es.CreateQuickLaunchIcon=Mostrar/Quitar icono de &Inicio Rápido
; Spanish Mexican
es_mx.components_plugins=Plugins
es_mx.components_skins=Skins
es_mx.tasks_languages=Idiomas
es_mx.tasks_allusers=Todos los usuarios
es_mx.tasks_selectusers=Instalar %1 para:
es_mx.tasks_currentuser=%1 solamente
es_mx.tasks_multisetup=Habilitar soporte multi-usuario
es_mx.tasks_firewall=Agregar una excepción al Firewall de Windows
es_mx.tasks_upnp=Enable discovery of UPnP devices
es_mx.tasks_deleteoldsetup=Borrar archivos de instaladores viejos
es_mx.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
es_mx.run_skinexe=Instalando Skin...
es_mx.reg_incomingchat=Hay un mensaje de chat entrante
es_mx.reg_apptitle=Shareaza Ultimate File Sharing
es_mx.icons_license=Licencia
es_mx.icons_uninstall=Desinstalar
es_mx.icons_downloads=Descargas
es_mx.icons_basicmode=Modo Normal
es_mx.icons_tabbedmode=Modo Avanzado
es_mx.icons_windowedmode=Modo Ventanas
es_mx.dialog_shutdown=%1 se encuentra ejecutándose. ¿Deseas que %1 sea cerrado para que la instalación pueda continuar?
es_mx.dialog_firewall=La instalación fallo al agregar la excepción de Shareaza al cortafuego Firewall.%n Por favor hágalo manualmente.
es_mx.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
es_mx.page_viruswarning_text=Cuando estas usando Internet, debes siempre asegurarte que tienes un antivirus actualizado hasta la fecha para protegerte de troyanos, gusanos, y otros programas maliciosos. Puedes encontrar una lista de buenos antivirus y sugerencias de seguridad para proteger tu computadora en la siguiente dirección:
es_mx.page_viruswarning_title=Peligro de Virus
es_mx.page_viruswarning_subtitle=¿Tienes un programa antivirus instalado?
es_mx.CreateDesktopIcon=Mostrar/Quitar icono de &Escritorio
es_mx.CreateQuickLaunchIcon=Mostrar/Quitar icono de &Inicio Rápido
; Russian
ru.components_plugins=Ìîäóëè
ru.components_skins=Øêóğêè
ru.tasks_languages=ßçûêè
ru.tasks_allusers=Âñåì ïîëüçîâàòåëÿì
ru.tasks_selectusers=Óñòàíîâèòü %1 äëÿ:
ru.tasks_currentuser=Òîëüêî äëÿ %1
ru.tasks_multisetup=Ğàçğåøèòü ïîääåğæêó íåñêîëüêèõ ïîëüçîâàòåëåé
ru.tasks_firewall=Äîáàâèòü â ñïèñîê èñêëş÷åíèé áğàíìàóığà Windows
ru.tasks_upnp=Âêëş÷èòü íàõîæäåíèå óñòğîéñâ UPnP
ru.tasks_deleteoldsetup=Óäàëèòü ñòàğûå èíñòàëëÿòîğû
ru.tasks_resetdiscoveryhostcache=Ñáğîñèòü ğåñóğñû ñîåäèíåíèé è êıø õîñòîâ
ru.run_skinexe=Èä¸ò óñòàíîâêà øêóğîê...
ru.reg_incomingchat=Âõîäÿùåå ñîîáùåíèå äëÿ ÷àòà
ru.reg_apptitle=Shareaza - ïğîãğàììà äëÿ îáìåíà ôàéëàìè
ru.icons_license=Ëèöåíçèÿ
ru.icons_uninstall=Äåèíñòàëëÿöèÿ
ru.icons_downloads=Çàãğóçêè
ru.icons_basicmode=Îáû÷íûé ğåæèì
ru.icons_tabbedmode=Ğåæèì çàêëàäîê
ru.icons_windowedmode=Îêîííûé ğåæèì
ru.dialog_shutdown=%1 â äàííûé ìîìåíò ğàáîòàåò. Õîòèòå ëè çàâåğøèòü ğàáîòó %1, ÷òîáû ïğîäîëæèòü óñòàíîâêó?
ru.dialog_firewall=Ïğîãğàììà óñòàíîâêè íå ñìîãëà äîáàâèòü Shareaza â ñïèñîê èñêëş÷åíèé áğàíäìàóığà Windows.%nÏîæàëóéñòà, äîáàâüòå åå â ıòîò ñïèñîê âğó÷íóş.
ru.dialog_malwaredetected=Â Âàøåé ñèñòåìå îáíàğóæåííû âğåäîíîñíûå ïğîãğàììû (%1), óäàëèòå èõ ïåğåä èíñòàëëÿöèåé Shareaza ïğè ïğîìîùè àíòèâèğóñíîé ïğîãğàììû èëè ñêåíåğà. Õîòèòå âûéòè ñåé÷àñ?
ru.page_viruswarning_text=Âñåãäà, êîãäà ïîëüçóåòåñü èíòåğíåòîì, óäîñòîâåğüòåñü, ÷òî ó Âàñ åñòü íîâåéøèé ñêàíåğ äëÿ âèğóñîâ, ÷òîáû çàùèòèòü êîìïüşòåğ îò òğîÿíîâ, ÷åğâåé è äğóãèõ âğåäèòåëüñêèõ ïğîãğàìì. Âû ìîæåòå íàéòè ñïèñîê õîğîøèõ ñêàíåğîâ äëÿ âèğóñîâ è äğóãèå ñîâåòû î çàùèòå êîìïüşòåğà ïî ıòîìó àäğåñó:
ru.page_viruswarning_title=Ïğåäîñòåğåæåíèå î âèğóñàõ
ru.page_viruswarning_subtitle=Èìååòå ëè Âû óñòàíîâëåííóş àíòèâèğóñíóş ïğîãğàììó?
ru.CreateDesktopIcon=Ïîêàçûâàòü çíà÷îê íà &Ğàáî÷åì ñòîëå
ru.CreateQuickLaunchIcon=Ïîêàçûâàòü çíà÷îê â &Ïàíåëè áûñòğîãî çàïóñêà
ru.PathNotExist=Îøèáêà, ïóòü ê ïàïêå %1 íå ñóùåñòâóåò
; Greek
gr.components_plugins=Plugins
gr.components_skins=Skins
gr.tasks_languages=Ãëşóóåò
gr.tasks_allusers=¼ëïé ïé ÷ñŞóôåò
gr.tasks_selectusers=ÅãêáôÜóôáóç %1 ãéá:
gr.tasks_currentuser=%1 ìüíï
gr.tasks_multisetup=Åíåñãïğïßçóç ôçò âïŞèåéáò ğïëëáğëşí ÷ñçóôşí
gr.tasks_firewall=ÂÜëå ìéá åîáßñåóç óôï ôåß÷ïò ğñïóôáóßáò ôùí Windows
gr.tasks_upnp=Enable discovery of UPnP devices
gr.tasks_deleteoldsetup=Äéİãñáøå ôçí ğáëéÜ åãêáôÜóôáóç
gr.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
gr.run_skinexe=Running ÅãêáôÜóôáóç ôïõ skin...
gr.reg_incomingchat=Íİï ìŞíõìá chat
gr.reg_apptitle=Shareaza Ultimate File Sharing
gr.icons_license=¢äåéá
gr.icons_uninstall=ÁğåãêáôÜóôáóç
gr.icons_downloads=Êáôİâáóìá
gr.icons_basicmode=ÊáíïíéêŞ ÌïñöŞ
gr.icons_tabbedmode=ĞáñáèõñéêŞ ÌïñöŞ
gr.icons_windowedmode=ÌïñöŞ Windowed
gr.dialog_shutdown=Ôï %1 áêüìá ôñİ÷åé. Èİëåôå íá ôåñìáôßóåôå ôçí ëåéôïõñãåßá ôïõ %1 ãéá íá óõíå÷éóôåß ç åãêáôÜóôáóç?
gr.dialog_firewall=Ç åãêáôÜóôáóç ôïõ ğñïãñÜììáôïò áğİôõ÷å íá ğñïóèİóåé ôï Shareaza óôï Windows Firewall. % Ğáñáêáëş ğñïóèİóôå ôï Shareaza óôçí exception ëßóôá ÷åéñïêßíçôá
gr.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
gr.page_viruswarning_text=¼ôáí ÷ñçóéìïğïéåßôå ôï internet, èá ğñİğåé ğÜíôá íá İ÷åôå İíá ğñüãñáììá ğñïóôáóßáò ãéá éïıò åíçìåñùìİíï ãéá íá óáò ğñïóôáôåıåé áğï áõôïıò êáé Üëëá åğéêßíäõíá ğñïãñÜììáôá. Ìğïñåßôå íá âñåßôå ìéá ëßóôá ìå êáëÜ ğñïãñÜììáôá ğñïóôáóßáò ãéá éïıò êáé Üëëá âïçèŞìáôá ãéá íá ğñïóôáôİøåôå ôïí õğïëïãéóôŞ óáò áêïëïõèşíôáò áõôüí ôïí óıíäåóìï:
gr.page_viruswarning_title=Ğñïåéäïğïßçóç ãéá éü
gr.page_viruswarning_subtitle=¸÷åôå İíá ğñüãñáììá ğñïóôáóßáò ãéá éïıò åãêáôåóôçìİíï?
gr.CreateDesktopIcon=ÅìöÜíéóå ôï &åéêïíßäéï óôçí åğéöÜíåéá åñãáóßáò
gr.CreateQuickLaunchIcon=ÅìöÜíéóå İíá å&éêïíßäéï ÃñŞãïñçò Åêêßíçóçò
; Hungarian
hu.components_plugins=Pluginek
hu.components_skins=Kinézetek
hu.tasks_languages=Nyelvek telepítése
hu.tasks_allusers=Minden felhasználó
hu.tasks_selectusers=Megadott felhaszáló:
hu.tasks_currentuser=Jelenlegi felhasználó
hu.tasks_multisetup=Többfelhasználós mód engedélyezése
hu.tasks_firewall=Felvétel a Windows tûzfal kivételeihez
hu.tasks_upnp=Automatikus routerbeállítás (UPnP szolgáltatás)
hu.tasks_deleteoldsetup=Régi telepítõk törlése
hu.tasks_resetdiscoveryhostcache=A Szerverkeresõ és a Kiszolgálók listájának alaphelyzetbe állítása
hu.run_skinexe=Kinézet telepítése folyamatban...
hu.reg_incomingchat=Bejövõ chat üzenet
hu.reg_apptitle=Shareaza fájlmegosztó program
hu.icons_license=Licenc
hu.icons_uninstall=Shareaza eltávolítása
hu.icons_downloads=Letöltések
hu.icons_basicmode=Egyszerû nézet
hu.icons_tabbedmode=Összetett nézet
hu.icons_windowedmode=Ablakos nézet
hu.dialog_shutdown=A %1 jelenleg fut. Be akarja zárni a programot, hogy a telepítés folytatódhasson?
hu.dialog_firewall=A telepítõ nem tudta hozzáadni a Shareazát a Windows tûzfal kivételeihez.%nManuálisan kell ezt megtenni.
hu.dialog_malwaredetected=A telepítõ egy kártékony programot talált a rendszerben: %1. Mielõtt telepítené a Shareazát, elõbb távolítsa el azt. Ki akar most lépni?
hu.page_viruswarning_text=Az internet használata során mindig legyen feltelepítve egy, a legfrissebb vírusadatbázissal rendelkezõ antivírus program, ami megvéd a férgektõl, trójai és egyéb kártékony programoktól. Ha követi ezt a linket, sok jó víruskeresõt találhat és további hasznos tippeket kaphat a számítógép védelmérõl:
hu.page_viruswarning_title=Vírusveszély
hu.page_viruswarning_subtitle=Van feltelepített antivírus programja?
hu.CreateDesktopIcon=Ikon létrehozása az &Asztalon
hu.CreateQuickLaunchIcon=Ikon létrehozása a &Gyorsindítás eszköztáron
hu.PathNotExist=Hiba, a megadott %1 mappa nem létezik
; Chinese Simp
chs.components_plugins=²å¼ş
chs.components_skins=Æ¤·ô
chs.tasks_languages=ÓïÑÔ
chs.tasks_allusers=ËùÓĞÓÃ»§
chs.tasks_selectusers=°²×° %1 Îª:
chs.tasks_currentuser=½ö %1
chs.tasks_multisetup=ÆôÓÃ¶àÓÃ»§Ö§³Ö
chs.tasks_firewall=Ìí¼ÓÒ»¸öÀıÍâµ½ Windows ·À»ğÇ½
chs.tasks_upnp=ÆôÓÃ UPnP Çı¶¯Æ÷²éÑ¯
chs.tasks_deleteoldsetup=É¾³ı¾ÉµÄ°²×°ÎÄ¼ş
chs.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
chs.run_skinexe=°²×°Æ¤·ô...
chs.reg_incomingchat=´«ÈëÁÄÌìÏûÏ¢
chs.reg_apptitle=Shareaza ÖÕ¼«ÎÄ¼ş¹²Ïí
chs.icons_license=Ğí¿É
chs.icons_uninstall=Ğ¶ÔØ
chs.icons_downloads=ÏÂÔØ
chs.icons_basicmode=ÆÕÍ¨Ä£Ê½
chs.icons_tabbedmode=±êÇ©Ä£Ê½
chs.icons_windowedmode=ÊÓ´°Ä£Ê½
chs.dialog_shutdown=%1 ÕıÔÚÔËĞĞ¡£ÄúÏ£Íû¹Ø±Õ %1 ÒÔ±ã¼ÌĞø°²×°Âğ£¿
chs.dialog_firewall=°²×°Ìí¼Ó Shareaza µ½ Windows ·À»ğÇ½Ê§°Ü¡£%nÇë½« Shareaza ÊÖ¶¯Ìí¼ÓÖÁ³ıÍâÁĞ±í¡£
chs.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
chs.page_viruswarning_text=ÔÚÊÔÓÃ»¥ÁªÍøÊ±£¬ÄúĞèÒªÈ·¶¨ÄúÓµÓĞ×îĞÂµÄ²¡¶¾É¨ÃèÈí¼şÒÔ±£»¤ÄúÃâÊÜÄ¾Âí¡¢Èä³æºÍÆäËû¶ñÒâ³ÌĞòµÄÇÖº¦¡£Äú¿ÉÒÔÔÚÒÔÏÂÁ´½ÓÖĞÕÒµ½½ÏºÃµÄ²¡¶¾É¨ÃèÈí¼şµÄÁĞ±íÒÔ¼°ÆäËû±£»¤ÄúµÄ¼ÆËã»úµÄ°²È«½¨Òé:
chs.page_viruswarning_title=²¡¶¾¾¯¸æ
chs.page_viruswarning_subtitle=Äú°²×°ÁË·À²¡¶¾Èí¼şÁËÂğ£¿
chs.CreateDesktopIcon=ÏÔÊ¾×ÀÃæÍ¼±ê(&D)
chs.CreateQuickLaunchIcon=ÏÔÊ¾¿ìËÙÆô¶¯À¸Í¼±ê(&Q)
; Swedish
sv.components_skins=Skinn
sv.tasks_languages=Språk
sv.tasks_allusers=Alla användare
sv.tasks_selectusers=Installera %1 för:
sv.tasks_currentuser=%1 endast
sv.tasks_multisetup=Aktivera stöd för flera användare
sv.tasks_firewall=Lägg till ett undantag till Windows brandvägg
sv.tasks_upnp=Enable discovery of UPnP devices
sv.tasks_deleteoldsetup=Delete old installers
sv.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
sv.run_skinexe=Kör skinninstallation...
sv.reg_incomingchat=Inkommande chattmeddelande
sv.reg_apptitle=Shareaza ultimat fildelning
sv.icons_license=Licens
sv.icons_uninstall=Avinstallera
sv.icons_downloads=Nedladdningar
sv.icons_basicmode=Normalt läge
sv.icons_tabbedmode=Tabbläge
sv.icons_windowedmode=Fönsterläge
sv.dialog_shutdown=%1 körs för tillfället. Vill du att %1 ska stängas ned så att installationen kan fortsätta?
sv.dialog_firewall=Setup failed to add Shareaza to the Windows Firewall.%nPlease add Shareaza to the exception list manually.
sv.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
sv.page_viruswarning_text=När du använder internet ska du alltid försäkra dig om att du har ett uppdaterat antivirusprogram som skyddar dig mot trojaner, maskar och andra skadliga program. Här finns en lista på bra antivirusprogram och andra säkerhetstips för att skydda din dator:
sv.page_viruswarning_title=Virusvarning
sv.page_viruswarning_subtitle=Har du ett antivirusprogram installerat?
sv.CreateDesktopIcon=Skapa en ikon på srivbordet
sv.CreateQuickLaunchIcon=Skapa en ikon i Snabbstartfältet
; Finnish
fi.components_plugins=Laajennukset
fi.components_skins=Ulkoasut
fi.tasks_languages=Kielet
fi.tasks_allusers=Kaikille käyttäjille
fi.tasks_selectusers=Asenna %1 käyttäjille:
fi.tasks_currentuser=%1 vain
fi.tasks_multisetup=Asenna kaikille koneen käyttäjille
fi.tasks_firewall=Lisää poikkeus Windowsin palomuuriin
fi.tasks_upnp=Enable discovery of UPnP devices
fi.tasks_deleteoldsetup=Poista vanhat asennusohjelmat
fi.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
fi.run_skinexe=Käynnissä ulkoasujen asennus...
fi.reg_incomingchat=Tuleva keskusteluviesti
fi.reg_apptitle=Shareaza jako-ohjelma
fi.icons_license=Lisenssi
fi.icons_uninstall=Poista
fi.icons_downloads=Lataukset
fi.icons_basicmode=Normaali Tila
fi.icons_tabbedmode=Välilehti Tila
fi.icons_windowedmode=Ikkunoitu Tila
fi.dialog_shutdown=%1 on tällä hetkellä käynnissä. Haluatko että %1 suljetaan jotta asennus voisi jatkua?
fi.dialog_firewall=Asentaja epäonnistui lisätessään Shareazaa Windowsiin Firewall.%nOle hyvä ja lisää Shareaza poikkeuslistaan manuaalisesti.
fi.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
fi.page_viruswarning_text=Kun käytät internetiä, sinun tulee aina varmistaa että sinulla on viimeisimmät päivitykset virusohjelmissasi jotka suojaavat sinua troijalaisilta, madoilta, ja muilta haittaohjelmilta. Löydät hyvän listan hyvistä virusohjelmista ja turvallisuusvinkkejä seuraavista linkeistä:
fi.page_viruswarning_title=Virus Varoitus
fi.page_viruswarning_subtitle=Onko sinulla AntiVirus ohjelmaa asennettuna?
fi.CreateDesktopIcon=Luo kuvake työpöydälle
fi.CreateQuickLaunchIcon=Luo kuvake pikakäynnistyspalkkiin
; Hebrew
heb.components_plugins=úåñôéí
heb.components_skins=îòèôåú
heb.tasks_languages=ùôåú
heb.tasks_allusers=ëì äîùúîùéí
heb.tasks_selectusers=äú÷ï àú %1 òáåø
heb.tasks_currentuser=%1 ø÷
heb.tasks_multisetup=àôùø úîéëä ìîùúîùéí îøåáéí
heb.tasks_firewall=äåñó ëìì ìçåîú äàù ìàéôùåø ùøæä
heb.tasks_upnp=Enable discovery of UPnP devices
heb.tasks_deleteoldsetup=îç÷ äú÷ğåú éùğåú
heb.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
heb.run_skinexe=îøéõ äú÷ğú îòèôåú...
heb.reg_incomingchat=äåãòú ö'àè ğëğñú
heb.reg_apptitle=ùéúåó ä÷áöéí äàåìèéîèéáé ùì ùøæä
heb.icons_license=øùéåï
heb.icons_uninstall=äñø äú÷ğä
heb.icons_downloads=äåøãåú
heb.icons_basicmode=îöá øâéì
heb.icons_tabbedmode=îöá èàáéí
heb.icons_windowedmode=îöá çìåğåú
heb.dialog_shutdown=?ëøâò %1 ôåòì äàí àú øåöä ìñâåø àú %1 ëê ùääú÷ğä úåëì ìäîùê
heb.dialog_firewall=ääú÷ğä ğëùìä ìäåñéó àú ùøæä àì çåîú äàù%nàğà äåñó àú ùøæä ìøùéîú äçøéâéí áçåîú äàù áàåôï éãğé
heb.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
heb.page_viruswarning_text=ëùàú/ä îùúîù áàéğèøğè òìééê úîéã ìååãà ùîåú÷ï àöìê àğèé-åéøåñ îòåãëï ìäâğä îôğé åéøåñéí/úåìòéí/øåâìåú, ìøùéîä ùì àğèé-åéøåñéí åèéôéí áğåùà àáèçä ôğä ì÷éùåø äáà:
heb.page_viruswarning_title=äúøàú åéøåñ
heb.page_viruswarning_subtitle=?äàí éù ìê úåëğú àğèé-åéøåñ îåú÷ğú
heb.CreateDesktopIcon= äöâ ñîì òì ù&åìçï òáåãä
heb.CreateQuickLaunchIcon=äöâ ñîì òì ä&ôòìä îäéøä
; Polish
pl.components_plugins=Wtyczki
pl.components_skins=Skórki
pl.tasks_languages=Jêzyki
pl.tasks_allusers=Dla wszystkich u¿ytkowników
pl.tasks_selectusers=Instaluj dla %1:
pl.tasks_currentuser=tylko %1
pl.tasks_multisetup=W³¹cz obs³ugê wielu u¿ytkowników
pl.tasks_firewall=Dodaj wyj¹tek do Firewall'a Windows'a
pl.tasks_upnp=Enable discovery of UPnP devices
pl.tasks_deleteoldsetup=Usuñ stare instalatory
pl.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
pl.run_skinexe=Instalowanie skórek...
pl.reg_incomingchat=Przychodz¹ca wiadomoœæ chatowa
pl.reg_apptitle=Shareaza Ultimate File Sharing
pl.icons_license=Licencja
pl.icons_uninstall=Odinstaluj
pl.icons_downloads=Pobierania
pl.icons_basicmode=Tryb normalny
pl.icons_tabbedmode=Tryb zaawansowany
pl.icons_windowedmode=Tryb okienkowy
pl.dialog_shutdown=%1 obecnie dzia³a. Czy chcia³byœ wy³¹czyæ %1 aby kontynuowaæ instalacjê?
pl.dialog_firewall=Instalator nie potrafi³ dodaæ Shareazy do Firewall'a Windows'a.%nProszê dodaæ Shareazê do listy wyj¹tków rêcznie.
pl.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
pl.page_viruswarning_text=Podczas u¿ywania internetu zawsze powinieneœ byæ pewny, ¿e masz program antywirusowy z aktualn¹ baz¹ wirusów, który ciê ochroni przed trojanami, robakami i innym niebezpiecznym oprogramowaniem. Mo¿esz znaleŸæ listê dobrych programów antywirusowych i porady jak zabezpieczyæ komputer pod nastêpuj¹cymi adresami:
pl.page_viruswarning_title=Ostrze¿enie przed wirusem
pl.page_viruswarning_subtitle=Czy masz zainstalowany jakiœ program antywirusowy?
pl.CreateDesktopIcon=Wyœwietl ikonê na pulpicie
pl.CreateQuickLaunchIcon=Wyœwietl ikonê na pasku szybkiego uruchamiania
; Serbian
sr.components_plugins=Pluginovi
sr.components_skins=Skinovi
sr.tasks_languages=Jezici
sr.tasks_allusers=Svi korisnici
sr.tasks_selectusers=Instaliraj %1 za:
sr.tasks_currentuser=%1 samo
sr.tasks_multisetup=Omoguæi više-korisnièku podršku
sr.tasks_firewall=Dodaj izuzetak u Windows Vatrozid
sr.tasks_upnp=Enable discovery of UPnP devices
sr.tasks_deleteoldsetup=Ukloni stare instalere
sr.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
sr.run_skinexe=Instalacija skina u toku...
sr.reg_incomingchat=Dolazeæe cet poruke
sr.reg_apptitle=Shareaza Najbolji P2P za Deljenje Datoteka
sr.icons_license=Licenca
sr.icons_uninstall=Ukloni Program
sr.icons_downloads=Skinutno
sr.icons_basicmode=Normalni Prikaz
sr.icons_tabbedmode=Prikaz s Karticama
sr.icons_windowedmode=U Prozoru
sr.dialog_shutdown=%1 je ukljuèena. Da li bi eleli da %1 bude ugašena da bi se instalacija nastavila?
sr.dialog_firewall=Instalacija nije uspla da doda Shareaza-u u Windows Vatrozid.%nMolimo dodajte Shareaza-u na listu izuzetaka ruèno.
sr.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
sr.page_viruswarning_text=Kada koristite internet, trebali bi uvek da budete sigurni da imate auriaran virus skener koji Vas štiti od trojanaca, crva, i drugih zlonamernih programa. Moete pronaæi listu dobrih anti-virus programa i drugih sigurnosnih saveta kako da zaštitite Vaš raèunar prateæi ovaj link:
sr.page_viruswarning_title=Virus Uopzorenje
sr.page_viruswarning_subtitle=Da li imate AntiVirus program instaliran?
sr.CreateDesktopIcon=Napravi &desktop ikonu
sr.CreateQuickLaunchIcon=Napravi &Brzo Pokretanje(QL) ikonu
;Turkish
tr.components_plugins=Eklentiler
tr.components_skins=Arayüzler
tr.tasks_languages=Diller
tr.tasks_allusers=Tüm Kullanıcılar
tr.tasks_selectusers=%1 Kuruldu:
tr.tasks_currentuser=Sadece %1
tr.tasks_multisetup=Çoklu kullanıcı desteğini etkinleştir
tr.tasks_firewall=Windows Güvenlik Duvarına bir değişiklik ekle
tr.tasks_upnp=Enable discovery of UPnP devices
tr.tasks_deleteoldsetup=Eski kurulumları sil
tr.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
tr.run_skinexe=Arayüz kurulumu çalışıyor...
tr.reg_incomingchat=Gelen sohbet mesajı
tr.reg_apptitle=Shareaza En iyi Dosya Paylaşımı
tr.icons_license=Lisans
tr.icons_uninstall=Kurulumu Kaldır
tr.icons_downloads=İndirmeler
tr.icons_basicmode=Normal Mod
tr.icons_tabbedmode=Sekmeli Mod
tr.icons_windowedmode=Pencereli Mode
tr.dialog_shutdown=Şuan %1 çalışıyor.Kurulumun devam edebilmesi için %1'in kapalı olmasını istiyor musunuz?
tr.dialog_firewall=Windows güvenlik duvarına Shareaza kurulumunu eklemek başarısız oldu.%n Lütfen Shareaza'yı el ile istisna listesine ekle
tr.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
tr.page_viruswarning_text=İnternet kullanıyorken, trojanlardan, wormlardan ve diğer kötü niyetli programlardan sizi koruyan güncel bir virüs tarayıcısına sahip olduğunuzdan emin olmalısınız. Bu bağlantıyı izleyerek bilgisayarınızı koruyan iyi virüs tarayıcılarının ve diğer güvenlik tiplerinin listesini bulacaksınız:
tr.page_viruswarning_title=Virüs Uyarısı
tr.page_viruswarning_subtitle=Bir AntiVirus programı yüklediniz mi?
tr.CreateDesktopIcon=Bir &Masaüstü ikonu görüntüle
tr.CreateQuickLaunchIcon=Bir &Hızlı Başlat ikonu görüntüle
; Czech
cz.components_plugins=Doplòky
cz.components_skins=Skiny
cz.tasks_languages=Více jazykù
cz.tasks_allusers=všechny uivatele
cz.tasks_selectusers=Instalovat %1 pro:
cz.tasks_currentuser=jen %1
cz.tasks_multisetup=Povolit pøístup více uivatelù
cz.tasks_firewall=Pøidat pravidlo do Windows firewallu
cz.tasks_upnp=Povolit zjišování UPnP zaøízení
cz.tasks_deleteoldsetup=Smazat staré instalátory
cz.tasks_resetdiscoveryhostcache=Obnovit seznamy serverù
cz.run_skinexe=Spouštím instalaci skinù...
cz.reg_incomingchat=Pøíchozí zpráva chatu
cz.reg_apptitle=Shareaza Ultimate File Sharing
cz.icons_license=Licence
cz.icons_uninstall=Odinstalovat
cz.icons_downloads=Staené soubory
cz.icons_basicmode=Normální reim
cz.icons_tabbedmode=Reim záloek
cz.icons_windowedmode=Reim oken
cz.dialog_shutdown=Aplikace %1 je spuštìna. Chcete ukonèit %1 a pokraèovat v instalaci?
cz.dialog_firewall=Instalátor nemùe pøidat pravidlo pro Shareazu do nastavení Windows Firewallu.%nProsím pøidejte pravidlo ruènì.
cz.dialog_malwaredetected=Byl nalezen škodlivı software (malware) v %1. Pøed pokraèování instalace jej prosím odstraòte pomocí vhodného nástroje pro odstraòování škodlivého softwaru. Ukonèit instalaci?
cz.page_viruswarning_text=Pøi pouívání internetu se vdy ujistìte, e máte aktuální antivirovı program, kterı vás chrání pøed nebezpeènımi a škodlivımi programy. Seznam vhodnıch programù a další rady tıkající se bezpeènosti mùete nalézt na následující adrese:
cz.page_viruswarning_title=Nalezen virus
cz.page_viruswarning_subtitle=Máte nainstalovanı antivirovı program?
cz.CreateDesktopIcon=Zobrazovat ikonu na &ploše
cz.CreateQuickLaunchIcon=Zobrazovat ikonu v panelu &Snadné spouštìní
; japanese
ja.components_plugins=ƒvƒ‰ƒOƒCƒ“
ja.components_skins=ƒXƒLƒ“
ja.tasks_languages=Œ¾Œêƒtƒ@ƒCƒ‹
ja.tasks_allusers=‚·‚×‚Ä‚Ìƒ†[ƒU[
ja.tasks_selectusers=%1‚ğ‚µ‚æ‚¤‚·‚éƒ†[ƒU[:
ja.tasks_currentuser=%1‚Ì‚İ
ja.tasks_multisetup=ƒ}ƒ‹ƒ`ƒ†[ƒU[ƒTƒ|[ƒg
ja.tasks_firewall=Windowsƒtƒ@ƒCƒ„[ƒEƒH[ƒ‹‚Ì—áŠO‚Éİ’è
ja.tasks_upnp=UPnP‘Î‰‹@Ší‚ÌŒŸo‚ğ—LŒø‚É‚·‚é
ja.tasks_deleteoldsetup=ŒÃ‚¢ƒCƒ“ƒXƒg[ƒ‰[‚Ìíœ
ja.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
ja.run_skinexe=ƒXƒLƒ“ƒCƒ“ƒXƒg[ƒ‰[‚ğÀs‚µ‚Ä‚¢‚Ü‚·...
ja.reg_incomingchat=ƒ`ƒƒƒbƒgƒƒbƒZ[ƒW‚ğó‚¯“ü‚ê‚é
ja.reg_apptitle=Shareazaƒtƒ@ƒCƒ‹‹¤—Lƒ\ƒtƒg
ja.icons_license=ƒ‰ƒCƒZƒ“ƒX
ja.icons_uninstall=ƒAƒ“ƒCƒ“ƒXƒg[ƒ‹
ja.icons_downloads=ƒ_ƒEƒ“ƒ[ƒh
ja.icons_basicmode=•W€ƒ‚[ƒh
ja.icons_tabbedmode=ƒ^ƒuƒ‚[ƒh
ja.icons_windowedmode=ƒEƒBƒ“ƒhƒEƒ‚[ƒh
ja.dialog_shutdown=%1 ‚ªŠˆ“®’†‚Å‚·. %1‚ğI—¹‚µ‚ÄƒCƒ“ƒXƒg[ƒ‹‚ğ‘±‚¯‚Ü‚·‚©?
ja.dialog_firewall=WindowsXPƒtƒ@ƒCƒ„[ƒEƒH[ƒ‹‚Ì“o˜^‚É¸”s‚µ‚Ü‚µ‚½.%nè“®‚Å“o˜^‚µ‚Ä‚­‚¾‚³‚¢.
ja.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
ja.page_viruswarning_text=‚ ‚È‚½‚ªƒCƒ“ƒ^[ƒlƒbƒg‚ÉÚ‘±‚·‚é‚Æ‚«‚ÍAƒgƒƒC‚âƒ[ƒ€–”‚ÍA‚»‚êˆÈŠO‚ÌŠëŒ¯‚Èƒtƒ@ƒCƒ‹‚©‚çPC‚ğ•ÛŒì‚·‚é‚½‚ß‚ÉA•K‚¸ƒEƒCƒ‹ƒX’è‹`ƒtƒ@ƒCƒ‹‚ğÅV‚Ì‚à‚Ì‚É‚µ‚Ü‚µ‚å‚¤BƒEƒCƒ‹ƒXƒXƒLƒƒƒi[‚âƒZƒLƒ…ƒŠƒeƒB-‚ÉŠÖ‚·‚éî•ñ‚ª‰º‹L‚ÌƒŠƒ“ƒN‚É‚ ‚è‚Ü‚·B
ja.page_viruswarning_title=ƒEƒCƒ‹ƒX‚ÌŒx
ja.page_viruswarning_subtitle=ƒAƒ“ƒ`EƒEƒCƒ‹ƒXEƒ\ƒtƒg‚Í“ü‚Á‚Ä‚¢‚Ü‚·‚©?
ja.CreateDesktopIcon=ƒfƒXƒNƒgƒbƒv‚ÉƒAƒCƒRƒ“‚ğ•\¦(&d)
ja.CreateQuickLaunchIcon=ƒNƒCƒbƒNƒ‰ƒ“ƒ`‚ÉƒAƒCƒRƒ“‚ğ•\¦(&Q)
; arabic
ar.components_plugins=ÇáæÙÇÆİ ÇáÅÖÇİíÉ
ar.components_skins=ÇáÛáÇİÇÊ
ar.tasks_languages=ÇááÛÇÊ
ar.tasks_allusers=ßÇİÉ ÇáãÓÊÎÏãíä
ar.tasks_selectusers=ãä ÃÌá %1 ÊÑßíÈ:
ar.tasks_currentuser=%1 İŞØ
ar.tasks_multisetup=Êãßíä ÇáÏÚã áÚÏÉ ãÓÊÎÏãíä
ar.tasks_firewall=ÅÖÇİÉ ÅÓÊËäÇÁ Åáì ÌÏÇÑ ÇáæíäÏæÒ ÇáäÇÑí
ar.tasks_upnp=Enable discovery of UPnP devices
ar.tasks_deleteoldsetup=ÍĞİ ãáİÇÊ ÇáÊÑßíÈ ÇáŞÏíãÉ
ar.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
ar.run_skinexe=...íÊã ÊÔÛíá ÊÑßíÈ ÇáÛáÇİ
ar.reg_incomingchat=ÑÓÇáÉ ãÍÇÏËÉ æÇÑÏÉ
ar.reg_apptitle=ÔíÑ-ÃÒÇ ááãÔÇÑßÉ ÇáÛíÑ ãÍÏæÏÉ ÈÇáãáİÇÊ
ar.icons_license=ÇáÑÎÕÉ
ar.icons_uninstall=ÅáÛÇÁ ÇáÊËÈíÊ
ar.icons_downloads=ÇáÊÍãíáÇÊ
ar.icons_basicmode=äÙÇã ÚÇÏí
ar.icons_tabbedmode=äÙÇã ãÊŞÏã
ar.icons_windowedmode=äÙÇã Ğæ äæÇİĞ
ar.dialog_shutdown=%1 íÚãá ÍÇáíÇ . åá ÊÑíÏ ÅÛáÇŞ %1 áíÓÊãÑ ÇáÊÑßíÈ ¿
ar.dialog_firewall=İÔá ÇáÅÚÏÇÏ İí ÅÖÇİÉ ÔíÑ-ÃÒÇ Åáì ÈÑäÇãÌ ÍãÇíÉ ÇáæíäÏæÒ %nÇáÑÌÇÁ ÅÖÇİÉ ÔíÑ-ÃÒÇ Åáì ŞÇÆãÉ ÇáÅÓÊËäÇÁÇÊ íÏæíÇ
ar.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
ar.page_viruswarning_text=ÚäÏãÇ ÅÓÊÚãÇá ÇáÅäÊÑäÊ ¡ íÌÈ Ãä ÊÊÃßÏ ãä æÌæÏ ÃÍÏË ÈÑäÇãÌ ááİíÑæÓ áÏíß . íãßäß ÇáÍÕæá Úáì ÈÑÇãÌ ááİíÑæÓÇÊ æ äÕÇÆÍ ÃãäíÉ ÃÎÑì áÍãÇíÉ ßãÈíæÊÑß ãä åĞå ÇáæÕáÉ:
ar.page_viruswarning_title=ÊÍĞíÑ ãä ÇáİíÑæÓÇÊ
ar.page_viruswarning_subtitle=åá ÚäÏß ÈÑäÇãÌ ááİíÑæÓÇÊ ¿
ar.CreateDesktopIcon=ÅÙåÇÑ &ÃíŞæäÉ ÓØÍ ÇáãßÊÈ
ar.CreateQuickLaunchIcon=ÅÙåÇÑ &ÃíŞæäÉ ÇáÊÔÛíá ÇáÓÑíÚ
; estonian
ee.components_plugins=Pluginad
ee.components_skins=Nahad
ee.tasks_languages=Keeled
ee.tasks_allusers=Kõik kasutajad
ee.tasks_selectusers=Installi %1 jaoks:
ee.tasks_currentuser=%1 ainult
ee.tasks_multisetup=Võimalda mitmekasutaja tugi
ee.tasks_firewall=Lisa erand Windowsi Tulemüüri
ee.tasks_upnp=Enable discovery of UPnP devices
ee.tasks_deleteoldsetup=Kustuta vanad installerid
ee.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
ee.run_skinexe=Käivitan Naha installi...
ee.reg_incomingchat=Sisse tulev vestlusteade
ee.reg_apptitle=Shareaza Ülim Failijagamine
ee.icons_license=Litsents
ee.icons_uninstall=Uninstalli
ee.icons_downloads=Tõmbamised
ee.icons_basicmode=Tavaline Vaade
ee.icons_tabbedmode=Sakiline Vaade
ee.icons_windowedmode=Akendega Vaade
ee.dialog_shutdown=%1 töötab hetkel. Kas tahad  %1 sulgeda, et saaksid installeerimist jätkata?
ee.dialog_firewall=Installeril ebaõnnestus Shareaza lisamine Windowsi Tulemüüri.%Palun lisa Shareaza käsitsi erandite nimekirja.
ee.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
ee.page_viruswarning_text=Internetti kasutades peaksid kontrollima, et sul oleks uusim viirusetõrje, et kaitsta ennast troojalaste, usside, viiruste ja teiste kahjulike programmide eest. Sa leiad nimekirja headest viirus skänneritest ja teisi turva nõuandeid oma arvuti kaitseks sellelt lingilt:
ee.page_viruswarning_title=Viiruse Hoiatus
ee.page_viruswarning_subtitle=Kas sul on AntiVirus programm installeeeritud?
ee.CreateDesktopIcon=Loo &Töölaua ikoon
ee.CreateQuickLaunchIcon=Loo &Quick Launch ikoon
; Chinese Trad
tw.components_plugins=¥~±¾
tw.components_skins=¥~Æ[
tw.tasks_languages=»y¨¥
tw.tasks_allusers=©Ò¦³ªº¨Ï¥ÎªÌ
tw.tasks_selectusers=¬°³o¨Ç¨Ï¥ÎªÌ¦w¸Ë %1:
tw.tasks_currentuser=¥u¦³ %1
tw.tasks_multisetup=±Ò¥Î¦h­«¨Ï¥ÎªÌ¤ä´©
tw.tasks_firewall=¼W¥[¨Ò¥~¦Ü Windows ¨¾¤õÀğ (XP)
tw.tasks_upnp=Enable discovery of UPnP devices
tw.tasks_deleteoldsetup=§R°£ÂÂªº¦w¸Ëµ{¦¡
tw.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
tw.run_skinexe=¥¿¦b¦w¸Ë¥~Æ[...
tw.reg_incomingchat=¿é¤Jªº²á¤Ñ°T®§
tw.reg_apptitle=Shareaza---³Ì²×ªºÀÉ®×¤À¨É³nÅé
tw.icons_license=³nÅé±ÂÅv¨ó©w
tw.icons_uninstall=¸Ñ°£¦w¸Ë
tw.icons_downloads=¤U¸ü
tw.icons_basicmode=¼Ğ·Ç¼Ò¦¡
tw.icons_tabbedmode=¼ĞÅÒ¼Ò¦¡
tw.icons_windowedmode=¦h­«µøµ¡¼Ò¦¡
tw.dialog_shutdown=%1 ¥¿¦b¹B§@¤¤. ±z­nÃö³¬ %1 , Åı¦w¸Ëµ{¦¡±o¥HÄ~Äò¶i¦æ¶Ü?
tw.dialog_firewall=¦w¸Ëµ{¦¡µLªk·s¼W Shareaza ¦ÜWindows ¨¾¤õÀğ.%n½Ğ¤â°Ê·s¼W Shareaza ¦Ü¨Ò¥~²M³æ
tw.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
tw.page_viruswarning_text=¨Ï¥Îºô»Úºô¸ô®É, ±zÀ³¸ÓÁ`¬O½T«O¨¾¬r³nÅé¬°³Ì·sª©¥», ¦p¦¹¤~¯à«OÅ@±zÁ×§K¨ü¨ì¤ì°¨, Ä¯ÂÎ, ©Î¬O´c·Nµ{¦¡ªº«I®`. ±z¥i¥HÂI¿ï³o­Ó³sµ²¨ú±o¦w¥ş©Ê¯µ³Z»P¨}¦n¨¾¬r³nÅéªº²M³æ:
tw.page_viruswarning_title=¯f¬rÄµ§i
tw.page_viruswarning_subtitle=±z¬O§_¤w¸g¦w¸Ë¤F¤@­Ó¨¾¬r³nÅé?
tw.CreateDesktopIcon=½Ğ·s¼W¤@­Ó&®à­±¹Ï¥Ü
tw.CreateQuickLaunchIcon=½Ğ·s¼W¤@­Ó&§Ö³t±Ò°Ê¹Ï¥Ü
; Slovenian
sl.components_plugins=Vtièniki
sl.components_skins=Preobleke
sl.tasks_languages=Jeziki
sl.tasks_allusers=Vsi uporabniki
sl.tasks_selectusers=Namesti %1 za:
sl.tasks_currentuser=Samo %1
sl.tasks_multisetup=Omogoèi veè-uporabniško podporo
sl.tasks_firewall=Dodaj izjemo v Windows poarni zid
sl.tasks_upnp=Enable discovery of UPnP devices
sl.tasks_deleteoldsetup=Briši stare namešèevalce
sl.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
sl.run_skinexe=Namestitev preobleke v teku...
sl.reg_incomingchat=Vhodno kramljalno sporoèilo
sl.reg_apptitle=Izjemno Shareaza deljenje datotek
sl.icons_license=Licenca
sl.icons_uninstall=Odnamestitev
sl.icons_downloads=Prenosi
sl.icons_basicmode=Navadni naèin
sl.icons_tabbedmode=Naèin z zavihki
sl.icons_windowedmode=Naèin z okni
sl.dialog_shutdown=%1 je trenutno v teku. Ali elite zapreti %1 zato, da se lahko nadaljuje namestitev?
sl.dialog_firewall=Program namestitve ni uspel dodati Shareaze v poarni zid Windows-ov.%nShareazo boste morali roèno dodati v seznam izjem v poarnem zidu.
sl.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
sl.page_viruswarning_text=Pri uporabi medmreja imejte namešèeno vedno najnovejšo razlièico protivirusne zašèite. Tako boste kar najbolje zašèiteni pred trojanskimi konji, èrvi in drugimi zlonamernimi programji. Seznam dobrih protivirusnih programov, ter drugih nasvetov glede zašèite vašega raèunalnika, boste našli preko naslednje spletne povezave:
sl.page_viruswarning_title=Virusno opozorilo
sl.page_viruswarning_subtitle=Ali imate namešèen protivirusni program?
sl.CreateDesktopIcon=Prikai ikono &namizja
sl.CreateQuickLaunchIcon=Prikai ikono &Hitrega zaganjalnika
; Catalan
ca.components_plugins=Agregats (plug-in)
ca.components_skins=Pells (skins)
ca.tasks_languages=Idiomes
ca.tasks_allusers=Tots els usuaris
ca.tasks_selectusers=Instal·lar %1 per a:
ca.tasks_currentuser=%1 únicament
ca.tasks_multisetup=Habilitar el suport multi-usuari
ca.tasks_firewall=Afegeix una excepció al tallafocs de Windows
ca.tasks_upnp=Habilitar el descobriment de dispositius UPnP
ca.tasks_deleteoldsetup=Esborrar instal·ladors antics
ca.tasks_resetdiscoveryhostcache=Reset Discovery and Hostcache
ca.run_skinexe=Executant la instal·lació de la pell (skin)...
ca.reg_incomingchat=Missatge de xat entrant
ca.reg_apptitle=Shareaza: compartició d'arxius d'última generació
ca.icons_license=Llicència
ca.icons_uninstall=Desinstal·lació
ca.icons_downloads=Descàrregues
ca.icons_basicmode=Mode normal
ca.icons_tabbedmode=Mode en pestanyes
ca.icons_windowedmode=Mode de finestra
ca.dialog_shutdown=%1 està sent executat. Dessitja que %1 siga aturat per que la instal·lació puga continuar?
ca.dialog_firewall=La instal·lació ha fallat mentre s'afegia una exepció al tallafocs del Windows.%nSi us plau, afegeixi Shareaza al llistat d'excepcions manualment.
ca.dialog_malwaredetected=A malware has been detected on your system at %1, please remove it with a virus/malware scanner before installing Shareaza. Do you want to exit now?
ca.page_viruswarning_text=Mentre utilitzes Internet, has d'assegurar-te que tens un antivirus actualitzat per protegir-te de troians, cucs, virus i altres programes maliciosos. Pots consultar un llistat de programari antivirus i consells de seguretat fent clic a la següent drecera:
ca.page_viruswarning_title=Advertència de virus
ca.page_viruswarning_subtitle=Tens un programa antivíric instal·lat?
ca.CreateDesktopIcon=Afegeix una icona a l'&escriptori
ca.CreateQuickLaunchIcon=Afegeix una icona a l’&escriptori
; Albanian
sq.components_plugins=Prizat
sq.components_skins=Lëvoret
sq.tasks_languages=Gjuhët
sq.tasks_allusers=Gjithë përdoruesit
sq.tasks_selectusers=Instalo %1 për:
sq.tasks_currentuser=%1 vetëm
sq.tasks_multisetup=Lejo përkrahjen me shumë përdorues
sq.tasks_firewall=Shto një përjashtim në Fajruollin e Windows
sq.tasks_upnp=Lejo zbulimin e mjeteve UPnP
sq.tasks_deleteoldsetup=Fshiji instaluesit e vjetër
sq.tasks_resetdiscoveryhostcache=Rivendose Zbulimin dhe Depon me Hostet
sq.run_skinexe=Po vepron instalimi i lëvores...
sq.reg_incomingchat=Mesazhi i ardhur në chat
sq.reg_apptitle=Shareaza - Shpërndarja Bazë e Skedave
sq.icons_license=Licensë
sq.icons_uninstall=Çinstalo
sq.icons_downloads=Shkarkesat
sq.icons_basicmode=Mënyrë e Thjeshtë
sq.icons_tabbedmode=Mënyrë e Lidhur
sq.icons_windowedmode=Mënyrë e Dritarizuar
sq.dialog_shutdown=%1 aktualisht po vepron. A doni të mbyllet %1, që të vijoni me instalimin?
sq.dialog_firewall=Ndërtimi dështoi ta shtojë Shareaza-n në Fajruollin e Windows.%nJu lutemi shtoni Shareaza-n vetë në listën me përjashtimet.
sq.dialog_malwaredetected=Te sistemi juaj në %1 diktohet një keqbërës. Ju lutemi fshijeni atë me një skanues virusi/keqbërësi para se Shareaza të instalohet. A doni të dilni tani?
sq.page_viruswarning_text=Kur përdorni internetin duhet të siguroheni që keni një skanues virusi të ridatuar, në mënyrë që të mbroheni nga trojanet, krimbat dhe programe të tjera keqbërëse. Duke ndjekur këtë link mund të gjeni një listë me skanuesa virusi të mirë dhe këshilla të tjera sigurie se si të mbroni kompjuterin:
sq.page_viruswarning_title=Kujdes Viruset
sq.page_viruswarning_subtitle=A keni instaluar një program AntiVirus?
sq.CreateDesktopIcon=Shfaqe një ikonë në &desktop
sq.CreateQuickLaunchIcon=Shfaqe një ikonën në &Quick Launch
sq.PathNotExist=Gabim, shtegu i dosjes së %1 nuk ekziston!
