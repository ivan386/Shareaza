;
; main.iss
;
; Copyright (c) Shareaza Development Team, 2002-2017.
; This file is part of SHAREAZA (shareaza.sourceforge.net)
;
; Shareaza is free software; you can redistribute it
; and/or modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2 of
; the License, or (at your option) any later version.
;
; Shareaza is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with Shareaza; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;

#if VER < 0x05050000
  #error Inno Setup version 5.5.0 or higher is needed for this script
#endif
#if PREPROCVER < 0x05050000
  #error Inno Setup PreProcessor version 5.5.0.0 or higher is needed for this script
#endif
#ifndef UNICODE
  #error Inno Setup UNICODE version is needed for this script
#endif

; Set "1" for release and "0" for alpha, beta, etc. daily builds
#ifndef RELEASE_BUILD
	#define RELEASE_BUILD 0
#endif

; Test for VS2010
#ifexist SourcePath + "..\..\vc10\Win32\Release\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc10"
  #define PlatformName "Win32"
  #define ConfigurationName "Release"
#endif
#ifexist SourcePath + "..\..\vc10\x64\Release\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc10"
  #define PlatformName "x64"
  #define ConfigurationName "Release"
#endif
#ifexist SourcePath + "..\..\vc10\Win32\Debug\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc10"
  #define PlatformName "Win32"
  #define ConfigurationName "Debug"
#endif
#ifexist SourcePath + "..\..\vc10\x64\Debug\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc10"
  #define PlatformName "x64"
  #define ConfigurationName "Debug"
#endif

; Test for VS2013
#ifexist SourcePath + "..\..\vc12\Win32\Release\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc12"
  #define PlatformName "Win32"
  #define ConfigurationName "Release"
#endif
#ifexist SourcePath + "..\..\vc12\x64\Release\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc12"
  #define PlatformName "x64"
  #define ConfigurationName "Release"
#endif
#ifexist SourcePath + "..\..\vc12\Win32\Debug\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc12"
  #define PlatformName "Win32"
  #define ConfigurationName "Debug"
#endif
#ifexist SourcePath + "..\..\vc12\x64\Debug\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc12"
  #define PlatformName "x64"
  #define ConfigurationName "Debug"
#endif

; Test for VS2015
#ifexist SourcePath + "..\..\vc14\Win32\Release\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc14"
  #define PlatformName "Win32"
  #define ConfigurationName "Release"
#endif
#ifexist SourcePath + "..\..\vc14\x64\Release\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc14"
  #define PlatformName "x64"
  #define ConfigurationName "Release"
#endif
#ifexist SourcePath + "..\..\vc14\Win32\Debug\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc14"
  #define PlatformName "Win32"
  #define ConfigurationName "Debug"
#endif
#ifexist SourcePath + "..\..\vc14\x64\Debug\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc14"
  #define PlatformName "x64"
  #define ConfigurationName "Debug"
#endif

; Test for VS2017
#ifexist SourcePath + "..\..\vc141\Win32\Release\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc141"
  #define PlatformName "Win32"
  #define ConfigurationName "Release"
#endif
#ifexist SourcePath + "..\..\vc141\x64\Release\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc141"
  #define PlatformName "x64"
  #define ConfigurationName "Release"
#endif
#ifexist SourcePath + "..\..\vc141\Win32\Debug\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc141"
  #define PlatformName "Win32"
  #define ConfigurationName "Debug"
#endif
#ifexist SourcePath + "..\..\vc141\x64\Debug\Shareaza.exe"
  #ifdef Compiler
    #error Found a few Shareaza.exe files, you need to leave only one
  #endif
  #define Compiler "vc141"
  #define PlatformName "x64"
  #define ConfigurationName "Debug"
#endif

#ifndef Compiler
  #error No Shareaza.exe files are found, compile some
#endif

#define shareaza SourcePath + "..\..\" + Compiler + "\" + PlatformName + "\" + ConfigurationName + "\Shareaza.exe"
#define internal_name GetStringFileInfo(shareaza, INTERNAL_NAME)
#if ConfigurationName == "Debug"
  #ifdef REVISION
    #define name internal_name + " Debug build (" + REVISION + ")"
  #else
    #define name internal_name + " Debug build"
  #endif
#else
  #define name internal_name
#endif
#define version       GetFileVersion(shareaza)
#define Publisher     "Shareaza Development Team"
#define Description   internal_name + " Ultimate File Sharing"
#define date          GetDateTimeString('yyyy/mm/dd', '-', '')

; Output files names
#if Str(RELEASE_BUILD) == "1" && ConfigurationName == "Release"
  #define output_name  internal_name + "_" + version + "_" + PlatformName
  #define symbols_name internal_name + "_" + version + "_" + PlatformName + "_Symbols.7z"
  #define source_name  internal_name + "_" + version + "_Source.7z"
#else
  #ifdef REVISION
    #define output_name  internal_name + "_" + version + "_" + PlatformName + "_" + ConfigurationName + "_" + REVISION + "_" + date
    #define symbols_name internal_name + "_" + PlatformName + "_" + ConfigurationName + "_" + REVISION + "_" + date + "_Symbols.7z"
    #define source_name  internal_name + "_" + REVISION + "_Source.7z"
  #else
    #define output_name  internal_name + "_" + version + "_" + PlatformName + "_" + ConfigurationName + "_" + date
    #define symbols_name internal_name + "_" + version + "_" + PlatformName + "_" + ConfigurationName + "_" + date + "_Symbols.7z"
    #define source_name  internal_name + "_" + version + "_Source.7z"
  #endif
#endif

; Detect 7-Zip
#ifexist GetEnv( "ProgramFiles" ) + "\7-Zip\7z.exe"
  #define Zip GetEnv( "ProgramFiles" ) + "\7-Zip\7z.exe"
#endif
#ifexist GetEnv( "ProgramFiles(x86)" ) + "\7-Zip\7z.exe"
  #define Zip GetEnv( "ProgramFiles(x86)" ) + "\7-Zip\7z.exe"
#endif
#ifexist GetEnv( "ProgramW6432" ) + "\7-Zip\7z.exe"
  #define Zip GetEnv( "ProgramW6432" ) + "\7-Zip\7z.exe"
#endif
#ifndef Zip
  #error The 7z utility is missing. Please go to https://sourceforge.net/projects/sevenzip/ and install 7-Zip.
#endif

