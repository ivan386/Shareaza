//
// Shareaza.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
	BOOL m_bHelp;

private:
	CShareazaCommandLineInfo(const CShareazaCommandLineInfo&);
	CShareazaCommandLineInfo& operator=(const CShareazaCommandLineInfo&);
};

class __declspec(novtable) CLogMessage
{
public:
	inline CLogMessage(WORD nType, const CString& strLog) :
		m_strLog( strLog ),
		m_nType( nType )
	{
	}
	CString m_strLog;
	WORD	m_nType;
};

typedef CList< CLogMessage* > CLogMessageList;

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
	CString				m_strBinaryPath;			// Shareaza.exe path
	CFont				m_gdiFont;
	CFont				m_gdiFontBold;
	CFont				m_gdiFontLine;
	CWnd*				m_pSafeWnd;
	volatile LONG		m_bBusy;					// Shareaza is busy
	volatile bool		m_bInteractive;				// Shareaza begins initialization
	volatile bool		m_bLive;					// Shareaza fully initialized
	volatile bool		m_bClosing;					// Shareaza begins closing
	bool				m_bIsServer;				// Is OS a Server version
	bool				m_bIsWin2000;				// Is OS Windows 2000
	bool				m_bIsVistaOrNewer;			// Is OS Vista or newer
	bool				m_bLimitedConnections;		// Networking is limited (XP SP2)
	DWORD				m_nWindowsVersion;			// Windows version
	DWORD				m_nWindowsVersionMinor;		// Windows minor version
	QWORD				m_nPhysicalMemory;			// Physical RAM installed
	int					m_nLogicalProcessors;		// Multi-CPUs, multi-cores or HT modules
	BOOL				m_bMenuWasVisible;			// For the menus in media player window
	int					m_nDefaultFontSize;			// The basic font size. (11)
	CString				m_sDefaultFont;				// Main font. (Tahoma)
	CString				m_sPacketDumpFont;			// Packet Window. (Lucida Console)
	CString				m_sSystemLogFont;			// System Window. (Courier New)
	boost::scoped_ptr< CUPnPFinder > m_pUPnPFinder;
	TRISTATE			m_bUPnPPortsForwarded;		// UPnP values are assigned when the discovery is complete
	TRISTATE			m_bUPnPDeviceConnected;		// or when the service notifies
	DWORD				m_nUPnPExternalAddress;
	DWORD				m_nLastInput;				// Time of last input event (in secs)
	HHOOK				m_hHookKbd;
	HHOOK				m_hHookMouse;

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

	// Shell functions
	HINSTANCE			m_hShlWapi;
	BOOL		(WINAPI *m_pfnAssocIsDangerous)(LPCWSTR);

	// GeoIP - IP to Country lookup
	HINSTANCE			m_hGeoIP;
	GeoIP*				m_pGeoIP;
	typedef GeoIP* (*GeoIP_newFunc)(int);
	typedef const char * (*GeoIP_country_code_by_ipnumFunc) (GeoIP* gi, unsigned long ipnum);
	typedef const char * (*GeoIP_country_name_by_ipnumFunc) (GeoIP* gi, unsigned long ipnum);
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
	void				PrintMessage(WORD nType, const CString& strLog);
	void				LogMessage(const CString& strLog);

	void				SplashStep(LPCTSTR pszMessage = NULL, int nMax = 0, bool bClosing = false);

	CString				GetCountryCode(IN_ADDR pAddress) const;
	CString				GetCountryName(IN_ADDR pAddress) const;

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
	// Open .co, .collection or .emulecollection file
	static BOOL			OpenCollection(LPCTSTR lpszFileName, BOOL bDoIt);
	// Open url
	static BOOL			OpenURL(LPCTSTR lpszFileName, BOOL bDoIt, BOOL bSilent = FALSE);

protected:
	CSplashDlg*					m_dlgSplash;
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

// Post message to main window in safe way
BOOL PostMainWndMessage(UINT Msg, WPARAM wParam = NULL, LPARAM lParam = NULL);

CRuntimeClass* AfxClassForName(LPCTSTR pszClass);

BOOL LoadString(CString& str, UINT nID);
CString LoadString(UINT nID);
LPCTSTR _tcsistr(LPCTSTR pszString, LPCTSTR pszSubString);
LPCTSTR _tcsnistr(LPCTSTR pszString, LPCTSTR pszPattern, size_t plen);
void Split(const CString& strSource, TCHAR cDelimiter, CStringArray& pAddIt, BOOL bAddFirstEmpty = FALSE);
BOOL LoadSourcesString(CString& str, DWORD num, bool bFraction=false);

DWORD	TimeFromString(LPCTSTR psz);
CString	TimeToString(time_t tVal);
BOOL	TimeFromString(LPCTSTR psz, FILETIME* pTime);
CString	TimeToString(FILETIME* pTime);

void	RecalcDropWidth(CComboBox* pWnd);
// Load 16x16, 32x32, 48x48 icons from .ico, .exe, .dll files
BOOL LoadIcon(LPCTSTR szFilename, HICON* phSmallIcon, HICON* phLargeIcon, HICON* phHugeIcon);
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

