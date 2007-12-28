//
// Shareaza.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Registry.h"
#include "CoolInterface.h"
#include "Network.h"
#include "Firewall.h"
#include "UPnPFinder.h"
#include "Security.h"
#include "HostCache.h"
#include "DiscoveryServices.h"
#include "VersionChecker.h"
#include "SchemaCache.h"
#include "VendorCache.h"
#include "EDClients.h"
#include "BTClients.h"
#include "Library.h"
#include "LibraryBuilderPlugins.h"
#include "Transfers.h"
#include "DownloadGroups.h"
#include "Downloads.h"
#include "Uploads.h"
#include "UploadQueues.h"
#include "QueryHashMaster.h"
#include "DDEServer.h"
#include "IEProtocol.h"
#include "ShareazaURL.h"
#include "GProfile.h"
#include "SharedFile.h"
#include "Emoticons.h"
#include "Flags.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "Scheduler.h"
#include "FileExecutor.h"
#include "ThumbCache.h"
#include "BTInfo.h"
#include "Plugins.h"

#include "WndMain.h"
#include "WndSystem.h"
#include "DlgSplash.h"
#include "DlgHelp.h"
#include "FontManager.h"

#ifndef WIN64
extern "C" HMODULE (__stdcall *_PfnLoadUnicows)(void) = &LoadUnicows;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const LPCTSTR RT_BMP = _T("BMP");
const LPCTSTR RT_JPEG = _T("JPEG");
const LPCTSTR RT_PNG = _T("PNG");
const LPCTSTR RT_GZIP = _T("GZIP");
double scaleX = 1;
double scaleY = 1;

