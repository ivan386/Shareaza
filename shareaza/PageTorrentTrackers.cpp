//
// PageTorrentTrackers.cpp
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

#include "ShellIcons.h"
#include "BTInfo.h"
#include "BENode.h"
#include "PageTorrentTrackers.h"
#include "CoolInterface.h"
#include "Network.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTorrentTrackersPage, CTorrentInfoPage)

BEGIN_MESSAGE_MAP(CTorrentTrackersPage, CTorrentInfoPage)
	//{{AFX_MSG_MAP(CTorrentTrackersPage)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_TORRENT_REFRESH, OnTorrentRefresh)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTorrentTrackersPage property page

CTorrentTrackersPage::CTorrentTrackersPage() : CTorrentInfoPage( CTorrentTrackersPage::IDD )
{
	//{{AFX_DATA_INIT(CTorrentTrackersPage)
	m_sName = _T("");
	m_sTracker = _T("");
	//}}AFX_DATA_INIT
}

CTorrentTrackersPage::~CTorrentTrackersPage()
{
}

void CTorrentTrackersPage::DoDataExchange(CDataExchange* pDX)
{
	CTorrentInfoPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTorrentTrackersPage)
	DDX_Text(pDX, IDC_TORRENT_NAME, m_sName);
	DDX_Text(pDX, IDC_TORRENT_TRACKER, m_sTracker);
	DDX_Control(pDX, IDC_TORRENT_COMPLETED, m_wndComplete);
	DDX_Control(pDX, IDC_TORRENT_INCOMPLETE, m_wndIncomplete);
	DDX_Control(pDX, IDC_TORRENT_REFRESH, m_wndRefresh);
	DDX_Control(pDX, IDC_TORRENT_TRACKERS, m_wndTrackers);
	DDX_Control(pDX, IDC_TORRENT_TRACKERMODE, m_wndTrackerMode);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentTrackersPage message handlers

BOOL CTorrentTrackersPage::OnInitDialog()
{
	CTorrentInfoPage::OnInitDialog();
	
	m_sName			= m_pInfo->m_sName;
	m_sTracker		= m_pInfo->m_sTracker;

	m_wndTrackerMode.SetItemData( 0, tNull );
	m_wndTrackerMode.SetItemData( 1, tCustom );
	m_wndTrackerMode.SetItemData( 2, tSingle );
	m_wndTrackerMode.SetItemData( 3, tMultiFinding );
	m_wndTrackerMode.SetItemData( 4, tMultiFound );

	m_wndTrackerMode.SetCurSel( m_pInfo->m_nTrackerMode );

	CRect rc;
	m_wndTrackers.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );
	m_wndTrackers.SetImageList( &CoolInterface.m_pImages, LVSIL_SMALL );
	m_wndTrackers.InsertColumn( 0, _T("Tracker"), LVCFMT_LEFT, rc.right - 80, -1 );
	m_wndTrackers.InsertColumn( 1, _T("Status"), LVCFMT_RIGHT, 80, 0 );

	int nTracker = 0;
	for ( nTracker = 0 ; nTracker < m_pInfo->m_pTrackerList.GetCount() ; nTracker++ )
	{
		CBTInfo::CBTTracker* pTrack = m_pInfo->m_pTrackerList.GetAt(nTracker);
		
		LV_ITEM pItem = {};
		pItem.mask		= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		pItem.iItem		= m_wndTrackers.GetItemCount();
		pItem.lParam	= (LPARAM)nTracker;

		if ( m_pInfo->m_nTrackerIndex == nTracker )
			pItem.iImage = CoolInterface.ImageForID( ID_MEDIA_SELECT );
		else
			pItem.iImage = CoolInterface.ImageForID( ID_DOWNLOADS_COPY );

		pItem.pszText	= (LPTSTR)(LPCTSTR)pTrack->m_sAddress;
		pItem.iItem		= m_wndTrackers.InsertItem( &pItem );

		CString sType;
		if ( ( pTrack->m_tLastFail == 0 ) && ( pTrack->m_tLastSuccess == 0 ) )
			LoadString( sType, IDS_STATUS_UNKNOWN );
		else if ( pTrack->m_tLastFail > pTrack->m_tLastSuccess )
			LoadString( sType, IDS_STATUS_TRACKERDOWN );
		else
			LoadString( sType, IDS_STATUS_ACTIVE );

		m_wndTrackers.SetItemText( pItem.iItem, 1, sType );
	}

	if ( m_pInfo->m_pAnnounceTracker )
	{
		nTracker ++;

		LV_ITEM pItem = {};
		pItem.mask		= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		pItem.iItem		= m_wndTrackers.GetItemCount();
		pItem.lParam	= (LPARAM)nTracker;
		pItem.iImage = CoolInterface.ImageForID( ID_TOOLS_LANGUAGE );
		pItem.pszText	= (LPTSTR)(LPCTSTR)m_pInfo->m_pAnnounceTracker->m_sAddress;
		pItem.iItem		= m_wndTrackers.InsertItem( &pItem );
		
		CString sType;
		if ( ( m_pInfo->m_pAnnounceTracker->m_tLastFail == 0 ) && ( m_pInfo->m_pAnnounceTracker->m_tLastSuccess == 0 ) )
			LoadString( sType, IDS_STATUS_UNKNOWN );
		else if ( m_pInfo->m_pAnnounceTracker->m_tLastFail > m_pInfo->m_pAnnounceTracker->m_tLastSuccess )
			LoadString( sType, IDS_STATUS_TRACKERDOWN );
		else
			LoadString( sType, IDS_STATUS_ACTIVE );

		m_wndTrackers.SetItemText( pItem.iItem, 1, sType );
	}
	
	UpdateData( FALSE );
	m_hThread = NULL;

	if ( Network.IsConnected() )
		PostMessage( WM_COMMAND, MAKELONG( IDC_TORRENT_REFRESH, BN_CLICKED ), (LPARAM)m_wndRefresh.GetSafeHwnd() );

	return TRUE;
}