CString GetFolderPath( int nFolder );
CString GetWindowsFolder();
CString GetProgramFilesFolder();
CString GetDocumentsFolder();
CString GetAppDataFolder();
CString GetLocalAppDataFolder();

// Create directory. If one or more of the intermediate folders do not exist, they are created as well.
BOOL CreateDirectory(LPCTSTR szPath);

// Delete file in many ways
BOOL DeleteFileEx(LPCTSTR szFileName, BOOL bShared, BOOL bToRecycleBin, BOOL bEnableDelayed);
void PurgeDeletes();

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

// Do message loop
void SafeMessageLoop();

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

// Use with whole numbers only
template <typename T>
inline T GetRandomNum(const T& min, const T& max)
{
	if ( theApp.m_hCryptProv != 0 )
	{
		T nRandom = 0;
		if ( CryptGenRandom( theApp.m_hCryptProv, sizeof( T ), (BYTE*)&nRandom ) )
			return min + (double)nRandom / ( (double)( static_cast< T >( -1 ) ) / ( max - min + 1 ) + 1 );
	}

	// Fallback to non-secure method
	return min + (double)rand() / ( (double)RAND_MAX / ( max - min + 1 ) + 1 );
}

template <>
inline __int8 GetRandomNum<__int8>(const __int8& min, const __int8& max)
{
	return (__int8)GetRandomNum<unsigned __int8>( min, max );
}

template <>
inline __int16 GetRandomNum<__int16>(const __int16& min, const __int16& max)
{
	return (__int16)GetRandomNum<unsigned __int16>( min, max );
}

template <>
inline __int32 GetRandomNum<__int32>(const __int32& min, const __int32& max)
{
	return (__int32)GetRandomNum<unsigned __int32>( min, max );
}

template <>
inline __int64 GetRandomNum<__int64>(const __int64& min, const __int64& max)
{
	return (__int64)GetRandomNum<unsigned __int64>( min, max );
}

// Log severity (log level)
#define MSG_SEVERITY_MASK		0x00ff
#define MSG_ERROR				0x0000
#define MSG_WARNING				0x0001
#define MSG_NOTICE				0x0002
#define MSG_INFO				0x0003
#define MSG_DEBUG				0x0004

// Log facility
#define MSG_FACILITY_MASK		0xff00
#define MSG_FACILITY_DEFAULT	0x0000
#define MSG_FACILITY_SEARCH		0x0100
#define MSG_FACILITY_INCOMING	0x0200
#define MSG_FACILITY_OUTGOING	0x0300

#define WM_WINSOCK			(WM_APP+101)	// Winsock messages proxy to Network object ( used by WSAAsyncGetHostByName() function )
#define WM_VERSIONCHECK		(WM_APP+102)	// Version check ( WAPARM: VERSION_CHECK nCode, LPARAM: unused )
#define WM_OPENCHAT			(WM_APP+103)	// Open chat window ( WAPARM: CChatSession* pChat, LPARAM: unused )
#define WM_TRAY				(WM_APP+104)	// Tray icon notification ( WPARAM: unused, LPARAM: uMouseMessage )
#define WM_URL				(WM_APP+105)	// Open URL ( WPARAM: CShareazaURL* pURL, LPARAM: unused )
#define WM_SKINCHANGED		(WM_APP+106)	// Skin change ( WPARAM: unused, LPARAM: unused )
#define WM_COLLECTION		(WM_APP+107)	// Open collection file ( WPARAM: unused, LPARAM: LPTSTR szFilename )
#define WM_OPENSEARCH		(WM_APP+108)	// Open new search ( WPARAM: CQuerySearch* pSearch, LPARAM: unused )
#define WM_LIBRARYSEARCH	(WM_APP+110)	// Start file library search ( WPARAM: LPTSTR pszSearch, LPARAM: unused )
#define WM_PLAYFILE			(WM_APP+111)	// Play file by media system ( WPARAM: unused, LPARAM: CString* pFilename )
#define WM_ENQUEUEFILE		(WM_APP+112)	// Enqueue file to media system ( WPARAM: unused, LPARAM: CString* pFilename )
#define WM_SETALPHA			(WM_APP+113)	// Increase/decrease main window transparency ( WPARAM: 0 - to decrease or 1 - to increase, LPARAM: unused )
#define WM_METADATA			(WM_APP+114)	// Set/clear library meatapanel data and status message ( WPARAM: CMetaPanel* pPanelData, LPARAM: LPCTSTR pszMessage )
#define WM_SANITY_CHECK		(WM_APP+115)	// Run allsystem check against banned hosts ( WPARAM: unused, LPARAM: unused )
#define WM_QUERYHITS		(WM_APP+116)	// Route query hits over windows ( WPARAM: unused, LPARAM: CQueryHit* pHits )
#define WM_NOWUPLOADING		(WM_APP+117)	// New upload notification ( WPARAM: unused, LPARAM: CString* pFilename )

#define ID_PLUGIN_FIRST	27000
#define ID_PLUGIN_LAST	27999


#define PANEL_WIDTH			200				// Left panel size in pixels (Home, Search, IRC tabs)
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
