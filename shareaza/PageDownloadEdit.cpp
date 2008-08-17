//
// PageDownloadEdit.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "PageDownloadEdit.h"

#include "Download.h"
#include "Downloads.h"
#include "DownloadTask.h"
#include "Transfers.h"
#include "FragmentedFile.h"
#include "CoolInterface.h"

#include "SHA.h"
#include "ED2K.h"
#include "TigerTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDownloadEditPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CDownloadEditPage, CPropertyPageAdv)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_ERASE, OnErase)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////////
// CDownloadEditPage construction

CDownloadEditPage::CDownloadEditPage() :
	CPropertyPageAdv( CDownloadEditPage::IDD ),
	m_bSHA1Trusted( FALSE ),
	m_bTigerTrusted( FALSE ),
	m_bED2KTrusted( FALSE )
{
}

CDownloadEditPage::~CDownloadEditPage()
{
}

void CDownloadEditPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NAME, m_sName);
	DDX_Text(pDX, IDC_DISKNAME, m_sDiskName);
	DDX_Text(pDX, IDC_FILESIZE, m_sFileSize);
	DDX_Text(pDX, IDC_URN_SHA1, m_sSHA1);
	DDX_Text(pDX, IDC_URN_TIGER, m_sTiger);
	DDX_Text(pDX, IDC_URN_ED2K, m_sED2K);
	DDX_Check(pDX, IDC_TRUST_SHA1, m_bSHA1Trusted);
	DDX_Check(pDX, IDC_TRUST_TIGER, m_bTigerTrusted);
	DDX_Check(pDX, IDC_TRUST_ED2K, m_bED2KTrusted);
	DDX_Control(pDX, IDC_FORGET_VERIFY, m_wndForgetVerify);
	DDX_Control(pDX, IDC_FORGET_SOURCES, m_wndForgetSources);
	DDX_Text(pDX, IDC_ERASE_FROM, m_sEraseFrom);
	DDX_Text(pDX, IDC_ERASE_TO, m_sEraseTo);
	DDX_Control(pDX, IDC_COMPLETE_AND_VERIFY, m_wndCompleteVerify);
	DDX_Control(pDX, IDC_MERGE_AND_VERIFY, m_wndMergeVerify);
	DDX_Control(pDX, IDC_CANCEL_DOWNLOAD, m_wndCancelDownload);
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadEditPage message handlers

BOOL CDownloadEditPage::OnInitDialog()
{
	CPropertyPageAdv::OnInitDialog();

	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;

	m_sName = pDownload->m_sName;
	m_sDiskName = pDownload->m_sPath;
	if ( pDownload->m_nSize != SIZE_UNKNOWN )
		m_sFileSize.Format( _T("%I64i"), pDownload->m_nSize );

	if ( pDownload->m_oSHA1 )
		m_sSHA1 = pDownload->m_oSHA1.toString();
	if ( pDownload->m_oTiger )
		m_sTiger = pDownload->m_oTiger.toString();
	if ( pDownload->m_oED2K )
        m_sED2K = pDownload->m_oED2K.toString();

	m_bSHA1Trusted	=	pDownload->m_bSHA1Trusted;
	m_bTigerTrusted	=	pDownload->m_bTigerTrusted;
	m_bED2KTrusted	=	pDownload->m_bED2KTrusted;

	UpdateData( FALSE );

	return TRUE;
}

HBRUSH CDownloadEditPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CDownloadEditPage::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
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

void CDownloadEditPage::OnLButtonUp(UINT nFlags, CPoint point)
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
		if ( ! Commit() ) return;

		CString strMessage;
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_FORGET_VERIFY );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

		CSingleLock pLock( &Transfers.m_pSection, TRUE );
		CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
		if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) return;

		pDownload->ClearVerification();
	}
	else if ( rcCtrl2.PtInRect( point ) )
	{
		if ( ! Commit() ) return;

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
	else if ( rcCtrl3.PtInRect( point ) )
	{
		if ( ! Commit() ) return;

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
	else if ( rcCtrl4.PtInRect( point ) )
	{
		if ( ! Commit() ) return;

		OnMergeAndVerify ();
	}
	else if ( rcCtrl5.PtInRect( point ) )
	{
		if ( ! Commit() ) return;

		CString strMessage;
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_CANCEL_DOWNLOAD );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

		CSingleLock pLock( &Transfers.m_pSection, TRUE );
		CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;
		if ( ! Downloads.Check( pDownload ) ||
			pDownload->IsMoving() || pDownload->IsCompleted() ) return;

		pDownload->ForceComplete();
	}
}

