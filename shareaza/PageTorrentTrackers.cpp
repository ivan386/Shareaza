//
// PageTorrentTrackers.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

#include "BENode.h"
#include "DlgDownloadSheet.h"
#include "PageTorrentTrackers.h"
#include "CoolInterface.h"
#include "Network.h"
#include "Skin.h"
#include "Transfers.h"
#include "Downloads.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTorrentTrackersPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CTorrentTrackersPage, CPropertyPageAdv)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_TORRENT_REFRESH, OnTorrentRefresh)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTorrentTrackersPage property page

CTorrentTrackersPage::CTorrentTrackersPage() : 
	CPropertyPageAdv( CTorrentTrackersPage::IDD ),
	m_pDownload( NULL )
{
}

CTorrentTrackersPage::~CTorrentTrackersPage()
{
}

void CTorrentTrackersPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TORRENT_TRACKER, m_sTracker);
	DDX_Control(pDX, IDC_TORRENT_COMPLETED, m_wndComplete);
	DDX_Control(pDX, IDC_TORRENT_INCOMPLETE, m_wndIncomplete);
	DDX_Control(pDX, IDC_TORRENT_REFRESH, m_wndRefresh);
	DDX_Control(pDX, IDC_TORRENT_TRACKERS, m_wndTrackers);
	DDX_Control(pDX, IDC_TORRENT_TRACKERMODE, m_wndTrackerMode);
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentTrackersPage message handlers

BOOL CTorrentTrackersPage::OnInitDialog()
{
	CPropertyPageAdv::OnInitDialog();
	
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	m_pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
	CBTInfo* pInfo = &m_pDownload->m_pTorrent;

	m_sTracker		= pInfo->m_sTracker;

	m_wndTrackerMode.SetItemData( 0, tNull );
	m_wndTrackerMode.SetItemData( 1, tCustom );
	m_wndTrackerMode.SetItemData( 2, tSingle );
	m_wndTrackerMode.SetItemData( 3, tMultiFinding );
	m_wndTrackerMode.SetItemData( 4, tMultiFound );

	m_wndTrackerMode.SetCurSel( pInfo->m_nTrackerMode );

	CRect rc;
	m_wndTrackers.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );
	CoolInterface.SetImageListTo( m_wndTrackers, LVSIL_SMALL );
	m_wndTrackers.InsertColumn( 0, _T("Tracker"), LVCFMT_LEFT, rc.right - 80, -1 );
	m_wndTrackers.InsertColumn( 1, _T("Status"), LVCFMT_RIGHT, 80, 0 );
	m_wndTrackers.InsertColumn( 2, _T("Type"), LVCFMT_LEFT, 0, 0 );
	Skin.Translate( _T("CTorrentTrackerList"), m_wndTrackers.GetHeaderCtrl() );

	int nTracker = 0;
	for ( nTracker = 0 ; nTracker < pInfo->m_pTrackerList.GetCount() ; nTracker++ )
	{
		CBTInfo::CBTTracker* pTrack = pInfo->m_pTrackerList.GetAt(nTracker);
		
		LV_ITEM pItem = {};
		pItem.mask		= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		pItem.iItem		= m_wndTrackers.GetItemCount();
		pItem.lParam	= (LPARAM)nTracker;

		if ( pInfo->m_nTrackerIndex == nTracker )
			pItem.iImage = CoolInterface.ImageForID( ID_MEDIA_SELECT );
		else
			pItem.iImage = CoolInterface.ImageForID( ID_DOWNLOADS_COPY );

		pItem.pszText	= (LPTSTR)(LPCTSTR)pTrack->m_sAddress;
		pItem.iItem		= m_wndTrackers.InsertItem( &pItem );

		// Display status
		CString sStatus;
		if ( ( pTrack->m_tNextTry == 0 ) && ( pTrack->m_tLastSuccess == 0 ) )
			LoadString( sStatus, IDS_STATUS_UNKNOWN );
		else if ( pTrack->m_tNextTry > pTrack->m_tLastSuccess )
			LoadString( sStatus, IDS_STATUS_TRACKERDOWN );
		else
			LoadString( sStatus, IDS_STATUS_ACTIVE );

		m_wndTrackers.SetItemText( pItem.iItem, 1, sStatus );

		// Display type
		CString sType;
		sType.Format( _T("Tier %i"), pTrack->m_nTier );
		m_wndTrackers.SetItemText( pItem.iItem, 2, sType );
	}

	if ( ! pInfo->IsMultiTracker() &&
		pInfo->m_pAnnounceTracker )
	{
		nTracker ++;

		LV_ITEM pItem = {};
		pItem.mask		= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		pItem.iItem		= m_wndTrackers.GetItemCount();
		pItem.lParam	= (LPARAM)nTracker;
		pItem.iImage = CoolInterface.ImageForID( ID_TOOLS_LANGUAGE );
		pItem.pszText	= (LPTSTR)(LPCTSTR)pInfo->m_pAnnounceTracker->m_sAddress;
		pItem.iItem		= m_wndTrackers.InsertItem( &pItem );
		
		// Display status
		CString sStatus;
		if ( ( pInfo->m_pAnnounceTracker->m_tNextTry == 0 ) &&
			( pInfo->m_pAnnounceTracker->m_tLastSuccess == 0 ) )
			LoadString( sStatus, IDS_STATUS_UNKNOWN );
		else if ( pInfo->m_pAnnounceTracker->m_tNextTry >
			pInfo->m_pAnnounceTracker->m_tLastSuccess )
			LoadString( sStatus, IDS_STATUS_TRACKERDOWN );
		else
			LoadString( sStatus, IDS_STATUS_ACTIVE );

		m_wndTrackers.SetItemText( pItem.iItem, 1, sStatus );

		// Display type
		CString sType;
		sType.Format( _T("Announce") );
		m_wndTrackers.SetItemText( pItem.iItem, 2, sType );
	}
	
	UpdateData( FALSE );

	if ( Network.IsConnected() )
		PostMessage( WM_COMMAND, MAKELONG( IDC_TORRENT_REFRESH, BN_CLICKED ),
			(LPARAM)m_wndRefresh.GetSafeHwnd() );

	return TRUE;
}

