//
// WndDownloads.cpp
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
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadGroups.h"

#include "Library.h"
#include "LibraryMaps.h"
#include "SharedFile.h"
#include "FileExecutor.h"
#include "ShareazaURL.h"
#include "SHA.h"
#include "ChatWindows.h"

#include "WindowManager.h"
#include "WndDownloads.h"
#include "WndUploads.h"
#include "WndBrowseHost.h"
#include "DlgDownload.h"
#include "DlgDownloadEdit.h"
#include "DlgSettingsManager.h"
#include "DlgTorrentTracker.h"
#include "DlgDeleteFile.h"
#include "DlgFilePropertiesSheet.h"
#include "DlgURLCopy.h"
#include "DlgHelp.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CDownloadsWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CDownloadsWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_MDIACTIVATE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SETCURSOR()
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_RESUME, OnUpdateDownloadsResume)
	ON_COMMAND(ID_DOWNLOADS_RESUME, OnDownloadsResume)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_PAUSE, OnUpdateDownloadsPause)
	ON_COMMAND(ID_DOWNLOADS_PAUSE, OnDownloadsPause)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_CLEAR, OnUpdateDownloadsClear)
	ON_COMMAND(ID_DOWNLOADS_CLEAR, OnDownloadsClear)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_LAUNCH, OnUpdateDownloadsLaunch)
	ON_COMMAND(ID_DOWNLOADS_LAUNCH, OnDownloadsLaunch)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_SOURCES, OnUpdateDownloadsSources)
	ON_COMMAND(ID_DOWNLOADS_SOURCES, OnDownloadsSources)
	ON_COMMAND(ID_DOWNLOADS_CLEAR_COMPLETED, OnDownloadsClearCompleted)
	ON_COMMAND(ID_DOWNLOADS_CLEAR_PAUSED, OnDownloadsClearPaused)
	ON_UPDATE_COMMAND_UI(ID_TRANSFERS_DISCONNECT, OnUpdateTransfersDisconnect)
	ON_COMMAND(ID_TRANSFERS_DISCONNECT, OnTransfersDisconnect)
	ON_UPDATE_COMMAND_UI(ID_TRANSFERS_FORGET, OnUpdateTransfersForget)
	ON_COMMAND(ID_TRANSFERS_FORGET, OnTransfersForget)
	ON_UPDATE_COMMAND_UI(ID_TRANSFERS_CHAT, OnUpdateTransfersChat)
	ON_COMMAND(ID_TRANSFERS_CHAT, OnTransfersChat)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_URL, OnUpdateDownloadsUrl)
	ON_COMMAND(ID_DOWNLOADS_URL, OnDownloadsUrl)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_ENQUEUE, OnUpdateDownloadsEnqueue)
	ON_COMMAND(ID_DOWNLOADS_ENQUEUE, OnDownloadsEnqueue)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_AUTO_CLEAR, OnUpdateDownloadsAutoClear)
	ON_COMMAND(ID_DOWNLOADS_AUTO_CLEAR, OnDownloadsAutoClear)
	ON_UPDATE_COMMAND_UI(ID_TRANSFERS_CONNECT, OnUpdateTransfersConnect)
	ON_COMMAND(ID_TRANSFERS_CONNECT, OnTransfersConnect)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_SHOW_SOURCES, OnUpdateDownloadsShowSources)
	ON_COMMAND(ID_DOWNLOADS_SHOW_SOURCES, OnDownloadsShowSources)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_LAUNCH, OnUpdateBrowseLaunch)
	ON_COMMAND(ID_BROWSE_LAUNCH, OnBrowseLaunch)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_BOOST, OnUpdateDownloadsBoost)
	ON_COMMAND(ID_DOWNLOADS_BOOST, OnDownloadsBoost)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_LAUNCH_COPY, OnUpdateDownloadsLaunchCopy)
	ON_COMMAND(ID_DOWNLOADS_LAUNCH_COPY, OnDownloadsLaunchCopy)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_MONITOR, OnUpdateDownloadsMonitor)
	ON_COMMAND(ID_DOWNLOADS_MONITOR, OnDownloadsMonitor)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FILE_DELETE, OnUpdateDownloadsFileDelete)
	ON_COMMAND(ID_DOWNLOADS_FILE_DELETE, OnDownloadsFileDelete)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_RATE, OnUpdateDownloadsRate)
	ON_COMMAND(ID_DOWNLOADS_RATE, OnDownloadsRate)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_MOVE_UP, OnUpdateDownloadsMoveUp)
	ON_COMMAND(ID_DOWNLOADS_MOVE_UP, OnDownloadsMoveUp)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_MOVE_DOWN, OnUpdateDownloadsMoveDown)
	ON_COMMAND(ID_DOWNLOADS_MOVE_DOWN, OnDownloadsMoveDown)
	ON_COMMAND(ID_DOWNLOADS_SETTINGS, OnDownloadsSettings)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FILTER_ALL, OnUpdateDownloadsFilterAll)
	ON_COMMAND(ID_DOWNLOADS_FILTER_ALL, OnDownloadsFilterAll)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FILTER_ACTIVE, OnUpdateDownloadsFilterActive)
	ON_COMMAND(ID_DOWNLOADS_FILTER_ACTIVE, OnDownloadsFilterActive)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FILTER_QUEUED, OnUpdateDownloadsFilterQueued)
	ON_COMMAND(ID_DOWNLOADS_FILTER_QUEUED, OnDownloadsFilterQueued)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FILTER_SOURCES, OnUpdateDownloadsFilterSources)
	ON_COMMAND(ID_DOWNLOADS_FILTER_SOURCES, OnDownloadsFilterSources)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_FILTER_PAUSED, OnUpdateDownloadsFilterPaused)
	ON_COMMAND(ID_DOWNLOADS_FILTER_PAUSED, OnDownloadsFilterPaused)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_LAUNCH_COMPLETE, OnUpdateDownloadsLaunchComplete)
	ON_COMMAND(ID_DOWNLOADS_LAUNCH_COMPLETE, OnDownloadsLaunchComplete)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_SHARE, OnUpdateDownloadsShare)
	ON_COMMAND(ID_DOWNLOADS_SHARE, OnDownloadsShare)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_COPY, OnUpdateDownloadsCopy)
	ON_COMMAND(ID_DOWNLOADS_COPY, OnDownloadsCopy)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_TORRENT_INFO, OnUpdateDownloadsTorrentInfo)
	ON_COMMAND(ID_DOWNLOADS_TORRENT_INFO, OnDownloadsTorrentInfo)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOAD_GROUP_SHOW, OnUpdateDownloadGroupShow)
	ON_COMMAND(ID_DOWNLOAD_GROUP_SHOW, OnDownloadGroupShow)
	ON_COMMAND(ID_DOWNLOADS_HELP, OnDownloadsHelp)
	ON_COMMAND(ID_DOWNLOADS_FILTER_MENU, OnDownloadsFilterMenu)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_CLEAR_INCOMPLETE, OnUpdateDownloadsClearIncomplete)
	ON_COMMAND(ID_DOWNLOADS_CLEAR_INCOMPLETE, OnDownloadsClearIncomplete)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_CLEAR_COMPLETE, OnUpdateDownloadsClearComplete)
	ON_COMMAND(ID_DOWNLOADS_CLEAR_COMPLETE, OnDownloadsClearComplete)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOADS_EDIT, OnUpdateDownloadsEdit)
	ON_COMMAND(ID_DOWNLOADS_EDIT, OnDownloadsEdit)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadsWnd construction

