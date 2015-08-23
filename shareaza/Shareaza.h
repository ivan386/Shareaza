//
// Shareaza.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
// This file is part of SHAREAZA (shareaza.sourceforge.net)
//
// Shareaza is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Shareaza is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#include "Resource.h"
#include "ComObject.h"

class CBuffer;
class CDatabase;
class CFontManager;
class CMainWnd;
class CPacketWnd;
class CShareazaFile;
class CSplashDlg;


class __declspec(novtable) CLogMessage
{
public:
	CLogMessage(WORD nType, const CString& strLog) : m_strLog( strLog ), m_nType( nType ), m_Time( CTime::GetCurrentTime() ) {}
	CString m_strLog;
	WORD	m_nType;
	CTime	m_Time;
};

typedef CList< CLogMessage* > CLogMessageList;


class CShareazaCommandLineInfo : public CCommandLineInfo
{
public:
	CShareazaCommandLineInfo();

	virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);

	BOOL	m_bTray;
	BOOL	m_bNoSplash;
	BOOL	m_bNoAlphaWarning;
	INT		m_nGUIMode;
	BOOL	m_bHelp;
	CString	m_sTask;
	BOOL	m_bWait;

private:
	CShareazaCommandLineInfo(const CShareazaCommandLineInfo&);
	CShareazaCommandLineInfo& operator=(const CShareazaCommandLineInfo&);
};


class CShareazaApp : public CWinApp
{
	DECLARE_DYNAMIC(CShareazaApp)

public:
	CShareazaApp();
	virtual ~CShareazaApp();

	HANDLE				m_pMutex;
	CMutex				m_pSection;
	WORD				m_nVersion[4];
	BYTE				m_pBTVersion[4];			// SZxx
	CString				m_sVersion;					// x.x.x.x
	CString				m_sVersionLong;				// x.x.x.x Release/Debug 32-bit/64-bit (rXXXX date)
	CString				m_sSmartAgent;				// Shareaza x.x.x.x
	CString				m_sBuildDate;
	CString				m_strBinaryPath;			// Shareaza.exe path
	BYTE				m_nFontQuality;
	CFont				m_gdiFont;
	CFont				m_gdiFontBold;
	CFont				m_gdiFontLine;
	CWnd*				m_pSafeWnd;
	volatile LONG		m_bBusy;					// Shareaza is busy
	volatile bool		m_bInteractive;				// Shareaza begins initialization
	volatile bool		m_bLive;					// Shareaza fully initialized
	volatile bool		m_bClosing;					// Shareaza begins closing
	bool				m_bIsVistaOrNewer;			// Is OS Vista or newer
	bool				m_bIs7OrNewer;				// Is OS 7 or newer
	bool				m_bLimitedConnections;		// Networking is limited (XP SP2)
	BOOL				m_bMenuWasVisible;			// For the menus in media player window
	DWORD				m_nLastInput;				// Time of last input event (in secs)
	HHOOK				m_hHookKbd;
	HHOOK				m_hHookMouse;
	CPacketWnd*			m_pPacketWnd;				// Packet Window (NULL - not opened)
	CShareazaCommandLineInfo m_cmdInfo;				// Command-line options

	// Cryptography Context handle
	HCRYPTPROV			m_hCryptProv;

	// Kernel functions
	HRESULT		(WINAPI *m_pRegisterApplicationRestart)( __in_opt PCWSTR pwzCommandline, __in DWORD dwFlags );

	// For themes functions
	HINSTANCE			m_hTheme;
	HRESULT		(WINAPI *m_pfnSetWindowTheme)(HWND, LPCWSTR, LPCWSTR);
	BOOL		(WINAPI *m_pfnIsThemeActive)(VOID);
	HANDLE		(WINAPI *m_pfnOpenThemeData)(HWND, LPCWSTR);
	HRESULT		(WINAPI *m_pfnCloseThemeData)(HANDLE);
	HRESULT		(WINAPI *m_pfnDrawThemeBackground)(HANDLE, HDC, int, int, const RECT*, const RECT*);
	HRESULT		(WINAPI *m_pfnGetThemeSysFont)(HTHEME, int, __out LOGFONTW* );

