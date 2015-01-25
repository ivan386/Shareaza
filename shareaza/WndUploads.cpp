//
// WndUploads.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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
#include "Transfers.h"
#include "Uploads.h"
#include "UploadQueues.h"
#include "UploadQueue.h"
#include "UploadFiles.h"
#include "UploadFile.h"
#include "UploadTransfer.h"
#include "UploadTransferED2K.h"
#include "EDClient.h"
#include "Library.h"
#include "FileExecutor.h"
#include "Security.h"

#include "Skin.h"
#include "ChatWindows.h"
#include "WindowManager.h"
#include "WndDownloads.h"
#include "WndLibrary.h"
#include "WndMain.h"
#include "WndUploads.h"
#include "WndBrowseHost.h"
#include "DlgSettingsManager.h"
#include "DlgQueueProperties.h"

#include "DlgHelp.h"
#include "LibraryDictionary.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CUploadsWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CUploadsWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_WM_MDIACTIVATE()
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_DISCONNECT, OnUpdateUploadsDisconnect)
	ON_COMMAND(ID_UPLOADS_DISCONNECT, OnUploadsDisconnect)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_LAUNCH, OnUpdateUploadsLaunch)
	ON_COMMAND(ID_UPLOADS_LAUNCH, OnUploadsLaunch)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_CLEAR, OnUpdateUploadsClear)
	ON_COMMAND(ID_UPLOADS_CLEAR, OnUploadsClear)
	ON_COMMAND(ID_UPLOADS_CLEAR_COMPLETED, OnUploadsClearCompleted)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_CHAT, OnUpdateUploadsChat)
	ON_COMMAND(ID_UPLOADS_CHAT, OnUploadsChat)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_AUTO_CLEAR, OnUpdateUploadsAutoClear)
	ON_COMMAND(ID_UPLOADS_AUTO_CLEAR, OnUploadsAutoClear)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_BAN, OnUpdateSecurityBan)
	ON_COMMAND(ID_SECURITY_BAN, OnSecurityBan)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_LAUNCH, OnUpdateBrowseLaunch)
	ON_COMMAND(ID_BROWSE_LAUNCH, OnBrowseLaunch)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_START, OnUpdateUploadsStart)
	ON_COMMAND(ID_UPLOADS_START, OnUploadsStart)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_EDIT_QUEUE, OnUpdateEditQueue)
	ON_COMMAND(ID_UPLOADS_EDIT_QUEUE, OnEditQueue)
	ON_COMMAND(ID_UPLOADS_HELP, OnUploadsHelp)
	ON_COMMAND(ID_UPLOADS_SETTINGS, OnUploadsSettings)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_FILTER_ALL, OnUpdateUploadsFilterAll)
	ON_COMMAND(ID_UPLOADS_FILTER_ALL, OnUploadsFilterAll)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_FILTER_ACTIVE, OnUpdateUploadsFilterActive)
	ON_COMMAND(ID_UPLOADS_FILTER_ACTIVE, OnUploadsFilterActive)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_FILTER_QUEUED, OnUpdateUploadsFilterQueued)
	ON_COMMAND(ID_UPLOADS_FILTER_QUEUED, OnUploadsFilterQueued)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_FILTER_HISTORY, OnUpdateUploadsFilterHistory)
	ON_COMMAND(ID_UPLOADS_FILTER_HISTORY, OnUploadsFilterHistory)
	ON_COMMAND(ID_UPLOADS_FILTER_MENU, OnUploadsFilterMenu)
	ON_UPDATE_COMMAND_UI(ID_UPLOADS_FILTER_TORRENT, OnUpdateUploadsFilterTorrent)
	ON_COMMAND(ID_UPLOADS_FILTER_TORRENT, OnUploadsFilterTorrent)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CUploadsWnd construction

CUploadsWnd::CUploadsWnd() : CPanelWnd( Settings.General.GUIMode == GUI_TABBED, TRUE )
{
	Create( IDR_UPLOADSFRAME );
}

CUploadsWnd::~CUploadsWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CUploadsWnd message handlers

int CUploadsWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndUploads.Create( this, IDC_UPLOADS );

	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );
	m_wndToolBar.SetSyncObject( &Transfers.m_pSection );

	LoadState( NULL, TRUE );

	SetTimer( 2, 2000, NULL );
	PostMessage( WM_TIMER, 2 );

	SetTimer( 4, 5000, NULL );
	PostMessage( WM_TIMER, 4 );

	m_tSel			= 0;
	m_tLastUpdate	= 0;

	return 0;
}