[Setup]
AppComments={#Description}
AppId={#internal_name}
AppName={#name}
AppVersion={#version}
AppVerName={#name} {#version}
AppMutex={#internal_name}
DefaultDirName={ini:{param:SETTINGS|},Locations,Path|{reg:HKLM\SOFTWARE\{#internal_name},|{pf}\{#internal_name}}}
DirExistsWarning=no
DefaultGroupName={#internal_name}
AllowNoIcons=yes
OutputDir=setup\builds
OutputBaseFilename={#output_name}
SolidCompression=yes
Compression=lzma2/max
InternalCompressLevel=max
LZMAUseSeparateProcess=yes
PrivilegesRequired=poweruser
ShowLanguageDialog=yes
LanguageDetectionMethod=locale
UninstallDisplayIcon={app}\Shareaza.exe
UninstallDisplayName={#internal_name} {#version}
UninstallFilesDir={app}\Uninstall
SetupIconFile=setup\misc\install.ico
ShowComponentSizes=no
WizardImageFile=setup\misc\sidebar.bmp
WizardSmallImageFile=setup\misc\corner.bmp
; AppModifyPath="{app}\Uninstall\setup.exe"
ChangesAssociations=yes
ChangesEnvironment=yes
OutputManifestFile=Manifest_{#ConfigurationName}{#PlatformName}.txt
MinVersion=5.0
#if PlatformName == "x64"
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
#endif

; Set the CVS root as source dir (up 2 levels)
SourceDir={#SourcePath}..\..

VersionInfoVersion={#version}
VersionInfoDescription={#Description}
AppPublisher={#Publisher}
AppCopyright=Copyright © {#Publisher}. All rights reserved.

; Links to website for software panel
AppPublisherURL=http://shareaza.sourceforge.net/
AppSupportURL=http://shareaza.sourceforge.net/?id=support
AppUpdatesURL=http://shareaza.sourceforge.net/?id=download

#if ConfigurationName == "Release"
  #include SourcePath + "..\..\" + Compiler + "\vcredist\vcredist.iss"
#endif

[Tasks]
Name: "language"; Description: "{cm:tasks_languages}";
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"
Name: "quicklaunch"; Description: "{cm:CreateQuickLaunchIcon}"
Name: "firewall"; Description: "{cm:tasks_firewall}"; MinVersion: 5.1sp2
Name: "upnp"; Description: "{cm:tasks_upnp}"; MinVersion: 5.1; Check: CanUserModifyServices
Name: "deleteoldsetup"; Description: "{cm:tasks_deleteoldsetup}"; Check: EnableDeleteOldSetup
Name: "resetdiscoveryhostcache"; Description: "{cm:tasks_resetdiscoveryhostcache}"; Flags: unchecked

[Files]
; Main files
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\Shareaza.exe"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\Skin.exe"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\TorrentWizard.exe"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

; Save/Restore scripts
Source: "setup\builds\SaveSettings.bat"; DestDir: "{app}"; DestName: "SaveSettings.bat";    Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist
Source: "setup\builds\RestoreSettings.bat"; DestDir: "{app}"; DestName: "RestoreSettings.bat"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension skipifsourcedoesntexist

; ZLib
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\zlibwapi.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

; BZLib
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\bzlib.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

; BugTrap
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\BugTrap.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\dbghelp.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

; HashLib
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\HashLib.dll"; DestDir: "{app}"; DestName: "HashLib.dll"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

; RegExp
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\RegExp.dll"; DestDir: "{app}"; DestName: "RegExp.dll"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

; SQLite
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\sqlite3.dll"; DestDir: "{app}"; DestName: "sqlite3.dll"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

; GeoIP
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\GeoIP.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "GeoIP\GeoIP.dat"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

; MiniUPnPc
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\MiniUPnPc.dll"; DestDir: "{app}"; DestName: "MiniUPnPc.dll"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

; Plugins
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\7ZipBuilder.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
#if PlatformName == "Win32"
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\7zxa.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
#elif PlatformName == "x64"
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\7zxa64.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
#endif
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\DocumentReader.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\GFLImageServices.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\libgfl340.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\GFLLibraryBuilder.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\ImageViewer.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\MediaImageServices.exe"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\MediaLibraryBuilder.exe"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\MediaPlayer.exe"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\Preview.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\RARBuilder.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
#if PlatformName == "Win32"
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\unrar.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
#elif PlatformName == "x64"
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\unrar64.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
#endif
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\RatDVDReader.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
Source: "{#Compiler}\Win32\{#ConfigurationName}\RazaWebHook32.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly uninsrestartdelete sortfilesbyextension regserver noregerror
Source: "{#Compiler}\x64\{#ConfigurationName}\RazaWebHook64.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly uninsrestartdelete sortfilesbyextension regserver noregerror
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\ShortURL.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\SkinScanSKS.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\SWFPlugin.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\VirusTotal.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\WindowsThumbnail.exe"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\ZIPBuilder.dll"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension regserver

;--== Debug Databases ==--
#if ConfigurationName == "Debug"

; Main files
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\Shareaza.pdb"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension
Source: "{#Compiler}\{#PlatformName}\{#ConfigurationName}\BugTrap.pdb"; DestDir: "{app}"; Flags: overwritereadonly replacesameversion restartreplace uninsremovereadonly sortfilesbyextension

#endif

; Main Files
Source: "Data\*.*"; DestDir: "{app}\Data"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension; Excludes: "*.bak,*.url"

; Schemas
Source: "Schemas\*"; DestDir: "{app}\Schemas"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension

; Visualisations
Source: "plugins\MediaVis\*"; DestDir: "{app}\Vis"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"

; Common skins
Source: "Skins\BlueStreak\*"; DestDir: "{app}\Skins\BlueStreak"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"
Source: "Skins\CleanBlue\*";  DestDir: "{app}\Skins\CleanBlue";  Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"
Source: "Skins\Corona\*";     DestDir: "{app}\Skins\Corona";     Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"
Source: "Skins\Shareaza2\*";  DestDir: "{app}\Skins\Shareaza2";  Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"
Source: "Skins\ShareazaOS\*"; DestDir: "{app}\Skins\ShareazaOS"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"
Source: "Skins\No Banner\*";  DestDir: "{app}\Skins\No Banner";  Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"

; Thematic skins
; Source: "Skins\Halloween\*";         DestDir: "{app}\Skins\Halloween";         Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"
; Source: "Skins\Winter\*";            DestDir: "{app}\Skins\Winter";            Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"
; Source: "Skins\Valentine\*";         DestDir: "{app}\Skins\Valentine";         Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"
; Source: "Skins\St. Patrick's Day\*"; DestDir: "{app}\Skins\St. Patrick's Day"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"
; Source: "Skins\X-Mas\*";             DestDir: "{app}\Skins\X-Mas";             Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"

; Templates
Source: "Templates\*"; DestDir: "{app}\Templates"; Flags: ignoreversion overwritereadonly uninsremovereadonly sortfilesbyextension recursesubdirs; Excludes: ".svn"

;--== Copy files ==--
; Copy skins back from {userappdata}\Shareaza\Skins
Source: "{userappdata}\Shareaza\Skins\*"; DestDir: "{app}\Skins"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist recursesubdirs; AfterInstall: DeleteFolder('{userappdata}\Shareaza\Skins')

; Copy templates back from {userappdata}\Shareaza\Templates
Source: "{userappdata}\Shareaza\Templates\*"; DestDir: "{app}\Templates"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist recursesubdirs; AfterInstall: DeleteFolder('{userappdata}\Shareaza\Templates')

; Switch user data between locations
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Shareaza\Shareaza,UserPath|{userappdata}\Shareaza}}\Data\Library1.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Shareaza\Shareaza,UserPath|{userappdata}\Shareaza}}\Data\Library2.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Shareaza\Shareaza,UserPath|{userappdata}\Shareaza}}\Data\DownloadGroups.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Shareaza\Shareaza,UserPath|{userappdata}\Shareaza}}\Data\TigerTree.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Shareaza\Shareaza,UserPath|{userappdata}\Shareaza}}\Data\Security.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Shareaza\Shareaza,UserPath|{userappdata}\Shareaza}}\Data\UploadQueues.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Shareaza\Shareaza,UserPath|{userappdata}\Shareaza}}\Data\Searches.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Shareaza\Shareaza,UserPath|{userappdata}\Shareaza}}\Data\Schedule.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,UserPath|{reg:HKCU\Software\Shareaza\Shareaza,UserPath|{userappdata}\Shareaza}}\Data\Profile.xml"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Shareaza\Shareaza,Path|{app}}}\Data\Library1.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Shareaza\Shareaza,Path|{app}}}\Data\Library2.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Shareaza\Shareaza,Path|{app}}}\Data\DownloadGroups.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Shareaza\Shareaza,Path|{app}}}\Data\TigerTree.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Shareaza\Shareaza,Path|{app}}}\Data\Security.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Shareaza\Shareaza,Path|{app}}}\Data\UploadQueues.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Shareaza\Shareaza,Path|{app}}}\Data\Searches.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Shareaza\Shareaza,Path|{app}}}\Data\Schedule.dat"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist
Source: "{ini:{param:SETTINGS|},Locations,Path|{reg:HKCU\Software\Shareaza\Shareaza,Path|{app}}}\Data\Profile.xml"; DestDir: "{userappdata}\Shareaza\Data"; Flags: ignoreversion uninsremovereadonly sortfilesbyextension external onlyifdoesntexist skipifsourcedoesntexist

; Copy default security rules
Source: "Data\DefaultSecurity.dat"; DestDir: "{userappdata}\Shareaza\Data"; DestName: "Security.dat"; Flags: onlyifdoesntexist uninsremovereadonly sortfilesbyextension

[Icons]
; Shareaza startmenu icons
Name: "{group}\{#internal_name}"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; AppUserModelID: "Shareaza"
Name: "{group}\TorrentWizard"; Filename: "{app}\TorrentWizard.exe"; WorkingDir: "{app}"; Comment: "Shareaza Torrent File Creator"; AppUserModelID: "TorrentWizard"
Name: "{group}\GUI Mode\{#internal_name} ({cm:icons_basicmode})"; Filename: "{app}\Shareaza.exe"; Parameters: "-basic"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; AppUserModelID: "Shareaza"
Name: "{group}\GUI Mode\{#internal_name} ({cm:icons_tabbedmode})"; Filename: "{app}\Shareaza.exe"; Parameters: "-tabbed"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; AppUserModelID: "Shareaza"
Name: "{group}\GUI Mode\{#internal_name} ({cm:icons_windowedmode})"; Filename: "{app}\Shareaza.exe"; Parameters: "-windowed"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; AppUserModelID: "Shareaza"
Name: "{commondesktop}\{#internal_name}"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; AppUserModelID: "Shareaza"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#internal_name}"; Filename: "{app}\Shareaza.exe"; WorkingDir: "{app}"; Comment: "{cm:reg_apptitle}"; AppUserModelID: "Shareaza"; Tasks: quicklaunch

; Other icons in user language
Name: "{group}\{cm:icons_license}"; Filename: "{app}\Uninstall\license.rtf"; WorkingDir: "{app}\Uninstall"; Comment: "{cm:icons_license}"

[Messages]
; Overwrite standard ISL entries
; DO NOT use for localized messages
BeveledLabel=Shareaza Development Team
SetupAppTitle=Setup - {#internal_name}

[Run]
; Register EXE servers
Filename: "{app}\Shareaza.exe";            Parameters: "/RegServer"; WorkingDir: "{app}"
Filename: "{app}\MediaImageServices.exe";  Parameters: "/RegServer"; WorkingDir: "{app}"
Filename: "{app}\MediaLibraryBuilder.exe"; Parameters: "/RegServer"; WorkingDir: "{app}"
Filename: "{app}\MediaPlayer.exe";         Parameters: "/RegServer"; WorkingDir: "{app}"
Filename: "{app}\WindowsThumbnail.exe";    Parameters: "/RegServer"; WorkingDir: "{app}"

; Run the skin installer at end of installation
Filename: "{app}\skin.exe"; Parameters: "/installsilent"; WorkingDir: "{app}"; StatusMsg: "{cm:run_skinexe}"

; Run Shareaza at end of installation
Filename: "{app}\Shareaza.exe"; Description: "{cm:LaunchProgram,Shareaza}"; WorkingDir: "{app}"; Flags: postinstall skipifsilent nowait

[UninstallRun]
; Unregister EXE servers
Filename: "{app}\MediaImageServices.exe";  Parameters: "/UnRegServer"; WorkingDir: "{app}"
Filename: "{app}\MediaLibraryBuilder.exe"; Parameters: "/UnRegServer"; WorkingDir: "{app}"
Filename: "{app}\MediaPlayer.exe";         Parameters: "/UnRegServer"; WorkingDir: "{app}"
Filename: "{app}\WindowsThumbnail.exe";    Parameters: "/UnRegServer"; WorkingDir: "{app}"
Filename: "{app}\Shareaza.exe";            Parameters: "/UnRegServer"; WorkingDir: "{app}"

; Run the skin installer at start of uninstallation and make sure it only runs once
Filename: "{app}\skin.exe"; Parameters: "/uninstallsilent"; WorkingDir: "{app}"; StatusMsg: "{cm:run_skinexe}"; RunOnceId: "uninstallskinexe"

[Registry]
; Write installation path to registry
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Settings"; ValueType: dword; ValueName: "Upgrade" ; ValueData: "1"; Flags: uninsdeletekey deletevalue
Root: HKCU; Subkey: "Software\Shareaza\Shareaza"; ValueType: string; ValueName: "Installer" ; ValueData: "{srcexe}"; Flags: uninsdeletekey deletevalue
Root: HKCU; Subkey: "Software\Shareaza\Shareaza"; ValueType: string; ValueName: "Path" ; ValueData: "{app}"; Flags: uninsdeletekey deletevalue
Root: HKCU; Subkey: "Software\Shareaza\Shareaza"; ValueType: string; ValueName: "UserPath" ; ValueData: "{ini:{param:SETTINGS|},Locations,UserPath|{userappdata}\Shareaza}"; Flags: uninsdeletekey deletevalue
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\Shareaza.exe"; ValueType: string; ValueName: ; ValueData: "{app}\Shareaza.exe"; Flags: uninsdeletekey deletevalue
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\Shareaza.exe"; ValueType: string; ValueName: "Path"; ValueData: "{app}"; Flags: uninsdeletekey deletevalue

; Install chat notify sound
Root: HKCU; Subkey: "AppEvents\EventLabels\RAZA_IncomingChat"; ValueType: string; ValueName: ; ValueData: "{cm:reg_incomingchat}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Shareaza"; ValueType: string; ValueName: ; ValueData: "{cm:reg_apptitle}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Shareaza\RAZA_IncomingChat\.current"; ValueType: string; ValueName: ; ValueData: "%SystemRoot%\media\notify.wav"; Flags: uninsdeletekey
Root: HKCU; Subkey: "AppEvents\Schemes\Apps\Shareaza\RAZA_IncomingChat\.default"; ValueType: string; ValueName: ; ValueData: "%SystemRoot%\media\notify.wav"; Flags: uninsdeletekey

; Set UPNP as choosed during the setup
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Connection"; ValueType: dword; ValueName: "EnableUPnP"; ValueData: 1; Flags: deletevalue; Tasks: upnp
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Connection"; ValueType: dword; ValueName: "EnableUPnP"; ValueData: 0; Flags: deletevalue; Tasks: not upnp

; Delete keys at uninstall
Root: HKLM; Subkey: "SOFTWARE\Shareaza"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Shareaza"; Flags: dontcreatekey uninsdeletekey
Root: HKU; Subkey: ".DEFAULT\Software\Shareaza"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueName: "Shareaza"; Flags: dontcreatekey uninsdeletevalue

Root: HKLM; Subkey: "SOFTWARE\Classes\.sd"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\.co"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\.collection"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\.emulecollection"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\.torrent"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\.sks"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\dchub"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\dcfile"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\ed2k"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\foxy"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\gnet"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\gnutella"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\gnutella1"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\gnutella2"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\gwc"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\uhc"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\ukhl"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\magnet"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\mp2p"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\shareaza*"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\Shareaza.exe"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\skin.exe"; Flags: dontcreatekey uninsdeletekey

Root: HKCU; Subkey: "SOFTWARE\Classes\.sd"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\.co"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\.collection"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\.emulecollection"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\.torrent"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\.sks"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\dchub"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\dcfile"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\ed2k"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\foxy"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\gnet"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\gnutella"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\gnutella1"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\gnutella2"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\gwc"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\uhc"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\ukhl"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\magnet"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\mp2p"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\shareaza*"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\Applications\Shareaza.exe"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\Applications\skin.exe"; Flags: dontcreatekey uninsdeletekey

Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.sks"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.torrent"; Flags: dontcreatekey uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Management\ARPCache\Shareaza_is1"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\Microsoft\Windows\ShellNoRoam\MUICache"; ValueName:"{app}\Shareaza.exe"; Flags: dontcreatekey uninsdeletevalue

Root: HKLM; Subkey: "SOFTWARE\Magnet"; Flags: dontcreatekey uninsdeletekey

; How did we end up inhere?
Root: HKCU; Subkey: "Software\Intervideo\Common\AudioDec\Shareaza"; Flags: dontcreatekey uninsdeletekey

; Delete NSIS entry on software panel
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Shareaza"; Flags: dontcreatekey deletekey

; Clear version check key
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\VersionCheck"; Flags: dontcreatekey deletekey
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\VersionCheck"; Flags: dontcreatekey deletekey

; Delete Generic Video thumbnailer plugin
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Plugins"; ValueName: "{{17BF74FD-69AF-4BD5-A982-EA6DE6F3449C}"; Flags: dontcreatekey deletevalue
Root: HKCR; Subkey: "Shareaza.AVIThumb"; Flags: dontcreatekey deletekey
Root: HKCR; Subkey: "Shareaza.AVIThumb.1"; Flags: dontcreatekey deletekey
Root: HKCR; Subkey: "CLSID\{{17BF74FD-69AF-4BD5-A982-EA6DE6F3449C}"; Flags: dontcreatekey deletekey

; Create TorrentWizard default dir locations
Root: HKCU; Subkey: "Software\Shareaza\TorrentWizard\Folders"; ValueType: string; ValueName: "001.Path"; ValueData: "{userappdata}\Shareaza\Torrents"; Flags: createvalueifdoesntexist
Root: HKCU; Subkey: "Software\Shareaza\TorrentWizard\Folders"; ValueType: string; ValueName: "Last"; ValueData: "{userappdata}\Shareaza\Torrents"; Flags: createvalueifdoesntexist
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\BitTorrent"; ValueType: string; ValueName: "TorrentCreatorPath"; ValueData: "TorrentWizard.exe" ; Flags: createvalueifdoesntexist uninsdeletekey

; Turn on default ShareazaOS skin
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Skins"; ValueType: dword; ValueName: "ShareazaOS\ShareazaOS.xml"; ValueData: "1"; Flags: createvalueifdoesntexist uninsdeletevalue

; Select thematic skin
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Skins"; ValueType: dword; ValueName: "Halloween\Halloween.xml";                 ValueData: "0"; Flags: deletevalue uninsdeletevalue
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Skins"; ValueType: dword; ValueName: "Winter\Winter.xml";                       ValueData: "0"; Flags: deletevalue uninsdeletevalue
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Skins"; ValueType: dword; ValueName: "Valentine\Valentine.xml";                 ValueData: "0"; Flags: deletevalue uninsdeletevalue
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Skins"; ValueType: dword; ValueName: "St. Patrick's Day\St. Patrick's Day.xml"; ValueData: "0"; Flags: deletevalue uninsdeletevalue
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Skins"; ValueType: dword; ValueName: "X-Mas\X-Mas.xml";                         ValueData: "0"; Flags: deletevalue uninsdeletevalue

; Disable extensions for plugins which make trouble
; Since it is image services plugin we need to add extensions required for the first run
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Plugins"; ValueType: string; ValueName: "{{FF5FCD00-2C20-49D8-84F6-888D2E2C95DA}"; ValueData: "|-.pdf||.bmp||.png||.jpg|"; Flags: createvalueifdoesntexist uninsdeletekey

;It is just some crap added by unofficial and bad versions of Shareaza
Root: HKCU; SubKey: "Software\Microsoft\Internet Explorer\LowRegistry\MediaBar"; Flags: deletekey
Root: HKCU; SubKey: "Software\Microsoft\Internet Explorer\LowRegistry\BHO Shareaza"; Flags: deletekey
Root: HKCU; SubKey: "Software\BHO Shareaza"; Flags: deletekey

; Set program default associations
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities"; ValueType: string; ValueName: "ApplicationDescription"; ValueData: "{cm:reg_apptitle}" ; Flags: createvalueifdoesntexist uninsdeletekey

Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\FileAssociations"; ValueType: string; ValueName: ".torrent"; ValueData: "Shareaza.Torrent"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\FileAssociations"; ValueType: string; ValueName: ".sd"; ValueData: "Shareaza.Download"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\FileAssociations"; ValueType: string; ValueName: ".co"; ValueData: "Shareaza.Collection"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\FileAssociations"; ValueType: string; ValueName: ".collection"; ValueData: "Shareaza.Collection"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\FileAssociations"; ValueType: string; ValueName: ".emulecollection"; ValueData: "Shareaza.Collection"; Flags: createvalueifdoesntexist uninsdeletevalue

Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "shareaza"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "magnet"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "dchub"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "dcfile"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "foxy"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "gnutella"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "gnet"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "uhc"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "ukhl"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "gnutella1"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "gnutella2"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "gwc"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "ed2k"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue
Root: HKLM; Subkey: "Software\Shareaza\Shareaza\Capabilities\UrlAssociations"; ValueType: string; ValueName: "mp2p"; ValueData: "Shareaza"; Flags: createvalueifdoesntexist uninsdeletevalue

Root: HKLM; Subkey: "Software\RegisteredApplications"; ValueType: string; ValueName: "Shareaza"; ValueData: "Software\Shareaza\Shareaza\Capabilities" ; Flags: createvalueifdoesntexist uninsdeletevalue

[Dirs]
Name: "{app}\Skins"; Flags: uninsalwaysuninstall; Permissions: users-modify

[InstallDelete]
; Very basic malware removal
Type: files; Name: "{app}\Shareaza.exe"; Check: IsMalwareDetected
Type: files; Name: "{app}\vc2.dll"

; Clean up old files from Shareaza
Type: files; Name: "{app}\*.pdb"
Type: files; Name: "{app}\zlib*.dll"
Type: files; Name: "{app}\7zx*.dll"
Type: files; Name: "{app}\RazaWebHook.dll"
Type: files; Name: "{app}\MediaImageServices.dll"
Type: files; Name: "{app}\MediaLibraryBuilder.dll"
Type: files; Name: "{app}\MediaPlayer.dll"
Type: files; Name: "{app}\libgfl*.dll"
Type: files; Name: "{app}\Skins\skin.exe"
Type: files; Name: "{app}\Schemas\VendorCache.xsd"
Type: files; Name: "{app}\Schemas\SchemaDescriptor.xsd"
Type: files; Name: "{app}\LICENSE.txt"
Type: files; Name: "{app}\uninstall.exe"
Type: files; Name: "{app}\*.dat"
Type: files; Name: "{app}\*.xml"
Type: files; Name: "{app}\*.png"
Type: files; Name: "{app}\*.bmp"
Type: files; Name: "{app}\Data\*.url"
Type: filesandordirs; Name: "{app}\Uninstall"
Type: filesandordirs; Name: "{app}\Plugins"
Type: filesandordirs; Name: "{userappdata}\Shareaza\Remote"
Type: filesandordirs; Name: "{userappdata}\Shareaza\Schemas"
Type: filesandordirs; Name: "{userappdata}\Shareaza\Skins"
Type: files; Name: "{userappdata}\Shareaza\Data\DefaultAvatar.png"
Type: files; Name: "{app}\Skins\Languages\default-en.xml"
Type: files; Name: "{app}\unicows.dll"

; Delete renamed translations
; "jp" to "ja"
Type: files; Name: "{app}\Skins\Languages\default-jp.ico"
Type: files; Name: "{app}\Skins\Languages\default-jp.xml"

; Clean up old Shareaza icons
Type: files; Name: "{userdesktop}\Shareaza.lnk"; Tasks: not desktopicon
Type: files; Name: "{commondesktop}\Shareaza.lnk"; Tasks: not desktopicon
Type: files; Name: "{userdesktop}\Start Shareaza.lnk"; Check: NSISUsed
Type: filesandordirs; Name: "{userprograms}\Shareaza"; Check: NSISUsed
Type: filesandordirs; Name: "{commonprograms}\Shareaza"; Check: NSISUsed
Type: filesandordirs; Name: "{userprograms}\Shareaza"; Check: InnoSetupUsed
Type: filesandordirs; Name: "{commonprograms}\Shareaza"; Check: InnoSetupUsed
Type: files; Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Shareaza.lnk"; Tasks: not quicklaunch

; Delete extra components so installer can "uninstall" them
Type: filesandordirs; Name: "{app}\Remote"
Type: filesandordirs; Name: "{app}\Skins\Languages"; Tasks: not language

; Delete Discovery.dat and HostCache.dat
Type: files; Name: "{app}\Data\Discovery.dat"; Tasks: resetdiscoveryhostcache
Type: files; Name: "{app}\Data\HostCache.dat"; Tasks: resetdiscoveryhostcache
Type: files; Name: "{userappdata}\Shareaza\Data\Discovery.dat"; Tasks: resetdiscoveryhostcache
Type: files; Name: "{userappdata}\Shareaza\Data\HostCache.dat"; Tasks: resetdiscoveryhostcache

[UninstallDelete]
; Clean up files created after installation
Type: filesandordirs; Name: "{app}\Data"
Type: filesandordirs; Name: "{userappdata}\Shareaza\Data"
Type: filesandordirs; Name: "{app}\Skins"
Type: filesandordirs; Name: "{app}\Templates"
Type: filesandordirs; Name: "{app}\Schemas"

Type: filesandordirs; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{userdocs}\Shareaza Downloads}\Metadata"
Type: files; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{userdocs}\Shareaza Downloads}\Thumbs.db"
Type: files; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{userdocs}\Shareaza Downloads}\SThumbs.dat"
Type: files; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{userdocs}\Shareaza Downloads}\desktop.ini"
Type: filesandordirs; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,TorrentPath|{userappdata}\Shareaza\Torrents}\Metadata"
Type: files; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,TorrentPath|{userappdata}\Shareaza\Torrents}\desktop.ini"
Type: filesandordirs; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CollectionPath|{userappdata}\Shareaza\Collections}\Metadata"
Type: files; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CollectionPath|{userappdata}\Shareaza\Collections}\desktop.ini"

