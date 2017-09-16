//
// WndMain.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
#include "CoolInterface.h"
#include "CoolMenu.h"
#include "Network.h"
#include "Handshake.h"
#include "HostCache.h"
#include "Neighbours.h"
#include "Transfers.h"
#include "Download.h"
#include "Downloads.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "LibraryFolders.h"
#include "Plugins.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "VersionChecker.h"
#include "GraphItem.h"
#include "ShareazaURL.h"
#include "ChatCore.h"
#include "ChatSession.h"
#include "ChatWindows.h"
#include "Statistics.h"
#include "BTInfo.h"
#include "Skin.h"
#include "SkinWindow.h"
#include "DlgHelp.h"
#include "LibraryHistory.h"
#include "SharedFile.h"
#include "DiscoveryServices.h"
#include "Scheduler.h"
#include "SchemaCache.h"

#include "WndMain.h"
#include "WndChild.h"
#include "WndSystem.h"
#include "WndNeighbours.h"
#include "WndTraffic.h"
#include "WndDownloads.h"
#include "WndUploads.h"
#include "WndLibrary.h"
#include "WndMedia.h"
#include "WndHostCache.h"
#include "WndDiscovery.h"
#include "WndPacket.h"
#include "WndSearchMonitor.h"
#include "WndHitMonitor.h"
#include "WndSecurity.h"
#include "WndSearch.h"
#include "WndScheduler.h"
#include "WndBrowseHost.h"
#include "WndHome.h"
#include "WndIRC.h"
#include "WizardSheet.h"

#include "DlgAbout.h"
#include "DlgCloseMode.h"
#include "DlgConnectTo.h"
#include "DlgDonkeyImport.h"
#include "DlgDownload.h"
#include "DlgDownloadMonitor.h"
#include "DlgExistingFile.h"
#include "DlgFilePreview.h"
#include "DlgLanguage.h"
#include "DlgNewSearch.h"
#include "DlgProfileManager.h"
#include "DlgPromote.h"
#include "DlgScheduleTask.h"
#include "DlgSettingsManager.h"
#include "DlgShareManager.h"
#include "DlgTorrentSeed.h"
#include "DlgURLAction.h"
#include "DlgUpgrade.h"
#include "DlgWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CMainWnd, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainWnd, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_INITMENUPOPUP()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOMMAND()
	ON_WM_ACTIVATE()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_ENDSESSION()
	ON_WM_WINDOWPOSCHANGING()
	ON_MESSAGE(WM_WINSOCK, OnWinsock)
	ON_MESSAGE(WM_URL, OnHandleURL)
	ON_MESSAGE(WM_COLLECTION, OnHandleCollection)
	ON_MESSAGE(WM_IMPORT, OnHandleImport)
	ON_MESSAGE(WM_TORRENT, OnHandleTorrent)
	ON_MESSAGE(WM_VERSIONCHECK, OnVersionCheck)
	ON_MESSAGE(WM_OPENCHAT, OnOpenChat)
	ON_MESSAGE(WM_OPENSEARCH, OnOpenSearch)
	ON_MESSAGE(WM_TRAY, OnTray)
	ON_MESSAGE(WM_SETALPHA, OnChangeAlpha)
	ON_MESSAGE(WM_SKINCHANGED, OnSkinChanged)
	ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_APPCOMMAND, OnMediaKey)
	ON_MESSAGE(WM_DEVMODECHANGE, OnDevModeChange)
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_MESSAGE(WM_LIBRARYSEARCH, OnLibrarySearch)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PLUGIN_FIRST, ID_PLUGIN_LAST, OnUpdatePluginRange)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SYSTEM, OnUpdateViewSystem)
	ON_COMMAND(ID_VIEW_SYSTEM, OnViewSystem)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEIGHBOURS, OnUpdateViewNeighbours)
	ON_COMMAND(ID_VIEW_NEIGHBOURS, OnViewNeighbours)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_CONNECT, OnUpdateNetworkConnect)
	ON_COMMAND(ID_NETWORK_CONNECT, OnNetworkConnect)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_DISCONNECT, OnUpdateNetworkDisconnect)
	ON_COMMAND(ID_NETWORK_DISCONNECT, OnNetworkDisconnect)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PACKETS, OnUpdateViewPackets)
	ON_COMMAND(ID_VIEW_PACKETS, OnViewPackets)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HOSTS, OnUpdateViewHosts)
	ON_COMMAND(ID_VIEW_HOSTS, OnViewHosts)
	ON_COMMAND(ID_NETWORK_CONNECT_TO, OnNetworkConnectTo)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SEARCH_MONITOR, OnUpdateViewSearchMonitor)
	ON_COMMAND(ID_VIEW_SEARCH_MONITOR, OnViewSearchMonitor)
	ON_COMMAND(ID_NETWORK_EXIT, OnNetworkExit)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_SEARCH, OnUpdateNetworkSearch)
	ON_COMMAND(ID_NETWORK_SEARCH, OnNetworkSearch)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RESULTS_MONITOR, OnUpdateViewResultsMonitor)
	ON_COMMAND(ID_VIEW_RESULTS_MONITOR, OnViewResultsMonitor)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_CONNECT_TO, OnUpdateNetworkConnectTo)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DOWNLOADS, OnUpdateViewDownloads)
	ON_COMMAND(ID_VIEW_DOWNLOADS, OnViewDownloads)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LIBRARY, OnUpdateViewLibrary)
	ON_COMMAND(ID_VIEW_LIBRARY, OnViewLibrary)
	ON_UPDATE_COMMAND_UI(ID_VIEW_UPLOADS, OnUpdateViewUploads)
	ON_COMMAND(ID_VIEW_UPLOADS, OnViewUploads)
	ON_COMMAND(ID_TOOLS_SETTINGS, OnToolsSettings)
	ON_COMMAND(ID_HELP_ABOUT, OnHelpAbout)
	ON_COMMAND(ID_HELP_VERSION_CHECK, OnHelpVersionCheck)
	ON_COMMAND(ID_HELP_HOMEPAGE, OnHelpHomepage)
	ON_COMMAND(ID_HELP_WEB_1, OnHelpWeb1)
	ON_COMMAND(ID_HELP_WEB_2, OnHelpWeb2)
	ON_COMMAND(ID_HELP_WEB_3, OnHelpWeb3)
	ON_COMMAND(ID_HELP_WEB_4, OnHelpWeb4)
	ON_COMMAND(ID_HELP_WEB_5, OnHelpWeb5)
	ON_COMMAND(ID_HELP_WEB_6, OnHelpWeb6)
	ON_COMMAND(ID_HELP_FAQ, OnHelpFaq)
	ON_COMMAND(ID_HELP_GUIDE, OnHelpGuide)
	ON_COMMAND(ID_HELP_FORUMS, OnHelpForums)
	ON_COMMAND(ID_HELP_UPDATE, OnHelpUpdate)
	ON_COMMAND(ID_HELP_ROUTER, OnHelpRouter)
	ON_COMMAND(ID_HELP_SECURITY, OnHelpSecurity)
	ON_COMMAND(ID_HELP_CODEC, OnHelpCodec)
	ON_COMMAND(ID_HELP_TORRENT, OnHelpTorrent)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TRAFFIC, OnUpdateViewTraffic)
	ON_COMMAND(ID_VIEW_TRAFFIC, OnViewTraffic)
	ON_COMMAND(ID_WINDOW_CASCADE, OnWindowCascade)
	ON_COMMAND(ID_TOOLS_WIZARD, OnToolsWizard)
	ON_COMMAND(ID_TRAY_OPEN, OnTrayOpen)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_AUTO_CLOSE, OnUpdateNetworkAutoClose)
	ON_COMMAND(ID_NETWORK_AUTO_CLOSE, OnNetworkAutoClose)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_DOWNLOAD, OnUpdateToolsDownload)
	ON_COMMAND(ID_TOOLS_DOWNLOAD, OnToolsDownload)
	ON_UPDATE_COMMAND_UI(ID_IMPORT_DOWNLOADS, OnUpdateToolsImportDownloads)
	ON_COMMAND(ID_IMPORT_DOWNLOADS, OnToolsImportDownloads)
	ON_UPDATE_COMMAND_UI(ID_OPEN_DOWNLOADS_FOLDER, OnUpdateOpenDownloadsFolder)
	ON_COMMAND(ID_OPEN_DOWNLOADS_FOLDER, OnOpenDownloadsFolder)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SECURITY, OnUpdateViewSecurity)
	ON_COMMAND(ID_VIEW_SECURITY, OnViewSecurity)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_CASCADE, OnUpdateWindowCascade)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_TILE_HORZ, OnUpdateWindowTileHorz)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_TILE_VERT, OnUpdateWindowTileVert)
	ON_UPDATE_COMMAND_UI(ID_TAB_CONNECT, OnUpdateTabConnect)
	ON_COMMAND(ID_TAB_CONNECT, OnTabConnect)
	ON_UPDATE_COMMAND_UI(ID_TAB_NETWORK, OnUpdateTabNetwork)
	ON_COMMAND(ID_TAB_NETWORK, OnTabNetwork)
	ON_UPDATE_COMMAND_UI(ID_TAB_LIBRARY, OnUpdateTabLibrary)
	ON_COMMAND(ID_TAB_LIBRARY, OnTabLibrary)
	ON_UPDATE_COMMAND_UI(ID_TAB_TRANSFERS, OnUpdateTabTransfers)
	ON_COMMAND(ID_TAB_IRC, OnTabIRC)
	ON_UPDATE_COMMAND_UI(ID_TAB_IRC, OnUpdateTabIRC)
	ON_COMMAND(ID_TAB_TRANSFERS, OnTabTransfers)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TABBED, OnUpdateViewTabbed)
	ON_COMMAND(ID_VIEW_TABBED, OnViewTabbed)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WINDOWED, OnUpdateViewWindowed)
	ON_COMMAND(ID_VIEW_WINDOWED, OnViewWindowed)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DISCOVERY, OnUpdateViewDiscovery)
	ON_COMMAND(ID_VIEW_DISCOVERY, OnViewDiscovery)
	ON_UPDATE_COMMAND_UI(ID_TAB_HOME, OnUpdateTabHome)
	ON_COMMAND(ID_TAB_HOME, OnTabHome)
	ON_COMMAND(ID_TOOLS_RESKIN, OnToolsReskin)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_TABBAR, OnUpdateWindowTabBar)
	ON_COMMAND(ID_WINDOW_TABBAR, OnWindowTabBar)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_TOOLBAR, OnUpdateWindowToolBar)
	ON_COMMAND(ID_WINDOW_TOOLBAR, OnWindowToolBar)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_MONITOR, OnUpdateWindowMonitor)
	ON_COMMAND(ID_WINDOW_MONITOR, OnWindowMonitor)
	ON_COMMAND(ID_NETWORK_BROWSE_TO, OnNetworkBrowseTo)
	ON_COMMAND(ID_NETWORK_CHAT_TO, OnNetworkChatTo)
	ON_COMMAND(ID_TOOLS_SKIN, OnToolsSkin)
	ON_COMMAND(ID_TOOLS_LANGUAGE, OnToolsLanguage)
	ON_COMMAND(ID_TOOLS_SEEDTORRENT, OnToolsSeedTorrent)
	ON_COMMAND(ID_TOOLS_RESEEDTORRENT, OnToolsReseedTorrent)
	ON_COMMAND(ID_HELP_DISKSPACE, OnDiskSpace)
	ON_COMMAND(ID_HELP_DISKWRITEFAIL, OnDiskWriteFail)
	ON_COMMAND(ID_HELP_CONNECTIONFAIL, OnConnectionFail)
	ON_COMMAND(ID_HELP_DONKEYSERVERS, OnNoDonkeyServers)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MEDIA, OnUpdateViewMedia)
	ON_COMMAND(ID_VIEW_MEDIA, OnViewMedia)
	ON_UPDATE_COMMAND_UI(ID_TAB_MEDIA, OnUpdateTabMedia)
	ON_COMMAND(ID_TAB_MEDIA, OnTabMedia)
	ON_UPDATE_COMMAND_UI(ID_TAB_SEARCH, OnUpdateTabSearch)
	ON_COMMAND(ID_TAB_SEARCH, OnTabSearch)
	ON_COMMAND(ID_TOOLS_PROFILE, OnToolsProfile)
	ON_COMMAND(ID_LIBRARY_FOLDERS, OnLibraryFolders)
	ON_COMMAND(ID_HELP_WARNINGS, OnHelpWarnings)
	ON_COMMAND(ID_HELP_PROMOTE, OnHelpPromote)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_G2, OnUpdateNetworkG2)
	ON_COMMAND(ID_NETWORK_G2, OnNetworkG2)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_G1, OnUpdateNetworkG1)
	ON_COMMAND(ID_NETWORK_G1, OnNetworkG1)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_ED2K, OnUpdateNetworkED2K)
	ON_COMMAND(ID_NETWORK_ED2K, OnNetworkED2K)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_DC, OnUpdateNetworkDC)
	ON_COMMAND(ID_NETWORK_DC, OnNetworkDC)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_BT, OnUpdateNetworkBT)
	ON_COMMAND(ID_NETWORK_BT, OnNetworkBT)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BASIC, OnUpdateViewBasic)
	ON_COMMAND(ID_VIEW_BASIC, OnViewBasic)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_HASH_PRIORITY, OnUpdateLibraryHashPriority)
	ON_COMMAND(ID_LIBRARY_HASH_PRIORITY, OnLibraryHashPriority)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_NAVBAR, OnUpdateWindowNavBar)
	ON_COMMAND(ID_WINDOW_NAVBAR, OnWindowNavBar)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_REMOTE, OnUpdateWindowRemote)
	ON_COMMAND(ID_WINDOW_REMOTE, OnWindowRemote)
	ON_COMMAND(ID_MONITOR_CLOSE, OnRemoteClose)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_PLAY, OnUpdateMediaCommand)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_ADD, OnUpdateMediaCommand)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_ADD_FOLDER, OnUpdateMediaCommand)
	ON_COMMAND(ID_MEDIA_PLAY, OnMediaCommand)
	ON_COMMAND(ID_MEDIA_ADD, OnMediaCommand)
	ON_COMMAND(ID_MEDIA_ADD_FOLDER, OnMediaCommand)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SCHEDULER, OnUpdateViewScheduler)
	ON_COMMAND(ID_VIEW_SCHEDULER, OnViewScheduler)
	ON_COMMAND(ID_HELP, OnHelpFaq)
	ON_COMMAND(ID_HELP_TEST, OnHelpConnectiontest)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SHELL_MENU_MIN, ID_SHELL_MENU_MAX, OnUpdateShell)
	ON_WM_MENUCHAR()
	ON_MESSAGE(WM_SANITY_CHECK, &CMainWnd::OnSanityCheck)
	ON_MESSAGE(WM_NOWUPLOADING, &CMainWnd::OnNowUploading)
	ON_WM_POWERBROADCAST()
	ON_WM_COPYDATA()
	ON_UPDATE_COMMAND_UI(ID_PATH_EXPLORE, &CMainWnd::OnUpdatePathExplore)
	ON_UPDATE_COMMAND_UI(ID_PATH_COPY, &CMainWnd::OnUpdatePathCopy)
	ON_WM_QUERYENDSESSION()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainWnd construction

