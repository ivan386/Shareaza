//
// Shareaza.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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

#include "BTClients.h"
#include "BTInfo.h"
#include "CoolInterface.h"
#include "DCClients.h"
#include "DDEServer.h"
#include "DiscoveryServices.h"
#include "DlgDeleteFile.h"
#include "DownloadGroups.h"
#include "Downloads.h"
#include "EDClients.h"
#include "Emoticons.h"
#include "FileExecutor.h"
#include "Flags.h"
#include "FontManager.h"
#include "GProfile.h"
#include "HostCache.h"
#include "IEProtocol.h"
#include "ImageServices.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "Neighbours.h"
#include "Network.h"
#include "Plugins.h"
#include "QueryHashMaster.h"
#include "Registry.h"
#include "Scheduler.h"
#include "SchemaCache.h"
#include "Security.h"
#include "Settings.h"
#include "ShareazaURL.h"
#include "SharedFile.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "SQLite.h"
#include "ThumbCache.h"
#include "Transfers.h"
#include "UploadQueues.h"
#include "Uploads.h"
#include "VendorCache.h"
#include "VersionChecker.h"

#include "DlgHelp.h"
#include "DlgSplash.h"

#include "WndMain.h"
#include "WndMedia.h"
#include "WndPacket.h"
#include "WndSystem.h"

#include "revision.h"		// to update build time

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const LPCTSTR RT_BMP = _T("BMP");
const LPCTSTR RT_JPEG = _T("JPEG");
const LPCTSTR RT_PNG = _T("PNG");
const LPCTSTR RT_GZIP = _T("GZIP");
// double scaleX = 1;
// double scaleY = 1;

/////////////////////////////////////////////////////////////////////////////
// CShareazaCommandLineInfo

CShareazaCommandLineInfo::CShareazaCommandLineInfo() :
	m_bTray( FALSE ),
	m_bNoSplash( FALSE ),
	m_bNoAlphaWarning( FALSE ),
	m_nGUIMode( -1 ),
	m_bHelp( FALSE ),
	m_bWait( FALSE )
{
}

void CShareazaCommandLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
	if ( bFlag )
	{
		if ( ! _tcsicmp( pszParam, _T("tray") ) )
		{
			m_bTray = TRUE;
			m_bNoSplash = TRUE;
			return;
		}
		else if ( ! _tcsicmp( pszParam, _T("nosplash") ) )
		{
			m_bNoSplash = TRUE;
			return;
		}
		else if ( ! _tcsicmp( pszParam, _T("nowarn") ) )
		{
			m_bNoAlphaWarning = TRUE;
			return;
		}
		else if ( ! _tcsicmp( pszParam, _T("basic") ) )
		{
			m_nGUIMode = GUI_BASIC;
			return;
		}
		else if ( ! _tcsicmp( pszParam, _T("tabbed") ) )
		{
			m_nGUIMode = GUI_TABBED;
			return;
		}
		else if ( ! _tcsicmp( pszParam, _T("windowed") ) )
		{
			m_nGUIMode = GUI_WINDOWED;
			return;
		}
		else if ( ! _tcsicmp( pszParam, _T("?") ) )
		{
			m_bHelp = TRUE;
			return;
		}
		else if ( ! _tcsncicmp( pszParam, _T("task"), 4 ) )
		{
			m_sTask = pszParam + 4;
			return;
		}
		else if ( ! _tcsicmp( pszParam, _T("wait") ) )
		{
			m_bWait = TRUE;
			return;
		}
	}
	CCommandLineInfo::ParseParam( pszParam, bFlag, bLast );
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp

BEGIN_MESSAGE_MAP(CShareazaApp, CWinApp)
END_MESSAGE_MAP()

CShareazaApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp construction

CShareazaApp::CShareazaApp() :
	m_pMutex				( NULL )
,	m_nFontQuality			( DEFAULT_QUALITY )
,	m_pSafeWnd				( NULL )
,	m_bBusy					( 0 )
,	m_bInteractive			( false )
,	m_bLive					( false )
,	m_bClosing				( false )
,	m_bIsServer				( false )
,	m_bIsWin2000			( false )
,	m_bIsVistaOrNewer		( false )
,	m_bLimitedConnections	( true )
,	m_nWindowsVersion		( 0ul )
,	m_nWindowsVersionMinor	( 0ul )
,	m_bMenuWasVisible		( FALSE )
,	m_nLastInput			( 0ul )
,	m_hHookKbd				( NULL )
,	m_hHookMouse			( NULL )
,	m_pPacketWnd			( NULL )

,	m_hCryptProv			( NULL )

,	m_pRegisterApplicationRestart( NULL )

,	m_hTheme				( NULL )
,	m_pfnSetWindowTheme		( NULL )
,	m_pfnIsThemeActive		( NULL )
,	m_pfnOpenThemeData		( NULL )
,	m_pfnCloseThemeData		( NULL )
,	m_pfnDrawThemeBackground( NULL )

,	m_hShlWapi				( NULL )
,	m_pfnAssocIsDangerous	( NULL )

,	m_hShell32				( NULL )
,	m_pfnSHGetFolderPathW	( NULL )
,	m_pfnSHGetKnownFolderPath( NULL )

,	m_hGeoIP				( NULL )
,	m_pGeoIP				( NULL )
,	m_pfnGeoIP_delete		( NULL )
,	m_pfnGeoIP_country_code_by_ipnum( NULL )
,	m_pfnGeoIP_country_name_by_ipnum( NULL )

,	m_hLibGFL				( NULL )

,	m_dlgSplash				( NULL )
,	m_SysInfo				()
{
	ZeroMemory( m_nVersion, sizeof( m_nVersion ) );
	ZeroMemory( m_pBTVersion, sizeof( m_pBTVersion ) );

// BugTrap http://www.intellesoft.net/
	BT_SetAppName( _T(CLIENT_NAME) );
	BT_SetFlags( BTF_INTERCEPTSUEF | BTF_SHOWADVANCEDUI | BTF_DESCRIBEERROR |
		BTF_DETAILEDMODE | BTF_ATTACHREPORT | BTF_EDITMAIL );
	BT_SetExitMode( BTEM_CONTINUESEARCH );
	BT_SetDumpType( 0x00001851 /* MiniDumpWithDataSegs | MiniDumpScanMemory | MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithFullMemoryInfo | MiniDumpWithThreadInfo */ );
	BT_SetSupportEMail( _T("shareaza-bugtrap@lists.sourceforge.net") );
	BT_SetSupportURL( WEB_SITE_T _T("?id=support") );
	BT_AddRegFile( _T("settings.reg"), _T("HKEY_CURRENT_USER\\Software\\") _T(CLIENT_NAME) );
	BT_InstallSehFilter();
	BT_SetTerminate();
}