Type: filesandordirs; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{app}\Downloads}\Metadata"
Type: files; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{app}\Downloads}\Thumbs.db"
Type: files; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{app}\Downloads}\SThumbs.dat"
Type: files; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CompletePath|{app}\Downloads}\desktop.ini"
Type: filesandordirs; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,TorrentPath|{app}\Torrents}\Metadata"
Type: files; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,TorrentPath|{app}\Torrents}\desktop.ini"
Type: filesandordirs; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CollectionPath|{app}\Collections}\Metadata"
Type: files; Name: "{reg:HKCU\Software\Shareaza\Shareaza\Downloads,CollectionPath|{app}\Collections}\desktop.ini"

; Pull in languages and localized files
#include "languages.iss"

; Code sections need to be the last section in a script or the compiler will get confused
[Code]
type
  SERVICE_STATUS = record
    dwServiceType: cardinal;
    dwCurrentState: cardinal;
    dwControlsAccepted: cardinal;
    dwWin32ExitCode: cardinal;
    dwServiceSpecificExitCode: cardinal;
    dwCheckPoint: cardinal;
    dwWaitHint: cardinal;
  end;
  HANDLE = cardinal;
const
  WM_CLOSE = $0010;
  KeyLoc1 = 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Shareaza_is1';
  KeyLoc2 = 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Shareaza';
  KeyName = 'UninstallString';
  NET_FW_SCOPE_ALL = 0;
  NET_FW_IP_VERSION_ANY       = 2;
  SERVICE_QUERY_CONFIG        = $1;
  SERVICE_CHANGE_CONFIG       = $2;
  SERVICE_QUERY_STATUS        = $4;
  SERVICE_START               = $10;
  SERVICE_STOP                = $20;
  SERVICE_ALL_ACCESS          = $f01ff;
  SC_MANAGER_ALL_ACCESS       = $f003f;
  SERVICE_AUTO_START          = $2;
  SERVICE_DEMAND_START        = $3;
  SERVICE_RUNNING             = $4;
  SERVICE_NO_CHANGE           = $ffffffff;
