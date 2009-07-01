//
// DlgDownloadSheet.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

#include "PageDownloadEdit.h"
#include "PageDownloadActions.h"
#include "PageTorrentGeneral.h"
#include "PageTorrentFiles.h"
#include "PageTorrentTrackers.h"

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

CDownloadSheet::CDownloadSheet(CDownload* pDownload) : 
	m_pDownload( pDownload ),
	m_sDownloadTitle( L"General" ),
	m_sActionsTitle( L"Actions" ),
	m_sGeneralTitle( L"Torrent" ),
	m_sFilesTitle( L"Files" ),
	m_sTrackersTitle( L"Trackers" )
{
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadSheet operations

INT_PTR CDownloadSheet::DoModal(int nPage)
{
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

	m_psh.nStartPage = nPage;

	return CPropertySheetAdv::DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadSheet message handlers

BOOL CDownloadSheet::OnInitDialog()
{
	BOOL bResult = CPropertySheetAdv::OnInitDialog();

	SetFont( &CoolInterface.m_fntNormal );
	SetIcon( theApp.LoadIcon( IDI_PROPERTIES ), TRUE );

	CString strCaption;
	LoadString( strCaption, IDS_DOWNLOAD_PROPERTIES );
	SetWindowText( strCaption );

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

	return bResult;
}