CDownloadsWnd::CDownloadsWnd() : CPanelWnd( TRUE, TRUE )
{
	Create( IDR_DOWNLOADSFRAME );
}

CDownloadsWnd::~CDownloadsWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadsWnd system message handlers

int CDownloadsWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	if ( ! m_wndTabBar.Create( this, WS_CHILD|CBRS_TOP, AFX_IDW_TOOLBAR+1 ) ) return -1;
	m_wndTabBar.SetBarStyle( m_wndTabBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_BOTTOM );
	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );
	m_wndToolBar.SetSyncObject( &Transfers.m_pSection );
	
	m_wndDownloads.Create( this, IDC_DOWNLOADS );
	
	LoadState( NULL, TRUE );
	
	SetTimer( 2, 2000, NULL );
	PostMessage( WM_TIMER, 2 );
	
	SetTimer( 4, 10000, NULL );
	PostMessage( WM_TIMER, 4 );
	
	m_pDragList		= NULL;
	m_pDragImage	= NULL;
	m_hCursMove		= AfxGetApp()->LoadCursor( IDC_MOVE );
	m_hCursCopy		= AfxGetApp()->LoadCursor( IDC_COPY );
	m_tSel			= 0;
	
	return 0;
}

void CDownloadsWnd::OnDestroy() 
{
	CancelDrag();
	KillTimer( 4 );
	KillTimer( 2 );
	SaveState();
	CPanelWnd::OnDestroy();
}

void CDownloadsWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();
	Skin.Translate( _T("CDownloadCtrl"), &m_wndDownloads.m_wndHeader);
	Skin.CreateToolBar( _T("CDownloadsWnd"), &m_wndToolBar );
}

void CDownloadsWnd::Update()
{
	int nCookie = 0;
	
	if ( Settings.General.GUIMode != GUI_BASIC && Settings.Downloads.ShowGroups )
	{
		nCookie = (int)GetTickCount();
		m_wndTabBar.Update( nCookie );
	}
	
	m_wndDownloads.Update( nCookie );
}

BOOL CDownloadsWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if ( m_wndToolBar.m_hWnd )
	{
		if ( m_wndToolBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}
	
	if ( m_wndTabBar.m_hWnd )
	{
		if ( m_wndTabBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}
	
	return CPanelWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CDownloadsWnd::OnSize(UINT nType, int cx, int cy) 
{
	CPanelWnd::OnSize( nType, cx, cy );
	
	CRect rc( 0, 0, cx, cy - 28 );
	
	BOOL bTabs = ( Settings.General.GUIMode != GUI_BASIC ) && Settings.Downloads.ShowGroups;
	
	if ( bTabs )
		rc.top += 24;
	else
		m_wndTabBar.ShowWindow( SW_HIDE );
	
	HDWP hPos = BeginDeferWindowPos( bTabs ? 3 : 2 );
	DeferWindowPos( hPos, m_wndDownloads, NULL,
		rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_SHOWWINDOW );
	if ( bTabs ) DeferWindowPos( hPos, m_wndTabBar, NULL,
		rc.left, 0, rc.Width(), 24, SWP_NOZORDER|SWP_SHOWWINDOW );
	DeferWindowPos( hPos, m_wndToolBar, NULL,
		rc.left, rc.bottom, rc.Width(), 28, SWP_NOZORDER );
	EndDeferWindowPos( hPos );
}

void CDownloadsWnd::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 5 ) m_tSel = 0;
	
	if ( nIDEvent == 4 && Settings.Downloads.AutoClear )
	{
		CSingleLock pLock( &Transfers.m_pSection );
		if ( ! pLock.Lock( 10 ) ) return;
		
		DWORD tNow = GetTickCount();
		
		for ( POSITION pos = Downloads.GetIterator() ; pos ; )
		{
			CDownload* pDownload = Downloads.GetNext( pos );
			
			if ( pDownload->IsCompleted() &&
				 pDownload->IsPreviewVisible() == FALSE &&
				 pDownload->m_pTorrent.IsAvailable() == FALSE &&
				 tNow - pDownload->m_tCompleted > Settings.Downloads.ClearDelay )
			{
				pDownload->Remove();
			}
		}
	}
	
    if ( nIDEvent != 1 && m_pDragList == NULL && IsPartiallyVisible() ) Update();
}

void CDownloadsWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) 
{
	CPanelWnd::OnMDIActivate( bActivate, pActivateWnd, pDeactivateWnd );
	
	if ( bActivate )
	{
		Update();
		m_wndDownloads.SetFocus();
	}
}

void CDownloadsWnd::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	AfxGetMainWnd()->SendMessage( WM_MEASUREITEM, nIDCtl, (LPARAM)lpMeasureItemStruct );
}

void CDownloadsWnd::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	AfxGetMainWnd()->SendMessage( WM_DRAWITEM, nIDCtl, (LPARAM)lpDrawItemStruct );
}

void CDownloadsWnd::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownloadSource* pSource;
	CDownload* pDownload;
	
	CPoint ptLocal( point );
	m_wndDownloads.ScreenToClient( &ptLocal );
	m_tSel = 0;
	
	if ( m_wndDownloads.HitTest( ptLocal, &pDownload, &pSource, NULL, NULL ) )
	{
		if ( pSource != NULL )
		{
			pLock.Unlock();
			TrackPopupMenu( _T("CDownloadsWnd.Source"), point, ID_TRANSFERS_CONNECT );
			return;
		}
		else if ( pDownload->IsSeeding() )
		{
			pLock.Unlock();
			TrackPopupMenu( _T("CDownloadsWnd.Seeding"), point, ID_DOWNLOADS_LAUNCH );
			return;
		}
		else if ( pDownload->IsCompleted() )
		{
			pLock.Unlock();
			TrackPopupMenu( _T("CDownloadsWnd.Completed"), point, ID_DOWNLOADS_LAUNCH );
			return;
		}
	}
	
	pLock.Unlock();
	TrackPopupMenu( _T("CDownloadsWnd.Download"), point,
		Settings.General.GUIMode == GUI_BASIC ? ID_DOWNLOADS_LAUNCH_COPY : ID_DOWNLOADS_LAUNCH );
}

BOOL CDownloadsWnd::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB )
	{
		GetManager()->Open( RUNTIME_CLASS(CUploadsWnd) );
		return TRUE;
	}
	
	return CPanelWnd::PreTranslateMessage( pMsg );
}

BOOL CDownloadsWnd::Select(CDownload* pSelect)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	BOOL bFound = FALSE;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		if ( pDownload == pSelect )
		{
			pDownload->m_bSelected = TRUE;
			bFound = TRUE;
		}
		else
		{
			pDownload->m_bSelected = FALSE;
		}
		
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource != NULL ; pSource = pSource->m_pNext )
		{
			pSource->m_bSelected = FALSE;
		}
	}
	
	Invalidate();
	m_tSel = 0;
	
	return bFound;
}

void CDownloadsWnd::Prepare()
{
	DWORD tNow = GetTickCount();
	if ( tNow - m_tSel < 250 ) return;
	m_tSel = tNow;
	
	m_bSelAny = m_bSelDownload = m_bSelSource = m_bSelTrying = m_bSelPaused = FALSE;
	m_bSelNotPausedOrMoving = m_bSelNoPreview = m_bSelNotCompleteAndNoPreview = FALSE;
	m_bSelCompletedAndNoPreview = m_bSelStartedAndNotMoving = m_bSelCompleted = FALSE;
	m_bSelNotMoving = m_bSelBoostable = m_bSelSHA1orED2K = FALSE;
	m_bSelTorrent = m_bSelIdleSource = m_bSelActiveSource = FALSE;
	m_bSelHttpSource = m_bSelShareState = FALSE;
	m_bSelShareConsistent = TRUE;
	m_bSelSourceAcceptConnections = m_bSelSourceExtended = FALSE;
	
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	BOOL bFirstShare = TRUE;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		if ( pDownload->m_bSelected )
		{
			m_bSelAny = TRUE;
			m_bSelDownload = TRUE;
			if ( pDownload->IsCompleted() )
				m_bSelCompleted = TRUE;
			if ( ! pDownload->IsMoving() )
				m_bSelNotMoving = TRUE;
			if ( ! pDownload->IsBoosted() )
				m_bSelBoostable = TRUE;
			if ( pDownload->m_bSHA1 || pDownload->m_bED2K )
				m_bSelSHA1orED2K = TRUE;
			if ( pDownload->m_pTorrent.IsAvailable() )
				m_bSelTorrent = TRUE;
			if ( pDownload->IsTrying() )
				m_bSelTrying = TRUE;
			if ( pDownload->IsPaused() )
				m_bSelPaused = TRUE;
			else if ( ! pDownload->IsMoving() )
				m_bSelNotPausedOrMoving = TRUE;
			if ( ! pDownload->IsPreviewVisible() )
			{
				m_bSelNoPreview = TRUE;
				if ( pDownload->IsCompleted() )
					m_bSelCompletedAndNoPreview = TRUE;
				else
					m_bSelNotCompleteAndNoPreview = TRUE;
			}
			if ( pDownload->IsStarted() && ! pDownload->IsMoving() )
				m_bSelStartedAndNotMoving = TRUE;
			if ( bFirstShare )
			{
				m_bSelShareState = pDownload->IsShared();
				bFirstShare = FALSE;
			}
			else if ( m_bSelShareState != pDownload->IsShared() )
			{
				m_bSelShareState = FALSE;
				m_bSelShareConsistent = FALSE;
			}
		}
		
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
		{
			if ( pSource->m_bSelected )
			{
				m_bSelAny = TRUE;
				m_bSelSource = TRUE;
				if ( pSource->m_pTransfer == NULL )
					m_bSelIdleSource = TRUE;
				else
					m_bSelActiveSource = TRUE;
				if ( pSource->m_nProtocol == PROTOCOL_HTTP ) m_bSelHttpSource = TRUE;
				if ( ! pSource->m_bPushOnly ) m_bSelSourceAcceptConnections = TRUE;
				m_bSelSourceExtended = pSource->m_bClientExtended;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadsWnd command handlers

void CDownloadsWnd::OnUpdateDownloadsResume(CCmdUI* pCmdUI) 
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelPaused || ( m_bSelDownload && ! m_bSelTrying ) );
	pCmdUI->Enable( m_bSelPaused || ( m_bSelDownload && ! m_bSelTrying ) );
}

void CDownloadsWnd::OnDownloadsResume() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		if ( pDownload->m_bSelected )
		{
			pDownload->Resume();
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsPause(CCmdUI* pCmdUI) 
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelNotPausedOrMoving || ! m_bSelDownload );
	pCmdUI->Enable( m_bSelNotPausedOrMoving );
}

