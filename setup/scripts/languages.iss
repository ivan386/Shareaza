[Languages]
; Use compiler's built in ISL file to patch up holes in ISL collection and specify localized license and readme files
; Note: first language specified is default > English
Name: "en"; MessagesFile: "compiler:Default.isl,setup\isl\default.isl"; LicenseFile: "setup/license/default.rtf"; InfoBeforeFile: "setup/readme/default.txt"
Name: "nl"; MessagesFile: "compiler:Default.isl,setup\isl\dutch.isl"; LicenseFile: "setup/license/dutch.rtf"; InfoBeforeFile: "setup/readme/dutch.txt"

[Files]
; Install localized remotes
; English:
Source: "Remote\*.htm"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: en
Source: "Remote\*.txt"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist ; Components: mainfiles; Languages: en
; Dutch:
Source: "Remote-localized\dutch\*.htm"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: nl
Source: "Remote-localized\dutch\*.txt"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Components: mainfiles; Languages: nl

; Install localized license
; English
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: en
; Dutch
Source: "setup\license\dutch.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: nl

; Always install user's local language
; English is always installed and specified in main script
; Dutch:
Source: "Languages\default-nl.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist; Components: mainfiles; Languages: nl
Source: "Languages\default-nl.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Components: mainfiles; Languages: nl
Source: "Languages\NavbarAlpha-nl.bmp"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension  skipifsourcedoesntexist; Components: mainfiles; Languages: nl

[CustomMessages]
; This section specifies phrazes and words not specified in the ISL files
; DO NOT customize the ISL files since they will change with each version of Inno Setup.
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
icons_license=Licentie
icons_uninstall=Verwijderen