void CDownloadEditPage::OnErase()
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

void CDownloadEditPage::OnMergeAndVerify()
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

		new CDownloadTask( pDownload, CDownloadTask::dtaskMergeFile,
			dlgSelectFile.GetPathName() );

		pLock.Unlock();
	}
}

void CDownloadEditPage::OnOK()
{
	if ( ! Commit() ) return;
	CPropertyPageAdv::OnOK();
}

BOOL CDownloadEditPage::Commit()
{
	CString strMessage;
	Hashes::Sha1Hash oSHA1;
	Hashes::TigerHash oTiger;
	Hashes::Ed2kHash oED2K;

	UpdateData();
	
    oSHA1.fromString( m_sSHA1 );
    oTiger.fromString( m_sTiger );
    oED2K.fromString( m_sED2K );
	
	if ( m_sSHA1.GetLength() > 0 && !oSHA1 )
	{
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_BAD_SHA1 );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return FALSE;
	}
	else if ( m_sTiger.GetLength() > 0 && !oTiger )
	{
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_BAD_TIGER );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return FALSE;
	}
	else if ( m_sED2K.GetLength() > 0 && !oED2K )
	{
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_BAD_ED2K );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return FALSE;
	}

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownload* pDownload = ((CDownloadSheet*)GetParent())->m_pDownload;

	bool bNeedUpdate = false;
	bool bCriticalChange = false;

	bNeedUpdate	= pDownload->m_bSHA1Trusted ^ ( m_bSHA1Trusted == TRUE );
	bNeedUpdate	|= pDownload->m_bTigerTrusted ^ ( m_bTigerTrusted == TRUE );
	bNeedUpdate	|= pDownload->m_bED2KTrusted ^ ( m_bED2KTrusted == TRUE );

    if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) return FALSE;

	if ( pDownload->m_sName != m_sName )
	{
		pLock.Unlock();
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_RENAME );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) return FALSE;

		pDownload->Rename( m_sName );
		bNeedUpdate = true;
	}

	QWORD nNewSize = 0;
    if ( _stscanf( m_sFileSize, _T("%I64i"), &nNewSize ) == 1 && nNewSize != pDownload->m_nSize )
	{
		pLock.Unlock();
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_CHANGE_SIZE );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) return FALSE;
		pDownload->m_nSize = nNewSize;

		pDownload->CloseTransfers();
		pDownload->ClearVerification();
		bCriticalChange = true;
	}
	
	if ( pDownload->m_oSHA1.isValid() != oSHA1.isValid()
		|| validAndUnequal( pDownload->m_oSHA1, oSHA1 ) )
	{
		pLock.Unlock();
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_CHANGE_SHA1 );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) return FALSE;
		
		pDownload->m_oSHA1 = oSHA1;
		if ( oSHA1 ) pDownload->m_bSHA1Trusted = true;
		
		pDownload->CloseTransfers();
		pDownload->ClearVerification();
		bCriticalChange = true;
	}
	
	if ( pDownload->m_oTiger.isValid() != oTiger.isValid()
		|| validAndUnequal( pDownload->m_oTiger, oTiger ) )
	{
		pLock.Unlock();
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_CHANGE_TIGER );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) return FALSE;
		
		pDownload->m_oTiger = oTiger;
		if ( oTiger ) pDownload->m_bTigerTrusted = true;
		
		pDownload->CloseTransfers();
		pDownload->ClearVerification();
		bCriticalChange = true;
	}
	
	if ( pDownload->m_oED2K.isValid() != oED2K.isValid()
		|| validAndUnequal( pDownload->m_oED2K, oED2K ) )
	{
		pLock.Unlock();
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_CHANGE_ED2K );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		if ( ! Downloads.Check( pDownload ) || pDownload->IsMoving() ) return FALSE;
		
		pDownload->m_oED2K = oED2K;
		if ( oED2K ) pDownload->m_bED2KTrusted = true;
		
		pDownload->CloseTransfers();
		pDownload->ClearVerification();
		bCriticalChange = true;
	}

	pDownload->m_bSHA1Trusted = m_bSHA1Trusted != FALSE;
	pDownload->m_bTigerTrusted = m_bTigerTrusted != FALSE;
	pDownload->m_bED2KTrusted = m_bED2KTrusted != FALSE;

	if ( bCriticalChange )
	{
		pDownload->CloseTransfers();
		pDownload->ClearSources();
		pDownload->ClearFailedSources();
		pDownload->ClearVerification();
		bNeedUpdate = true;
	}

	if ( bNeedUpdate )
	{
		pDownload->SetModified();
	}

	return TRUE;
}
