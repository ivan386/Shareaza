//
// PageTorrentTrackers.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "BTInfo.h"
#include "CoolInterface.h"
#include "DlgDownloadSheet.h"
#include "Downloads.h"
#include "Network.h"
#include "PageTorrentTrackers.h"
#include "Skin.h"
#include "Transfers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTorrentTrackersPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CTorrentTrackersPage, CPropertyPageAdv)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_TORRENT_REFRESH, &CTorrentTrackersPage::OnTorrentRefresh)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_NOTIFY(NM_CLICK, IDC_TORRENT_TRACKERS, &CTorrentTrackersPage::OnNMClickTorrentTrackers)
	ON_CBN_SELCHANGE(IDC_TORRENT_TRACKERMODE, &CTorrentTrackersPage::OnCbnSelchangeTorrentTrackermode)
	ON_NOTIFY(LVN_KEYDOWN, IDC_TORRENT_TRACKERS, &CTorrentTrackersPage::OnLvnKeydownTorrentTrackers)
	ON_BN_CLICKED(IDC_TORRENT_TRACKERS_ADD, &CTorrentTrackersPage::OnBnClickedTorrentTrackersAdd)
	ON_BN_CLICKED(IDC_TORRENT_TRACKERS_DEL, &CTorrentTrackersPage::OnBnClickedTorrentTrackersDel)
	ON_BN_CLICKED(IDC_TORRENT_TRACKERS_REN, &CTorrentTrackersPage::OnBnClickedTorrentTrackersRen)
	ON_NOTIFY(NM_DBLCLK, IDC_TORRENT_TRACKERS, &CTorrentTrackersPage::OnNMDblclkTorrentTrackers)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_TORRENT_TRACKERS, &CTorrentTrackersPage::OnLvnEndlabeleditTorrentTrackers)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTorrentTrackersPage property page

CTorrentTrackersPage::CTorrentTrackersPage()
	: CPropertyPageAdv	( CTorrentTrackersPage::IDD )
	, m_nOriginalMode	( CBTInfo::tNull )
	, m_nComplete		( 0 )
	, m_nIncomplete		( 0 )
	, m_nRequest		( NULL )
{
}

CTorrentTrackersPage::~CTorrentTrackersPage()
{
}

void CTorrentTrackersPage::UpdateInterface()
{
	int nItem = m_wndTrackers.GetNextItem( -1, LVNI_SELECTED );
	m_wndDel.EnableWindow( ( nItem != -1 ) );
	m_wndRen.EnableWindow( ( nItem != -1 ) );

	// Find item with current tracker ...
	LVFINDINFO fi =
	{
		LVFI_STRING,
		m_sOriginalTracker
	};
	int nCurrentItem = m_wndTrackers.FindItem( &fi );

	// ... and mark it
	LVITEM lvi =
	{
		LVIF_PARAM | LVIF_IMAGE
	};
	int nCount = m_wndTrackers.GetItemCount();
	for ( int i = 0; i < nCount; ++i )
	{
		lvi.iItem = i;
		lvi.iImage = ( i == nCurrentItem ) ? CoolInterface.ImageForID( ID_MEDIA_NEXT ) : CoolInterface.ImageForID( ID_DOWNLOADS_COPY );		
		lvi.lParam = ( i == nCurrentItem ) ? TRUE : FALSE;
		m_wndTrackers.SetItem( &lvi );
	}

	if ( nCount == 0 )
		m_wndTrackerMode.SetCurSel( CBTInfo::tNull );
	else if ( nCount == 1 )
		m_wndTrackerMode.SetCurSel( CBTInfo::tSingle );
	m_wndTrackerMode.EnableWindow( nCount > 1 );

	UpdateWindow();
}

