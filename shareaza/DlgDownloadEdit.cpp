//
// DlgDownloadEdit.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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
#include "Download.h"
#include "Downloads.h"
#include "Transfers.h"
#include "DlgDownloadEdit.h"
#include "DlgTorrentTracker.h"

#include "SHA.h"
#include "ED2K.h"
#include "TigerTree.h"

IMPLEMENT_DYNAMIC(CDownloadEditDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CDownloadEditDlg, CSkinDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_TORRENT_INFO, OnTorrentInfo)
	ON_BN_CLICKED(IDC_ERASE, OnErase)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////////
// CDownloadEditDlg construction

CDownloadEditDlg::CDownloadEditDlg(CDownload* pDownload, CWnd* pParent) : CSkinDialog( CDownloadEditDlg::IDD, pParent )
{
	m_pDownload = pDownload;
}

CDownloadEditDlg::~CDownloadEditDlg()
{
}

void CDownloadEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NAME, m_sName);
	DDX_Text(pDX, IDC_URN_SHA1, m_sSHA1);
	DDX_Text(pDX, IDC_URN_TIGER, m_sTiger);
	DDX_Text(pDX, IDC_URN_ED2K, m_sED2K);
	DDX_Control(pDX, IDC_FORGET_VERIFY, m_wndForgetVerify);
	DDX_Control(pDX, IDC_FORGET_SOURCES, m_wndForgetSources);
	DDX_Text(pDX, IDC_ERASE_FROM, m_sEraseFrom);
	DDX_Text(pDX, IDC_ERASE_TO, m_sEraseTo);
	DDX_Control(pDX, IDC_TORRENT_INFO, m_wndTorrent);
	DDX_Control(pDX, IDC_COMPLETE_AND_VERIFY, m_wndCompleteVerify);
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadEditDlg message handlers

BOOL CDownloadEditDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();
	
	SkinMe( NULL, ID_TOOLS_DOWNLOAD );
	
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	if ( ! Downloads.Check( m_pDownload ) || m_pDownload->IsMoving() )
	{
		PostMessage( WM_CLOSE );
		return TRUE;
	}
	
	m_sName = m_pDownload->m_sRemoteName;
	
	if ( m_pDownload->m_bSHA1 )
		m_sSHA1 = CSHA::HashToString( &m_pDownload->m_pSHA1 );
	if ( m_pDownload->m_bTiger )
		m_sTiger = CTigerNode::HashToString( &m_pDownload->m_pTiger );
	if ( m_pDownload->m_bED2K )
		m_sED2K = CED2K::HashToString( &m_pDownload->m_pED2K );
	
	m_wndTorrent.EnableWindow( m_pDownload->m_pTorrent.IsAvailable() );
	
	UpdateData( FALSE );
	
	return TRUE;
}

HBRUSH CDownloadEditDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSkinDialog::OnCtlColor( pDC, pWnd, nCtlColor );
	
	if ( pWnd == &m_wndForgetVerify || pWnd == &m_wndForgetSources || pWnd == &m_wndCompleteVerify )
	{
		pDC->SelectObject( &theApp.m_gdiFontLine );
		pDC->SetTextColor( RGB( 0, 0, 255 ) );
	}
	
	return hbr;
}

BOOL CDownloadEditDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CRect rcCtrl1, rcCtrl2, rcCtrl3;
	CPoint point;
	
	GetCursorPos( &point );
    m_wndForgetVerify.GetWindowRect( &rcCtrl1 );
	m_wndForgetSources.GetWindowRect( &rcCtrl2 );
	m_wndCompleteVerify.GetWindowRect( &rcCtrl3 );
	
	if ( rcCtrl1.PtInRect( point ) || rcCtrl2.PtInRect( point ) || rcCtrl3.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}
	
	return CSkinDialog::OnSetCursor( pWnd, nHitTest, message );
}

void CDownloadEditDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CSkinDialog::OnLButtonUp(nFlags, point);
	
	CRect rcCtrl1, rcCtrl2, rcCtrl3;
	
	m_wndForgetVerify.GetWindowRect( &rcCtrl1 );
	ScreenToClient( &rcCtrl1 );
	m_wndForgetSources.GetWindowRect( &rcCtrl2 );
	ScreenToClient( &rcCtrl2 );
	m_wndCompleteVerify.GetWindowRect( &rcCtrl3 );
	ScreenToClient( &rcCtrl3 );
	
	if ( rcCtrl1.PtInRect( point ) )
	{
		if ( ! Commit() ) return;
		
		CString strMessage;
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_FORGET_VERIFY );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
		
		CSingleLock pLock( &Transfers.m_pSection, TRUE );
		if ( ! Downloads.Check( m_pDownload ) || m_pDownload->IsMoving() ) return;
		m_pDownload->ClearVerification();
	}
	else if ( rcCtrl2.PtInRect( point ) )
	{
		if ( ! Commit() ) return;
		
		CString strMessage;
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_FORGET_SOURCES );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
		
		CSingleLock pLock( &Transfers.m_pSection, TRUE );
		if ( ! Downloads.Check( m_pDownload ) || m_pDownload->IsMoving() ) return;
		
		m_pDownload->CloseTransfers();
		m_pDownload->ClearSources();
		m_pDownload->SetModified();
	}
	else if ( rcCtrl3.PtInRect( point ) )
	{
		if ( ! Commit() ) return;
		
		CSingleLock pLock( &Transfers.m_pSection, TRUE );
		CString strMessage;
		
		if ( ! Downloads.Check( m_pDownload ) || m_pDownload->IsMoving() ) return;
		
		if ( m_pDownload->NeedTigerTree() && m_pDownload->NeedHashset() && ! m_pDownload->m_bBTH )
		{
			pLock.Unlock();
			LoadString( strMessage, IDS_DOWNLOAD_EDIT_COMPLETE_NOHASH );
			AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
			return;
		}
		else
		{
			pLock.Unlock();
            LoadString( strMessage, IDS_DOWNLOAD_EDIT_COMPLETE_VERIFY );
			if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
		}
		
		pLock.Lock();
		if ( ! Downloads.Check( m_pDownload ) || m_pDownload->IsMoving() ) return;
		
		m_pDownload->MakeComplete();
		m_pDownload->ResetVerification();
		m_pDownload->SetModified();
	}
}