CMainWnd::CMainWnd()
	: m_bTrayNotify		( FALSE )
	, m_bTrayHide		( FALSE )
	, m_bTrayIcon		( FALSE )
	, m_bTrayUpdate		( TRUE )
	, m_bTimer			( FALSE )
	, m_pSkin			( NULL )
	, m_nAlpha			( 255 )
{
	ZeroMemory( &m_pTray, sizeof( NOTIFYICONDATA ) );
	m_pTray.cbSize				= theApp.m_bIsVistaOrNewer ? sizeof( NOTIFYICONDATA ) : NOTIFYICONDATA_V3_SIZE;
	m_pTray.uCallbackMessage	= WM_TRAY;
	m_pTray.uVersion			= NOTIFYICON_VERSION; // NOTIFYICON_VERSION_4;
}

BOOL CMainWnd::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
	const RECT& rect, CWnd* pParentWnd, LPCTSTR /* lpszMenuName */,
	DWORD dwExStyle, CCreateContext* pContext)
{
	// Bypass menu creation
	return CMDIFrameWnd::Create( lpszClassName, lpszWindowName,
		dwStyle, rect, pParentWnd, NULL,
		dwExStyle | ( Settings.General.LanguageRTL ? WS_EX_LAYOUTRTL : 0 ) |
		WS_EX_APPWINDOW, pContext );
}

CMDIChildWnd* CMainWnd::MDIGetActive(BOOL* pbMaximized) const
{
	if ( m_hWnd == NULL || m_pWindows.IsEmpty() ) return NULL;

	static DWORD tLastUpdate = 0;
	static CMDIChildWnd* pActive = NULL;
	DWORD tNow = GetTickCount();
	if ( tNow > tLastUpdate + 100 || tNow < tLastUpdate ||
		 ! m_pWindows.Check( static_cast< CChildWnd* >( pActive ) ) )
	{
		tLastUpdate = tNow;
		pActive = CMDIFrameWnd::MDIGetActive( pbMaximized );
	}
	return pActive;
}

BOOL CMainWnd::PreTranslateMessage(MSG* pMsg)
{
	// check for special cancel modes for ComboBoxes
	if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_NCLBUTTONDOWN)
		AfxCancelModes(pMsg->hwnd);    // filter clicks

	// allow tooltip messages to be filtered
	if (CWnd::PreTranslateMessage(pMsg))
		return TRUE;

//#ifndef _AFX_NO_OLE_SUPPORT
//	// allow hook to consume message
//	if (m_pNotifyHook != NULL && m_pNotifyHook->OnPreTranslateMessage(pMsg))
//		return TRUE;
//#endif

	CMDIChildWnd* pActiveChild = MDIGetActive();

	// current active child gets first crack at it
	if (pActiveChild != NULL && pActiveChild->PreTranslateMessage(pMsg))
		return TRUE;

	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		// translate accelerators for frame and any children
		if (m_hAccelTable != NULL &&
			::TranslateAccelerator(m_hWnd, m_hAccelTable, pMsg))
		{
			return TRUE;
		}

		// special processing for MDI accelerators last
		// and only if it is not in SDI mode (print preview)
		if (GetActiveView() == NULL)
		{
			if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)
			{
				// the MDICLIENT window may translate it
				if (::TranslateMDISysAccel(m_hWndMDIClient, pMsg))
					return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CMainWnd::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* /* pContext */)
{
	// Bypass menu creation
	return CreateClient( lpcs, NULL );
}

CMainWnd::~CMainWnd()
{
	theApp.m_pSafeWnd = NULL;
	theApp.m_pMainWnd = NULL;
}

void CMainWnd::SaveState()
{
	if ( ! IsIconic() )
		SaveBarState( _T("Toolbars\\CoolBar") );

	Settings.Toolbars.ShowRemote = ( m_wndRemoteWnd.IsVisible() != FALSE );
	Settings.Toolbars.ShowMonitor = ( m_wndMonitorBar.IsVisible() != FALSE );

	Settings.SaveWindow( _T("CMainWnd"), this );

	m_pWindows.SaveWindowStates();
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd create window

BOOL CMainWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.lpszClass = CLIENT_HWND;

	if ( cs.hInstance )
	{
		WNDCLASS wndcls = {};
		wndcls.style			= CS_PARENTDC | CS_DBLCLKS;
		wndcls.lpfnWndProc		= AfxWndProc;
		wndcls.hInstance		= cs.hInstance;
		wndcls.hIcon			= CoolInterface.ExtractIcon( IDR_MAINFRAME, FALSE, LVSIL_NORMAL );
		wndcls.hCursor			= theApp.LoadStandardCursor( IDC_ARROW );
		wndcls.lpszClassName	= cs.lpszClass;
		VERIFY( AfxRegisterClass( &wndcls ) );
	}

	return TRUE;
}

int CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CMDIFrameWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	// Task Bar

	m_pTaskbar.CoCreateInstance( CLSID_TaskbarList );

	// Tray

	m_pTray.hWnd = GetSafeHwnd();
	m_pTray.hIcon = CoolInterface.ExtractIcon( IDR_MAINFRAME, FALSE, LVSIL_SMALL );

	SnarlRegister();

	// Icon

	SetIcon( CoolInterface.ExtractIcon( IDR_MAINFRAME, FALSE, LVSIL_NORMAL ), TRUE );
	SetIcon( CoolInterface.ExtractIcon( IDR_MAINFRAME, FALSE, LVSIL_SMALL  ), FALSE );

	// Status Bar

	const UINT wID[ 3 ] = { ID_SEPARATOR, ID_SEPARATOR, ID_SEPARATOR };
	if ( ! m_wndStatusBar.Create( this ) ) return -1;
	m_wndStatusBar.SetIndicators( wID, _countof( wID ) );
	m_wndStatusBar.SetPaneInfo( 0, ID_SEPARATOR, SBPS_STRETCH, 0 );
	m_wndStatusBar.SetPaneInfo( 1, ID_SEPARATOR, SBPS_NORMAL, 200 );
	m_wndStatusBar.SetPaneInfo( 2, ID_SEPARATOR, SBPS_NORMAL, 200 );

	EnableDocking( CBRS_ALIGN_ANY );

	// Menu Bar

	SetMenu( NULL );

	if ( ! m_wndMenuBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_TOP, IDW_MENU_BAR ) ) return -1;
	m_wndMenuBar.SetWindowText( _T("Menubar") );
	m_wndMenuBar.EnableDocking( CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM );
	DockControlBar( &m_wndMenuBar, AFX_IDW_DOCKBAR_TOP );

	// Nav Bar

	if ( ! m_wndNavBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_TOP, IDW_NAV_BAR ) ) return -1;
	m_wndNavBar.SetWindowText( _T("Navigation Bar") );
	m_wndNavBar.EnableDocking( CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM );
	m_wndNavBar.SetBarStyle( m_wndNavBar.GetBarStyle() | CBRS_TOOLTIPS );
	DockControlBar( &m_wndNavBar, AFX_IDW_DOCKBAR_TOP );
	ShowControlBar( &m_wndNavBar, FALSE, FALSE );

	// Tool Bar

	m_wndToolBar.EnableDrop();
	if ( ! m_wndToolBar.Create( this, WS_CHILD|CBRS_TOP, IDW_TOOL_BAR ) ) return -1;
	m_wndToolBar.SetWindowText( _T("Toolbar") );
	m_wndToolBar.EnableDocking( CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM );
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS );
	m_wndToolBar.SetGripper( TRUE );
	DockControlBar( &m_wndToolBar, AFX_IDW_DOCKBAR_TOP );
	ShowControlBar( &m_wndToolBar, FALSE, FALSE );

	// Tab Bar

	if ( ! m_wndTabBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_BOTTOM, IDW_TAB_BAR ) ) return -1;
	m_wndTabBar.SetWindowText( _T("Windows") );
	m_wndTabBar.EnableDocking( CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM );
	m_wndTabBar.SetBarStyle( m_wndTabBar.GetBarStyle() | CBRS_TOOLTIPS );
	DockControlBar( &m_wndTabBar, AFX_IDW_DOCKBAR_TOP );

	// Monitor Bar

	if ( ! m_wndMonitorBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_BOTTOM, IDW_MONITOR_BAR ) ) return -1;
	m_wndMonitorBar.m_pSnapBar[0] = &m_wndNavBar;
	m_wndMonitorBar.m_pSnapBar[1] = &m_wndToolBar;
	m_wndMonitorBar.SetWindowText( _T("Monitor") );
	m_wndMonitorBar.EnableDocking( CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM );
	DockControlBar( &m_wndMonitorBar, AFX_IDW_DOCKBAR_TOP );

	// Default Size
	int iXSize, iYSize;
	/*
	//Creates problems in dualview mode
	if ( ( theApp.m_dwWindowsVersion >= 5 ) && (GetSystemMetrics( SM_CMONITORS ) > 1) )
	{	// Multi Monitor
		iXSize = GetSystemMetrics( SM_CXVIRTUALSCREEN );
		iYSize = GetSystemMetrics( SM_CYVIRTUALSCREEN );
	}
	else
	*/
	{	// Single Monitor
		iXSize = GetSystemMetrics( SM_CXSCREEN );
		iYSize = GetSystemMetrics( SM_CYSCREEN );
	}

	SetWindowPos( NULL, iXSize  * 1 / 10,
						iYSize * 1 / 10,
						iXSize  * 8 / 10,
						iYSize * 8 / 10, 0 );

	// Disable bars themes
	if ( CWnd* pMDIClientWnd = GetWindow( GW_CHILD ) )
	if ( CWnd* pStatusbarWnd = pMDIClientWnd->GetWindow( GW_HWNDNEXT ) )
	if ( CWnd* pBar1 = pStatusbarWnd->GetWindow( GW_HWNDNEXT ) )
	if ( CWnd* pBar2 = pBar1->GetWindow( GW_HWNDNEXT ) )
	if ( CWnd* pBar3 = pBar2->GetWindow( GW_HWNDNEXT ) )
	if ( CWnd* pBar4 = pBar3->GetWindow( GW_HWNDNEXT ) )
	{
		CoolInterface.EnableTheme( pBar1, FALSE );
		CoolInterface.EnableTheme( pBar2, FALSE );
		CoolInterface.EnableTheme( pBar3, FALSE );
		CoolInterface.EnableTheme( pBar4, FALSE );
	}

	// Plugins

	Plugins.Enumerate();

	// Window Setup

	Settings.LoadWindow( _T("CMainWnd"), this );

	LoadBarState( _T("Toolbars\\CoolBar") );

	if ( ! m_wndMenuBar.IsVisible() ) ShowControlBar( &m_wndMenuBar, TRUE, TRUE );
	if ( ! m_wndNavBar.IsVisible() && ! m_wndToolBar.IsVisible() )
	{
		ShowControlBar( &m_wndNavBar, Settings.General.GUIMode != GUI_WINDOWED, TRUE );
		ShowControlBar( &m_wndToolBar, Settings.General.GUIMode == GUI_WINDOWED, TRUE );
	}
	if ( ! m_wndTabBar.IsVisible() ) ShowControlBar( &m_wndTabBar, TRUE, FALSE );

	if ( Settings.Toolbars.ShowRemote )
		m_wndRemoteWnd.Create( &m_wndMonitorBar );

	m_pWindows.SetOwner( this );
	SetGUIMode( Settings.General.GUIMode, FALSE );

	// Boot

	if ( ! Settings.Windows.RunWizard )
	{
		PostMessage( WM_COMMAND, ID_TOOLS_WIZARD );
	}
	else if ( ! Settings.Windows.RunWarnings )
	{
		PostMessage( WM_COMMAND, ID_HELP_WARNINGS );
	}
	else if ( ! Settings.Windows.RunPromote )
	{
		PostMessage( WM_COMMAND, ID_HELP_PROMOTE );
	}

	// If it is the first run we will connect only in the QuickStart Wizard
	if (  Settings.Connection.AutoConnect &&
		! Settings.Live.FirstRun )
	{
		PostMessage( WM_COMMAND, ID_NETWORK_CONNECT );
	}

	Settings.Live.LoadWindowState = TRUE;

	// Go

	SetTimer( 1, 1000, NULL );

	ENABLE_DROP()

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd destroy window

