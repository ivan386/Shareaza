; This sub-script defines all languages to be compiled
; WARNING: Do not change this file's encoding

[Languages]
; Use compiler's built in ISL file to patch up holes in ISL collection and specify localized license and readme files
; Note: first language specified is default > English

Name: "en"; MessagesFile: "compiler:Default.isl,setup\isl\default.isl"; LicenseFile: "setup/license/default.rtf"
Name: "nl"; MessagesFile: "compiler:Default.isl,setup\isl\dutch.isl"; LicenseFile: "setup/license/dutch.rtf"
Name: "lt"; MessagesFile: "compiler:Default.isl,setup\isl\lithuanian.isl"; LicenseFile: "setup/license/lithuanian.rtf"
Name: "de"; MessagesFile: "compiler:Default.isl,setup\isl\german.isl"; LicenseFile: "setup/license/German.rtf"

[Files]
; Install default remote
Source: "Remote\*.htm"; DestDir: "{userappdata}\Shareaza\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: not language; Tasks: multiuser
Source: "Remote\*.txt"; DestDir: "{userappdata}\Shareaza\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Components: not language; Tasks: multiuser
Source: "Remote\*.htm"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: not language; Tasks: not multiuser
Source: "Remote\*.txt"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Components: not language; Tasks: not multiuser
; Install localized remote
; English:
Source: "Remote\*.htm"; DestDir: "{userappdata}\Shareaza\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: en; Components: language; Tasks: multiuser
Source: "Remote\*.txt"; DestDir: "{userappdata}\Shareaza\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Languages: en; Components: language; Tasks: multiuser
Source: "Remote\*.htm"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: en; Components: language; Tasks: not multiuser
Source: "Remote\*.txt"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Languages: en; Components: language; Tasks: not multiuser
; Dutch:
Source: "Remote-localized\dutch\*.htm"; DestDir: "{userappdata}\Shareaza\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: nl; Components: language; Tasks: multiuser
Source: "Remote-localized\dutch\*.txt"; DestDir: "{userappdata}\Shareaza\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Languages: nl; Components: language; Tasks: multiuser
Source: "Remote-localized\dutch\*.htm"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: nl; Components: language; Tasks: not multiuser
Source: "Remote-localized\dutch\*.txt"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Languages: nl; Components: language; Tasks: not multiuser
; Lithuanian
Source: "Remote-localized\lithuanian\*.htm"; DestDir: "{userappdata}\Shareaza\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: lt; Components: language; Tasks: multiuser
Source: "Remote-localized\lithuanian\*.txt"; DestDir: "{userappdata}\Shareaza\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Languages: lt; Components: language; Tasks: multiuser
Source: "Remote-localized\lithuanian\*.htm"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: lt; Components: language; Tasks: not multiuser
Source: "Remote-localized\lithuanian\*.txt"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Languages: lt; Components: language; Tasks: not multiuser
; German:
Source: "Remote\*.htm"; DestDir: "{userappdata}\Shareaza\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: de; Components: language; Tasks: multiuser
Source: "Remote\*.txt"; DestDir: "{userappdata}\Shareaza\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Languages: de; Components: language; Tasks: multiuser
Source: "Remote\*.htm"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: de; Components: language; Tasks: not multiuser
Source: "Remote\*.txt"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Languages: de; Components: language; Tasks: not multiuser

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

[CustomMessages]
; This section specifies phrazes and words not specified in the ISL files
; Avoid customizing the ISL files since they will change with each version of Inno Setup.
; English:
components_plugins=Plugins
components_skins=Skins
components_languages=Languages
tasks_allusers=All users
tasks_selectusers=Install %1 for:
tasks_currentuser=%1 only
tasks_multisetup=Enable multi-user support
run_skinexe=Running skin installation...
reg_incomingchat=Incoming chat message
reg_apptitle=Shareaza Ultimate File Sharing
icons_license=License
icons_uninstall=Uninstall
; Dutch:
nl.components_plugins=Plugins
nl.components_skins=Skins
nl.components_languages=Talen
nl.tasks_allusers=Alle gebruikers
nl.tasks_selectusers=Installeer %1 voor:
nl.tasks_currentuser=Aleen %1
nl.tasks_multisetup=Ondersteuning voor meerdere gebruikers
nl.run_skinexe=Skin installatie uitvoeren...
nl.reg_incomingchat=Nieuw chat bericht
nl.reg_apptitle=Shareaza Ultimate File Sharing
nl.icons_license=Gebruiksovereenkomst
nl.icons_uninstall=Verwijderen
; Lithuanian:
lt.components_plugins=Papildiniai
lt.components_skins=Apvalkalai
lt.components_languages=Kalbos
lt.tasks_allusers=Visiems vartotojams
lt.tasks_selectusers=Ádiegti „%1“ ðiems vartotojams:
lt.tasks_currentuser=Tik vartotojui %1
lt.tasks_multisetup=Enable multi-user support
lt.run_skinexe=Vykdoma apvalkalo sàranka...
lt.reg_incomingchat=Gaunama þinutë pokalbiui
lt.icons_license=Licencinë sutartis
lt.icons_uninstall=Paðalinti
; German
de.components_plugins=Plugins
de.components_skins=Skins
de.components_languages=Sprachen
de.tasks_allusers=Alle Benutzer
de.tasks_selectusers=Installieren %1 für:
de.tasks_currentuser=Nur für %1
de.tasks_multisetup=Enable multi-user support
de.run_skinexe=Skin Installer einrichten...
de.reg_incomingchat=Eingehende Chat Nachricht
de.reg_apptitle=Shareaza Ultimate File Sharing
de.icons_license=Lizenz
de.icons_uninstall=Shareaza deinstallieren