void CUploadsWnd::OnDestroy()
{
	KillTimer( 4 );
	SaveState();
	CPanelWnd::OnDestroy();
}

BOOL CUploadsWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndToolBar.m_hWnd )
	{
		if ( m_wndToolBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}

	return CPanelWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

void CUploadsWnd::OnSize(UINT nType, int cx, int cy)
{
	CPanelWnd::OnSize( nType, cx, cy );
	SizeListAndBar( &m_wndUploads, &m_wndToolBar );
}

void CUploadsWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();
	Skin.Translate( _T("CUploadCtrl"), &m_wndUploads.m_wndHeader);
	Skin.CreateToolBar( _T("CUploadsWnd"), &m_wndToolBar );
	m_wndUploads.OnSkinChange();
}

void CUploadsWnd::OnTimer(UINT_PTR nIDEvent)
{
	DWORD tNow = GetTickCount();

	// Reset Selection Timer event (posted by ctrluploads)
	if ( nIDEvent == 5 ) m_tSel	= 0;

	// Clear event (5 second timer)
	if ( nIDEvent == 4 )
	{
		CSingleLock pLock( &Transfers.m_pSection );
		if ( ! pLock.Lock( 10 ) ) return;

		DWORD nCount = 0;
		for ( POSITION pos = Uploads.GetIterator() ; pos ; )
		{
			CUploadTransfer* pUpload = Uploads.GetNext( pos );
			if ( pUpload->m_nState == upsNull )
			{
				nCount++;
				if ( tNow > pUpload->m_tConnected + Settings.Uploads.ClearDelay )
				{
					if ( Settings.Uploads.AutoClear || pUpload->m_nUploaded == 0 || nCount > 25 )
					{
						nCount--;
						pUpload->Remove( FALSE );
					}
				}
			}
		}
	}

	// Update event (2 second timer)
	if ( nIDEvent == 2 )
	{
		// If the window is visible or hasn't been updated in 10 seconds
		if ( ( IsWindowVisible() && IsActive( FALSE ) ) || ( tNow > m_tLastUpdate + 10*1000 ) )
		{
			m_wndUploads.Update();
			m_tLastUpdate = tNow;
		}
	}
}

void CUploadsWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	BOOL bHit = FALSE;
	CUploadQueue* pQueue = NULL;
	CUploadFile* pUpload = NULL;

	CPoint ptLocal( point );
	m_wndUploads.ScreenToClient( &ptLocal );
	m_tSel = 0;

	{
		CSingleLock pTransfersLock( &Transfers.m_pSection, FALSE );
		if ( pTransfersLock.Lock( 250 ) )
		{
			CSingleLock pUploadQueuesLock( &UploadQueues.m_pSection, FALSE );
			if ( pUploadQueuesLock.Lock( 250 ) )
			{
				bHit = m_wndUploads.HitTest( ptLocal, &pQueue, &pUpload, NULL, NULL );
			}
		}
	}

	if ( bHit )
	{
		if ( pUpload != NULL )
		{
			Skin.TrackPopupMenu( _T("CUploadsWnd.Upload"), point, ID_UPLOADS_LAUNCH );
			return;
		}
	}

	if ( ( pQueue == NULL ) || ( Settings.General.GUIMode == GUI_BASIC ) ||		//If we're not pointing at a queue, or in basic mode
		( pQueue == UploadQueues.m_pHistoryQueue ) || ( pQueue == UploadQueues.m_pTorrentQueue ) )	//Or pointing at a pre-defined queue
		Skin.TrackPopupMenu( _T("CUploadsWnd.Nothing"), point, ID_UPLOADS_HELP );
	else
		Skin.TrackPopupMenu( _T("CUploadsWnd.Queue"), point, ID_UPLOADS_EDIT_QUEUE );
}

void CUploadsWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CPanelWnd::OnMDIActivate( bActivate, pActivateWnd, pDeactivateWnd );
	if ( bActivate ) m_wndUploads.SetFocus();
}

