; This sub-script defines all languages to be compiled
; WARNING: Do not change this file's encoding

[Languages]
; Use compiler's built in ISL file to patch up holes in ISL collection and specify localized license files
; Note: first language specified is default > English

Name: "en";    MessagesFile: "setup\isl\default.isl";         LicenseFile: "setup/license/default.rtf"
Name: "nl";    MessagesFile: "setup\isl\dutch.isl";           LicenseFile: "setup/license/dutch.rtf"
Name: "lt";    MessagesFile: "setup\isl\lithuanian.isl";      LicenseFile: "setup/license/lithuanian.rtf"
Name: "de";    MessagesFile: "setup\isl\german.isl";          LicenseFile: "setup/license/german.rtf"
Name: "it";    MessagesFile: "setup\isl\italian.isl";         LicenseFile: "setup/license/italian.rtf"
Name: "no";    MessagesFile: "setup\isl\norwegian.isl";       LicenseFile: "setup/license/norwegian.rtf"
Name: "af";    MessagesFile: "setup\isl\afrikaans.isl";       LicenseFile: "setup/license/afrikaans.rtf"
Name: "pt";    MessagesFile: "setup\isl\portuguese.isl";      LicenseFile: "setup/license/portuguese.rtf"
Name: "pt_br"; MessagesFile: "setup\isl\brazilianportuguese.isl"; LicenseFile: "setup/license/portuguese-braz.rtf"
Name: "fr";    MessagesFile: "setup\isl\french.isl";          LicenseFile: "setup/license/french.rtf"
Name: "es";    MessagesFile: "setup\isl\spanish.isl";         LicenseFile: "setup/license/spanish.rtf"
Name: "es_mx"; MessagesFile: "setup\isl\spanish-mexican.isl"; LicenseFile: "setup/license/spanish-mexican.rtf"
Name: "ru";    MessagesFile: "setup\isl\russian.isl";         LicenseFile: "setup/license/russian.rtf"
Name: "gr";    MessagesFile: "setup\isl\greek.isl";           LicenseFile: "setup/license/greek.rtf"
Name: "hu";    MessagesFile: "setup\isl\hungarian.isl";       LicenseFile: "setup/license/hungarian.rtf"
Name: "chs";   MessagesFile: "setup\isl\chinese-simp.isl";    LicenseFile: "setup/license/chinese.rtf"
Name: "sv";    MessagesFile: "setup\isl\swedish.isl";         LicenseFile: "setup/license/swedish.rtf"
Name: "fi";    MessagesFile: "setup\isl\finnish.isl";         LicenseFile: "setup/license/finnish.rtf"
Name: "heb";   MessagesFile: "setup\isl\hebrew.isl";          LicenseFile: "setup/license/hebrew.rtf"
Name: "pl";    MessagesFile: "setup\isl\polish.isl";          LicenseFile: "setup/license/polish.rtf"
Name: "cz";    MessagesFile: "setup\isl\czech.isl";           LicenseFile: "setup/license/czech.rtf"
Name: "sr";    MessagesFile: "setup\isl\serbianlatin.isl";    LicenseFile: "setup/license/serbian.rtf"
Name: "tr";    MessagesFile: "setup\isl\turkish.isl";         LicenseFile: "setup/license/turkish.rtf"
Name: "ja";    MessagesFile: "setup\isl\japanese.isl";        LicenseFile: "setup/license/japanese.rtf"
Name: "ar";    MessagesFile: "setup\isl\arabic.isl";          LicenseFile: "setup/license/arabic.rtf"
Name: "ee";    MessagesFile: "setup\isl\estonian.isl";        LicenseFile: "setup/license/estonian.rtf"
Name: "tw";    MessagesFile: "setup\isl\chinese-trad.isl";    LicenseFile: "setup/license/chinese-trad.rtf"
Name: "sl";    MessagesFile: "setup\isl\slovenian.isl";       LicenseFile: "setup/license/default.rtf"
Name: "ca";    MessagesFile: "setup\isl\catalan.isl";         LicenseFile: "setup/license/catalan.rtf"
Name: "sq";    MessagesFile: "setup\isl\albanian.isl";        LicenseFile: "setup/license/albanian.rtf"