void CMainWnd::OnClose()
{
	CWaitCursor pCursor;

	if ( theApp.m_bBusy )
	{
		// Delayed close
		if ( ! theApp.m_bClosing )
		{
			SetTimer( 2, 1000, NULL );
			return;
		}
	}

	if ( theApp.m_bClosing )
		// Already closing
		return;
	theApp.m_bClosing = true;

	theApp.m_pSafeWnd = NULL;

	DISABLE_DROP()

	KillTimer( 1 );
	KillTimer( 2 );

	DeleteTray();

	SaveState();

	theApp.HideApplication();

	RemoveSkin();

	theApp.SplashStep( L"Closing GUI", 7, true );

	LibraryBuilder.StopThread();
	Library.StopThread();

	m_pWindows.SaveSearchWindows();
	m_pWindows.SaveBrowseHostWindows();
	m_pWindows.Close();

	CDownloadMonitorDlg::CloseAll();
	CFilePreviewDlg::CloseAll();

	Network.Disconnect();
	Transfers.StopThread();
	ChatCore.Close();

	if ( m_wndRemoteWnd.IsVisible() ) m_wndRemoteWnd.DestroyWindow();

	m_brshDockbar.DeleteObject();

	m_pTaskbar.Release();

	// Destroy main window
	CMDIFrameWnd::OnClose();
}

void CMainWnd::RemoveSkin()
{
	m_pSkin = NULL;
	m_pWindows.PostSkinRemove();
	CDownloadMonitorDlg::OnSkinChange( FALSE );
	CSettingsManagerDlg::OnSkinChange( FALSE );
	CFilePreviewDlg::OnSkinChange( FALSE );
	m_wndRemoteWnd.RemoveSkin();
	m_wndNavBar.RemoveSkin();
}

BOOL CMainWnd::OnQueryEndSession()
{
	UpdateWindow();

	CMDIFrameWnd::OnQueryEndSession();

	return FALSE;
}

void CMainWnd::OnEndSession(BOOL bEnding)
{
	if ( bEnding )
	{
		AfxOleSetUserCtrl( TRUE );    // keeps from randomly shutting down

		SendMessage( WM_CLOSE );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd GUI modes

void CMainWnd::SetGUIMode(int nMode, BOOL bSaveState)
{
	m_pWindows.ShowWindow( SW_HIDE );

	if ( bSaveState )
	{
		if ( nMode < 0 ) nMode = Settings.General.GUIMode;

		ShowControlBar( &m_wndMenuBar, FALSE, TRUE );
		ShowControlBar( &m_wndNavBar, FALSE, TRUE );
		ShowControlBar( &m_wndToolBar, FALSE, TRUE );
		ShowControlBar( &m_wndTabBar, FALSE, TRUE );
		ShowControlBar( &m_wndMonitorBar, FALSE, TRUE );
	}

	m_pWindows.SetGUIMode( nMode, bSaveState );
	OnSkinChanged( 0, 0 );

	if ( bSaveState )
	{
		DockControlBar( &m_wndMenuBar, AFX_IDW_DOCKBAR_TOP );
		ShowControlBar( &m_wndMenuBar, TRUE, TRUE );

		if ( nMode != GUI_WINDOWED && m_wndNavBar.HasLocalVersion() )
		{
			DockControlBar( &m_wndNavBar, AFX_IDW_DOCKBAR_TOP );
			ShowControlBar( &m_wndNavBar, TRUE, TRUE );
		}
		else
		{
			DockControlBar( &m_wndToolBar, AFX_IDW_DOCKBAR_TOP );
			ShowControlBar( &m_wndToolBar, TRUE, TRUE );
		}

		DockControlBar( &m_wndTabBar, nMode == GUI_WINDOWED ? AFX_IDW_DOCKBAR_BOTTOM : AFX_IDW_DOCKBAR_TOP );
		ShowControlBar( &m_wndTabBar, TRUE, TRUE );
	}

	m_wndTabBar.SetMaximumWidth( nMode != GUI_WINDOWED ? 200 : 140 );
	m_wndTabBar.SetMessage( (UINT)0 );

	if ( bSaveState )
	{
		CRect rcWnd, rcBar;

		GetWindowRect( &rcWnd );
		if ( m_wndNavBar.IsVisible() )
			m_wndNavBar.GetWindowRect( &rcBar );
		else
			m_wndToolBar.GetWindowRect( &rcBar );
		rcBar.left	= rcWnd.right - 128;
		rcBar.right	= rcWnd.right;

		DockControlBar( &m_wndMonitorBar, AFX_IDW_DOCKBAR_TOP, &rcBar );
		ShowControlBar( &m_wndMonitorBar, nMode == GUI_WINDOWED, TRUE );
	}

	m_pWindows.ShowWindow( SW_SHOW );
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd command architecture

BOOL CMainWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndMonitorBar.m_hWnd != NULL )
	{
		if ( m_wndMonitorBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
			return TRUE;
	}

	if ( CMediaFrame* pMediaFrame = CMediaFrame::GetMediaFrame() )
	{
		if ( pMediaFrame->OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
			return TRUE;
	}

	CMDIChildWnd* pActiveChild = MDIGetActive();
	// pump through active child FIRST
	if ( pActiveChild != NULL )
	{
		CPushRoutingFrame push(this);
		if ( pActiveChild->OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
			return TRUE;
	}

	// then pump through normal frame
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CMainWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if ( Plugins.OnCommand( m_pWindows.GetActive(), LOWORD( wParam ) ) ) return TRUE;
	return CMDIFrameWnd::OnCommand( wParam, lParam );
}

void CMainWnd::OnUpdatePluginRange(CCmdUI* pCmdUI)
{
	if ( ! Plugins.OnUpdate( m_pWindows.GetActive(), pCmdUI ) ) pCmdUI->Enable( FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd menu GUI

void CMainWnd::OnUpdateShell(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( TRUE );
}

void CMainWnd::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CMDIFrameWnd::OnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );

	CoolMenu.OnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );
}

void CMainWnd::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	CoolMenu.OnMeasureItem( lpMeasureItemStruct );
}

void CMainWnd::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CoolMenu.OnDrawItem( lpDrawItemStruct );
}

LRESULT CMainWnd::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu)
{
	if ( LRESULT lResult = CoolMenu.OnMenuChar( nChar, nFlags, pMenu ) )
		return lResult;

	return CMDIFrameWnd::OnMenuChar( nChar, nFlags, pMenu );
}

void CMainWnd::OnSysColorChange()
{
	CMDIFrameWnd::OnSysColorChange();
	CoolInterface.OnSysColourChange();
}

void CMainWnd::OnUpdateCmdUI()
{
	m_wndTabBar.OnUpdateCmdUI( this, FALSE );
	m_wndNavBar.OnUpdateCmdUI( this, FALSE );
}

void CMainWnd::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if ( ( pWnd != this || OnNcHitTest( point ) != HTCAPTION ) )
	{
		CMenu* pMenu = Skin.GetMenu( _T("CMainWnd.View") );
		if ( pMenu == NULL ) return;
		pMenu->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this );
	}
}

#define SNAP_SIZE 6

void CMainWnd::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CMDIFrameWnd::OnWindowPosChanging( lpwndpos );

	HMONITOR hMonitor = MonitorFromWindow( GetSafeHwnd(),
		MONITOR_DEFAULTTOPRIMARY );

	MONITORINFO oMonitor = {0};
	oMonitor.cbSize = sizeof( MONITORINFO );
	GetMonitorInfo( hMonitor, &oMonitor );

	if ( abs( lpwndpos->x - oMonitor.rcWork.left ) < SNAP_SIZE )
		lpwndpos->x = oMonitor.rcWork.left;
	if ( abs( lpwndpos->y - oMonitor.rcWork.top ) < SNAP_SIZE )
		lpwndpos->y = oMonitor.rcWork.top;
	if ( abs( lpwndpos->x + lpwndpos->cx - oMonitor.rcWork.right ) < SNAP_SIZE )
		lpwndpos->x = oMonitor.rcWork.right - lpwndpos->cx;
	if ( abs( lpwndpos->y + lpwndpos->cy - oMonitor.rcWork.bottom ) < SNAP_SIZE )
		lpwndpos->y = oMonitor.rcWork.bottom - lpwndpos->cy;
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd common timer

void CMainWnd::OnTimer(UINT_PTR nIDEvent)
{
	CMDIFrameWnd::OnTimer( nIDEvent );

	DWORD tNow = static_cast< DWORD >( time( NULL ) );
	static DWORD tLast60SecInterval = 0;
	static DWORD tLast5SecInterval = 0;

	// Delayed close
	if ( nIDEvent == 2 )
	{
		PostMessage( WM_CLOSE );
		return;
	}

	// Propagate to children

	if ( m_bTimer )
		return;
	m_bTimer = TRUE;

	for ( POSITION pos = m_pWindows.GetIterator() ; pos ; )
	{
		CChildWnd* pChild = m_pWindows.GetNext( pos );
		pChild->PostMessage( WM_TIMER, 1, 0 );
	}

	m_bTimer = FALSE;
	Settings.Live.LoadWindowState = FALSE;

	// Statistics

	Statistics.Update();

	// Hashing progress window

	m_wndHashProgressBar.Run();

	// Switch tray icon
	if ( m_bTrayHide ||
		Settings.General.TrayMinimise ||
		Settings.General.CloseMode == 2 )
	{
		AddTray();
	}
	else if ( ! m_bTrayNotify )
	{
		DeleteTray();
	}

	if ( tNow - tLast5SecInterval > 5 )
	{
		tLast5SecInterval = tNow;

		// Periodic cleanup
		PurgeDeletes();

		if ( tNow - tLast60SecInterval > 60 )
		{
			tLast60SecInterval = tNow;

			// Periodic saves
			SaveState();

			// Network / disk space / directory checks
			LocalSystemChecks();
		}

		m_bTrayUpdate = TRUE;
	}

	// Update messages
	UpdateMessages();
}

void CMainWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CMDIFrameWnd::OnActivate( nState, pWndOther, bMinimized );

	if ( nState != WA_INACTIVE )
	{
		if ( CChildWnd* pChildWnd = m_pWindows.GetActive() )
		{
			pChildWnd->SendMessage( WM_MDIACTIVATE, NULL, (LPARAM)pChildWnd->GetSafeHwnd() );
		}
	}
}

void CMainWnd::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	CMDIFrameWnd::OnGetMinMaxInfo( lpMMI );

	lpMMI->ptMinTrackSize.x = 320;
	lpMMI->ptMinTrackSize.y = 240;

	if ( m_pSkin )
		m_pSkin->OnGetMinMaxInfo( lpMMI );
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd tray functionality

void CMainWnd::AddTray()
{
	if ( ! m_bTrayIcon )
	{
		// Delete existing tray icon (if any), windows can't create a new icon with same uID
		Shell_NotifyIcon( NIM_DELETE, &m_pTray );

		m_pTray.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

		_tcsncpy( m_pTray.szTip, Settings.SmartAgent(), _countof( m_pTray.szTip ) - 1 );
		m_pTray.szTip[ _countof( m_pTray.szTip ) - 1 ] = _T('\0');

		m_bTrayUpdate = TRUE;
		m_bTrayIcon = Shell_NotifyIcon( NIM_ADD, &m_pTray );
		Shell_NotifyIcon( NIM_SETVERSION, &m_pTray );
	}
}

void CMainWnd::DeleteTray()
{
	if ( m_bTrayIcon )
	{
		Shell_NotifyIcon( NIM_DELETE, &m_pTray );
		m_bTrayIcon = FALSE;
		m_bTrayNotify = FALSE;
	}
}

void CMainWnd::CloseToTray()
{
	if ( ! IsIconic() )
		ShowWindow( SW_MINIMIZE );

	if ( IsWindowVisible() )
		ShowWindow( SW_HIDE );

	m_bTrayHide = TRUE;
}

void CMainWnd::OpenFromTray(int nShowCmd)
{
	if ( ! IsWindowVisible() )
		ShowWindow( nShowCmd );

	if ( IsIconic() )
		ShowWindow( SW_RESTORE );

	SetForegroundWindow();
	m_bTrayHide = FALSE;
}

LRESULT CMainWnd::OnTray(WPARAM /*wParam*/, LPARAM lParam)
{
	switch ( LOWORD( lParam ) )
	{
	case WM_MOUSEMOVE:
		m_bTrayUpdate = TRUE;
		break;

	case WM_LBUTTONDBLCLK:
		if ( m_bTrayHide )
			OpenFromTray();
		else
			CloseToTray();
		break;

	case WM_RBUTTONDOWN:
		OpenTrayMenu();
		break;

	case NIN_BALLOONHIDE:
	case NIN_BALLOONTIMEOUT:
	case NIN_BALLOONUSERCLICK:
		m_bTrayNotify = FALSE;
		break;
	}

	return 0;
}