CShareazaApp::~CShareazaApp()
{
	if ( m_pMutex != NULL )
	{
		CloseHandle( m_pMutex );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp initialization

BOOL CShareazaApp::InitInstance()
{
	CWinApp::InitInstance();

	SetRegistryKey( _T(CLIENT_NAME) );

	AfxOleInit();									// Initializes OLE support for the application.
//	m_pFontManager = new CFontManager();
	AfxEnableControlContainer( /*m_pFontManager*/); // Enable support for containment of OLE controls.
	InitResources();								// Loads theApp settings.

	if ( ! ParseCommandLine() )
		return FALSE;

	Register();					// Re-register Shareaza Type Library

	LoadStdProfileSettings();	// Load MRU file list and last preview state.
	EnableShellOpen();			// Enable open data files when user double-click the files from within the Windows File Manager.
	Settings.Load();			// Loads settings. Depends on InitResources().
	InitFonts();				// Loads default fonts. Depends on Settings.Load().
	Skin.CreateDefault();		// Loads colors, fonts and language. Depends on InitFonts().

	if ( m_cmdInfo.m_bHelp )
	{
		AfxMessageBox( IDS_COMMANDLINE, MB_ICONINFORMATION | MB_OK );
		return FALSE;
	}

	if ( m_pRegisterApplicationRestart )
		m_pRegisterApplicationRestart( _T("-nowarn"), 0 );

	ShowStartupText();

	DDEServer.Create();
	IEProtocol.Create();

#if ! RELEASE_BUILD
	// Beta expiry. Remember to re-compile to update the time, and remove this
	// section for final releases and public betas.
	COleDateTime tCompileTime;
	tCompileTime.ParseDateTime( _T(__DATE__), LOCALE_NOUSEROVERRIDE, 1033 );
	COleDateTime tCurrent = COleDateTime::GetCurrentTime();
	COleDateTimeSpan tTimeOut( 7, 0, 0, 0);			// Daily builds
	if ( ( tCompileTime + tTimeOut )  < tCurrent )
	{
		if ( AfxMessageBox(
			_T("This is a pre-release version of ") CLIENT_NAME_T _T(", and the beta testing period has ended.  ")
			_T("Please download the full, official release from ") WEB_SITE_T _T("."), MB_ICONQUESTION|MB_OK ) != IDOK )
			return FALSE;
	}

	// Alpha warning. Remember to remove this section for final releases and public betas.
	if ( ! m_cmdInfo.m_bNoAlphaWarning && m_cmdInfo.m_bShowSplash )
	if ( AfxMessageBox(
		_T("WARNING: This is an ALPHA TEST version of ") CLIENT_NAME_T _T(".\n\n")
		_T("It is NOT FOR GENERAL USE, and is only for testing specific features in a controlled ")
		_T("environment. It will frequently stop running, or display debug information to assist testing.\n\n")
		_T("If you wish to actually use this software, you should download ")
		_T("the current stable release from ") WEB_SITE_T _T("\n")
		_T("If you continue past this point, you may experience system instability, lose downloads, ")
		_T("or corrupt system files. Corrupted downloads/files may not be recoverable. ")
		_T("Do you wish to continue?"), MB_ICONEXCLAMATION|MB_YESNO ) == IDNO )
		return FALSE;
#endif // RELEASE_BUILD

	m_bInteractive = true;

	if ( m_cmdInfo.m_nGUIMode != -1 )
		Settings.General.GUIMode = m_cmdInfo.m_nGUIMode;
	if ( Settings.General.GUIMode != GUI_WINDOWED &&
		 Settings.General.GUIMode != GUI_TABBED &&
		 Settings.General.GUIMode != GUI_BASIC )
		Settings.General.GUIMode = GUI_BASIC;

	SplashStep( L"Network", ( ( m_cmdInfo.m_bNoSplash || ! m_cmdInfo.m_bShowSplash ) ? 0 : 17 ), false );
		if ( ! Network.Init() )
		{
			SplashAbort();
			AfxMessageBox( _T("Failed to initialize Windows Sockets."), MB_ICONHAND | MB_OK );
			return FALSE;
		}
	SplashStep( L"Database" );
		PurgeDeletes();
		CThumbCache::InitDatabase();
	SplashStep( L"P2P URIs" );
		CShareazaURL::Register( TRUE, TRUE );
		if ( Settings.Live.FirstRun )
			Plugins.Register();
	SplashStep( L"Shell Icons" );
		ShellIcons.Clear();
	SplashStep( L"Metadata Schemas" );
		if ( SchemaCache.Load() < 46 )
		{
			SplashAbort();
			AfxMessageBox( IDS_SCHEMA_LOAD_ERROR, MB_ICONHAND | MB_OK );
			return FALSE;
		}
		if ( ! Settings.MediaPlayer.FileTypes.size() )
		{
			CString sTypeFilter;
			static const LPCTSTR szTypes[] =
			{
				CSchema::uriAudio,
				CSchema::uriVideo,
				NULL
			};
			for ( int i = 0; szTypes[ i ]; ++ i )
				if ( CSchemaPtr pSchema = SchemaCache.Get( szTypes[ i ] ) )
					sTypeFilter += pSchema->m_sTypeFilter;
			sTypeFilter.Replace( _T("|."), _T("|") );
			CSettings::LoadSet( &Settings.MediaPlayer.FileTypes, sTypeFilter );
		}
		if ( ! Settings.Library.SafeExecute.size() )
		{
			CString sTypeFilter;
			static const LPCTSTR szTypes[] =
			{
				CSchema::uriArchive,
				CSchema::uriAudio,
				CSchema::uriVideo,
				CSchema::uriBook,
				CSchema::uriCollection,
				CSchema::uriImage,
				CSchema::uriBitTorrent,
				NULL
			};
			for ( int i = 0; szTypes[ i ]; ++ i )
				if ( CSchemaPtr pSchema = SchemaCache.Get( szTypes[ i ] ) )
					sTypeFilter += pSchema->m_sTypeFilter;
			sTypeFilter.Replace( _T("|."), _T("|") );
			CSettings::LoadSet( &Settings.Library.SafeExecute, sTypeFilter );
		}
	SplashStep( L"Vendor Data" );
		VendorCache.Load();
	SplashStep( L"Profile" );
		MyProfile.Load();
	SplashStep( L"Security Services" );
		Security.Load();
		AdultFilter.Load();
		MessageFilter.Load();
	SplashStep( L"Query Manager" );
		QueryHashMaster.Create();
	SplashStep( L"Host Cache" );
		HostCache.Load();
	SplashStep( L"Discovery Services" );
		DiscoveryServices.Load();
	SplashStep( L"Rich Documents" );
		if ( ! Emoticons.Load() )
		{
			Message( MSG_ERROR, _T("Failed to load Emoticons.") );
		}
		if ( ! Flags.Load() )
		{
			Message( MSG_ERROR, _T("Failed to load Flags.") );
		}
	SplashStep( L"GUI" );
		if ( m_cmdInfo.m_bTray ) WriteProfileInt( _T("Windows"), _T("CMainWnd.ShowCmd"), 0 );
		new CMainWnd();
		CoolMenu.EnableHook();
		if ( m_cmdInfo.m_bTray )
		{
			((CMainWnd*)m_pMainWnd)->CloseToTray();
		}
		else
		{
			m_pMainWnd->ShowWindow( SW_SHOW );
			if ( m_dlgSplash )
				m_dlgSplash->Topmost();
			m_pMainWnd->UpdateWindow();
		}

	SplashStep( L"Download Manager" );
		Downloads.Load();
	SplashStep( L"Upload Manager" );
		UploadQueues.Load();
	SplashStep( L"Library" );
		Library.Load();
	SplashStep( L"Upgrade Manager" );
		VersionChecker.Start();

	SplashStep();

	m_bLive = true;

	ProcessShellCommand( m_cmdInfo );

//	afxMemDF = allocMemDF | delayFreeMemDF | checkAlwaysMemDF;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp termination

int CShareazaApp::ExitInstance()
{
	if ( m_bInteractive )
	{
		CWaitCursor pCursor;

		SplashStep( L"Disconnecting" );
		VersionChecker.Stop();
		DiscoveryServices.Stop();
		Network.Disconnect();

		SplashStep( L"Stopping Library Tasks" );
		LibraryBuilder.CloseThread();
		Library.CloseThread();

		SplashStep( L"Stopping Transfers" );
		Transfers.StopThread();
		Downloads.CloseTransfers();

		SplashStep( L"Clearing Clients" );
		Uploads.Clear( FALSE );
		BTClients.Clear();
		DCClients.Clear();
		EDClients.Clear();

		SplashStep( L"Saving" );
		if ( m_bLive )
		{
			Downloads.Save();
			DownloadGroups.Save();
			Library.Save();
			Security.Save();
			HostCache.Save();
			UploadQueues.Save();
			DiscoveryServices.Save();
			Settings.Save( TRUE );
		}

		SplashStep( L"Finalizing" );
		Downloads.Clear( true );
		Library.Clear();
		CoolMenu.Clear();
		Network.Clear();

		SplashStep();
	}

	DDEServer.Close();
	IEProtocol.Close();
	Skin.Clear();
	Plugins.Clear();

	if ( m_hTheme != NULL )
		FreeLibrary( m_hTheme );

	if ( m_hShell32 != NULL )
		FreeLibrary( m_hShell32 );

	if ( m_hShlWapi != NULL )
		FreeLibrary( m_hShlWapi );

	FreeCountry();

	if ( m_hLibGFL != NULL )
		FreeLibrary( m_hLibGFL );

//	delete m_pFontManager;

	UnhookWindowsHookEx( m_hHookKbd );
	UnhookWindowsHookEx( m_hHookMouse );

	if ( m_hCryptProv )
		CryptReleaseContext( m_hCryptProv, 0 );

	{
		CQuickLock pLock( m_csMessage );
		while ( ! m_oMessages.IsEmpty() )
			delete m_oMessages.RemoveHead();
	}

	return CWinApp::ExitInstance();
}

BOOL CShareazaApp::ParseCommandLine()
{
	CWinApp::ParseCommandLine( m_cmdInfo );

	AfxSetPerUserRegistration( m_cmdInfo.m_bRegisterPerUser || ! IsRunAsAdmin() );

	if ( m_cmdInfo.m_nShellCommand == CCommandLineInfo::AppUnregister ||
		 m_cmdInfo.m_nShellCommand == CCommandLineInfo::AppRegister )
	{
		m_cmdInfo.m_bRunEmbedded = TRUE; // Suppress dialog

		ProcessShellCommand( m_cmdInfo );

		return FALSE;
	}

	BOOL bFirst = FALSE;
	HWND hwndFirst = NULL;
	for (;;)
	{
		m_pMutex = CreateMutex( NULL, FALSE, _T("Global\\") _T(CLIENT_NAME) );
		if ( m_pMutex != NULL )
		{
			if ( GetLastError() == ERROR_ALREADY_EXISTS )
			{
				CloseHandle( m_pMutex );
				m_pMutex = NULL;
				hwndFirst = FindWindow( _T(CLIENT_NAME) _T("MainWnd"), NULL );
			}
			else
				// We are first!
				bFirst = TRUE;
		}
		// else Probably mutex created in another user's session

		if ( ! m_cmdInfo.m_bWait || bFirst )
			break;

		// Wait for first instance exit
		Sleep( 250 );
	}

	if ( ! m_cmdInfo.m_sTask.IsEmpty() )
	{
		// Pass scheduler task to existing instance
		CScheduler::Execute( hwndFirst, m_cmdInfo.m_sTask );

		// Don't start second instance
		return FALSE;
	}

	if ( ! bFirst )
	{
		if ( hwndFirst )
		{
			if ( m_cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen )
			{
				// Pass command line to first instance
				m_cmdInfo.m_strFileName.Trim( _T(" \t\r\n\"") );
				COPYDATASTRUCT cd =
				{
					COPYDATA_OPEN,
					m_cmdInfo.m_strFileName.GetLength() * sizeof( TCHAR ),
					(PVOID)(LPCTSTR)m_cmdInfo.m_strFileName
				};
				DWORD_PTR dwResult;
				SendMessageTimeout( hwndFirst, WM_COPYDATA, NULL, (WPARAM)&cd, SMTO_NORMAL, 250, &dwResult );
			}
			else
			{
				// Popup first instance
				DWORD_PTR dwResult;
				SendMessageTimeout( hwndFirst, WM_COMMAND, ID_TRAY_OPEN, 0,
					SMTO_NORMAL, 250, &dwResult );
				ShowWindow( hwndFirst, SW_SHOWNA );
				BringWindowToTop( hwndFirst );
				SetForegroundWindow( hwndFirst );
			}
		}
		// else Probably window created in another user's session

		// Don't start second instance
		return FALSE;
	}

	if ( m_cmdInfo.m_bWait )
		// Wait for other instance complete exit
		Sleep( 1000 );

	// Continue Shareaza execution
	return TRUE;
}

void CShareazaApp::WinHelp(DWORD /*dwData*/, UINT /*nCmd*/)
{
	// Suppress F1
}

BOOL CShareazaApp::Register()
{
	COleObjectFactory::UpdateRegistryAll();
	AfxOleRegisterTypeLib( AfxGetInstanceHandle(), LIBID_Shareaza );

	return CWinApp::Register();
}

BOOL CShareazaApp::Unregister()
{
	CShareazaURL::Register( FALSE, TRUE );

	AfxOleUnregisterTypeLib( LIBID_Shareaza );
	AfxOleUnregisterTypeLib( LIBID_Shareaza );
	AfxOleUnregisterTypeLib( LIBID_Shareaza );
	COleObjectFactory::UpdateRegistryAll( FALSE );
	COleObjectFactory::UpdateRegistryAll( FALSE );
	COleObjectFactory::UpdateRegistryAll( FALSE );

	return TRUE; // Don't call CWinApp::Unregister(), it removes Shareaza settings
}

CDocument* CShareazaApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	if ( lpszFileName )
		Open( lpszFileName, TRUE );
	return NULL;
}

BOOL CShareazaApp::Open(LPCTSTR lpszFileName, BOOL bDoIt)
{
	size_t nLength = _tcslen( lpszFileName );
	if (      nLength > 8  && ! _tcsicmp( lpszFileName + nLength - 8,  _T(".torrent") ) )
		return OpenTorrent( lpszFileName, bDoIt );
	else if ( nLength > 3  && ! _tcsicmp( lpszFileName + nLength - 3,  _T(".co") ) )
		return OpenCollection( lpszFileName, bDoIt );
	else if ( nLength > 11 && ! _tcsicmp( lpszFileName + nLength - 11, _T(".collection") ) )
		return OpenCollection( lpszFileName, bDoIt );
	else if ( nLength > 16 && ! _tcsicmp( lpszFileName + nLength - 16, _T(".emulecollection") ) )
		return OpenCollection( lpszFileName, bDoIt );
	else if ( nLength > 15 && ! _tcsicmp( lpszFileName + nLength - 15, _T("hublist.xml.bz2") ) )
		return OpenImport( lpszFileName, bDoIt );
	else if ( nLength > 8  && ! _tcsicmp( lpszFileName + nLength - 8,  _T(".xml.bz2") ) )
		return OpenCollection( lpszFileName, bDoIt );
	else if ( nLength > 4  && ! _tcsicmp( lpszFileName + nLength - 4,  _T(".url") ) )
		return OpenInternetShortcut( lpszFileName, bDoIt );
	else if ( nLength > 4  && ! _tcsicmp( lpszFileName + nLength - 4,  _T(".met") ) )
		return OpenImport( lpszFileName, bDoIt );
	else if ( nLength > 4  && ! _tcsicmp( lpszFileName + nLength - 4,  _T(".dat") ) )
		return OpenImport( lpszFileName, bDoIt );
	else if ( nLength > 4  && ! _tcsicmp( lpszFileName + nLength - 4,  _T(".lnk") ) )
		return OpenShellShortcut( lpszFileName, bDoIt );
	else
		return OpenURL( lpszFileName, bDoIt );
}

BOOL CShareazaApp::OpenImport(LPCTSTR lpszFileName, BOOL bDoIt)
{
	if ( ! bDoIt )
		return TRUE;

	const size_t nLen = _tcslen( lpszFileName ) + 1;
	auto_array< TCHAR > pszPath( new TCHAR[ nLen ] );
	if ( pszPath.get() )
	{
		_tcscpy_s( pszPath.get(), nLen, lpszFileName );
		if ( PostMainWndMessage( WM_IMPORT, (WPARAM)pszPath.release() ) )
			return TRUE;
	}

	return FALSE;
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
	if ( ! bDoIt )
		return TRUE;

	// Open torrent
	const size_t nLen = _tcslen( lpszFileName ) + 1;
	auto_array< TCHAR > pszPath( new TCHAR[ nLen ] );
	if ( pszPath.get() )
	{
		_tcscpy_s( pszPath.get(), nLen, lpszFileName );
		if ( PostMainWndMessage( WM_TORRENT, (WPARAM)pszPath.release() ) )
			return TRUE;
	}

	return FALSE;
}

BOOL CShareazaApp::OpenCollection(LPCTSTR lpszFileName, BOOL bDoIt)
{
	if ( ! bDoIt )
		return TRUE;

	const size_t nLen = _tcslen( lpszFileName ) + 1;
	auto_array< TCHAR > pszPath( new TCHAR[ nLen ] );
	if ( pszPath.get() )
	{
		_tcscpy_s( pszPath.get(), nLen, lpszFileName );
		if ( PostMainWndMessage( WM_COLLECTION, (WPARAM)pszPath.release() ) )
			return TRUE;
	}

	return FALSE;
}

BOOL CShareazaApp::OpenURL(LPCTSTR lpszFileName, BOOL bDoIt, BOOL bSilent)
{
	if ( bDoIt && ! bSilent )
		theApp.Message( MSG_NOTICE, IDS_URL_RECEIVED, lpszFileName );

	auto_ptr< CShareazaURL > pURL( new CShareazaURL() );
	if ( pURL.get() )
	{
		if ( pURL->Parse( lpszFileName ) )
		{
			if ( bDoIt )
			{
				PostMainWndMessage( WM_URL, (WPARAM)pURL.release() );
			}
			return TRUE;
		}
	}

	if ( bDoIt && ! bSilent )
		theApp.Message( MSG_NOTICE, IDS_URL_PARSE_ERROR );

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp resources

void CShareazaApp::InitResources()
{
	// Set Build Date
	COleDateTime tCompileTime;
	tCompileTime.ParseDateTime( _T(__DATE__), LOCALE_NOUSEROVERRIDE, 1033 );
	m_sBuildDate = tCompileTime.Format( _T("%Y%m%d") );

	// Get .exe-file name
	GetModuleFileName( NULL, m_strBinaryPath.GetBuffer( MAX_PATH ), MAX_PATH );
	m_strBinaryPath.ReleaseBuffer( MAX_PATH );

	// Load version from .exe-file properties
	m_nVersion[0] = m_nVersion[1] = m_nVersion[2] = m_nVersion[3] = 0;
	DWORD dwSize;
	dwSize = GetFileVersionInfoSize( m_strBinaryPath, &dwSize );
	if ( dwSize )
	{
		if ( BYTE* pBuffer = new BYTE[ dwSize ] )
		{
			if ( GetFileVersionInfo( m_strBinaryPath, NULL, dwSize, pBuffer ) )
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

	m_sVersionLong = m_sVersion + 
#ifdef _DEBUG
		_T(" Debug")
#else
		_T(" Release")
#endif
#ifdef _WIN64
		_T(" 64-bit")
#else
		_T(" 32-bit")
#endif
#ifdef LAN_MODE
		_T(" LAN")
#endif		
		_T(" (r") _T(__REVISION__) _T(" ") + m_sBuildDate + _T(")");

	BT_SetAppVersion( m_sVersionLong );

	m_sSmartAgent = _T( CLIENT_NAME ) _T(" ");
	m_sSmartAgent += m_sVersion;

	m_pBTVersion[ 0 ] = BT_ID1;
	m_pBTVersion[ 1 ] = BT_ID2;
	m_pBTVersion[ 2 ] = (BYTE)m_nVersion[ 0 ];
	m_pBTVersion[ 3 ] = (BYTE)m_nVersion[ 1 ];

	// Determine the version of Windows
	OSVERSIONINFOEX pVersion = { sizeof( OSVERSIONINFOEX ) };
	GetVersionEx( (OSVERSIONINFO*)&pVersion );
	GetSystemInfo( &m_SysInfo );

	// Determine if it's a server
	m_bIsServer = pVersion.wProductType != VER_NT_WORKSTATION;

	// Get Major version
	// Windows 2000 (Major Version == 5) does not support some functions
	m_nWindowsVersion = pVersion.dwMajorVersion;

	// Get Minor version
	//	Major version == 5
	//		Win2000 = 0, WinXP = 1, WinXP64 = 2, Server2003 = 2
	//	Major version == 6
	//		Vista = 0, Server2008 = 0, Windows7 = 1
	m_nWindowsVersionMinor = pVersion.dwMinorVersion;

	// Most supported windows versions have network limiting
	m_bLimitedConnections = true;
	TCHAR* sp = _tcsstr( pVersion.szCSDVersion, _T("Service Pack") );

	// Set some variables for different Windows OSes
	if ( m_nWindowsVersion == 5 )
	{
		if ( m_nWindowsVersionMinor == 0 )		// Windows 2000
			m_bIsWin2000 = true;
		else if ( m_nWindowsVersionMinor == 1 )	// Windows XP
		{
			// 10 Half-open concurrent TCP connections limit was introduced in SP2
			// Windows Server will never compare this value though.
			if ( !sp || sp[ 13 ] == '1' )
				m_bLimitedConnections = false;
		}
		else if ( m_nWindowsVersionMinor == 2 )	// Windows 2003 or Windows XP64
		{
			if ( !sp )
				m_bLimitedConnections = false;
		}
	}
	else if ( m_nWindowsVersion >= 6 ) 
	{
		// Used for GUI improvement
		m_bIsVistaOrNewer = true;
		bool bCanBeRegistryPatched = true;

		if ( m_nWindowsVersionMinor == 0 )
		{
			if ( !sp || sp[ 13 ] == '1' )
			{
				// Has TCP connections limit
				bCanBeRegistryPatched = false;
			} 
			else
			{
				m_bLimitedConnections = false;
			}
		}

		// Server versions can be limited too
		if ( bCanBeRegistryPatched ) 
		{
			HKEY hKey;
			if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
							   L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", 
							   0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
			{
				DWORD nSize = sizeof( DWORD ), nResult = 0, nType = REG_NONE;
				if ( ( RegQueryValueEx( hKey, L"EnableConnectionRateLimiting", 
									    NULL, &nType, (LPBYTE)&nResult, &nSize ) == ERROR_SUCCESS ) &&
					 nType == REG_DWORD && nResult == 1 )
					m_bLimitedConnections = true;
				RegCloseKey( hKey );
				if ( nResult == 1 )
					return; // Don't check if that was a server
			}
		}
	}

	// Windows Small Business Server allows 74 simultaneous
	// connections and any full Server version allows unlimited connections
	if ( pVersion.wProductType == VER_NT_SERVER && 
		( pVersion.wSuiteMask & VER_SUITE_SMALLBUSINESS ) == 0 )
		m_bLimitedConnections = false;

	// Get pointers to some functions that require Windows Vista or greater
	if ( HMODULE hKernel32 = GetModuleHandle( _T("kernel32.dll") ) )
	{
		(FARPROC&)m_pRegisterApplicationRestart = GetProcAddress( hKernel32, "RegisterApplicationRestart" );
	}

	// Get pointers to some functions that require Windows XP or greater
	if ( ( m_hTheme = LoadLibrary( _T("UxTheme.dll") ) ) != NULL )
	{
		(FARPROC&)m_pfnSetWindowTheme = GetProcAddress( m_hTheme, "SetWindowTheme" );
		(FARPROC&)m_pfnIsThemeActive = GetProcAddress( m_hTheme, "IsThemeActive" );
		(FARPROC&)m_pfnOpenThemeData = GetProcAddress( m_hTheme, "OpenThemeData" );
		(FARPROC&)m_pfnCloseThemeData = GetProcAddress( m_hTheme, "CloseThemeData" );
		(FARPROC&)m_pfnDrawThemeBackground = GetProcAddress( m_hTheme, "DrawThemeBackground" );
		(FARPROC&)m_pfnGetThemeSysFont = GetProcAddress( m_hTheme, "GetThemeSysFont" );
	}

	// Get pointers to some functions that require Internet Explorer 6.01 or greater
	if ( ( m_hShlWapi = LoadLibrary( _T("shlwapi.dll") ) ) != NULL )
	{
		(FARPROC&)m_pfnAssocIsDangerous = GetProcAddress( m_hShlWapi, "AssocIsDangerous" );
	}

	if ( ( m_hShell32 = LoadLibrary( _T("shell32.dll") ) ) != NULL )
	{
		(FARPROC&)m_pfnSHGetFolderPathW = GetProcAddress( m_hShell32, "SHGetFolderPathW" );
		(FARPROC&)m_pfnSHGetKnownFolderPath = GetProcAddress( m_hShell32, "SHGetKnownFolderPath" );
	}

	LoadCountry();

	// We load it in a custom way, so Shareaza plugins can use this library also when it isn't in its search path but loaded by CustomLoadLibrary (very useful when running Shareaza inside Visual Studio)
	m_hLibGFL = CustomLoadLibrary( _T("libgfl290.dll") );

	CryptAcquireContext( &m_hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT );

	srand( GetTickCount() );

	m_hHookKbd   = SetWindowsHookEx( WH_KEYBOARD, (HOOKPROC)KbdHook, NULL, AfxGetThread()->m_nThreadID );
	m_hHookMouse = SetWindowsHookEx( WH_MOUSE, (HOOKPROC)MouseHook, NULL, AfxGetThread()->m_nThreadID );
	m_nLastInput = (DWORD)time( NULL );
}

void CShareazaApp::InitFonts()
{
	// Set up the default fonts
	BOOL bFontSmoothing = FALSE;
	UINT nSmoothingType = 0;
	if ( SystemParametersInfo( SPI_GETFONTSMOOTHING, 0, &bFontSmoothing, 0 ) &&
		 bFontSmoothing &&
		 SystemParametersInfo( SPI_GETFONTSMOOTHINGTYPE, 0, &nSmoothingType, 0 ) )
	{
		m_nFontQuality = ( nSmoothingType == FE_FONTSMOOTHINGSTANDARD ) ?
			ANTIALIASED_QUALITY : ( ( nSmoothingType == FE_FONTSMOOTHINGCLEARTYPE ) ?
			CLEARTYPE_QUALITY : DEFAULT_QUALITY );
	}
	if ( Settings.Fonts.DefaultFont.IsEmpty() )
	{
		// Get font from current theme
		if ( m_pfnGetThemeSysFont )
		{
			LOGFONT pFont = {};
			if ( m_pfnGetThemeSysFont( NULL, TMT_MENUFONT, &pFont ) == S_OK )
			{
				Settings.Fonts.DefaultFont = pFont.lfFaceName;
			}
		}
	}
	if ( Settings.Fonts.DefaultFont.IsEmpty() )
	{
		// Get font by legacy method
		NONCLIENTMETRICS pMetrics = { sizeof( NONCLIENTMETRICS ) };
		SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof( NONCLIENTMETRICS ),
			&pMetrics, 0 );
		Settings.Fonts.DefaultFont = pMetrics.lfMenuFont.lfFaceName;
	}
	if ( Settings.Fonts.SystemLogFont.IsEmpty() )
	{
		Settings.Fonts.SystemLogFont = Settings.Fonts.DefaultFont;
	}
	if ( Settings.Fonts.PacketDumpFont.IsEmpty() )
	{
		Settings.Fonts.PacketDumpFont = _T("Lucida Console");
	}

	m_gdiFont.CreateFont( -(int)Settings.Fonts.FontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );

	m_gdiFontBold.CreateFont( -(int)Settings.Fonts.FontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );

	m_gdiFontLine.CreateFont( -(int)Settings.Fonts.FontSize, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );
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
	if ( m_pSafeWnd == NULL ) return NULL;
	ASSERT_KINDOF( CMainWnd, m_pSafeWnd );
	return static_cast< CMainWnd* >( IsWindow( m_pSafeWnd->m_hWnd ) ? m_pSafeWnd : NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp message

bool CShareazaApp::IsLogDisabled(WORD nType) const
{
	return
		// Severity filter
		( static_cast< DWORD >( nType & MSG_SEVERITY_MASK ) > Settings.General.LogLevel ) ||
		// Facility filter
		( ( nType & MSG_FACILITY_MASK ) == MSG_FACILITY_SEARCH && ! Settings.General.SearchLog );
}

void CShareazaApp::ShowStartupText()
{
	CString strBody;
	LoadString( strBody, IDS_SYSTEM_MESSAGE );

	strBody.Replace( _T("(version)"),
		(LPCTSTR)(theApp.m_sVersion + _T(" (") + theApp.m_sBuildDate + _T(")")) );

	for ( strBody += '\n' ; strBody.GetLength() ; )
	{
		CString strLine = strBody.SpanExcluding( _T("\r\n") );
		strBody = strBody.Mid( strLine.GetLength() + 1 );

		strLine.TrimLeft();
		strLine.TrimRight();
		if ( strLine.IsEmpty() ) continue;

		if ( strLine == _T(".") ) strLine.Empty();

		if ( _tcsnicmp( strLine, _T("!"), 1 ) == 0 )
			PrintMessage( MSG_NOTICE, (LPCTSTR)strLine + 1 );
		else
			PrintMessage( MSG_INFO, strLine );
	}

	CString strCPU;
	strCPU.Format( _T("\n%u x CPU. Features:"), m_SysInfo.dwNumberOfProcessors );
	if ( Machine::SupportsMMX() )
		strCPU += _T(" MMX");
	if ( Machine::SupportsSSE() )
		strCPU += _T(" SSE");
	if ( Machine::SupportsSSE2() )
		strCPU += _T(" SSE2");
	if ( Machine::SupportsSSE3() )
		strCPU += _T(" SSE3");
	if ( Machine::SupportsSSSE3() )
		strCPU += _T(" SSSE3");
	if ( Machine::SupportsSSE41() )
		strCPU += _T(" SSE4.1");
	if ( Machine::SupportsSSE42() )
		strCPU += _T(" SSE4.2");
	if ( Machine::SupportsSSE4A() )
		strCPU += _T(" SSE4A");
	if ( Machine::SupportsSSE5() )
		strCPU += _T(" SSE5");
	if ( Machine::Supports3DNOW() )
		strCPU += _T(" 3DNow");
	if ( Machine::Supports3DNOWEXT() )
		strCPU += _T(" 3DNowExt");
	PrintMessage( MSG_INFO, strCPU );
}

void CShareazaApp::SplashAbort()
{
	if ( m_dlgSplash )
	{
		m_dlgSplash->Hide( TRUE );
		m_dlgSplash = NULL;
	}
}

void CShareazaApp::SplashStep(LPCTSTR pszMessage, int nMax, bool bClosing)
{
	if ( ! pszMessage )
	{
		if ( m_dlgSplash )
		{
			m_dlgSplash->Hide( FALSE );
			m_dlgSplash = NULL;
		}
	}
	else if ( ! m_dlgSplash && nMax )
	{
		m_dlgSplash = new CSplashDlg( nMax, bClosing );
		m_dlgSplash->Step( pszMessage );
	}
	else if ( m_dlgSplash && ! nMax )
		m_dlgSplash->Step( pszMessage );

	TRACE( _T("Step: %s\n"), pszMessage ? pszMessage : _T("Done") );
}

void CShareazaApp::Message(WORD nType, UINT nID, ...)
{
	// Check if logging this type of message is enabled
	if ( IsLogDisabled( nType ) )
		return;

	// Load the format string from the resource file
	CString strFormat;
	LoadString( strFormat, nID );

	// Initialize variable arguments list
	va_list pArgs;
	va_start( pArgs, nID );

	// Work out the type of format string and call the appropriate function
	CString strTemp;
	if ( strFormat.Find( _T("%1") ) >= 0 )
		strTemp.FormatMessageV( strFormat, &pArgs );
	else
		strTemp.FormatV( strFormat, pArgs );

	// Print the message if there still is one
	if ( strTemp.GetLength() )
		PrintMessage( nType, strTemp );

	// Null the argument list pointer
	va_end( pArgs );

	return;
}

void CShareazaApp::Message(WORD nType, LPCTSTR pszFormat, ...)
{
	// Check if logging this type of message is enabled
	if ( IsLogDisabled( nType ) )
		return;

	// Initialize variable arguments list
	va_list pArgs;
	va_start( pArgs, pszFormat );

	// Format the message
	CString strTemp;
	strTemp.FormatV( pszFormat, pArgs );

	// Print the message if there still is one
	if ( strTemp.GetLength() )
		PrintMessage( nType, strTemp );

	// Null the argument list pointer
	va_end( pArgs );

	return;
}

void CShareazaApp::PrintMessage(WORD nType, const CString& strLog)
{
	/*if ( Settings.General.DebugLog )
	{
		// Default: "%APPDATA%\Shareaza\Shareaza.txt"
		if ( INT_PTR nFile = BT_OpenLogFile( NULL, BTLF_STREAM ) )
		{
			if ( Settings.General.MaxDebugLogSize )
				VERIFY( BT_SetLogSizeInBytes( nFile, Settings.General.MaxDebugLogSize ) );
			VERIFY( BT_SetLogLevel( nFile, BTLL_VERBOSE ) );
			VERIFY( BT_SetLogFlags( nFile, BTLF_SHOWLOGLEVEL |
				( Settings.General.ShowTimestamp ? BTLF_SHOWTIMESTAMP : 0 ) ) );
			VERIFY( BT_AppLogEntry( nFile,
				(BUGTRAP_LOGLEVEL)( nType & MSG_SEVERITY_MASK + 1 ), strLog ) );
			VERIFY( BT_CloseLogFile( nFile ) );
		}
	}*/

	CQuickLock pLock( m_csMessage );

	// Max 1000 lines
	if ( m_oMessages.GetCount() >= 1000 )
		delete m_oMessages.RemoveHead();

	m_oMessages.AddTail( new CLogMessage( nType, strLog ) );

	if ( ! Settings.General.DebugLog )
		return;

	CFile pFile;
	if ( pFile.Open( Settings.General.UserPath + _T("\\Data\\") CLIENT_NAME_T _T(".log"), CFile::modeReadWrite ) )
	{
		pFile.Seek( 0, CFile::end ); // go to the end of the file to add entires.

		if ( ( Settings.General.MaxDebugLogSize ) &&					// If log rotation is on
			( pFile.GetLength() > Settings.General.MaxDebugLogSize ) )	// and file is too long...
		{
			// Close the file
			pFile.Close();

			// Rotate the logs
			MoveFileEx( CString( _T("\\\\?\\") ) + Settings.General.UserPath + _T("\\Data\\") CLIENT_NAME_T _T(".log"),
				CString( _T("\\\\?\\") ) + Settings.General.UserPath + _T("\\Data\\") CLIENT_NAME_T _T(".old.log"),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH );
			// Start a new log
			if ( ! pFile.Open( Settings.General.UserPath + _T("\\Data\\") CLIENT_NAME_T _T(".log"),
				CFile::modeWrite|CFile::modeCreate ) ) return;
			// Unicode marker
			WORD nByteOrder = 0xFEFF;
			pFile.Write( &nByteOrder, 2 );
		}
	}
	else
	{
		if ( ! pFile.Open( Settings.General.UserPath + _T("\\Data\\") CLIENT_NAME_T _T(".log"),
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

	pFile.Write( (LPCTSTR)strLog, static_cast< UINT >( sizeof(TCHAR) * strLog.GetLength() ) );
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

void ReportError(DWORD dwError)
{
	CString sError = GetErrorString( dwError );
	theApp.Message( MSG_ERROR, _T("%s"), sError );
	AfxMessageBox( sError, MB_OK | MB_ICONEXCLAMATION );
}

CString CShareazaApp::GetCountryCode(IN_ADDR pAddress) const
{
	if ( m_pfnGeoIP_country_code_by_ipnum && m_pGeoIP )
		return CString( m_pfnGeoIP_country_code_by_ipnum( m_pGeoIP, htonl( pAddress.s_addr ) ) );
	return _T("");
}

CString CShareazaApp::GetCountryName(IN_ADDR pAddress) const
{
	if ( m_pfnGeoIP_country_name_by_ipnum && m_pGeoIP )
		return CString( m_pfnGeoIP_country_name_by_ipnum( m_pGeoIP, htonl( pAddress.s_addr ) ) );
	return _T("");
}

void CShareazaApp::LoadCountry()
{
	if ( ( m_hGeoIP = CustomLoadLibrary( _T("geoip.dll") ) ) != NULL )
	{
		GeoIP_newFunc pfnGeoIP_new = (GeoIP_newFunc)GetProcAddress( m_hGeoIP, "GeoIP_new" );
		m_pfnGeoIP_delete = (GeoIP_deleteFunc)GetProcAddress( m_hGeoIP, "GeoIP_delete" );
		m_pfnGeoIP_country_code_by_ipnum = (GeoIP_country_code_by_ipnumFunc)GetProcAddress( m_hGeoIP, "GeoIP_country_code_by_ipnum" );
		m_pfnGeoIP_country_name_by_ipnum = (GeoIP_country_name_by_ipnumFunc)GetProcAddress( m_hGeoIP, "GeoIP_country_name_by_ipnum" );
		if ( pfnGeoIP_new )
			m_pGeoIP = pfnGeoIP_new( GEOIP_MEMORY_CACHE );
	}
}

void CShareazaApp::FreeCountry()
{
	if ( m_hGeoIP != NULL )
	{
		if ( m_pGeoIP && m_pfnGeoIP_delete )
		{
			__try
			{
				m_pfnGeoIP_delete( m_pGeoIP );
			}
			__except( EXCEPTION_EXECUTE_HANDLER )
			{
			}
			m_pGeoIP = NULL;
		}
		FreeLibrary( m_hGeoIP );
		m_hGeoIP = NULL;
	}
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
	else if ( strURI.Find( _T("raza:launch:") ) == 0 )
	{
		DWORD nIndex = 0;
		_stscanf( (LPCTSTR)strURI + 12, _T("%lu"), &nIndex );

		CSingleLock oLock( &Library.m_pSection, TRUE );
		if ( CLibraryFile* pFile = Library.LookupFile( nIndex ) )
		{
			if ( pFile->IsAvailable() )
			{
				CString strPath = pFile->GetPath();
				oLock.Unlock();
				CFileExecutor::Execute( strPath );
			}
		}
	}
	else if (	strURI.Find( _T("http://") ) == 0 ||
				strURI.Find( _T("https://") ) == 0 ||
				strURI.Find( _T("ftp://") ) == 0 ||
				strURI.Find( _T("mailto:") ) == 0 ||
				strURI.Find( _T("aim:") ) == 0 ||
				strURI.Find( _T("magnet:") ) == 0 ||
				strURI.Find( _T("foxy:") ) == 0 ||
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
				strURI.Find( _T("dchub://") ) == 0 ||
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
		pMainWnd->PostMessage( WM_VERSIONCHECK, VC_CONFIRM );
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

BOOL IsRunAsAdmin()
{
	PSID pAdministratorsGroup = NULL;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if ( ! AllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup ) )
	{
		return FALSE;
	}

	BOOL bIsRunAsAdmin = FALSE;
	if ( ! CheckTokenMembership( NULL, pAdministratorsGroup, &bIsRunAsAdmin ) )
	{
		FreeSid( pAdministratorsGroup );
		return FALSE;
	}

	FreeSid( pAdministratorsGroup );
    return bIsRunAsAdmin;
}

/////////////////////////////////////////////////////////////////////////////
// Runtime class lookup

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

BOOL LoadString(CString& str, UINT nID)
{
	return Skin.LoadString( str, nID );
}

CString LoadString(UINT nID)
{
	CString str;
	LoadString( str, nID );
	return str;
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

	time_t tGMT = mktime( &pTime ), tSub;
	tm pGM = {};
	if ( tGMT == -1 ||
		gmtime_s( &pGM, &tGMT ) != 0 ||
		( tSub = mktime( &pGM ) ) == -1 )
	{
		return 0;
	}

	return DWORD( 2 * tGMT - tSub );
}

CString TimeToString(time_t tVal)
{
	tm time = {};
	CString str;
	if ( gmtime_s( &time, &tVal ) == 0 )
	{
		str.Format( _T("%.4i-%.2i-%.2iT%.2i:%.2iZ"),
			time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
			time.tm_hour, time.tm_min );
	}
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

BOOL LoadIcon(LPCTSTR szFilename, HICON* phSmallIcon, HICON* phLargeIcon, HICON* phHugeIcon, int nIcon)
{
	CString strIcon( szFilename );

	if ( phSmallIcon )
		*phSmallIcon = NULL;
	if ( phLargeIcon )
		*phLargeIcon = NULL;
	if ( phHugeIcon )
		*phHugeIcon = NULL;

	int nIndex = strIcon.ReverseFind( _T(',') );
	if ( nIndex != -1 )
	{
		if ( _stscanf( strIcon.Mid( nIndex + 1 ), _T("%i"), &nIcon ) == 1 )
		{
			strIcon = strIcon.Left( nIndex );
		}
	}
	else
		nIndex = 0;

	if ( strIcon.GetLength() < 3 )
		return FALSE;

	if ( strIcon.GetAt( 0 ) == _T('\"') &&
		 strIcon.GetAt( strIcon.GetLength() - 1 ) == _T('\"') )
		strIcon = strIcon.Mid( 1, strIcon.GetLength() - 2 );

	if ( phLargeIcon || phSmallIcon )
	{
		ExtractIconEx( strIcon, nIcon, phLargeIcon, phSmallIcon, 1 );
	}

	if ( phHugeIcon )
	{
		UINT nLoadedID;
		PrivateExtractIcons( strIcon, nIcon, 48, 48,
			phHugeIcon, &nLoadedID, 1, 0 );
	}

	return ( phLargeIcon && *phLargeIcon ) || ( phSmallIcon && *phSmallIcon ) ||
		( phHugeIcon && *phHugeIcon );
}

HICON LoadCLSIDIcon(LPCTSTR szCLSID)
{
	CString strPath;
	HKEY hKey;
	strPath.Format( _T("CLSID\\%s\\InProcServer32"), szCLSID );
	if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, strPath, 0, KEY_READ, &hKey ) != ERROR_SUCCESS )
	{
		strPath.Format( _T("CLSID\\%s\\LocalServer32"), szCLSID );
		if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, strPath, 0, KEY_READ, &hKey ) != ERROR_SUCCESS )
			return NULL;
	}

	DWORD dwType = REG_SZ, dwSize = MAX_PATH * sizeof( TCHAR );
	LONG lResult = RegQueryValueEx( hKey, _T(""), NULL, &dwType,
		(LPBYTE)strPath.GetBuffer( MAX_PATH ), &dwSize );
	strPath.ReleaseBuffer( dwSize / sizeof(TCHAR) );
	RegCloseKey( hKey );

	if ( lResult != ERROR_SUCCESS )
		return NULL;

	strPath.Trim( _T(" \"") );

	HICON hSmallIcon;
	if ( ! LoadIcon( strPath, &hSmallIcon, NULL, NULL ) )
		return NULL;

	return hSmallIcon;
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
			SetLayout( hdcBitmap, LAYOUT_RTL );
			SetLayout( hdcMask, LAYOUT_RTL );
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
		SetLayout( hdcMem2, LAYOUT_RTL );
		BitBlt( hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem1, 0, 0, SRCCOPY );
		SelectObject( hdcMem1, hOld_bm1 );
		SelectObject( hdcMem2, hOld_bm2 );
		DeleteDC( hdcMem1 );
		DeleteDC( hdcMem2 );
		ReleaseDC( NULL, hdc );
	}
	return hbm;
}

/////////////////////////////////////////////////////////////////////////////
// Keyboard hook: record tick count

LRESULT CALLBACK KbdHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if ( nCode == HC_ACTION )
	{
		theApp.m_nLastInput = (DWORD)time( NULL );

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
		theApp.m_nLastInput = (DWORD)time( NULL );

	return ::CallNextHookEx( theApp.m_hHookMouse, nCode, wParam, lParam );
}

CString CShareazaApp::GetWindowsFolder() const
{
	HRESULT hr;
	CString sWindows;

	// Vista way
	if ( m_pfnSHGetKnownFolderPath )
	{
		PWSTR pPath = NULL;
		hr = m_pfnSHGetKnownFolderPath( FOLDERID_Windows,
			KF_FLAG_CREATE | KF_FLAG_INIT, NULL, &pPath );
		if ( pPath )
		{
			sWindows = pPath;
			CoTaskMemFree( pPath );
		}
		if ( SUCCEEDED( hr ) && ! sWindows.IsEmpty() )
		{
			return sWindows;
		}
	}

	// Win2K/XP way
	if ( m_pfnSHGetFolderPathW )
	{
		hr = m_pfnSHGetFolderPathW( NULL, CSIDL_WINDOWS, NULL, NULL,
			sWindows.GetBuffer( MAX_PATH ) );
		sWindows.ReleaseBuffer();
		if ( SUCCEEDED( hr  ) && ! sWindows.IsEmpty() )
		{
			return sWindows;
		}
	}

	// Legacy way
	GetWindowsDirectory( sWindows.GetBuffer( MAX_PATH ), MAX_PATH );
	sWindows.ReleaseBuffer();
	return sWindows;
}

CString CShareazaApp::GetProgramFilesFolder() const
{
	HRESULT hr;
	CString sProgramFiles;

	// Vista way
	if ( m_pfnSHGetKnownFolderPath )
	{
		PWSTR pPath = NULL;
		hr = m_pfnSHGetKnownFolderPath( FOLDERID_ProgramFiles,
			KF_FLAG_CREATE | KF_FLAG_INIT, NULL, &pPath );
		if ( pPath )
		{
			sProgramFiles = pPath;
			CoTaskMemFree( pPath );
		}
		if ( SUCCEEDED( hr ) && ! sProgramFiles.IsEmpty() )
		{
			return sProgramFiles;
		}
	}

	// Win2K/XP way
	if ( m_pfnSHGetFolderPathW )
	{
		hr = m_pfnSHGetFolderPathW( NULL, CSIDL_PROGRAM_FILES, NULL, NULL,
			sProgramFiles.GetBuffer( MAX_PATH ) );
		sProgramFiles.ReleaseBuffer();
		if ( SUCCEEDED( hr  ) && ! sProgramFiles.IsEmpty() )
		{
			return sProgramFiles;
		}
	}

	// Legacy way
	sProgramFiles = GetWindowsFolder().Left( 1 ) + _T(":\\Program Files");

	return sProgramFiles;
}

CString CShareazaApp::GetDocumentsFolder() const
{
	HRESULT hr;
	CString sDocuments;

	// Vista way
	if ( m_pfnSHGetKnownFolderPath )
	{
		PWSTR pPath = NULL;
		hr = m_pfnSHGetKnownFolderPath( FOLDERID_Documents,
			KF_FLAG_CREATE | KF_FLAG_INIT, NULL, &pPath );
		if ( pPath )
		{
			sDocuments = pPath;
			CoTaskMemFree( pPath );
		}
		if ( SUCCEEDED( hr ) && ! sDocuments.IsEmpty() )
		{
			return sDocuments;
		}
	}

	// Win2K/XP way
	if ( m_pfnSHGetFolderPathW )
	{
		hr = m_pfnSHGetFolderPathW( NULL, CSIDL_PERSONAL, NULL, NULL,
			sDocuments.GetBuffer( MAX_PATH ) );
		sDocuments.ReleaseBuffer();
		if ( SUCCEEDED( hr  ) && ! sDocuments.IsEmpty() )
		{
			return sDocuments;
		}
	}

	// Legacy way
	sDocuments = CRegistry::GetString( _T("Shell Folders"), _T("Personal"),
		_T(""), _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer") );

	return sDocuments;
}

CString CShareazaApp::GetDownloadsFolder() const
{
	HRESULT hr;
	CString sDownloads;

	// Vista way
	if ( m_pfnSHGetKnownFolderPath )
	{
		PWSTR pPath = NULL;
		hr = m_pfnSHGetKnownFolderPath( FOLDERID_Downloads,
			KF_FLAG_CREATE | KF_FLAG_INIT, NULL, &pPath );
		if ( pPath )
		{
			sDownloads = pPath;
			CoTaskMemFree( pPath );
		}
		if ( SUCCEEDED( hr ) && ! sDownloads.IsEmpty() )
		{
			return sDownloads;
		}
	}

	// Winx2K/XP or legacy way
	sDownloads = GetDocumentsFolder() + _T("\\") CLIENT_NAME_T _T(" Downloads");

	return sDownloads;
}

CString CShareazaApp::GetAppDataFolder() const
{
	HRESULT hr;
	CString sAppData;

	// Vista way
	if ( m_pfnSHGetKnownFolderPath )
	{
		PWSTR pPath = NULL;
		hr = m_pfnSHGetKnownFolderPath( FOLDERID_RoamingAppData,
			KF_FLAG_CREATE | KF_FLAG_INIT, NULL, &pPath );
		if ( pPath )
		{
			sAppData = pPath;
			CoTaskMemFree( pPath );
		}
		if ( SUCCEEDED( hr ) && ! sAppData.IsEmpty() )
		{
			return sAppData;
		}
	}

	// Win2K/XP way
	if ( m_pfnSHGetFolderPathW )
	{
		hr = m_pfnSHGetFolderPathW( NULL, CSIDL_APPDATA, NULL, NULL,
			sAppData.GetBuffer( MAX_PATH ) );
		sAppData.ReleaseBuffer();
		if ( SUCCEEDED( hr ) && ! sAppData.IsEmpty() )
		{
			return sAppData;
		}
	}

	// Legacy way
	sAppData = CRegistry::GetString( _T("Shell Folders"), _T("AppData"),
		_T(""), _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer") );

	return sAppData;
}

CString CShareazaApp::GetLocalAppDataFolder() const
{
	HRESULT hr;
	CString sLocalAppData;

	// Vista way
	if ( m_pfnSHGetKnownFolderPath )
	{
		PWSTR pPath = NULL;
		hr = m_pfnSHGetKnownFolderPath( FOLDERID_LocalAppData,
			KF_FLAG_CREATE | KF_FLAG_INIT, NULL, &pPath );
		if ( pPath )
		{
			sLocalAppData = pPath;
			CoTaskMemFree( pPath );
		}
		if ( SUCCEEDED( hr ) && ! sLocalAppData.IsEmpty() )
		{
			return sLocalAppData;
		}
	}

	// Win2K/XP way
	if ( m_pfnSHGetFolderPathW )
	{
		hr = m_pfnSHGetFolderPathW( NULL, CSIDL_LOCAL_APPDATA, NULL, NULL,
			sLocalAppData.GetBuffer( MAX_PATH ) );
		sLocalAppData.ReleaseBuffer();
		if ( SUCCEEDED( hr ) && ! sLocalAppData.IsEmpty() )
		{
			return sLocalAppData;
		}
	}

	// Legacy way
	sLocalAppData = CRegistry::GetString( _T("Shell Folders"), _T("Local AppData"),
		_T(""), _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer") );
	if ( ! sLocalAppData.IsEmpty() )
	{
		return sLocalAppData;
	}

	// Prehistoric way
	return GetAppDataFolder();
}

void CShareazaApp::OnRename(LPCTSTR pszSource, LPCTSTR pszTarget)
{
	LibraryBuilder.Remove( pszSource );

	Uploads.OnRename( pszSource, pszTarget );

	Downloads.OnRename( pszSource, pszTarget );

	// Notify built-in MediaPlayer
	if ( pszTarget == NULL )
	{
		CQuickLock otheAppLock( theApp.m_pSection );

		if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
		{
			if ( CMediaWnd* pMediaWnd =
				(CMediaWnd*)pMainWnd->m_pWindows.Find( RUNTIME_CLASS( CMediaWnd ) ) )
			{
				pMediaWnd->OnFileDelete( pszSource );
			}
		}
	}
}

CDatabase* CShareazaApp::GetDatabase() const
{
	return new CDatabase( Settings.General.UserPath + _T("\\Data\\Shareaza.db3") );
}

CString SafeFilename(CString strName, bool bPath)
{
	// Restore spaces
	strName.Replace( _T("%20"), _T(" ") );

	// Replace incompatible symbols
	for ( ;; )
	{
		int nChar = strName.FindOneOf(
			bPath ? _T("/:*?\"<>|") : _T("\\/:*?\"<>|") );

		if ( nChar == -1 )
			break;

		strName.SetAt( nChar, _T('_') );
	}

	LPCTSTR szExt = PathFindExtension( strName );
	int nExtLen = lstrlen( szExt );

	// Limit maximum filepath length
	int nMaxFilenameLength = MAX_PATH - 1 - max( max(
		Settings.Downloads.IncompletePath.GetLength(),
		Settings.Downloads.CompletePath.GetLength() ),
		Settings.Downloads.TorrentPath.GetLength() );
	if ( strName.GetLength() > nMaxFilenameLength )
	{
		strName = strName.Left( nMaxFilenameLength - nExtLen ) + strName.Right( nExtLen );
	}

	return strName;
}

BOOL CreateDirectory(LPCTSTR szPath)
{
	CString strDir = szPath;

	DWORD dwAttr = ( strDir.GetLength() == 2 ) ?
		// Disk only
		GetFileAttributes( strDir + _T("\\") ) :
		// Normal directory
		GetFileAttributes( CString( _T("\\\\?\\") ) + strDir );
	if ( ( dwAttr != INVALID_FILE_ATTRIBUTES ) &&
		( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) )
		return TRUE;

	for ( int nStart = 3; ; )
	{
		int nSlash = strDir.Find( _T('\\'), nStart );
		if ( ( nSlash == -1 ) || ( nSlash == strDir.GetLength() - 1 ) )
			break;
		CString strSubDir( strDir.Left( nSlash + 1 ) );
		dwAttr = GetFileAttributes( CString( _T("\\\\?\\") ) + strSubDir );
		if ( ( dwAttr == INVALID_FILE_ATTRIBUTES ) ||
			! ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) )
			if ( ! CreateDirectory( CString( _T("\\\\?\\") ) + strSubDir, NULL ) )
				return FALSE;
		nStart = nSlash + 1;
	}
	return CreateDirectory( CString( _T("\\\\?\\") ) + szPath, NULL );
}
	
void DeleteFiles(CStringList& pList)
{
	while ( ! pList.IsEmpty() )
	{
		CString strFirstPath = pList.GetHead();

		CDeleteFileDlg dlg;
		dlg.m_bAll = ( pList.GetCount() > 1 );

		{
			CQuickLock pLibraryLock( Library.m_pSection );

			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( strFirstPath ) )
			{
				dlg.m_sName	= pFile->m_sName;
				dlg.m_sComments = pFile->m_sComments;
				dlg.m_nRateValue = pFile->m_nRating;
			}
			else
			{
				dlg.m_sName = PathFindFileName( strFirstPath );
			}
		}

		if ( dlg.DoModal() != IDOK )
			break;

		for ( INT_PTR nProcess = dlg.m_bAll ? pList.GetCount() : 1 ;
			nProcess > 0 && pList.GetCount() > 0 ; nProcess-- )
		{
			CString strPath = pList.RemoveHead();

			{
				CQuickLock pTransfersLock( Transfers.m_pSection ); // Can clear uploads and downloads
				CQuickLock pLibraryLock( Library.m_pSection );

				if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( strPath ) )
				{
					// It's library file
					dlg.Apply( pFile );
					pFile->Delete();
					continue;
				}
			}

			// It's wild file
			BOOL bToRecycleBin = ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 );
			DeleteFileEx( strPath, TRUE, bToRecycleBin, TRUE );
		}
	}
}

BOOL DeleteFileEx(LPCTSTR szFileName, BOOL bShared, BOOL bToRecycleBin, BOOL bEnableDelayed)
{
	// Should be double zeroed long path
	BOOL bLong;
	DWORD len = GetLongPathName( szFileName, NULL, 0 );
	if ( len )
	{
		bLong = TRUE;
	}
	else
	{
		len = lstrlen( szFileName );
		bLong = FALSE;
	}
	auto_array< TCHAR > szPath( new TCHAR[ len + 1 ] );
	if ( bLong )
	{
		GetLongPathName( szFileName, szPath.get(), len );
	}
	else
	{
		lstrcpy( szPath.get(), szFileName );
	}
	szPath[ len ] = 0;

	if ( bShared )
	{
		// Stop uploads
		theApp.OnRename( szPath.get(), NULL );
	}

	DWORD dwAttr = GetFileAttributes( szPath.get() );
	if ( ( dwAttr != INVALID_FILE_ATTRIBUTES ) &&		// Filename exist
		( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) == 0 )	// Not a folder
	{
		if ( bToRecycleBin )
		{
			SHFILEOPSTRUCT sfo = {};
			sfo.hwnd = GetDesktopWindow();
			sfo.wFunc = FO_DELETE;
			sfo.pFrom = szPath.get();
			sfo.fFlags = FOF_ALLOWUNDO | FOF_FILESONLY | FOF_NORECURSION | FOF_NO_UI;
			SHFileOperation( &sfo );
		}
		else
			DeleteFile( szPath.get() );

		dwAttr = GetFileAttributes( szPath.get() );
		if ( dwAttr != INVALID_FILE_ATTRIBUTES )
		{
			// File still exist
			if ( bEnableDelayed )
			{
				// Set delayed deletion
				CString sJob;
				sJob.Format( _T("%d%d"), bShared, bToRecycleBin );
				theApp.WriteProfileString( _T("Delete"), szPath.get(), sJob );
			}
			return FALSE;
		}
	}

	// Cancel delayed deletion (if any)
	theApp.WriteProfileString( _T("Delete"), szPath.get(), NULL );

	return TRUE;
}

void PurgeDeletes()
{
	HKEY hKey = NULL;
	LSTATUS nResult = RegOpenKeyEx( HKEY_CURRENT_USER,
		_T(REGISTRY_KEY) _T("\\Delete"), 0, KEY_ALL_ACCESS, &hKey );
	if ( ERROR_SUCCESS == nResult )
	{
		CList< CString > pRemove;
		for ( DWORD nIndex = 0 ; ; ++nIndex )
		{
			DWORD nPath = MAX_PATH * 2;
			TCHAR szPath[ MAX_PATH * 2 ] = {};
			DWORD nType;
			DWORD nMode = 8;
			TCHAR szMode[ 8 ] = {};
			nResult = RegEnumValue( hKey, nIndex, szPath, &nPath, 0,
				&nType, (LPBYTE)szMode, &nMode );
			if ( ERROR_SUCCESS != nResult )
				break;

			BOOL bShared = ( nType == REG_SZ ) && ( szMode[ 0 ] == '1' );
			BOOL bToRecycleBin = ( nType == REG_SZ ) && ( szMode[ 1 ] == '1' );
			if ( DeleteFileEx( szPath, bShared, bToRecycleBin, TRUE ) )
			{
				pRemove.AddTail( szPath );
			}
		}

		while ( ! pRemove.IsEmpty() )
		{
			nResult = RegDeleteValue( hKey, pRemove.RemoveHead() );
		}

		nResult = RegCloseKey( hKey );
	}
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

CString LoadRichHTML(UINT nResourceID, CString& strResponse, CShareazaFile* pFile)
{
	CString strBody = LoadHTML( GetModuleHandle( NULL ), nResourceID );

	bool bWindowsEOL = true;
	int nBreak = strBody.Find( _T("\r\n") );
	if ( nBreak == -1 )
	{
		nBreak = strBody.Find( _T("\n") );
		bWindowsEOL = false;
	}
	strResponse	= strBody.Left( nBreak + ( bWindowsEOL ? 2 : 1 ) );
	strBody = strBody.Mid( nBreak + ( bWindowsEOL ? 2 : 1 ) );

	for (;;)
	{
		int nStart = strBody.Find( _T("<%") );
		if ( nStart < 0 ) break;
		
		int nEnd = strBody.Find( _T("%>") );
		if ( nEnd < nStart ) break;

		CString strReplace = strBody.Mid( nStart + 2, nEnd - nStart - 2 );

		strReplace.TrimLeft();
		strReplace.TrimRight();
		
		if ( strReplace.CompareNoCase( _T("Client") ) == 0 )
			strReplace = CLIENT_NAME_T;
		else if ( strReplace.CompareNoCase( _T("SmartAgent") ) == 0 )
			strReplace = theApp.m_sSmartAgent;
		else if ( strReplace.CompareNoCase( _T("Name") ) == 0 )
			strReplace = pFile ? pFile->m_sName : _T("");
		else if ( strReplace.CompareNoCase( _T("SHA1") ) == 0 )
            strReplace = pFile ? pFile->m_oSHA1.toString() : _T("");
		else if ( strReplace.CompareNoCase( _T("URN") ) == 0 )
			strReplace = pFile ? pFile->m_oSHA1.toUrn() : _T("");
		else if ( strReplace.CompareNoCase( _T("Version") ) == 0 )
			strReplace = theApp.m_sVersion;
		else if ( strReplace.Find( _T("Neighbours") ) == 0 )
			strReplace = Neighbours.GetNeighbourList( strReplace.Right( strReplace.GetLength() - 11 ) );
		else if ( strReplace.CompareNoCase( _T("ListenIP") ) == 0 )
		{
			if ( Network.IsListening() )
			{
				strReplace.Format( _T("%s:%i"),
					(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
					htons( Network.m_pHost.sin_port ) );
			}
			else
				strReplace.Empty();
		}

		strBody = strBody.Left( nStart ) + strReplace + strBody.Mid( nEnd + 2 );
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
	{ _T("/style.css"),		IDR_HTML_STYLE,		RT_GZIP,		_T("text/css") },
	{ _T("/header_1.png"),	IDR_HOME_HEADER_1,	RT_PNG,			_T("image/png") },
	{ _T("/header_2.png"),	IDR_HOME_HEADER_2,	RT_PNG,			_T("image/png") },
	{ _T("/favicon.ico"),	IDR_MAINFRAME,		RT_GROUP_ICON,	_T("image/x-icon") },
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
						if ( WebResources[ i ].szType == RT_GZIP ||
							 WebResources[ i ].szType == RT_HTML )
						{
							pResponse.Print( LoadHTML( hModule, WebResources[ i ].nID ) );
						}
						else if ( WebResources[ i ].szType == RT_GROUP_ICON )
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
							for ( WORD j = 0; j < piDir->idCount; j++ )
							{
								// pResponse.m_pBuffer may be changed
								ICONDIRENTRY* piEntry = (ICONDIRENTRY*)
									( pResponse.m_pBuffer + sizeof( ICONDIR ) );

								// Load subicon
								HRSRC hResIcon = FindResource( hModule, MAKEINTRESOURCE(
									piDirEntry[ j ].nID ), RT_ICON );
								if ( hResIcon )
								{
									DWORD nSizeIcon = SizeofResource( hModule, hResIcon );
									HGLOBAL hMemoryIcon = LoadResource( hModule, hResIcon );
									if ( hMemoryIcon )
									{
										BITMAPINFOHEADER* piImage = (BITMAPINFOHEADER*)LockResource( hMemoryIcon );

										// Fill subicon header
										piEntry[ j ].bWidth = piDirEntry[ j ].bWidth;
										piEntry[ j ].bHeight = piDirEntry[ j ].bHeight;
										piEntry[ j ].wPlanes = piDirEntry[ j ].wPlanes;
										piEntry[ j ].bColorCount = piDirEntry[ j ].bColorCount;
										piEntry[ j ].bReserved = 0;
										piEntry[ j ].wBitCount = piDirEntry[ j ].wBitCount;
										piEntry[ j ].dwBytesInRes = nSizeIcon;
										piEntry[ j ].dwImageOffset = dwTotalSize;

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
	if ( !pszExt )
		return false;

	bool bSuccess = false;

	if ( Settings.Library.MarkFileAsDownload )
	{
		// TODO: pFile->m_bVerify and IDS_LIBRARY_VERIFY_FIX warning features could be merged
		// with this function, because they resemble the security warning.
		// Should raza unblock files from the application without forcing user to do that manually?
		if ( CFileExecutor::IsSafeExecute( pszExt ) )
			return false;

		// Temporary clear R/O attribute
		BOOL bChanged = FALSE;
		DWORD dwOrigAttr = GetFileAttributes( sFilename );
		if ( dwOrigAttr != INVALID_FILE_ATTRIBUTES && ( dwOrigAttr & FILE_ATTRIBUTE_READONLY ) )
			bChanged = SetFileAttributes( sFilename, dwOrigAttr & ~FILE_ATTRIBUTE_READONLY );

		HANDLE hStream = CreateFile( sFilename + _T(":Zone.Identifier"),
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

		if ( hStream == INVALID_HANDLE_VALUE )
		{
			HANDLE hFile = CreateFile( sFilename, GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL );

			if ( hFile != INVALID_HANDLE_VALUE )
			{
				hStream = CreateFile( sFilename + _T(":Zone.Identifier"),
					GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
				CloseHandle( hFile );
			}
		}

		if ( hStream != INVALID_HANDLE_VALUE )
		{
			DWORD dwWritten = 0;
			bSuccess = ( WriteFile( hStream, "[ZoneTransfer]\r\nZoneID=3\r\n", 26,
				&dwWritten, NULL ) && dwWritten == 26 );
			CloseHandle( hStream );
		}
		else
			TRACE( "MarkFileAsDownload() : CreateFile \"%s\" error %d\n", (LPCSTR)CT2A( sFilename ), GetLastError() );

		if ( bChanged )
			SetFileAttributes( sFilename, dwOrigAttr );
	}
	return bSuccess;
}

bool LoadGUID(const CString& sFilename, Hashes::Guid& oGUID)
{
	bool bSuccess = false;
	if ( Settings.Library.UseFolderGUID )
	{
		HANDLE hFile = CreateFile( sFilename + _T(":") CLIENT_NAME_T _T(".GUID"),
			GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			Hashes::Guid oTmpGUID;
			DWORD dwReaded = 0;
			bSuccess = ( ReadFile( hFile, &*oTmpGUID.begin(), oTmpGUID.byteCount,
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
	if ( Settings.Library.UseFolderGUID )
	{
		// Temporary clear R/O attribute
		BOOL bChanged = FALSE;
		DWORD dwOrigAttr = GetFileAttributes( sFilename );
		if ( dwOrigAttr != 0xffffffff && ( dwOrigAttr & FILE_ATTRIBUTE_READONLY ) )
			bChanged = SetFileAttributes( sFilename, dwOrigAttr & ~FILE_ATTRIBUTE_READONLY );

		HANDLE hStream = CreateFile( sFilename + _T(":") CLIENT_NAME_T _T(".GUID"),
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

		if ( hStream == INVALID_HANDLE_VALUE )
		{
			HANDLE hFile = CreateFile( sFilename, GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL );

			if ( hFile != INVALID_HANDLE_VALUE )
			{
				hStream = CreateFile( sFilename + _T(":") CLIENT_NAME_T _T(".GUID"),
					GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
				CloseHandle( hFile );
			}
		}

		if ( hStream != INVALID_HANDLE_VALUE )
		{
			DWORD dwWritten = 0;
			bSuccess = ( WriteFile( hStream, &*oGUID.begin(), oGUID.byteCount,
				&dwWritten, NULL ) && dwWritten == oGUID.byteCount );
			CloseHandle( hStream );
		}

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
			SetWindowLongPtr( hWnd, GWL_STYLE,
				GetWindowLongPtr( hWnd, GWL_STYLE ) & ~DS_CONTEXTHELP );
			SetWindowLongPtr( hWnd, GWL_EXSTYLE,
				GetWindowLongPtr( hWnd, GWL_EXSTYLE ) & ~WS_EX_CONTEXTHELP );

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
			_tcsncpy_s( szDefaultPath, MAX_PATH, (LPCTSTR)theApp.GetDocumentsFolder(), MAX_PATH - 1 );
		szInitialPath = szDefaultPath;
	}

	TCHAR szDisplayName[ MAX_PATH ] = {};
	BROWSEINFO pBI = {};
	pBI.hwndOwner = hWnd ? hWnd : AfxGetMainWnd()->GetSafeHwnd();
	pBI.pszDisplayName = szDisplayName;
	pBI.lpszTitle = szTitle;
	pBI.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
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
	_tcsncpy_s( szDefaultPath, MAX_PATH, szPath, MAX_PATH - 1 );

	return CString( szPath );
}

BOOL PostMainWndMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if ( CMainWnd* pWnd = theApp.SafeMainWnd() )
		return pWnd->PostMessage( Msg, wParam, lParam );
	else
		return FALSE;
}

void SafeMessageLoop()
{
	InterlockedIncrement( &theApp.m_bBusy );
	__try
	{
		MSG msg;
		while ( PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		if ( GetWindowThreadProcessId( AfxGetMainWnd()->GetSafeHwnd(), NULL ) ==
			GetCurrentThreadId() )
		{
			LONG lIdle = 0;
			while ( AfxGetApp()->OnIdle( lIdle++ ) );
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
	InterlockedDecrement( &theApp.m_bBusy );
}

BOOL IsUserUsingFullscreen()
{
	HWND hwnd = GetForegroundWindow();
	if ( ! hwnd )
		return FALSE;
	RECT rcWindow;
	GetWindowRect( hwnd, &rcWindow );
	HMONITOR hm = MonitorFromRect( &rcWindow, MONITOR_DEFAULTTONULL );
	if ( ! hm )
		return FALSE;
	MONITORINFO mi = { sizeof( MONITORINFO ) };
	GetMonitorInfo( hm, &mi );
	if ( ! EqualRect( &rcWindow, &mi.rcMonitor ) )
		return FALSE;
	return TRUE;
}

template <>
__int8 GetRandomNum<__int8>(const __int8& min, const __int8& max)
{
	return (__int8)GetRandomNum<unsigned __int8>( min, max );
}

template <>
__int16 GetRandomNum<__int16>(const __int16& min, const __int16& max)
{
	return (__int16)GetRandomNum<unsigned __int16>( min, max );
}

template <>
__int32 GetRandomNum<__int32>(const __int32& min, const __int32& max)
{
	return (__int32)GetRandomNum<unsigned __int32>( min, max );
}

template <>
__int64 GetRandomNum<__int64>(const __int64& min, const __int64& max)
{
	return (__int64)GetRandomNum<unsigned __int64>( min, max );
}

BOOL AreServiceHealthy(LPCTSTR szService)
{
	BOOL bResult = TRUE;	// Ok or unknown state

	// Open a handle to the Service Control Manager database
	SC_HANDLE schSCManager = OpenSCManager(
		NULL,				// local machine
		NULL,				// ServicesActive database
		GENERIC_READ );		// for enumeration and status lookup
	if ( schSCManager )
	{
		SC_HANDLE schServiceRead = OpenService( schSCManager, szService, GENERIC_READ );
		if ( schServiceRead )
		{
			SERVICE_STATUS_PROCESS ssStatus = {};
			DWORD nBytesNeeded = 0;
			if ( QueryServiceStatusEx( schServiceRead, SC_STATUS_PROCESS_INFO,
				(LPBYTE)&ssStatus, sizeof( SERVICE_STATUS_PROCESS ), &nBytesNeeded ) )
			{
				bResult = ( ssStatus.dwCurrentState == SERVICE_RUNNING );
			}
			CloseServiceHandle( schServiceRead );

			if ( ! bResult )
			{
				SC_HANDLE schServiceStart = OpenService( schSCManager, szService, SERVICE_START );
				if ( schServiceStart )
				{
					// Power users have only right to start service, thus try to start it here
					bResult = StartService( schServiceStart, 0, NULL );

					CloseServiceHandle( schServiceStart );
				}
			}
		}
		CloseServiceHandle( schSCManager );
	}

	return bResult;
}