BOOL CTorrentTrackersPage::ApplyTracker()
{
	CString sNewTracker;
	m_wndTracker.GetWindowText( sNewTracker );
	bool bAddressChanged = m_sOriginalTracker != sNewTracker;

	int nNewTrackerMode = m_wndTrackerMode.GetCurSel();
	bool bModeChanged = m_nOriginalMode != nNewTrackerMode;

	bool bListChanged = false;
	int nCount = m_wndTrackers.GetItemCount();
	if ( nCount == m_sOriginalTrackers.GetCount() )
	{
		int i = 0;
		for ( POSITION pos = m_sOriginalTrackers.GetHeadPosition(); pos; ++i )
		{
			if ( m_sOriginalTrackers.GetNext( pos ) != m_wndTrackers.GetItemText( i, 0 ) )
			{
				bListChanged = true;
				break;
			}
		}
	}
	else
		bListChanged = true;


	if ( bAddressChanged || bModeChanged || bListChanged )
	{
		CSingleLock oLock( &Transfers.m_pSection );

		// Display warning
		if ( AfxMessageBox( IDS_BT_TRACK_CHANGE, MB_ICONQUESTION | MB_YESNO ) != IDYES || ! oLock.Lock( 250 ) )
		{
			// Restore original settings
			m_wndTracker.SetWindowText( m_sOriginalTracker );
			m_wndTrackerMode.SetCurSel( m_nOriginalMode );
			return FALSE;
		}

		// Apply new settings
		CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
		if ( CDownload* pDownload = pSheet->GetDownload() )
		{
			CBTInfo& oInfo = pDownload->m_pTorrent;

			m_sOriginalTracker.Empty();
			m_nOriginalMode = nCount ? ( ( nCount > 1 ) ? CBTInfo::tMultiFinding : CBTInfo::tSingle ) : CBTInfo::tNull;
			m_sOriginalTrackers.RemoveAll();

			oInfo.RemoveAllTrackers();
			for ( int i = 0; i < nCount; ++i )
			{
				CString sTracker = m_wndTrackers.GetItemText( i, 0 );
				if ( sTracker == sNewTracker )
				{
					m_sOriginalTracker = sNewTracker;
					m_nOriginalMode = nNewTrackerMode;
				}
				m_sOriginalTrackers.AddTail( sTracker );
				oInfo.SetTracker( sTracker );
			}

			if ( ! m_sOriginalTracker.IsEmpty() )
				oInfo.SetTracker( m_sOriginalTracker );
			oInfo.SetTrackerMode( m_nOriginalMode );

			pDownload->SetModified();
		}
	}
	return TRUE;
}

void CTorrentTrackersPage::InsertTracker()
{
	// De-select all
	int nCount = m_wndTrackers.GetItemCount();
	for ( int i = 0; i < nCount; ++i )
		m_wndTrackers.SetItemState( i, 0, LVIS_SELECTED );

	LVITEM lvi =
	{
		LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT,
		nCount,
		0,
		LVIS_SELECTED,
		LVIS_SELECTED,
		(LPTSTR)(LPCTSTR)Settings.BitTorrent.DefaultTracker,
		0,
		CoolInterface.ImageForID( ID_DOWNLOADS_COPY )
	};
	int nItem = m_wndTrackers.InsertItem( &lvi );
	m_wndTrackers.SetItemText( nItem, 1, LoadString( IDS_STATUS_UNKNOWN ) );
	m_wndTrackers.SetFocus();
	m_wndTrackers.EditLabel( nItem );

	if ( nCount == 1 )
		m_wndTrackerMode.SetCurSel( CBTInfo::tMultiFinding );

	UpdateInterface();
}

void CTorrentTrackersPage::EditTracker(int nItem, LPCTSTR szText)
{
	CString sEditedTracker = m_wndTrackers.GetItemText( nItem, 0 );
	if ( ! szText )
	{
		// New item
		LVFINDINFO fi =
		{
			LVFI_STRING,
			sEditedTracker
		};
		int nDuplicate = m_wndTrackers.FindItem( &fi );
		if ( nDuplicate != -1 && nDuplicate != nItem )
		{
			// Remove duplicate tracker
			m_wndTrackers.DeleteItem( nItem );
		}
		return;
	}

	CString sNewTracker = szText;
	sNewTracker.Trim();
	if ( sNewTracker.IsEmpty() )
	{
		// Remove tracker
		m_wndTrackers.DeleteItem( nItem );
		return;
	}

	if ( sEditedTracker == sNewTracker )
		// No changes
		return;

	// Fix URL
	if ( ! StartsWith( sNewTracker, _PT("http://") ) && ! StartsWith( sNewTracker, _PT("udp://") ) )
		sNewTracker = _T("http://") + sNewTracker;

	LVFINDINFO fi =
	{
		LVFI_STRING,
		sNewTracker
	};
	int nDuplicate = m_wndTrackers.FindItem( &fi );
	if ( nDuplicate == -1 || nItem == nDuplicate )
	{
		// User entered unique tracker
		m_wndTrackers.SetItemText( nItem, 0, sNewTracker );
	}
	else
	{
		// Remove duplicate tracker
		m_wndTrackers.DeleteItem( nItem );
	}

	UpdateInterface();
}

