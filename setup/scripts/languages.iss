[Languages]
; Use compiler's built in ISL file to patch up holes in ISL collection and specify localized license and readme files
; Note: first language specified is default > English
Name: "en"; MessagesFile: "compiler:Default.isl,setup\isl\default.isl"; LicenseFile: "setup/license/default.rtf"; InfoBeforeFile: "setup/readme/default.txt"
Name: "nl"; MessagesFile: "compiler:Default.isl,setup\isl\dutch.isl"; LicenseFile: "setup/license/dutch.rtf"; InfoBeforeFile: "setup/readme/dutch.txt"
Name: "lt"; MessagesFile: "compiler:Default.isl,setup\isl\lithuanian.isl"; LicenseFile: "setup/license/lithuanian.rtf"; InfoBeforeFile: "setup/readme/lithuanian.txt"

[Files]
; Install localized remote
; English:
Source: "Remote\*.htm"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: en
Source: "Remote\*.txt"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist ; Components: mainfiles; Languages: en
; Dutch:
Source: "Remote-localized\dutch\*.htm"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: nl
Source: "Remote-localized\dutch\*.txt"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Components: mainfiles; Languages: nl
; Lithuanian
Source: "Remote-localized\lithuanian\*.htm"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: lt
Source: "Remote-localized\lithuanian\*.txt"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Components: mainfiles; Languages: lt

; Install localized license
; English
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: en
; Dutch
Source: "setup\license\dutch.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: nl
; Lithuanian
Source: "setup\license\lithuanian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: lt

; Install localized readme
; English
Source: "setup\readme\default.txt"; DestDir: "{app}\Uninstall"; DestName: "readme.txt"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: en
; Dutch
Source: "setup\readme\dutch.txt"; DestDir: "{app}\Uninstall"; DestName: "readme.txt"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: nl
; Lithuanian
Source: "setup\readme\lithuanian.txt"; DestDir: "{app}\Uninstall"; DestName: "readme.txt"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: lt

; Always install user's local language
; English is always installed and specified in main script
; Dutch:
Source: "Languages\default-nl.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Components: mainfiles; Languages: nl
Source: "Languages\default-nl.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: nl
Source: "Languages\NavbarAlpha-nl.bmp"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension  skipifsourcedoesntexist; Components: mainfiles; Languages: nl
; Lithuanian:
Source: "Languages\default-lt.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Components: mainfiles; Languages: lt
Source: "Languages\default-lt.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: lt
Source: "Languages\NavbarAlpha-lt.bmp"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension  skipifsourcedoesntexist; Components: mainfiles; Languages: lt

[CustomMessages]
; This section specifies phrazes and words not specified in the ISL files
; Avoid customizing the ISL files since they will change with each version of Inno Setup.
; English:
components_mainfiles=Main files
components_plugins=Plugins
components_skins=Skins
components_languages=Languages
tasks_allusers=All users
tasks_selectusers=Install %1 for:
tasks_currentuser=%1 only
run_skinexe=Running skin installation...
reg_incomingchat=Incoming chat message
reg_apptitle=Shareaza Ultimate File Sharing
icons_license=License
icons_uninstall=Uninstall
; Dutch:
nl.components_mainfiles=hoofdbestanden
nl.components_plugins=Plugins
nl.components_skins=Skins
nl.components_languages=Talen
nl.tasks_allusers=Alle gebruikers
nl.tasks_selectusers=Installeer %1 voor:
nl.tasks_currentuser=Aleen %1
nl.run_skinexe=Skin installatie uitvoeren...
nl.reg_incomingchat=Nieuw chat bericht
nl.reg_apptitle=Shareaza Ultimate File Sharing
icons_license=Gebruiksovereenkomst
icons_uninstall=Verwijderen
; Lithuanian:
lt.components_mainfiles=Pagrindinës bylos
lt.components_plugins=Papildiniai
lt.components_skins=Apvalkalai
lt.components_languages=Kalbos
lt.tasks_allusers=Visiems vartotojams
lt.tasks_selectusers=Ádiegti „%1“ ðiems vartotojams:
lt.tasks_currentuser=Tik vartotojui %1
lt.run_skinexe=Vykdoma apvalkalo sàranka...
lt.reg_incomingchat=Gaunama þinutë pokalbiui
lt.icons_license=Licencinë sutartis
lt.icons_uninstall=Paðalinti
