//
// DlgExistingFile.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "Settings.h"
#include "Library.h"
#include "SharedFile.h"
#include "DlgExistingFile.h"
#include "TigerTree.h"
#include "SHA.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CExistingFileDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CExistingFileDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CExistingFileDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ACTION_0, OnAction0)
	ON_BN_CLICKED(IDC_ACTION_1, OnAction1)
	ON_BN_CLICKED(IDC_ACTION_2, OnAction2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CExistingFileDlg dialog

CExistingFileDlg::CExistingFileDlg(CLibraryFile* pFile, CWnd* pParent) : CSkinDialog( CExistingFileDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CExistingFileDlg)
	m_sName = _T("");
	m_sURN = _T("");
	m_nAction = 0;
	//}}AFX_DATA_INIT

	m_sName = pFile->m_sName;

	if ( pFile->m_bSHA1 && pFile->m_bTiger )
	{
		m_sURN	= _T("bitprint:") + CSHA::HashToString( &pFile->m_pSHA1 )
				+ '.' + CTigerNode::HashToString( &pFile->m_pTiger );
	}
	else if ( pFile->m_bSHA1 )
	{
		m_sURN = CSHA::HashToString( &pFile->m_pSHA1, TRUE );
	}
	else if ( pFile->m_bTiger )
	{
		m_sURN = CTigerNode::HashToString( &pFile->m_pTiger, TRUE );
	}

	m_bAvailable	= pFile->IsAvailable();
	m_sComments		= pFile->m_sComments;
}

void CExistingFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExistingFileDlg)
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_FILE_NAME, m_wndName);
	DDX_Text(pDX, IDC_FILE_NAME, m_sName);
	DDX_Text(pDX, IDC_FILE_URN, m_sURN);
	DDX_Radio(pDX, IDC_ACTION_0, m_nAction);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_FILE_COMMENTS, m_wndComments);
	DDX_Control(pDX, IDC_MESSAGE_AVAILABLE, m_wndMessageAvailable);
	DDX_Control(pDX, IDC_MESSAGE_DELETED, m_wndMessageDeleted);
	DDX_Control(pDX, IDC_ACTION_0, m_wndLocate);
	DDX_Text(pDX, IDC_FILE_COMMENTS, m_sComments);
}

/////////////////////////////////////////////////////////////////////////////
// CExistingFileDlg message handlers

BOOL CExistingFileDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( NULL, IDR_DOWNLOADSFRAME );

	if ( ! m_bAvailable ) m_nAction = 1;
	UpdateData( FALSE );

	m_wndComments.ShowWindow( m_sComments.GetLength() > 0 ? SW_SHOW : SW_HIDE );
	m_wndMessageAvailable.ShowWindow( m_bAvailable ? SW_SHOW : SW_HIDE );
	m_wndMessageDeleted.ShowWindow( m_bAvailable ? SW_HIDE : SW_SHOW );
	m_wndLocate.EnableWindow( m_bAvailable );

	return TRUE;
}

HBRUSH CExistingFileDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSkinDialog::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( pWnd == &m_wndName || pWnd == &m_wndMessageAvailable || pWnd == &m_wndMessageDeleted )
		pDC->SelectObject( &theApp.m_gdiFontBold );
	if ( pWnd == &m_wndComments && ! m_bAvailable )
		pDC->SetTextColor( RGB( 255, 0, 0 ) );

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
	if ( ! m_bAvailable && m_nAction == 0 ) return;
	CSkinDialog::OnOK();
}