var
  CurrentPath: string;
  Installed: Boolean;
  MalwareDetected: Boolean;
  FirewallFailed: string;
  HasUserPrivileges: Boolean;

{ NT API functions for services }
Function OpenSCManager(lpMachineName, lpDatabaseName: string; dwDesiredAccess: cardinal): HANDLE;
external 'OpenSCManagerW@advapi32.dll stdcall setuponly';

Function OpenService(hSCManager: HANDLE; lpServiceName: string; dwDesiredAccess: cardinal): HANDLE;
external 'OpenServiceW@advapi32.dll stdcall setuponly';

Function CloseServiceHandle(hSCObject: HANDLE): Boolean;
external 'CloseServiceHandle@advapi32.dll stdcall setuponly';

Function StartNTService(hService: HANDLE; dwNumServiceArgs: cardinal; lpServiceArgVectors: cardinal): Boolean;
external 'StartServiceW@advapi32.dll stdcall setuponly';

Function QueryServiceStatus(hService: HANDLE; var ServiceStatus: SERVICE_STATUS): Boolean;
external 'QueryServiceStatus@advapi32.dll stdcall setuponly';

Function ChangeServiceConfig(hService: HANDLE; dwServiceType, dwStartType, dwErrorControl: cardinal;
                             lpBinaryPathName, lpLoadOrderGroup: string; lpdwTagId: cardinal;
                             lpDependencies, lpServiceStartName, lpPassword, lpDisplayName: string): Boolean;