BOOL CUploadsWnd::IsSelected(const CUploadFile* pFile) const
{
	if ( ! pFile->m_bSelected ) return FALSE;

	if ( const CUploadTransfer* pTransfer = pFile->GetActive() )
	{
		if ( pTransfer->m_nProtocol == PROTOCOL_BT )
		{
			if ( 0 == ( Settings.Uploads.FilterMask & ULF_TORRENT ) ) return FALSE;
		}
		else if ( pTransfer->m_pQueue != NULL )
		{
			if ( pTransfer->m_pQueue->m_bExpanded == FALSE ) return FALSE;

			if ( pTransfer->m_pQueue->IsActive( pTransfer ) )
			{
				if ( 0 == ( Settings.Uploads.FilterMask & ULF_ACTIVE ) ) return FALSE;
			}
			else
			{
				if ( 0 == ( Settings.Uploads.FilterMask & ULF_QUEUED ) ) return FALSE;
			}
		}
		else
		{
			if ( 0 == ( Settings.Uploads.FilterMask & ULF_HISTORY ) ) return FALSE;
		}
	}
	else
	{
		if ( 0 == ( Settings.Uploads.FilterMask & ULF_HISTORY ) ) return FALSE;
	}

	return TRUE;
}

void CUploadsWnd::Prepare()
{
	if ( GetTickCount() - m_tSel < 250 )
		return;

	CSingleLock pLock( &Transfers.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return;

	m_tSel = GetTickCount();
	m_bSelFile = m_bSelUpload = FALSE;
	m_bSelActive = m_bSelQueued = FALSE;
	m_bSelChat = m_bSelBrowse = FALSE;
	m_bSelSourceAcceptConnections = FALSE;

	for ( POSITION posFile = UploadFiles.GetIterator() ; posFile ; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( posFile );

		if ( pFile->m_bSelected && IsSelected( pFile ) )
		{
			m_bSelFile = TRUE;

			if ( CUploadTransfer* pTransfer = pFile->GetActive() )
			{
				m_bSelUpload = TRUE;

				if ( pTransfer->m_bClientExtended || pTransfer->m_nProtocol == PROTOCOL_ED2K )
				{
					m_bSelChat = TRUE;
					m_bSelBrowse = TRUE;
				}
				else if ( pTransfer->m_nProtocol == PROTOCOL_DC )
				{
					// TODO: Implement DC++ private chat
					// m_bSelChat = TRUE;
					m_bSelBrowse = TRUE;
				}

				if ( pTransfer->m_pQueue != NULL )
				{
					if ( pTransfer->m_pQueue->IsActive( pTransfer ) )
						m_bSelActive = TRUE;
					else
						m_bSelQueued = TRUE;
				}
				else if ( pTransfer->m_nState != upsNull )
				{
					m_bSelActive = TRUE;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CUploadsWnd command handlers

void CUploadsWnd::OnUpdateUploadsDisconnect(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelActive );
}

void CUploadsWnd::OnUploadsDisconnect()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList<CUploadFile*> pList;

	for ( POSITION pos = UploadFiles.GetIterator() ; pos ; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );
		if ( IsSelected( pFile ) ) pList.AddTail( pFile );
	}

	while ( ! pList.IsEmpty() )
	{
		CUploadFile* pFile = pList.RemoveHead();

		if ( UploadFiles.Check( pFile ) && pFile->GetActive() != NULL )
		{
			CUploadTransfer* pUpload = pFile->GetActive();

			if ( pUpload->m_nProtocol == PROTOCOL_ED2K && pUpload->m_nState != upsNull )
			{
				CString strFormat, strMessage;
				LoadString( strFormat, IDS_UPLOAD_CANCEL_ED2K );
				strMessage.Format( strFormat, (LPCTSTR)pUpload->m_sName );
				pLock.Unlock();
				INT_PTR nResp = AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNOCANCEL|MB_DEFBUTTON2 );
				pLock.Lock();
				if ( nResp == IDCANCEL )
					break;
				if ( nResp != IDYES || ! Uploads.Check( pUpload ) )
					continue;
			}

			pUpload->Close( TRUE );
		}
	}
}

void CUploadsWnd::OnUpdateUploadsStart(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelQueued );
}

void CUploadsWnd::OnUploadsStart()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	for ( POSITION pos = UploadFiles.GetIterator() ; pos ; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );

		if ( IsSelected( pFile ) && pFile->GetActive() != NULL )
		{
			pFile->GetActive()->Promote();
		}
	}
}

void CUploadsWnd::OnUpdateUploadsClear(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelFile );
}

void CUploadsWnd::OnUploadsClear()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList<CUploadFile*> pList;

	for ( POSITION pos = UploadFiles.GetIterator() ; pos ; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );
		if ( IsSelected( pFile ) ) pList.AddTail( pFile );
	}

	while ( ! pList.IsEmpty() )
	{
		CUploadFile* pFile = pList.RemoveHead();

		if ( UploadFiles.Check( pFile ) )
		{
			CUploadTransfer* pUpload = pFile->GetActive();

			if ( pUpload != NULL && pUpload->m_nProtocol == PROTOCOL_ED2K && pUpload->m_nState != upsNull )
			{
				CString strFormat, strMessage;
				LoadString( strFormat, IDS_UPLOAD_CANCEL_ED2K );
				strMessage.Format( strFormat, (LPCTSTR)pUpload->m_sName );
				pLock.Unlock();
				INT_PTR nResp = AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNOCANCEL|MB_DEFBUTTON2 );
				pLock.Lock();
				if ( nResp == IDCANCEL )
					break;
				if ( nResp != IDYES || ! UploadFiles.Check( pFile ) )
					continue;
			}

			pFile->Remove();
		}
	}
}

