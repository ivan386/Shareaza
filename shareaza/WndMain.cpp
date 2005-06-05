//
// WndMain.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "CoolMenu.h"
#include "Network.h"
#include "Handshake.h"
#include "HostCache.h"
#include "Neighbours.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "Plugins.h"
#include "QuerySearch.h"
#include "VersionChecker.h"
#include "GraphItem.h"
#include "ShareazaURL.h"
#include "ChatCore.h"
#include "ChatSession.h"
#include "Statistics.h"
#include "Plugins.h"
#include "BTInfo.h"
#include "Skin.h"
#include "SkinWindow.h"
#include "Scheduler.h"
#include "DlgHelp.h"
#include "LibraryHistory.h"

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
#include "WndSearchPad.h"
#include "WndSearchMonitor.h"
#include "WndHitMonitor.h"
#include "WndSecurity.h"
#include "WndSearch.h"
#include "WndBrowseHost.h"
#include "WndHome.h"
#include "WndHelp.h"
#include "WizardSheet.h"

#include "DlgSettingsManager.h"
#include "DlgShareManager.h"
#include "DlgAbout.h"
#include "DlgConnectTo.h"
#include "DlgNewSearch.h"
#include "DlgDownload.h"
#include "DlgURLAction.h"
#include "DlgUpgrade.h"
#include "DlgDownloadMonitor.h"
#include "DlgFilePreview.h"
#include "DlgLanguage.h"
#include "DlgProfileManager.h"
#include "DlgWarnings.h"
#include "DlgPromote.h"
#include "DlgCloseMode.h"
#include "DlgTorrentSeed.h"

IMPLEMENT_DYNCREATE(CMainWnd, CMDIFrameWnd)

BEGIN_INTERFACE_MAP(CMainWnd, CMDIFrameWnd)
    INTERFACE_PART(CMainWnd, IID_IDropTarget, DropTarget)
END_INTERFACE_MAP()

BEGIN_MESSAGE_MAP(CMainWnd, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_INITMENUPOPUP()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOMMAND()
	ON_WM_ACTIVATEAPP()
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
	ON_MESSAGE(WM_VERSIONCHECK, OnVersionCheck)
	ON_MESSAGE(WM_OPENCHAT, OnOpenChat)
	ON_MESSAGE(WM_OPENSEARCH, OnOpenSearch)
	ON_MESSAGE(WM_TRAY, OnTray)
	ON_MESSAGE(WM_LOG, OnLog)
	ON_MESSAGE(WM_SKINCHANGED, OnSkinChanged)
	ON_MESSAGE(WM_AFX_SETMESSAGESTRING, OnSetMessageString)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(0x0319, OnMediaKey)
	ON_MESSAGE(WM_DEVMODECHANGE, OnDevModeChange)
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
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
	ON_UPDATE_COMMAND_UI(ID_VIEW_TRAFFIC, OnUpdateViewTraffic)
	ON_COMMAND(ID_VIEW_TRAFFIC, OnViewTraffic)
	ON_COMMAND(ID_WINDOW_CASCADE, OnWindowCascade)
	ON_COMMAND(ID_TOOLS_WIZARD, OnToolsWizard)
	ON_COMMAND(ID_TRAY_OPEN, OnTrayOpen)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_AUTO_CLOSE, OnUpdateNetworkAutoClose)
	ON_COMMAND(ID_NETWORK_AUTO_CLOSE, OnNetworkAutoClose)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_DOWNLOAD, OnUpdateToolsDownload)
	ON_COMMAND(ID_TOOLS_DOWNLOAD, OnToolsDownload)
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
	ON_COMMAND(ID_TOOLS_SKIN, OnToolsSkin)
	ON_COMMAND(ID_TOOLS_LANGUAGE, OnToolsLanguage)
	ON_COMMAND(ID_TOOLS_MERCORA, OnToolsMercora)
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
	ON_UPDATE_COMMAND_UI(ID_VIEW_HELP, OnUpdateViewHelp)
	ON_COMMAND(ID_VIEW_HELP, OnViewHelp)
	ON_UPDATE_COMMAND_UI(ID_TAB_HELP, OnUpdateTabHelp)
	ON_COMMAND(ID_TAB_HELP, OnTabHelp)
	ON_COMMAND(ID_LIBRARY_FOLDERS, OnLibraryFolders)
	ON_COMMAND(ID_HELP_WARNINGS, OnHelpWarnings)
	ON_COMMAND(ID_HELP_PROMOTE, OnHelpPromote)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_G2, OnUpdateNetworkG2)
	ON_COMMAND(ID_NETWORK_G2, OnNetworkG2)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_G1, OnUpdateNetworkG1)
	ON_COMMAND(ID_NETWORK_G1, OnNetworkG1)
	ON_UPDATE_COMMAND_UI(ID_NETWORK_ED2K, OnUpdateNetworkED2K)
	ON_COMMAND(ID_NETWORK_ED2K, OnNetworkED2K)
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
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMainWnd construction

CMainWnd::CMainWnd()
{
	theApp.m_pMainWnd = this;

	m_hInstance		= AfxGetResourceHandle();
	m_pSkin			= NULL;
	m_pURLDialog	= NULL;
	m_tURLTime		= 0;
	
	LoadFrame( IDR_MAINFRAME, WS_OVERLAPPEDWINDOW );
	
	theApp.m_pSafeWnd = this;
}

CMainWnd::~CMainWnd()
{
	theApp.m_pSafeWnd = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd create window

BOOL CMainWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
	WNDCLASS wndcls;
	
	ZeroMemory( &wndcls, sizeof(WNDCLASS) );
	wndcls.style			= CS_DBLCLKS;
	wndcls.lpfnWndProc		= AfxWndProc;
	wndcls.hInstance		= AfxGetInstanceHandle();
	wndcls.hIcon			= theApp.LoadIcon( IDR_MAINFRAME );
	wndcls.hCursor			= theApp.LoadStandardCursor( IDC_ARROW );
	wndcls.hbrBackground	= NULL;
	wndcls.lpszMenuName		= NULL;
	wndcls.lpszClassName	= _T("ShareazaMainWnd");
	
	AfxRegisterClass( &wndcls );
	
	cs.lpszClass = wndcls.lpszClassName;
	
	return CMDIFrameWnd::PreCreateWindow( cs );
}

int CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CMDIFrameWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	// Icon
	
	SetIcon( AfxGetApp()->LoadIcon( IDI_SMALL ), FALSE );
	
	// Status Bar
	
	UINT wID[2] = { ID_SEPARATOR, ID_SEPARATOR };
	if ( ! m_wndStatusBar.Create( this ) ) return -1;
	m_wndStatusBar.SetIndicators( wID, 2 );
	m_wndStatusBar.SetPaneInfo( 0, ID_SEPARATOR, SBPS_STRETCH, 0 );
	m_wndStatusBar.SetPaneInfo( 1, ID_SEPARATOR, SBPS_NORMAL, 210 );
	
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
	
	if ( theApp.GetProfileInt( _T("Toolbars"), _T("CRemoteWnd"), TRUE ) )
		m_wndRemoteWnd.Create( &m_wndMonitorBar );
	
	m_pWindows.SetOwner( this );
	SetGUIMode( Settings.General.GUIMode, FALSE );
	
	// Boot
	
	if ( theApp.GetProfileInt( _T("Windows"), _T("RunLanguage"), FALSE ) == FALSE )
	{
		ShowControlBar( &m_wndMonitorBar, FALSE, TRUE );
		PostMessage( WM_COMMAND, ID_TOOLS_LANGUAGE );
	}
	else if ( theApp.GetProfileInt( _T("Windows"), _T("RunWarnings"), FALSE ) == FALSE )
	{
		PostMessage( WM_COMMAND, ID_HELP_WARNINGS );
	}
	else if ( theApp.GetProfileInt( _T("Windows"), _T("RunPromote"), FALSE ) == FALSE )
	{
		PostMessage( WM_COMMAND, ID_HELP_PROMOTE );
	}
	
	if ( Settings.Connection.AutoConnect )
		PostMessage( WM_COMMAND, ID_NETWORK_CONNECT );
	
	Settings.Live.LoadWindowState = TRUE;
	
	RegisterDragDrop( GetSafeHwnd(), &m_xDropTarget );
	
	// Go
	
	m_bTrayHide	= FALSE;
	m_bTrayIcon	= FALSE;
	m_bTimer	= FALSE;
	
	SetTimer( 1, 1000, NULL );
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd destroy window

void CMainWnd::OnClose() 
{
	CWaitCursor pCursor;
	
	theApp.m_pSafeWnd		= NULL;
	m_pWindows.m_bClosing	= TRUE;
	
	if ( m_bTrayIcon )
	{
		Shell_NotifyIcon( NIM_DELETE, &m_pTray );
		m_bTrayIcon = FALSE;
	}
	
	if ( ! m_bTrayHide )
	{
		if ( ! IsIconic() ) SaveBarState( _T("Toolbars\\CoolBar") );
		theApp.WriteProfileInt( _T("Toolbars"), _T("CRemoteWnd"), m_wndRemoteWnd.IsVisible() );
		Settings.SaveWindow( _T("CMainWnd"), this );
		m_pWindows.SaveWindowStates();
	}
	
	m_pWindows.SaveSearchWindows();
	m_pWindows.Close();	
	
	CDownloadMonitorDlg::CloseAll();
	CFilePreviewDlg::CloseAll();
	
	Network.Disconnect();
	Transfers.StopThread();
	Library.StopThread();
	ChatCore.StopThread();
	Plugins.Clear();
	
	CMDIFrameWnd::OnClose();
}

void CMainWnd::OnDestroy() 
{
	KillTimer( 1 );
	if ( m_wndRemoteWnd.IsVisible() ) m_wndRemoteWnd.DestroyWindow();
	
	RevokeDragDrop( GetSafeHwnd() );
	Network.Disconnect();
	
	CMDIFrameWnd::OnDestroy();
}

void CMainWnd::OnEndSession(BOOL bEnding) 
{
	CMDIFrameWnd::OnEndSession( bEnding );
	if ( bEnding ) SendMessage( WM_CLOSE );
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
		if ( m_wndMonitorBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}
	
	if ( CMediaFrame::g_pMediaFrame != NULL )
	{
		if ( CMediaFrame::g_pMediaFrame->OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
			return TRUE;
	}
	
	return CMDIFrameWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
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

void CMainWnd::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CMDIFrameWnd::OnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );
	UINT nID = pPopupMenu->GetMenuItemID( 0 );
	if ( nID != SC_RESTORE ) CoolMenu.AddMenu( pPopupMenu, TRUE );
}

void CMainWnd::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	CoolMenu.OnMeasureItem( lpMeasureItemStruct );
}

void CMainWnd::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CoolMenu.OnDrawItem( lpDrawItemStruct );
}

void CMainWnd::OnSysColorChange() 
{
	CMDIFrameWnd::OnSysColorChange();
	CoolInterface.OnSysColourChange();
}

