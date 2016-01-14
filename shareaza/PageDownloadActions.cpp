//
// PageDownloadActions.cpp
//
// Copyright (c) Shareaza Development Team, 2008-2013.
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
#include "DlgDownloadSheet.h"
#include "PageDownloadActions.h"

#include "Download.h"
#include "Downloads.h"
#include "DownloadTask.h"
#include "Transfers.h"
#include "FragmentedFile.h"
#include "CoolInterface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDownloadActionsPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CDownloadActionsPage, CPropertyPageAdv)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_ERASE, &CDownloadActionsPage::OnErase)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////////
// CDownloadActionsPage construction

CDownloadActionsPage::CDownloadActionsPage() :
	CPropertyPageAdv( CDownloadActionsPage::IDD )
{
}

CDownloadActionsPage::~CDownloadActionsPage()
{
}

void CDownloadActionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FORGET_VERIFY, m_wndForgetVerify);
	DDX_Control(pDX, IDC_FORGET_SOURCES, m_wndForgetSources);
	DDX_Text(pDX, IDC_ERASE_FROM, m_sEraseFrom);
	DDX_Text(pDX, IDC_ERASE_TO, m_sEraseTo);
	DDX_Control(pDX, IDC_COMPLETE_AND_VERIFY, m_wndCompleteVerify);
	DDX_Control(pDX, IDC_MERGE_AND_VERIFY, m_wndMergeVerify);
	DDX_Control(pDX, IDC_CANCEL_DOWNLOAD, m_wndCancelDownload);
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadActionsPage message handlers

BOOL CDownloadActionsPage::OnInitDialog()
{
	if ( ! CPropertyPageAdv::OnInitDialog() )
		return FALSE;

	ASSUME_LOCK( Transfers.m_pSection );

	return TRUE;
}

HBRUSH CDownloadActionsPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPageAdv::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( pWnd == &m_wndForgetVerify ||
		 pWnd == &m_wndForgetSources ||
		 pWnd == &m_wndCompleteVerify ||
		 pWnd == &m_wndMergeVerify ||
		 pWnd == &m_wndCancelDownload )
	{
		pDC->SelectObject( &theApp.m_gdiFontLine );
		pDC->SetTextColor( CoolInterface.m_crTextLink );
	}

	return hbr;
}

BOOL CDownloadActionsPage::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CRect rcCtrl1, rcCtrl2, rcCtrl3, rcCtrl4, rcCtrl5;
	CPoint point;

	GetCursorPos( &point );
    m_wndForgetVerify.GetWindowRect( &rcCtrl1 );
	m_wndForgetSources.GetWindowRect( &rcCtrl2 );
	m_wndCompleteVerify.GetWindowRect( &rcCtrl3 );
	m_wndMergeVerify.GetWindowRect( &rcCtrl4 );
	m_wndCancelDownload.GetWindowRect( &rcCtrl5 );

	if ( rcCtrl1.PtInRect( point ) ||
		 rcCtrl2.PtInRect( point ) ||
		 rcCtrl3.PtInRect( point ) ||
		 rcCtrl4.PtInRect( point ) ||
		 rcCtrl5.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}

	return CPropertyPageAdv::OnSetCursor( pWnd, nHitTest, message );
}

void CDownloadActionsPage::OnLButtonUp(UINT nFlags, CPoint point)
{
	CPropertyPageAdv::OnLButtonUp(nFlags, point);

	CRect rcCtrl1, rcCtrl2, rcCtrl3, rcCtrl4, rcCtrl5;

	m_wndForgetVerify.GetWindowRect( &rcCtrl1 );
	ScreenToClient( &rcCtrl1 );
	m_wndForgetSources.GetWindowRect( &rcCtrl2 );
	ScreenToClient( &rcCtrl2 );
	m_wndCompleteVerify.GetWindowRect( &rcCtrl3 );
	ScreenToClient( &rcCtrl3 );
	m_wndMergeVerify.GetWindowRect( &rcCtrl4 );
	ScreenToClient( &rcCtrl4 );
	m_wndCancelDownload.GetWindowRect( &rcCtrl5 );
	ScreenToClient( &rcCtrl5 );

	if ( rcCtrl1.PtInRect( point ) )
	{
		OnForgetVerify();
	}
	else if ( rcCtrl2.PtInRect( point ) )
	{
		OnForgetSources();
	}
	else if ( rcCtrl3.PtInRect( point ) )
	{
		OnCompleteVerify();
	}
	else if ( rcCtrl4.PtInRect( point ) )
	{
		OnMergeAndVerify();
	}
	else if ( rcCtrl5.PtInRect( point ) )
	{
		OnCancelDownload();
	}
}