void CTorrentTrackersPage::OnDestroy() 
{
	if ( IsThreadAlive() ) 
	{
		m_pRequest.Cancel();
		CloseThread();
	}
	
	CPropertyPageAdv::OnDestroy();
}

void CTorrentTrackersPage::OnTorrentRefresh() 
{
	if ( m_wndRefresh.IsWindowEnabled() == FALSE ) return;
	
	if ( IsThreadAlive() ) 
	{
		m_pRequest.Cancel();
		CloseThread();
	}
	
	m_wndRefresh.EnableWindow( FALSE );
	BeginThread( "PageTorrentTrackers" );
}

void CTorrentTrackersPage::OnTimer(UINT_PTR nIDEvent) 
{
	if ( nIDEvent == 1 )
	{
		// Re-enable the refresh button
		m_wndRefresh.EnableWindow( TRUE );
		KillTimer( 1 );
	}
	else
	{
		// Close the scrape thread
		CloseThread();
		// Re-enable the refresh button in one minute
		SetTimer( 1, 60000, NULL );
		
		if ( nIDEvent == 3 )
		{
			// Update the display
			CString str;
			str.Format( _T("%i"), m_nComplete );
			m_wndComplete.SetWindowText( str );
			str.Format( _T("%i"), m_nIncomplete );
			m_wndIncomplete.SetWindowText( str );
		}
	}
}

void CTorrentTrackersPage::OnOK()
{
	UpdateData();

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
	if ( Downloads.Check( pDownload ) && pDownload->IsTorrent() )
	{
		CBTInfo* pInfo = &pDownload->m_pTorrent;

		// Check if tracker has been changed, and the new value could be valid
		if ( ( pInfo->m_sTracker != m_sTracker ) &&
			( m_sTracker.Find( _T("http") ) == 0 ) )
		{
			CString strMessage;
			LoadString( strMessage, IDS_BT_TRACK_CHANGE );
			
			// Display warning
			pLock.Unlock();
			if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
			{
				pLock.Lock();
				pInfo->m_sTracker = m_sTracker;
				pInfo->m_nTrackerMode = tCustom;
				pInfo->m_oBTH.validate();
			}
		}
		else
		{
			int nTrackerMode = m_wndTrackerMode.GetCurSel();
			if ( pInfo->m_nTrackerMode != nTrackerMode )
			{
				// Check it's valid
				if ( ( ( nTrackerMode == tMultiFound )		&& ( pInfo->IsMultiTracker() ) ) ||
					 ( ( nTrackerMode == tMultiFinding )	&& ( pInfo->IsMultiTracker() ) ) ||
					 ( ( nTrackerMode == tSingle )			&& ( pInfo->m_pAnnounceTracker ) ) ||
					 ( ( nTrackerMode == tCustom )			&& ( pInfo->m_sTracker.GetLength() > 7 ) ) )
				{
					CString strMessage;
					LoadString( strMessage, IDS_BT_TRACK_CHANGE );
					pLock.Unlock();
					// Display warning
					if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
					{
						pLock.Lock();
						pInfo->m_nTrackerMode = nTrackerMode;
					}
				}
			}
		}
	}
	CPropertyPageAdv::OnOK();
}