void CMainWnd::OpenTrayMenu()
{
	CPoint pt;
	CRect rc;

	GetCursorPos( &pt );
	SystemParametersInfo( SPI_GETWORKAREA, 0, &rc, 0 );

	UINT nFlags = TPM_CENTERALIGN | TPM_RIGHTBUTTON;

	if ( pt.y > GetSystemMetrics( SM_CYSCREEN ) / 2 )
	{
		nFlags |= TPM_TOPALIGN;
	}
	else
	{
		nFlags |= TPM_BOTTOMALIGN;
	}

	SetForegroundWindow();

	CMenu* pMenu = Skin.GetMenu( _T("CMainWnd.Tray") );
	if ( pMenu == NULL )
		return;

	MENUITEMINFO pInfo = {};
	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= MIIM_STATE;
	GetMenuItemInfo( pMenu->GetSafeHmenu(), ID_TRAY_OPEN, FALSE, &pInfo );
	pInfo.fState	|= MFS_DEFAULT;
	SetMenuItemInfo( pMenu->GetSafeHmenu(), ID_TRAY_OPEN, FALSE, &pInfo );

	pMenu->TrackPopupMenu( nFlags, pt.x, pt.y, this, NULL );

	PostMessage( WM_NULL );

	Shell_NotifyIcon( NIM_SETFOCUS, &m_pTray );
}

void CMainWnd::OnTrayOpen()
{
	OpenFromTray();
}

LRESULT CMainWnd::OnChangeAlpha(WPARAM wParam, LPARAM /*lParam*/)
{
	if ( wParam == 1 ) // Increase transparency
	{
		m_nAlpha += 5;
		m_nAlpha = min( 255ul, m_nAlpha );
	}
	else if ( m_nAlpha > 0 )
	{
		m_nAlpha -= 5;
		m_nAlpha = max( 0ul, m_nAlpha );
	}

	ModifyStyleEx( 0, WS_EX_LAYERED );
	SetLayeredWindowAttributes( 0, (BYTE)m_nAlpha, LWA_ALPHA );

	CWnd* pWndPopup = (CWnd*)GetWindow( GW_ENABLEDPOPUP );
	if ( pWndPopup == NULL ) return 0;

	if ( m_nAlpha == 0 )
	{
		pWndPopup->ModifyStyleEx( WS_EX_LAYERED, 0 );
		pWndPopup->RedrawWindow( NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN );
	}
	else
	{
		pWndPopup->ModifyStyleEx( 0, WS_EX_LAYERED );
		::SetLayeredWindowAttributes( pWndPopup->GetSafeHwnd(), 0, (BYTE)m_nAlpha, LWA_ALPHA );
	}

	return 0;
}

void CMainWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ( ( nID & 0xFFF0 ) == SC_KEYMENU )
	{
		if ( lParam != ' ' && lParam != '-' )
		{
			if ( lParam )
				m_wndMenuBar.OpenMenuChar( static_cast< UINT >( lParam ) );
			else
				m_wndMenuBar.OpenMenuBar();

			return;
		}
	}

	if ( m_bTrayHide )
	{
		switch ( nID & 0xFFF0 )
		{
		case SC_RESTORE:
			OpenFromTray( SW_SHOWNORMAL );
			return;
		case SC_MAXIMIZE:
			OpenFromTray( SW_SHOWMAXIMIZED );
			return;
		}
	}
	else
	{
		BOOL bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 );

		switch ( nID & 0xFFF0 )
		{
		case SC_CLOSE:
			if ( Settings.General.CloseMode == 0 )
			{
				CCloseModeDlg dlg;
				if ( dlg.DoModal() != IDOK ) return;
			}

			if ( Settings.General.CloseMode == 3 && ! bShift )
			{
				if ( Settings.Live.AutoClose )
					CloseToTray();
				else
					OnNetworkAutoClose();
				return;
			}
			else if ( Settings.General.CloseMode == 2 && ! bShift )
			{
				CloseToTray();
				return;
			}
			break;
		case SC_MINIMIZE:
			if ( ( Settings.General.TrayMinimise && ! bShift ) || ( ! Settings.General.TrayMinimise && bShift ) )
			{
				CloseToTray();
				return;
			}
			break;
		}
	}

	CMDIFrameWnd::OnSysCommand( nID, lParam );
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd custom message handlers

LRESULT CMainWnd::OnSkinChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	CWaitCursor pCursor;

	LockWindowUpdate();

	if ( m_pSkin )
		CoolInterface.EnableTheme( this, TRUE );

	RemoveSkin();

	m_wndMenuBar.SetMenu( NULL );
	m_wndToolBar.Clear();

	Skin.Apply();

	if ( CMenu* pMenu = Skin.GetMenu( _T("CMainWnd") ) )
	{
		m_wndMenuBar.SetMenu( pMenu->GetSafeHmenu() );
	}

	Skin.CreateToolBar( _T("CMainWnd"), &m_wndToolBar );
	m_wndNavBar.OnSkinChange();

	m_wndMenuBar.SetWatermark( Skin.GetWatermark( _T("CCoolMenuBar") ) );
	m_wndTabBar.OnSkinChange();

	if ( CWnd* pDockBar = GetDlgItem( AFX_IDW_DOCKBAR_TOP ) )
	{
		m_brshDockbar.DeleteObject();
		m_brshDockbar.CreateSolidBrush( CoolInterface.m_crMidtone );
		SetClassLongPtr( pDockBar->GetSafeHwnd(), GCLP_HBRBACKGROUND,
			(LONG_PTR)(HBRUSH)m_brshDockbar );
	}

	m_pSkin = Skin.GetWindowSkin( this );

	if ( m_pSkin )
		CoolInterface.EnableTheme( this, FALSE );

	SetWindowRgn( NULL, TRUE );

	if ( m_pSkin )
		m_pSkin->OnSize( this );

	m_wndRemoteWnd.OnSkinChange();
	m_wndMonitorBar.OnSkinChange();
	if ( Settings.Toolbars.ShowMonitor )
	{
		// A quick workaround to show a monitor bar when skin or GUI mode is changed
		if ( !m_wndMonitorBar.IsVisible() && Settings.General.GUIMode != GUI_WINDOWED )
			PostMessage( WM_COMMAND, ID_WINDOW_MONITOR );
	}

	SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|
		SWP_NOACTIVATE|SWP_NOZORDER|SWP_FRAMECHANGED|SWP_DRAWFRAME );

	UpdateMessages();

	m_wndToolBar.OnUpdated();
	m_pWindows.PostSkinChange();
	m_wndTabBar.OnUpdateCmdUI( this, FALSE );
	RedrawWindow( NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_ALLCHILDREN );

	CSettingsManagerDlg::OnSkinChange( TRUE );
	CDownloadMonitorDlg::OnSkinChange( TRUE );
	CFilePreviewDlg::OnSkinChange( TRUE );

	Invalidate();
	UnlockWindowUpdate();
	UpdateWindow();

	if ( theApp.m_bLive )
	{
		// Update shell icons
		LibraryFolders.Maintain();
		CShareazaURL::Register();
	}

	return 0;
}

LRESULT CMainWnd::OnWinsock(WPARAM wParam, LPARAM lParam)
{
	Network.OnWinsock( wParam, lParam );
	return 0;
}

LRESULT CMainWnd::OnHandleURL(WPARAM wParam, LPARAM /*lParam*/)
{
	CWaitCursor wc;

	UpdateWindow();

	new CURLActionDlg( (CShareazaURL*)wParam );

	return 0;
}

LRESULT CMainWnd::OnHandleImport(WPARAM wParam, LPARAM /*lParam*/)
{
	LPTSTR pszPath = (LPTSTR)wParam;

	CWaitCursor wc;
	HostCache.Import( pszPath );

	delete [] pszPath;

	if ( CHostCacheWnd* pCacheWnd = (CHostCacheWnd*)m_pWindows.Open( RUNTIME_CLASS(CHostCacheWnd) ) )
	{
		pCacheWnd->Update( TRUE );
	}

	return 0;
}

LRESULT CMainWnd::OnHandleCollection(WPARAM wParam, LPARAM /*lParam*/)
{
	LPTSTR pszPath = (LPTSTR)wParam;
	CString strPath( pszPath );
	delete [] pszPath;

	OpenFromTray();

	if ( CLibraryWnd* pLibrary = CLibraryWnd::GetLibraryWindow() )
	{
		pLibrary->OnCollection( strPath );
	}

	return 0;
}

LRESULT CMainWnd::OnHandleTorrent(WPARAM wParam, LPARAM /*lParam*/)
{
	LPTSTR pszPath = (LPTSTR)wParam;
	CString strPath( pszPath );
	delete [] pszPath;

	CTorrentSeedDlg dlg( strPath );
	dlg.DoModal();

	return 0;
}

LRESULT CMainWnd::OnVersionCheck(WPARAM wParam, LPARAM /*lParam*/)
{
	if ( wParam == VC_MESSAGE_AND_CONFIRM && VersionChecker.m_sMessage.GetLength() )
	{
		AfxMessageBox( VersionChecker.m_sMessage, MB_ICONINFORMATION | MB_OK );
	}

	if ( wParam == VC_MESSAGE_AND_CONFIRM || wParam == VC_CONFIRM )
	{
		// Check for already downloaded file
		if ( VersionChecker.CheckUpgradeHash() )
			;
		else if ( VersionChecker.IsUpgradeAvailable() )
		{
			CUpgradeDlg dlg;
			dlg.DoModal();
		}
		else
		{
			CString strMessage;
			LoadString( strMessage, IDS_UPGRADE_NO_NEW );

			ShowTrayPopup( strMessage );

			if ( VersionChecker.IsVerbose() )
				AfxMessageBox( strMessage, MB_ICONINFORMATION | MB_OK );
		}
	}

	if ( wParam == VC_UPGRADE && VersionChecker.m_sUpgradePath.GetLength() )
	{
		CString strFormat, strMessage;
		LoadString( strFormat, IDS_UPGRADE_LAUNCH );
		strMessage.Format( strFormat, (LPCTSTR)Settings.VersionCheck.UpgradeFile );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
		{
			ShellExecute( GetSafeHwnd(), _T("open"), VersionChecker.m_sUpgradePath,
				_T("/launch"), NULL, SW_SHOWNORMAL );
			PostMessage( WM_CLOSE );
		}
		else
			// Postponed till next session
			Settings.VersionCheck.NextCheck = 0;
	}

	return 0;
}

LRESULT CMainWnd::OnOpenChat(WPARAM wParam, LPARAM /*lParam*/)
{
	CSingleLock pLock( &ChatCore.m_pSection, TRUE );
	CChatSession* pSession = (CChatSession*)wParam;
	if ( ChatCore.Check( pSession ) ) pSession->OnOpenWindow();
	return 0;
}

// Used from Remote pages when the search is performed. Receives WM_OPENSEARCH message
LRESULT CMainWnd::OnOpenSearch(WPARAM wParam, LPARAM /*lParam*/)
{
	CQuerySearchPtr pSearch;
	pSearch.Attach( (CQuerySearch*)wParam );
	CQuerySearch::OpenWindow( pSearch );
	m_wndTabBar.OnSkinChange();
	return 0;
}

LRESULT CMainWnd::OnMediaKey(WPARAM /*wParam*/, LPARAM lParam)
{
	if ( CMediaWnd* pWnd = (CMediaWnd*)m_pWindows.Find( RUNTIME_CLASS(CMediaWnd) ) )
	{
		return pWnd->SendMessage( 0x0319, 1, lParam );
	}
	return 0;
}

LRESULT CMainWnd::OnDevModeChange(WPARAM wParam, LPARAM lParam)
{
	if ( CMediaWnd* pWnd = (CMediaWnd*)m_pWindows.Find( RUNTIME_CLASS(CMediaWnd) ) )
	{
		return pWnd->SendMessage( WM_DEVMODECHANGE, wParam, lParam );
	}
	return 0;
}

LRESULT CMainWnd::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	if ( CMediaWnd* pWnd = (CMediaWnd*)m_pWindows.Find( RUNTIME_CLASS(CMediaWnd) ) )
	{
		return pWnd->SendMessage( WM_DISPLAYCHANGE, wParam, lParam );
	}
	return 0;
}

LRESULT CMainWnd::OnLibrarySearch(WPARAM wParam, LPARAM /*lParam*/)
{
	OnTabLibrary();

	LRESULT result = 0;
	LPCTSTR pszSearch = (LPCTSTR)wParam;

	if ( CLibraryWnd* pWnd = CLibraryWnd::GetLibraryWindow() )
	{
		pWnd->m_wndFrame.SetSearchText( pszSearch );
		result = pWnd->m_wndFrame.SendMessage( WM_COMMAND, ID_LIBRARY_SEARCH_QUICK );
	}
	delete pszSearch;
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd status message functionality

LRESULT CMainWnd::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	if ( wParam == AFX_IDS_IDLEMESSAGE )
	{
		if ( IsForegroundWindow() )
		{
			CString strOld;
			m_wndStatusBar.GetPaneText( 0, strOld );
			if ( strOld != m_sMsgStatus )
				m_wndStatusBar.SetPaneText( 0, m_sMsgStatus );
		}

		m_nIDLastMessage = m_nIDTracking = AFX_IDS_IDLEMESSAGE;

		return 0;
	}
	else
	{
		return CMDIFrameWnd::OnSetMessageString( wParam, lParam );
	}
}

void CMainWnd::GetMessageString(UINT nID, CString& rMessage) const
{
	if ( LoadString( rMessage, nID ) )
	{
		int nPos = rMessage.Find( '\n' );
		if ( nPos >= 0 ) rMessage.SetAt( nPos, 0 );
	}
}