void CDownloadsWnd::OnDownloadsPause() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		if ( pDownload->m_bSelected )
		{
			if ( ! pDownload->IsPaused() && ! pDownload->IsMoving() ) pDownload->Pause();
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsClear(CCmdUI* pCmdUI) 
{
	Prepare();
	pCmdUI->Enable( m_bSelNoPreview );
}

void CDownloadsWnd::OnDownloadsClear() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList<CDownload*> pList;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) pList.AddTail( pDownload );
	}
	
	while ( ! pList.IsEmpty() )
	{
		CDownload* pDownload = pList.RemoveHead();
		
		if ( Downloads.Check( pDownload ) )
		{
			if ( pDownload->IsPreviewVisible() )
			{
				// Can't
			}
			else if ( pDownload->IsCompleted() )
			{
				pDownload->Remove();
			}
			else if ( pDownload->IsStarted() )
			{
				CDeleteFileDlg dlg;
				dlg.m_sName = pDownload->m_sRemoteName;
				BOOL bShared = pDownload->IsShared();
				
				pLock.Unlock();
				if ( dlg.DoModal() != IDOK ) break;
				pLock.Lock();
				
				if ( Downloads.Check( pDownload ) )
				{
					dlg.Create( pDownload, bShared );
					pDownload->Remove();
				}
			}
			else
			{
				pDownload->Remove();
			}
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsClearIncomplete(CCmdUI *pCmdUI)
{
	Prepare();
	
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelNotCompleteAndNoPreview || ! m_bSelDownload );
	pCmdUI->Enable( m_bSelNotCompleteAndNoPreview );
}

void CDownloadsWnd::OnDownloadsClearIncomplete()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList<CDownload*> pList;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) pList.AddTail( pDownload );
	}
	
	while ( ! pList.IsEmpty() )
	{
		CDownload* pDownload = pList.RemoveHead();
		
		if ( Downloads.Check( pDownload ) )
		{
			if ( ! pDownload->IsCompleted() && ! pDownload->IsPreviewVisible() )
			{
				if ( pDownload->IsStarted() )
				{
					CDeleteFileDlg dlg;
					dlg.m_sName = pDownload->m_sRemoteName;
					BOOL bShared = pDownload->IsShared();
					
					pLock.Unlock();
					if ( dlg.DoModal() != IDOK ) break;
					pLock.Lock();
					
					if ( Downloads.Check( pDownload ) ) dlg.Create( pDownload, bShared );
				}
				
				if ( Downloads.Check( pDownload ) ) pDownload->Remove();
			}
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsClearComplete(CCmdUI *pCmdUI)
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelCompletedAndNoPreview );
	pCmdUI->Enable( m_bSelCompletedAndNoPreview );
}

void CDownloadsWnd::OnDownloadsClearComplete()
{
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		if ( pDownload->m_bSelected )
		{
			if ( pDownload->IsCompleted() && ! pDownload->IsPreviewVisible() )
			{
				pDownload->Remove();
			}
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsLaunch(CCmdUI* pCmdUI) 
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelStartedAndNotMoving || ! m_bSelCompleted );
	pCmdUI->Enable( m_bSelCompleted || m_bSelStartedAndNotMoving );
}

void CDownloadsWnd::OnDownloadsLaunch() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList<CDownload*> pList;
	int nCount = 0;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) pList.AddTail( pDownload );
	}
	
	while ( ! pList.IsEmpty() )
	{
		CDownload* pDownload = pList.RemoveHead();
		
		if ( Downloads.Check( pDownload ) )
		{
			CString strName = pDownload->m_sLocalName;
			
			if ( pDownload->IsCompleted() )
			{
				if ( pDownload->m_pTorrent.m_nFiles > 1 )
				{
					CString strPath = DownloadGroups.GetCompletedPath( pDownload );
					strPath += _T("\\");
					strPath += pDownload->m_pTorrent.m_sName;
					
					if ( GetFileAttributes( strPath ) & FILE_ATTRIBUTE_DIRECTORY )
					{
						ShellExecute( NULL, NULL, strPath, NULL, NULL, SW_SHOWNORMAL );
						continue;
					}
				}
				
				if ( pDownload->m_bVerify == TS_FALSE )
				{
					CString strFormat, strMessage;
					
					LoadString( strFormat, IDS_LIBRARY_VERIFY_FAIL );
					strMessage.Format( strFormat, (LPCTSTR)strName );
					
					pLock.Unlock();
					UINT nResponse = AfxMessageBox( strMessage, MB_ICONEXCLAMATION|MB_YESNOCANCEL|MB_DEFBUTTON2 );
					if ( nResponse == IDCANCEL ) break;
					pLock.Lock();
					if ( nResponse == IDNO ) continue;
				}
				
				pLock.Unlock();
				if ( ! CFileExecutor::Execute( strName, FALSE ) ) break;
				pLock.Lock();
				
				if ( ++nCount >= 5 ) break;
			}
			else if ( pDownload->IsStarted() && ! pDownload->IsMoving() )
			{
				if ( pDownload->CanPreview() )
				{
					pDownload->Preview( &pLock );
				}
				else
				{
					pLock.Unlock();
					if ( ! CFileExecutor::Execute( strName, FALSE ) ) break;
					pLock.Lock();
				}
				
				if ( ++nCount >= 3 ) break;
			}
		}
	}
}

