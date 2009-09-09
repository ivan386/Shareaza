//
// DlgExistingFile.cpp
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
#include "Settings.h"
#include "CoolInterface.h"
#include "Library.h"
#include "SharedFile.h"
#include "DlgExistingFile.h"
#include "WndMain.h"
#include "WndLibrary.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CExistingFileDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CExistingFileDlg, CSkinDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ACTION_0, OnAction0)
	ON_BN_CLICKED(IDC_ACTION_1, OnAction1)
	ON_BN_CLICKED(IDC_ACTION_2, OnAction2)
END_MESSAGE_MAP()


CExistingFileDlg::Action CExistingFileDlg::CheckExisting(const CShareazaFile* pFile)
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 1000 ) )
		return Download;

	CLibraryFile* pLibFile = LibraryMaps.LookupFileByHash( pFile );
	if ( pLibFile == NULL )
		return Download;

	DWORD nIndex = pLibFile->m_nIndex;

	CExistingFileDlg dlg( pLibFile );

	pLock.Unlock();

	dlg.DoModal();

	if ( dlg.m_nAction == 0 )
	{
		if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
		{	
			if ( CLibraryWnd* pLibrary = (CLibraryWnd*)(
				pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CLibraryWnd) ) ) )
			{
				pLock.Lock();
				if ( CLibraryFile* pLibFile = Library.LookupFile( nIndex ) )
				{
					pLibrary->Display( pLibFile );
				}
				pLock.Unlock();
			}
		}
	}

	return dlg.GetResult();
}

/////////////////////////////////////////////////////////////////////////////
// CExistingFileDlg dialog

CExistingFileDlg::CExistingFileDlg(CLibraryFile* pFile, CWnd* pParent,
	 bool bDuplicateSearch)
:	CSkinDialog( CExistingFileDlg::IDD, pParent )
,	m_nAction( 0 )
{
	m_sName = pFile->m_sName;

	if ( !bDuplicateSearch )
	{
		if ( pFile->m_oSHA1 && pFile->m_oTiger )
		{
			m_sURN	= _T("bitprint:") + pFile->m_oSHA1.toString()
				+ '.' + pFile->m_oTiger.toString();
		}
		else if ( pFile->m_oSHA1 )
		{
			m_sURN = pFile->m_oSHA1.toString();
		}
		else if ( pFile->m_oTiger )
		{
			m_sURN = pFile->m_oTiger.toUrn();
		}

		m_sComments		= pFile->m_sComments;
		m_bAvailable	= pFile->IsAvailable() ? TRI_FALSE : TRI_UNKNOWN;
	}
	else if ( pFile->m_oMD5 )
	{
		m_sURN = pFile->m_oMD5.toUrn();
		m_bAvailable	= TRI_TRUE;
	}
}

INT_PTR CExistingFileDlg::DoModal()
{
	INT_PTR ret = CSkinDialog::DoModal();
	if ( ret != IDOK )
		m_nAction = 3;
	return ret;
}

void CExistingFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_FILE_NAME, m_wndName);
	DDX_Text(pDX, IDC_FILE_NAME, m_sName);
	DDX_Text(pDX, IDC_FILE_URN, m_sURN);
	DDX_Radio(pDX, IDC_ACTION_0, m_nAction);
	DDX_Control(pDX, IDC_FILE_COMMENTS, m_wndComments);
	DDX_Control(pDX, IDC_MESSAGE_AVAILABLE, m_wndMessageAvailable);
	DDX_Control(pDX, IDC_MESSAGE_DELETED, m_wndMessageDeleted);
	DDX_Control(pDX, IDC_MESSAGE_DUPLICATES, m_wndMessageDuplicates);
	DDX_Control(pDX, IDC_ACTION_0, m_wndLocate);
	DDX_Control(pDX, IDC_ACTION_1, m_wndDownload);
	DDX_Control(pDX, IDC_ACTION_2, m_wndDontDownload);
	DDX_Text(pDX, IDC_FILE_COMMENTS, m_sComments);
}

/////////////////////////////////////////////////////////////////////////////
// CExistingFileDlg message handlers

BOOL CExistingFileDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( NULL, IDR_DOWNLOADSFRAME );

	if ( m_bAvailable == TRI_UNKNOWN ) 
		m_nAction = 1;
	else if ( m_bAvailable == TRI_TRUE )
		m_nAction = 0;

	UpdateData( FALSE );

	m_wndComments.ShowWindow( m_sComments.GetLength() > 0 ? SW_SHOW : SW_HIDE );
	m_wndMessageAvailable.ShowWindow( m_bAvailable == TRI_FALSE ? SW_SHOW : SW_HIDE );
	m_wndMessageDeleted.ShowWindow( m_bAvailable != TRI_UNKNOWN ? SW_HIDE : SW_SHOW );
	m_wndMessageDuplicates.ShowWindow( m_bAvailable == TRI_TRUE ? SW_SHOW : SW_HIDE );
	m_wndDownload.ShowWindow( m_bAvailable == TRI_TRUE ? SW_HIDE : SW_SHOW );
	m_wndDontDownload.ShowWindow( m_bAvailable == TRI_TRUE ? SW_HIDE : SW_SHOW );
	m_wndLocate.EnableWindow( m_bAvailable != TRI_UNKNOWN );

	return TRUE;
}

HBRUSH CExistingFileDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSkinDialog::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( pWnd == &m_wndName || pWnd == &m_wndMessageAvailable || pWnd == &m_wndMessageDeleted )
		pDC->SelectObject( &theApp.m_gdiFontBold );
	if ( pWnd == &m_wndComments && m_bAvailable == TRI_UNKNOWN )
		pDC->SetTextColor( CoolInterface.m_crTextAlert );
	if ( pWnd == &m_wndMessageDuplicates )
		pDC->SetTextColor( CoolInterface.m_crTextAlert );

	return hbr;
}

void CExistingFileDlg::OnAction0()
{
	if ( m_wndLocate.IsWindowEnabled() ) m_wndOK.EnableWindow( TRUE );
}

void CExistingFileDlg::OnAction1()
{
	m_wndOK.EnableWindow( TRUE );
}

void CExistingFileDlg::OnAction2()
{
	m_wndOK.EnableWindow( TRUE );
}

void CExistingFileDlg::OnOK()
{
	UpdateData();
	if ( m_bAvailable == TRI_UNKNOWN && m_nAction == 0 ) return;
	CSkinDialog::OnOK();
}