void CMainWnd::UpdateMessages()
{
	// StatusBar
	if ( IsForegroundWindow() )
	{
		CString strStatusbar, strOld;

		QWORD nLocalVolume;
		LibraryMaps.GetStatistics( NULL, &nLocalVolume );

		if ( Network.IsWellConnected() )
		{
			// If you have neighbours, you are connected
			if ( Settings.General.GUIMode == GUI_BASIC )
			{
				// In the basic GUI, don't bother with mode details or neighbour count.
				strStatusbar.Format( LoadString( IDS_STATUS_BAR_CONNECTED_SIMPLE ), (LPCTSTR)Settings.SmartVolume( nLocalVolume, KiloBytes ) );
			}
			else
			{
				// Display node type and number of neighbours
				strStatusbar.Format( LoadString( Neighbours.IsG2Hub() ?
					( Neighbours.IsG1Ultrapeer() ? IDS_STATUS_BAR_CONNECTED_HUB_UP : IDS_STATUS_BAR_CONNECTED_HUB ) :
					( Neighbours.IsG1Ultrapeer() ? IDS_STATUS_BAR_CONNECTED_UP : IDS_STATUS_BAR_CONNECTED ) ),
					Neighbours.GetStableCount(), (LPCTSTR)Settings.SmartVolume( nLocalVolume, KiloBytes ) );
			}
		}
		else if ( Network.IsConnected() )
		{
			// If G1, G2, eDonkey are disabled and only BitTorrent is enabled say connected
			if( ! Settings.Gnutella1.EnableToday &&
				! Settings.Gnutella2.EnableToday &&
				! Settings.eDonkey.EnableToday &&
				! Settings.DC.EnableToday )
			{
				strStatusbar.Format( LoadString( IDS_STATUS_BAR_CONNECTED_SIMPLE ), (LPCTSTR)Settings.SmartVolume( nLocalVolume, KiloBytes ) );
			}
			else
				// Trying to connect
				strStatusbar = LoadString( IDS_STATUS_BAR_CONNECTING );
		}
		else
			// Idle
			strStatusbar = LoadString( IDS_STATUS_BAR_DISCONNECTED );

		if ( Settings.VersionCheck.Quote.GetLength() )
		{
			strStatusbar += _T("  ");
			strStatusbar += Settings.VersionCheck.Quote;
		}

		m_sMsgStatus = strStatusbar;

		if ( m_nIDLastMessage == AFX_IDS_IDLEMESSAGE )
		{
			m_wndStatusBar.GetPaneText( 0, strOld );
			if ( strOld != strStatusbar )
				m_wndStatusBar.SetPaneText( 0, m_sMsgStatus );
		}

		// StatusBar pane 1
		strStatusbar.Format( LoadString( IDS_STATUS_BAR_BANDWIDTH ),
			(LPCTSTR)Settings.SmartSpeed( CGraphItem::GetValue( GRC_TOTAL_BANDWIDTH_IN ), bits ),
			(LPCTSTR)Settings.SmartSpeed( CGraphItem::GetValue( GRC_TOTAL_BANDWIDTH_OUT ), bits ),
			CGraphItem::GetValue( GRC_DOWNLOADS_TRANSFERS ),
			CGraphItem::GetValue( GRC_UPLOADS_TRANSFERS ) );
		m_wndStatusBar.GetPaneText( 1, strOld );
		if ( strOld != strStatusbar )
			m_wndStatusBar.SetPaneText( 1, strStatusbar );

		// StatusBar pane 2
		strStatusbar = HostToString( &Network.m_pHost );
		m_wndStatusBar.GetPaneText( 2, strOld );
		if ( strOld != strStatusbar )
			m_wndStatusBar.SetPaneText( 2, strStatusbar );
	}

	// Tray
	if ( m_bTrayIcon && m_bTrayUpdate )
	{
		m_bTrayUpdate = FALSE;

		CString strTip;
		strTip.Format( LoadString( IDS_TRAY_TIP ),
			CGraphItem::GetValue( GRC_GNUTELLA_CONNECTIONS ),
			(LPCTSTR)Settings.SmartSpeed( CGraphItem::GetValue( GRC_TOTAL_BANDWIDTH_IN ), bits ),
			(LPCTSTR)Settings.SmartSpeed( CGraphItem::GetValue( GRC_TOTAL_BANDWIDTH_OUT ), bits ),
			CGraphItem::GetValue( GRC_DOWNLOADS_TRANSFERS ),
			CGraphItem::GetValue( GRC_UPLOADS_TRANSFERS ) );
		strTip = Settings.SmartAgent() + _T("\r\n") + strTip;

		m_pTray.uFlags = NIF_TIP;
		_tcsncpy( m_pTray.szTip, strTip, _countof( m_pTray.szTip ) - 1 );
		m_pTray.szTip[ _countof( m_pTray.szTip ) - 1 ] = _T('\0');
		m_bTrayIcon = Shell_NotifyIcon( NIM_MODIFY, &m_pTray );
	}

	// Task Bar
	if ( ! m_bTrayHide && m_pTaskbar )
	{
		CString sAppBarTip;
		HWND hWnd = GetSafeHwnd();

		QWORD nTotal = Downloads.m_nTotal, nComplete = Downloads.m_nComplete;
		if ( nTotal && nComplete != nTotal )
		{
			m_pTaskbar->SetProgressState( hWnd, TBPF_NORMAL );
			m_pTaskbar->SetProgressValue( hWnd, nComplete, nTotal );
			sAppBarTip.Format( _T("%s\r\n%s %.2f%%\r\n%s %s"), (LPCTSTR)Settings.SmartAgent(),
				(LPCTSTR)LoadString( IDS_DLM_VOLUME_DOWNLOADED ), float( ( 10000 * nComplete ) / nTotal ) / 100.f,
				(LPCTSTR)LoadString( IDS_DLM_TOTAL_SPEED ), (LPCTSTR)Settings.SmartSpeed( CGraphItem::GetValue( GRC_TOTAL_BANDWIDTH_IN ), bits ) );
		}
		else
		{
			m_pTaskbar->SetProgressState( hWnd, TBPF_NOPROGRESS );
			sAppBarTip = Settings.SmartAgent();
		}
		m_pTaskbar->SetThumbnailTooltip( hWnd, sAppBarTip );
	}
}

// This function runs some basic checks that everything is okay- disks, directories, local network is
// up, etc.
void CMainWnd::LocalSystemChecks()
{
	static DWORD nConnectionFailCount = 0;	// Counter for times a connection problem has been detected


	// Check disk space
	if ( Settings.Live.DiskSpaceStop == FALSE )
	{
		if ( ! Downloads.IsSpaceAvailable( (QWORD)Settings.General.DiskSpaceStop * 1024 * 1024 ) )
		{
			CSingleLock pLock( &Transfers.m_pSection );
			if ( pLock.Lock( 250 ) )
			{
				Settings.Live.DiskSpaceStop = TRUE;
				Downloads.PauseAll();
			}

		}
	}
	if ( Settings.Live.DiskSpaceWarning == FALSE )
	{
		if ( ! Downloads.IsSpaceAvailable( (QWORD)Settings.General.DiskSpaceWarning * 1024 * 1024 ) )
		{
			Settings.Live.DiskSpaceWarning = TRUE;
			PostMessage( WM_COMMAND, ID_HELP_DISKSPACE );
		}
	}


	// Check disk/directory exists and isn't read-only
	if ( Settings.Live.DiskWriteWarning == FALSE )
	{
		DWORD nCompleteAttributes, nIncompleteAttributes;
		nCompleteAttributes = GetFileAttributes( Settings.Downloads.CompletePath );
		nIncompleteAttributes = GetFileAttributes( Settings.Downloads.IncompletePath );

		if ( ( nCompleteAttributes == INVALID_FILE_ATTRIBUTES ) ||  ( nIncompleteAttributes == INVALID_FILE_ATTRIBUTES ) ||
			!( nCompleteAttributes & FILE_ATTRIBUTE_DIRECTORY ) || !( nIncompleteAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
		{
			Settings.Live.DiskWriteWarning = TRUE;
			PostMessage( WM_COMMAND, ID_HELP_DISKWRITEFAIL );
		}
		else
		{
/*
// Note: These checks fail on some machines. WinXP goofyness?
			// Extra NT/Win2000/XP permission checks
			if ( ( _taccess( Settings.Downloads.IncompletePath, 06 ) != 0 ) ||
				 ( _taccess( Settings.Downloads.CompletePath, 06 ) != 0 ) )
			{
				Settings.Live.DiskWriteWarning = TRUE;
				PostMessage( WM_COMMAND, ID_HELP_DISKWRITEFAIL );
			}
*/
		}
	}


	// Check network connection state
	if ( Settings.Connection.DetectConnectionLoss )
	{
		if ( Network.IsConnected() )
		{
			if ( Network.IsAvailable() || Network.IsWellConnected() )
			{
				// Internet is available
				nConnectionFailCount = 0;
			}
			else
			{
				// Internet may have failed
				nConnectionFailCount++;
				// Give it at least two failures before assuming it's bad
				if ( nConnectionFailCount > 1 )
				{
					if ( Settings.Connection.DetectConnectionReset )
						theApp.Message( MSG_ERROR, _T("Internet disconnection detected- shutting down network") );
					else
						PostMessage( WM_COMMAND, ID_HELP_CONNECTIONFAIL );

					PostMessage( WM_COMMAND, ID_NETWORK_DISCONNECT );
				}
			}
		}
		else
		{
			// We are not currently connected. Check if we disconnected because of failure and want to re-connect.
			if ( ( nConnectionFailCount > 0 ) && ( Settings.Connection.DetectConnectionReset ) )
			{
				// See if we can reconnect
				if ( Network.IsAvailable() )
				{
					nConnectionFailCount = 0;
					PostMessage( WM_COMMAND, ID_NETWORK_CONNECT );
					theApp.Message( MSG_ERROR, _T("Internet reconnect detected- restarting network") );
				}
			}
		}
	}

	// Check we have minimum servers
	if ( ! Settings.Live.DefaultED2KServersLoaded )
	{
		Settings.Live.DefaultED2KServersLoaded = true;
		if ( Settings.eDonkey.EnableToday ) HostCache.CheckMinimumServers( PROTOCOL_ED2K );
	}
	if ( ! Settings.Live.DefaultDCServersLoaded )
	{
		Settings.Live.DefaultDCServersLoaded = true;
		if ( Settings.DC.EnableToday ) HostCache.CheckMinimumServers( PROTOCOL_DC );
	}

	if ( ! Settings.Live.DonkeyServerWarning && Settings.eDonkey.EnableToday )
	{
		Settings.Live.DonkeyServerWarning = true;
		if ( ! Settings.eDonkey.AutoDiscovery && ! HostCache.EnoughServers( PROTOCOL_ED2K ) )
			PostMessage( WM_COMMAND, ID_HELP_DONKEYSERVERS );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd network menu

void CMainWnd::OnUpdateNetworkSearch(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( IsWindowEnabled() );
}

void CMainWnd::OnNetworkSearch()
{
	if ( ! Network.IsWellConnected() ) Network.Connect( TRUE );

	if ( Settings.Search.SearchPanel && IsForegroundWindow() )
	{
		m_pWindows.OpenNewSearchWindow();
		m_wndTabBar.OnSkinChange();
	}
	else
	{
		CNewSearchDlg dlg;
		if ( dlg.DoModal() != IDOK ) return;
		new CSearchWnd( dlg.GetSearch() );
	}

	OpenFromTray();
}

void CMainWnd::OnUpdateNetworkConnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( TRUE );

	UINT nTextID = 0;
	bool bNetworksEnabled = ( Settings.Gnutella1.EnableToday || Settings.Gnutella2.EnableToday || Settings.eDonkey.EnableToday );

	if ( Network.IsConnected() &&
		( Network.IsWellConnected() || !bNetworksEnabled )	// If Network.IsWellConnected() or G1, G2, eDonkey are disabled and only BitTorrent is enabled say connected
	)
	{
		nTextID = IDS_NETWORK_CONNECTED;
		pCmdUI->SetCheck( TRUE );
	}
	else if ( Network.IsConnected() )
	{
		nTextID = IDS_NETWORK_CONNECTING;
		pCmdUI->SetCheck( TRUE );
	}
	else
	{
		nTextID = IDS_NETWORK_CONNECT;
		pCmdUI->SetCheck( FALSE );
	}

	CString strText;
	LoadString( strText, nTextID );
	pCmdUI->SetText( strText );
}

void CMainWnd::OnNetworkConnect()
{
	Network.Connect( TRUE );
}

void CMainWnd::OnUpdateNetworkDisconnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( Network.IsConnected() );
}

void CMainWnd::OnNetworkDisconnect()
{
	Network.Disconnect();
}

void CMainWnd::OnUpdateNetworkConnectTo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( TRUE );
}

void CMainWnd::OnNetworkConnectTo()
{
	CConnectToDlg dlg( this, CConnectToDlg::Connect );
	if ( dlg.DoModal() != IDOK )
		return;

	Network.ConnectTo( dlg.m_sHost, dlg.m_nPort, dlg.m_nProtocol, dlg.m_bNoUltraPeer );
}

void CMainWnd::OnNetworkBrowseTo()
{
	CConnectToDlg dlg( this, CConnectToDlg::Browse );
	if ( dlg.DoModal() != IDOK )
		return;

	SOCKADDR_IN pAddress = {};
	if ( Network.Resolve( dlg.m_sHost, dlg.m_nPort, &pAddress ) )
	{
		new CBrowseHostWnd( dlg.m_nProtocol, &pAddress );
	}
}

void CMainWnd::OnNetworkChatTo()
{
	CConnectToDlg dlg( this, CConnectToDlg::Chat );
	if ( dlg.DoModal() != IDOK )
		return;

	SOCKADDR_IN pAddress = {};
	if ( Network.Resolve( dlg.m_sHost, dlg.m_nPort, &pAddress ) )
	{
		ChatWindows.OpenPrivate( Hashes::Guid(), &pAddress, FALSE, dlg.m_nProtocol );
	}
}

void CMainWnd::OnUpdateNetworkG2(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.Gnutella2.EnableToday );
}

