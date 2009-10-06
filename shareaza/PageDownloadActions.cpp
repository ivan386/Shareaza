//
// PageDownloadActions.cpp
//
// Copyright (c) Shareaza Development Team, 2008-2009.
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
	ON_BN_CLICKED(IDC_ERASE, OnErase)
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

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
	if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) return;

	pDownload->CloseTransfers();
	QWORD nErased = pDownload->EraseRange( nFrom, nTo + 1 - nFrom );

	if ( nErased > 0 )
	{
		pDownload->ClearVerification();
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

void CDownloadActionsPage::OnForgetVerify()
{
	CString strMessage;
	LoadString( strMessage, IDS_DOWNLOAD_EDIT_FORGET_VERIFY );
	if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
	if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) return;

	pDownload->ClearVerification();
}

void CDownloadActionsPage::OnForgetSources()
{
	CString strMessage;
	LoadString( strMessage, IDS_DOWNLOAD_EDIT_FORGET_SOURCES );
	if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
	if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) return;

	pDownload->CloseTransfers();
	pDownload->ClearSources();
	pDownload->SetModified();
}

void CDownloadActionsPage::OnCompleteVerify()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
	if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) return;
	
	if ( pDownload->NeedTigerTree() && pDownload->NeedHashset() &&
		! pDownload->IsTorrent() )
	{
		pLock.Unlock();
		CString strMessage;
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_COMPLETE_NOHASH );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return;
	}
	else
	{
		pLock.Unlock();
		CString strMessage;
        LoadString( strMessage, IDS_DOWNLOAD_EDIT_COMPLETE_VERIFY );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
	}

	pLock.Lock();
	if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) return;

	pDownload->MakeComplete();
	pDownload->ResetVerification();
	pDownload->SetModified();
}

void CDownloadActionsPage::OnMergeAndVerify()
{
	CString strMessage, strFormat;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
	if ( ! Downloads.Check( pDownload ) ||
		pDownload->IsCompleted() ||
		pDownload->IsMoving() ||
		! pDownload->PrepareFile() )
	{
		// Download almost completed
		pLock.Unlock();
		return;
	}
	if ( pDownload->NeedTigerTree() &&
		 pDownload->NeedHashset() &&
		! pDownload->IsTorrent() )
	{
		// No hashsets
		pLock.Unlock();
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_COMPLETE_NOHASH );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return;
	}
	const Fragments::List oList( pDownload->GetEmptyFragmentList() );
	if ( ! oList.size() )
	{
		// No available fragments
		pLock.Unlock();
		return;
	}
	if ( ! Downloads.Check( pDownload ) || pDownload->IsTasking() )
	{
		pLock.Unlock();
		strMessage = _T("The selected download item currently has some Task attached, please wait and do it later.");
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return;
	}

	pLock.Unlock();

	// Select file
	CString strExt( PathFindExtension( pDownload->m_sName ) );
	if ( ! strExt.IsEmpty() ) strExt = strExt.Mid( 1 );
	CFileDialog dlgSelectFile( TRUE, strExt, pDownload->m_sName,
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR,
		NULL, this );
	if ( dlgSelectFile.DoModal() == IDOK )
	{
		pLock.Lock();
		if ( ! Downloads.Check( pDownload ) || pDownload->IsTasking() )
		{
			pLock.Unlock();
			strMessage = _T("The selected download item currently has some Task attached, please wait and do it later.");
			AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
			return;
		}

		CDownloadTask::MergeFile( pDownload, dlgSelectFile.GetPathName() );

		pLock.Unlock();
	}
}

void CDownloadActionsPage::OnCancelDownload()
{
	CString strMessage;
	LoadString( strMessage, IDS_DOWNLOAD_EDIT_CANCEL_DOWNLOAD );
	if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
	if ( ! Downloads.Check( pDownload ) ||
		pDownload->IsMoving() || pDownload->IsCompleted() ) return;

	pDownload->ForceComplete();
}
