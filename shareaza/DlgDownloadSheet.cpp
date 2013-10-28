//
// DlgDownloadSheet.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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
#include "CoolInterface.h"
#include "DlgDownloadSheet.h"
#include "Downloads.h"
#include "PageDownloadEdit.h"
#include "PageDownloadActions.h"
#include "PageTorrentGeneral.h"
#include "PageTorrentFiles.h"
#include "PageTorrentTrackers.h"
#include "Transfers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CDownloadSheet, CPropertySheetAdv)

BEGIN_MESSAGE_MAP(CDownloadSheet, CPropertySheetAdv)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadSheet

CDownloadSheet::CDownloadSheet(CSingleLock& pLock, CDownload* pDownload)
	: m_pLock			( pLock )
	, m_pDownload		( pDownload )
	, m_sDownloadTitle	( L"General" )
	, m_sActionsTitle	( L"Actions" )
	, m_sGeneralTitle	( L"Torrent" )
	, m_sFilesTitle		( L"Files" )
	, m_sTrackersTitle	( L"Trackers" )
{
	ASSUME_LOCK( Transfers.m_pSection );
}

CDownload* CDownloadSheet::GetDownload() const
{
	ASSUME_LOCK( Transfers.m_pSection );

	return ( Downloads.Check( m_pDownload ) && ! m_pDownload->IsMoving() ) ? m_pDownload : NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadSheet operations

INT_PTR CDownloadSheet::DoModal()
{
	ASSUME_LOCK( Transfers.m_pSection );

	CDownloadEditPage		pDownload;
	CDownloadActionsPage	pActions;
	CTorrentGeneralPage		pGeneral;
	CTorrentFilesPage		pFiles;
	CTorrentTrackersPage	pTrackers;

	if ( ! m_pDownload->IsMoving() && ! m_pDownload->IsCompleted() )
	{
		SetTabTitle( &pDownload, m_sDownloadTitle );
		AddPage( &pDownload );
		SetTabTitle( &pActions, m_sActionsTitle );
		AddPage( &pActions );
	}

	if ( m_pDownload->IsTorrent() )
	{
		SetTabTitle( &pGeneral, m_sGeneralTitle );
		AddPage( &pGeneral );
		SetTabTitle( &pFiles, m_sFilesTitle );
		AddPage( &pFiles );
		SetTabTitle( &pTrackers, m_sTrackersTitle );
		AddPage( &pTrackers );
	}

	return CPropertySheetAdv::DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadSheet message handlers

BOOL CDownloadSheet::OnInitDialog()
{
	BOOL bResult = CPropertySheetAdv::OnInitDialog();

	SetFont( &CoolInterface.m_fntNormal );
	SetIcon( theApp.LoadIcon( IDI_PROPERTIES ), TRUE );

	SetWindowText( LoadString( IDS_DOWNLOAD_PROPERTIES ) );

	if ( GetDlgItem( IDOK ) )
	{
		CRect rc;
		GetDlgItem( IDOK )->GetWindowRect( &rc );
		ScreenToClient( &rc );
		GetDlgItem( IDOK )->SetWindowPos( NULL, 6, rc.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE );
		GetDlgItem( IDCANCEL )->SetWindowPos( NULL, 11 + rc.Width(), rc.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE );
	}

	if ( GetDlgItem( 0x3021 ) ) GetDlgItem( 0x3021 )->ShowWindow( SW_HIDE );
	if ( GetDlgItem( 0x0009 ) ) GetDlgItem( 0x0009 )->ShowWindow( SW_HIDE );

	// Forcibly create all pages under protection of Transfers.m_pSection
	for ( int i = GetPageCount() - 1; i >= 0; --i )
	{
		SetActivePage( i );
	}

	m_pLock.Unlock();

	return bResult;
}