void CMainWnd::OnNetworkG2()
{
	if ( Settings.Gnutella2.EnableToday )
	{
		CString strMessage;
		LoadString( strMessage, IDS_NETWORK_DISABLE_G2 );

		if ( AfxMessageBox( strMessage, MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES )
			return;
	}

	Settings.Gnutella2.EnableToday = !Settings.Gnutella2.EnableToday;

	if ( Settings.Gnutella2.EnableToday )
	{
		if ( ! Settings.Gnutella2.EnableAlways )
		{
			CString strMessage;
			LoadString( strMessage, IDS_NETWORK_ALWAYS );
			if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
			{
				Settings.Gnutella2.EnableAlways = true;
			}
		}
		if( !Network.IsConnected() )
			Network.Connect( TRUE );
		else
			DiscoveryServices.ExecuteBootstraps( PROTOCOL_G2 );
	}
}

void CMainWnd::OnUpdateNetworkG1(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.Gnutella1.EnableToday );
#ifdef LAN_MODE
	pCmdUI->Enable( FALSE );
#else // LAN_MODE
	pCmdUI->Enable( Settings.GetOutgoingBandwidth() >= 2 );
#endif // LAN_MODE
}

void CMainWnd::OnNetworkG1()
{
#ifndef LAN_MODE
	Settings.Gnutella1.EnableToday = !Settings.Gnutella1.EnableToday;

	if ( Settings.Gnutella1.EnableToday )
	{
		if ( ! Settings.Gnutella1.EnableAlways )
		{
			CString strMessage;
			LoadString( strMessage, IDS_NETWORK_ALWAYS );
			if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
			{
				Settings.Gnutella1.EnableAlways = true;
			}
		}

		if ( !Network.IsConnected() )
			Network.Connect( TRUE );
		else
			DiscoveryServices.ExecuteBootstraps( PROTOCOL_G1 );
	}
#endif // LAN_MODE
}

void CMainWnd::OnUpdateNetworkED2K(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.eDonkey.EnableToday );
#ifdef LAN_MODE
	pCmdUI->Enable( FALSE );
#else // LAN_MODE
	pCmdUI->Enable( Settings.GetOutgoingBandwidth() >= 2 );
#endif // LAN_MODE
}

void CMainWnd::OnNetworkED2K()
{
#ifndef LAN_MODE
	Settings.eDonkey.EnableToday = !Settings.eDonkey.EnableToday;

	if ( Settings.eDonkey.EnableToday )
	{
		if ( ! Settings.eDonkey.EnableAlways )
		{
			CString strMessage;
			LoadString( strMessage, IDS_NETWORK_ALWAYS );
			if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
			{
				Settings.eDonkey.EnableAlways = true;
			}
		}

		if ( !Network.IsConnected() )
			Network.Connect( TRUE );
	}
#endif // LAN_MODE
}

void CMainWnd::OnUpdateNetworkDC(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.DC.EnableToday );
#ifdef LAN_MODE
	pCmdUI->Enable( FALSE );
#else // LAN_MODE
	pCmdUI->Enable( Settings.GetOutgoingBandwidth() >= 2 );
#endif // LAN_MODE
}

void CMainWnd::OnNetworkDC()
{
#ifndef LAN_MODE
	Settings.DC.EnableToday = !Settings.DC.EnableToday;

	if ( Settings.DC.EnableToday )
	{
		if ( ! Settings.DC.EnableAlways )
		{
			CString strMessage;
			LoadString( strMessage, IDS_NETWORK_ALWAYS );
			if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
			{
				Settings.DC.EnableAlways = true;
			}
		}

		if ( !Network.IsConnected() )
			Network.Connect( TRUE );
	}
#endif // LAN_MODE
}

void CMainWnd::OnUpdateNetworkBT(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.BitTorrent.EnableToday );
#ifdef LAN_MODE
	pCmdUI->Enable( FALSE );
#else // LAN_MODE
	pCmdUI->Enable( Settings.GetOutgoingBandwidth() >= 2 );
#endif // LAN_MODE
}

void CMainWnd::OnNetworkBT()
{
#ifndef LAN_MODE
	Settings.BitTorrent.EnableToday = !Settings.BitTorrent.EnableToday;

	if ( Settings.BitTorrent.EnableToday )
	{
		if ( ! Settings.BitTorrent.EnableAlways )
		{
			CString strMessage;
			LoadString( strMessage, IDS_NETWORK_ALWAYS );
			if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
			{
				Settings.BitTorrent.EnableAlways = true;
			}
		}

		if ( !Network.IsConnected() )
			Network.Connect( TRUE );
	}
#endif // LAN_MODE
}

void CMainWnd::OnUpdateNetworkAutoClose(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( Settings.Live.AutoClose || Transfers.GetActiveCount() > 0 );
	pCmdUI->SetCheck( Settings.Live.AutoClose );
}

void CMainWnd::OnNetworkAutoClose()
{
	if ( Settings.Live.AutoClose )
	{
		Settings.Live.AutoClose = false;
	}
	else
	{
		Settings.Live.AutoClose = ( Transfers.GetActiveCount() > 0 );
		if ( Settings.Live.AutoClose )
		{
			if ( ! m_bTrayHide ) CloseToTray();
		}
		else
		{
			PostMessage( WM_CLOSE );
		}
	}

	CString strCaption = LoadString( IDR_MAINFRAME );
	if ( Settings.Live.AutoClose )
		strCaption += LoadString( IDS_CLOSING_AFTER );
	SetWindowText( strCaption );
}

void CMainWnd::OnNetworkExit()
{
	PostMessage( WM_CLOSE );
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd view menu

void CMainWnd::OnUpdateViewBasic(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.General.GUIMode == GUI_BASIC );
}

void CMainWnd::OnViewBasic()
{
	if ( Settings.General.GUIMode == GUI_BASIC ) return;
	CString strMessage;
	LoadString( strMessage, IDS_VIEW_MODE_CONFIRM );
	if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
	CWaitCursor pCursor;
	SetGUIMode( GUI_BASIC );
}

void CMainWnd::OnUpdateViewTabbed(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.General.GUIMode == GUI_TABBED );
}

void CMainWnd::OnViewTabbed()
{
	if ( Settings.General.GUIMode == GUI_TABBED ) return;
	CString strMessage;
	LoadString( strMessage, IDS_VIEW_MODE_CONFIRM );
	if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
	CWaitCursor pCursor;
	SetGUIMode( GUI_TABBED );
}

void CMainWnd::OnUpdateViewWindowed(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.General.GUIMode == GUI_WINDOWED );
}

void CMainWnd::OnViewWindowed()
{
	if ( Settings.General.GUIMode == GUI_WINDOWED ) return;
	CString strMessage;
	LoadString( strMessage, IDS_VIEW_MODE_CONFIRM );
	if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
	CWaitCursor pCursor;
	SetGUIMode( GUI_WINDOWED );
}

void CMainWnd::OnUpdateViewSystem(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CSystemWnd) ) != NULL );
}

void CMainWnd::OnViewSystem()
{
	m_pWindows.Open( RUNTIME_CLASS(CSystemWnd), TRUE );
	OpenFromTray();
}

void CMainWnd::OnUpdateViewNeighbours(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CNeighboursWnd) ) != NULL );
}

void CMainWnd::OnViewNeighbours()
{
	m_pWindows.Open( RUNTIME_CLASS(CNeighboursWnd), TRUE );
	OpenFromTray();
}

void CMainWnd::OnUpdateViewTraffic(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CTrafficWnd) ) != NULL );
}

void CMainWnd::OnViewTraffic()
{
	if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
	{
		new CTrafficWnd();
	}
	else
	{
		m_pWindows.Open( RUNTIME_CLASS(CTrafficWnd), TRUE );
	}
	OpenFromTray();
}

void CMainWnd::OnUpdateViewDownloads(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CDownloadsWnd) ) != NULL );
}

void CMainWnd::OnViewDownloads()
{
	m_pWindows.Open( RUNTIME_CLASS(CDownloadsWnd), TRUE );
	OpenFromTray();
}

void CMainWnd::OnUpdateViewUploads(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CUploadsWnd) ) != NULL );
}

void CMainWnd::OnViewUploads()
{
	m_pWindows.Open( RUNTIME_CLASS(CUploadsWnd), TRUE );
	OpenFromTray();
}

void CMainWnd::OnUpdateViewLibrary(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CLibraryWnd) ) != NULL );
}

void CMainWnd::OnViewLibrary()
{
	CLibraryWnd::GetLibraryWindow( TRUE );

	OpenFromTray();
}

void CMainWnd::OnUpdateViewMedia(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CMediaWnd) ) != NULL );
}

void CMainWnd::OnViewMedia()
{
	if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
	{
		new CMediaWnd();
	}
	else
	{
		CMediaWnd::GetMediaWindow( TRUE );
	}
	OpenFromTray();
}

void CMainWnd::OnUpdateViewHosts(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CHostCacheWnd) ) != NULL );
}

void CMainWnd::OnViewHosts()
{
	m_pWindows.Open( RUNTIME_CLASS(CHostCacheWnd), TRUE );
	OpenFromTray();
}

void CMainWnd::OnUpdateViewDiscovery(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CDiscoveryWnd) ) != NULL );
}

void CMainWnd::OnViewDiscovery()
{
	m_pWindows.Open( RUNTIME_CLASS(CDiscoveryWnd), TRUE );
	OpenFromTray();
}

void CMainWnd::OnUpdateViewPackets(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CPacketWnd) ) != NULL );
}

void CMainWnd::OnViewPackets()
{
	if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
	{
		new CPacketWnd();
	}
	else
	{
		m_pWindows.Open( RUNTIME_CLASS(CPacketWnd), TRUE );
	}
	OpenFromTray();
}

void CMainWnd::OnUpdateViewSearchMonitor(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CSearchMonitorWnd) ) != NULL );
}

void CMainWnd::OnViewSearchMonitor()
{
	m_pWindows.Open( RUNTIME_CLASS(CSearchMonitorWnd), TRUE );
	OpenFromTray();
}

void CMainWnd::OnUpdateViewResultsMonitor(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CHitMonitorWnd) ) != NULL );
}

void CMainWnd::OnViewResultsMonitor()
{
	m_pWindows.Open( RUNTIME_CLASS(CHitMonitorWnd), TRUE );
	OpenFromTray();
}

void CMainWnd::OnUpdateViewSecurity(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CSecurityWnd) ) != NULL );
}

void CMainWnd::OnViewSecurity()
{
	m_pWindows.Open( RUNTIME_CLASS(CSecurityWnd), TRUE );
	OpenFromTray();
}

void CMainWnd::OnUpdateViewScheduler(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CSchedulerWnd) ) != NULL );
}

void CMainWnd::OnViewScheduler()
{
	m_pWindows.Open( RUNTIME_CLASS(CSchedulerWnd), TRUE );
	OpenFromTray();
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd tab menu

void CMainWnd::OnUpdateTabConnect(CCmdUI* /*pCmdUI*/)
{
	CCoolBarItem* pItem = m_wndToolBar.GetID( ID_TAB_CONNECT );

	UINT nTextID = 0;
	UINT nTipID = 0;
	bool bNetworksEnabled = ( Settings.Gnutella1.EnableToday || Settings.Gnutella2.EnableToday || Settings.eDonkey.EnableToday );

	if ( Network.IsConnected() &&
		( Network.IsWellConnected() || !bNetworksEnabled )	// If Network.IsWellConnected() or G1, G2, eDonkey are disabled and only BitTorrent is enabled say connected
	)
	{
		if ( pItem ) pItem->SetCheck( FALSE );
		if ( pItem ) pItem->SetTextColour( RGB( 255, 0, 0 ) );
		m_wndTabBar.SetMessage( IDS_TABBAR_CONNECTED );
		nTextID	= IDS_NETWORK_DISCONNECT;
		nTipID	= ID_NETWORK_DISCONNECT;
	}
	else if ( Network.IsConnected() )
	{
		if ( pItem ) pItem->SetCheck( TRUE );
		if ( pItem ) pItem->SetTextColour( CoolInterface.m_crCmdText == 0 ?  CoolInterface.m_crTextStatus : CoolInterface.m_crCmdText );
		m_wndTabBar.SetMessage( (UINT)0 );
		nTextID	= IDS_NETWORK_CONNECTING;
		nTipID	= ID_NETWORK_DISCONNECT;
	}
	else
	{
		if ( pItem ) pItem->SetCheck( FALSE );
		if ( pItem ) pItem->SetTextColour( CoolInterface.m_crCmdText == 0 ?  CoolInterface.m_crTextStatus : CoolInterface.m_crCmdText );
		if ( m_wndToolBar.IsVisible() ) m_wndTabBar.SetMessage( IDS_TABBAR_NOT_CONNECTED );
		nTextID	= IDS_NETWORK_CONNECT;
		nTipID	= ID_NETWORK_CONNECT;
	}

	CString strText;

	LoadString( strText, nTextID );
	if ( pItem ) pItem->SetText( strText );

	LoadString( strText, nTipID );
	if ( pItem ) pItem->SetTip( strText );

	if ( pItem ) pItem->SetImage( nTipID );
}

void CMainWnd::OnTabConnect()
{
	if ( Network.IsConnected() )
	{
		CString strMessage;
		LoadString( strMessage, IDS_NETWORK_DISCONNECT_CONFIRM );

		if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ||
			 ! Network.IsWellConnected() ||
			AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
		{
			Network.Disconnect();
		}
	}
	else
	{
		Network.Connect( TRUE );

		CChildWnd* pActive = m_pWindows.GetActive();

		if ( ! pActive || ! pActive->IsKindOf( RUNTIME_CLASS(CHomeWnd) ) )
		{
			m_pWindows.Open( RUNTIME_CLASS(CNeighboursWnd) );
		}
	}
}

void CMainWnd::OnUpdateTabHome(CCmdUI* pCmdUI)
{
	if ( Settings.General.GUIMode != GUI_WINDOWED )
	{
		CChildWnd* pChild = m_pWindows.GetActive();
		pCmdUI->SetCheck( pChild && pChild->IsKindOf( RUNTIME_CLASS(CHomeWnd) ) );
	}
	else
	{
		pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CHomeWnd) ) != NULL );
	}
}