void CMainWnd::OnUpdateFrameTitle(BOOL bAddToTitle)
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
	
	if ( theApp.m_pfnGetMonitorInfoA != NULL ) //If GetMonitorInfo() is available
	{
		MONITORINFO oMonitor;
		ZeroMemory( &oMonitor, sizeof(oMonitor) );
		oMonitor.cbSize = sizeof(oMonitor);
		
		theApp.m_pfnGetMonitorInfoA( theApp.m_pfnMonitorFromWindow( GetSafeHwnd(), MONITOR_DEFAULTTOPRIMARY ), &oMonitor );
		
		if ( abs( lpwndpos->x - oMonitor.rcWork.left ) < SNAP_SIZE )
			lpwndpos->x = oMonitor.rcWork.left;
		if ( abs( lpwndpos->y - oMonitor.rcWork.top ) < SNAP_SIZE )
			lpwndpos->y = oMonitor.rcWork.top;
		if ( abs( lpwndpos->x + lpwndpos->cx - oMonitor.rcWork.right ) < SNAP_SIZE )
			lpwndpos->x = oMonitor.rcWork.right - lpwndpos->cx;
		if ( abs( lpwndpos->y + lpwndpos->cy - oMonitor.rcWork.bottom ) < SNAP_SIZE )
			lpwndpos->y = oMonitor.rcWork.bottom - lpwndpos->cy;
	}
	else
	{
		CRect rcWork;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWork, 0 );
		
		if ( abs( lpwndpos->x ) <= ( rcWork.left + SNAP_SIZE ) )
		{
			lpwndpos->x = rcWork.left;
		}
		else if (	( lpwndpos->x + lpwndpos->cx ) >= ( rcWork.right - SNAP_SIZE ) &&
					( lpwndpos->x + lpwndpos->cx ) <= ( rcWork.right + SNAP_SIZE ) )
		{
			lpwndpos->x = rcWork.right - lpwndpos->cx;
		}
		
		if ( abs( lpwndpos->y ) <= ( rcWork.top + SNAP_SIZE ) )
		{
			lpwndpos->y = rcWork.top;
		}
		else if (	( lpwndpos->y + lpwndpos->cy ) >= ( rcWork.bottom - SNAP_SIZE ) &&
					( lpwndpos->y + lpwndpos->cy ) <= ( rcWork.bottom + SNAP_SIZE ) )
		{
			lpwndpos->y = rcWork.bottom-lpwndpos->cy;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd common timer

void CMainWnd::OnTimer(UINT nIDEvent) 
{
	// Fix resource handle
	
	if ( AfxGetResourceHandle() != m_hInstance )
		AfxSetResourceHandle( m_hInstance );

	// Propagate to children
	
	if ( m_bTimer ) return;
	m_bTimer = TRUE;
	
	for ( POSITION pos = m_pWindows.GetIterator() ; pos ; )
	{
		CChildWnd* pChild = m_pWindows.GetNext( pos );
		if ( pChild->IsPartiallyVisible() )	pChild->PostMessage( WM_TIMER, 1, 0 );
	}
	
	m_bTimer = FALSE;
	Settings.Live.LoadWindowState = FALSE;
	
	// Statistics
	
	Statistics.Update();
	
	// Hashing progress window
	
	m_wndHashProgressBar.Run();
	
	// Switch tray icon
	
	BOOL bNeedTrayIcon = m_bTrayHide || Settings.General.TrayMinimise || Settings.General.CloseMode == 2;
	
	if ( bNeedTrayIcon && ! m_bTrayIcon )
	{
		m_pTray.cbSize				= sizeof(m_pTray);
		m_pTray.hWnd				= GetSafeHwnd();
		m_pTray.uID					= 0;
		m_pTray.uFlags				= NIF_ICON | NIF_MESSAGE | NIF_TIP;
		m_pTray.uCallbackMessage	= WM_TRAY;
		
		if ( CoolInterface.IsNewWindows() )
			m_pTray.hIcon = AfxGetApp()->LoadIcon( IDI_SMALL );
		else
			m_pTray.hIcon = AfxGetApp()->LoadIcon( IDI_ICON );
		
		_tcscpy( m_pTray.szTip, _T("Shareaza") );
		m_bTrayIcon = ( Shell_NotifyIcon( NIM_ADD, &m_pTray ) != 0 );
	}
	else if ( m_bTrayIcon && ! bNeedTrayIcon )
	{
		Shell_NotifyIcon( NIM_DELETE, &m_pTray );
		m_bTrayIcon = FALSE;
	}
	
	// Menu Bar
	
	if ( m_wndMenuBar.IsWindowVisible() == FALSE ) ShowControlBar( &m_wndMenuBar, TRUE, FALSE );

	// Scheduler
	
	if ( Settings.Scheduler.Enable ) Schedule.Update();

	// Network / disk space / directory checks

	LocalSystemChecks();
	
	// Update messages
	
	UpdateMessages();
}

void CMainWnd::OnActivateApp(BOOL bActive, DWORD dwTask) 
{
	CMDIFrameWnd::OnActivateApp( bActive, dwTask );
	
	if ( ! bActive )
	{
		CoFreeUnusedLibraries();
		// SetProcessWorkingSetSize( GetCurrentProcess(), 0xFFFFFFFF, 0xFFFFFFFF );
	}
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
	
	if ( m_pSkin ) m_pSkin->OnGetMinMaxInfo( lpMMI );
}

LONG CMainWnd::OnLog(WPARAM wParam, LPARAM lParam)
{
	LPTSTR pszLog = (LPTSTR)lParam;
	
	if ( CSystemWnd* pWnd = (CSystemWnd*)m_pWindows.Find( RUNTIME_CLASS(CSystemWnd) ) )
	{
		pWnd->Add( (int)wParam, pszLog );
	}
	
	free( pszLog );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd tray functionality

void CMainWnd::CloseToTray()
{
	if ( m_bTrayHide ) return;

	ShowWindow( SW_HIDE );

	m_bTrayHide = TRUE;
}

void CMainWnd::OpenFromTray(int nShowCmd)
{
	if ( m_bTrayHide ) ShowWindow( nShowCmd );
	if ( IsIconic() ) SendMessage( WM_SYSCOMMAND, SC_RESTORE );
	
	SetForegroundWindow();
	m_bTrayHide = FALSE;
}

LONG CMainWnd::OnTray(UINT wParam, LONG lParam)
{
	if ( LOWORD(lParam) == WM_LBUTTONDBLCLK )
	{
		OpenFromTray();
	}
	else if ( LOWORD(lParam) == WM_RBUTTONDOWN )
	{
		UINT nFlags = TPM_RIGHTBUTTON;
		CPoint pt;
		CRect rc;
		
		GetCursorPos( &pt );
		SystemParametersInfo( SPI_GETWORKAREA, 0, &rc, 0 );
		
		nFlags |= TPM_CENTERALIGN;
		
		if ( pt.y > GetSystemMetrics( SM_CYSCREEN ) / 2 )
		{
			pt.y = rc.bottom;
			nFlags |= TPM_TOPALIGN;
		}
		else
		{
			pt.y = rc.top;
			nFlags |= TPM_BOTTOMALIGN;
		}
		
		SetForegroundWindow();
		
		CMenu* pMenu = Skin.GetMenu( _T("CMainWnd.Tray") );
		if ( pMenu == NULL ) return 0;
		
		MENUITEMINFO pInfo;
		pInfo.cbSize	= sizeof(pInfo);
		pInfo.fMask		= MIIM_STATE;
		GetMenuItemInfo( pMenu->GetSafeHmenu(), ID_TRAY_OPEN, FALSE, &pInfo );
		pInfo.fState	|= MFS_DEFAULT;
		SetMenuItemInfo( pMenu->GetSafeHmenu(), ID_TRAY_OPEN, FALSE, &pInfo );
		
		pMenu->TrackPopupMenu( nFlags, pt.x, pt.y, this, NULL );
		
		PostMessage( WM_NULL );
	}

	return 0;
}

void CMainWnd::OnTrayOpen() 
{
	OpenFromTray();
}

void CMainWnd::OnSysCommand(UINT nID, LPARAM lParam) 
{
	if ( ( nID & 0xFFF0 ) == SC_KEYMENU )
	{
		if ( lParam != ' ' && lParam != '-' )
		{
			if ( lParam )
				m_wndMenuBar.OpenMenuChar( lParam );
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

LONG CMainWnd::OnSkinChanged(UINT wParam, LONG lParam)
{
	CWaitCursor pCursor;
	
	m_pSkin = NULL;
	
	m_pWindows.PostSkinRemove();
	m_wndMenuBar.SetMenu( NULL );
	m_wndToolBar.Clear();
	
	CDownloadMonitorDlg::OnSkinChange( FALSE );
	CSettingsManagerDlg::OnSkinChange( FALSE );
	CFilePreviewDlg::OnSkinChange( FALSE );
	
	Skin.Apply();
	
	if ( CMenu* pMenu = Skin.GetMenu( _T("CMainWnd") ) )
	{
		m_wndMenuBar.SetMenu( pMenu->Detach() );
	}
	
	Skin.CreateToolBar( _T("CMainWnd"), &m_wndToolBar );
	m_wndNavBar.OnSkinChange();
	
	m_wndMenuBar.SetWatermark( Skin.GetWatermark( _T("CCoolMenuBar") ) );
	m_wndTabBar.SetWatermark( Skin.GetWatermark( _T("CWndTabBar") ) );
	
	if ( CWnd* pDockBar = GetDlgItem( AFX_IDW_DOCKBAR_TOP ) )
	{
		DWORD nBrush = GetClassLong( pDockBar->GetSafeHwnd(), GCL_HBRBACKGROUND );
		if ( nBrush > 64 ) DeleteObject( (HBRUSH)nBrush );
		nBrush = (DWORD)CreateSolidBrush( CoolInterface.m_crMidtone );
		SetClassLong( pDockBar->GetSafeHwnd(), GCL_HBRBACKGROUND, (LONG)nBrush );
	}
	
	m_pSkin = Skin.GetWindowSkin( this );
	
	if ( m_pSkin != NULL )
	{
		ModifyStyle( WS_CAPTION, 0 );
		m_pWindows.ModifyStyleEx( WS_EX_CLIENTEDGE, 0 );
		SetWindowRgn( NULL, TRUE );
		m_pSkin->OnSize( this );
	}
	else
	{
		ModifyStyle( 0, WS_CAPTION, 0 );
		m_pWindows.ModifyStyleEx( 0, WS_EX_CLIENTEDGE );
		SetWindowRgn( NULL, TRUE );
	}
	
	m_wndMonitorBar.OnSkinChange();
	m_wndRemoteWnd.OnSkinChange();
	
	SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|
		SWP_NOACTIVATE|SWP_NOZORDER|SWP_FRAMECHANGED|SWP_DRAWFRAME );
	
	UpdateMessages();
	
	if ( TRUE )
	{
		m_wndToolBar.OnUpdated();
		m_pWindows.PostSkinChange();
		m_wndTabBar.OnUpdateCmdUI( this, FALSE );
		RedrawWindow( NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_ALLCHILDREN );
	}
	
	CSettingsManagerDlg::OnSkinChange( TRUE );
	CDownloadMonitorDlg::OnSkinChange( TRUE );
	CFilePreviewDlg::OnSkinChange( TRUE );
	
	Invalidate();
	
	return 0;
}

LONG CMainWnd::OnWinsock(WPARAM wParam, LPARAM lParam)
{
	Network.OnWinsock( wParam, lParam );
	return 0;
}

LONG CMainWnd::OnHandleURL(WPARAM wParam, LPARAM lParam)
{
	CShareazaURL* pURL = (CShareazaURL*)wParam;
	
	DWORD tNow = GetTickCount();
	BOOL bSoon = ( tNow - m_tURLTime < 750 );
	m_tURLTime = tNow;
	
	if ( IsWindowEnabled() )
	{
		m_pURLDialog = new CURLActionDlg( this, pURL, bSoon );
		m_pURLDialog->DoModal();
		delete m_pURLDialog;
		m_pURLDialog = NULL;
	}
	else
	{
		if ( m_pURLDialog != NULL && bSoon )
		{
			m_pURLDialog->AddURL( pURL );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_URL_BUSY );
			delete pURL;
		}
	}
	
	return 0;
}

LONG CMainWnd::OnHandleCollection(WPARAM wParam, LPARAM lParam)
{
	LPTSTR pszPath = (LPTSTR)wParam;
	CString strPath( pszPath );
	delete [] pszPath;
	
	if ( CLibraryWnd* pLibrary = (CLibraryWnd*)m_pWindows.Open( RUNTIME_CLASS(CLibraryWnd) ) )
	{
		pLibrary->OnCollection( strPath );
	}
	
	return 0;
}

LONG CMainWnd::OnVersionCheck(WPARAM wParam, LPARAM lParam)
{
	if ( wParam == 0 && VersionChecker.m_sMessage.GetLength() )
	{
		AfxMessageBox( VersionChecker.m_sMessage, MB_ICONINFORMATION );
	}
	
	if ( ( wParam == 0 || wParam == 1 ) && VersionChecker.m_bUpgrade )
	{
		CUpgradeDlg dlg;
		dlg.DoModal();
	}
	
	if ( wParam == 2 && VersionChecker.m_sUpgradePath.GetLength() )
	{
		CString strFormat, strMessage;
		
		LoadString( strFormat, IDS_UPGRADE_LAUNCH );
		strMessage.Format( strFormat, (LPCTSTR)VersionChecker.m_sUpgradeFile );
		
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
		{
			ShellExecute( GetSafeHwnd(), _T("open"), VersionChecker.m_sUpgradePath,
				_T("/launch"), NULL, SW_SHOWNORMAL );
			PostMessage( WM_CLOSE );
		}
	}
	
	return 0;
}

LONG CMainWnd::OnOpenChat(WPARAM wParam, LPARAM lParam)
{
	CSingleLock pLock( &ChatCore.m_pSection, TRUE );
	CChatSession* pSession = (CChatSession*)wParam;
	if ( ChatCore.Check( pSession ) ) pSession->OnOpenWindow();
	return 0;
}

LONG CMainWnd::OnOpenSearch(WPARAM wParam, LPARAM lParam)
{
	CQuerySearch* pSearch = (CQuerySearch*)wParam;
	if ( pSearch->OpenWindow() == NULL ) delete pSearch;
	return 0;
}

LONG CMainWnd::OnMediaKey(WPARAM wParam, LPARAM lParam)
{
	if ( CMediaWnd* pWnd = (CMediaWnd*)m_pWindows.Find( RUNTIME_CLASS(CMediaWnd) ) )
	{
		return pWnd->SendMessage( 0x0319, 1, lParam );
	}
	return 0;
}

LONG CMainWnd::OnDevModeChange(WPARAM wParam, LPARAM lParam)
{
	if ( CMediaWnd* pWnd = (CMediaWnd*)m_pWindows.Find( RUNTIME_CLASS(CMediaWnd) ) )
	{
		return pWnd->SendMessage( WM_DEVMODECHANGE, wParam, lParam );
	}
	return 0;
}

LONG CMainWnd::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	if ( CMediaWnd* pWnd = (CMediaWnd*)m_pWindows.Find( RUNTIME_CLASS(CMediaWnd) ) )
	{
		return pWnd->SendMessage( WM_DISPLAYCHANGE, wParam, lParam );
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd status message functionality

LONG CMainWnd::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	if ( wParam == AFX_IDS_IDLEMESSAGE )
	{
		CString strOld;
		m_wndStatusBar.GetWindowText( strOld );
		if ( strOld != m_sMsgStatus ) m_wndStatusBar.SetWindowText( m_sMsgStatus );
		m_nIDLastMessage = m_nIDTracking = wParam;
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
	CString strFormat, strMessage, strOld;

	int nCount = Network.IsWellConnected();

	if ( nCount > 0 )
	{	//If you have neighbours, you are connected
		QWORD nLocalVolume;
		LibraryMaps.GetStatistics( NULL, &nLocalVolume );

		if( Settings.General.GUIMode == GUI_BASIC )
		{	//In the basic GUI, don't bother with mode details or neighbour count.
			LoadString( strFormat, IDS_STATUS_BAR_CONNECTED_SIMPLE );
			strMessage.Format( strFormat, (LPCTSTR)Settings.SmartVolume( nLocalVolume, TRUE ) );
		}
		else
		{	//Display node type and number of neighbours
			if (  Neighbours.IsG2Hub() )
			{
				if (  Neighbours.IsG1Ultrapeer() )
					LoadString( strFormat, IDS_STATUS_BAR_CONNECTED_HUB_UP );
				else
					LoadString( strFormat, IDS_STATUS_BAR_CONNECTED_HUB );
			}
			else
			{
				if (  Neighbours.IsG1Ultrapeer() )
					LoadString( strFormat, IDS_STATUS_BAR_CONNECTED_UP );
				else
					LoadString( strFormat, IDS_STATUS_BAR_CONNECTED );
			}
			strMessage.Format( strFormat, nCount,
								(LPCTSTR)Settings.SmartVolume( nLocalVolume, TRUE ) );
		}
	}
	else if ( Network.IsConnected() )
	{	//Trying to connect
		LoadString( strMessage, IDS_STATUS_BAR_CONNECTING );
	}
	else
	{	//Idle
		LoadString( strMessage, IDS_STATUS_BAR_DISCONNECTED );
	}

	if ( VersionChecker.m_sQuote.GetLength() )
	{
		strMessage += _T("  ");
		strMessage += VersionChecker.m_sQuote;
	}

	if ( m_nIDLastMessage == AFX_IDS_IDLEMESSAGE )
	{
		m_wndStatusBar.GetWindowText( strOld );
		if ( strOld != strMessage ) m_wndStatusBar.SetWindowText( strMessage );
	}

	m_sMsgStatus = strMessage;

	LoadString( strFormat, IDS_STATUS_BAR_BANDWIDTH );
	strMessage.Format( strFormat,
		(LPCTSTR)Settings.SmartVolume( CGraphItem::GetValue( GRC_TOTAL_BANDWIDTH_IN ), FALSE, TRUE ),
		(LPCTSTR)Settings.SmartVolume( CGraphItem::GetValue( GRC_TOTAL_BANDWIDTH_OUT ), FALSE, TRUE ),
		(int)CGraphItem::GetValue( GRC_DOWNLOADS_TRANSFERS, 0 ),
		(int)CGraphItem::GetValue( GRC_UPLOADS_TRANSFERS, 0 ) );

	m_wndStatusBar.GetPaneText( 1, strOld );
	if ( strOld != strMessage ) m_wndStatusBar.SetPaneText( 1, strMessage );

	if ( m_bTrayIcon )
	{
		LoadString( strFormat, IDS_TRAY_TIP );
		strMessage.Format( strFormat,
			(int)CGraphItem::GetValue( GRC_GNUTELLA_CONNECTIONS, 0 ),
			(LPCTSTR)Settings.SmartVolume( CGraphItem::GetValue( GRC_TOTAL_BANDWIDTH_IN ), FALSE, TRUE ),
			(LPCTSTR)Settings.SmartVolume( CGraphItem::GetValue( GRC_TOTAL_BANDWIDTH_OUT ), FALSE, TRUE ),
			(int)CGraphItem::GetValue( GRC_DOWNLOADS_TRANSFERS ),
			(int)CGraphItem::GetValue( GRC_UPLOADS_TRANSFERS ) );

		if ( strMessage != m_pTray.szTip )
		{
			m_pTray.uFlags = NIF_TIP;
			_tcsncpy( m_pTray.szTip, strMessage, 63 );
			if ( ! Shell_NotifyIcon( NIM_MODIFY, &m_pTray ) ) m_bTrayIcon = FALSE;
		}
	}
	
	LoadString( strMessage, IDR_MAINFRAME );
	
	if ( Settings.Live.AutoClose )
	{
		LoadString( strOld, IDS_CLOSING_AFTER );
		strMessage += strOld;
	}
	
	if ( _tcsistr( strMessage, _T("Shareaza") ) == NULL )
	{
		strMessage = _T("Shareaza ") + strMessage;
	}
	
	GetWindowText( strOld );
	if ( strOld != strMessage ) SetWindowText( strMessage );
}

// This function runs some basic checks that everything is okay- disks, directories, local network is
// up, etc.
void CMainWnd::LocalSystemChecks()
{
	static DWORD tLastCheck = 0;			// Time the checks were last run
	static DWORD nConnectionFailCount = 0;	// Counter for times a connection problem has been detected
	DWORD tTicks = GetTickCount();			// Current time

	if ( tTicks - tLastCheck > 1 * 60 * 1000 )  // Run once every minute
	{
		tLastCheck = tTicks;

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
			else if ( theApp.m_bNT )
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
				if ( ( Network.IsAvailable() ) || ( Network.IsWellConnected() > 0 ) )				
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

						Network.Disconnect();
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
						Network.Connect();
						theApp.Message( MSG_ERROR, _T("Internet reconnect detected- restarting network") );
					}
				}
			}
		}


		// Check we have donkey servers
		if ( ( Settings.Live.DonkeyServerWarning == FALSE ) && ( Settings.eDonkey.EnableToday ) )
		{
			if ( ( ! Settings.eDonkey.MetAutoQuery ) && ( HostCache.eDonkey.CountHosts() < 1 ) )
			{
				Settings.Live.DonkeyServerWarning = TRUE;
				PostMessage( WM_COMMAND, ID_HELP_DONKEYSERVERS );
			}
		}
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
	
	if ( Settings.Search.SearchPanel && ! m_bTrayHide && ! IsIconic() )
	{
		m_pWindows.OpenNewSearchWindow();
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

	if ( Network.IsWellConnected() )
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
	CConnectToDlg dlg;
	if ( dlg.DoModal() != IDOK ) return;
	Network.ConnectTo( dlg.m_sHost, dlg.m_nPort, dlg.m_nProtocol + 1, dlg.m_bNoUltraPeer );
}

void CMainWnd::OnNetworkBrowseTo() 
{
	CConnectToDlg dlg( NULL, TRUE );
	if ( dlg.DoModal() != IDOK ) return;

	SOCKADDR_IN pAddress;

	if ( Network.Resolve( dlg.m_sHost, dlg.m_nPort, &pAddress ) )
	{
		new CBrowseHostWnd( &pAddress );
	}
}

void CMainWnd::OnUpdateNetworkG2(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( Settings.Gnutella2.EnableToday );
}

void CMainWnd::OnNetworkG2() 
{
	if ( Network.IsConnected() && Settings.Gnutella2.EnableToday )
	{
		CString strMessage;
		LoadString( strMessage, IDS_NETWORK_DISABLE_G2 );
		
		if ( AfxMessageBox( strMessage, MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2 ) == IDYES )
		{
			Settings.Gnutella2.EnableToday = FALSE;
		}
	}
	else
	{
		Settings.Gnutella2.EnableToday = TRUE;
		Network.Connect( TRUE );
	}
}

void CMainWnd::OnUpdateNetworkG1(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( Settings.Gnutella1.EnableToday );
}

void CMainWnd::OnNetworkG1() 
{
	if ( Network.IsConnected() && Settings.Gnutella1.EnableToday )
	{
		Settings.Gnutella1.EnableToday = FALSE;
	}
	else
	{
		Settings.Gnutella1.EnableToday = TRUE;
		Network.Connect( TRUE );
	}
}

void CMainWnd::OnUpdateNetworkED2K(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( Settings.eDonkey.EnableToday );
	pCmdUI->Enable( Settings.GetOutgoingBandwidth() >= 2 );
}

void CMainWnd::OnNetworkED2K() 
{
	if ( Network.IsConnected() && Settings.eDonkey.EnableToday )
	{
		Settings.eDonkey.EnableToday = FALSE;
	}
	else
	{
		Settings.eDonkey.EnableToday = Settings.GetOutgoingBandwidth() >= 2;
		Network.Connect( TRUE );
	}
}

void CMainWnd::OnUpdateNetworkAutoClose(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( Settings.Live.AutoClose || ( Transfers.GetActiveCount() > 0 ) );
	pCmdUI->SetCheck( Settings.Live.AutoClose );
}

void CMainWnd::OnNetworkAutoClose() 
{
	if ( Settings.Live.AutoClose )
	{
		Settings.Live.AutoClose = FALSE;
	}
	else
	{
		Network.Disconnect();
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
	m_pWindows.Open( RUNTIME_CLASS(CLibraryWnd), TRUE );
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
		m_pWindows.Open( RUNTIME_CLASS(CMediaWnd), TRUE );
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

void CMainWnd::OnUpdateViewHelp(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_pWindows.Find( RUNTIME_CLASS(CHelpWnd) ) != NULL );
}

void CMainWnd::OnViewHelp() 
{
	m_pWindows.Open( RUNTIME_CLASS(CHelpWnd), TRUE );
	OpenFromTray();
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd tab menu

void CMainWnd::OnUpdateTabConnect(CCmdUI* pCmdUI) 
{
	CCoolBarItem* pItem = m_wndToolBar.GetID( ID_TAB_CONNECT );
	
	UINT nTextID = 0;
	UINT nTipID = 0;
	
	if ( Network.IsWellConnected() )
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
		if ( pItem ) pItem->SetTextColour( CoolInterface.m_crCmdText == 0 ? RGB( 0, 127, 0 ) : CoolInterface.m_crCmdText );
		m_wndTabBar.SetMessage( (UINT)0 );
		nTextID	= IDS_NETWORK_CONNECTING;
		nTipID	= ID_NETWORK_DISCONNECT;
	}
	else
	{
		if ( pItem ) pItem->SetCheck( FALSE );
		if ( pItem ) pItem->SetTextColour( CoolInterface.m_crCmdText == 0 ? RGB( 0, 127, 0 ) : CoolInterface.m_crCmdText );
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
	CChildWnd* pChild = m_pWindows.GetActive();
	pCmdUI->SetCheck( pChild && pChild->IsKindOf( RUNTIME_CLASS(CHomeWnd) ) );
}

void CMainWnd::OnTabHome() 
{
	m_pWindows.Open( RUNTIME_CLASS(CHomeWnd) );
	OpenFromTray();
}

void CMainWnd::OnUpdateTabLibrary(CCmdUI* pCmdUI) 
{
	CChildWnd* pChild = m_pWindows.GetActive();
	pCmdUI->SetCheck( pChild && pChild->IsKindOf( RUNTIME_CLASS(CLibraryWnd) ) );
}

void CMainWnd::OnTabLibrary() 
{
	m_pWindows.Open( RUNTIME_CLASS(CLibraryWnd) );
	OpenFromTray();
}

void CMainWnd::OnUpdateTabMedia(CCmdUI* pCmdUI) 
{
	CMediaWnd* pChild = (CMediaWnd*)m_pWindows.GetActive();
	pCmdUI->SetCheck( pChild && pChild->IsKindOf( RUNTIME_CLASS(CMediaWnd) ) );

	if ( CCoolBarItem* pItem = m_wndToolBar.GetID( ID_TAB_MEDIA ) )
	{
		if ( pChild = (CMediaWnd*)m_pWindows.Find( RUNTIME_CLASS(CMediaWnd) ) )
		{
			pItem->SetTextColour( pChild->IsPlaying() ? RGB( 0, 0x80, 0 ) : CoolInterface.m_crCmdText );
		}
		else
		{
			pItem->SetTextColour( CoolInterface.m_crCmdText );
		}
	}
}

void CMainWnd::OnTabMedia() 
{
	m_pWindows.Open( RUNTIME_CLASS(CMediaWnd) );
	OpenFromTray();
}

void CMainWnd::OnUpdateTabSearch(CCmdUI* pCmdUI) 
{
	CChildWnd* pChild = m_pWindows.GetActive();
	// pCmdUI->SetCheck( pChild && pChild->IsKindOf( RUNTIME_CLASS(CSearchPadWnd) ) );
	pCmdUI->SetCheck( pChild && pChild->IsKindOf( RUNTIME_CLASS(CSearchWnd) ) );
}

void CMainWnd::OnTabSearch() 
{
	if ( Settings.General.GUIMode != GUI_WINDOWED )
	{
		// m_pWindows.Open( RUNTIME_CLASS(CSearchPadWnd) );
		m_pWindows.OpenNewSearchWindow();
		OpenFromTray();
	}
	else
	{
		PostMessage( WM_COMMAND, ID_NETWORK_SEARCH );
	}
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

void CMainWnd::OnUpdateTabHelp(CCmdUI* pCmdUI) 
{
	CChildWnd* pChild = m_pWindows.GetActive();
	pCmdUI->SetCheck( pChild && pChild->IsKindOf( RUNTIME_CLASS(CHelpWnd) ) );
}

void CMainWnd::OnTabHelp() 
{
	m_pWindows.Open( RUNTIME_CLASS(CHelpWnd) );
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
	CDownloadDlg dlg;
	if ( dlg.DoModal() != IDOK ) return;

	if ( dlg.m_pURL->m_nAction == CShareazaURL::uriDownload )
	{
		Downloads.Add( dlg.m_pURL );
		if ( ! Network.IsWellConnected() ) Network.Connect( TRUE );
		m_pWindows.Open( RUNTIME_CLASS(CDownloadsWnd) );
	}
	else if ( dlg.m_pURL->m_nAction == CShareazaURL::uriSource )
	{
		Downloads.Add( dlg.m_pURL );
		m_pWindows.Open( RUNTIME_CLASS(CDownloadsWnd) );
	}
	else
	{
		PostMessage( WM_URL, (WPARAM)dlg.GetURL() );
	}
}

void CMainWnd::OnToolsSkin() 
{
	if ( ! IsWindowEnabled() ) return;
	CSettingsManagerDlg::Run( _T("CSkinsSettingsPage") );
}

void CMainWnd::OnToolsLanguage() 
{
	if ( ! IsWindowEnabled() ) return;
	theApp.WriteProfileInt( _T("Windows"), _T("RunLanguage"), TRUE );
	
	CLanguageDlg dlg;
	
	if ( dlg.DoModal() == IDOK )
	{
		CWaitCursor pCursor;
		OnSkinChanged( 0, 0 );
		SetGUIMode( Settings.General.GUIMode );
	}
	
	if ( theApp.GetProfileInt( _T("Windows"), _T("RunWizard"), FALSE ) == FALSE )
	{
		PostMessage( WM_COMMAND, ID_TOOLS_WIZARD );
	}
}

void CMainWnd::OnToolsMercora()
{
	if ( CWnd* pWnd = FindWindow( _T("MercoraClient"), NULL ) )
	{
		pWnd->ShowWindow( SW_SHOWNORMAL );
		pWnd->BringWindowToTop();
		pWnd->SetForegroundWindow();
		return;
	}
	
	HKEY hKey = NULL;
	RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("Software\\Mercora\\MercoraClient"), 0, KEY_READ, &hKey );
	
	if ( hKey != NULL )
	{
		DWORD nType = REG_SZ, nEXE = sizeof(TCHAR) * MAX_PATH;
		TCHAR szEXE[MAX_PATH+1];
		RegQueryValueEx( hKey, _T("EXE"), NULL, &nType, (PBYTE)szEXE, &nEXE );
		szEXE[ nEXE ] = 0;
		RegCloseKey( hKey );
		int nResult = (int)ShellExecute( GetSafeHwnd(), NULL, szEXE, NULL, NULL, SW_SHOWNORMAL );
		if ( nResult > 32 ) return;
	}
	
	ShellExecute( GetSafeHwnd(), NULL, _T("http://www.mercora.com/"), NULL, NULL, SW_SHOWNORMAL );
}


void CMainWnd::OnToolsSeedTorrent()
{
	CFileDialog dlgFile( TRUE, _T("torrent"), ( Settings.Downloads.TorrentPath + "\\." ) , OFN_HIDEREADONLY,
		_T("Torrent Files|*.torrent|All Files|*.*||"), this );
	
	if ( dlgFile.DoModal() != IDOK ) return;
	
	CTorrentSeedDlg dlgSeed( dlgFile.GetPathName() );
	dlgSeed.DoModal();
}

void CMainWnd::OnToolsReseedTorrent()
{
	CTorrentSeedDlg dlgSeed( LibraryHistory.LastSeededTorrent.m_sPath );
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
	theApp.WriteProfileInt( _T("Windows"), _T("RunWizard"), TRUE );
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
	ShowControlBar( &m_wndToolBar, FALSE, TRUE );
	ShowControlBar( &m_wndNavBar, ! m_wndNavBar.IsVisible(), TRUE );
	if ( ! m_wndNavBar.IsVisible() )
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
	ShowControlBar( &m_wndNavBar, FALSE, TRUE );
	ShowControlBar( &m_wndToolBar, ! m_wndToolBar.IsVisible(), TRUE );
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
	ShowControlBar( &m_wndMonitorBar, ! m_wndMonitorBar.IsVisible(), TRUE );
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
	if ( CMediaFrame::g_pMediaFrame != NULL )
		pCmdUI->ContinueRouting();
	else
		pCmdUI->Enable( TRUE );
}

void CMainWnd::OnMediaCommand()
{
	if ( CMediaFrame::g_pMediaFrame == NULL ) OnTabMedia();
	if ( CMediaFrame::g_pMediaFrame == NULL ) return;
	CMediaFrame::g_pMediaFrame->SendMessage( WM_COMMAND, GetCurrentMessage()->wParam );
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd help menu

void CMainWnd::OnHelpAbout() 
{
	CAboutDlg dlg;
	dlg.DoModal();
	Neighbours.IsG1UltrapeerCapable( TRUE );
	Neighbours.IsG2HubCapable( TRUE );
}

void CMainWnd::OnHelpHomepage() 
{
	ShellExecute( GetSafeHwnd(), _T("open"),
		_T("http://www.shareaza.com/?Version=") + theApp.m_sVersion,
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWeb1() 
{
	ShellExecute( GetSafeHwnd(), _T("open"), _T("http://www.shareaza.com/help/external/?link1"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWeb2() 
{
	ShellExecute( GetSafeHwnd(), _T("open"), _T("http://www.shareaza.com/help/external/?link2"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWeb3() 
{
	ShellExecute( GetSafeHwnd(), _T("open"), _T("http://www.shareaza.com/help/external/?link3"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWeb4() 
{
	ShellExecute( GetSafeHwnd(), _T("open"), _T("http://www.shareaza.com/help/external/?link4"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWeb5() 
{
	ShellExecute( GetSafeHwnd(), _T("open"), _T("http://www.shareaza.com/help/external/?link5"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWeb6() 
{
	ShellExecute( GetSafeHwnd(), _T("open"), _T("http://www.shareaza.com/help/external/?link6"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpFaq() 
{
	ShellExecute( GetSafeHwnd(), _T("open"),
		_T("http://www.shareaza.com/help/?faq"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpGuide() 
{
	ShellExecute( GetSafeHwnd(), _T("open"),
		_T("http://www.shareaza.com/help/?guide"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpForums() 
{
	ShellExecute( GetSafeHwnd(), _T("open"),
		_T("http://forums.shareaza.com/"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpUpdate() 
{
	ShellExecute( GetSafeHwnd(), _T("open"),
		_T("http://www.shareaza.com/help/update/?Version=") + theApp.m_sVersion,
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpRouter()
{
	ShellExecute( GetSafeHwnd(), _T("open"), _T("http://www.shareaza.com/help/?router"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpSecurity()
{
	ShellExecute( GetSafeHwnd(), _T("open"), _T("http://www.shareaza.com/help/?security"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpCodec()
{
	ShellExecute( GetSafeHwnd(), _T("open"), _T("http://www.shareaza.com/help/?codec"),
		NULL, NULL, SW_SHOWNORMAL );
}

void CMainWnd::OnHelpWarnings() 
{
	if ( IsWindowEnabled() && IsWindowVisible() && ! IsIconic() )
	{
		theApp.WriteProfileInt( _T("Windows"), _T("RunWarnings"), TRUE );
		CWarningsDlg dlg;
		dlg.DoModal();
	}
}

void CMainWnd::OnHelpPromote() 
{
	if ( IsWindowEnabled() && IsWindowVisible() && ! IsIconic() )
	{
		theApp.WriteProfileInt( _T("Windows"), _T("RunPromote"), TRUE );
		CPromoteDlg dlg;
		dlg.DoModal();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd skin forwarding

void CMainWnd::OnSize(UINT nType, int cx, int cy) 
{
	if ( m_pSkin ) m_pSkin->OnSize( this );
	CMDIFrameWnd::OnSize( nType, cx, cy );
}

void CMainWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	if ( m_pSkin )
		m_pSkin->OnNcCalcSize( this, bCalcValidRects, lpncsp );
	else
		CMDIFrameWnd::OnNcCalcSize( bCalcValidRects, lpncsp );
}

UINT CMainWnd::OnNcHitTest(CPoint point) 
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
	{
		return CMDIFrameWnd::OnNcActivate( bActive );
	}
}

void CMainWnd::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	if ( m_pSkin ) m_pSkin->OnNcMouseMove( this, nHitTest, point );
	CMDIFrameWnd::OnNcMouseMove( nHitTest, point );
}

void CMainWnd::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDown( this, nHitTest, point ) ) return;
	CMDIFrameWnd::OnNcLButtonDown( nHitTest, point );
}

void CMainWnd::OnNcLButtonUp(UINT nHitTest, CPoint point) 
{
	if ( m_pSkin && m_pSkin->OnNcLButtonUp( this, nHitTest, point ) ) return;
	CMDIFrameWnd::OnNcLButtonUp( nHitTest, point );
}

void CMainWnd::OnNcLButtonDblClk(UINT nHitTest, CPoint point) 
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDblClk( this, nHitTest, point ) ) return;
	CMDIFrameWnd::OnNcLButtonDblClk( nHitTest, point );
}

LONG CMainWnd::OnSetText(WPARAM wParam, LPARAM lParam)
{
	if ( m_pSkin )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible ) ModifyStyle( WS_VISIBLE, 0 );
		LONG lResult = Default();
		if ( bVisible ) ModifyStyle( 0, WS_VISIBLE );
		if ( m_pSkin ) m_pSkin->OnSetText( this );
		return lResult;
	}
	else
	{
		return Default();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd IDropTarget implementation

IMPLEMENT_UNKNOWN(CMainWnd, DropTarget)

STDMETHODIMP CMainWnd::XDropTarget::DragEnter(IDataObject FAR* pDataObj, DWORD grfKeyState, POINTL pt, DWORD FAR* pdwEffect)
{
	METHOD_PROLOGUE( CMainWnd, DropTarget )
	
	if ( ObjectToFiles( pDataObj ) )
	{
		*pdwEffect = DROPEFFECT_COPY;
		return S_OK;
	}
	else if ( ObjectToURL( pDataObj ).GetLength() > 0 )
	{
		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMainWnd::XDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD FAR* pdwEffect)
{
	METHOD_PROLOGUE( CMainWnd, DropTarget )
	
	if ( m_pFiles.GetCount() > 0 )
	{
		CPoint point( pt.x, pt.y );
		pThis->ClientToScreen( &point );
		*pdwEffect = DROPEFFECT_NONE;
		
		if ( IsTorrentFile() )
		{
			*pdwEffect = DROPEFFECT_COPY;
		}
		else if ( CChildWnd* pChild = pThis->m_pWindows.FindFromPoint( point ) )
		{
			if ( pChild->OnDropFiles( m_pFiles, point, FALSE ) )
			{
				*pdwEffect = DROPEFFECT_COPY;
			}
		}
	}
	else
	{
		*pdwEffect = DROPEFFECT_LINK;
	}
	
	return S_OK;
}

STDMETHODIMP CMainWnd::XDropTarget::DragLeave()
{
	METHOD_PROLOGUE( CMainWnd, DropTarget )
	return S_OK;
}

STDMETHODIMP CMainWnd::XDropTarget::Drop(IDataObject FAR* pDataObj, DWORD grfKeyState, POINTL pt, DWORD FAR* pdwEffect)
{
	METHOD_PROLOGUE( CMainWnd, DropTarget )
	
	if ( ObjectToFiles( pDataObj ) )
	{
		CPoint point( pt.x, pt.y );
		pThis->ClientToScreen( &point );
		
		if ( IsTorrentFile() )
		{
			theApp.Message( MSG_SYSTEM, IDS_BT_PREFETCH_FILE, (LPCTSTR)m_pFiles.GetHead() );
			
			CBTInfo* pTorrent = new CBTInfo();
			
			if ( pTorrent->LoadTorrentFile( m_pFiles.GetHead() ) )
			{
				if ( pTorrent->HasEncodingError() ) theApp.Message( MSG_SYSTEM, _T("Possible encoding error detected while parsing torrent") );
				CShareazaURL* pURL = new CShareazaURL( pTorrent );
				if ( pThis->PostMessage( WM_URL, (WPARAM)pURL ) ) return S_OK;
				delete pURL;
			}
			
			delete pTorrent;
			theApp.Message( MSG_ERROR, IDS_BT_PREFETCH_ERROR, (LPCTSTR)m_pFiles.GetHead() );
		}
		else if ( CChildWnd* pChild = pThis->m_pWindows.FindFromPoint( point ) )
		{
			pChild->OnDropFiles( m_pFiles, point, TRUE );
		}
	}
	else
	{
		CString strURL = ObjectToURL( pDataObj );
		theApp.Message( MSG_SYSTEM, IDS_URL_RECEIVED, (LPCTSTR)strURL );
		
		CShareazaURL* pURL = new CShareazaURL();
		if ( pURL->Parse( strURL ) && pThis->PostMessage( WM_URL, (WPARAM)pURL ) ) return S_OK;
		delete pURL;
		
		theApp.Message( MSG_ERROR, IDS_URL_PARSE_ERROR );
	}
	
	return S_OK;
}

CString CMainWnd::XDropTarget::ObjectToURL(IDataObject* pObject)
{
	FORMATETC pFormat = { 0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM pMedium;
	CString str;
	
	pFormat.cfFormat = RegisterClipboardFormat( _T("UniformResourceLocator") );
	
	if ( SUCCEEDED( pObject->GetData( &pFormat, &pMedium ) ) && pMedium.hGlobal != NULL )
	{
		if ( LPCSTR psz = (LPCSTR)GlobalLock( pMedium.hGlobal ) )
		{
			str = psz;
			GlobalUnlock( pMedium.hGlobal );
		}
		
		ReleaseStgMedium( &pMedium );
	}
	
	return str;
}

BOOL CMainWnd::XDropTarget::ObjectToFiles(IDataObject* pObject)
{
	FORMATETC pFormat = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM pMedium;

	m_pFiles.RemoveAll();
	
	if ( FAILED( pObject->GetData( &pFormat, &pMedium ) ) ) return FALSE;
	if ( pMedium.hGlobal == NULL ) return FALSE;
	
	HDROP	hDropInfo	= (HDROP)pMedium.hGlobal;
	UINT	nCount		= DragQueryFile( hDropInfo, 0xFFFFFFFF, NULL, 0 );
	
	for ( UINT nFile = 0 ; nFile < nCount ; nFile++ )
	{
		TCHAR szFile[MAX_PATH];
		DragQueryFile( hDropInfo, nFile, szFile, MAX_PATH );
		m_pFiles.AddTail( szFile );
	}
	
	ReleaseStgMedium( &pMedium );
	
	return ( m_pFiles.GetCount() > 0 );
}

BOOL CMainWnd::XDropTarget::IsTorrentFile() const
{
	return	m_pFiles.GetCount() == 1 &&
			_tcsistr( m_pFiles.GetHead(), _T(".torrent") ) != NULL;
}