void CDownloadsWnd::OnUpdateDownloadsLaunchComplete(CCmdUI* pCmdUI) 
{
	Prepare();
	if ( CCoolBarItem* pItem = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pItem->Show( m_bSelCompleted );
	pCmdUI->Enable( m_bSelCompleted );
}

void CDownloadsWnd::OnDownloadsLaunchComplete() 
{
	OnDownloadsLaunch();
}

void CDownloadsWnd::OnUpdateDownloadsLaunchCopy(CCmdUI* pCmdUI) 
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelStartedAndNotMoving || ! m_bSelCompleted );
	pCmdUI->Enable( m_bSelCompleted || m_bSelStartedAndNotMoving );
}

void CDownloadsWnd::OnDownloadsLaunchCopy() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList<CDownload*> pList;
	int nCount = 0;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) pList.AddTail( pDownload );
	}
	
	while ( ! pList.IsEmpty() )
	{
		CDownload* pDownload = pList.RemoveHead();
		
		if ( Downloads.Check( pDownload ) )
		{
			if ( pDownload->IsStarted() && ! pDownload->IsMoving() )
			{
				CString strType;
				
				int nExtPos = pDownload->m_sLocalName.ReverseFind( '.' );
				if ( nExtPos > 0 ) strType = pDownload->m_sLocalName.Mid( nExtPos + 1 );
				strType = _T("|") + strType + _T("|");
				
				if ( _tcsistr( Settings.Library.SafeExecute, strType ) == NULL ||
					 pDownload->CanPreview() )
				{
					CString strFormat, strPrompt;
					
					LoadString( strFormat, IDS_LIBRARY_CONFIRM_EXECUTE );
					strPrompt.Format( strFormat, (LPCTSTR)pDownload->m_sLocalName );
					
					pLock.Unlock();
					int nResult = AfxMessageBox( strPrompt, MB_ICONQUESTION|MB_YESNOCANCEL|MB_DEFBUTTON2 );
					pLock.Lock();
					
					if ( nResult == IDCANCEL ) break;
					else if ( nResult == IDNO ) continue;
				}
				
				if ( Downloads.Check( pDownload ) ) pDownload->Preview( &pLock );
				
				if ( ++nCount >= 3 ) break;
			}
		}
	}
}

void CDownloadsWnd::OnUpdateDownloadsEnqueue(CCmdUI* pCmdUI) 
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelStartedAndNotMoving || ! m_bSelCompleted );
	pCmdUI->Enable( m_bSelCompleted || m_bSelStartedAndNotMoving );
}

void CDownloadsWnd::OnDownloadsEnqueue() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList<CDownload*> pList;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) pList.AddTail( pDownload );
	}
	
	while ( ! pList.IsEmpty() )
	{
		CDownload* pDownload = pList.RemoveHead();
		
		if ( Downloads.Check( pDownload ) )
		{
			if ( pDownload->IsStarted() )
			{
				CString strPath = pDownload->m_sLocalName;
				pLock.Unlock();
				CFileExecutor::Enqueue( strPath );
				pLock.Lock();
			}
		}
	}
}

void CDownloadsWnd::OnUpdateDownloadsSources(CCmdUI* pCmdUI) 
{
	Prepare();
	pCmdUI->Enable( m_bSelNotMoving && m_bSelTrying );
}

void CDownloadsWnd::OnDownloadsSources() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		if ( pDownload->m_bSelected )
		{
			if ( ! pDownload->IsMoving() ) pDownload->FindMoreSources();
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsUrl(CCmdUI* pCmdUI) 
{
	Prepare();
	pCmdUI->Enable( m_bSelNotMoving );
}

void CDownloadsWnd::OnDownloadsUrl() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList<CDownload*> pList;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) pList.AddTail( pDownload );
	}
	
	while ( ! pList.IsEmpty() )
	{
		CDownload* pDownload = pList.RemoveHead();
		
		if ( Downloads.Check( pDownload ) )
		{
			if ( ! pDownload->IsMoving() )
			{
				CDownloadDlg dlg( NULL, pDownload );
				
				pLock.Unlock();
				if ( dlg.DoModal() != IDOK ) break;
				pLock.Lock();

				if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) continue;

				pDownload->AddSourceURL( dlg.m_pURL->m_sURL, FALSE );
			}
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsBoost(CCmdUI* pCmdUI) 
{
	if ( Settings.Bandwidth.Downloads == 0 )
	{
		pCmdUI->Enable( FALSE );
		return;
	}
	
	Prepare();
	pCmdUI->Enable( m_bSelBoostable );
}

void CDownloadsWnd::OnDownloadsBoost() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		if ( pDownload->m_bSelected )
		{
			if ( pDownload->GetTransferCount() && ! pDownload->IsBoosted() )
			{
				pDownload->Boost();
			}
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsCopy(CCmdUI* pCmdUI) 
{
	Prepare();
	pCmdUI->Enable( m_bSelSHA1orED2K );
}

void CDownloadsWnd::OnDownloadsCopy() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList<CDownload*> pList;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) pList.AddTail( pDownload );
	}
	
	while ( ! pList.IsEmpty() )
	{
		CDownload* pDownload = pList.RemoveHead();
		
		if ( Downloads.Check( pDownload ) && ( pDownload->m_bSHA1 || pDownload->m_bED2K ) )
		{
			CURLCopyDlg dlg;
			dlg.m_sName		= pDownload->m_sRemoteName;
			dlg.m_bSHA1		= pDownload->m_bSHA1;
			dlg.m_pSHA1		= pDownload->m_pSHA1;
			dlg.m_bTiger	= pDownload->m_bTiger;
			dlg.m_pTiger	= pDownload->m_pTiger;
			dlg.m_bED2K		= pDownload->m_bED2K;
			dlg.m_pED2K		= pDownload->m_pED2K;
			dlg.m_bSize		= pDownload->m_nSize != SIZE_UNKNOWN;
			dlg.m_nSize		= pDownload->m_nSize;
			dlg.DoModal();
		}
	}
}

