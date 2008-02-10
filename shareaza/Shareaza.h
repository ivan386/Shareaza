//
// Shareaza.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "Buffer.h"

#ifndef WIN64
static HMODULE __stdcall LoadUnicows();
#endif

class CUPnPFinder;
class CMainWnd;
class CSplashDlg;
class CFontManager;

class CShareazaCommandLineInfo : public CCommandLineInfo
{
public:
	CShareazaCommandLineInfo();

	virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);

	BOOL m_bTray;
	BOOL m_bNoSplash;
	BOOL m_bNoAlphaWarning;
	INT  m_nGUIMode;

private:
	CShareazaCommandLineInfo(const CShareazaCommandLineInfo&);
	CShareazaCommandLineInfo& operator=(const CShareazaCommandLineInfo&);
};

class CShareazaApp : public CWinApp
{
public:
	CShareazaApp();

	HANDLE				m_pMutex;
	CMutex				m_pSection;
	WORD				m_nVersion[4];
	BYTE				m_pBTVersion[4];			// SZxx
	CString				m_sVersion;					// x.x.x.x
	CString				m_sSmartAgent;				// Shareaza x.x.x.x
	CString				m_sBuildDate;
	CFont				m_gdiFont;
	CFont				m_gdiFontBold;
	CFont				m_gdiFontLine;
	CWnd*				m_pSafeWnd;
	BOOL				m_bLive;
	BOOL				m_bInteractive;
	BOOL				m_bNT;						// NT based core. (NT, 2000, XP, etc)
	BOOL				m_bServer;					// Server version
	BOOL				m_bWinME;					// Windows Millennium
	BOOL				m_bLimitedConnections;		// Networking is limited (XP SP2)
	DWORD				m_dwWindowsVersion;			// Windows version
	DWORD				m_dwWindowsVersionMinor;	// Windows minor version
	QWORD				m_nPhysicalMemory;			// Physical RAM installed
	BOOL				m_bMenuWasVisible;			// For the menus in media player window
	int					m_nDefaultFontSize;			// The basic font size. (11)
	CString				m_sDefaultFont;				// Main font. (Tahoma)
	CString				m_sPacketDumpFont;			// Packet Window. (Lucida Console)
	CString				m_sSystemLogFont;			// System Window. (Courier New)
	boost::scoped_ptr< CUPnPFinder > m_pUPnPFinder;
	TRISTATE			m_bUPnPPortsForwarded;		// UPnP values are assigned when the discovery is complete
	TRISTATE			m_bUPnPDeviceConnected;		// or when the service notifies
	DWORD				m_nUPnPExternalAddress;
	DWORD				m_dwLastInput;				// Time of last input event (in secs)
	HHOOK				m_hHookKbd;
	HHOOK				m_hHookMouse;

	// GDI and display monitor functions
	HINSTANCE	m_hUser32;
	BOOL		(WINAPI *m_pfnSetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD);
	BOOL		(WINAPI *m_pfnGetMonitorInfoA)(HMONITOR, LPMONITORINFO);
	HMONITOR	(WINAPI *m_pfnMonitorFromRect)(LPCRECT, DWORD);
	HMONITOR	(WINAPI *m_pfnMonitorFromWindow)(HWND, DWORD);
	HWND		(WINAPI *m_pfnGetAncestor)(HWND, UINT);
	UINT		(WINAPI *m_pfnPrivateExtractIconsW)(LPCWSTR, int, int, int, HICON*, UINT*, UINT, UINT);

	HINSTANCE	m_hKernel;
	BOOL		(WINAPI *m_pfnGetDiskFreeSpaceExW)(LPCWSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);

	HINSTANCE	m_hShellFolder;
	HRESULT		(WINAPI *m_pfnSHGetFolderPathW)(HWND, int, HANDLE, DWORD, LPWSTR);

	// For RTL layout support
	HINSTANCE	m_hGDI32;
	DWORD		(WINAPI *m_pfnSetLayout)(HDC, DWORD);