external 'ChangeServiceConfigW@advapi32.dll stdcall setuponly';

Function InnoSetupUsed(): boolean;
Begin
    Result := RegKeyExists(HKEY_LOCAL_MACHINE, KeyLoc1);
End;

Function NSISUsed(): boolean;
Begin
    Result := RegKeyExists(HKEY_LOCAL_MACHINE, KeyLoc2);
End;

{ check if the current install path exists }
Function DoesPathExist(): boolean;
Begin
    if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Shareaza','', CurrentPath) then
        Result := DirExists(CurrentPath)
    else
        Result := False;

End;

Function OpenServiceManager(): HANDLE;
begin
  Result := 0;
  { Only for Windows XP and above }
  if ( GetWindowsVersion >= $05010000 ) then
    Result := OpenSCManager('', 'ServicesActive', SC_MANAGER_ALL_ACCESS);
end;

Function CanUserModifyServices(): Boolean;
var
 hSCManager: HANDLE;
begin
  hSCManager := 0;
  Result := false;
  HasUserPrivileges := false;
  { Only for Windows XP and above }
  if ( GetWindowsVersion >= $05010000 ) then begin
    hSCManager := OpenSCManager('', 'ServicesActive', SC_MANAGER_ALL_ACCESS);
    if (hSCManager <> 0) then begin
      HasUserPrivileges := true;
      Result := true;
      CloseServiceHandle(hSCManager);
    end;
  end;