[Files]
; Install all languages
Source: "Languages\default-*.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Tasks: language; Excludes: "default-en.xml"
Source: "Languages\default-*.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Tasks: language
; Install localized language
; English
; Dutch
Source: "Languages\default-nl.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: nl; Tasks: not language
Source: "Languages\default-nl.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: nl; Tasks: not language
; Lithuanian
Source: "Languages\default-lt.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: lt; Tasks: not language
Source: "Languages\default-lt.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: lt; Tasks: not language
; German
Source: "Languages\default-de.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: de; Tasks: not language
Source: "Languages\default-de.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: de; Tasks: not language
; Italian
Source: "Languages\default-it.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: it; Tasks: not language
Source: "Languages\default-it.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: it; Tasks: not language
; Norwegian
Source: "Languages\default-no.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: no; Tasks: not language
Source: "Languages\default-no.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: no; Tasks: not language
; Afrikaans
Source: "Languages\default-af.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: af; Tasks: not language
Source: "Languages\default-af.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: af; Tasks: not language
; Portuguese std
Source: "Languages\default-pt-br.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt; Tasks: not language
Source: "Languages\default-pt-br.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt; Tasks: not language
; Portuguese Braz
Source: "Languages\default-pt-br.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt_br; Tasks: not language
Source: "Languages\default-pt-br.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt_br; Tasks: not language
; French
Source: "Languages\default-fr.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fr; Tasks: not language
Source: "Languages\default-fr.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fr; Tasks: not language
; Spanish std
Source: "Languages\default-es.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es; Tasks: not language
Source: "Languages\default-es.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es; Tasks: not language
; Spanish Mexican
Source: "Languages\default-es-mx.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es_mx; Tasks: not language
Source: "Languages\default-es-mx.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es_mx; Tasks: not language
; Russian
Source: "Languages\default-ru.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ru; Tasks: not language
Source: "Languages\default-ru.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ru; Tasks: not language
; Greek
Source: "Languages\default-gr.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: gr; Tasks: not language
Source: "Languages\default-gr.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: gr; Tasks: not language
; Hungarian
Source: "Languages\default-hu.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: hu; Tasks: not language
Source: "Languages\default-hu.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: hu; Tasks: not language
; Chinese Simp
Source: "Languages\default-chs.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: chs; Tasks: not language
Source: "Languages\default-chs.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: chs; Tasks: not language
; Swedish
Source: "Languages\default-sv.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sv; Tasks: not language
Source: "Languages\default-sv.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sv; Tasks: not language
; Finnish
Source: "Languages\default-fi.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fi; Tasks: not language
Source: "Languages\default-fi.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fi; Tasks: not language
; Hebrew
Source: "Languages\default-heb.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: heb; Tasks: not language
Source: "Languages\default-heb.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: heb; Tasks: not language
; Polish
Source: "Languages\default-pl.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pl; Tasks: not language
Source: "Languages\default-pl.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pl; Tasks: not language
; Czech
Source: "Languages\default-cz.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: cz; Tasks: not language
Source: "Languages\default-cz.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: cz; Tasks: not language
; Serbian
Source: "Languages\default-sr.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sr; Tasks: not language
Source: "Languages\default-sr.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sr; Tasks: not language
; Turkish
Source: "Languages\default-tr.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tr; Tasks: not language
Source: "Languages\default-tr.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tr; Tasks: not language
; Japanese
Source: "Languages\default-ja.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ja; Tasks: not language
Source: "Languages\default-ja.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ja; Tasks: not language
; Arabic
Source: "Languages\default-ar.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ar; Tasks: not language
Source: "Languages\default-ar.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ar; Tasks: not language
; Estonian
Source: "Languages\default-ee.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ee; Tasks: not language
Source: "Languages\default-ee.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ee; Tasks: not language
; Chinese Trad
Source: "Languages\default-tw.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tw; Tasks: not language
Source: "Languages\default-tw.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tw; Tasks: not language
; Slovenian
Source: "Languages\default-sl-si.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sl; Tasks: not language
Source: "Languages\default-sl-si.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sl; Tasks: not language
; Catalan
Source: "Languages\default-ca.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ca; Tasks: not language
Source: "Languages\default-ca.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ca; Tasks: not language
; Albanian
Source: "Languages\default-sq.xml"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sq; Tasks: not language
Source: "Languages\default-sq.ico"; DestDir: "{app}\Skins\Languages"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sq; Tasks: not language