	// For themes functions
	HINSTANCE	m_hTheme;
	HRESULT		(WINAPI *m_pfnSetWindowTheme)(HWND, LPCWSTR, LPCWSTR);
	BOOL		(WINAPI *m_pfnIsThemeActive)(VOID);
	HANDLE		(WINAPI *m_pfnOpenThemeData)(HWND, LPCWSTR);
	HRESULT		(WINAPI *m_pfnCloseThemeData)(HANDLE);
	HRESULT		(WINAPI *m_pfnDrawThemeBackground)(HANDLE, HDC, int, int, const RECT*, const RECT*);
	HRESULT		(WINAPI *m_pfnEnableThemeDialogTexture)(HWND, DWORD);
	HRESULT		(WINAPI *m_pfnDrawThemeParentBackground)(HWND, HDC, RECT*);
	HRESULT		(WINAPI *m_pfnGetThemeBackgroundContentRect)(HANDLE, HDC, int, int, const RECT*, RECT*);
	HRESULT		(WINAPI *m_pfnGetThemeSysFont)(HANDLE, int, LOGFONT);
	HRESULT		(WINAPI *m_pfnDrawThemeText)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT*);

	// Power schemes management
	HINSTANCE	m_hPowrProf;
	BOOLEAN		(WINAPI *m_pfnGetActivePwrScheme)(PUINT);
	BOOLEAN		(WINAPI *m_pfnGetCurrentPowerPolicies)(PGLOBAL_POWER_POLICY, PPOWER_POLICY);
	BOOLEAN		(WINAPI *m_pfnSetActivePwrScheme)(UINT, PGLOBAL_POWER_POLICY, PPOWER_POLICY);

	HINSTANCE	m_hShlWapi;
	BOOL		(WINAPI *m_pfnAssocIsDangerous)(LPCWSTR);

	// GeoIP - IP to Country lookup
	HINSTANCE	m_hGeoIP;
	GeoIP*		m_pGeoIP;
	GeoIP_country_code_by_addrFunc	m_pfnGeoIP_country_code_by_addr;
	GeoIP_country_name_by_addrFunc	m_pfnGeoIP_country_name_by_addr;

	HINSTANCE	m_hLibGFL;

	HINSTANCE			CustomLoadLibrary(LPCTSTR);
	CMainWnd*			SafeMainWnd() const;
	void				Message(int nType, UINT nID, ...) const;
	void				Message(int nType, CString strFormat, ...) const;
	BOOL				InternalURI(LPCTSTR pszURI);
	void				PrintMessage(int nType, CString& strLog) const;
	void				LogMessage(LPCTSTR strLog) const;

	CString				GetCountryCode(IN_ADDR pAddress) const;
	CString				GetCountryName(IN_ADDR pAddress) const;

	CFontManager*		m_pFontManager;
	
	// Open file or url. Returns NULL always.
	virtual CDocument*	OpenDocumentFile(LPCTSTR lpszFileName);
	// Open file or url (generic function)
	static BOOL			Open(LPCTSTR lpszFileName, BOOL bDoIt);
	// Open .lnk file
	static BOOL			OpenShellShortcut(LPCTSTR lpszFileName, BOOL bDoIt);
	// Open .url file
	static BOOL			OpenInternetShortcut(LPCTSTR lpszFileName, BOOL bDoIt);
	// Open .torrent file
	static BOOL			OpenTorrent(LPCTSTR lpszFileName, BOOL bDoIt);
	// Open .co or .collection file
	static BOOL			OpenCollection(LPCTSTR lpszFileName, BOOL bDoIt);
	// Open url
	static BOOL			OpenURL(LPCTSTR lpszFileName, BOOL bDoIt, BOOL bSilent = FALSE);

protected:
	mutable CCriticalSection	m_csMessage;
	CShareazaCommandLineInfo	m_ocmdInfo;

	virtual BOOL		InitInstance();
	virtual int			ExitInstance();
	virtual void		WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);

	void				GetVersionNumber();
	void				InitResources();

	DECLARE_MESSAGE_MAP()
};

extern CShareazaApp theApp;

//
// Utility Functions
//

CRuntimeClass* AfxClassForName(LPCTSTR pszClass);