void CTorrentTrackersPage::SelectTracker(int nItem)
{
	if ( nItem != -1 )
	{
		m_wndTracker.SetWindowText( m_wndTrackers.GetItemText( nItem, 0 ) );
		m_wndTrackerMode.SetCurSel( CBTInfo::tSingle );

		if ( ApplyTracker() )
		{
			OnTorrentRefresh();
		}

		UpdateInterface();
	}
}

void CTorrentTrackersPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TORRENT_TRACKER, m_wndTracker);
	DDX_Control(pDX, IDC_TORRENT_COMPLETED, m_wndComplete);
	DDX_Control(pDX, IDC_TORRENT_INCOMPLETE, m_wndIncomplete);
	DDX_Control(pDX, IDC_TORRENT_REFRESH, m_wndRefresh);
	DDX_Control(pDX, IDC_TORRENT_TRACKERS, m_wndTrackers);
	DDX_Control(pDX, IDC_TORRENT_TRACKERMODE, m_wndTrackerMode);
	DDX_Control(pDX, IDC_TORRENT_TRACKERS_ADD, m_wndAdd);
	DDX_Control(pDX, IDC_TORRENT_TRACKERS_DEL, m_wndDel);
	DDX_Control(pDX, IDC_TORRENT_TRACKERS_REN, m_wndRen);
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentTrackersPage message handlers

BOOL CTorrentTrackersPage::OnInitDialog()
{
	if ( ! CPropertyPageAdv::OnInitDialog() )
		return FALSE;

	m_wndAdd.SetIcon( CoolInterface.ExtractIcon( ID_LIBRARY_ADD ) );
	m_wndDel.SetIcon( CoolInterface.ExtractIcon( ID_LIBRARY_DELETE ) );
	m_wndRen.SetIcon( CoolInterface.ExtractIcon( ID_LIBRARY_RENAME ) );

	ASSUME_LOCK( Transfers.m_pSection );

	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
	CDownload* pDownload = pSheet->GetDownload();
	ASSERT( pDownload && pDownload->IsTorrent() );

	CBTInfo& oInfo = pDownload->m_pTorrent;

	m_sOriginalTracker = oInfo.GetTrackerAddress();
	m_wndTracker.SetWindowText( m_sOriginalTracker );

	int nCount = oInfo.GetTrackerCount();
	m_nOriginalMode = oInfo.GetTrackerMode();
	m_wndTrackerMode.SetCurSel( m_nOriginalMode );

	CRect rc;
	m_wndTrackers.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );

	CoolInterface.SetImageListTo( m_wndTrackers, LVSIL_SMALL );
	m_wndTrackers.InsertColumn( 0, _T("Tracker"), LVCFMT_LEFT, rc.right - 80, -1 );
	m_wndTrackers.InsertColumn( 1, _T("Status"), LVCFMT_RIGHT, 80, 0 );
	m_wndTrackers.InsertColumn( 2, _T("Type"), LVCFMT_LEFT, 0, 0 );
	Skin.Translate( _T("CTorrentTrackerList"), m_wndTrackers.GetHeaderCtrl() );

	for ( int nTracker = 0; nTracker < nCount; ++nTracker )
	{
		CString sTracker = oInfo.GetTrackerAddress( nTracker );
		m_sOriginalTrackers.AddTail( sTracker );

		int nItem = m_wndTrackers.InsertItem( m_wndTrackers.GetItemCount(), sTracker, CoolInterface.ImageForID( ID_DOWNLOADS_COPY ) );

		// Display status
		UINT nStatus = IDS_STATUS_UNKNOWN;
		switch ( oInfo.GetTrackerStatus( nTracker ) )
		{
		case TRI_FALSE:
			nStatus = IDS_STATUS_TRACKERDOWN;
			break;
		case TRI_TRUE:
			nStatus = IDS_STATUS_ACTIVE;
			break;
		default:
			;
		}
		m_wndTrackers.SetItemText( nItem, 1, LoadString( nStatus ) );

		// Display type
		CString sType;
		if ( oInfo.IsMultiTracker() )
			sType.Format( _T("Tier %i"), oInfo.GetTrackerTier( nTracker ) );
		else
			sType = _T("Announce");
		m_wndTrackers.SetItemText( nItem, 2, sType );
	}

	if ( Network.IsConnected() )
		PostMessage( WM_COMMAND, MAKELONG( IDC_TORRENT_REFRESH, BN_CLICKED ),
			(LPARAM)m_wndRefresh.GetSafeHwnd() );

	UpdateInterface();

	return TRUE;
}