void CTorrentTrackersPage::OnRun()
{
	m_pRequest.Clear();

	CString strURL = m_sTracker;
	strURL.Replace( _T("/announce"), _T("/scrape") );

	if ( ( strURL.Find( _T("http") ) == 0 ) && ( strURL.Find( _T("/scrape") ) != -1 ) )
	{
		// Skip obviously invalid trackers
		if ( strURL.GetLength() > 7 ) 
		{
			// Fetch scrape only for the given info hash
			{
				CSingleLock pLock( &Transfers.m_pSection, TRUE );
				CString strParam = CBTTrackerRequest::Escape( m_pDownload->m_pTorrent.m_oBTH );
				if ( strURL.Find( _T("/scrape?") ) != -1 )
				{
					strURL.Append( L"&info_hash=" + strParam );
				}
				else
				{
					strURL.Append( L"?info_hash=" + strParam );
				}
				strURL.Append( L"&peer_id=" + CBTTrackerRequest::Escape( m_pDownload->m_pPeerID ) );
			}

			m_pRequest.SetURL( strURL );
			m_pRequest.AddHeader( _T("Accept-Encoding"), _T("deflate, gzip") );
			m_pRequest.EnableCookie( false );
			m_pRequest.SetUserAgent( Settings.SmartAgent() );

			theApp.Message( MSG_DEBUG | MSG_FACILITY_OUTGOING,
				_T("[BT] Sending BitTorrent tracker scrape: %s"), strURL );

			if ( m_pRequest.Execute( FALSE ) && m_pRequest.InflateResponse() )
			{
				CBuffer* pResponse = m_pRequest.GetResponseBuffer();

				if ( pResponse != NULL && pResponse->m_pBuffer != NULL )
				{
					if ( CBENode* pNode = CBENode::Decode( pResponse ) )
					{
						theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING,
							_T("[BT] Recieved BitTorrent tracker response: %s"), pNode->Encode() );

						if ( OnTree( pNode ) )
						{
							delete pNode;
							PostMessage( WM_TIMER, 3 );
							return;
						}
						
						delete pNode;
					}
				}
			}
		}
	}
	
	m_pRequest.Clear();
	PostMessage( WM_TIMER, 2 );
}

BOOL CTorrentTrackersPage::OnTree(CBENode* pNode)
{
	if ( ! pNode->IsType( CBENode::beDict ) ) return FALSE;
	
	CBENode* pFiles = pNode->GetNode( "files" );
	if ( ! pFiles->IsType( CBENode::beDict ) ) return FALSE;

	LPBYTE nKey;
	{
		CSingleLock pLock( &Transfers.m_pSection, TRUE );
		if ( ! Downloads.Check( m_pDownload ) ) return FALSE;
		CBTInfo* pInfo = &m_pDownload->m_pTorrent;
		nKey = &pInfo->m_oBTH[ 0 ];
	}
	
    CBENode* pFile = pFiles->GetNode( nKey, Hashes::BtHash::byteCount );
	if ( ! pFile->IsType( CBENode::beDict ) ) return FALSE;	
	
	m_nComplete		= 0;
	m_nIncomplete	= 0;
	
	if ( CBENode* pComplete = pFile->GetNode( "complete" ) )
	{
		if ( ! pComplete->IsType( CBENode::beInt ) ) return FALSE;
		// Since we read QWORDs, make sure we won't get negative values;
		// Some buggy trackers send very huge numbers, so let's leave them as
		// the max int.	
		m_nComplete = (int)(pComplete->GetInt() & ~0xFFFF0000);
	}
	
	if ( CBENode* pIncomplete = pFile->GetNode( "incomplete" ) )
	{
		if ( ! pIncomplete->IsType( CBENode::beInt ) ) return FALSE;	
		m_nIncomplete = (int)(pIncomplete->GetInt() & ~0xFFFF0000);
	}
	
	return TRUE;
}