void CUploadsWnd::OnUpdateUploadsLaunch(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelFile );
}

void CUploadsWnd::OnUploadsLaunch()
{
	CSingleLock pTransfersLock( &Transfers.m_pSection, TRUE );

	for ( POSITION pos = UploadFiles.GetIterator() ; pos ; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );

		if ( IsSelected( pFile ) )
		{
			CShareazaFile oFile = *pFile;
			pTransfersLock.Unlock();

			CSingleLock pLibraryLock( &Library.m_pSection, TRUE );

			const CLibraryFile* pLibFile = LibraryMaps.LookupFileByHash( &oFile );
			if ( ! pLibFile )
				pLibFile = LibraryMaps.LookupFileByPath( oFile.m_sPath );
			if ( pLibFile )
			{
				if ( CLibraryWnd* pLibrary = CLibraryWnd::GetLibraryWindow() )
				{
					pLibrary->Display( pLibFile );
				}
			}
			break;
		}
	}
}

void CUploadsWnd::OnUpdateUploadsChat(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelChat && Settings.Community.ChatEnable );
}

void CUploadsWnd::OnUploadsChat()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList< CUploadFile* > pList;

	for ( POSITION pos = UploadFiles.GetIterator() ; pos ; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );
		if ( IsSelected( pFile ) ) pList.AddTail( pFile );
	}

	while ( ! pList.IsEmpty() )
	{
		CUploadFile* pFile = pList.RemoveHead();

		if ( UploadFiles.Check( pFile ) )
		{
			if ( CUploadTransfer* pTransfer = pFile->GetActive() )
			{
				PROTOCOLID nProtocol = pTransfer->m_nProtocol;
				SOCKADDR_IN pAddress = pTransfer->m_pHost;
//				CString sNick = pTransfer->m_sRemoteNick;
				BOOL bClientExtended = pTransfer->m_bClientExtended;
				pLock.Unlock();

				if ( nProtocol == PROTOCOL_HTTP )		// HTTP chat. (G2, G1)
					ChatWindows.OpenPrivate( Hashes::Guid(), &pAddress, FALSE, PROTOCOL_HTTP );
				else if ( bClientExtended )				// Client accepts G2 chat
					ChatWindows.OpenPrivate( Hashes::Guid(), &pAddress, FALSE, PROTOCOL_G2 );
				else if ( nProtocol == PROTOCOL_ED2K )	// ED2K chat.
					ChatWindows.OpenPrivate( Hashes::Guid(), &pAddress, FALSE, PROTOCOL_ED2K );

				pLock.Lock();
			}
		}
	}
}

void CUploadsWnd::OnUpdateSecurityBan(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelUpload );
}

void CUploadsWnd::OnSecurityBan()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList<CUploadFile*> pList;

	for ( POSITION pos = UploadFiles.GetIterator() ; pos ; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );
		if ( IsSelected( pFile ) ) pList.AddTail( pFile );
	}

	while ( ! pList.IsEmpty() )
	{
		CUploadFile* pFile = pList.RemoveHead();

		if ( UploadFiles.Check( pFile ) && pFile->GetActive() != NULL )
		{
			CUploadTransfer* pUpload = pFile->GetActive();

			IN_ADDR pAddress = pUpload->m_pHost.sin_addr;
			pUpload->Remove( FALSE );
			pLock.Unlock();
			Security.Ban( &pAddress, banSession );
			pLock.Lock();
		}
	}
}

void CUploadsWnd::OnUpdateBrowseLaunch(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( m_bSelBrowse );
}