void CMainWnd::OnTabHome()
{
	if ( Settings.General.GUIMode != GUI_WINDOWED )
	{
		m_pWindows.Open( RUNTIME_CLASS(CHomeWnd) );
		OpenFromTray();
	}
	else
	{
		m_pWindows.Open( RUNTIME_CLASS(CHomeWnd), TRUE );
		OpenFromTray();
	}
}

void CMainWnd::OnUpdateTabLibrary(CCmdUI* pCmdUI)
{
	CChildWnd* pChild = m_pWindows.GetActive();
	pCmdUI->SetCheck( pChild && pChild->IsKindOf( RUNTIME_CLASS(CLibraryWnd) ) );
}

void CMainWnd::OnTabLibrary()
{
	CChildWnd* pChild = m_pWindows.GetActive();

	if ( CLibraryWnd* pLibraryWnd = CLibraryWnd::GetLibraryWindow() )
	{
		if ( pChild == pLibraryWnd )
		{
			pLibraryWnd->m_wndFrame.Switch();
		}
	}

	OpenFromTray();
}

void CMainWnd::OnUpdateTabMedia(CCmdUI* pCmdUI)
{
	CMediaWnd* pChild = (CMediaWnd*)m_pWindows.GetActive();
	pCmdUI->SetCheck( pChild && pChild->IsKindOf( RUNTIME_CLASS(CMediaWnd) ) );

	if ( CCoolBarItem* pItem = m_wndToolBar.GetID( ID_TAB_MEDIA ) )
	{
		if ( ( pChild = (CMediaWnd*)m_pWindows.Find( RUNTIME_CLASS(CMediaWnd) ) ) != NULL )
		{
			pItem->SetTextColour( pChild->IsPlaying() ? CoolInterface.m_crTextStatus : CoolInterface.m_crCmdText );
		}
		else
		{
			pItem->SetTextColour( CoolInterface.m_crCmdText );
		}
	}
}

void CMainWnd::OnTabMedia()
{
	CMediaWnd::GetMediaWindow();

	theApp.m_bMenuWasVisible = FALSE;

	OpenFromTray();
}

void CMainWnd::OnUpdateTabSearch(CCmdUI* pCmdUI)
{
	CChildWnd* pChild = m_pWindows.GetActive();
	pCmdUI->SetCheck( pChild && pChild->IsKindOf( RUNTIME_CLASS(CSearchWnd) ) );
}

void CMainWnd::OnTabSearch()
{
	m_pWindows.OpenNewSearchWindow();
	m_wndTabBar.OnSkinChange();
	OpenFromTray();
}

void CMainWnd::OnUpdateTabTransfers(CCmdUI* pCmdUI)
{
	CChildWnd* pChild = m_pWindows.GetActive();

	if ( pChild != NULL && pChild->m_pGroupParent != NULL )
	{
		pChild = pChild->m_pGroupParent;
		if ( ! m_pWindows.Check( pChild ) ) pChild = NULL;
	}

	pCmdUI->SetCheck( pChild != NULL && pChild->IsKindOf( RUNTIME_CLASS(CDownloadsWnd) ) );
}

void CMainWnd::OnTabTransfers()
{
	m_pWindows.Open( RUNTIME_CLASS(CDownloadsWnd) );
	OpenFromTray();
}

void CMainWnd::OnUpdateTabIRC(CCmdUI* pCmdUI)
{
	CChildWnd* pChild = m_pWindows.GetActive();
	pCmdUI->SetCheck( pChild && pChild->IsKindOf( RUNTIME_CLASS(CIRCWnd) ) );
}

void CMainWnd::OnTabIRC()
{
	m_pWindows.Open( RUNTIME_CLASS(CIRCWnd) );
	OpenFromTray();
}
void CMainWnd::OnUpdateTabNetwork(CCmdUI* pCmdUI)
{
	CChildWnd* pChild = m_pWindows.GetActive();

	if ( pChild != NULL && pChild->m_pGroupParent != NULL )
	{
		pChild = pChild->m_pGroupParent;
		if ( ! m_pWindows.Check( pChild ) ) pChild = NULL;
	}

	pCmdUI->SetCheck( pChild != NULL && pChild->IsKindOf( RUNTIME_CLASS(CNeighboursWnd) ) );
}

void CMainWnd::OnTabNetwork()
{
	m_pWindows.Open( RUNTIME_CLASS(CNeighboursWnd) );
	OpenFromTray();
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd tools menu

void CMainWnd::OnUpdateToolsDownload(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( IsWindowEnabled() );
}

void CMainWnd::OnToolsDownload()
{
	for ( BOOL bBreak = FALSE; ! bBreak; )
	{
		bBreak = TRUE;

		CDownloadDlg dlg;
		if ( dlg.DoModal() != IDOK )
			return;

		for ( POSITION pos = dlg.m_pURLs.GetHeadPosition(); pos; )
		{
			CShareazaURL pURL( dlg.m_pURLs.GetNext( pos ) );

			CExistingFileDlg::Action action = CExistingFileDlg::CheckExisting( &pURL );
			if ( action == CExistingFileDlg::Cancel )
			{
				// Reopen download dialog
				bBreak = FALSE;
				break;
			}
			else if ( action != CExistingFileDlg::Download )
				// Skip this file
				continue;

			if ( pURL.m_nAction == CShareazaURL::uriDownload ||
				 pURL.m_nAction == CShareazaURL::uriSource )
			{
				if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 && ! Network.IsWellConnected() )
					Network.Connect( TRUE );

				m_pWindows.Open( RUNTIME_CLASS(CDownloadsWnd) );

				CSingleLock pLock( &Transfers.m_pSection, TRUE );

				if ( CDownload* pDownload = Downloads.Add( pURL ) )
				{
					if ( Settings.Downloads.ShowMonitorURLs )
						pDownload->ShowMonitor();
				}
			}
			else
			{
				PostMessage( WM_URL, (WPARAM)new CShareazaURL( pURL ) );
			}
		}
	}
}

void CMainWnd::OnUpdateToolsImportDownloads(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( IsWindowEnabled() );
}

void CMainWnd::OnToolsImportDownloads()
{
	CString strPath( BrowseForFolder( IDS_SELECT_ED2K_TEMP_FOLDER ) );
	if ( strPath.IsEmpty() )
		return;

	CDonkeyImportDlg dlg;
	dlg.m_pImporter.AddFolder( strPath );
	dlg.DoModal();
}

void CMainWnd::OnUpdateOpenDownloadsFolder(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( IsWindowEnabled() );
}

void CMainWnd::OnOpenDownloadsFolder()
{
	CMainWnd* pMainWnd = theApp.SafeMainWnd();

	if ( pMainWnd )
		ShellExecute( pMainWnd->GetSafeHwnd(), _T("open"), Settings.Downloads.CompletePath, NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnToolsSkin()
{
	if ( ! IsWindowEnabled() ) return;
	CSettingsManagerDlg::Run( _T("CSkinsSettingsPage") );
}

void CMainWnd::OnToolsLanguage()
{
	if ( ! IsWindowEnabled() )
		return;

	theApp.WriteProfileInt( _T("Windows"), _T("RunLanguage"), TRUE );

	CLanguageDlg dlg;
	if ( dlg.DoModal() != IDOK )
		 return;

	bool bRestart = AfxMessageBox( IDS_GENERAL_RTL_WARNING,
		MB_ICONQUESTION | MB_YESNO ) == IDYES;

	CWaitCursor pCursor;

	Settings.General.Language = dlg.m_sLanguage;
	Settings.General.LanguageRTL = dlg.m_bLanguageRTL;
	Settings.Save();

	if ( bRestart )
	{
		HINSTANCE hResult = ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), NULL,
			theApp.m_strBinaryPath, _T("-nowarn -wait"), Settings.General.Path, SW_SHOWNORMAL );
		if ( hResult > (HINSTANCE)32 )
		{
			PostMessage( WM_CLOSE );
			return;
		}
	}

	PostMessage( WM_SKINCHANGED );
}

void CMainWnd::OnToolsSeedTorrent()
{
	CFileDialog dlgFile( TRUE, _T("torrent"), ( Settings.Downloads.TorrentPath + _T("\\.") ) , OFN_HIDEREADONLY,
		_T("Torrent Files|*.torrent|") +
		SchemaCache.GetFilter( CSchema::uriAllFiles ) +
		_T("|"), this );

	if ( dlgFile.DoModal() != IDOK ) return;

	CTorrentSeedDlg dlgSeed( dlgFile.GetPathName(), TRUE );
	dlgSeed.DoModal();
}

void CMainWnd::OnToolsReseedTorrent()
{
	CTorrentSeedDlg dlgSeed( LibraryHistory.LastSeededTorrent.m_sPath, TRUE );
	dlgSeed.DoModal();
}

void CMainWnd::OnDiskSpace()
{
	CHelpDlg::Show( _T("GeneralHelp.DiskSpace") );
}

void CMainWnd::OnDiskWriteFail()
{
	CHelpDlg::Show( _T("GeneralHelp.DiskWriteFail") );
}

void CMainWnd::OnConnectionFail()
{
	CHelpDlg::Show( _T("GeneralHelp.ConnectionFail") );
}

void CMainWnd::OnNoDonkeyServers()
{
	CHelpDlg::Show( _T("GeneralHelp.DonkeyServerList") );
}

void CMainWnd::OnToolsProfile()
{
	CProfileManagerDlg dlg;
	dlg.DoModal();
}

void CMainWnd::OnLibraryFolders()
{
	CShareManagerDlg dlg;
	dlg.DoModal();
}

void CMainWnd::OnToolsWizard()
{
	if ( ! IsWindowEnabled() ) return;

	Settings.Windows.RunWizard = true;

	CWizardSheet::RunWizard( this );
}

void CMainWnd::OnToolsSettings()
{
	if ( ! IsWindowEnabled() ) return;
	CSettingsManagerDlg::Run();
}

void CMainWnd::OnToolsReskin()
{
	OnSkinChanged( 0, 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd library hook in

void CMainWnd::OnUpdateLibraryHashPriority(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( LibraryBuilder.GetBoostPriority() );
}

void CMainWnd::OnLibraryHashPriority()
{
	LibraryBuilder.BoostPriority( ! LibraryBuilder.GetBoostPriority() );
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd window menu

void CMainWnd::OnUpdateWindowCascade(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( TRUE );
}

void CMainWnd::OnWindowCascade()
{
	m_pWindows.Cascade( GetAsyncKeyState( VK_SHIFT ) & 0x8000 );
}

void CMainWnd::OnUpdateWindowTileHorz(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( Settings.General.GUIMode == GUI_WINDOWED );
}

void CMainWnd::OnUpdateWindowTileVert(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( Settings.General.GUIMode == GUI_WINDOWED );
}

void CMainWnd::OnUpdateWindowNavBar(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( m_wndNavBar.IsVisible() );
}

void CMainWnd::OnWindowNavBar()
{
	ShowControlBar( &m_wndToolBar, FALSE, FALSE );
	ShowControlBar( &m_wndNavBar, ! m_wndNavBar.IsVisible(), FALSE );
	if ( m_wndToolBar.IsVisible() )
	{
		if ( ! Network.IsConnected() ) m_wndTabBar.SetMessage( IDS_TABBAR_NOT_CONNECTED );
	}
	else
		m_wndTabBar.SetMessage( _T("") );
}

void CMainWnd::OnUpdateWindowToolBar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_wndToolBar.IsVisible() );
}

void CMainWnd::OnWindowToolBar()
{
	ShowControlBar( &m_wndNavBar, FALSE, FALSE );
	ShowControlBar( &m_wndToolBar, ! m_wndToolBar.IsVisible(), FALSE );
	if ( m_wndToolBar.IsVisible() )
	{
		if ( ! Network.IsConnected() ) m_wndTabBar.SetMessage( IDS_TABBAR_NOT_CONNECTED );
	}
	else
		m_wndTabBar.SetMessage( _T("") );
}

void CMainWnd::OnUpdateWindowTabBar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_wndTabBar.IsVisible() );
}

void CMainWnd::OnWindowTabBar()
{
	ShowControlBar( &m_wndTabBar, ! m_wndTabBar.IsVisible(), TRUE );
}

void CMainWnd::OnUpdateWindowMonitor(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_wndMonitorBar.IsVisible() );
}

void CMainWnd::OnWindowMonitor()
{
	BOOL bVisible = ! m_wndMonitorBar.IsVisible();
	Settings.Toolbars.ShowMonitor = ( bVisible != FALSE );
	ShowControlBar( &m_wndMonitorBar, bVisible, TRUE );
}

void CMainWnd::OnUpdateWindowRemote(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( m_wndRemoteWnd.IsVisible() );
}

void CMainWnd::OnWindowRemote()
{
	if ( m_wndRemoteWnd.IsVisible() )
		m_wndRemoteWnd.DestroyWindow();
	else
		m_wndRemoteWnd.Create( &m_wndMonitorBar );
}

void CMainWnd::OnRemoteClose()
{
	if ( m_wndRemoteWnd.IsVisible() ) m_wndRemoteWnd.DestroyWindow();
}

void CMainWnd::OnUpdateMediaCommand(CCmdUI *pCmdUI)
{
	if ( CMediaFrame::GetMediaFrame() != NULL )
		pCmdUI->ContinueRouting();
	else
		pCmdUI->Enable( TRUE );
}

