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
	ON_BN_CLICKED(IDC_TORRENT_REFRESH, &CTorrentTrackersPage::OnTorrentRefresh)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_TORRENT_TRACKER, &CTorrentTrackersPage::OnEnChangeTorrentTracker)
	ON_NOTIFY(NM_CLICK, IDC_TORRENT_TRACKERS, &CTorrentTrackersPage::OnNMClickTorrentTrackers)
	ON_CBN_SELCHANGE(IDC_TORRENT_TRACKERMODE, &CTorrentTrackersPage::OnCbnSelchangeTorrentTrackermode)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTorrentTrackersPage property page

CTorrentTrackersPage::CTorrentTrackersPage()
	: CPropertyPageAdv	( CTorrentTrackersPage::IDD )
	, m_nComplete		( 0 )
	, m_nIncomplete		( 0 )
	, m_pRequest		( NULL )
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
	if ( ! CPropertyPageAdv::OnInitDialog() )
		return FALSE;
	
	ASSUME_LOCK( Transfers.m_pSection );

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	ASSERT( pDownload && pDownload->IsTorrent() );

	CBTInfo& oInfo = pDownload->m_pTorrent;

	m_sTracker = oInfo.GetTrackerAddress();
	int nCount = oInfo.GetTrackerCount();
	int nTrackerMode = oInfo.GetTrackerMode();

	// Remove invalid modes
	if ( nCount < 2 )
	{
		ASSERT( nTrackerMode!= CBTInfo::tMultiFound );
		m_wndTrackerMode.DeleteString( CBTInfo::tMultiFound );
		ASSERT( nTrackerMode != CBTInfo::tMultiFinding );
		m_wndTrackerMode.DeleteString( CBTInfo::tMultiFinding );
	}
	m_wndTrackerMode.SetCurSel( nTrackerMode );

	CRect rc;
	m_wndTrackers.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );
	CoolInterface.SetImageListTo( m_wndTrackers, LVSIL_SMALL );
	m_wndTrackers.InsertColumn( 0, _T("Tracker"), LVCFMT_LEFT, rc.right - 80, -1 );
	m_wndTrackers.InsertColumn( 1, _T("Status"), LVCFMT_RIGHT, 80, 0 );
	m_wndTrackers.InsertColumn( 2, _T("Type"), LVCFMT_LEFT, 0, 0 );
	Skin.Translate( _T("CTorrentTrackerList"), m_wndTrackers.GetHeaderCtrl() );

	int nTracker = 0;
	for ( nTracker = 0 ; nTracker < nCount; nTracker++ )
	{
		LV_ITEM pItem = {};
		pItem.mask		= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		pItem.iItem		= m_wndTrackers.GetItemCount();
		pItem.lParam	= (LPARAM)nTracker;

		if ( oInfo.GetTrackerIndex() == nTracker )
			pItem.iImage = CoolInterface.ImageForID( ID_MEDIA_SELECT );
		else
			pItem.iImage = CoolInterface.ImageForID( ID_DOWNLOADS_COPY );

		pItem.pszText	= (LPTSTR)(LPCTSTR)oInfo.GetTrackerAddress( nTracker );
		pItem.iItem		= m_wndTrackers.InsertItem( &pItem );

		// Display status
		CString sStatus;
		switch ( oInfo.GetTrackerStatus( nTracker ) )
		{
		case TRI_UNKNOWN:
			LoadString( sStatus, IDS_STATUS_UNKNOWN );
			break;
		case TRI_FALSE:
			LoadString( sStatus, IDS_STATUS_TRACKERDOWN );
			break;
		case TRI_TRUE:
			LoadString( sStatus, IDS_STATUS_ACTIVE );
			break;
		}
		m_wndTrackers.SetItemText( pItem.iItem, 1, sStatus );

		// Display type
		CString sType;
		if ( oInfo.IsMultiTracker() )
			sType.Format( _T("Tier %i"), oInfo.GetTrackerTier( nTracker ) );
		else
			sType = _T("Announce");
		m_wndTrackers.SetItemText( pItem.iItem, 2, sType );
	}

	GetDlgItem( IDC_TORRENT_TRACKER )->EnableWindow( nTrackerMode != CBTInfo::tNull );

	UpdateData( FALSE );

	if ( Network.IsConnected() )
		PostMessage( WM_COMMAND, MAKELONG( IDC_TORRENT_REFRESH, BN_CLICKED ),
			(LPARAM)m_wndRefresh.GetSafeHwnd() );

	return TRUE;
}