BOOL LoadString(CString& str, UINT nID);
LPCTSTR _tcsistr(LPCTSTR pszString, LPCTSTR pszPattern);
LPCTSTR _tcsnistr(LPCTSTR pszString, LPCTSTR pszPattern, size_t plen);
void Split(const CString& strSource, TCHAR cDelimiter, CStringArray& pAddIt, BOOL bAddFirstEmpty = FALSE);
BOOL LoadSourcesString(CString& str, DWORD num, bool bFraction=false);

DWORD	TimeFromString(LPCTSTR psz);
CString	TimeToString(time_t tVal);
BOOL	TimeFromString(LPCTSTR psz, FILETIME* pTime);
CString	TimeToString(FILETIME* pTime);

void	RecalcDropWidth(CComboBox* pWnd);
// Load and add icon to CImageList, mirrored if needed
int		AddIcon(UINT nIcon, CImageList& gdiImageList);
// Add icon to CImageList, mirrored if needed
int		AddIcon(HICON hIcon, CImageList& gdiImageList);
// Create mirrored icon. Returns:
// mirrored icon (original destroyed if needed) if succeed or original icon otherwise
HICON	CreateMirroredIcon(HICON hIconOrig, BOOL bDestroyOriginal = TRUE);
HBITMAP	CreateMirroredBitmap(HBITMAP hbmOrig);

#ifdef _DEBUG
	#define ALMOST_INFINITE	INFINITE
#else
	#define ALMOST_INFINITE	20000
#endif

inline void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName);
HANDLE BeginThread(LPCSTR pszName, AFX_THREADPROC pfnThreadProc,
	LPVOID pParam, int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
	DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);
void CloseThread(HANDLE* phThread, DWORD dwTimeout = ALMOST_INFINITE);