; Install common remote
Source: "Remote\Common\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"
; Install localized remote
; English
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: en
; Dutch
Source: "Remote\nl\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: nl
; Lithuanian
Source: "Remote\lt\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: lt
; German
Source: "Remote\de\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: de
; Italian
Source: "Remote\it\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: it
; Norwegian
Source: "Remote\no\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: no
; Afrikaans
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: af
; Portuguese std
Source: "Remote\pt-br\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: pt
; Portuguese Braz
Source: "Remote\pt-br\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: pt_br
; French
Source: "Remote\fr\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: fr
; Spanish std
Source: "Remote\es-mx\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: es
; Spanish Mexican
Source: "Remote\es-mx\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: es_mx
; Russian
Source: "Remote\ru\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: ru
; Greek
Source: "Remote\gr\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: gr
; Hungarian
Source: "Remote\hu\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: hu
; Chinese Simp
Source: "Remote\chs\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: chs
; Swedish
Source: "Remote\sv\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: sv
; Finnish
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: fi
; Hebrew
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: heb
; Polish
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: pl
; Czech
Source: "Remote\cz\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: cz
; Serbian
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: sr
; Turkish
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: tr
; Japanese
Source: "Remote\ja\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: ja
; Arabic
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: ar
; Estonian
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: ee
; Chinese Trad
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: tw
; Slovenian
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: sl
; Catalan
Source: "Remote\en\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: ca
; Albanian
Source: "Remote\sq\*"; DestDir: "{app}\Remote"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: ".svn"; Languages: sq

; Install localized license
; English
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: en
; Dutch
Source: "setup\license\dutch.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: nl
; Lithuanian
Source: "setup\license\lithuanian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: lt
; German
Source: "setup\license\german.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: de
; Italian
Source: "setup\license\italian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: it
; Norwegian
Source: "setup\license\norwegian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: no
; Afrikaans
Source: "setup\license\afrikaans.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: af
; Portuguese std
Source: "setup\license\portuguese.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt
; Portuguese Braz
Source: "setup\license\portuguese-braz.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt_br
; French
Source: "setup\license\french.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fr
; Spanish std
Source: "setup\license\spanish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es
; Spanish Mexican
Source: "setup\license\spanish-mexican.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es_mx
; Russian
Source: "setup\license\russian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ru
; Greek
Source: "setup\license\greek.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: gr
; Hungarian
Source: "setup\license\hungarian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: hu
; Chinese Simp
Source: "setup\license\chinese.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: chs
; Swedish
Source: "setup\license\swedish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sv
; Finnish
Source: "setup\license\finnish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fi
; Hebrew
Source: "setup\license\hebrew.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: heb
; Polish
Source: "setup\license\polish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pl
; Czech
Source: "setup\license\czech.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: cz
; Serbian
Source: "setup\license\serbian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sr
; Turkish
Source: "setup\license\turkish.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tr
; Japanese
Source: "setup\license\japanese.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ja
; Arabic
Source: "setup\license\arabic.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ar
; Estonian
Source: "setup\license\estonian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ee
; Chinese Trad
Source: "setup\license\chinese-trad.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tw
; Slovenian
Source: "setup\license\default.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sl
; Catalan
Source: "setup\license\catalan.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ca
; Catalan
Source: "setup\license\albanian.rtf"; DestDir: "{app}\Uninstall"; DestName: "license.rtf"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sq

; Install localized filter
; English
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: en
; Dutch
Source: "setup\filter\dutch.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: nl
; Lithuanian
Source: "setup\filter\lithuanian.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: lt
; German
Source: "setup\filter\german.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: de
; Italian
Source: "setup\filter\italian.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: it
; Norwegian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: no
; Afrikaans
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: af
; Portuguese std
Source: "setup\filter\portuguese-br.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt
; Portuguese Braz
Source: "setup\filter\portuguese-br.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pt_br
; French
Source: "setup\filter\french.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fr
; Spanish std
Source: "setup\filter\spanish-mexican.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es
; Spanish Mexican
Source: "setup\filter\spanish-mexican.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: es_mx
; Russian
Source: "setup\filter\russian.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ru
; Greek
Source: "setup\filter\greek.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: gr
; Hungarian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: hu
; Chinese Simp
Source: "setup\filter\chinese-simpl.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: chs
; Swedish
Source: "setup\filter\swedish.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sv
; Finnish
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: fi
; Hebrew
Source: "setup\filter\hebrew.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: heb
; Polish
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: pl
; Czech
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: cz
; Serbian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sr
; Turkish
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tr
; Japanese
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ja
; Arabic
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ar
; Estonian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ee
; Chinese Trad
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: tw
; Slovenian
Source: "setup\filter\default.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sl
; Catalan
Source: "setup\filter\catalan.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: ca
; Albanian
Source: "setup\filter\albanian.dat"; DestDir: "{app}\Data"; DestName: "AdultFilter.dat"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Languages: sq