	// Shell functions
	HINSTANCE			m_hShlWapi;
	BOOL		(WINAPI *m_pfnAssocIsDangerous)(LPCWSTR);

	HINSTANCE			m_hShell32;
	HRESULT		(WINAPI *m_pfnSHGetFolderPathW)(__reserved HWND hwnd, __in int csidl, __in_opt HANDLE hToken, __in DWORD dwFlags, __out_ecount(MAX_PATH) LPWSTR pszPath);
	HRESULT		(WINAPI *m_pfnSHGetKnownFolderPath)(__in REFKNOWNFOLDERID rfid, __in DWORD /* KNOWN_FOLDER_FLAG */ dwFlags, __in_opt HANDLE hToken, __deref_out PWSTR *ppszPath);
	HRESULT		(WINAPI *m_pfnSHCreateItemFromParsingName)(__in PCWSTR pszPath, __in_opt IBindCtx *pbc, __in REFIID riid, __deref_out void **ppv);
	HRESULT		(WINAPI *m_pfnSHGetPropertyStoreFromParsingName)(__in PCWSTR pszPath, __in_opt IBindCtx *pbc, __in GETPROPERTYSTOREFLAGS flags, __in REFIID riid, __deref_out void **ppv);
	HRESULT		(WINAPI *m_pfnSetCurrentProcessExplicitAppUserModelID)(__in PCWSTR pszAppID);
	HRESULT		(WINAPI *m_pfnSHGetImageList)(__in int iImageList, __in REFIID riid, __out void **ppv);

	HINSTANCE			m_hUser32;
	BOOL		(WINAPI *m_pfnChangeWindowMessageFilter)(UINT message, DWORD dwFlag);
	BOOL		(WINAPI *m_pfnShutdownBlockReasonCreate)(_In_ HWND hWnd, _In_ LPCWSTR pwszReason);
	BOOL		(WINAPI *m_pfnShutdownBlockReasonDestroy)(_In_ HWND hWnd);

	BOOL GetPropertyStoreFromParsingName( LPCWSTR pszPath, IPropertyStore**ppv );

	// GeoIP - IP to Country lookup
	HINSTANCE			m_hGeoIP;
	GeoIP*				m_pGeoIP;
	typedef GeoIP* (*GeoIP_newFunc)(int);
	typedef int (*GeoIP_cleanupFunc)(void);
	typedef void (*GeoIP_deleteFunc)(GeoIP* gi);
	typedef const char * (*GeoIP_country_code_by_ipnumFunc) (GeoIP* gi, unsigned long ipnum);
	typedef const char * (*GeoIP_country_name_by_ipnumFunc) (GeoIP* gi, unsigned long ipnum);
	GeoIP_cleanupFunc				m_pfnGeoIP_cleanup;
	GeoIP_deleteFunc				m_pfnGeoIP_delete;
	GeoIP_country_code_by_ipnumFunc	m_pfnGeoIP_country_code_by_ipnum;
	GeoIP_country_name_by_ipnumFunc	m_pfnGeoIP_country_name_by_ipnum;

	HINSTANCE			m_hLibGFL;

	HINSTANCE			CustomLoadLibrary(LPCTSTR);
	CMainWnd*			SafeMainWnd() const;
	BOOL				InternalURI(LPCTSTR pszURI);

	// Logging functions
	CLogMessageList		m_oMessages;	// Log temporary storage
	CCriticalSection	m_csMessage;	// m_oMessages guard
	bool				IsLogDisabled(WORD nType) const;
	void				ShowStartupText();
	void				Message(WORD nType, UINT nID, ...);
	void				Message(WORD nType, LPCTSTR pszFormat, ...);
	// Log to file and to system window
	void				PrintMessage(WORD nType, const CString& strLog);