LRESULT CALLBACK KbdHook(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MouseHook(int nCode, WPARAM wParam, LPARAM lParam);

CString GetFolderPath( int nFolder );
CString GetWindowsFolder();
CString GetProgramFilesFolder();
CString GetDocumentsFolder();
CString GetAppDataFolder();
CString GetLocalAppDataFolder();

// Create directory. If one or more of the intermediate folders do not exist, they are created as well.
BOOL CreateDirectory(LPCTSTR szPath);

// Loads RT_HTML or RT_GZIP resource as string
CString LoadHTML(HINSTANCE hInstance, UINT nResourceID);

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

// Displays a dialog box enabling the user to select a Shell folder
CString BrowseForFolder(UINT nTitle, LPCTSTR szInitialPath = NULL, HWND hWnd = NULL);
CString BrowseForFolder(LPCTSTR szTitle, LPCTSTR szInitialPath = NULL, HWND hWnd = NULL);

typedef enum
{
	sNone = 0,
	sNumeric = 1,
	sRegular = 2,
	sKanji = 4,
	sHiragana = 8,
	sKatakana = 16
} ScriptType;

struct CompareNums
{
	inline bool operator()(WORD lhs, WORD rhs) const
	{
		return lhs > rhs;
	}
};

inline bool IsCharacter(TCHAR nChar)
{
    WORD nCharType = 0;
	
	if ( GetStringTypeExW( LOCALE_NEUTRAL, CT_CTYPE3, &nChar, 1, &nCharType ) )
		return ( ( nCharType & C3_ALPHA ) == C3_ALPHA ||
				 ( ( nCharType & C3_KATAKANA ) == C3_KATAKANA ||
				   ( nCharType & C3_HIRAGANA ) == C3_HIRAGANA ) && 
				   !( ( nCharType & C3_SYMBOL ) == C3_SYMBOL )  ||
				 ( nCharType & C3_IDEOGRAPH ) == C3_IDEOGRAPH ||
				 _istdigit( nChar ) );

	return false;
}

inline bool IsHiragana(TCHAR nChar)
{
	WORD nCharType = 0;
	
	if ( GetStringTypeExW( LOCALE_NEUTRAL, CT_CTYPE3, &nChar, 1, &nCharType ) )
		return ( ( nCharType & C3_HIRAGANA ) == C3_HIRAGANA );
	return false;
}

inline bool IsKatakana(TCHAR nChar)
{
	WORD nCharType = 0;
	
	if ( GetStringTypeExW( LOCALE_NEUTRAL, CT_CTYPE3, &nChar, 1, &nCharType ) )
		return ( ( nCharType & C3_KATAKANA ) == C3_KATAKANA );
	return false;
}

inline bool IsKanji(TCHAR nChar)
{
	WORD nCharType = 0;
	
	if ( GetStringTypeExW( LOCALE_NEUTRAL, CT_CTYPE3, &nChar, 1, &nCharType ) )
		return ( ( nCharType & C3_IDEOGRAPH ) == C3_IDEOGRAPH );
	return false;
}

inline bool IsWord(LPCTSTR pszString, size_t nStart, size_t nLength)
{
	for ( pszString += nStart ; *pszString && nLength ; pszString++, nLength-- )
	{
		if ( _istdigit( *pszString ) ) return false;
	}
	return true;
}

inline bool IsHasDigit(LPCTSTR pszString, size_t nStart, size_t nLength)
{
	for ( pszString += nStart ; *pszString && nLength ; pszString++, nLength-- )
	{
		if ( _istdigit( *pszString ) ) return true;
	}
	return false;
}

inline bool IsNumeric(LPCTSTR pszString, size_t nStart, size_t nLength)
{
	bool bDigit = true;
	for ( pszString += nStart ; *pszString && nLength ; pszString++, nLength-- )
	{
		if ( !_istdigit( *pszString ) ) bDigit = false;
	}
	return bDigit;
}

inline void IsType(LPCTSTR pszString, size_t nStart, size_t nLength, bool& bWord, bool& bDigit, bool& bMix)
{
	bWord = false;
	bDigit = false;
	for ( pszString += nStart ; *pszString && nLength ; pszString++, nLength-- )
	{
		if ( _istdigit( *pszString ) ) bDigit = true;
		else if ( IsCharacter( *pszString ) ) bWord = true;
	}

	bMix = bWord && bDigit;
	if ( bMix )
	{
		bWord = false;
		bDigit = false;
	}
}


// To see the color of the message you must look at CTextCtrl::CTextCtrl() in CtrlText.cpp
#define MSG_DEFAULT			0
#define MSG_SYSTEM			1
#define MSG_DOWNLOAD		1
#define MSG_ERROR			2
#define MSG_DEBUG			3
#define MSG_TEMP			4
#define MSG_DISPLAYED_ERROR	5	// It behave as MSG_ERROR but it is displayed also when VerboseMode is off

#define WM_WINSOCK		(WM_APP+101)
#define WM_VERSIONCHECK	(WM_APP+102)
#define WM_OPENCHAT		(WM_APP+103)
#define WM_TRAY			(WM_APP+104)
#define WM_URL			(WM_APP+105)
#define WM_SKINCHANGED	(WM_APP+106)
#define WM_COLLECTION	(WM_APP+107)
#define WM_OPENSEARCH	(WM_APP+108)
#define WM_LOG			(WM_APP+109)
#define WM_LIBRARYSEARCH (WM_APP+110)
#define WM_PLAYFILE		(WM_APP+111)
#define WM_ENQUEUEFILE	(WM_APP+112)
#define WM_SETALPHA		(WM_APP+113)
#define WM_THUMBFAILED	(WM_APP+114)

#define WM_AFX_SETMESSAGESTRING 0x0362
#define WM_AFX_POPMESSAGESTRING 0x0375
#define WM_IDLEUPDATECMDUI		0x0363

#define ID_PLUGIN_FIRST	27000
#define ID_PLUGIN_LAST	27999


#define THUMB_STORE_SIZE	128


// Client's name
#define CLIENT_NAME			"Shareaza"


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


// Drag-n-drop stuff

#define MAX_DRAG_SIZE		256
#define MAX_DRAG_SIZE_2		(MAX_DRAG_SIZE/2)
#define DRAG_COLOR_KEY		(RGB(250,255,250))	// Light-green
#define DRAG_HOVER_TIME		1000				// Dragging mouse press button after X ms

extern const LPCTSTR RT_BMP;
extern const LPCTSTR RT_JPEG;
extern const LPCTSTR RT_PNG;
extern const LPCTSTR RT_GZIP;

extern double scaleX;
extern double scaleY;
#define SCALEX(argX) ((int) ((argX) * scaleX))
#define SCALEY(argY) ((int) ((argY) * scaleY))