void CDownloadEditDlg::OnTorrentInfo()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	if ( ! Downloads.Check( m_pDownload ) ) return;
	if ( ! m_pDownload->m_pTorrent.IsAvailable() ) return;
	
	CTorrentTrackerDlg dlg( m_pDownload );
	pLock.Unlock();
	dlg.DoModal();
}

void CDownloadEditDlg::OnErase()
{
	QWORD nFrom = 0, nTo = 0;
	CString strMessage;
	
	UpdateData();
	
	if ( _stscanf( m_sEraseFrom, _T("%I64i"), &nFrom ) != 1 ||
		 _stscanf( m_sEraseTo, _T("%I64i"), &nTo ) != 1 ||
		 nTo < nFrom )
	{
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_BAD_RANGE );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return;
	}
	
	if ( ! Commit() ) return;
	
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	if ( ! Downloads.Check( m_pDownload ) || m_pDownload->IsMoving() ) return;
	
	m_pDownload->CloseTransfers();
	QWORD nErased = m_pDownload->EraseRange( nFrom, nTo + 1 - nFrom );
	
	if ( nErased > 0 )
	{
		m_pDownload->ClearVerification();
		pLock.Unlock();
		CString strFormat;
		LoadString( strFormat, IDS_DOWNLOAD_EDIT_ERASED );
		strMessage.Format( strFormat, nErased );
		AfxMessageBox( strMessage, MB_ICONINFORMATION );
	}
	else
	{
		pLock.Unlock();
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_CANT_ERASE );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
	}
}

void CDownloadEditDlg::OnOK()
{
	if ( ! Commit() ) return;
	CSkinDialog::OnOK();
}

BOOL CDownloadEditDlg::Commit()
{
	CString strMessage;
	
	UpdateData();
	
	SHA1 pSHA1;
	BOOL bSHA1 = CSHA::HashFromString( m_sSHA1, &pSHA1 );
	TIGEROOT pTiger;
	BOOL bTiger = CTigerNode::HashFromString( m_sTiger, &pTiger );
	MD4 pED2K;
	BOOL bED2K = CED2K::HashFromString( m_sED2K, &pED2K );
	
	if ( m_sSHA1.GetLength() > 0 && ! bSHA1 )
	{
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_BAD_SHA1 );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return FALSE;
	}
	else if ( m_sTiger.GetLength() > 0 && ! bTiger )
	{
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_BAD_TIGER );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return FALSE;
	}
	else if ( m_sED2K.GetLength() > 0 && ! bED2K )
	{
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_BAD_ED2K );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return FALSE;
	}
	
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
    if ( ! Downloads.Check( m_pDownload ) || m_pDownload->IsMoving() ) return FALSE;
	
	if ( m_pDownload->m_sRemoteName != m_sName )
	{
		pLock.Unlock();
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_RENAME );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		if ( ! Downloads.Check( m_pDownload ) || m_pDownload->IsMoving() ) return FALSE;
		
		m_pDownload->Rename( m_sName );
	}
	
	if ( m_pDownload->m_bSHA1 != bSHA1 || ( bSHA1 && m_pDownload->m_pSHA1 != pSHA1 ) )
	{
		pLock.Unlock();
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_CHANGE_SHA1 );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		if ( ! Downloads.Check( m_pDownload ) || m_pDownload->IsMoving() ) return FALSE;
		
		m_pDownload->m_bSHA1 = bSHA1;
		m_pDownload->m_pSHA1 = pSHA1;
		
		m_pDownload->CloseTransfers();
		m_pDownload->ClearVerification();
	}
	
	if ( m_pDownload->m_bTiger != bTiger || ( bTiger && m_pDownload->m_pTiger != pTiger ) )
	{
		pLock.Unlock();
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_CHANGE_TIGER );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		if ( ! Downloads.Check( m_pDownload ) || m_pDownload->IsMoving() ) return FALSE;
		
		m_pDownload->m_bTiger = bTiger;
		m_pDownload->m_pTiger = pTiger;
		
		m_pDownload->CloseTransfers();
		m_pDownload->ClearVerification();
	}
	
	if ( m_pDownload->m_bED2K != bED2K || ( bED2K && m_pDownload->m_pED2K != pED2K ) )
	{
		pLock.Unlock();
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_CHANGE_ED2K );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		if ( ! Downloads.Check( m_pDownload ) || m_pDownload->IsMoving() ) return FALSE;
		
		m_pDownload->m_bED2K = bED2K;
		m_pDownload->m_pED2K = pED2K;
		
		m_pDownload->CloseTransfers();
		m_pDownload->ClearVerification();
	}
	
	return TRUE;
}