	void				SplashStep(LPCTSTR pszMessage = NULL, int nMax = 0, bool bClosing = false);
	void				SplashAbort();

	CString				GetCountryCode(IN_ADDR pAddress) const;
	CString				GetCountryName(IN_ADDR pAddress) const;

	// Open file or url. Returns NULL always.
	virtual CDocument*	OpenDocumentFile(LPCTSTR lpszFileName);
	// Open file or url (generic function)
	BOOL				Open(LPCTSTR lpszFileName, BOOL bDoIt, BOOL bDispay = FALSE);
	// Show file in Library
	BOOL				DisplayFile(LPCTSTR lpszFileName, BOOL bDoIt);
	// Open host list file
	BOOL				OpenImport(LPCTSTR lpszFileName, BOOL bDoIt);
	// Open .lnk file
	BOOL				OpenShellShortcut(LPCTSTR lpszFileName, BOOL bDoIt);
	// Open .url file
	BOOL				OpenInternetShortcut(LPCTSTR lpszFileName, BOOL bDoIt);
	// Open .torrent file
	BOOL				OpenTorrent(LPCTSTR lpszFileName, BOOL bDoIt);
	// Open Shareaza, eMule or DC++ collection file
	BOOL				OpenCollection(LPCTSTR lpszFileName, BOOL bDoIt);
	// Open URL
	BOOL				OpenURL(LPCTSTR lpszFileName, BOOL bDoIt, BOOL bSilent = FALSE);
	// Open Shareaza Download file
	BOOL				OpenDownload(LPCTSTR lpszFileName, BOOL bDoIt);

	CString				GetWindowsFolder() const;
	CString				GetProgramFilesFolder64() const;
	CString				GetProgramFilesFolder() const;
	CString				GetDocumentsFolder() const;
	CString				GetDownloadsFolder() const;
	CString				GetAppDataFolder() const;
	CString				GetLocalAppDataFolder() const;

	// Rename, delete or release file.
	// pszTarget == 0 - delete file; pszTarget == 1 - release file.
	void				OnRename(LPCTSTR strSource, LPCTSTR pszTarget = (LPCTSTR)1);

	// Get database handler
	// Must be freed by "delete" operator.
	CDatabase*			GetDatabase() const;

	// Copy text to clipboard (Unicode)
	BOOL SetClipboardText(const CString& strText);

protected:
	CSplashDlg*			m_dlgSplash;		// Splash dialog

	virtual BOOL		InitInstance();
	virtual int			ExitInstance();
	virtual void		WinHelp(DWORD_PTR dwData, UINT nCmd = HELP_CONTEXT);
	virtual BOOL		Register();
	virtual BOOL		Unregister();
	virtual void		AddToRecentFileList(LPCTSTR lpszPathName);

	void				InitResources();	// Initialize Shareaza version, system info, load DLLs, etc.
	void				InitFonts();		// Create default fonts
	BOOL				ParseCommandLine();	// Parse and execute command-line

	void				LoadCountry();		// Load the GeoIP library for mapping IPs to countries
	void				FreeCountry();		// Free GeoIP resources

	DECLARE_MESSAGE_MAP()

private:
	CShareazaApp(const CShareazaApp&);
	CShareazaApp& operator=(const CShareazaApp&);
};

extern CShareazaApp			theApp;						// Shareaza Application
extern SYSTEM_INFO			System;						// System Information


class CProgressDialog : public CComPtr< IProgressDialog >
{
public:
	CProgressDialog(LPCTSTR szTitle, DWORD dwFlags = PROGDLG_NOCANCEL | PROGDLG_AUTOTIME);
	virtual ~CProgressDialog();

	void Progress(LPCTSTR szText, QWORD nCompleted = 0, QWORD nTotal = 0);
};

//
// Utility Functions
//

// Detect Administrative privileges
BOOL IsRunAsAdmin();

// Post message to main window in safe way
BOOL PostMainWndMessage(UINT Msg, WPARAM wParam = NULL, LPARAM lParam = NULL);