void CTorrentTrackersPage::OnDestroy() 
{
	KillTimer( 1 );

	// Cancel unfinished yet request
	if ( m_pRequest )
	{
		CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();

		CQuickLock oLock( Transfers.m_pSection );

		if ( CDownload* pDownload = pSheet->GetDownload() )
		{
			pDownload->CancelRequest( m_pRequest );
		}
	}

	CPropertyPageAdv::OnDestroy();
}

void CTorrentTrackersPage::OnTrackerEvent(bool bSuccess, LPCTSTR /*pszReason*/, LPCTSTR /*pszTip*/, CBTTrackerRequest* pEvent)
{
	ASSUME_LOCK( Transfers.m_pSection );

	m_pRequest = NULL; // Need no cancel

	if ( bSuccess )
	{
		m_nComplete = pEvent->m_nSeeders;
		m_nIncomplete = pEvent->m_nLeechers;
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

	m_pRequest = new CBTTrackerRequest( pDownload, BTE_TRACKER_SCRAPE, 0, this );
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
	if ( ! UpdateData() )
		return FALSE;

	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();

	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return FALSE;

	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
		return CPropertyPageAdv::OnApply();

	CBTInfo& oInfo = pDownload->m_pTorrent;

	int nTrackerMode = m_wndTrackerMode.GetCurSel();
	bool bAddressChanged = oInfo.GetTrackerAddress() != m_sTracker;
	bool bModeChanged = oInfo.GetTrackerMode() != nTrackerMode;
	if ( bAddressChanged || bModeChanged )
	{
		oLock.Unlock();

		GetDlgItem( IDC_TORRENT_TRACKER )->SetFocus();

		// Display warning
		if ( AfxMessageBox( IDS_BT_TRACK_CHANGE, MB_ICONQUESTION | MB_YESNO ) != IDYES )
			return FALSE;

		if ( ! oLock.Lock( 250 ) )
			return FALSE;

		pDownload = pSheet->GetDownload();
		if ( ! pDownload )
			return CPropertyPageAdv::OnApply();

		if ( bAddressChanged )
			oInfo.SetTracker( m_sTracker );
		if ( bModeChanged )
			oInfo.SetTrackerMode( nTrackerMode );
	}

	return CPropertyPageAdv::OnApply();
}

void CTorrentTrackersPage::OnEnChangeTorrentTracker()
{
	if ( ! UpdateData() )
		return;
 
	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return;
 
	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	if ( ! pDownload )
		return;

	CBTInfo& oInfo = pDownload->m_pTorrent;

	if ( m_sTracker != oInfo.GetTrackerAddress() )
		m_wndTrackerMode.SetCurSel( CBTInfo::tSingle );
	else if ( oInfo.GetTrackerMode() != CBTInfo::tNull )
		m_wndTrackerMode.SetCurSel( oInfo.GetTrackerMode() );
}

void CTorrentTrackersPage::OnNMClickTorrentTrackers(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast< LPNMITEMACTIVATE >( pNMHDR );
	*pResult = 0;

	if ( ! UpdateData() )
		return;

	if ( pNMItemActivate->iItem != -1 )
	{
		m_sTracker = m_wndTrackers.GetItemText( pNMItemActivate->iItem, 0 );
		m_wndTrackerMode.SetCurSel( CBTInfo::tSingle );

		GetDlgItem( IDC_TORRENT_TRACKER )->EnableWindow( TRUE );

		UpdateData( FALSE );
	}
}

void CTorrentTrackersPage::OnCbnSelchangeTorrentTrackermode()
{
	int nMode = m_wndTrackerMode.GetCurSel();

	GetDlgItem( IDC_TORRENT_TRACKER )->EnableWindow( nMode != CBTInfo::tNull );
}