void CDownloadsWnd::OnUpdateDownloadsShare(CCmdUI* pCmdUI) 
{
#ifndef _DEBUG
	if ( Settings.eDonkey.EnableToday )
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( TRUE );
		return;
	}
#endif
	
	Prepare();
	pCmdUI->Enable( m_bSelDownload && m_bSelShareConsistent );
	pCmdUI->SetCheck( m_bSelShareState );
}

void CDownloadsWnd::OnDownloadsShare() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		if ( pDownload->m_bSelected )
		{
			pDownload->Share( ! pDownload->IsShared() );
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsMonitor(CCmdUI* pCmdUI) 
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( ! m_bSelCompleted );
	pCmdUI->Enable( m_bSelDownload && ! m_bSelCompleted );
}

void CDownloadsWnd::OnDownloadsMonitor() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList<CDownload*> pList;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) pList.AddTail( pDownload );
	}
	
	while ( ! pList.IsEmpty() )
	{
		CDownload* pDownload = pList.RemoveHead();
		
		if ( Downloads.Check( pDownload ) && ! pDownload->IsMoving() )
		{
			pDownload->ShowMonitor( &pLock );
		}
	}
}

void CDownloadsWnd::OnUpdateDownloadsTorrentInfo(CCmdUI* pCmdUI) 
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelTorrent );
	pCmdUI->Enable( m_bSelTorrent );
}

void CDownloadsWnd::OnDownloadsTorrentInfo() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		if ( pDownload->m_bSelected && pDownload->m_pTorrent.IsAvailable() )
		{
			//CTorrentTrackerDlg dlg( &pDownload->m_pTorrent );
			CTorrentTrackerDlg dlg( pDownload );
			
			pLock.Unlock();
			dlg.DoModal();
			pLock.Lock();
			
			if ( dlg.m_pInfo.IsAvailable() && Downloads.Check( pDownload ) )
			{
				pDownload->m_pTorrent.m_sTracker = dlg.m_pInfo.m_sTracker;
			}
			
			break;
		}
	}
}

void CDownloadsWnd::OnUpdateDownloadsEdit(CCmdUI *pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelNotMoving );
}

void CDownloadsWnd::OnDownloadsEdit()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		if ( pDownload->m_bSelected && ! pDownload->IsMoving() )
		{
			CDownloadEditDlg dlg( pDownload );
			pLock.Unlock();
			dlg.DoModal();
			break;
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsMoveUp(CCmdUI* pCmdUI) 
{
	Prepare();
	pCmdUI->Enable( m_bSelDownload );
}

void CDownloadsWnd::OnDownloadsMoveUp() 
{
	m_wndDownloads.MoveSelected( -1 );
}

void CDownloadsWnd::OnUpdateDownloadsMoveDown(CCmdUI* pCmdUI) 
{
	Prepare();
	pCmdUI->Enable( m_bSelDownload );
}

void CDownloadsWnd::OnDownloadsMoveDown() 
{
	m_wndDownloads.MoveSelected( 1 );
}

void CDownloadsWnd::OnUpdateTransfersConnect(CCmdUI* pCmdUI) 
{
	Prepare();
	pCmdUI->Enable( m_bSelIdleSource );
}

void CDownloadsWnd::OnTransfersConnect() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource != NULL ; pSource = pSource->m_pNext )
		{
			if ( pSource->m_bSelected && pSource->m_pTransfer == NULL )
			{
				if ( pSource->m_nProtocol != PROTOCOL_ED2K )
				{
					pSource->m_pDownload->Resume();

					if ( pSource->m_bPushOnly )
					{
						pSource->PushRequest();
					}
					else if ( CDownloadTransfer* pTransfer = pSource->CreateTransfer() )
					{
						pTransfer->Initiate();
					}
				}
			}
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateTransfersDisconnect(CCmdUI* pCmdUI) 
{
	Prepare();
	pCmdUI->Enable( m_bSelActiveSource );
}

void CDownloadsWnd::OnTransfersDisconnect() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource != NULL ; )
		{
			CDownloadSource* pNext = pSource->m_pNext;
			
			if ( pSource->m_bSelected && pSource->m_pTransfer != NULL )
			{
				pSource->m_pTransfer->Close( TS_TRUE );
			}
			
			pSource = pNext;
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateTransfersForget(CCmdUI* pCmdUI) 
{
	Prepare();
	pCmdUI->Enable( m_bSelSource );
}

void CDownloadsWnd::OnTransfersForget() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource != NULL ; )
		{
			CDownloadSource* pNext = pSource->m_pNext;
			if ( pSource->m_bSelected ) pSource->Remove( TRUE, TRUE );
			pSource = pNext;
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateTransfersChat(CCmdUI* pCmdUI) 
{
	if ( ! Settings.Community.ChatEnable )
	{
		pCmdUI->Enable( FALSE );
		return;
	}
	
	Prepare();
	pCmdUI->Enable( m_bSelHttpSource || ( m_bSelSourceExtended && m_bSelSourceAcceptConnections) );
}

void CDownloadsWnd::OnTransfersChat() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource != NULL ; pSource = pSource->m_pNext )
		{
			if ( pSource->m_bSelected ) 
			{
				if ( pSource->m_nProtocol == PROTOCOL_HTTP ) //Many HTTP clients support this
					ChatWindows.OpenPrivate( NULL, &pSource->m_pAddress, pSource->m_nPort, pSource->m_bPushOnly );
				else if ( pSource->m_bClientExtended ) //Over ED2K/BT, you can only contact non-push Shareaza clients
					ChatWindows.OpenPrivate( NULL, &pSource->m_pAddress, pSource->m_nPort, FALSE );
			}
		}
	}
}

