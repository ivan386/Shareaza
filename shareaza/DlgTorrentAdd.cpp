//
// DlgTorrentAdd.cpp
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
#include "Network.h"
#include "Library.h"
#include "SharedFile.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "ShareazaURL.h"
#include "HttpRequest.h"
#include "DlgTorrentAdd.h"
#include "WndMain.h"
#include "WndDownloads.h"
#include "DlgHelp.h"
#include "DownloadTask.h"
#include "FragmentedFile.h"
#include "LibraryHistory.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CTorrentAddDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CTorrentAddDlg, CSkinDialog)
	ON_BN_CLICKED( IDOK, &CTorrentAddDlg::OnOk )
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED( IDC_COMPLETE_BROWSE, OnCompleteBrowse )
	ON_BN_CLICKED( IDC_TEMPORARY_BROWSE, OnTemporaryBrowse )
END_MESSAGE_MAP()

const DWORD BUFFER_SIZE = 2 * 1024 * 1024u;


/////////////////////////////////////////////////////////////////////////////
// CTorrentAddDlg construction

CTorrentAddDlg::CTorrentAddDlg(wstring& sTempFolder, wstring& sCompleteFolder, bool& bUseTemp, bool& bStartPaused, bool& bManaged, LtHook::bit::allocations& nAllocationType) :
	CSkinDialog( CTorrentAddDlg::IDD, pParent )
,	m_sTempFolder( sTempFolder )
,   m_sCompleteFolder( sCompleteFolder )
,	m_bUseTemp( bUseTemp )
,   m_bStartPaused( bStartPaused )
,   m_bManaged( bManaged )
,   m_nAllocationType( nAllocationType )
{
}

void CTorrentAddDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDOK, m_wndOK );
	DDX_Control( pDX, IDCANCEL, m_wndCancel );
    DDX_Control( pDX, IDC_CHECK_TEMP, m_bUseTemp );
    DDX_Control( pDX, IDC_CHECK_MANAGED, m_bManaged );
    DDX_Control( pDX, IDC_CHECK_PAUSED, m_bStartPaused );
    DDX_Text( pDX, IDC_EDIT_SAVE, m_sCompleteFolder );
    DDX_Text( pDX, IDC_EDIT_TEMP, m_sTempFolder );
    DDX_Control( pDX, IDC_COMPLETE_BROWSE, m_wndCompletePath );
    DDX_Control( pDX, IDC_TEMPORARY_BROWSE, m_wndTempPath );
	DDX_Control( pDX, IDC_COMBO_ALLOCATION, m_wndAllocation );
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentAddDlg message handlers

BOOL CTorrentAddDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( NULL, IDR_MAINFRAME );

	if ( Settings.General.LanguageRTL ) m_wndProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	m_wndProgress.SetRange( 0, 1000 );
	m_wndProgress.SetPos( 0 );

	m_wndCompletePath.SetIcon( IDI_BROWSE );
	m_wndTempPath.SetIcon( IDI_BROWSE );

	// if ( m_bForceSeed ) m_wndDownload.ShowWindow( SW_HIDE );

	return TRUE;
}

void CTorrentAddDlg::OnOk()
{

}

void CTorrentAddDlg::OnCancel()
{
	if ( m_wndDownload.IsWindowEnabled() || m_bCancel )
	{
		CSkinDialog::OnCancel();
	}
	else
	{
		m_bCancel = TRUE;
	}
}

void CTorrentAddDlg::OnDestroy()
{
	m_bCancel = TRUE;
	CloseThread();

	CSkinDialog::OnDestroy();
}

void CTorrentAddDlg::OnTimer(UINT_PTR nIDEvent)
{

}

/////////////////////////////////////////////////////////////////////////////
// CTorrentAddDlg thread run

void CTorrentAddDlg::OnRun()
{
	if ( CreateDownload() )
	{
		PostMessage( WM_TIMER, 1 );
	}
	else
	{
		PostMessage( WM_TIMER, 2 );
	}
}

void CTorrentAddDlg::OnCompleteBrowse()
{

}

void CTorrentAddDlg::OnTemporaryBrowse()
{

}