CRuntimeClass* AfxClassForName(LPCTSTR pszClass);

BOOL	LoadString(CString& str, UINT nID);
CString	LoadString(UINT nID);
BOOL	LoadSourcesString(CString& str, DWORD num, bool bFraction=false);

DWORD	TimeFromString(LPCTSTR psz);
CString	TimeToString(time_t tVal);
BOOL	TimeFromString(LPCTSTR psz, FILETIME* pTime);
CString	TimeToString(FILETIME* pTime);

void	RecalcDropWidth(CComboBox* pWnd, int nMargin = 0);
// Load 16x16, 32x32, 48x48 icons from .ico, .exe, .dll files
BOOL LoadIcon(LPCTSTR szFilename, HICON* phSmallIcon, HICON* phLargeIcon, HICON* phHugeIcon, int nIcon = 0);
// Load 16x16 icon from module pointed by its CLSID
HICON LoadCLSIDIcon(LPCTSTR szCLSID);
// Load and add icon to CImageList, mirrored if needed
int		AddIcon(UINT nIcon, CImageList& gdiImageList);
// Add icon to CImageList, mirrored if needed
int		AddIcon(HICON hIcon, CImageList& gdiImageList);
// Create mirrored icon. Returns:
// mirrored icon (original destroyed if needed) if succeed or original icon otherwise
HICON	CreateMirroredIcon(HICON hIconOrig, BOOL bDestroyOriginal = TRUE);
HBITMAP	CreateMirroredBitmap(HBITMAP hbmOrig);