void CTorrentTrackersPage::OnDestroy() 
{
	KillTimer( 1 );

	// Cancel unfinished yet request
	TrackerRequests.Cancel( m_nRequest );

	CPropertyPageAdv::OnDestroy();
}

void CTorrentTrackersPage::OnTrackerEvent(bool bSuccess, LPCTSTR /*pszReason*/, LPCTSTR /*pszTip*/, CBTTrackerRequest* pEvent)
{
	ASSUME_LOCK( Transfers.m_pSection );

	m_nRequest = 0; // Need no cancel

	if ( bSuccess )
	{
		m_nComplete = pEvent->GetComplete();
		m_nIncomplete = pEvent->GetIncomplete();
	}

	PostMessage( WM_TIMER, bSuccess ? 3 : 2 );
}

void CTorrentTrackersPage::OnTorrentRefresh() 
{
	if ( m_wndRefresh.IsWindowEnabled() == FALSE )
		return;

	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();

	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return;

	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
		return;

	m_wndRefresh.EnableWindow( FALSE );

	m_nRequest = TrackerRequests.Request( pDownload, BTE_TRACKER_SCRAPE, 0, this );
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
		// Re-enable the refresh button
		SetTimer( 1, 5000, NULL );

		if ( nIDEvent == 3 )
		{
			// Update the display
			CString str;
			str.Format( _T("%u"), m_nComplete );
			m_wndComplete.SetWindowText( str );
			str.Format( _T("%u"), m_nIncomplete );
			m_wndIncomplete.SetWindowText( str );
		}
		else
		{
			m_wndComplete.SetWindowText( _T("") );
			m_wndIncomplete.SetWindowText( _T("") );
		}
	}
}

BOOL CTorrentTrackersPage::OnApply()
{
	return ApplyTracker() ? CPropertyPageAdv::OnApply() : FALSE;
}

void CTorrentTrackersPage::OnNMClickTorrentTrackers(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;

	UpdateInterface();
}

void CTorrentTrackersPage::OnNMDblclkTorrentTrackers(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;

	SelectTracker( m_wndTrackers.GetNextItem( -1, LVNI_SELECTED ) );
}

void CTorrentTrackersPage::OnLvnKeydownTorrentTrackers(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pNMKD = (LPNMLVKEYDOWN)pNMHDR;

	*pResult = 0;

	if ( pNMKD->wVKey == VK_DELETE )
	{
		int nItem = m_wndTrackers.GetNextItem( -1, LVNI_SELECTED );
		if ( nItem != -1 )
		{
			m_wndTrackers.DeleteItem( nItem );
			UpdateInterface();
		}
	}
	else if ( pNMKD->wVKey == VK_INSERT )
	{
		InsertTracker();
	}
	else if ( pNMKD->wVKey == VK_SPACE || pNMKD->wVKey == VK_RETURN )
	{
		SelectTracker( m_wndTrackers.GetNextItem( -1, LVNI_SELECTED ) );
	}
}

void CTorrentTrackersPage::OnCbnSelchangeTorrentTrackermode()
{
	UpdateInterface();
}

void CTorrentTrackersPage::OnBnClickedTorrentTrackersAdd()
{
	InsertTracker();
}

void CTorrentTrackersPage::OnBnClickedTorrentTrackersDel()
{
	int nItem = m_wndTrackers.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem != -1 )
	{
		m_wndTrackers.DeleteItem( nItem );
		UpdateInterface();
	}
}

void CTorrentTrackersPage::OnBnClickedTorrentTrackersRen()
{
	int nItem = m_wndTrackers.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem != -1 )
	{
		m_wndTrackers.SetFocus();
		m_wndTrackers.EditLabel( nItem );
	}
}

void CTorrentTrackersPage::OnLvnEndlabeleditTorrentTrackers(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pDispInfo = reinterpret_cast< NMLVDISPINFO* >( pNMHDR );

	*pResult = 0;

	EditTracker( pDispInfo->item.iItem, pDispInfo->item.pszText );
}