end;

Function IsServiceInstalled(ServiceName: string): boolean;
var
 hSCManager: HANDLE;
 hService: HANDLE;
begin
  hSCManager := OpenServiceManager();
  Result := false;
  if (hSCManager <> 0) then begin
    hService := OpenService(hSCManager, ServiceName, SERVICE_QUERY_CONFIG);
    if (hService <> 0) then begin
      Result := true;
      CloseServiceHandle(hService);
    end;
    CloseServiceHandle(hSCManager);
  end;
end;

Function StartService(ServiceName: string): boolean;
var
  hSCManager: HANDLE;
  hService: HANDLE;
begin
  hSCManager := OpenServiceManager();
  Result := false;
  if (hSCManager <> 0) then begin
    hService := OpenService(hSCManager, ServiceName, SERVICE_START);
    if (hService <> 0) then begin
      Result := StartNTService(hService, 0, 0);
      CloseServiceHandle(hService);
    end;
    CloseServiceHandle(hSCManager);
  end;
end;

Function IsServiceRunning(ServiceName: string): boolean;
var
  hSCManager: HANDLE;
  hService: HANDLE;
  sStatus: SERVICE_STATUS;
begin
  hSCManager := OpenServiceManager();
  Result := false;
  if (hSCManager <> 0) then begin
    hService := OpenService(hSCManager, ServiceName, SERVICE_QUERY_STATUS);
    if (hService <> 0) then begin
      if (QueryServiceStatus(hService, sStatus)) then
        Result := (sStatus.dwCurrentState = SERVICE_RUNNING);
      CloseServiceHandle(hService);
    end;
    CloseServiceHandle(hSCManager);
 end;
end;

Function ChangeServiceStartup(ServiceName: string; dwStartType: cardinal): boolean;
var
  hSCManager: HANDLE;
  hService: HANDLE;
