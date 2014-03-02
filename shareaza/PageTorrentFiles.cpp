//
// PageTorrentGeneral.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
	ON_NOTIFY(NM_DBLCLK, IDC_TORRENT_FILES, &CTorrentFilesPage::OnNMDblclkTorrentFiles)
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

	ASSUME_LOCK( Transfers.m_pSection );

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	ASSERT( pDownload && pDownload->IsTorrent() );
	auto_ptr< CFragmentedFile > pFragFile( pDownload->GetFile() );
	DWORD nCount = pFragFile.get() ? pFragFile->GetCount() : 0;
	bool bCompleted = pDownload->IsCompleted() || nCount < 2;

	auto_ptr< CLibraryTipCtrl > pTip( new CLibraryTipCtrl );
	pTip->Create( this, &Settings.Interface.TipDownloads );
	m_wndFiles.EnableTips( pTip );

	CRect rc;
	m_wndFiles.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );
	ShellIcons.AttachTo( &m_wndFiles, 16 );
	m_wndFiles.InsertColumn( 0, _T("Filename"), LVCFMT_LEFT, rc.right - 70 - 60
		- ( bCompleted ? 0 : 60 ), -1 );
	m_wndFiles.InsertColumn( 1, _T("Size"), LVCFMT_RIGHT, 70, 0 );
	m_wndFiles.InsertColumn( 2, _T("Status"), LVCFMT_RIGHT, 60, 0 );
	if ( ! bCompleted )
		m_wndFiles.InsertColumn( 3, _T("Priority"), LVCFMT_RIGHT, 60, 0 );
	Skin.Translate( _T("CTorrentFileList"), m_wndFiles.GetHeaderCtrl() );

	if ( ! bCompleted )
	{
		BEGIN_COLUMN_MAP()
			COLUMN_MAP( CFragmentedFile::prNotWanted,	LoadString( IDS_PRIORITY_OFF ) )
			COLUMN_MAP( CFragmentedFile::prLow,			LoadString( IDS_PRIORITY_LOW ) )
			COLUMN_MAP( CFragmentedFile::prNormal,		LoadString( IDS_PRIORITY_NORMAL ) )
			COLUMN_MAP( CFragmentedFile::prHigh,		LoadString( IDS_PRIORITY_HIGH ) )
		END_COLUMN_MAP( m_wndFiles, 3 )
	}

	for ( DWORD i = 0; i < nCount; ++i )
	{
		LV_ITEM pItem = {};
		pItem.mask		= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		pItem.iItem		= i;
		pItem.lParam	= (LPARAM)pFragFile->GetAt( i );
		pItem.iImage	= ShellIcons.Get( pFragFile->GetName( i ), 16 );
		pItem.pszText	= (LPTSTR)(LPCTSTR)pFragFile->GetName( i );
		pItem.iItem		= m_wndFiles.InsertItem( &pItem );
		m_wndFiles.SetItemText( pItem.iItem, 1, Settings.SmartVolume( pFragFile->GetLength( i ) ) );
		if ( ! bCompleted )
			m_wndFiles.SetColumnData( pItem.iItem, 3, pFragFile->GetPriority( i ) );
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

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	if ( ! pDownload )
		// Invalid download
		return CPropertyPageAdv::OnApply();

	if ( ! pDownload->IsCompleted() )
	{
		auto_ptr< CFragmentedFile > pFragFile( pDownload->GetFile() );
		if ( pFragFile.get() )
		{
			DWORD nCount = pFragFile->GetCount();
			if ( nCount > 1 )
			{
				for ( DWORD i = 0; i < nCount; ++i )
				{
					pFragFile->SetPriority( i, m_wndFiles.GetColumnData( i, 3 ) );
				}
			}
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

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	if ( ! pDownload )
		// Invalid download
		return;

	auto_ptr< CFragmentedFile > pFragFile( pDownload->GetFile() );
	if ( pFragFile.get() )
	{
		for ( DWORD i = 0 ; i < pFragFile->GetCount() ; ++i )
		{
			CString sCompleted;
			float fProgress = pFragFile->GetProgress( i );
			if ( fProgress >= 0.0 )
				sCompleted.Format( _T("%.2f%%"), fProgress );
			m_wndFiles.SetItemText( i, 2, sCompleted );
		}
	}
}

void CTorrentFilesPage::OnDestroy() 
{
	KillTimer( 1 );

	CPropertyPageAdv::OnDestroy();
}

void CTorrentFilesPage::OnNMDblclkTorrentFiles(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return;

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	if ( ! pDownload )
		// Invalid download
		return;

	pDownload->Launch( pNMItemActivate->iItem, &oLock, FALSE );
}