void CDownloadActionsPage::OnErase()
{
	QWORD nFrom = 0, nTo = 0;
	if ( ! UpdateData() ||
		 _stscanf( m_sEraseFrom, _T("%I64u"), &nFrom ) != 1 ||
		 _stscanf( m_sEraseTo, _T("%I64u"), &nTo ) != 1 ||
		 nTo < nFrom )
	{
		AfxMessageBox( IDS_DOWNLOAD_EDIT_BAD_RANGE, MB_ICONEXCLAMATION );
		return;
	}

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
	{
		return;
	}

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		AfxMessageBox( IDS_DOWNLOAD_EDIT_BUSY, MB_ICONEXCLAMATION );
		return;
	}

	pDownload->CloseTransfers();
	QWORD nErased = pDownload->EraseRange( nFrom, nTo + 1 - nFrom );

	if ( nErased > 0 )
	{
		pDownload->ClearVerification();
		pLock.Unlock();
		CString strMessage;
		strMessage.Format( LoadString( IDS_DOWNLOAD_EDIT_ERASED ), nErased );
		AfxMessageBox( strMessage, MB_ICONINFORMATION );
	}
	else
	{
		pLock.Unlock();
		AfxMessageBox( IDS_DOWNLOAD_EDIT_CANT_ERASE, MB_ICONEXCLAMATION );
	}
}

void CDownloadActionsPage::OnForgetVerify()
{
	if ( AfxMessageBox( IDS_DOWNLOAD_EDIT_FORGET_VERIFY, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
	{
		return;
	}

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		AfxMessageBox( IDS_DOWNLOAD_EDIT_BUSY, MB_ICONEXCLAMATION );
		return;
	}

	pDownload->ClearVerification();
}

void CDownloadActionsPage::OnForgetSources()
{
	if ( AfxMessageBox( IDS_DOWNLOAD_EDIT_FORGET_SOURCES, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
	{
		return;
	}

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		AfxMessageBox( IDS_DOWNLOAD_EDIT_BUSY, MB_ICONEXCLAMATION );
		return;
	}

	pDownload->CloseTransfers();
	pDownload->ClearSources();
	pDownload->SetModified();
}

void CDownloadActionsPage::OnCompleteVerify()
{
	if ( AfxMessageBox( IDS_DOWNLOAD_EDIT_COMPLETE_VERIFY, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
	{
		return;
	}

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		AfxMessageBox( IDS_DOWNLOAD_EDIT_BUSY, MB_ICONEXCLAMATION );
		return;
	}

	if (  pDownload->NeedTigerTree() &&
		  pDownload->NeedHashset() &&
		! pDownload->IsTorrent() )
	{
		pLock.Unlock();
		AfxMessageBox( IDS_DOWNLOAD_EDIT_COMPLETE_NOHASH, MB_ICONEXCLAMATION );
		return;
	}

	pDownload->MakeComplete();
	pDownload->ResetVerification();
	pDownload->SetModified();
}

void CDownloadActionsPage::OnMergeAndVerify()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
	{
		return;
	}

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		AfxMessageBox( IDS_DOWNLOAD_EDIT_BUSY, MB_ICONEXCLAMATION );
		return;
	}

	if ( pDownload->IsCompleted() || ! pDownload->PrepareFile() )
	{
		// Download almost completed
		return;
	}

	const Fragments::List oList( pDownload->GetEmptyFragmentList() );
	if ( ! oList.size() )
	{
		// No available fragments
		return;
	}

	CString sName = pDownload->m_sName;

	pLock.Unlock();

	// Select file
	CString strExt( PathFindExtension( sName ) );
	if ( ! strExt.IsEmpty() ) strExt = strExt.Mid( 1 );

	CFileDialog dlgSelectFile( TRUE, strExt, sName,
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR |
		OFN_ALLOWMULTISELECT, NULL, this );

	CAutoVectorPtr< TCHAR >szFiles( new TCHAR[ 2048 ] );
	if ( ! szFiles )
		// Out of memory
		return;

	*szFiles = 0;
	dlgSelectFile.GetOFN().lpstrFile = szFiles;
	dlgSelectFile.GetOFN().nMaxFile = 2048;

	if ( dlgSelectFile.DoModal() != IDOK )
	{
		return;
	}

	pLock.Lock();
	pDownload = pSheet->GetDownload();
	if ( ! pDownload )
	{
		return;
	}

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		AfxMessageBox( IDS_DOWNLOAD_EDIT_BUSY, MB_ICONEXCLAMATION );
		return;
	}

	CList< CString > oFiles;
	CString sFolder = (LPCTSTR)szFiles;
	for ( LPCTSTR szFile = szFiles; *szFile ; )
	{
		szFile += _tcslen( szFile ) + 1;
		if ( *szFile )
			// Folder + files
			oFiles.AddTail( sFolder + _T("\\") + szFile );
		else
			// Single file
			oFiles.AddTail( sFolder );
	}

	if ( oFiles.GetCount() )
	{
		pDownload->MergeFile( &oFiles );
	}
}

void CDownloadActionsPage::OnCancelDownload()
{
	if ( AfxMessageBox( IDS_DOWNLOAD_EDIT_CANCEL_DOWNLOAD, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	if ( ! pDownload )
	{
		return;
	}

	if ( pDownload->IsTasking() )
	{
		pLock.Unlock();
		AfxMessageBox( IDS_DOWNLOAD_EDIT_BUSY, MB_ICONEXCLAMATION );
		return;
	}

	if ( pDownload->IsCompleted() )
	{
		// Download almost completed
		return;
	}

	pDownload->ForceComplete();
}