begin
  hSCManager := OpenServiceManager();
  Result := false;
  if (hSCManager <> 0) then begin
    hService := OpenService(hSCManager, ServiceName, SERVICE_CHANGE_CONFIG);
    if (hService <> 0) then begin
       Result := ChangeServiceConfig(hService, SERVICE_NO_CHANGE, dwStartType, SERVICE_NO_CHANGE, '','',0,'','','','');
       CloseServiceHandle(hService);
    end;
    CloseServiceHandle(hSCManager);
  end;
end;

Function NextButtonClick(CurPageID: integer): Boolean;
var
  Wnd: HWND;
  Shutdownmessage: string;
begin
  Result := True;
  if (CurPageID = wpWelcome) then begin
    Wnd := FindWindowByClassName('ShareazaMainWnd');
    if Wnd <> 0 then begin
      Shutdownmessage := ExpandConstant('{cm:dialog_shutdown,Shareaza}');
      if MsgBox(Shutdownmessage, mbConfirmation, MB_OKCANCEL) = IDOK then begin
        SendMessage(Wnd, WM_CLOSE, 0, 0);
        while Wnd <> 0 do begin
          Sleep(100);
          Wnd := FindWindowByClassName('ShareazaMainWnd');
        end;
      end else Result := False;
    end;
  end;
end;

Function MalwareCheck(MalwareFile: string): Boolean;
Begin
  Result := False;
  if FileExists( MalwareFile ) then Begin
    if MsgBox(ExpandConstant( '{cm:dialog_malwaredetected,' + MalwareFile + '}' ), mbConfirmation, MB_YESNO) = IDYES then begin
      Result := True;
    End;
    MalwareDetected := True;
  End;
End;

Function InitializeSetup: Boolean;
Begin
  Result := True;
  Installed := (RegValueExists(HKEY_LOCAL_MACHINE, KeyLoc1, KeyName) or RegValueExists(HKEY_LOCAL_MACHINE, KeyLoc2, KeyName)) and DoesPathExist();
  MalwareDetected := False;

  { Malware check }
  Result := NOT MalwareCheck( ExpandConstant('{win}\vgraph.dll') );
  if Result then Begin Result := NOT MalwareCheck( ExpandConstant('{win}\Shareaza.exe') ); End;
  if Result then Begin Result := NOT MalwareCheck( ExpandConstant('{sys}\Shareaza.exe') ); End;
  if Result then Begin Result := NOT MalwareCheck( ExpandConstant('{pf}\Shareaza\vc2.dll') ); End;
End;

Function IsMalwareDetected: Boolean;
Begin
  Result := MalwareDetected;
End;

Function EnableDeleteOldSetup: Boolean;
Begin
  Result := Installed;
End;

Function WeOwnTorrentAssoc: boolean;
var
  CommandString: string;
  Position: Integer;
Begin
  Result := False;
  if RegQueryStringValue(HKEY_CLASSES_ROOT, 'bittorrent\shell\open\command','', CommandString) then
    Begin
      Position := Pos('shareaza.exe', LowerCase(CommandString));
      Result := (Position > 0);
    End
End;

Function ShouldSkipPage(PageID: Integer): Boolean;
Begin
  Result := False;
  if PageID = wpSelectDir then Result := Installed;
End;

Procedure DeleteFolder(Param: String);
var
  Foldername: string;
Begin
  Foldername := ExpandConstant(Param);
  DelTree(Foldername, True, True, True);
End;

Procedure DeleteFile(Param: String);
var
  Filename: string;
Begin
  Filename := ExpandConstant(Param);
  DelayDeleteFile(Filename,3);
End;

Procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  InstallFolder: string;
  FirewallManager: Variant;
  FirewallProfile: Variant;
  Wnd: HWND;
Begin
  if CurUninstallStep = usUninstall then begin
    { Only for Windows XP and above }
    if ( GetWindowsVersion >= $05010000 ) then begin
      try
        InstallFolder := ExpandConstant('{app}\Shareaza.exe');
        FirewallManager := CreateOleObject('HNetCfg.FwMgr');
        FirewallProfile := FirewallManager.LocalPolicy.CurrentProfile;
        FirewallProfile.AuthorizedApplications.Remove(InstallFolder);
      except
      End;
    End;
    Wnd := FindWindowByClassName('ShareazaMainWnd');
    if Wnd <> 0 then begin
      SendMessage(Wnd, WM_CLOSE, 0, 0);
      while Wnd <> 0 do
        begin
          Sleep(100);
          Wnd := FindWindowByClassName('ShareazaMainWnd');
        End;
    End;
    if WeOwnTorrentAssoc then begin
      RegDeleteKeyIncludingSubkeys(HKEY_CLASSES_ROOT,'.torrent');
      RegDeleteKeyIncludingSubkeys(HKEY_CLASSES_ROOT,'bittorrent');
    End;
  End;
End;

Function IsLanguageRTL(LangCode: String): String;
Begin
  if ( (LangCode = 'heb') or (LangCode = 'ar') ) then
    Result := '1'
  else
    Result := '0';
End;

Function GetRelFilePath(LangCode: String): String;
Begin
  StringChangeEx(LangCode, '_', '-', True);

  if ( LangCode = 'pt' ) then
    Result := 'Languages\default-pt-br.xml'
  else if ( LangCode = 'sl' ) then
    Result := 'Languages\default-sl-SI.xml'
  else
    Result := 'Languages\default-' + LangCode + '.xml';
End;

Function ResetLanguages: boolean;
var
  Names: TArrayOfString;
  I: Integer;
  S: String;
  Value: String;
begin
  if RegGetValueNames(HKEY_CURRENT_USER, 'Software\Shareaza\Shareaza\Skins', Names) then
  begin
    S := '';
    Value := LowerCase(GetRelFilePath(ExpandConstant('{language}')));
    for I := 0 to GetArrayLength(Names)-1 do
    begin
      S := LowerCase(Names[I]);
      if Pos('languages', S) <> 0 then
        if Value <> S then
          RegWriteDWordValue(HKEY_CURRENT_USER, 'Software\Shareaza\Shareaza\Skins', S, 0);
    end;
    RegWriteDWordValue(HKEY_CURRENT_USER, 'Software\Shareaza\Shareaza\Skins', Value, 1);
    Value := IsLanguageRTL(ExpandConstant('{language}'));
    RegWriteDWordValue(HKEY_CURRENT_USER, 'Software\Shareaza\Shareaza\Settings', 'LanguageRTL', StrToInt(Value));
    RegWriteStringValue(HKEY_CURRENT_USER, 'Software\Shareaza\Shareaza\Settings', 'Language', ExpandConstant('{language}'));
    { Set default values for other users }
    RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'Software\Shareaza\Shareaza', 'DefaultLanguageRTL', StrToInt(Value));
    RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\Shareaza\Shareaza', 'DefaultLanguage', ExpandConstant('{language}'));
  end;
  Result := True;
