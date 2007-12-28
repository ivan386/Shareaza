//
// PageTorrentTrackers.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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

#include "ShellIcons.h"
#include "BTInfo.h"
#include "BENode.h"
#include "PageTorrentTrackers.h"
#include "CoolInterface.h"
#include "Network.h"
#include "Skin.h"

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

CTorrentTrackersPage::CTorrentTrackersPage() : 
	CTorrentInfoPage( CTorrentTrackersPage::IDD ),
	m_sName(), m_sTracker(), m_sEscapedPeerID()
{
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
	m_sEscapedPeerID = Escape( GetPeerID().toString() );

	m_wndTrackerMode.SetItemData( 0, tNull );
	m_wndTrackerMode.SetItemData( 1, tCustom );
	m_wndTrackerMode.SetItemData( 2, tSingle );
	m_wndTrackerMode.SetItemData( 3, tMultiFinding );
	m_wndTrackerMode.SetItemData( 4, tMultiFound );

	m_wndTrackerMode.SetCurSel( m_pInfo->m_nTrackerMode );

	CRect rc;
	m_wndTrackers.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );
	CoolInterface.SetImageListTo( m_wndTrackers, LVSIL_SMALL );
	m_wndTrackers.InsertColumn( 0, _T("Tracker"), LVCFMT_LEFT, rc.right - 80, -1 );
	m_wndTrackers.InsertColumn( 1, _T("Status"), LVCFMT_RIGHT, 80, 0 );
	m_wndTrackers.InsertColumn( 2, _T("Type"), LVCFMT_LEFT, 0, 0 );
	Skin.Translate( _T("CTorrentTrackerList"), m_wndTrackers.GetHeaderCtrl() );

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

	if ( !m_pInfo->IsMultiTracker() && m_pInfo->m_pAnnounceTracker )
	{
		nTracker ++;

		LV_ITEM pItem = {};
		pItem.mask		= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		pItem.iItem		= m_wndTrackers.GetItemCount();
		pItem.lParam	= (LPARAM)nTracker;
		pItem.iImage = CoolInterface.ImageForID( ID_TOOLS_LANGUAGE );
		pItem.pszText	= (LPTSTR)(LPCTSTR)m_pInfo->m_pAnnounceTracker->m_sAddress;
		pItem.iItem		= m_wndTrackers.InsertItem( &pItem );
		
		// Display status
		CString sStatus;
		if ( ( m_pInfo->m_pAnnounceTracker->m_tNextTry == 0 ) && ( m_pInfo->m_pAnnounceTracker->m_tLastSuccess == 0 ) )
			LoadString( sStatus, IDS_STATUS_UNKNOWN );
		else if ( m_pInfo->m_pAnnounceTracker->m_tNextTry > m_pInfo->m_pAnnounceTracker->m_tLastSuccess )
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
		CloseThread( &m_hThread );
	}
	
	CTorrentInfoPage::OnDestroy();
}

void CTorrentTrackersPage::OnTorrentRefresh() 
{
	if ( m_wndRefresh.IsWindowEnabled() == FALSE ) return;
	
	if ( m_hThread != NULL ) 
	{
		m_pRequest.Cancel();
		CloseThread( &m_hThread );
	}
	
	m_wndRefresh.EnableWindow( FALSE );
	m_hThread = BeginThread( "PageTorrentTrackers", ThreadStart, this );
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
		CloseThread( &m_hThread );
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
			m_pInfo->m_oBTH.validate();
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
	strURL.Replace( _T("/announce"), _T("/scrape") );

	if ( ( strURL.Find( _T("http") ) == 0 ) && ( strURL.Find( _T("/scrape") ) != -1 ) )
	{
		// Skip obviously invalid trackers
		if ( strURL.GetLength() > 7 ) 
		{
			// Fetch scrape only for the given info hash
			CString strParam = Escape( m_pInfo->m_oBTH.toString< Hashes::base16Encoding >() );

			if ( strURL.Find( _T("/scrape?") ) != -1 )
			{
				strURL.Append( L"&info_hash=" + strParam );
			}
			else
			{
				strURL.Append( L"?info_hash=" + strParam );
			}
			strURL.Append( L"&peer_id=" + m_sEscapedPeerID );

			m_pRequest.SetURL( strURL );

			theApp.Message( MSG_DEBUG, _T("Sending scrape to %s"), strURL );
			
			if ( m_pRequest.Execute( FALSE ) && m_pRequest.InflateResponse() )
			{
				CBuffer* pResponse = m_pRequest.GetResponseBuffer();

				if ( pResponse != NULL && pResponse->m_pBuffer != NULL )
				{
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
	}
	
	m_pRequest.Clear();
	PostMessage( WM_TIMER, 2 );
}

CString	CTorrentTrackersPage::Escape(const CString& str)
{
	int nLen = str.GetLength();
	if ( nLen == 0 ) 
		return CString();

	CString strResult, strToken;
	
	for ( int nPos = 0 ; nPos < nLen ; nPos++ )
	{
		int nValue = 0;
		strToken = str.Mid( nPos, 2 );

		if ( _stscanf( strToken.GetBuffer(), L"%2x", &nValue ) == 1 )
		{
			if ( ( nValue >= '0' && nValue <= '9' ) ||
				 ( nValue >= 'a' && nValue <= 'z' ) ||
				 ( nValue >= 'A' && nValue <= 'Z' ) )
			{
				strResult += (TCHAR)nValue;
			}
			else
			{
				strResult += '%';
				strResult.Append( strToken );
			}
			nPos++;
		}
		else
			strResult.Append( strToken );
	}	
	
	return strResult;
}

BOOL CTorrentTrackersPage::OnTree(CBENode* pNode)
{
	if ( ! pNode->IsType( CBENode::beDict ) ) return FALSE;
	
	CBENode* pFiles = pNode->GetNode( "files" );
	if ( ! pFiles->IsType( CBENode::beDict ) ) return FALSE;
	
    CBENode* pFile = pFiles->GetNode( &m_pInfo->m_oBTH[ 0 ], Hashes::BtHash::byteCount );
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