#ifndef WIN64
HMODULE __stdcall LoadUnicows()
{
	HMODULE hUnicows = LoadLibraryA("unicows.dll");

	if ( !hUnicows )
	{
		// If the load is failed, then exit.
		MessageBoxA(NULL, "Unicode wrapper not found.", NULL, MB_ICONSTOP | MB_OK);
		_exit(-1);
	}

	return hUnicows;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CShareazaCommandLineInfo

CShareazaCommandLineInfo::CShareazaCommandLineInfo() :
	m_bTray( FALSE ),
	m_bNoSplash( FALSE ),
	m_bNoAlphaWarning( FALSE ),
	m_nGUIMode( -1 )
{
}

void CShareazaCommandLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
	if ( bFlag )
	{
		if ( ! lstrcmpi( pszParam, _T("tray") ) )
		{
			m_bTray = TRUE;
			m_bNoSplash = TRUE;
			return;
		}
		else if ( ! lstrcmpi( pszParam, _T("nosplash") ) )
		{
			m_bNoSplash = TRUE;
			return;
		}
		else if ( ! lstrcmpi( pszParam, _T("nowarn") ) )
		{
			m_bNoAlphaWarning = TRUE;
			return;
		}
		else if ( ! lstrcmpi( pszParam, _T("basic") ) )
		{
			m_nGUIMode = GUI_BASIC;
			return;
		}
		else if ( ! lstrcmpi( pszParam, _T("tabbed") ) )
		{
			m_nGUIMode = GUI_TABBED;
			return;
		}
		else if ( ! lstrcmpi( pszParam, _T("windowed") ) )
		{
			m_nGUIMode = GUI_WINDOWED;
			return;
		}
	}
	CCommandLineInfo::ParseParam( pszParam, bFlag, bLast );
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp

BEGIN_MESSAGE_MAP(CShareazaApp, CWinApp)
	//{{AFX_MSG_MAP(CShareazaApp)
	//}}AFX_MSG
END_MESSAGE_MAP()

const GUID CDECL BASED_CODE _tlid =
	{ 0xE3481FE3, 0xE062, 0x4E1C, { 0xA2, 0x3A, 0x62, 0xA6, 0xD1, 0x3C, 0xBF, 0xB8 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;

CShareazaApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp construction

CShareazaApp::CShareazaApp()
: m_pMutex( NULL )
, m_pSafeWnd( NULL )
, m_bLive( FALSE )
, m_bInteractive( FALSE )
, m_bNT( FALSE )
, m_bServer( FALSE )
, m_bWinME( FALSE )
, m_bLimitedConnections( FALSE )
, m_dwWindowsVersion( 0 )
, m_dwWindowsVersionMinor( 0 )
, m_nPhysicalMemory( 0 )
, m_bMenuWasVisible( FALSE )
, m_nDefaultFontSize( 0 )
, m_bUPnPPortsForwarded( TRI_UNKNOWN )
, m_bUPnPDeviceConnected( TRI_UNKNOWN )
, m_nUPnPExternalAddress( 0 )
, m_dwLastInput( 0 )
, m_hHookKbd( NULL )
, m_hHookMouse( NULL )
, m_hUser32( NULL )
, m_hKernel( NULL )
, m_hShellFolder( NULL )
, m_hGDI32( NULL )
, m_hTheme( NULL )
, m_hPowrProf( NULL )
, m_hShlWapi( NULL )
, m_hGeoIP( NULL )
, m_pGeoIP( NULL )
, m_pFontManager( NULL )
{
	ZeroMemory( m_nVersion, sizeof( m_nVersion ) );
	ZeroMemory( m_pBTVersion, sizeof( m_pBTVersion ) );
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp initialization

BOOL CShareazaApp::InitInstance()
{
	CWinApp::InitInstance();

	CWaitCursor pCursor;

	SetRegistryKey( _T("Shareaza") );
	GetVersionNumber();
	Settings.Load();			// Loads settings. Depends on GetVersionNumber()
	InitResources();			// Loads theApp settings. Depends on Settings::Load()
	CoolInterface.Load();		// Loads colors and fonts. Depends on InitResources()

	AfxOleInit();
	m_pFontManager = new CFontManager();
	AfxEnableControlContainer( m_pFontManager );

	LoadStdProfileSettings();
	EnableShellOpen();
//	RegisterShellFileTypes();

	ParseCommandLine( m_ocmdInfo );
	if ( m_ocmdInfo.m_nShellCommand == CCommandLineInfo::AppUnregister )
	{
		// Do not call this ->
		// ProcessShellCommand( m_ocmdInfo );
		// ... else all INI settings will be deleted (by design)

		// Do not call this -> 
		// AfxOleUnregisterTypeLib( _tlid, _wVerMajor, _wVerMinor );
		// COleTemplateServer::UnregisterAll();
		// COleObjectFactory::UpdateRegistryAll( FALSE );
		// ... else OLE interface settings may be deleted (bug in MFC?)
		return FALSE;
	}
	if ( m_ocmdInfo.m_nShellCommand == CCommandLineInfo::AppRegister )
	{
		ProcessShellCommand( m_ocmdInfo );
	}
	AfxOleRegisterTypeLib( AfxGetInstanceHandle(), _tlid );
	COleTemplateServer::RegisterAll();
	COleObjectFactory::UpdateRegistryAll( TRUE );
	if ( m_ocmdInfo.m_nShellCommand == CCommandLineInfo::AppRegister )
	{
		return FALSE;
	}

	m_pMutex = CreateMutex( NULL, FALSE,
		( m_dwWindowsVersion < 5 ) ? _T("Shareaza") : _T("Global\\Shareaza") );
	if ( m_pMutex != NULL )
	{
		if ( GetLastError() == ERROR_ALREADY_EXISTS )
		{
			CloseHandle( m_pMutex );
			m_pMutex = NULL;

			// Popup first instance
			if ( CWnd* pWnd = CWnd::FindWindow( _T("ShareazaMainWnd"), NULL ) )
			{
				pWnd->SendMessage( WM_SYSCOMMAND, SC_RESTORE );
				pWnd->ShowWindow( SW_SHOWNORMAL );
				pWnd->BringWindowToTop();
				pWnd->SetForegroundWindow();
			}
			else
			{
				// Probably window created in another user's session
			}
			return FALSE;
		}
		// We are first!
	}
	else
	{
		// Probably mutex created in another user's session
		return FALSE;
	}

	m_bInteractive = TRUE;

	DDEServer.Create();
	IEProtocol.Create();

	// Set Build Date
	COleDateTime tCompileTime; 
	tCompileTime.ParseDateTime( _T(__DATE__), LOCALE_NOUSEROVERRIDE, 1033 );
	m_sBuildDate = tCompileTime.Format( _T("%Y%m%d") );

	// ***********
	//*
	// Beta expiry. Remember to re-compile to update the time, and remove this 
	// section for final releases and public betas.
	COleDateTime tCurrent = COleDateTime::GetCurrentTime();
	//COleDateTimeSpan tTimeOut( 31 * 2, 0, 0, 0);	// Betas that aren't on sourceforge
	COleDateTimeSpan tTimeOut( 7, 0, 0, 0);			// Daily builds
	if ( ( tCompileTime + tTimeOut )  < tCurrent )
	{
		CString strMessage;
		LoadString( strMessage, IDS_BETA_EXPIRED);
		AfxMessageBox( strMessage, MB_SYSTEMMODAL|MB_ICONQUESTION|MB_OK );
		//return FALSE;
	}
	//*/

	//*
	// Alpha warning. Remember to remove this section for final releases and public betas.
	if ( ! m_ocmdInfo.m_bNoAlphaWarning )
	if ( AfxMessageBox( 
		L"WARNING: This is an ALPHA TEST version of Shareaza.\n\n"
		L"It is NOT FOR GENERAL USE, and is only for testing specific features in a controlled "
		L"environment. It will frequently stop running, or display debug information to assist testing.\n\n"
		L"If you wish to actually use this software, you should download "
		L"the current stable release from http://shareaza.sourceforge.net/\n"
		L"If you continue past this point, you may experience system instability, lose downloads, "
		L"or corrupt system files. Corrupted downloads/files may not be recoverable. "
		L"Do you wish to continue?", MB_SYSTEMMODAL|MB_ICONEXCLAMATION|MB_YESNO ) == IDNO )
		return FALSE;
	//*/
	// ***********

	CSplashDlg* dlgSplash = m_ocmdInfo.m_bNoSplash ? NULL : new CSplashDlg( 17 +
		( ( Settings.Connection.EnableFirewallException ) ? 1 : 0 ) +
		( ( Settings.Connection.EnableUPnP && !Settings.Live.FirstRun ) ? 1 : 0 ),
		false );

	SplashStep( dlgSplash, L"Winsock" );
		WSADATA wsaData;
		for ( int i = 1; i <= 2; i++ )
		{
			if ( WSAStartup( MAKEWORD( 1, 1 ), &wsaData ) ) return FALSE;
			if ( wsaData.wVersion == MAKEWORD( 1, 1 ) ) break;
			if ( i == 2 ) return FALSE;
			WSACleanup();
		}

	if ( m_ocmdInfo.m_nGUIMode != -1 )
		Settings.General.GUIMode = m_ocmdInfo.m_nGUIMode;

	if ( Settings.General.GUIMode != GUI_WINDOWED && Settings.General.GUIMode != GUI_TABBED && Settings.General.GUIMode != GUI_BASIC )
		Settings.General.GUIMode = GUI_BASIC;

	SplashStep( dlgSplash, L"P2P URIs" );
		CShareazaURL::Register( TRUE );
	SplashStep( dlgSplash, L"Shell Icons" );
		ShellIcons.Clear();
	SplashStep( dlgSplash, L"Metadata Schemas" );
		SchemaCache.Load();
	SplashStep( dlgSplash, L"Vendor Data" );
		VendorCache.Load();
	SplashStep( dlgSplash, L"Profile" );
		MyProfile.Load();
	SplashStep( dlgSplash, L"Query Manager" );
		QueryHashMaster.Create();
	SplashStep( dlgSplash, L"Host Cache" );
		HostCache.Load();
	SplashStep( dlgSplash, L"Discovery Services" );
		DiscoveryServices.Load();
	SplashStep( dlgSplash, L"Security Services" );
		Security.Load();
		AdultFilter.Load();
		MessageFilter.Load();
	SplashStep( dlgSplash, L"Scheduler" );
		Schedule.Load();
	SplashStep( dlgSplash, L"Rich Documents" );
		Emoticons.Load();
		Flags.Load();

	if ( Settings.Connection.EnableFirewallException )
	{
		SplashStep( dlgSplash, L"Windows Firewall Setup" );
		CFirewall firewall;
		if ( firewall.AccessWindowsFirewall() && firewall.AreExceptionsAllowed() )
		{
			// Add to firewall exception list if necessary
			// and enable UPnP Framework if disabled
			CString strBinaryPath;
			GetModuleFileName( NULL, strBinaryPath.GetBuffer( MAX_PATH ), MAX_PATH );
			strBinaryPath.ReleaseBuffer( MAX_PATH );
			firewall.SetupService( NET_FW_SERVICE_UPNP );
			firewall.SetupProgram( strBinaryPath, theApp.m_pszAppName );
		}
	}

	// If it is the first run we will run the UPnP discovery only in the QuickStart Wizard
	if ( Settings.Connection.EnableUPnP && ! Settings.Live.FirstRun )
	{
		SplashStep( dlgSplash, L"Firewall/Router Setup" );
		try
		{
			m_pUPnPFinder.reset( new CUPnPFinder );
			if ( m_pUPnPFinder->AreServicesHealthy() )
				m_pUPnPFinder->StartDiscovery();
		}
		catch ( CUPnPFinder::UPnPError& ) {}
		catch ( CException* e ) { e->Delete(); }
	}

	SplashStep( dlgSplash, L"GUI" );
		if ( m_ocmdInfo.m_bTray ) WriteProfileInt( _T("Windows"), _T("CMainWnd.ShowCmd"), 0 );
		new CMainWnd();
		CoolMenu.EnableHook();
		if ( m_ocmdInfo.m_bTray )
		{
			((CMainWnd*)m_pMainWnd)->CloseToTray();
		}
		else
		{
			m_pMainWnd->ShowWindow( SW_SHOW );
			if ( dlgSplash ) 
				dlgSplash->Topmost();
			m_pMainWnd->UpdateWindow();
		}

	// From this point translations are available and LoadString returns correct strings
	SplashStep( dlgSplash, L"Download Manager" ); 
		Downloads.Load();
	SplashStep( dlgSplash, L"Upload Manager" );
		UploadQueues.Load();
	SplashStep( dlgSplash, L"Library" );
		Library.Load();
	SplashStep( dlgSplash, L"Upgrade Manager" );
		VersionChecker.Start(); 

	pCursor.Restore();

	if ( dlgSplash )
		dlgSplash->Hide();
	m_bLive = TRUE;

	ProcessShellCommand( m_ocmdInfo );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp termination

int CShareazaApp::ExitInstance() 
{
	if ( m_bInteractive )
	{
		CWaitCursor pCursor;
		
		CSplashDlg* dlgSplash = new CSplashDlg( 6 +
			( ( Settings.Connection.DeleteFirewallException ) ? 1 : 0 ) +
			( ( m_pUPnPFinder ) ? 1 : 0 ) +
			( ( m_bLive ) ? 1 : 0 ),
			true );
		SplashStep( dlgSplash, L"Closing Server Processes" );
		DDEServer.Close();
		IEProtocol.Close();
		SplashStep( dlgSplash, L"Disconnecting" );
		VersionChecker.Stop();
		DiscoveryServices.Stop();
		Network.Disconnect();
		SplashStep( dlgSplash, L"Stopping Library Tasks" );
		Library.StopThread();
		SplashStep( dlgSplash, L"Stopping Transfers" );	
		Transfers.StopThread();
		Downloads.CloseTransfers();
		SplashStep( dlgSplash, L"Clearing Clients" );	
		Uploads.Clear( FALSE );
		EDClients.Clear();

		if ( Settings.Connection.DeleteFirewallException )
		{
			SplashStep( dlgSplash, L"Closing Windows Firewall Access" );	
			CFirewall firewall;
			if ( firewall.AccessWindowsFirewall() )
			{
				// Remove application from the firewall exception list
				CString strBinaryPath;
				GetModuleFileName( NULL, strBinaryPath.GetBuffer( MAX_PATH ), MAX_PATH );
				strBinaryPath.ReleaseBuffer( MAX_PATH );
				firewall.SetupProgram( strBinaryPath, theApp.m_pszAppName, TRUE );
			}
		}

		if ( m_pUPnPFinder )
		{
			SplashStep( dlgSplash, L"Closing Firewall/Router Access" );
			m_pUPnPFinder->StopAsyncFind();
			if ( Settings.Connection.DeleteUPnPPorts )
				m_pUPnPFinder->DeletePorts();
			m_pUPnPFinder.reset();
		}

		if ( m_bLive )
		{
			SplashStep( dlgSplash, L"Saving" );
			Downloads.Save();
			DownloadGroups.Save();
			Library.Save();
			Security.Save();
			HostCache.Save();
			UploadQueues.Save();
			DiscoveryServices.Save();
			Settings.Save( TRUE );
		}
		SplashStep( dlgSplash, L"Finalizing" );
		Downloads.Clear( TRUE );
		BTClients.Clear();
		Library.Clear();
		Skin.Clear();

		if ( dlgSplash )
			dlgSplash->Hide();

		CLibraryBuilderPlugins::Cleanup();
		Plugins.Clear();
	}

	if ( m_hUser32 != NULL ) FreeLibrary( m_hUser32 );

	WSACleanup();

	if ( m_hShellFolder != NULL ) FreeLibrary( m_hShellFolder );

	if ( m_hGDI32 != NULL ) FreeLibrary( m_hGDI32 );

	if ( m_hTheme != NULL ) FreeLibrary( m_hTheme );

	if ( m_hPowrProf != NULL ) FreeLibrary( m_hPowrProf );

	if ( m_hShlWapi != NULL ) FreeLibrary( m_hShlWapi );

	if ( m_hGeoIP != NULL ) FreeLibrary( m_hGeoIP );

	if ( m_hLibGFL != NULL ) FreeLibrary( m_hLibGFL );

	delete m_pFontManager;

	UnhookWindowsHookEx( m_hHookKbd );
	UnhookWindowsHookEx( m_hHookMouse );

	if ( m_pMutex != NULL ) CloseHandle( m_pMutex );

	return CWinApp::ExitInstance();
}

void CShareazaApp::WinHelp(DWORD /*dwData*/, UINT /*nCmd*/) 
{
	// Suppress F1
}

CDocument* CShareazaApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	if ( lpszFileName )
		Open( lpszFileName, TRUE );
	return NULL;
}

BOOL CShareazaApp::Open(LPCTSTR lpszFileName, BOOL bDoIt)
{
	int nLength = lstrlen( lpszFileName );
	if ( nLength > 8 && ! lstrcmpi( lpszFileName + nLength - 8, _T(".torrent") ) )
		return OpenTorrent( lpszFileName, bDoIt );
	else if ( nLength > 3 && ! lstrcmpi( lpszFileName + nLength - 3, _T(".co") ) )
		return OpenCollection( lpszFileName, bDoIt );
	else if ( nLength > 11 && ! lstrcmpi( lpszFileName + nLength - 11, _T(".collection") ) )
		return OpenCollection( lpszFileName, bDoIt );
	else if ( nLength > 4 && ! lstrcmpi( lpszFileName + nLength - 4, _T(".url") ) )
		return OpenInternetShortcut( lpszFileName, bDoIt );
	else if ( nLength > 4 && ! lstrcmpi( lpszFileName + nLength - 4, _T(".lnk") ) )
		return OpenShellShortcut( lpszFileName, bDoIt );
	else
		return OpenURL( lpszFileName, bDoIt );
}

BOOL CShareazaApp::OpenShellShortcut(LPCTSTR lpszFileName, BOOL bDoIt)
{
	CString sPath( ResolveShortcut( lpszFileName ) );
	return sPath.GetLength() && Open( sPath, bDoIt );
}

BOOL CShareazaApp::OpenInternetShortcut(LPCTSTR lpszFileName, BOOL bDoIt)
{
	CString sURL;
	BOOL bResult = ( GetPrivateProfileString( _T("InternetShortcut"), _T("URL"),
		_T(""), sURL.GetBuffer( MAX_PATH ), MAX_PATH, lpszFileName ) > 3 );
	sURL.ReleaseBuffer();
	return bResult && sURL.GetLength() && OpenURL( sURL, bDoIt );
}

BOOL CShareazaApp::OpenTorrent(LPCTSTR lpszFileName, BOOL bDoIt)
{
	if ( bDoIt )
		theApp.Message( MSG_SYSTEM, IDS_BT_PREFETCH_FILE, lpszFileName );

	BOOL bResult = FALSE;
	CBTInfo* pTorrent = new CBTInfo();
	if ( pTorrent )
	{
		if ( pTorrent->LoadTorrentFile( lpszFileName ) )
		{
			if ( bDoIt && pTorrent->HasEncodingError() )
				theApp.Message( MSG_SYSTEM, IDS_BT_ENCODING );
			CShareazaURL* pURL = new CShareazaURL( pTorrent );
			if ( pURL )
			{
				bResult = TRUE;
				if ( bDoIt )
					return AfxGetMainWnd()->PostMessage( WM_URL, (WPARAM)pURL );
				delete pURL;
				pTorrent = NULL;	// Deleted inside CShareazaURL::Clear()
			}
		}
		delete pTorrent;
	}

	if ( bDoIt )
		theApp.Message( MSG_DISPLAYED_ERROR, IDS_BT_PREFETCH_ERROR, lpszFileName );

	return bResult;
}

BOOL CShareazaApp::OpenCollection(LPCTSTR lpszFileName, BOOL bDoIt)
{
	if ( ! bDoIt )
		return TRUE;

	LPTSTR pszPath = new TCHAR[ lstrlen( lpszFileName ) + 1 ];
	if ( pszPath )
	{
		lstrcpy( pszPath, lpszFileName );
		if ( AfxGetMainWnd()->PostMessage( WM_COLLECTION, (WPARAM)pszPath ) )
			return TRUE;
		delete [] pszPath;
	}

	return FALSE;
}

BOOL CShareazaApp::OpenURL(LPCTSTR lpszFileName, BOOL bDoIt, BOOL bSilent)
{
	if ( bDoIt && ! bSilent )
		theApp.Message( MSG_SYSTEM, IDS_URL_RECEIVED, lpszFileName );

	CShareazaURL* pURL = new CShareazaURL();
	if ( pURL )
	{
		if ( pURL->Parse( lpszFileName ) )
		{
			if ( bDoIt )
				AfxGetMainWnd()->PostMessage( WM_URL, (WPARAM)pURL );
			return TRUE;
		}
		delete pURL;
	}

	if ( bDoIt && ! bSilent )
		theApp.Message( MSG_SYSTEM, IDS_URL_PARSE_ERROR );

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp version

void CShareazaApp::GetVersionNumber()
{
	TCHAR szPath[MAX_PATH];
	DWORD dwSize;

	m_nVersion[0] = m_nVersion[1] = m_nVersion[2] = m_nVersion[3] = 0;

	GetModuleFileName( NULL, szPath, MAX_PATH );
	dwSize = GetFileVersionInfoSize( szPath, &dwSize );

	if ( dwSize )
	{
		BYTE* pBuffer = new BYTE[ dwSize ];

		if ( pBuffer )
		{
			if ( GetFileVersionInfo( szPath, NULL, dwSize, pBuffer ) )
			{
				VS_FIXEDFILEINFO* pTable;

				if ( VerQueryValue( pBuffer, _T("\\"), (VOID**)&pTable, (UINT*)&dwSize ) )
				{
					m_nVersion[0] = (WORD)( pTable->dwFileVersionMS >> 16 );
					m_nVersion[1] = (WORD)( pTable->dwFileVersionMS & 0xFFFF );
					m_nVersion[2] = (WORD)( pTable->dwFileVersionLS >> 16 );
					m_nVersion[3] = (WORD)( pTable->dwFileVersionLS & 0xFFFF );
				}
			}

			delete [] pBuffer;
		}
	}

	m_sVersion.Format( _T("%i.%i.%i.%i"),
		m_nVersion[0], m_nVersion[1],
		m_nVersion[2], m_nVersion[3] );

	m_sSmartAgent = _T( CLIENT_NAME );
	m_sSmartAgent += _T(" ");
	m_sSmartAgent += m_sVersion;

	m_pBTVersion[ 0 ] = BT_ID1;
	m_pBTVersion[ 1 ] = BT_ID2;
	m_pBTVersion[ 2 ] = (BYTE)m_nVersion[ 0 ];
	m_pBTVersion[ 3 ] = (BYTE)m_nVersion[ 1 ];
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp resources

void CShareazaApp::InitResources()
{
	//Determine the version of Windows
	OSVERSIONINFOEX pVersion;
	pVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx( (OSVERSIONINFO*)&pVersion );
	
	//Networking is poor under Win9x based operating systems. (95/98/Me)
	m_bNT = ( pVersion.dwPlatformId == VER_PLATFORM_WIN32_NT );

	// Determine if it's a server
	m_bServer = m_bNT && pVersion.wProductType != VER_NT_WORKSTATION;

	//Win 95/98/Me/NT (<5) do not support some functions
	m_dwWindowsVersion = pVersion.dwMajorVersion;

	//Win2000 = 0 WinXP = 1
	m_dwWindowsVersionMinor = pVersion.dwMinorVersion; 

	// Detect Windows ME
	m_bWinME = ( m_dwWindowsVersion == 4 && m_dwWindowsVersionMinor == 90 );

	m_bLimitedConnections = FALSE;
	VER_PLATFORM_WIN32s;
	VER_PLATFORM_WIN32_WINDOWS;
	VER_PLATFORM_WIN32_NT;

	if ( m_dwWindowsVersion == 5 && m_dwWindowsVersionMinor == 1 )
	{	//Windows XP - Test for SP2
		TCHAR* sp = _tcsstr( pVersion.szCSDVersion, _T("Service Pack ") );
		if( sp && sp[ 13 ] >= '2' )
		{	//XP SP2 - Limit the networking.
			//AfxMessageBox(_T("Warning - Windows XP Service Pack 2 detected. Performance may be reduced."), MB_OK );
			m_bLimitedConnections = TRUE;
		}
	}
	else if ( m_dwWindowsVersion == 5 && m_dwWindowsVersionMinor == 2
		&& _tcsstr( pVersion.szCSDVersion, _T("Service Pack") ) )
	{
		// Windows 2003 or Win XP x64
		m_bLimitedConnections = TRUE;
	}
	else if ( m_dwWindowsVersion >= 6 )
	{
		// Windows Vista or higher
		m_bLimitedConnections = TRUE;
	}

	//Get pointers to some functions that don't exist under 95/NT
	if ( ( m_hUser32 = LoadLibrary( _T("User32.dll") ) ) != NULL )
	{
		(FARPROC&)m_pfnSetLayeredWindowAttributes = GetProcAddress(
			m_hUser32, "SetLayeredWindowAttributes" );

		(FARPROC&)m_pfnGetMonitorInfoA = GetProcAddress( 
			m_hUser32, "GetMonitorInfoA" ); 

		(FARPROC&)m_pfnMonitorFromRect = GetProcAddress( 
			m_hUser32, "MonitorFromRect" ); 

		(FARPROC&)m_pfnMonitorFromWindow = GetProcAddress( 
			m_hUser32, "MonitorFromWindow" ); 

		(FARPROC&)m_pfnGetAncestor = GetProcAddress( 
			m_hUser32, "GetAncestor" ); 

		(FARPROC&)m_pfnPrivateExtractIconsW = GetProcAddress( 
			m_hUser32, "PrivateExtractIconsW" ); 
	}
	else
	{
		m_pfnSetLayeredWindowAttributes = NULL;
		m_pfnGetMonitorInfoA = NULL;
		m_pfnMonitorFromRect = NULL;
		m_pfnMonitorFromWindow = NULL;
		m_pfnGetAncestor = NULL;
		m_pfnPrivateExtractIconsW = NULL;
	}

	// It is not necessary to call LoadLibrary on Kernel32.dll, because it is already loaded into every process address space.
	m_hKernel = GetModuleHandle( _T("kernel32.dll") );

	//Get the amount of installed memory.
	m_nPhysicalMemory = 0;
	if ( m_hKernel != NULL )
	{	//Use GlobalMemoryStatusEx if possible (WinXP)
		BOOL (WINAPI *m_pfnGlobalMemoryStatusEx)( LPMEMORYSTATUSEX );
		MEMORYSTATUSEX pMemory;

		pMemory.dwLength = sizeof(pMemory);
		(FARPROC&)m_pfnGlobalMemoryStatusEx = GetProcAddress( m_hKernel, "GlobalMemoryStatusEx" );

		if ( m_pfnGlobalMemoryStatusEx && (*m_pfnGlobalMemoryStatusEx)( &pMemory ) )
			m_nPhysicalMemory = pMemory.ullTotalPhys;
	}

	if ( m_nPhysicalMemory == 0 )
	{	//Fall back to GlobalMemoryStatus (always available)
		MEMORYSTATUS pMemory;
		GlobalMemoryStatus( &pMemory ); 
		m_nPhysicalMemory = pMemory.dwTotalPhys;
	}

	if ( m_hKernel != NULL )
		(FARPROC&)m_pfnGetDiskFreeSpaceExW = GetProcAddress( m_hKernel, "GetDiskFreeSpaceExW" );
	else
		m_pfnGetDiskFreeSpaceExW = NULL;

	if ( ( m_hShellFolder = LoadLibrary( _T("shfolder.dll") ) ) != NULL )
		(FARPROC&)m_pfnSHGetFolderPathW = GetProcAddress( m_hShellFolder, "SHGetFolderPathW" );
	else
		m_pfnSHGetFolderPathW = NULL;

	if ( ( m_hGDI32 = LoadLibrary( _T("gdi32.dll") ) ) != NULL )
		(FARPROC&)m_pfnSetLayout = GetProcAddress( m_hGDI32, "SetLayout" );
	else
		m_pfnSetLayout = NULL;

	if ( ( m_hTheme = LoadLibrary( _T("UxTheme.dll") ) ) != NULL )
		(FARPROC&)m_pfnSetWindowTheme = GetProcAddress( m_hTheme, "SetWindowTheme" );
	else
		m_pfnSetWindowTheme = NULL;

	if ( ( m_hPowrProf = LoadLibrary( _T("PowrProf.dll") ) ) != NULL )
	{
		(FARPROC&)m_pfnGetActivePwrScheme = GetProcAddress( m_hPowrProf, "GetActivePwrScheme" );
		(FARPROC&)m_pfnGetCurrentPowerPolicies = GetProcAddress( m_hPowrProf, "GetCurrentPowerPolicies" );
		(FARPROC&)m_pfnSetActivePwrScheme = GetProcAddress( m_hPowrProf, "SetActivePwrScheme" );
	}
	else
	{
		m_pfnGetActivePwrScheme = NULL;
		m_pfnGetCurrentPowerPolicies = NULL;
		m_pfnSetActivePwrScheme = NULL;
	}

	if ( ( m_hShlWapi = LoadLibrary( _T("shlwapi.dll") ) ) != NULL )
	{
		(FARPROC&)m_pfnAssocIsDangerous = GetProcAddress( m_hShlWapi, "AssocIsDangerous" );
	}
	else
	{
		m_pfnAssocIsDangerous = NULL;
	}

	// Load the GeoIP library for mapping IPs to countries
	m_hGeoIP = CustomLoadLibrary( _T("geoip.dll") );
	if ( m_hGeoIP )
	{
		GeoIP_newFunc pfnGeoIP_new = (GeoIP_newFunc)GetProcAddress( m_hGeoIP, "GeoIP_new" );
		m_pfnGeoIP_country_code_by_addr = (GeoIP_country_code_by_addrFunc)GetProcAddress( m_hGeoIP, "GeoIP_country_code_by_addr" );
		m_pfnGeoIP_country_name_by_addr = (GeoIP_country_name_by_addrFunc)GetProcAddress( m_hGeoIP, "GeoIP_country_name_by_addr" );

		m_pGeoIP = pfnGeoIP_new( GEOIP_MEMORY_CACHE );
	}
	else
	{
		m_pfnGeoIP_country_code_by_addr = NULL;
		m_pfnGeoIP_country_name_by_addr = NULL;
	}

	// We load it in a custom way, so Shareaza plugins can use this library also when it isn't in its search path but loaded by CustomLoadLibrary (very useful when running Shareaza inside Visual Studio)
	m_hLibGFL = CustomLoadLibrary( _T("libgfl267.dll") );

	HDC screen = GetDC( 0 );
	scaleX = GetDeviceCaps( screen, LOGPIXELSX ) / 96.0;
	scaleY = GetDeviceCaps( screen, LOGPIXELSY ) / 96.0;
	ReleaseDC( 0, screen );

	// Get the fonts from the registry
	CString strFont = ( m_dwWindowsVersion >= 6 ) ?
					  L"Segoe UI" : L"Tahoma" ;
	theApp.m_sDefaultFont		= theApp.GetProfileString( _T("Fonts"), _T("DefaultFont"), strFont );
	theApp.m_sPacketDumpFont	= theApp.GetProfileString( _T("Fonts"), _T("PacketDumpFont"), _T("Lucida Console") );
	theApp.m_sSystemLogFont		= theApp.GetProfileString( _T("Fonts"), _T("SystemLogFont"), strFont );
	theApp.m_nDefaultFontSize	= theApp.GetProfileInt( _T("Fonts"), _T("FontSize"), 11 );
	
	// Set up the default font
	m_gdiFont.CreateFontW( -theApp.m_nDefaultFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, theApp.m_sDefaultFont );
	
	m_gdiFontBold.CreateFontW( -theApp.m_nDefaultFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, theApp.m_sDefaultFont );
	
	m_gdiFontLine.CreateFontW( -theApp.m_nDefaultFontSize, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, theApp.m_sDefaultFont );

	srand( GetTickCount() );

	m_hHookKbd   = SetWindowsHookEx( WH_KEYBOARD, (HOOKPROC)KbdHook, NULL, AfxGetThread()->m_nThreadID );
	m_hHookMouse = SetWindowsHookEx( WH_MOUSE, (HOOKPROC)MouseHook, NULL, AfxGetThread()->m_nThreadID );
	m_dwLastInput = (DWORD)time( NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp custom library loader

HINSTANCE CShareazaApp::CustomLoadLibrary(LPCTSTR pszFileName)
{
	HINSTANCE hLibrary = NULL;

	if ( ( hLibrary = LoadLibrary( pszFileName ) ) != NULL || ( hLibrary = LoadLibrary( Settings.General.Path + _T("\\") + pszFileName ) ) != NULL );
	else
		TRACE( _T("DLL not found: %s\r\n"), pszFileName );

	return hLibrary;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp safe main window

CMainWnd* CShareazaApp::SafeMainWnd() const
{
	CMainWnd* pMainWnd = (CMainWnd*)theApp.m_pSafeWnd;
	if ( pMainWnd == NULL ) return NULL;
	ASSERT_KINDOF( CMainWnd, pMainWnd );
	return IsWindow( pMainWnd->m_hWnd ) ? pMainWnd : NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp message

void CShareazaApp::Message(int nType, UINT nID, ...) const
{
	// Return early if it's a debug message and debug isn't turned on
	if ( nType == MSG_DEBUG && ! Settings.General.Debug ) return;

	// Return early if it's a temp message and debug logging isn't turned on
#ifdef NDEBUG
	if ( nType == MSG_TEMP ) return;
#endif
	if ( nType == MSG_TEMP && ! Settings.General.DebugLog ) return;

	// Setup local strings
	CString strFormat, strTemp;

	// Load the format string from the resource file
	LoadString( strFormat, nID );

	// Initialise variable arguments list
	va_list pArgs;
	va_start( pArgs, nID );

	// Work out the type of format string and call the appropriate function
	if ( strFormat.Find( _T("%1") ) >= 0 )
		strTemp.FormatMessageV( strFormat, &pArgs );
	else
		strTemp.FormatV( strFormat, pArgs );

	// Print the message if there still is one
	if ( !strTemp.IsEmpty() )
		PrintMessage( nType, strTemp );

	// Null the argument list pointer
	va_end( pArgs );

	return;
}

void CShareazaApp::Message(int nType, CString strFormat, ...) const
{
	// Return early if it's a debug message and debug isn't turned on
	if ( nType == MSG_DEBUG && ! Settings.General.Debug ) return;

	// Return early if it's a temp message and debug logging isn't turned on
#ifdef NDEBUG
	if ( nType == MSG_TEMP ) return;
#endif
	if ( nType == MSG_TEMP && ! Settings.General.DebugLog ) return;

	// Setup local strings
	CString strTemp;

	// Initialise variable arguments list
	va_list pArgs;
	va_start( pArgs, strFormat );

	// Format the message
	strTemp.FormatV( strFormat, pArgs );

	// Print the message if there still is one
	if ( !strTemp.IsEmpty() )
		PrintMessage( nType, strTemp );

	// Null the argument list pointer
	va_end( pArgs );

	return;
}

void CShareazaApp::PrintMessage(int nType, CString& strLog) const
{
	// Check if there is a valid pointer to the main window
	// and we are not shutting down
	if ( m_pMainWnd && IsWindow( m_pMainWnd->m_hWnd )
		&& !static_cast< CMainWnd* >( m_pMainWnd )->m_pWindows.m_bClosing )
	{
		// Allocate a new character array on the heap (including null terminator)
		LPTSTR pszLog = new TCHAR[ strLog.GetLength() + 1 ];	// Released by CMainWnd::OnLog()

		if ( pszLog )
		{
			// Make a copy of the log message into the heap array
			_tcscpy( pszLog, strLog );

			// Send to the message pump for processing
			m_pMainWnd->PostMessage( WM_LOG, nType, (LPARAM)pszLog );
		}
	}
	else if ( Settings.General.DebugLog )
	{
		// We are shutting down and logging to file
		LogMessage( _T("ShutDown: ") + strLog );
	}

	return;
}

void CShareazaApp::LogMessage(LPCTSTR pszLog) const
{
	CQuickLock pLock( m_csMessage );

	CFile pFile;
	if ( pFile.Open( Settings.General.UserPath + _T("\\Data\\Shareaza.log"), CFile::modeReadWrite ) )
	{
		if ( ( Settings.General.MaxDebugLogSize ) &&					// If log rotation is on 
			( pFile.GetLength() > Settings.General.MaxDebugLogSize ) )	// and file is too long...
		{	
			// Close the file
			pFile.Close();				
			// Rotate the logs 
			DeleteFile( Settings.General.UserPath + _T("\\Data\\Shareaza.old.log") );
			MoveFile( Settings.General.UserPath + _T("\\Data\\Shareaza.log"), 
				Settings.General.UserPath + _T("\\Data\\Shareaza.old.log") );
			// Start a new log
			if ( ! pFile.Open( Settings.General.UserPath + _T("\\Data\\Shareaza.log"), 
				CFile::modeWrite|CFile::modeCreate ) ) return;
			// Unicode marker
			WORD nByteOrder = 0xFEFF;
			pFile.Write( &nByteOrder, 2 );
		}
		else
		{
			pFile.Seek( 0, CFile::end ); // Otherwise, go to the end of the file to add entires.
		}
	}
	else
	{
		if ( ! pFile.Open( Settings.General.UserPath + _T("\\Data\\Shareaza.log"), 
			CFile::modeWrite|CFile::modeCreate ) ) return;
		// Unicode marker
		WORD nByteOrder = 0xFEFF;
		pFile.Write( &nByteOrder, 2 );
	}

	if ( Settings.General.ShowTimestamp )
	{
		CTime pNow = CTime::GetCurrentTime();
		CString strLine;	
		strLine.Format( _T("[%.2i:%.2i:%.2i] "),
			pNow.GetHour(), pNow.GetMinute(), pNow.GetSecond() );
		pFile.Write( (LPCTSTR)strLine, sizeof(TCHAR) * strLine.GetLength() );
	}

	pFile.Write( pszLog, static_cast< UINT >( sizeof(TCHAR) * _tcslen(pszLog) ) );
	pFile.Write( _T("\r\n"), sizeof(TCHAR) * 2 );

	pFile.Close();
}

CString GetErrorString(DWORD dwError)
{
	LPTSTR MessageBuffer = NULL;
	CString strMessage;
	if ( FormatMessage (
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, dwError, 0, (LPTSTR)&MessageBuffer, 0, NULL ) )
	{
		strMessage = MessageBuffer;
		strMessage.Trim( _T(" \t\r\n") );
		LocalFree( MessageBuffer );
		return strMessage;
	}

	static LPCTSTR const szModules [] =
	{
		_T("netapi32.dll"),
		_T("netmsg.dll"),
		_T("wininet.dll"),
		_T("ntdll.dll"),
		_T("ntdsbmsg.dll"),
		NULL
	};
	for ( int i = 0; szModules[ i ]; i++ )
	{
		HMODULE hModule = LoadLibraryEx( szModules[ i ], NULL, LOAD_LIBRARY_AS_DATAFILE );
		if ( hModule )
		{
			DWORD bResult = FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_IGNORE_INSERTS |
				FORMAT_MESSAGE_FROM_HMODULE,
				hModule, dwError, 0, (LPTSTR)&MessageBuffer, 0, NULL );
			FreeLibrary( hModule );
			if ( bResult )
			{
				strMessage = MessageBuffer;
				strMessage.Trim( _T(" \t\r\n") );
				LocalFree( MessageBuffer );
				return strMessage;
			}
		}
	}	
	return CString();
}

CString CShareazaApp::GetCountryCode(IN_ADDR pAddress) const
{
	if ( m_pfnGeoIP_country_code_by_addr && m_pGeoIP )
		return CString( m_pfnGeoIP_country_code_by_addr( m_pGeoIP, inet_ntoa( pAddress ) ) );
	return _T("");
}

CString CShareazaApp::GetCountryName(IN_ADDR pAddress) const
{
	if ( m_pfnGeoIP_country_name_by_addr && m_pGeoIP )
		return CString( m_pfnGeoIP_country_name_by_addr( m_pGeoIP, inet_ntoa( pAddress ) ) );
	return _T("");
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp process an internal URI

BOOL CShareazaApp::InternalURI(LPCTSTR pszURI)
{
	CMainWnd* pMainWnd = SafeMainWnd();
	if ( pMainWnd == NULL ) return FALSE;

	CString strURI( pszURI );
	
	if ( strURI.Find( _T("raza:command:") ) == 0 )
	{
		if ( UINT nCmdID = CoolInterface.NameToID( pszURI + 13 ) )
		{
			pMainWnd->PostMessage( WM_COMMAND, nCmdID );
		}
	}
	else if ( strURI.Find( _T("raza:windowptr:") ) == 0 )
	{
		CChildWnd* pChild = NULL;
		_stscanf( (LPCTSTR)strURI + 15, _T("%lu"), &pChild );
		if ( pMainWnd->m_pWindows.Check( pChild ) ) pChild->MDIActivate();
	}
	else if ( strURI.Find( _T("raza:launch:") ) == 0 )
	{
		DWORD nIndex = 0;
		_stscanf( (LPCTSTR)strURI + 12, _T("%lu"), &nIndex );

		CSingleLock oLock( &Library.m_pSection, TRUE );
		if ( CLibraryFile* pFile = Library.LookupFile( nIndex ) )
		{
			if ( pFile->m_pFolder )
			{
				CString strPath = pFile->GetPath();
				oLock.Unlock();
				CFileExecutor::Execute( strPath, FALSE );
			}
		}
	}
	else if (	strURI.Find( _T("http://") ) == 0 ||
				strURI.Find( _T("https://") ) == 0 ||
				strURI.Find( _T("ftp://") ) == 0 ||
				strURI.Find( _T("mailto:") ) == 0 ||
				strURI.Find( _T("aim:") ) == 0 ||
				strURI.Find( _T("magnet:") ) == 0 ||
				strURI.Find( _T("gnutella:") ) == 0 ||
				strURI.Find( _T("gnet:") ) == 0 ||
				strURI.Find( _T("shareaza:") ) == 0 ||
				strURI.Find( _T("gwc:") ) == 0 ||
				strURI.Find( _T("uhc:") ) == 0 ||
				strURI.Find( _T("ukhl:") ) == 0 ||
				strURI.Find( _T("gnutella1:") ) == 0 ||
				strURI.Find( _T("gnutella2:") ) == 0 ||
				strURI.Find( _T("mp2p:") ) == 0 ||
				strURI.Find( _T("ed2k:") ) == 0 ||
				strURI.Find( _T("sig2dat:") ) == 0 )
	{
		ShellExecute( pMainWnd->GetSafeHwnd(), _T("open"), strURI,
			NULL, NULL, SW_SHOWNORMAL );
	}
	else if ( strURI == _T("raza:connect") )
	{
		pMainWnd->PostMessage( WM_COMMAND, ID_NETWORK_CONNECT );
	}
	else if ( strURI == _T("raza:disconnect") )
	{
		pMainWnd->PostMessage( WM_COMMAND, ID_NETWORK_DISCONNECT );
	}
	else if ( strURI == _T("raza:search") )
	{
		pMainWnd->PostMessage( WM_COMMAND, ID_TAB_SEARCH );
	}
	else if ( strURI == _T("raza:neighbours") )
	{
		pMainWnd->PostMessage( WM_COMMAND, ID_VIEW_NEIGHBOURS );
	}
	else if ( strURI == _T("raza:downloads") )
	{
		pMainWnd->PostMessage( WM_COMMAND, ID_VIEW_DOWNLOADS );
	}
	else if ( strURI == _T("raza:uploads") )
	{
		pMainWnd->PostMessage( WM_COMMAND, ID_VIEW_UPLOADS );
	}
	else if ( strURI == _T("raza:shell:downloads") )
	{
		ShellExecute( pMainWnd->GetSafeHwnd(), _T("open"),
			Settings.Downloads.CompletePath, NULL, NULL, SW_SHOWNORMAL );
	}
	else if ( strURI == _T("raza:upgrade") )
	{
		pMainWnd->PostMessage( WM_VERSIONCHECK, 1 );
	}
	else if ( strURI == _T("raza:options") )
	{
		pMainWnd->PostMessage( WM_COMMAND, ID_TOOLS_SETTINGS );
	}
	else if ( strURI == _T("raza:options:skins") )
	{
		pMainWnd->PostMessage( WM_COMMAND, ID_TOOLS_SKIN );
	}
	else if ( strURI == _T("raza:wizard") )
	{
		pMainWnd->PostMessage( WM_COMMAND, ID_TOOLS_WIZARD );
	}
	else if ( strURI == _T("raza:library") )
	{
		pMainWnd->PostMessage( WM_COMMAND, ID_VIEW_LIBRARY );
	}
	else if ( strURI == _T("raza:library:downloads") )
	{
		pMainWnd->PostMessage( WM_COMMAND, ID_VIEW_LIBRARY );
	}
	else if ( strURI == _T("raza:library:history") )
	{
		pMainWnd->PostMessage( WM_COMMAND, ID_VIEW_LIBRARY );
		pMainWnd->PostMessage( WM_COMMAND, ID_LIBRARY_TREE_VIRTUAL );
	}
	else if ( strURI.Find( _T("raza:library:/") ) == 0 )
	{
		pMainWnd->PostMessage( WM_COMMAND, ID_VIEW_LIBRARY );
		pMainWnd->PostMessage( WM_COMMAND, ID_LIBRARY_TREE_VIRTUAL );
	}
	else
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Runtime class lookup

void AFXAPI AfxLockGlobals(int nLockType);
void AFXAPI AfxUnlockGlobals(int nLockType);

CRuntimeClass* AfxClassForName(LPCTSTR pszClass)
{
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();

	AfxLockGlobals( 0 );

	for ( CRuntimeClass* pClass = pModuleState->m_classList ; pClass != NULL ; pClass = pClass->m_pNextClass )
	{
		if ( CString( pClass->m_lpszClassName ).CompareNoCase( pszClass ) == 0 )
		{
			AfxUnlockGlobals( 0 );
			return pClass;
		}
	}

	AfxUnlockGlobals( 0 );

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// String functions

void Split(const CString& strSource, TCHAR cDelimiter, CStringArray& pAddIt, BOOL bAddFirstEmpty)
{
	for( LPCTSTR start = strSource; *start; start++ )
	{
		LPCTSTR c = _tcschr( start, cDelimiter );
		int len = c ? (int) ( c - start ) : (int) _tcslen( start );
		if ( len > 0 )
			pAddIt.Add( CString( start, len ) );
		else
			if ( bAddFirstEmpty && ( start == strSource ) )
				pAddIt.Add( CString() );
		if ( ! c )
			break;
		start = c;
	}
}

BOOL LoadString(CString& str, UINT nID)
{
	return Skin.LoadString( str, nID );
}

BOOL LoadSourcesString(CString& str, DWORD num, bool bFraction)
{
	if ( bFraction )
	{
		return Skin.LoadString( str, IDS_STATUS_SOURCESOF );
	}
	else if ( num == 0 )
	{
		return Skin.LoadString( str, IDS_STATUS_NOSOURCES );
	}
	else if ( num == 1 )
	{
		return Skin.LoadString( str, IDS_STATUS_SOURCE );
	}
	else if ( ( ( num % 100 ) > 10) && ( ( num % 100 ) < 20 ) )
	{
		return Skin.LoadString( str, IDS_STATUS_SOURCES11TO19 );
	}
	else
	{
		switch ( num % 10 )
		{
			case 0: 
				return Skin.LoadString( str, IDS_STATUS_SOURCESTENS );
			case 1:
				return Skin.LoadString( str, IDS_STATUS_SOURCES );				
			case 2:
			case 3:
			case 4:
				return Skin.LoadString( str, IDS_STATUS_SOURCES2TO4 );
			default:
				return Skin.LoadString( str, IDS_STATUS_SOURCES5TO9 );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Case independent string search

LPCTSTR _tcsistr(LPCTSTR pszString, LPCTSTR pszPattern)
{
	if ( !*pszString || !*pszPattern ) return NULL;

	const TCHAR cFirstPatternChar = ToLower( *pszPattern );

	for ( ; ; ++pszString )
	{
		while ( *pszString && ToLower( *pszString ) != cFirstPatternChar ) ++pszString;

		if ( !*pszString ) return NULL;

		int i = 0;
		while ( const TCHAR cPatternChar = ToLower( pszPattern[ ++i ] ) )
		{
			if ( const TCHAR cStringChar = ToLower( pszString[ i ] ) )
			{
				if ( cStringChar != cPatternChar ) break;
			}
			else
			{
				return NULL;
			}
		}

		if ( !pszPattern[ i ] ) return pszString;
	}
}

LPCTSTR _tcsnistr(LPCTSTR pszString, LPCTSTR pszPattern, size_t plen)
{
	if ( !*pszString || !*pszPattern || !plen ) return NULL;

	const TCHAR cFirstPatternChar = ToLower( *pszPattern );

	for ( ; ; ++pszString )
	{
		while ( *pszString && ToLower( *pszString ) != cFirstPatternChar ) ++pszString;

		if ( !*pszString ) return NULL;

		DWORD i = 0;
		while ( ++i < plen )
		{
			if ( const TCHAR cStringChar = ToLower( pszString[ i ] ) )
			{
				if ( cStringChar != ToLower( pszPattern[ i ] ) ) break;
			}
			else
			{
				return NULL;
			}
		}

		if ( i == plen ) return pszString;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Time Management Functions (C-runtime)

DWORD TimeFromString(LPCTSTR pszTime)
{
	// 2002-04-30T08:30Z
	
	if ( _tcslen( pszTime ) != 17 ) return 0;
	if ( pszTime[4] != '-' || pszTime[7] != '-' ) return 0;
	if ( pszTime[10] != 'T' || pszTime[13] != ':' || pszTime[16] != 'Z' ) return 0;
	
	LPCTSTR psz;
	int nTemp;
	
	tm pTime = {};

	if ( _stscanf( pszTime, _T("%i"), &nTemp ) != 1 ) return 0;
	pTime.tm_year = nTemp - 1900;
	for ( psz = pszTime + 5 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, _T("%i"), &nTemp ) != 1 ) return 0;
	pTime.tm_mon = nTemp - 1;
	for ( psz = pszTime + 8 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, _T("%i"), &nTemp ) != 1 ) return 0;
	pTime.tm_mday = nTemp;
	for ( psz = pszTime + 11 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, _T("%i"), &nTemp ) != 1 ) return 0;
	pTime.tm_hour = nTemp;
	for ( psz = pszTime + 14 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, _T("%i"), &nTemp ) != 1 ) return 0;
	pTime.tm_min = nTemp;
	
	time_t tGMT = mktime( &pTime );
	// check for invalid dates
	if (tGMT == -1) 
	{
		theApp.Message( MSG_ERROR, _T("Invalid Date/Time"), pszTime );
		return 0;
	}
	struct tm* pGM = gmtime( &tGMT );
	time_t tSub = mktime( pGM );
	
	if (tSub == -1) 
	{
		theApp.Message( MSG_ERROR, _T("Invalid Date/Time"), pszTime );
		return 0;
	}
	
	return DWORD( 2 * tGMT - tSub );
}

CString TimeToString(time_t tVal)
{
	tm* pTime = gmtime( &tVal );
	CString str;

	str.Format( _T("%.4i-%.2i-%.2iT%.2i:%.2iZ"),
		pTime->tm_year + 1900, pTime->tm_mon + 1, pTime->tm_mday,
		pTime->tm_hour, pTime->tm_min );

	return str;
}

/////////////////////////////////////////////////////////////////////////////
// Time Management Functions (FILETIME)

BOOL TimeFromString(LPCTSTR pszTime, FILETIME* pTime)
{
	// 2002-04-30T08:30Z
	
	if ( _tcslen( pszTime ) != 17 ) return FALSE;
	if ( pszTime[4] != '-' || pszTime[7] != '-' ) return FALSE;
	if ( pszTime[10] != 'T' || pszTime[13] != ':' || pszTime[16] != 'Z' ) return FALSE;
	
	LPCTSTR psz;
	int nTemp;

	SYSTEMTIME pOut = {};

	if ( _stscanf( pszTime, _T("%i"), &nTemp ) != 1 ) return FALSE;
	pOut.wYear = WORD( nTemp );
	for ( psz = pszTime + 5 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, _T("%i"), &nTemp ) != 1 ) return FALSE;
	pOut.wMonth = WORD( nTemp );
	for ( psz = pszTime + 8 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, _T("%i"), &nTemp ) != 1 ) return FALSE;
	pOut.wDay = WORD( nTemp );
	for ( psz = pszTime + 11 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, _T("%i"), &nTemp ) != 1 ) return FALSE;
	pOut.wHour = WORD( nTemp );
	for ( psz = pszTime + 14 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, _T("%i"), &nTemp ) != 1 ) return FALSE;
	pOut.wMinute = WORD( nTemp );

	return SystemTimeToFileTime( &pOut, pTime );
}

CString	TimeToString(FILETIME* pTime)
{
	SYSTEMTIME pOut;
	CString str;

	FileTimeToSystemTime( pTime, &pOut );

	str.Format( _T("%.4i-%.2i-%.2iT%.2i:%.2iZ"),
		pOut.wYear, pOut.wMonth, pOut.wDay,
		pOut.wHour, pOut.wMinute );

	return str;
}

/////////////////////////////////////////////////////////////////////////////
// Automatic dropdown list width adjustment (to fit translations)
// Use in ON_CBN_DROPDOWN events

void RecalcDropWidth(CComboBox* pWnd)
{
    // Reset the dropped width
    int nNumEntries = pWnd->GetCount();
    int nWidth = 0;
    CString str;

    CClientDC dc( pWnd );
    int nSave = dc.SaveDC();
    dc.SelectObject( pWnd->GetFont() );

    int nScrollWidth = GetSystemMetrics( SM_CXVSCROLL );
    for ( int nEntry = 0; nEntry < nNumEntries; nEntry++ )
    {
        pWnd->GetLBText( nEntry, str );
        int nLength = dc.GetTextExtent( str ).cx + nScrollWidth;
        nWidth = max( nWidth, nLength );
    }
    
    // Add margin space to the calculations
    nWidth += dc.GetTextExtent( _T("0") ).cx;

    dc.RestoreDC( nSave );
    pWnd->SetDroppedWidth( nWidth );
}

int AddIcon(UINT nIcon, CImageList& gdiImageList)
{
	return AddIcon( theApp.LoadIcon( nIcon ), gdiImageList );
}

int AddIcon(HICON hIcon, CImageList& gdiImageList)
{
	int num = -1;
	if ( hIcon )
	{
		if ( Settings.General.LanguageRTL )
			hIcon = CreateMirroredIcon( hIcon );
		num = gdiImageList.Add( hIcon );
		VERIFY( DestroyIcon( hIcon ) );
	}
	return num;
}

HICON CreateMirroredIcon(HICON hIconOrig, BOOL bDestroyOriginal)
{
	HDC hdcScreen, hdcBitmap, hdcMask = NULL;
	HBITMAP hbm, hbmMask, hbmOld,hbmOldMask;
	BITMAP bm;
	ICONINFO ii;
	HICON hIcon = NULL;
	hdcBitmap = CreateCompatibleDC( NULL );
	if ( hdcBitmap )
	{
		hdcMask = CreateCompatibleDC( NULL );
		if( hdcMask )
		{
			if ( theApp.m_pfnSetLayout )
			{
				theApp.m_pfnSetLayout( hdcBitmap, LAYOUT_RTL );
				theApp.m_pfnSetLayout( hdcMask, LAYOUT_RTL );
			}
		}
		else
		{
			DeleteDC( hdcBitmap );
			hdcBitmap = NULL;
		}
	}
	hdcScreen = GetDC( NULL );
	if ( hdcScreen )
	{
		if ( hdcBitmap && hdcMask )
		{
			if ( hIconOrig )
			{
				if ( GetIconInfo( hIconOrig, &ii ) && GetObject( ii.hbmColor, sizeof(BITMAP), &bm ) )
				{
					// Do the cleanup for the bitmaps.
					DeleteObject( ii.hbmMask );
					DeleteObject( ii.hbmColor );
					ii.hbmMask = ii.hbmColor = NULL;
					hbm = CreateCompatibleBitmap( hdcScreen, bm.bmWidth, bm.bmHeight );
					if ( hbm != NULL )
					{
						hbmMask = CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, NULL );
						if ( hbmMask != NULL )
						{
							hbmOld = (HBITMAP)SelectObject( hdcBitmap, hbm );
							hbmOldMask = (HBITMAP)SelectObject( hdcMask,hbmMask );
							DrawIconEx( hdcBitmap, 0, 0, hIconOrig, bm.bmWidth, bm.bmHeight, 0, NULL, DI_IMAGE );
							DrawIconEx( hdcMask, 0, 0, hIconOrig, bm.bmWidth, bm.bmHeight, 0, NULL, DI_MASK );
							SelectObject( hdcBitmap, hbmOld );
							SelectObject( hdcMask, hbmOldMask );
							// Create the new mirrored icon and delete bitmaps

							ii.hbmMask = hbmMask;
							ii.hbmColor = hbm;
							hIcon = CreateIconIndirect( &ii );
							DeleteObject( hbmMask );
						}
						DeleteObject( hbm );
					}
				}
			}
		}
		ReleaseDC( NULL, hdcScreen );
	}

	if ( hdcBitmap ) DeleteDC( hdcBitmap );
	if ( hdcMask ) DeleteDC( hdcMask );
	if ( hIcon && hIconOrig && bDestroyOriginal ) VERIFY( DestroyIcon( hIconOrig ) );
	if ( ! hIcon ) hIcon = hIconOrig;
	return hIcon;
}

HBITMAP CreateMirroredBitmap(HBITMAP hbmOrig)
{
	HDC hdc, hdcMem1, hdcMem2;
	HBITMAP hbm = NULL, hOld_bm1, hOld_bm2;
	BITMAP bm;
	if ( !hbmOrig ) return NULL;
	if ( !GetObject( hbmOrig, sizeof(BITMAP), &bm ) ) return NULL;

	hdc = GetDC( NULL );
	if ( hdc )
	{
		hdcMem1 = CreateCompatibleDC( hdc );
		if ( !hdcMem1 )
		{
			ReleaseDC( NULL, hdc );
			return NULL;
		}
		hdcMem2 = CreateCompatibleDC( hdc );
		if ( !hdcMem2 )
		{
			DeleteDC( hdcMem1 );
			ReleaseDC( NULL, hdc );
			return NULL;
		}
		hbm = CreateCompatibleBitmap( hdc, bm.bmWidth, bm.bmHeight );
		if (!hbm)
		{
			DeleteDC( hdcMem1 );
			DeleteDC( hdcMem2 );
			ReleaseDC( NULL, hdc );
			return NULL;
		}
		// Flip the bitmap.
		hOld_bm1 = (HBITMAP)SelectObject( hdcMem1, hbmOrig );
		hOld_bm2 = (HBITMAP)SelectObject( hdcMem2, hbm );
		theApp.m_pfnSetLayout( hdcMem2, LAYOUT_RTL );
		BitBlt( hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem1, 0, 0, SRCCOPY );
		SelectObject( hdcMem1, hOld_bm1 );
		SelectObject( hdcMem2, hOld_bm2 );
		DeleteDC( hdcMem1 );
		DeleteDC( hdcMem2 );
		ReleaseDC( NULL, hdc );
	}
	return hbm;
}

void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
{
#ifndef NDEBUG
	struct
	{
		DWORD dwType;		// must be 0x1000
		LPCSTR szName;		// pointer to name (in user addr space)
		DWORD dwThreadID;	// thread ID (-1=caller thread)
		DWORD dwFlags;		// reserved for future use, must be zero
	} info =
	{
		0x1000,
		szThreadName,
		dwThreadID,
		0
	};
	__try
	{
		RaiseException( 0x406D1388, 0, sizeof info / sizeof( DWORD ), (ULONG_PTR*)&info );
	}
	__except( EXCEPTION_CONTINUE_EXECUTION )
	{
	}
#endif
	UNUSED_ALWAYS(dwThreadID);
	UNUSED_ALWAYS(szThreadName);
}

class CRazaThread : public CWinThread
{
	DECLARE_DYNAMIC(CRazaThread)

public:
	CRazaThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam) :
		CWinThread( NULL, pParam ),
		m_pfnThreadProcExt( pfnThreadProc )
	{
	}
	virtual ~CRazaThread()
	{
		Remove( m_hThread );
	}

	virtual BOOL InitInstance()
	{
		ASSERT_VALID( this );
		return TRUE;
	}

	virtual int Run()
	{
		ASSERT_VALID( this );
		ASSERT( m_pfnThreadProcExt );

		bool bCOM = SUCCEEDED( OleInitialize( NULL ) );

		int nResult = 0;
		if ( m_pfnThreadProcExt )
			nResult = ( *m_pfnThreadProcExt )( m_pThreadParams );

		if ( bCOM )
			OleUninitialize();

		return nResult;
	}

	static void Add(CRazaThread* pThread, LPCSTR pszName)
	{
		CSingleLock oLock( &m_ThreadMapSection, TRUE );

		if ( pszName )
		{
			SetThreadName( pThread->m_nThreadID, pszName );
		}

		CThreadTag tag = { pThread, pszName };
		m_ThreadMap.SetAt( pThread->m_hThread, tag );

		TRACE( _T("Creating '%hs' thread (0x%08x). Count: %d\n"),
			( pszName ? pszName : "unnamed" ), pThread->m_hThread, m_ThreadMap.GetCount() );
	}

	static void Remove(HANDLE hThread)
	{
		CSingleLock oLock( &m_ThreadMapSection, TRUE );

		CThreadTag tag = { 0 };
		if ( m_ThreadMap.Lookup( hThread, tag ) )
		{
			m_ThreadMap.RemoveKey( hThread );

			TRACE( _T("Removing '%hs' thread (0x%08x). Count: %d\n"),
				( tag.pszName ? tag.pszName : "unnamed" ),
				hThread, m_ThreadMap.GetCount() );
		}
	}

	static void Terminate(HANDLE hThread)
	{
		// Its a very dangerous function produces 100% urecoverable TLS leaks/deadlocks
		if ( TerminateThread( hThread, 0 ) )
		{
			CSingleLock oLock( &m_ThreadMapSection, TRUE );

			CThreadTag tag = { 0 };
			if ( m_ThreadMap.Lookup( hThread, tag ) )
			{
				ASSERT( hThread == tag.pThread->m_hThread );
				ASSERT_VALID( tag.pThread );
				ASSERT( static_cast<CWinThread*>( tag.pThread ) != AfxGetApp() );
				tag.pThread->Delete();
			}
			else
				CloseHandle( hThread );

			theApp.Message( MSG_DEBUG, _T("WARNING: Terminating '%hs' thread (0x%08x)."),
				( tag.pszName ? tag.pszName : "unnamed" ), hThread );
			TRACE( _T("WARNING: Terminating '%hs' thread (0x%08x).\n"),
				( tag.pszName ? tag.pszName : "unnamed" ), hThread );
		}
		else
		{
			theApp.Message( MSG_DEBUG, _T("WARNING: Terminating thread (0x%08x) failed."), hThread );
			TRACE( _T("WARNING: Terminating thread (0x%08x) failed.\n"), hThread );
		}
	}

protected:
	typedef struct
	{
		CRazaThread*	pThread;	// Thread object
		LPCSTR			pszName;	// Thread name
	} CThreadTag;

	typedef CMap<HANDLE, HANDLE, CThreadTag, const CThreadTag&> CThreadMap;

	static CCriticalSection	m_ThreadMapSection;	// Guarding of m_ThreadMap
	static CThreadMap		m_ThreadMap;		// Map of running threads
	AFX_THREADPROC			m_pfnThreadProcExt;
};

IMPLEMENT_DYNAMIC(CRazaThread, CWinThread)

CCriticalSection		CRazaThread::m_ThreadMapSection;
CRazaThread::CThreadMap	CRazaThread::m_ThreadMap;

HANDLE BeginThread(LPCSTR pszName, AFX_THREADPROC pfnThreadProc,
	LPVOID pParam, int nPriority, UINT nStackSize, DWORD dwCreateFlags,
	LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
	CRazaThread* pThread = new CRazaThread( pfnThreadProc, pParam );
	ASSERT_VALID( pThread );
	if ( pThread )
	{
		if ( pThread->CreateThread( dwCreateFlags | CREATE_SUSPENDED, nStackSize,
			lpSecurityAttrs ) )
		{
			VERIFY( pThread->SetThreadPriority( nPriority ) );
			if ( ! ( dwCreateFlags & CREATE_SUSPENDED ) )
				VERIFY( pThread->ResumeThread() != (DWORD)-1 );

			CRazaThread::Add( pThread, pszName );

			return pThread->m_hThread;
		}
		pThread->Delete();
	}
	return NULL;
}

void CloseThread(HANDLE* phThread, DWORD dwTimeout)
{
	if ( *phThread )
	{
		__try
		{
			SetThreadPriority( *phThread, THREAD_PRIORITY_HIGHEST );
			if ( WaitForSingleObject( *phThread, dwTimeout ) == WAIT_TIMEOUT )
			{
				CRazaThread::Terminate( *phThread );
			}
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			// Thread already ended
		}

		CRazaThread::Remove( *phThread );

		*phThread = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Keyboard hook: record tick count

LRESULT CALLBACK KbdHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if ( nCode == HC_ACTION )
	{
		theApp.m_dwLastInput = (DWORD)time( NULL );

		BOOL bAlt = (WORD)( lParam >> 16 ) & KF_ALTDOWN;
		// BOOL bCtrl = GetAsyncKeyState( VK_CONTROL ) & 0x80000000;
		if ( bAlt )
		{
			if ( wParam == VK_DOWN )
				SendMessage( AfxGetMainWnd()->GetSafeHwnd(), WM_SETALPHA, (WPARAM)0, 0 );
			if ( wParam == VK_UP )
				SendMessage( AfxGetMainWnd()->GetSafeHwnd(), WM_SETALPHA, (WPARAM)1, 0 );
		}
	}

	return ::CallNextHookEx( theApp.m_hHookKbd, nCode, wParam, lParam );
}

/////////////////////////////////////////////////////////////////////////////
// Mouse hook: record tick count

LRESULT CALLBACK MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if ( nCode == HC_ACTION )
		theApp.m_dwLastInput = (DWORD)time( NULL );

	return ::CallNextHookEx( theApp.m_hHookMouse, nCode, wParam, lParam );
}

CString GetFolderPath( int nFolder )
{
	TCHAR pszFolderPath[ MAX_PATH ] = { 0 };

	if ( theApp.m_pfnSHGetFolderPathW && SUCCEEDED( theApp.m_pfnSHGetFolderPathW( NULL, nFolder, NULL, NULL, pszFolderPath ) ) )
		return CString( pszFolderPath );

	return _T("");
}

CString GetWindowsFolder()
{
	TCHAR pszWindowsPath[ MAX_PATH ] = { 0 };
	UINT nReturnValue = GetWindowsDirectory( pszWindowsPath, MAX_PATH );
	if ( nReturnValue == 0 || nReturnValue > MAX_PATH ) return CString( _T("c:\\windows") );

	CharLower( pszWindowsPath );
	return CString( pszWindowsPath );
}

CString GetProgramFilesFolder()
{
	TCHAR pszProgramsPath[ MAX_PATH ] = { 0 };
	BOOL bOK = FALSE;

	if ( theApp.m_pfnSHGetFolderPathW && SUCCEEDED( theApp.m_pfnSHGetFolderPathW( NULL, CSIDL_PROGRAM_FILES, NULL, NULL, pszProgramsPath ) ) )
		bOK = TRUE;

	if ( !bOK || ! *pszProgramsPath )
	{
		// Get drive letter
		UINT nReturnValue = GetWindowsDirectory( pszProgramsPath, MAX_PATH );
		if ( nReturnValue == 0 || nReturnValue > MAX_PATH ) return CString( _T("c:\\program files") );

		_tcscpy( pszProgramsPath + 1, _T(":\\program files") );
	}

	CharLower( pszProgramsPath );
	return CString( pszProgramsPath );
}

CString GetDocumentsFolder()
{
	CString strDocumentsPath( GetFolderPath( CSIDL_PERSONAL ) );

	if ( !strDocumentsPath.GetLength() )
	{
		strDocumentsPath = CRegistry::GetString( _T("Shell Folders"), _T("Personal"), _T(""), _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer") );
	}
	ASSERT( strDocumentsPath.GetLength() );

	CharLower( strDocumentsPath.GetBuffer() );
	strDocumentsPath.ReleaseBuffer();

	return strDocumentsPath;
}

CString GetAppDataFolder()
{
	CString strAppDataPath( GetFolderPath( CSIDL_APPDATA ) );

	if ( !strAppDataPath.GetLength() )
	{
		strAppDataPath = CRegistry::GetString( _T("Shell Folders"), _T("AppData"), _T(""), _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer") );
	}
	ASSERT( strAppDataPath.GetLength() );

	CharLower( strAppDataPath.GetBuffer() );
	strAppDataPath.ReleaseBuffer();

	return strAppDataPath;
}

CString GetLocalAppDataFolder()
{
	CString strLocalAppDataPath( GetFolderPath( CSIDL_LOCAL_APPDATA ) );

	if ( !strLocalAppDataPath.GetLength() )
	{
		strLocalAppDataPath = CRegistry::GetString( _T("Shell Folders"), _T("Local AppData"), _T(""), _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer") );

		if ( !strLocalAppDataPath.GetLength() )
			return GetAppDataFolder();
	}	
	ASSERT( strLocalAppDataPath.GetLength() );

	CharLower( strLocalAppDataPath.GetBuffer() );
	strLocalAppDataPath.ReleaseBuffer();

	return strLocalAppDataPath;
}

BOOL CreateDirectory(LPCTSTR szPath)
{
	DWORD dwAttr = GetFileAttributes( szPath );
	if ( ( dwAttr != INVALID_FILE_ATTRIBUTES ) &&
		( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) )
		return TRUE;

	CString strDir( szPath );
	for ( int nStart = 2; ; )
	{
		int nSlash = strDir.Find( _T('\\'), nStart );
		if ( ( nSlash == -1 ) || ( nSlash == strDir.GetLength() - 1 ) )
			break;
		CString strSubDir( strDir.Left( nSlash ) );
		dwAttr = GetFileAttributes( strSubDir );
		if ( ( dwAttr == INVALID_FILE_ATTRIBUTES ) ||
			! ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) )
			if ( ! CreateDirectory( strSubDir, NULL ) )
				return FALSE;
		nStart = nSlash + 1;
	}
	return CreateDirectory( szPath, NULL );
}

CString LoadHTML(HINSTANCE hInstance, UINT nResourceID)
{
	CString strBody;
	BOOL bGZIP = FALSE;
	HRSRC hRes = FindResource( hInstance, MAKEINTRESOURCE( nResourceID ), RT_HTML );
	if ( ! hRes )
	{
		hRes = FindResource( hInstance, MAKEINTRESOURCE( nResourceID ), RT_GZIP );
		bGZIP = ( hRes != NULL );
	}
	if ( hRes )
	{
		DWORD nSize			= SizeofResource( hInstance, hRes );
		HGLOBAL hMemory		= LoadResource( hInstance, hRes );
		if ( hMemory )
		{
			LPCSTR pszInput	= (LPCSTR)LockResource( hMemory );
			if ( pszInput )
			{
				if ( bGZIP )
				{
					CBuffer buf;
					buf.Add( pszInput, nSize );
					if ( buf.Ungzip() )
					{
						int nWide = MultiByteToWideChar( 0, 0, (LPCSTR)buf.m_pBuffer, buf.m_nLength, NULL, 0 );
						LPTSTR pszOutput = strBody.GetBuffer( nWide + 1 );
						MultiByteToWideChar( 0, 0, (LPCSTR)buf.m_pBuffer, buf.m_nLength, pszOutput, nWide );
						pszOutput[ nWide ] = _T('\0');
						strBody.ReleaseBuffer();
					}
				}
				else
				{
					int nWide = MultiByteToWideChar( 0, 0, pszInput, nSize, NULL, 0 );
					LPTSTR pszOutput = strBody.GetBuffer( nWide + 1 );
					MultiByteToWideChar( 0, 0, pszInput, nSize, pszOutput, nWide );
					pszOutput[ nWide ] = _T('\0');
					strBody.ReleaseBuffer();
				}
			}
			FreeResource( hMemory );
		}
	}
	return strBody;
}

const struct
{
	LPCTSTR szPath;
	UINT	nID;
	LPCTSTR	szType;
	LPCTSTR	szContentType;
} WebResources [] =
{
	{ _T("/remote/header_1.png"),	IDR_HOME_HEADER_1,	RT_PNG,			_T("image/png") },
	{ _T("/remote/header_2.png"),	IDR_HOME_HEADER_2,	RT_PNG,			_T("image/png") },
	{ _T("/favicon.ico"),			IDR_MAINFRAME,		RT_GROUP_ICON,	_T("image/x-icon") },
	{ NULL, NULL, NULL, NULL }
};

bool ResourceRequest(const CString& strPath, CBuffer& pResponse, CString& sHeader)
{
	bool ret = false;
	for ( int i = 0; WebResources[ i ].szPath; i++ )
	{
		if ( strPath.Compare( WebResources[ i ].szPath ) == 0 )
		{
			HMODULE hModule = AfxGetResourceHandle();
			if ( HRSRC hRes = FindResource( hModule,
				MAKEINTRESOURCE( WebResources[ i ].nID ), WebResources[ i ].szType ) )
			{
				DWORD nSize			= SizeofResource( hModule, hRes );
				HGLOBAL hMemory		= LoadResource( hModule, hRes );
				if ( hMemory )
				{
					BYTE* pSource	= (BYTE*)LockResource( hMemory );
					if ( pSource )
					{
						if ( WebResources[ i ].szType == RT_GROUP_ICON )
						{
							// Save main header
							ICONDIR* piDir = (ICONDIR*)pSource;
							DWORD dwTotalSize = sizeof( ICONDIR ) +
								sizeof( ICONDIRENTRY ) * piDir->idCount; 
							pResponse.EnsureBuffer( dwTotalSize );
							CopyMemory( pResponse.m_pBuffer, piDir, sizeof( ICONDIR ) );

							GRPICONDIRENTRY* piDirEntry = (GRPICONDIRENTRY*)
								( pSource + sizeof( ICONDIR ) );

							// Find all subicons
							for ( WORD i = 0; i < piDir->idCount; i++ )
							{
								// pResponse.m_pBuffer may be changed
								ICONDIRENTRY* piEntry = (ICONDIRENTRY*)
									( pResponse.m_pBuffer + sizeof( ICONDIR ) );

								// Load subicon
								HRSRC hResIcon = FindResource( hModule, MAKEINTRESOURCE(
									piDirEntry[ i ].nID ), RT_ICON );
								if ( hResIcon )
								{
									DWORD nSizeIcon = SizeofResource( hModule, hResIcon );
									HGLOBAL hMemoryIcon = LoadResource( hModule, hResIcon );
									if ( hMemoryIcon )
									{
										BITMAPINFOHEADER* piImage = (BITMAPINFOHEADER*)LockResource( hMemoryIcon );

										// Fill subicon header
										piEntry[ i ].bWidth = piDirEntry[ i ].bWidth;
										piEntry[ i ].bHeight = piDirEntry[ i ].bHeight;
										piEntry[ i ].wPlanes = piDirEntry[ i ].wPlanes;
										piEntry[ i ].bColorCount = piDirEntry[ i ].bColorCount;
										piEntry[ i ].bReserved = 0;
										piEntry[ i ].wBitCount = piDirEntry[ i ].wBitCount;
										piEntry[ i ].dwBytesInRes = nSizeIcon;
										piEntry[ i ].dwImageOffset = dwTotalSize;

										// Save subicon
										pResponse.EnsureBuffer( dwTotalSize + nSizeIcon );
										CopyMemory( pResponse.m_pBuffer + dwTotalSize,
											piImage, nSizeIcon );
										dwTotalSize += nSizeIcon;

										FreeResource( hMemoryIcon );
									}
								}
							}
							pResponse.m_nLength = dwTotalSize;
						}
						else
						{
							pResponse.EnsureBuffer( nSize );
							CopyMemory( pResponse.m_pBuffer, pSource, nSize );
							pResponse.m_nLength = nSize;
						}
						sHeader.Format(
							_T("Content-Type: %s\r\n"),
							WebResources[ i ].szContentType);
						ret = true;
					}
					FreeResource( hMemory );
				}
			}
			break;
		}
	}
	return ret;
}

bool MarkFileAsDownload(const CString& sFilename)
{
	LPCTSTR pszExt = PathFindExtension( (LPCTSTR)sFilename );
	if ( pszExt == NULL ) return false;

	bool bSuccess = false;

	if ( theApp.m_bNT && Settings.Library.MarkFileAsDownload )
	{
		// TODO: pFile->m_bVerify and IDS_LIBRARY_VERIFY_FIX warning features could be merged 
		// with this function, because they resemble the security warning.
		// Should raza unblock files from the application without forcing user to do that manually?
		if ( IsIn( Settings.Library.SafeExecute, pszExt + 1 ) &&
			( !theApp.m_pfnAssocIsDangerous || !theApp.m_pfnAssocIsDangerous( pszExt ) ) )
			return false;

		// Temporary clear R/O attribute
		BOOL bChanged = FALSE;
		DWORD dwOrigAttr = GetFileAttributes( sFilename );
		if ( dwOrigAttr != INVALID_FILE_ATTRIBUTES && ( dwOrigAttr & FILE_ATTRIBUTE_READONLY ) )
			bChanged = SetFileAttributes( sFilename, dwOrigAttr & ~FILE_ATTRIBUTE_READONLY );

		HANDLE hFile = CreateFile( sFilename + _T(":Zone.Identifier"),
			GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			DWORD dwWritten = 0;
			bSuccess = ( WriteFile( hFile, "[ZoneTransfer]\r\nZoneID=3\r\n", 26,
				&dwWritten, NULL ) && dwWritten == 26 );
			CloseHandle( hFile );
		}
		else
			TRACE( "MarkFileAsDownload() : CreateFile \"%s\" error %d\n", sFilename, GetLastError() );

		if ( bChanged )
			SetFileAttributes( sFilename, dwOrigAttr );
	}
	return bSuccess;
}

bool LoadGUID(const CString& sFilename, Hashes::Guid& oGUID)
{
	bool bSuccess = false;
	if ( theApp.m_bNT && Settings.Library.UseFolderGUID )
	{
		HANDLE hFile = CreateFile( sFilename + _T(":Shareaza.GUID"),
			GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			Hashes::Guid oTmpGUID;
			DWORD dwReaded = 0;
			bSuccess = ( ReadFile( hFile, oTmpGUID.begin(), oTmpGUID.byteCount,
				&dwReaded, NULL ) && dwReaded == oTmpGUID.byteCount );
			if ( bSuccess )
			{
				oTmpGUID.validate();
				oGUID = oTmpGUID;
			}
			CloseHandle( hFile );
		}
	}
	return bSuccess;
}

bool SaveGUID(const CString& sFilename, const Hashes::Guid& oGUID)
{
	bool bSuccess = false;
	if ( theApp.m_bNT && Settings.Library.UseFolderGUID )
	{
		// Temporary clear R/O attribute
		BOOL bChanged = FALSE;
		DWORD dwOrigAttr = GetFileAttributes( sFilename );
		if ( dwOrigAttr != 0xffffffff && ( dwOrigAttr & FILE_ATTRIBUTE_READONLY ) )
			bChanged = SetFileAttributes( sFilename, dwOrigAttr & ~FILE_ATTRIBUTE_READONLY );

		HANDLE hFile = CreateFile( sFilename + _T(":Shareaza.GUID"),
			GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			DWORD dwWritten = 0;
			bSuccess = ( WriteFile( hFile, oGUID.begin(), oGUID.byteCount,
				&dwWritten, NULL ) && dwWritten == oGUID.byteCount );
			CloseHandle( hFile );
		}
		else
			TRACE( "SaveGUID() : CreateFile \"%s\" error %d\n", sFilename, GetLastError() );

		if ( bChanged )
			SetFileAttributes( sFilename, dwOrigAttr );
	}
	return bSuccess;
}

CString ResolveShortcut(LPCTSTR lpszFileName)
{
	CComPtr< IShellLink > pIShellLink;
	if ( SUCCEEDED( pIShellLink.CoCreateInstance( CLSID_ShellLink ) ) )
	{
		CComPtr< IPersistFile > pIPersistFile;
		pIPersistFile = pIShellLink;
		if ( pIPersistFile &&
			SUCCEEDED( pIPersistFile->Load( CComBSTR( lpszFileName ), STGM_READ ) ) &&
			SUCCEEDED( pIShellLink->Resolve( AfxGetMainWnd()->GetSafeHwnd(), SLR_NO_UI |
			SLR_NOUPDATE | SLR_NOSEARCH | SLR_NOTRACK | SLR_NOLINKINFO ) ) )
		{
			CString sPath;
			BOOL bResult = SUCCEEDED( pIShellLink->GetPath( sPath.GetBuffer( MAX_PATH ),
				MAX_PATH, NULL, 0 ) );
			sPath.ReleaseBuffer();
			if ( bResult )
				return sPath;
		}
	}
	return CString();
}

// BrowseCallbackProc - BrowseForFolder callback function
static int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch ( uMsg )
	{
	case BFFM_INITIALIZED:
		{
			// Remove context help button from dialog caption
			SetWindowLong( hWnd, GWL_STYLE,
				GetWindowLong( hWnd, GWL_STYLE ) & ~DS_CONTEXTHELP );
			SetWindowLong( hWnd, GWL_EXSTYLE,
				GetWindowLong( hWnd, GWL_EXSTYLE ) & ~WS_EX_CONTEXTHELP );

			// Set initial directory
			SendMessage( hWnd, BFFM_SETSELECTION, TRUE, lpData );
		}
		break;

	case BFFM_SELCHANGED:
		{
			// Fail if non-filesystem
			TCHAR szDir[ MAX_PATH ] = {};
			BOOL bResult = SHGetPathFromIDList( (LPITEMIDLIST)lParam, szDir );
			if ( bResult )
			{
				// Fail if folder not accessible
				bResult = ( _taccess( szDir, 4 ) == 0 );
				if ( bResult )
				{
					// Fail if pidl is a link
					SHFILEINFO sfi = {};
					bResult = ( SHGetFileInfo( (LPCTSTR)lParam, 0, &sfi, sizeof( sfi ), 
						SHGFI_PIDL | SHGFI_ATTRIBUTES ) &&
						( sfi.dwAttributes & SFGAO_LINK ) == 0 );
				}
			}
			SendMessage( hWnd, BFFM_ENABLEOK, 0, bResult );
		}
		break;
	}
	return 0;
}

// Displays a dialog box enabling the user to select a Shell folder
CString BrowseForFolder(UINT nTitle, LPCTSTR szInitialPath, HWND hWnd)
{
	CString strTitle;
	LoadString( strTitle, nTitle );
	return BrowseForFolder( strTitle, szInitialPath, hWnd );
}

// Displays a dialog box enabling the user to select a Shell folder
CString BrowseForFolder(LPCTSTR szTitle, LPCTSTR szInitialPath, HWND hWnd)
{
	// Get last used folder
	static TCHAR szDefaultPath[ MAX_PATH ] = {};
	if ( ! szInitialPath || ! *szInitialPath )
	{
		if ( ! *szDefaultPath )
			lstrcpyn( szDefaultPath, (LPCTSTR)GetDocumentsFolder(), MAX_PATH );
		szInitialPath = szDefaultPath;
	}

	TCHAR szDisplayName[ MAX_PATH ] = {};
	BROWSEINFO pBI = {};
	pBI.hwndOwner = hWnd ? hWnd : AfxGetMainWnd()->GetSafeHwnd();
	pBI.pszDisplayName = szDisplayName;
	pBI.lpszTitle = szTitle;
	pBI.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	pBI.lpfn = BrowseCallbackProc;
	pBI.lParam = (LPARAM)szInitialPath;
	LPITEMIDLIST pPath = SHBrowseForFolder( &pBI );
	if ( pPath == NULL )
		return CString();

	TCHAR szPath[ MAX_PATH ] = {};
	BOOL bResult = SHGetPathFromIDList( pPath, szPath );

	CComPtr< IMalloc > pMalloc;
	if ( SUCCEEDED( SHGetMalloc( &pMalloc ) ) )
		pMalloc->Free( pPath );

	if ( ! bResult )
		return CString();

	// Save last used folder
	lstrcpyn( szDefaultPath, szPath, MAX_PATH );

	return CString( szPath );
}