void CTorrentTrackersPage::OnDestroy() 
{
	if ( m_hThread != NULL ) 
	{
		m_pRequest.Cancel();
		CHttpRequest::CloseThread( &m_hThread, _T("CTorrentTrackersPage") );
	}
	
	CTorrentInfoPage::OnDestroy();
}

void CTorrentTrackersPage::OnTorrentRefresh() 
{
	if ( m_wndRefresh.IsWindowEnabled() == FALSE ) return;
	
	if ( m_hThread != NULL ) 
	{
		m_pRequest.Cancel();
		CHttpRequest::CloseThread( &m_hThread, _T("CTorrentTrackersPage") );
	}
	
	m_wndRefresh.EnableWindow( FALSE );
	m_hThread = AfxBeginThread( ThreadStart, this )->m_hThread;
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
		CHttpRequest::CloseThread( &m_hThread, _T("CTorrentTrackersPage") );
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

	// Check if tracker has been changed, and the new value could be valid
	if ( ( m_pInfo->m_sTracker != m_sTracker ) && ( m_sTracker.Find( _T("http") ) == 0 ) )
	{
		CString strMessage;
		LoadString( strMessage, IDS_BT_TRACK_CHANGE );
		
		// Display warning
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
		{
			m_pInfo->m_sTracker = m_sTracker;
			m_pInfo->m_nTrackerMode = tCustom;
			m_pInfo->m_oInfoBTH.validate();
		}
	}
	else
	{
		int nTrackerMode = m_wndTrackerMode.GetCurSel();
		if ( m_pInfo->m_nTrackerMode != nTrackerMode )
		{
			// Check it's valid
			if ( ( ( nTrackerMode == tMultiFound )		&& ( m_pInfo->IsMultiTracker() ) ) ||
				 ( ( nTrackerMode == tMultiFinding )	&& ( m_pInfo->IsMultiTracker() ) ) ||
				 ( ( nTrackerMode == tSingle )			&& ( m_pInfo->m_pAnnounceTracker ) ) ||
				 ( ( nTrackerMode == tCustom )			&& ( m_pInfo->m_sTracker.GetLength() > 7 ) ) )
			{
				CString strMessage;
				LoadString( strMessage, IDS_BT_TRACK_CHANGE );
				// Display warning
				if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
					m_pInfo->m_nTrackerMode = nTrackerMode;
			}
		}
	}
	
	CTorrentInfoPage::OnOK();
}

UINT CTorrentTrackersPage::ThreadStart(LPVOID pParam)
{
	CTorrentTrackersPage* pObject = (CTorrentTrackersPage*)pParam;
	pObject->OnRun();
	return 0;
}

void CTorrentTrackersPage::OnRun()
{
	m_pRequest.Clear();
	m_pRequest.AddHeader( _T("Accept-Encoding"), _T("deflate, gzip") );
	
	CString strURL = m_sTracker;
	Replace( strURL, _T("/announce"), _T("/scrape") );

	if ( ( strURL.Find( _T("http") ) == 0 ) && ( strURL.Find( _T("/scrape") ) != -1 ) )
	{
		// Skip obviously invalid trackers
		if ( strURL.GetLength() > 7 ) 
		{
			m_pRequest.SetURL( strURL );

			theApp.Message( MSG_DEBUG, _T("Sending scrape to %s"), strURL );
			
			if ( m_pRequest.Execute( FALSE ) && m_pRequest.InflateResponse() )
			{
				CBuffer* pResponse = m_pRequest.GetResponseBuffer();
				
				if ( CBENode* pNode = CBENode::Decode( pResponse ) )
				{
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
	
	m_pRequest.Clear();
	PostMessage( WM_TIMER, 2 );
}

BOOL CTorrentTrackersPage::OnTree(CBENode* pNode)
{
	if ( ! pNode->IsType( CBENode::beDict ) ) return FALSE;
	
	CBENode* pFiles = pNode->GetNode( "files" );
	if ( ! pFiles->IsType( CBENode::beDict ) ) return FALSE;
	
    CBENode* pFile = pFiles->GetNode( &m_pInfo->m_oInfoBTH[ 0 ], Hashes::BtHash::byteCount );
	if ( ! pFile->IsType( CBENode::beDict ) ) return FALSE;	
	
	m_nComplete		= 0;
	m_nIncomplete	= 0;
	
	if ( CBENode* pComplete = pFile->GetNode( "complete" ) )
	{
		if ( ! pComplete->IsType( CBENode::beInt ) ) return FALSE;	
		m_nComplete = (int)pComplete->GetInt();
	}
	
	if ( CBENode* pIncomplete = pFile->GetNode( "incomplete" ) )
	{
		if ( ! pIncomplete->IsType( CBENode::beInt ) ) return FALSE;	
		m_nIncomplete = (int)pIncomplete->GetInt();
	}
	
	return TRUE;
}