void CDownloadsWnd::OnUpdateBrowseLaunch(CCmdUI* pCmdUI) 
{
	Prepare();
	pCmdUI->Enable( m_bSelHttpSource || ( m_bSelSourceExtended && m_bSelSourceAcceptConnections) );
}

void CDownloadsWnd::OnBrowseLaunch() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource != NULL ; pSource = pSource->m_pNext )
		{
			if ( pSource->m_bSelected )
			{
				if ( pSource->m_nProtocol == PROTOCOL_HTTP ) //Many HTTP clients support this
					new CBrowseHostWnd( &pSource->m_pAddress, pSource->m_nPort, pSource->m_bPushOnly, &pSource->m_pGUID );
				else if ( pSource->m_bClientExtended ) //Over ED2K/BT, you can only contact non-push Shareaza clients
					new CBrowseHostWnd( &pSource->m_pAddress, pSource->m_nPort, FALSE, NULL );
			}
		}
	}
}

void CDownloadsWnd::OnUpdateDownloadsShowSources(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( Settings.Downloads.ShowSources );
}

void CDownloadsWnd::OnDownloadsShowSources() 
{
	Settings.Downloads.ShowSources = ! Settings.Downloads.ShowSources;
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsFileDelete(CCmdUI* pCmdUI) 
{
	Prepare();
	if ( CCoolBarItem* pcCmdUI = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pcCmdUI->Show( m_bSelCompleted );
	pCmdUI->Enable( m_bSelCompleted );
}

void CDownloadsWnd::OnDownloadsFileDelete() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList<CDownload*> pList;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) pList.AddTail( pDownload );
	}
	
	while ( ! pList.IsEmpty() )
	{
		CDownload* pDownload = pList.RemoveHead();
		
		if ( Downloads.Check( pDownload ) && pDownload->IsCompleted() )
		{
			CDeleteFileDlg dlg;
			dlg.m_sName		= pDownload->m_sRemoteName;
			CString strPath	= pDownload->m_sLocalName;
			
			pLock.Unlock();
			if ( dlg.DoModal() != IDOK ) break;
			
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( strPath, TRUE ) )
			{
				dlg.Apply( pFile );
				Library.Unlock();
			}
			
			pLock.Lock();
			if ( Downloads.Check( pDownload ) ) pDownload->Remove( TRUE );
		}
	}
	
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsRate(CCmdUI* pCmdUI) 
{
	Prepare();
	pCmdUI->Enable( m_bSelCompleted );
}

void CDownloadsWnd::OnDownloadsRate() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CFilePropertiesSheet dlg;
	CStringList pList;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected && pDownload->IsCompleted() )
			pList.AddTail( pDownload->m_sLocalName );
	}
	
	pLock.Unlock();
	
	while ( ! pList.IsEmpty() )
	{
		CString strPath = pList.RemoveHead();
		
		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( strPath, TRUE ) )
		{
			dlg.Add( pFile->m_nIndex );
			Library.Unlock();
		}
	}
	
	dlg.DoModal( 2 );
	Update();
}

void CDownloadsWnd::OnDownloadsClearCompleted() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	Downloads.ClearCompleted();
	Update();
}

void CDownloadsWnd::OnDownloadsClearPaused() 
{
	CString strMessage;
	LoadString( strMessage, IDS_DOWNLOAD_CONFIRM_CLEAR_PAUSED );
	
	if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 ) == IDYES )
	{
		Downloads.ClearPaused();
		Update();
	}
}

void CDownloadsWnd::OnUpdateDownloadsAutoClear(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( Settings.Downloads.AutoClear );
}

void CDownloadsWnd::OnDownloadsAutoClear() 
{
	Settings.Downloads.AutoClear = ! Settings.Downloads.AutoClear;
	if ( Settings.Downloads.AutoClear ) OnTimer( 4 );
}

void CDownloadsWnd::OnDownloadsFilterMenu() 
{
	CMenu* pMenu = Skin.GetMenu( _T("CDownloadsWnd.Filter") );
	m_wndToolBar.ThrowMenu( ID_DOWNLOADS_FILTER_MENU, pMenu, NULL, FALSE, TRUE );
}

void CDownloadsWnd::OnUpdateDownloadsFilterAll(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( ( Settings.Downloads.FilterMask & DLF_ALL ) == DLF_ALL );
}