void CMainWnd::OnMediaCommand()
{
	if ( ! CMediaFrame::GetMediaFrame() )
	{
		OnTabMedia();
	}

	if ( CMediaFrame* pMediaFrame = CMediaFrame::GetMediaFrame() )
	{
		pMediaFrame->SendMessage( WM_COMMAND, GetCurrentMessage()->wParam );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd help menu

void CMainWnd::OnHelpAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

void CMainWnd::OnHelpVersionCheck()
{
	VersionChecker.ForceCheck();
}

void CMainWnd::OnHelpHomepage()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("?Version=") + theApp.m_sVersion,
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWeb1()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/external/?link1"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWeb2()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/external/?link2"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWeb3()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/external/?link3"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWeb4()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/external/?link4"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWeb5()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/external/?link5"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWeb6()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/external/?link6"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpFaq()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/?faq"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpConnectiontest()
{
	CString strWebSite;
	strWebSite.Format( _T("%shelp/test/?port=%u&lang=%s&Version=%s"),
		WEB_SITE_T, Settings.Connection.InPort,
		(LPCTSTR)Settings.General.Language, (LPCTSTR)theApp.m_sVersion );
	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite,
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpGuide()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/?guide"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpForums()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/?forum"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpUpdate()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/update/?Version=") + theApp.m_sVersion,
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpRouter()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/?router"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpSecurity()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/?security"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpCodec()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/?codec"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpTorrent()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"), strWebSite + _T("help/?torrentencoding"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWarnings()
{
	if ( IsWindowEnabled() && IsForegroundWindow() )
	{
		Settings.Windows.RunWarnings = true;
		CWarningsDlg dlg;
		dlg.DoModal();
	}
}

void CMainWnd::OnHelpPromote()
{
	if ( IsWindowEnabled() && IsForegroundWindow() )
	{
		Settings.Windows.RunPromote = true;
		CPromoteDlg dlg;
		dlg.DoModal();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd skin forwarding

void CMainWnd::OnSize(UINT nType, int cx, int cy)
{
	if ( m_pSkin )
		m_pSkin->OnSize( this );

	CMDIFrameWnd::OnSize( nType, cx, cy );
}

void CMainWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	if ( m_pSkin )
		m_pSkin->OnNcCalcSize( this, bCalcValidRects, lpncsp );
	else
		CMDIFrameWnd::OnNcCalcSize( bCalcValidRects, lpncsp );
}

LRESULT CMainWnd::OnNcHitTest(CPoint point)
{
	if ( m_pSkin )
		return m_pSkin->OnNcHitTest( this, point, TRUE );
	else
		return CMDIFrameWnd::OnNcHitTest( point );
}

void CMainWnd::OnNcPaint()
{
	if ( m_pSkin )
		m_pSkin->OnNcPaint( this );
	else
		CMDIFrameWnd::OnNcPaint();
}

BOOL CMainWnd::OnNcActivate(BOOL bActive)
{
	if ( m_pSkin )
	{
		m_pSkin->OnNcActivate( this, IsWindowEnabled() && ( bActive || ( m_nFlags & WF_STAYACTIVE ) ) );
		return TRUE;
	}
	else
		return CMDIFrameWnd::OnNcActivate( bActive );
}

void CMainWnd::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	if ( m_pSkin )
		m_pSkin->OnNcMouseMove( this, nHitTest, point );

	CMDIFrameWnd::OnNcMouseMove( nHitTest, point );
}

void CMainWnd::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDown( this, nHitTest, point ) )
		return;

	CMDIFrameWnd::OnNcLButtonDown( nHitTest, point );

	// Windows Vista skinning workaround (system caption buttons over skin drawing)
	if ( ! theApp.m_bClosing && m_pSkin )	// Window can be destroyed at this point
		m_pSkin->OnNcPaint( this );
}

void CMainWnd::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonUp( this, nHitTest, point ) )
		return;

	CMDIFrameWnd::OnNcLButtonUp( nHitTest, point );
}

void CMainWnd::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDblClk( this, nHitTest, point ) )
		return;

	CMDIFrameWnd::OnNcLButtonDblClk( nHitTest, point );
}

LRESULT CMainWnd::OnSetText(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if ( m_pSkin )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible )
			ModifyStyle( WS_VISIBLE, 0 );
		LRESULT lResult = Default();
		if ( bVisible )
			ModifyStyle( 0, WS_VISIBLE );
		if ( m_pSkin )
			m_pSkin->OnSetText( this );
		return lResult;
	}
	else
	{
		return Default();
	}
}

LRESULT CMainWnd::OnSanityCheck(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// Remove extra messages
	MSG msg;
	while ( ::PeekMessage( &msg, NULL, WM_SANITY_CHECK, WM_SANITY_CHECK, PM_REMOVE ) );

	HostCache.SanityCheck();

	// TODO: Downloads.SanityCheck();

	// TODO: Uploads.SanityCheck();

	CChildWnd* pChildWnd = NULL;
	while ( ( pChildWnd = m_pWindows.Find( NULL, pChildWnd ) ) != NULL )
	{
		pChildWnd->SanityCheck();
	}

	return 0L;
}

LRESULT CMainWnd::OnNowUploading(WPARAM /*wParam*/, LPARAM lParam)
{
	CAutoPtr< CString > pFilename( (CString*)lParam );
	if ( pFilename && ! pFilename->IsEmpty() )
	{
		CQuickLock oLock( Library.m_pSection );

		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( *pFilename ) )
		{
			pFile->m_nUploadsToday++;
			pFile->m_nUploadsTotal++;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd IDropTarget implementation

IMPLEMENT_DROP(CMainWnd,CMDIFrameWnd)

BOOL CMainWnd::OnDrop(IDataObject* pDataObj, DWORD /* grfKeyState */, POINT /* ptScreen */, DWORD* pdwEffect, BOOL bDrop)
{
	if ( ! pDataObj || ! pdwEffect )
		return FALSE;

	FORMATETC fmtcFiles = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	FORMATETC fmtcURL = { (CLIPFORMAT) RegisterClipboardFormat( CFSTR_SHELLURL ), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	if ( SUCCEEDED ( pDataObj->QueryGetData( &fmtcFiles ) ) )
	{
		CList < CString > oFiles;
		if ( CShareazaDataSource::ObjectToFiles( pDataObj, oFiles ) == S_OK )
		{
			BOOL bAccepted = FALSE;
			POSITION pos = oFiles.GetHeadPosition();
			while ( pos && ! ( bAccepted && ! bDrop ) )
			{
				bAccepted = theApp.Open( oFiles.GetNext( pos ), bDrop ) || bAccepted;
			}
			if ( bAccepted )
				*pdwEffect = DROPEFFECT_COPY;
			return bAccepted;
		}
	}

	if ( SUCCEEDED ( pDataObj->QueryGetData( &fmtcURL ) ) )
	{
		CString strURL;
		if ( CShareazaDataSource::ObjectToURL( pDataObj, strURL ) == S_OK )
		{
			BOOL bAccepted = theApp.OpenURL( strURL, bDrop );
			if ( bAccepted )
				*pdwEffect = DROPEFFECT_LINK;
			return bAccepted;
		}
	}

	return FALSE;
}

int CMainWnd::SnarlCommand(LPCSTR szCommand)
{
	DWORD_PTR lResult = (DWORD_PTR)-1;
	if ( HWND hWndSnarl = ::FindWindow( _T("w>Snarl"), _T("Snarl") ) )
	{
		const ULONG_PTR uSnarlMagic = 0x534e4c03;
		COPYDATASTRUCT cds = { uSnarlMagic, (DWORD)strlen( szCommand ) + 1, (LPSTR)szCommand };
		SendMessageTimeout( hWndSnarl, WM_COPYDATA, (WPARAM)GetCurrentProcessId(), (LPARAM)&cds, SMTO_ABORTIFHUNG, 1000, &lResult );
	}
	return (int)lResult;
}

bool CMainWnd::SnarlRegister()
{
	return Settings.Interface.Snarl && ( SnarlCommand( "register?app-sig=app/" CLIENT_NAME "&title=" CLIENT_NAME ) >= 0 );
}

void CMainWnd::SnarlUnregister()
{
	SnarlCommand( "unregister?app-sig=app/" CLIENT_NAME );
}

bool CMainWnd::SnarlNotify(const CString& sText, const CString& sTitle, DWORD dwIcon, UINT uTimeout)
{
	if ( ! SnarlRegister() )
		return false;

	CString sSafeTitle = sTitle;
	sSafeTitle.Replace( _T("="), _T("==") );
	sSafeTitle.Replace( _T("&"), _T("&&") );

	CString sSafeText = sText;
	sSafeText.Replace( _T("="), _T("==") );
	sSafeText.Replace( _T("&"), _T("&&") );

	CStringA sCommand;
	sCommand.Format( "notify?app-sig=app/%s&timeout=%u&title=%s&text=%s",
		CLIENT_NAME, uTimeout, (LPCSTR)UTF8Encode( sSafeTitle ), (LPCSTR)UTF8Encode( sSafeText ) );

	switch ( dwIcon & NIIF_ICON_MASK )
	{
	case NIIF_INFO:
		sCommand += "&icon=!system-info";
		break;
	case NIIF_WARNING:
		sCommand += "&icon=!system-warning";
		break;
	case NIIF_ERROR:
		sCommand += "&icon=!system-critical";
		break;
	case NIIF_USER:
		sCommand += "&icon=!message-new_message";
		break;
	}
	return ( SnarlCommand( sCommand ) >= 0 );
}

void CMainWnd::ShowTrayPopup(const CString& sText, const CString& sTitle, DWORD dwIcon, UINT uTimeout)
{
	if ( sText.IsEmpty() )
		return;

	// Snarl notification
	if ( SnarlNotify( sText, sTitle, dwIcon, uTimeout ) )
		return;

	// Show temporary notify icon
	m_bTrayNotify = TRUE;

	AddTray();

	if ( ! m_bTrayIcon )
		// Tray error
		return;

	m_pTray.uFlags = NIF_INFO;

	_tcsncpy( m_pTray.szInfo, sText, _countof( m_pTray.szInfo ) - 1 );
	m_pTray.szInfo[ _countof( m_pTray.szInfo ) - 1 ] = _T('\0');
	if ( sText.GetLength() >= _countof( m_pTray.szInfo ) - 1 )
	{
		if ( sText[ _countof( m_pTray.szInfo ) - 1 ] != _T(' ') )
		{
			if ( LPTSTR pWordEnd = _tcsrchr( m_pTray.szInfo, _T(' ') ) )
			{
				pWordEnd[ 0 ] = _T('\x2026');
				pWordEnd[ 1 ] = _T('\0');
			}
		}
	}

	_tcsncpy( m_pTray.szInfoTitle, sTitle, _countof( m_pTray.szInfoTitle ) - 1 );
	m_pTray.szInfoTitle[ _countof( m_pTray.szInfoTitle ) - 1 ] = _T('\0');

	if ( ( dwIcon & NIIF_ICON_MASK ) == NIIF_USER )
		m_pTray.hBalloonIcon = CoolInterface.ExtractIcon( IDI_USER, FALSE, theApp.m_bIsVistaOrNewer ? LVSIL_NORMAL : LVSIL_SMALL );
	else
		m_pTray.hBalloonIcon = NULL;

	m_pTray.dwInfoFlags = dwIcon | ( theApp.m_bIsVistaOrNewer ? NIIF_LARGE_ICON : 0 );
	m_pTray.uTimeout = uTimeout * 1000;   // convert time to ms

	m_bTrayIcon = Shell_NotifyIcon( NIM_MODIFY, &m_pTray );

	m_pTray.szInfo[ 0 ] = _T('\0');
	m_pTray.szInfoTitle[ 0 ] = _T('\0');
}

#if _MSC_VER >= 1800 // Microsoft Visual Studio 2013
UINT CMainWnd::OnPowerBroadcast(UINT nPowerEvent, LPARAM nEventData)
#else
UINT CMainWnd::OnPowerBroadcast(UINT nPowerEvent, UINT nEventData)
#endif
{
	static bool bWasConnected = false;

	switch ( nPowerEvent )
	{
	case PBT_APMSUSPEND:
		if ( Network.IsConnected() )
		{
			bWasConnected = true;

			Network.Disconnect();
		}
		break;

	case PBT_APMRESUMEAUTOMATIC:
		if ( bWasConnected || Network.IsConnected() )
		{
			bWasConnected = false;

			Network.Disconnect();

			PostMessage( WM_COMMAND, ID_NETWORK_CONNECT );
		}
		break;
	}

	return CMDIFrameWnd::OnPowerBroadcast( nPowerEvent, nEventData );
}

BOOL CMainWnd::OnCopyData(CWnd* /*pWnd*/, COPYDATASTRUCT* pCopyDataStruct)
{
	if ( pCopyDataStruct )
	{
		switch ( pCopyDataStruct->dwData )
		{
		case COPYDATA_SCHEDULER:
			CScheduler::Execute( CString( (LPCTSTR)pCopyDataStruct->lpData,
				(int)( pCopyDataStruct->cbData / sizeof( TCHAR ) ) ) );
			break;

		case COPYDATA_OPEN:
			theApp.Open( CString( (LPCTSTR)pCopyDataStruct->lpData,
				(int)( pCopyDataStruct->cbData / sizeof( TCHAR ) ) ), TRUE );
		}
	}
	return TRUE;
}

void CMainWnd::OnUpdatePathCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable();
}

void CMainWnd::OnUpdatePathExplore(CCmdUI* pCmdUI)
{
	pCmdUI->Enable();
}
