!define TITLE		"Search Export plugin for Shareaza"
!define VERSION		"1.0.0.1"
!define COMPANY		"Nikolay Raspopov"
!define FILENAME	"Win32\searchexport_setup_32_${VERSION}.exe"
!define COPYRIGHT	"Copyright (C) 2009 Nikolay Raspopov"
!define UNINST		"SearchExport_Uninst.exe"

Name "${TITLE}"

VIProductVersion "${VERSION}"
VIAddVersionKey ProductName "${TITLE}"
VIAddVersionKey ProductVersion "${VERSION}"
VIAddVersionKey OriginalFilename "${FILENAME}"
VIAddVersionKey FileDescription "${TITLE}"
VIAddVersionKey FileVersion "${VERSION}"
VIAddVersionKey CompanyName "${COMPANY}"
VIAddVersionKey LegalCopyright "${COPYRIGHT}"

CRCCheck On
XPStyle On
BrandingText "Shareaza Development Team"
SetOverwrite On
OutFile "${FILENAME}"
InstallDir "$PROGRAMFILES32\Shareaza"
InstallDirRegKey HKCU "Software\Shareaza\Shareaza" "Path"
ShowInstDetails show
ShowUninstDetails show
RequestExecutionLevel admin
SetCompressor /SOLID lzma

Var STARTMENU_FOLDER

!include "MUI2.nsh"

!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
; !define MUI_COMPONENTSPAGE_NODESC
; !define MUI_FINISHPAGE_NOAUTOCLOSE
; !define MUI_UNFINISHPAGE_NOAUTOCLOSE
!define MUI_ICON "..\..\setup\misc\install.ico"
!define MUI_UNICON "..\..\setup\misc\uninstall.ico"
!define MUI_HEADERIMAGE_BITMAP "..\..\setup\misc\corner_150x57.bmp"
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
!define MUI_HEADERIMAGE_UNBITMAP "..\..\setup\misc\corner_150x57.bmp"
!define MUI_HEADERIMAGE_UNBITMAP_NOSTRETCH
!define MUI_WELCOMEFINISHPAGE_BITMAP "..\..\setup\misc\sidebar.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "..\..\setup\misc\sidebar.bmp"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Shareaza"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Shareaza"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\setup\license\default.rtf"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "SimpChinese"

LangString AGAIN_WARN ${LANG_ENGLISH}     "Setup already running."
LangString AGAIN_WARN ${LANG_Russian}     "Установка уже запущена."
LangString AGAIN_WARN ${LANG_French}      "Setup already running."
LangString AGAIN_WARN ${LANG_German}      "Setup already running."
LangString AGAIN_WARN ${LANG_Spanish}     "Setup already running."
LangString AGAIN_WARN ${LANG_Japanese}    "Setup already running."
LangString AGAIN_WARN ${LANG_SimpChinese} "Setup already running."

LangString CLOSE_WARN ${LANG_ENGLISH}     "Please close Shareaza and try again."
LangString CLOSE_WARN ${LANG_Russian}     "Пожалуйста закройте Shareaza и попробуйте снова."
LangString CLOSE_WARN ${LANG_French}      "Please close Shareaza and try again."
LangString CLOSE_WARN ${LANG_German}      "Please close Shareaza and try again."
LangString CLOSE_WARN ${LANG_Spanish}     "Please close Shareaza and try again."
LangString CLOSE_WARN ${LANG_Japanese}    "Please close Shareaza and try again."
LangString CLOSE_WARN ${LANG_SimpChinese} "Please close Shareaza and try again."

Function .onInit
	SetShellVarContext all
	SetRegView 32

	; Disable second run
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "Global\${TITLE}") i .r1 ?e'
	Pop $R0
	StrCmp $R0 0 +3
	MessageBox MB_ICONSTOP|MB_OK "$(AGAIN_WARN)"
	Quit

FunctionEnd

Function un.onInit
	SetShellVarContext all
	SetRegView 32

	; Disable second run
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "Global\${TITLE}") i .r1 ?e'
	Pop $R0
	StrCmp $R0 0 +3
	MessageBox MB_ICONSTOP|MB_OK "$(AGAIN_WARN)"
	Quit

FunctionEnd

Section "Install"
	SetOutPath $INSTDIR

	; Close Shareaza before installation
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "Global\Shareaza") i .r1 ?e'
	System::Call 'kernel32::CloseHandle(i $1) i .r1'
	Pop $R0
	StrCmp $R0 0 +3
	MessageBox MB_ICONEXCLAMATION|MB_ABORTRETRYIGNORE "$(CLOSE_WARN)" IDRETRY -4 IDIGNORE +2
	Quit

	; Install plugin
	File /r "SearchExport Templates"
	File "Win32\Release\SearchExport.dll"
	RegDLL "$INSTDIR\SearchExport.dll"

	; Install shortcuts
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
	CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
	CreateShortCut  "$SMPROGRAMS\$STARTMENU_FOLDER\$(^UninstallCaption).lnk" "$INSTDIR\${UNINST}" "" "$INSTDIR\${UNINST}" 0
	!insertmacro MUI_STARTMENU_WRITE_END

	; Install Uninstaller
	WriteRegStr   HKCU "Software\Shareaza\Shareaza" "Path" $INSTDIR
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "DisplayName"     "${TITLE}"
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "UninstallString" "$INSTDIR\${UNINST}"
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "InstallLocation" "$INSTDIR"
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "DisplayIcon"     "$INSTDIR\SearchExport.dll,0"
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "DisplayVersion"  "${VERSION}"
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "Publisher"       "${COMPANY}"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "NoModify"         1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "NoRepair"         1
	WriteUninstaller "${UNINST}"
SectionEnd

Section "Uninstall"
	SetOutPath $TEMP

	; Close Shareaza before uninstallation
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "Global\Shareaza") i .r1 ?e'
	System::Call 'kernel32::CloseHandle(i $1) i .r1'
	Pop $R0
	StrCmp $R0 0 +3
	MessageBox MB_ICONEXCLAMATION|MB_ABORTRETRYIGNORE "$(CLOSE_WARN)" IDRETRY -4 IDIGNORE +2
	Quit

	; Uninstall plugin
	UnRegDLL "$INSTDIR\SearchExport.dll"
	Delete   "$INSTDIR\SearchExport.dll"
	RmDir /r "$INSTDIR\SearchExport Templates"

	; Uninstall shortcuts
	!insertmacro MUI_STARTMENU_GETFOLDER Application $STARTMENU_FOLDER
	Delete "$SMPROGRAMS\$STARTMENU_FOLDER\$(^UninstallCaption).lnk"
	RmDir  "$SMPROGRAMS\$STARTMENU_FOLDER"

	; Uninstall Uninstaller
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}"
	Delete "$INSTDIR\${UNINST}"
	RmDir  "$INSTDIR"
SectionEnd

!appendfile "SearchExport.trg" "[${__TIMESTAMP__}] ${TITLE} ${VERSION} ${FILENAME}$\n"