void CUploadsWnd::OnBrowseLaunch()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList< CUploadFile* > pList;

	for ( POSITION pos = UploadFiles.GetIterator() ; pos ; )
	{
		CUploadFile* pFile = UploadFiles.GetNext( pos );
		if ( IsSelected( pFile ) ) pList.AddTail( pFile );
	}

	while ( ! pList.IsEmpty() )
	{
		CUploadFile* pFile = pList.RemoveHead();

		if ( UploadFiles.Check( pFile ) )
		{
			if ( CUploadTransfer* pTransfer = pFile->GetActive() )
			{
				PROTOCOLID nProtocol = pTransfer->m_nProtocol;
				SOCKADDR_IN pAddress = ( nProtocol == PROTOCOL_DC ) ? pTransfer->m_pServer : pTransfer->m_pHost;
				BOOL bPush = ( nProtocol == PROTOCOL_DC );
				CString sNick = pTransfer->m_sRemoteNick;
				pLock.Unlock();

				new CBrowseHostWnd( nProtocol, &pAddress, bPush, Hashes::Guid(), sNick );

				pLock.Lock();
			}
		}
	}
}

void CUploadsWnd::OnUploadsClearCompleted()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	for ( POSITION pos = Uploads.GetIterator() ; pos ; )
	{
		CUploadTransfer* pUpload = Uploads.GetNext( pos );
		if ( pUpload->m_nState == upsNull ) pUpload->Remove( FALSE );
	}

	m_wndUploads.Update();
}

void CUploadsWnd::OnUpdateUploadsAutoClear(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.Uploads.AutoClear );
}

void CUploadsWnd::OnUploadsAutoClear()
{
	Settings.Uploads.AutoClear = ! Settings.Uploads.AutoClear;
	if ( Settings.Uploads.AutoClear ) OnTimer( 4 );
}

void CUploadsWnd::OnUpdateEditQueue(CCmdUI* pCmdUI)
{
	Prepare();
	pCmdUI->Enable( ! m_bSelFile );
}

void CUploadsWnd::OnEditQueue()
{
	CSingleLock pLock( &UploadQueues.m_pSection, TRUE );

	for ( POSITION pos = UploadQueues.GetIterator() ; pos ; )
	{
		CUploadQueue* pQueue = UploadQueues.GetNext( pos );

		if ( pQueue->m_bSelected )
		{
			pLock.Unlock();
			CSettingsManagerDlg::Run( _T("CUploadsSettingsPage") );
			return;
		}
	}

}

void CUploadsWnd::OnUploadsHelp()
{
	CHelpDlg::Show( _T("UploadHelp") );
}

void CUploadsWnd::OnUploadsSettings()
{
	CSettingsManagerDlg::Run( _T("CUploadsSettingsPage") );
}

BOOL CUploadsWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DELETE )
	{
		OnUploadsClear();
		return TRUE;
	}
	else if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB )
	{
		GetManager()->Open( RUNTIME_CLASS(CDownloadsWnd) );
		return TRUE;
	}

	return CPanelWnd::PreTranslateMessage( pMsg );
}

void CUploadsWnd::OnUploadsFilterMenu()
{
	CMenu* pMenu = Skin.GetMenu( _T("CUploadsWnd.Filter") );
	m_wndToolBar.ThrowMenu( ID_UPLOADS_FILTER_MENU, pMenu, NULL, FALSE, TRUE );
}

void CUploadsWnd::OnUpdateUploadsFilterAll(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Uploads.FilterMask & ULF_ALL ) == ULF_ALL );
}

void CUploadsWnd::OnUploadsFilterAll()
{
	Settings.Uploads.FilterMask |= ULF_ALL;
	m_wndUploads.Update();
}

void CUploadsWnd::OnUpdateUploadsFilterActive(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Uploads.FilterMask & ULF_ACTIVE ) > 0 );
}

void CUploadsWnd::OnUploadsFilterActive()
{
	Settings.Uploads.FilterMask ^= ULF_ACTIVE;
	m_wndUploads.Update();
}

void CUploadsWnd::OnUpdateUploadsFilterQueued(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Uploads.FilterMask & ULF_QUEUED ) > 0 );
}

void CUploadsWnd::OnUploadsFilterQueued()
{
	Settings.Uploads.FilterMask ^= ULF_QUEUED;
	m_wndUploads.Update();
}

void CUploadsWnd::OnUpdateUploadsFilterHistory(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Uploads.FilterMask & ULF_HISTORY ) > 0 );
}

void CUploadsWnd::OnUploadsFilterHistory()
{
	Settings.Uploads.FilterMask ^= ULF_HISTORY;
	m_wndUploads.Update();
}

void CUploadsWnd::OnUpdateUploadsFilterTorrent(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( ( Settings.Uploads.FilterMask & ULF_TORRENT ) > 0 );
}

void CUploadsWnd::OnUploadsFilterTorrent()
{
	Settings.Uploads.FilterMask ^= ULF_TORRENT;
	m_wndUploads.Update();
}
