//
// Shareaza.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
// This file is part of SHAREAZA (www.shareaza.com)
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
#include "CoolInterface.h"
#include "Network.h"
#include "Security.h"
#include "HostCache.h"
#include "DiscoveryServices.h"
#include "VersionChecker.h"
#include "SchemaCache.h"
#include "VendorCache.h"
#include "EDClients.h"
#include "BTClients.h"
#include "Library.h"
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
#include "ShellIcons.h"
#include "Skin.h"
#include "Scheduler.h"

#include "WndMain.h"
#include "WndSystem.h"
#include "DlgSplash.h"
#include "DlgHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CShareazaApp, CWinApp)
	//{{AFX_MSG_MAP(CShareazaApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

CShareazaApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp construction

CShareazaApp::CShareazaApp() : m_pMutex( FALSE, _T("Shareaza") )
{
	m_pSafeWnd	= NULL;
	m_bLive		= FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp initialization

BOOL CShareazaApp::InitInstance()
{
	CWaitCursor pCursor;
	
	if ( ! m_pMutex.Lock( 0 ) )
	{
		if ( CWnd* pWnd = CWnd::FindWindow( _T("ShareazaMainWnd"), NULL ) )
		{
			pWnd->SendMessage( WM_SYSCOMMAND, SC_RESTORE );
			pWnd->ShowWindow( SW_SHOWNORMAL );
			pWnd->BringWindowToTop();
			pWnd->SetForegroundWindow();
		}
		
		return FALSE;
	}


	// ***********
	// Beta expiry. Remember to re-compile to update the time, and remove this 
	// section for final releases and public betas.
	COleDateTime tCompileTime; 
	tCompileTime.ParseDateTime( _T(__DATE__), LOCALE_NOUSEROVERRIDE, 1033 );
	COleDateTime tCurrent = COleDateTime::GetCurrentTime();
	COleDateTimeSpan tTimeOut( 7, 0, 0, 0);
	if ( ( tCompileTime + tTimeOut )  < tCurrent )
	{
		CString strMessage;
		LoadString( strMessage, IDS_BETA_EXPIRED);
		AfxMessageBox( strMessage, MB_SYSTEMMODAL|MB_ICONQUESTION|MB_OK );
		return FALSE;
	}

	// Alpha warning. Remember to remove this section for final releases and public betas.
	if ( AfxMessageBox( _T("WARNING: This is an ALPHA TEST version of Shareaza.\n\nIt it NOT FOR GENERAL USE, and is only for testing specific features in a controlled environment. It will frequently stop running, or display debug information to assist testing.\n\nIf you wish to actually use this software, you should download the current stable release from www.shareaza.com\nIf you continue past this point, you may experience system instability, lose downloads, or corrupt system files. Corrupted downloads/files may not be recoverable. Do you wish to continue?"), MB_SYSTEMMODAL|MB_ICONEXCLAMATION|MB_YESNO ) == IDNO )
		return FALSE;
	// ***********



	// Enable3dControls();
	SetRegistryKey( _T("Shareaza") );
	GetVersionNumber();
	InitResources();
	
	BOOL bSilentTray = ( m_lpCmdLine && _tcsistr( m_lpCmdLine, _T("-tray") ) != NULL );
	
		AfxOleInit();
		AfxEnableControlContainer();
	
	CSplashDlg* dlgSplash = new CSplashDlg( 18, bSilentTray );
	
	dlgSplash->Step( _T("Winsock") );
		WSADATA wsaData;
		if ( WSAStartup( 0x0101, &wsaData ) ) return FALSE;
	
	dlgSplash->Step( _T("Settings Database") );
		Settings.Load();
	dlgSplash->Step( _T("P2P URIs") );
		CShareazaURL::Register();
	dlgSplash->Step( _T("Shell Icons") );
		ShellIcons.Clear();
	dlgSplash->Step( _T("Metadata Schemas") );
		SchemaCache.Load();
	dlgSplash->Step( _T("Vendor Data") );
		VendorCache.Load();
	dlgSplash->Step( _T("Profile") );
		MyProfile.Load();
	dlgSplash->Step( _T("Library") );
		Library.Load();
	dlgSplash->Step( _T("Query Manager") );
		QueryHashMaster.Create();
	dlgSplash->Step( _T("Host Cache") );
		HostCache.Load();
	dlgSplash->Step( _T("Discovery Services") );
		DiscoveryServices.Load();
	dlgSplash->Step( _T("Security Services") );
		Security.Load();
		AdultFilter.Load();
	dlgSplash->Step( _T("Scheduler") );
		Schedule.Load();
	dlgSplash->Step( _T("Rich Documents") );
		Emoticons.Load();
	dlgSplash->Step( _T("GUI") );
	
	if ( bSilentTray ) WriteProfileInt( _T("Windows"), _T("CMainWnd.ShowCmd"), 0 );
	
	m_pMainWnd = new CMainWnd();
	CoolMenu.EnableHook();
	
	if ( bSilentTray )
	{
		((CMainWnd*)m_pMainWnd)->CloseToTray();
	}
	else
	{
		dlgSplash->Topmost();
		m_pMainWnd->ShowWindow( SW_SHOW );
		m_pMainWnd->UpdateWindow();
	}
	// From this point translations are available and LoadString returns correct strings
	dlgSplash->Step( _T("Download Manager") ); 
		Downloads.Load();
	dlgSplash->Step( _T("Upload Manager") );
		UploadQueues.Load();

	dlgSplash->Step( _T("IPC") );
		DDEServer.Create();
		IEProtocol.Create();

	dlgSplash->Step( _T("Upgrade Manager") );
	if ( VersionChecker.NeedToCheck() ) VersionChecker.Start( m_pMainWnd->GetSafeHwnd() );
	
	pCursor.Restore();
	
	dlgSplash->Hide();
	m_bLive = TRUE;
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp termination

int CShareazaApp::ExitInstance() 
{
	CWaitCursor pCursor;
	
	DDEServer.Close();
	IEProtocol.Close();
	VersionChecker.Stop();
	DiscoveryServices.Stop();
	Network.Disconnect();
	Library.StopThread();
	
	Transfers.StopThread();
	Downloads.CloseTransfers();
	Uploads.Clear( FALSE );
	EDClients.Clear();
	BTClients.Clear();
	
	if ( m_bLive )
	{
		Downloads.Save();
		DownloadGroups.Save();
		Library.Save();
		Security.Save();
		HostCache.Save();
		UploadQueues.Save();
		DiscoveryServices.Save();
	}
	
	Downloads.Clear( TRUE );
	Library.Clear();
	Skin.Clear();
	
	if ( m_bLive ) Settings.Save( TRUE );

	if ( m_hUser32 != NULL ) FreeLibrary( m_hUser32 );
	
	WSACleanup();
	
	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp help (suppress F1)

void CShareazaApp::WinHelp(DWORD dwData, UINT nCmd) 
{
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp version

void CShareazaApp::GetVersionNumber()
{
	TCHAR szPath[128];
	DWORD dwSize;

	m_nVersion[0] = m_nVersion[1] = m_nVersion[2] = m_nVersion[3] = 0;

	GetModuleFileName( NULL, szPath, 128 );
	dwSize = GetFileVersionInfoSize( szPath, &dwSize );

	if ( dwSize )
	{
		BYTE* pBuffer = new BYTE[ dwSize ];

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

	m_sVersion.Format( _T("%i.%i.%i.%i"),
		m_nVersion[0], m_nVersion[1],
		m_nVersion[2], m_nVersion[3] );
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp resources

void CShareazaApp::InitResources()
{
	//Determine the version of Windows
	OSVERSIONINFO pVersion;
	pVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx( &pVersion );
	
	//Networking is poor under Win9x based operating systems. (95/98/Me)
	m_bNT = ( pVersion.dwPlatformId == VER_PLATFORM_WIN32_NT );

	//Win 95/98/Me/NT (<5) do not support some functions
	m_dwWindowsVersion = pVersion.dwMajorVersion; 

	//Win2000 = 0 WinXP = 1
	m_dwWindowsVersionMinor = pVersion.dwMinorVersion; 

	m_bLimitedConnections = FALSE;
	if ( m_dwWindowsVersion == 5 && m_dwWindowsVersionMinor == 1 )
	{	//Windows XP - Test for SP2
		if( _tcsnicmp( pVersion.szCSDVersion, _T("Service Pack 2"), 15 ) == 0 )
		{	//XP SP2 - Limit the networking.
			//AfxMessageBox(_T("Warning - Windows XP Service Pack 2 detected. Performance may be reduced."), MB_OK );
			m_bLimitedConnections = TRUE;
		}
	}

	//Get the amount of installed memory.
	m_nPhysicalMemory = 0;
	if ( m_hUser32 = LoadLibrary( _T("User32.dll") ) )
	{	//Use GlobalMemoryStatusEx if possible (WinXP)
		void (WINAPI *m_pfnGlobalMemoryStatus)( LPMEMORYSTATUSEX );
		MEMORYSTATUSEX pMemory;

		(FARPROC&)m_pfnGlobalMemoryStatus = GetProcAddress(
			m_hUser32, "GlobalMemoryStatusEx" );

		if ( m_pfnGlobalMemoryStatus )
		{
			m_pfnGlobalMemoryStatus( &pMemory ); 
			m_nPhysicalMemory = pMemory.ullTotalPhys;
		}
	}

	if ( m_nPhysicalMemory == 0 )
	{	//Fall back to GlobalMemoryStatusEx (always available)
		MEMORYSTATUS pMemory;
		GlobalMemoryStatus( &pMemory ); 
		m_nPhysicalMemory = pMemory.dwTotalPhys;
	}
	
	//Get pointers to some functions that don't exist under 95/NT
	if ( m_hUser32 = LoadLibrary( _T("User32.dll") ) )
	{
		(FARPROC&)m_pfnSetLayeredWindowAttributes = GetProcAddress(
			m_hUser32, "SetLayeredWindowAttributes" );
		   
		(FARPROC&)m_pfnGetMonitorInfoA = GetProcAddress( 
			m_hUser32, "GetMonitorInfoA" ); 
    
		(FARPROC&)m_pfnMonitorFromRect = GetProcAddress( 
			m_hUser32, "MonitorFromRect" ); 

		(FARPROC&)m_pfnMonitorFromWindow = GetProcAddress( 
			m_hUser32, "MonitorFromWindow" ); 
	}
	else
	{
		m_pfnSetLayeredWindowAttributes = NULL;
		m_pfnGetMonitorInfoA = NULL; 
        m_pfnMonitorFromRect = NULL; 
		m_pfnMonitorFromWindow = NULL;
	}
	
	m_gdiFont.CreateFont( -11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, _T("Tahoma") );
	
	m_gdiFontBold.CreateFont( -11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, _T("Tahoma") );
	
	m_gdiFontLine.CreateFont( -11, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, _T("Tahoma") );

	srand( GetTickCount() );
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp safe main window

CMainWnd* CShareazaApp::SafeMainWnd()
{
	CMainWnd* pMainWnd = (CMainWnd*)theApp.m_pSafeWnd;
	if ( pMainWnd == NULL ) return NULL;
	ASSERT_KINDOF( CMainWnd, pMainWnd );
	return IsWindow( pMainWnd->m_hWnd ) ? pMainWnd : NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp message

TCHAR CShareazaApp::szMessageBuffer[16384];

void CShareazaApp::Message(int nType, UINT nID, ...)
{
	if ( nType == MSG_DEBUG && ! Settings.General.Debug ) return;
	if ( nType == MSG_TEMP && ! Settings.General.DebugLog ) return;
	
	CSingleLock pLock( &m_csMessage, TRUE );
	CString strFormat;
	va_list pArgs;
	
	LoadString( strFormat, nID );
	va_start( pArgs, nID );
	
	if ( strFormat.Find( _T("%1") ) >= 0 )
	{
		LPTSTR lpszTemp;
		if ( ::FormatMessage( FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
			strFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &pArgs ) != 0 && lpszTemp != NULL )
		{
			PrintMessage( nType, lpszTemp );
			if ( Settings.General.DebugLog ) LogMessage( lpszTemp );
			LocalFree( lpszTemp );
		}
	}
	else
	{
		_vsntprintf( szMessageBuffer, 16380, strFormat, pArgs );
		PrintMessage( nType, szMessageBuffer );
		if ( Settings.General.DebugLog ) LogMessage( szMessageBuffer );
	}

	va_end( pArgs );
}

void CShareazaApp::Message(int nType, LPCTSTR pszFormat, ...)
{
	if ( nType == MSG_DEBUG && ! Settings.General.Debug ) return;
	if ( nType == MSG_TEMP && ! Settings.General.DebugLog ) return;
	
	CSingleLock pLock( &m_csMessage, TRUE );
	CString strFormat;
	va_list pArgs;
	
	va_start( pArgs, pszFormat );
	_vsntprintf( szMessageBuffer, 16380, pszFormat, pArgs );
	va_end( pArgs );
	
	PrintMessage( nType, szMessageBuffer );
	if ( Settings.General.DebugLog ) LogMessage( szMessageBuffer );
}

void CShareazaApp::PrintMessage(int nType, LPCTSTR pszLog)
{
	if ( HWND hWnd = m_pSafeWnd->GetSafeHwnd() )
	{
		PostMessage( hWnd, WM_LOG, nType, (LPARAM)_tcsdup( pszLog ) );
	}
}

void CShareazaApp::LogMessage(LPCTSTR pszLog)
{
	CFile pFile;
	
	if ( pFile.Open( _T("\\Shareaza.log"), CFile::modeReadWrite ) )
	{
		if ( ( Settings.General.MaxDebugLogSize ) &&					// If file rotation is on 
			( pFile.GetLength() > Settings.General.MaxDebugLogSize ) )	// and file is too long...
		{	
			pFile.Close();				// Close the file and start a new one
			if ( ! pFile.Open( _T("\\Shareaza.log"), CFile::modeWrite|CFile::modeCreate ) ) return;
		}
		else
		{
			pFile.Seek( 0, CFile::end ); // Otherwise, go to the end of the file to add entires.
		}
	}
	else
	{
		if ( ! pFile.Open( _T("\\Shareaza.log"), CFile::modeWrite|CFile::modeCreate ) ) return;

#ifdef _UNICODE
		WORD nByteOrder = 0xFEFF;
		pFile.Write( &nByteOrder, 2 );
#endif
	}
	
	if ( Settings.General.ShowTimestamp )
	{
		CTime pNow = CTime::GetCurrentTime();
		CString strLine;
		
		strLine.Format( _T("[%.2i:%.2i:%.2i] %s\r\n"),
			pNow.GetHour(), pNow.GetMinute(), pNow.GetSecond(), pszLog );
		
		pFile.Write( (LPCTSTR)strLine, sizeof(TCHAR) * strLine.GetLength() );
	}
	else
	{
		pFile.Write( pszLog, sizeof(TCHAR) * _tcslen(pszLog) );
		pFile.Write( _T("\r\n"), sizeof(TCHAR) * 2 );
	}
	
	pFile.Close();
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp get error string

CString CShareazaApp::GetErrorString()
{
	LPTSTR pszMessage = NULL;
	CString strMessage;
	
	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&pszMessage, 0, NULL );
	
	if ( pszMessage != NULL )
	{
		strMessage = pszMessage;
		LocalFree( pszMessage );
	}
	
	return strMessage;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaApp process an internal URI

BOOL CShareazaApp::InternalURI(LPCTSTR pszURI)
{
	if ( m_pSafeWnd == NULL ) return FALSE;
	CMainWnd* pMainWnd = (CMainWnd*)m_pSafeWnd;
	
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

		if ( CLibraryFile* pFile = Library.LookupFile( nIndex, TRUE ) )
		{
			pFile->Execute();
			Library.Unlock();
		}
	}
	else if (	strURI.Find( _T("http://") ) == 0 ||
				strURI.Find( _T("ftp://") ) == 0 ||
				strURI.Find( _T("mailto:") ) == 0 ||
				strURI.Find( _T("aim:") ) == 0 ||
				strURI.Find( _T("magnet:") ) == 0 ||
				strURI.Find( _T("gnutella:") ) == 0 ||
				strURI.Find( _T("shareaza:") ) == 0 ||
				strURI.Find( _T("gnet:") ) == 0 ||
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
	{
		return FALSE;
	}

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

void Split(CString strSource, LPCTSTR pszDelimiter, CStringArray& pAddIt, BOOL bAddFirstEmpty)
{
	CString		strNew = strSource;
	CString		strTemp = strSource;
	CString		strAdd;
	BOOL		bFirstChecked = FALSE;

	int nPos1;
	int nPos = 0;

	if ( ! _tcslen( pszDelimiter ) )
		pszDelimiter = _T("|"); 

	do
	{
		nPos1 = 0;
		nPos = strNew.Find( pszDelimiter, nPos1 );
		if ( nPos != -1 ) 
		{
			CString strAdd = strTemp = strNew.Left( nPos );
			if ( ! strAdd.IsEmpty() && ! strTemp.Trim().IsEmpty() ) 
			{
				pAddIt.Add( strAdd );
			}
			else if ( bAddFirstEmpty && ! bFirstChecked ) 
			{
				pAddIt.Add( strAdd.Trim() );
			}
			strNew = strTemp = strNew.Mid( nPos + _tcslen( pszDelimiter ) );
		}
		bFirstChecked = TRUE; // Allow only the first item empty and ignore trailing empty items 
	} while ( nPos != -1 );
	
	if ( ! strTemp.Trim().IsEmpty() )
		pAddIt.Add( strNew );
}

BOOL LoadString(CString& str, UINT nID)
{
	return Skin.LoadString( str, nID );
}

void Replace(CString& strBuffer, LPCTSTR pszFind, LPCTSTR pszReplace)
{
	while ( TRUE )
	{
		int nPos = strBuffer.Find( pszFind );
		if ( nPos < 0 ) break;

		strBuffer = strBuffer.Left( nPos ) + pszReplace + strBuffer.Mid( nPos + _tcslen( pszFind ) );
	}
}

BOOL LoadSourcesString(CString& str, DWORD num)
{
	if (num == 0)
	{
		return Skin.LoadString( str, IDS_STATUS_NOSOURCES );
	}
	else if (num == 1)
	{
		return Skin.LoadString( str, IDS_STATUS_SOURCE );
	}
	else if ( ( (num % 100) > 10) && ( (num % 100) < 20) )
	{
		return Skin.LoadString( str, IDS_STATUS_SOURCES11TO19 );
	}
	else
	{
		switch (num % 10)
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
	LPCTSTR pptr, sptr, start;
	DWORD slen, plen;

	for (	start	= pszString,
			pptr	= pszPattern,
			slen	= _tcslen( pszString ),
			plen	= _tcslen( pszPattern ) ;
			slen >= plen ; start++, slen-- )
	{
		while ( toupper( *start ) != toupper( *pszPattern ) )
		{
			start++;
			slen--;

			if ( slen < plen ) return NULL;
		}

		sptr = start;
		pptr = pszPattern;

		while ( toupper( *sptr ) == toupper( *pptr ) )
		{
			sptr++;
			pptr++;

			if ( ! *pptr) return start;
		}
	}

	return NULL;
}

LPCTSTR _tcsnistr(LPCTSTR pszString, LPCTSTR pszPattern, DWORD plen)
{
	LPCTSTR pptr, sptr, start;
	DWORD slen, plen2;

	for (	start	= pszString,
			pptr	= pszPattern,
			slen	= _tcslen( pszString ) ;
			slen >= plen ; start++, slen-- )
	{
		while ( toupper( *start ) != toupper( *pszPattern ) )
		{
			start++;
			slen--;

			if ( slen < plen ) return NULL;
		}

		sptr = start;
		pptr = pszPattern;
		plen2 = plen;

		while ( toupper( *sptr ) == toupper( *pptr ) )
		{
			sptr++;
			pptr++;

			if ( ! --plen2 ) return start;
		}
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Time Management Functions (C-runtime)

DWORD TimeFromString(LPCTSTR pszTime)
{
	// 2002-04-30T08:30Z
	
	if ( _tcslen( pszTime ) != 17 ) return 0;
	if ( pszTime[4] != '-' || pszTime[7] != '-' ) return 0;
	if ( pszTime[10] != 'T' || pszTime[13] != ':' || pszTime[16] != 'Z' ) return 0;
	
	struct tm pTime;
	LPCTSTR psz;
	int nTemp;
	
	ZeroMemory( &pTime, sizeof(pTime) );
	
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
	
	return tGMT + ( tGMT - tSub );
}

CString TimeToString(DWORD tVal)
{
	struct tm* pTime = gmtime( (time_t*)&tVal );
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
	
	SYSTEMTIME pOut;
	LPCTSTR psz;
	int nTemp;

	ZeroMemory( &pOut, sizeof(pOut) );

	if ( _stscanf( pszTime, _T("%i"), &nTemp ) != 1 ) return FALSE;
	pOut.wYear = nTemp;
	for ( psz = pszTime + 5 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, _T("%i"), &nTemp ) != 1 ) return FALSE;
	pOut.wMonth = nTemp;
	for ( psz = pszTime + 8 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, _T("%i"), &nTemp ) != 1 ) return FALSE;
	pOut.wDay = nTemp;
	for ( psz = pszTime + 11 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, _T("%i"), &nTemp ) != 1 ) return FALSE;
	pOut.wHour = nTemp;
	for ( psz = pszTime + 14 ; *psz == '0' ; psz++ );
	if ( _stscanf( psz, _T("%i"), &nTemp ) != 1 ) return FALSE;
	pOut.wMinute = nTemp;

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