end;

Procedure CurStepChanged(CurStep: TSetupStep);
var
  InstallFolder: string;
  FirewallObject: Variant;
  FirewallManager: Variant;
  FirewallProfile: Variant;
  Reset: boolean;
  Success: boolean;
  Path: string;
Begin
  if CurStep=ssPostInstall then begin
    if IsTaskSelected('firewall') then begin
      if WizardSilent = True then begin
        try
          FirewallObject := CreateOleObject('HNetCfg.FwAuthorizedApplication');
          InstallFolder := ExpandConstant('{app}\Shareaza.exe');
          FirewallObject.ProcessImageFileName := InstallFolder;
          FirewallObject.Name := 'Shareaza';
          FirewallObject.Scope := NET_FW_SCOPE_ALL;
          FirewallObject.IpVersion := NET_FW_IP_VERSION_ANY;
          FirewallObject.Enabled := True;
          FirewallManager := CreateOleObject('HNetCfg.FwMgr');
          FirewallProfile := FirewallManager.LocalPolicy.CurrentProfile;
          FirewallProfile.AuthorizedApplications.Add(FirewallObject);
        except
        End;
      End else begin
        FirewallFailed := ExpandConstant('{cm:dialog_firewall}')
        try
          FirewallObject := CreateOleObject('HNetCfg.FwAuthorizedApplication');
          InstallFolder := ExpandConstant('{app}\Shareaza.exe');
          FirewallObject.ProcessImageFileName := InstallFolder;
          FirewallObject.Name := 'Shareaza';
          FirewallObject.Scope := NET_FW_SCOPE_ALL;
          FirewallObject.IpVersion := NET_FW_IP_VERSION_ANY;
          FirewallObject.Enabled := True;
          FirewallManager := CreateOleObject('HNetCfg.FwMgr');
          FirewallProfile := FirewallManager.LocalPolicy.CurrentProfile;
          FirewallProfile.AuthorizedApplications.Add(FirewallObject);
        except
          MsgBox(FirewallFailed, mbInformation, MB_OK);
        End;
      End;
    End;
    if IsTaskSelected('upnp') then begin
      if (HasUserPrivileges) then begin
        Success := false;
        if (IsServiceInstalled('SSDPSRV') and IsServiceInstalled('upnphost')) then begin
          if (not IsServiceRunning('SSDPSRV')) then begin
            { change the startup type to manual if it was disabled;
              we don't need to start it since UPnP Device Host service depends on it;
              assuming that user didn't modify the dependencies manually.
              Note: probably, we could elevate user rights with AdjustTokenPrivileges(?) }
            Success := ChangeServiceStartup('SSDPSRV', SERVICE_DEMAND_START);
          end else
            Success := true;
          if (Success) then begin
            { We succeeded to change the startup type, so we will change another service }
            Success := ChangeServiceStartup('upnphost', SERVICE_AUTO_START);
            if (Success and not IsServiceRunning('upnphost')) then
              StartService('upnphost');
          end;
        end;
      end;
    end;
  End;
  if CurStep=ssInstall then begin
    if not IsTaskSelected('firewall') then begin
      { Only for Windows XP and above }
      if ( GetWindowsVersion >= $05010000 ) then begin
        try
          InstallFolder := ExpandConstant('{app}\Shareaza.exe');
          FirewallManager := CreateOleObject('HNetCfg.FwMgr');
          FirewallProfile := FirewallManager.LocalPolicy.CurrentProfile;
          FirewallProfile.AuthorizedApplications.Remove(InstallFolder);
        except
        End;
      End;
    End;
  End;

  { Check if the needed paths exist otherwise delete it from the registry (They will be recreated later in the installation process) }
  if CurStep=ssInstall then begin
    if RegQueryStringValue(HKEY_CURRENT_USER, 'SOFTWARE\Shareaza\Shareaza\Downloads', 'CompletePath', Path) and (not DirExists(Path)) then begin
      if not RegDeleteValue(HKEY_CURRENT_USER, 'SOFTWARE\Shareaza\Shareaza\Downloads', 'CompletePath') then begin
        MsgBox(ExpandConstant('{cm:PathNotExist,complete}'), mbError, MB_OK);
      End;
    End;
    if RegQueryStringValue(HKEY_CURRENT_USER, 'SOFTWARE\Shareaza\Shareaza\Downloads', 'IncompletePath', Path) and (not DirExists(Path)) then begin
      if not RegDeleteValue(HKEY_CURRENT_USER, 'SOFTWARE\Shareaza\Shareaza\Downloads', 'IncompletePath') then begin
        MsgBox(ExpandConstant('{cm:PathNotExist,incomplete}'), mbError, MB_OK);
      End;
    End;
    if RegQueryStringValue(HKEY_CURRENT_USER, 'SOFTWARE\Shareaza\Shareaza\Downloads', 'CollectionPath', Path) and (not DirExists(Path)) then begin
      if not RegDeleteValue(HKEY_CURRENT_USER, 'SOFTWARE\Shareaza\Shareaza\Downloads', 'CollectionPath') then begin
        MsgBox(ExpandConstant('{cm:PathNotExist,collection}'), mbError, MB_OK);
      End;
    End;
    if RegQueryStringValue(HKEY_CURRENT_USER, 'SOFTWARE\Shareaza\Shareaza\Downloads', 'TorrentPath', Path) and (not DirExists(Path)) then begin
      if not RegDeleteValue(HKEY_CURRENT_USER, 'SOFTWARE\Shareaza\Shareaza\Downloads', 'TorrentPath') then begin
        MsgBox(ExpandConstant('{cm:PathNotExist,torrent}'), mbError, MB_OK);
      End;
    End;
  End;

  if CurStep=ssDone then Reset := ResetLanguages;
End;

#if ConfigurationName == "Release"
procedure InitializeWizard();
begin
  if InstallVCRedist() then begin
    idpSetDetailedMode( True );
    idpDownloadAfter( wpReady );
  end;
end;
#endif

#expr SaveToFile(SourcePath + "..\builds\Preprocessed.iss")
#expr Exec( Zip, "a -y -mx=9 builds\" + symbols_name + " ""..\" + Compiler + "\" + PlatformName + "\" + ConfigurationName + "\*.pdb""", ".." )
#expr Exec( Zip, "a -y -mx=9 -r -x!.vs -x!.svn -x!setup\builds\*.exe -x!setup\builds\*.txt -x!setup\builds\*.iss -x!Win32 -x!x64 -x!.vs -x!ipch -x!*.7z -x!*.log -x!*.bak -x!*.VC.db -x!*.VC.opendb -x!*.tmp -x!*.sdf -x!*.suo -x!*.ncb -x!*.user -x!*.opensdf builds\" + source_name + " ..", ".." )