void CDownloadsWnd::OnDownloadsFilterAll() 
{
	Settings.Downloads.FilterMask |= DLF_ALL;
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsFilterActive(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( ( Settings.Downloads.FilterMask & DLF_ACTIVE ) > 0 );
}

void CDownloadsWnd::OnDownloadsFilterActive() 
{
	Settings.Downloads.FilterMask ^= DLF_ACTIVE;
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsFilterQueued(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( ( Settings.Downloads.FilterMask & DLF_QUEUED ) > 0 );
}

void CDownloadsWnd::OnDownloadsFilterQueued() 
{
	Settings.Downloads.FilterMask ^= DLF_QUEUED;
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsFilterSources(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( ( Settings.Downloads.FilterMask & DLF_SOURCES ) > 0 );
}

void CDownloadsWnd::OnDownloadsFilterSources() 
{
	Settings.Downloads.FilterMask ^= DLF_SOURCES;
	Update();
}

void CDownloadsWnd::OnUpdateDownloadsFilterPaused(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( ( Settings.Downloads.FilterMask & DLF_PAUSED ) > 0 );
}

void CDownloadsWnd::OnDownloadsFilterPaused() 
{
	Settings.Downloads.FilterMask ^= DLF_PAUSED;
	Update();
}

void CDownloadsWnd::OnDownloadsSettings() 
{
	CSettingsManagerDlg::Run( _T("CDownloadsSettingsPage") );
}

void CDownloadsWnd::OnUpdateDownloadGroupShow(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( Settings.General.GUIMode != GUI_BASIC );
	pCmdUI->SetCheck( Settings.General.GUIMode != GUI_BASIC && Settings.Downloads.ShowGroups );
}

void CDownloadsWnd::OnDownloadGroupShow() 
{
	Settings.Downloads.ShowGroups = ! Settings.Downloads.ShowGroups;
	
	CRect rc;
	GetClientRect( &rc );
	OnSize( SIZE_RESTORED, rc.right, rc.bottom );
	
	Update();
}

void CDownloadsWnd::OnDownloadsHelp() 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownload* pDownload = NULL;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) break;
		pDownload = NULL;
	}
	
	if ( pDownload == NULL )
	{
		CHelpDlg::Show( _T("DownloadHelp.Select") );
	}
	else if ( pDownload->IsMoving() )
	{
		if ( pDownload->IsCompleted() )
			CHelpDlg::Show( _T("DownloadHelp.Completed") );
		else
			CHelpDlg::Show( _T("DownloadHelp.Moving") );
	}
	else if ( pDownload->IsPaused() )
	{
		if ( pDownload->m_bDiskFull )
			CHelpDlg::Show( _T("DownloadHelp.DiskFull") );
		else
			CHelpDlg::Show( _T("DownloadHelp.Paused") );
	}
	else if ( pDownload->GetProgress() == 1.0f && pDownload->IsStarted() )
	{
		CHelpDlg::Show( _T("DownloadHelp.Verifying") );
	}
	else if ( pDownload->IsDownloading() )
	{
		CHelpDlg::Show( _T("DownloadHelp.Downloading") );
	}
	else if ( ! pDownload->IsTrying() )
	{
		CHelpDlg::Show( _T("DownloadHelp.Queued") );
	}
	else if ( pDownload->GetSourceCount() > 0 )
	{
		CHelpDlg::Show( _T("DownloadHelp.Pending") );
	}
	else if ( pDownload->m_nSize == SIZE_UNKNOWN )
	{
		CHelpDlg::Show( _T("DownloadHelp.Searching") );
	}
	else if ( pDownload->m_bBTH && pDownload->IsTasking() )
	{
		CHelpDlg::Show( _T("DownloadHelp.Creating") );
	}
	else if ( pDownload->m_bTorrentTrackerError )
	{
		CHelpDlg::Show( _T("DownloadHelp.Tracker") );
	}
	else
	{
		CHelpDlg::Show( _T("DownloadHelp.Searching") );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadsWnd drag and drop

void CDownloadsWnd::DragDownloads(CPtrList* pList, CImageList* pImage, const CPoint& ptScreen)
{
	ASSERT( m_pDragList == NULL );
	
	m_pDragList		= pList;
	m_pDragImage	= pImage;
	
	CRect rcClient;
	GetClientRect( &rcClient );
	ClientToScreen( &rcClient );
	
	ClipCursor( &rcClient );
	SetCapture();
	SetFocus();
	UpdateWindow();
	
	OnSetCursor( NULL, 0, 0 );
	
	CPoint ptStart( ptScreen );
	ScreenToClient( &ptStart );
	
	CRect rcList;
	m_wndDownloads.GetWindowRect( &rcList );
	ScreenToClient( &rcList );
	m_pDragOffs = rcList.TopLeft();
	m_pDragOffs.y -= 4;
	
	m_pDragImage->DragEnter( this, ptStart + m_pDragOffs );
}

void CDownloadsWnd::CancelDrag()
{
	if ( m_pDragList == NULL ) return;
	
	ClipCursor( NULL );
	ReleaseCapture();
	
	m_pDragImage->DragLeave( this );
	m_pDragImage->EndDrag();
	delete m_pDragImage;
	m_pDragImage = NULL;
	
	delete m_pDragList;
	m_pDragList = NULL;
	
	CPoint point( 0, 0 );
	m_wndTabBar.DropObjects( NULL, point );
	m_wndDownloads.DropObjects( NULL, point );
}

void CDownloadsWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	if ( m_pDragList != NULL )
	{
		m_pDragImage->DragMove( point + m_pDragOffs );
		ClientToScreen( &point );
		m_wndTabBar.DropShowTarget( m_pDragList, point );
		m_wndDownloads.DropShowTarget( m_pDragList, point );
	}
	
	CPanelWnd::OnMouseMove( nFlags, point );
}

void CDownloadsWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ( m_pDragList != NULL )
	{
		ClipCursor( NULL );
		ReleaseCapture();
		
		m_pDragImage->DragLeave( this );
		m_pDragImage->EndDrag();
		delete m_pDragImage;
		m_pDragImage = NULL;
		
		ClientToScreen( &point );
		m_wndTabBar.DropObjects( m_pDragList, point );
		m_wndDownloads.DropObjects( m_pDragList, point );
		
		delete m_pDragList;
		m_pDragList = NULL;
		
		Update();
	}
	
	CPanelWnd::OnLButtonUp( nFlags, point );
}

void CDownloadsWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if ( m_pDragList != NULL )
	{
		CancelDrag();
		return;
	}
	
	CPanelWnd::OnRButtonDown( nFlags, point );
}

void CDownloadsWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ( nChar == VK_ESCAPE ) CancelDrag();
	if ( m_pDragList != NULL ) OnSetCursor( NULL, 0, 0 );
	CPanelWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CDownloadsWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ( m_pDragList ) OnSetCursor( NULL, 0, 0 );
	CPanelWnd::OnKeyUp( nChar, nRepCnt, nFlags );
}

BOOL CDownloadsWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if ( m_pDragList != NULL )
	{
		if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
			SetCursor( m_hCursCopy );
		else
			SetCursor( m_hCursMove );
		
		return TRUE;
	}
	
	return CPanelWnd::OnSetCursor( pWnd, nHitTest, message );
}