LRESULT CALLBACK KbdHook(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MouseHook(int nCode, WPARAM wParam, LPARAM lParam);

// Generate safe file name for file system (bPath == true - allow path i.e. "\" symbol)
CString SafeFilename(CString strName, bool bPath = false);

// Create directory. If one or more of the intermediate folders do not exist, they are created as well.
BOOL CreateDirectory(LPCTSTR szPath);

// Delete file(s) with user confirmation
void DeleteFiles(CStringList& pList);

// Delete file in many ways
BOOL DeleteFileEx(LPCTSTR szFileName, BOOL bShared, BOOL bToRecycleBin, BOOL bEnableDelayed);

// Delete postponed file
void PurgeDeletes();

// Loads RT_HTML or RT_GZIP resource as string
CString LoadHTML(HINSTANCE hInstance, UINT nResourceID);
CString LoadRichHTML(UINT nResourceID, CString& strResponse, CShareazaFile* pFile = NULL);

// Save icon in .ico-format to buffer
BOOL SaveIcon(HICON hIcon, CBuffer& oBuffer, int colors = -1);

// Loads well-known resource for HTTP-uploading
bool ResourceRequest(const CString& strPath, CBuffer& pResponse, CString& sHeader);

// Mark file as downloaded from Internet (using NTFS stream)
bool MarkFileAsDownload(const CString& sFilename);

// Load GUID from NTFS stream of file
bool LoadGUID(const CString& sFilename, Hashes::Guid& oGUID);

// Save GUID to NTFS stream of file
bool SaveGUID(const CString& sFilename, const Hashes::Guid& oGUID);

// Resolve shell shortcut (.lnk file)
CString ResolveShortcut(LPCTSTR lpszFileName);

// Get Win32 API error description
CString GetErrorString(DWORD dwError = GetLastError());

// Show message box using GetErrorString() message
void ReportError(DWORD dwError = GetLastError());

// Displays a dialog box enabling the user to select a Shell folder
CString BrowseForFolder(UINT nTitle, LPCTSTR szInitialPath = NULL, HWND hWnd = NULL);
CString BrowseForFolder(LPCTSTR szTitle, LPCTSTR szInitialPath = NULL, HWND hWnd = NULL);

// Do message loop
void SafeMessageLoop();

// Detect full screen application
BOOL IsUserUsingFullscreen();

// Start Windows service
BOOL AreServiceHealthy(LPCTSTR szService);

// Creates shell link
IShellLink* CreateShellLink(LPCWSTR szTargetExecutablePath, LPCWSTR szCommandLineArgs, LPCWSTR szTitle, LPCWSTR szIconPath, int nIconIndex, LPCWSTR szDescription);

// Select existing string of ComboBox, or add and select a new one
void AddAndSelect(CComboBox& wndBox, const CString& sText);

struct CompareNums
{
	bool operator()(WORD lhs, WORD rhs) const
	{
		return lhs > rhs;
	}
};

// Use with whole numbers only
template <typename T>
inline T GetRandomNum(const T& min, const T& max)
{
	if ( theApp.m_hCryptProv != 0 )
	{
		T nRandom = 0;
		if ( CryptGenRandom( theApp.m_hCryptProv, sizeof( T ), (BYTE*)&nRandom ) )
			return static_cast< T >( (double)nRandom  * ( (double)max - (double)min + 1 ) / ( (double)static_cast< T >( -1 ) + 1 ) + min );
	}

	// Fallback to non-secure method
	return static_cast< T >( (double)rand() * ( max - min + 1 ) / ( (double)RAND_MAX + 1 ) + min );
}

template <>
__int8 GetRandomNum<__int8>(const __int8& min, const __int8& max);

template <>
__int16 GetRandomNum<__int16>(const __int16& min, const __int16& max);

template <>
__int32 GetRandomNum<__int32>(const __int32& min, const __int32& max);

template <>
__int64 GetRandomNum<__int64>(const __int64& min, const __int64& max);

#define WM_WINSOCK			(WM_APP+101)	// Winsock messages proxy to Network object ( used by WSAAsyncGetHostByName() function )
#define WM_VERSIONCHECK		(WM_APP+102)	// Version check ( WAPARM: VERSION_CHECK nCode, LPARAM: unused )
#define WM_OPENCHAT			(WM_APP+103)	// Open chat window ( WAPARM: CChatSession* pChat, LPARAM: unused )
#define WM_TRAY				(WM_APP+104)	// Tray icon notification ( WPARAM: unused, LPARAM: uMouseMessage )
#define WM_URL				(WM_APP+105)	// Open URL ( WPARAM: CShareazaURL* pURL, LPARAM: unused )
#define WM_SKINCHANGED		(WM_APP+106)	// Skin change ( WPARAM: unused, LPARAM: unused )
#define WM_COLLECTION		(WM_APP+107)	// Open collection file ( WPARAM: LPTSTR szFilename, LPARAM: unused )
#define WM_OPENSEARCH		(WM_APP+108)	// Open new search ( WPARAM: CQuerySearch* pSearch, LPARAM: unused )
#define WM_LIBRARYSEARCH	(WM_APP+110)	// Start file library search ( WPARAM: LPTSTR pszSearch, LPARAM: unused )
#define WM_PLAYFILE			(WM_APP+111)	// Play file by media system ( WPARAM: TRISTATE bForcePlay - TRI_TRUE - force play, TRI_FALSE - force enqueue, LPARAM: CString* pFilename )
#define WM_SETALPHA			(WM_APP+113)	// Increase/decrease main window transparency ( WPARAM: 0 - to decrease or 1 - to increase, LPARAM: unused )
#define WM_METADATA			(WM_APP+114)	// Set/clear library meatapanel data and status message ( WPARAM: CMetaPanel* pPanelData, LPARAM: LPCTSTR pszMessage )
#define WM_SANITY_CHECK		(WM_APP+115)	// Run allsystem check against banned hosts ( WPARAM: unused, LPARAM: unused )
#define WM_NOWUPLOADING		(WM_APP+117)	// New upload notification ( WPARAM: unused, LPARAM: CString* pFilename )
#define WM_TORRENT			(WM_APP+118)	// Open torrent file ( WPARAM: LPTSTR szFilename, LPARAM: unused )
#define WM_IMPORT			(WM_APP+119)	// Import hub list file ( WPARAM: LPTSTR szFilename, LPARAM: unused )

// WM_COPYDATA types
#define COPYDATA_SCHEDULER	0				// Scheduler task ( lpData: LPCTSTR szTaskData - encoded string )
#define COPYDATA_OPEN		1				// Open file ( lpData: LPCTSTR szFilename - file name or URL )

#define ID_PLUGIN_FIRST	27000
#define ID_PLUGIN_LAST	27999

#define WM_COPYGLOBALDATA	0x0049			// Undocumented way for drag-n-drop

#define PANEL_WIDTH			200				// Left panel default size in pixels (Home, Search, IRC tabs)
#define THUMB_STORE_SIZE	128				// Thumbnail dimensions (128x128 px)

#define HTTP_HEADER_MAX_LINE	(256 * 1024)// Maximum allowed size of single HTTP-header line (256 Kb)


// Client's name
#define CLIENT_NAME			"Shareaza"
#define CLIENT_NAME_T		_T( CLIENT_NAME )

// Client's main window class name
#define CLIENT_HWND			CLIENT_NAME_T _T("MainWnd")

// Client's settings key
#define REGISTRY_KEY		_T("Software\\") CLIENT_NAME_T _T("\\") CLIENT_NAME_T

// Network ID stuff

// 4 Character vendor code (used on G1, G2)
// BEAR, LIME, RAZA, RAZB, etc
#define VENDOR_CODE			"RAZA"

// ed2k client ID number.
// 0 = eMule, 1 = cDonkey, 4 = old Shareaza alpha/beta/mod/fork, 0x28 (40) = Shareaza, 0xcb (203) = ShareazaPlus with RazaCB core, etc
#define ED2K_CLIENT_ID		40

// 2 Character BT peer-id code
// SZ = Shareaza, S~ = old Shareaza alpha/beta , CB = ShareazaPlus with RazaCB core, AZ = Azureus, etc
#define BT_ID1				'S'
#define BT_ID2				'Z'

#define WEB_SITE			"http://shareaza.sourceforge.net/"
#define WEB_SITE_T			_T( WEB_SITE )

// URLs used by Shareaza
// -----------------------------------------------------------------------
// ID					URL								Notes
// -----------------------------------------------------------------------
//
// Help
//
// ID_HELP_FAQ		help/?faq
// ID_HELP_TEST		help/test/?port=x&lang=x&Version=x.x.x.x
//
// Help/Guides
//
// ID_HELP_GUIDE	help/?guide
// ID_HELP_ROUTER	help/?router
// ID_HELP_SECURITY	help/?security
// ID_HELP_CODEC	help/?codec
//
// Help/Websites
//
// ID_HELP_HOMEPAGE	?Version=x.x.x.x
// ID_HELP_FORUMS	help/?forum
// ID_HELP_UPDATE	help/update/?Version=x.x.x.x			unused
// ID_HELP_WEB_1	help/external/?link1					unused
// ID_HELP_WEB_2	help/external/?link1					unused
// ID_HELP_WEB_3	help/external/?link1					unused
// ID_HELP_WEB_4	help/external/?link1					unused
// ID_HELP_WEB_5	help/external/?link1					unused
// ID_HELP_WEB_6	help/external/?link1					Skin Guide
//
// ID_HELP_TORRENT	help/?torrentencoding					Torrent Encoding Help
// -				?id=support								BugTrap
// -				version/?Version=x.x.x.x&&Platform=x	CVersionChecker


// Drag-n-drop stuff

#define MAX_DRAG_SIZE		256
#define MAX_DRAG_SIZE_2		(MAX_DRAG_SIZE/2)
#define DRAG_COLOR_KEY		(RGB(250,255,250))	// Light-green
#define DRAG_HOVER_TIME		1000				// Dragging mouse press button after X ms

extern const LPCTSTR RT_BMP;
extern const LPCTSTR RT_JPEG;
extern const LPCTSTR RT_PNG;
extern const LPCTSTR RT_GZIP;
