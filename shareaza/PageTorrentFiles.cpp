//
// PageTorrentGeneral.cpp
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

#include "ShellIcons.h"
#include "DlgDownloadSheet.h"
#include "PageTorrentFiles.h"
#include "Skin.h"
#include "Transfers.h"
#include "Downloads.h"
#include "CtrlLibraryTip.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTorrentFilesPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CTorrentFilesPage, CPropertyPageAdv)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTorrentFilesPage property page

CTorrentFilesPage::CTorrentFilesPage() : 
	CPropertyPageAdv( CTorrentFilesPage::IDD )
{
}

CTorrentFilesPage::~CTorrentFilesPage()
{
}

void CTorrentFilesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TORRENT_FILES, m_wndFiles);
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentFilesPage message handlers

BOOL CTorrentFilesPage::OnInitDialog()
{
	if ( ! CPropertyPageAdv::OnInitDialog() )
		return FALSE;

	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return FALSE;

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
	if ( ! Downloads.Check( pDownload ) || ! pDownload->IsTorrent() )
		return FALSE;
	
	CBTInfo& oInfo = pDownload->m_pTorrent;

	auto_ptr< CCoolTipCtrl > pTip( new CLibraryTipCtrl );
	pTip->Create( this, &Settings.Interface.TipDownloads );
	m_wndFiles.EnableTips( pTip );

	CRect rc;
	m_wndFiles.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );
	m_wndFiles.SetImageList( ShellIcons.GetObject( 16 ), LVSIL_SMALL );
	m_wndFiles.InsertColumn( 0, _T("Filename"), LVCFMT_LEFT, rc.right - 70 - 60 - 60, -1 );
	m_wndFiles.InsertColumn( 1, _T("Size"), LVCFMT_RIGHT, 70, 0 );
	m_wndFiles.InsertColumn( 2, _T("Status"), LVCFMT_RIGHT, 60, 0 );
	m_wndFiles.InsertColumn( 3, _T("Priority"), LVCFMT_RIGHT, 60, 0 );
	Skin.Translate( _T("CTorrentFileList"), m_wndFiles.GetHeaderCtrl() );

	BEGIN_COLUMN_MAP()
		COLUMN_MAP( CBTInfo::prNotWanted,	LoadString( IDS_PRIORITY_OFF ) )
		COLUMN_MAP( CBTInfo::prLow,			LoadString( IDS_PRIORITY_LOW ) )
		COLUMN_MAP( CBTInfo::prNormal,		LoadString( IDS_PRIORITY_NORMAL ) )
		COLUMN_MAP( CBTInfo::prHigh,		LoadString( IDS_PRIORITY_HIGH ) )
	END_COLUMN_MAP( m_wndFiles, 3 )

	for ( POSITION pos = oInfo.m_pFiles.GetHeadPosition(); pos ; )
	{
		CBTInfo::CBTFile* pFile = oInfo.m_pFiles.GetNext( pos );
		
		LV_ITEM pItem = {};
		pItem.mask		= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		pItem.iItem		= m_wndFiles.GetItemCount();
		pItem.lParam	= (LPARAM)pFile;
		pItem.iImage	= ShellIcons.Get( pFile->m_sPath, 16 );
		pItem.pszText	= (LPTSTR)(LPCTSTR)pFile->m_sPath;
		pItem.iItem		= m_wndFiles.InsertItem( &pItem );
		
		m_wndFiles.SetItemText( pItem.iItem, 1, Settings.SmartVolume( pFile->m_nSize ) );
		m_wndFiles.SetColumnData( pItem.iItem, 3, pFile->GetPriority() );
	}

	Update();

	SetTimer( 1, 2000, NULL );

	return TRUE;
}

BOOL CTorrentFilesPage::OnApply()
{
	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return FALSE;

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
	if ( ! Downloads.Check( pDownload ) || ! pDownload->IsTorrent() )
		return FALSE;

	CBTInfo& oInfo = pDownload->m_pTorrent;

	for ( int i = 0; i < m_wndFiles.GetItemCount(); ++i )
	{
		CBTInfo::CBTFile* pFile = (CBTInfo::CBTFile*)m_wndFiles.GetItemData( i );
		
		// Check if file still valid
		if ( POSITION pos = oInfo.m_pFiles.Find( pFile ) )
		{
			pFile->SetPriority( m_wndFiles.GetColumnData( i, 3 ) );
		}
	}

	return CPropertyPageAdv::OnApply();
}

void CTorrentFilesPage::OnTimer(UINT_PTR nIDEvent) 
{
	CPropertyPageAdv::OnTimer( nIDEvent );

	if ( static_cast< CPropertySheet* >( GetParent() )->GetActivePage() == this )
	{
		Update();
	}
}

void CTorrentFilesPage::Update()
{
	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return;

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
	if ( ! Downloads.Check( pDownload ) || ! pDownload->IsTorrent() )
		return;

	CBTInfo& oInfo = pDownload->m_pTorrent;

	for ( int i = 0; i < m_wndFiles.GetItemCount(); ++i )
	{
		CBTInfo::CBTFile* pFile = (CBTInfo::CBTFile*)m_wndFiles.GetItemData( i );

		CString sCompleted;

		// Check if file still valid
		if ( POSITION pos = oInfo.m_pFiles.Find( pFile ) )
		{
			float fProgress = pFile->GetProgress();
			if ( fProgress >= 0.0 )
				sCompleted.Format( _T("%.2f%%"), fProgress );
		}

		m_wndFiles.SetItemText( i, 2, sCompleted );
	}
}

void CTorrentFilesPage::OnDestroy() 
{
	KillTimer( 1 );

	CPropertyPageAdv::OnDestroy();
}
