//
// DlgDeleteFile.cpp
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
#include "DlgDeleteFile.h"

#include "Library.h"
#include "SharedFile.h"
#include "Download.h"

IMPLEMENT_DYNAMIC(CDeleteFileDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CDeleteFileDlg, CSkinDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_DELETE_ALL, OnDeleteAll)
	ON_BN_CLICKED(IDC_RATE_VALUE_0, OnBnClickedRateValue)
	ON_BN_CLICKED(IDC_RATE_VALUE_1, OnBnClickedRateValue)
	ON_BN_CLICKED(IDC_RATE_VALUE_2, OnBnClickedRateValue)
	ON_BN_CLICKED(IDC_RATE_VALUE_3, OnBnClickedRateValue)
END_MESSAGE_MAP()


CDeleteFileDlg::CDeleteFileDlg(CWnd* pParent) : CSkinDialog( CDeleteFileDlg::IDD, pParent )
{
	m_nRateValue = 0;
	m_bAll = FALSE;
}

CDeleteFileDlg::~CDeleteFileDlg()
{
}

void CDeleteFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_NAME, m_wndName);
	DDX_Control(pDX, IDC_RATE_COMMENTS, m_wndComments);
	DDX_Control(pDX, IDC_RATE_PROMPT, m_wndPrompt);
	DDX_Text(pDX, IDC_RATE_COMMENTS, m_sComments);
	DDX_Text(pDX, IDC_FILE_NAME, m_sName);
	DDX_Radio(pDX, IDC_RATE_VALUE_0, m_nRateValue);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_DELETE_ALL, m_wndAll);
}

BOOL CDeleteFileDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();
	
	SkinMe( NULL, ID_LIBRARY_DELETE );
	
	if ( m_bAll )
	{
		m_wndAll.EnableWindow( TRUE );
		m_bAll = FALSE;
	}
	
	return FALSE;
}

HBRUSH CDeleteFileDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSkinDialog::OnCtlColor( pDC, pWnd, nCtlColor );
	
	if ( pWnd == &m_wndName )
	{
		pDC->SelectObject( &theApp.m_gdiFontBold );
	}
	
	return hbr;
}

void CDeleteFileDlg::OnBnClickedRateValue()
{
	UpdateData();
	
	switch ( m_nRateValue )
	{
	case 0: // None
		m_sComments.Empty();
		break;
	case 1:	// Misnamed
		m_sComments = _T("Incorrectly named \"") + m_sName + _T("\"");
		break;
	case 2:	// Poor Quality
		m_sComments = _T("Very poor quality");
		break;
	case 3:	// Fake
		m_sComments = _T("Fake/corrupt");
		break;
	}
	
	m_wndComments.SetWindowText( m_sComments );
	
	BOOL bComments = ( m_nRateValue > 0 || m_sComments.GetLength() );
	
	if ( bComments != m_wndComments.IsWindowEnabled() )
	{
		m_wndPrompt.EnableWindow( bComments );
		m_wndComments.EnableWindow( bComments );
	}
	
	if ( bComments )
	{
		m_wndComments.SetFocus();
		m_wndComments.SetSel( 0, m_sComments.GetLength() );
	}
}

void CDeleteFileDlg::OnDeleteAll()
{
	if ( m_nRateValue != 1 ) m_bAll = TRUE; // Can't all if misnamed
	CDialog::OnOK();
}

void CDeleteFileDlg::Apply(CLibraryFile* pFile)
{
	if ( m_nRateValue > 0 || m_sComments.GetLength() > 0 )
	{
		if ( m_sComments.GetLength() > 0 )
			pFile->m_sComments = m_sComments;
		
		switch ( m_nRateValue )
		{
		case 1:	// Misnamed
			// pFile->m_nRating = 0;
			break;
		case 2:	// Poor Quality
			pFile->m_nRating = 2;
			break;
		case 3:	// Fake
			pFile->m_nRating = 1;
			break;
		}
		
		pFile->SaveMetadata();
	}
}

void CDeleteFileDlg::Create(CDownload* pDownload, BOOL bShare)
{
	if ( ! pDownload->m_oSHA1.IsValid() && ! pDownload->m_oTiger.IsValid() && ! pDownload->m_oED2K.IsValid() ) return;
	if ( m_sComments.IsEmpty() ) return;
	
	if ( ! Library.Lock( 500 ) ) return;
	
	CLibraryFile* pFile = NULL;
	
	if ( pFile == NULL && pDownload->m_oSHA1.IsValid() ) pFile = LibraryMaps.LookupFileBySHA1( pDownload->m_oSHA1 );
	if ( pFile == NULL && pDownload->m_oTiger.IsValid() ) pFile = LibraryMaps.LookupFileByTiger( pDownload->m_oTiger );
	if ( pFile == NULL && pDownload->m_oED2K.IsValid() ) pFile = LibraryMaps.LookupFileByED2K( pDownload->m_oED2K );
	
	if ( pFile == NULL )
	{
		pFile = new CLibraryFile( NULL, pDownload->m_sRemoteName );
		pFile->m_nSize		= pDownload->m_nSize;
		pFile->m_oSHA1		= pDownload->m_oSHA1;
		pFile->m_oTiger		= pDownload->m_oTiger;
		pFile->m_oMD5		= pDownload->m_oMD5;
		pFile->m_oED2K		= pDownload->m_oED2K;
		pFile->m_bShared	= bShare ? TS_TRUE : TS_FALSE;
		pFile->Ghost();
	}
	
	Apply( pFile );
	
	Library.Unlock( TRUE );
}
