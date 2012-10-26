//
// DlgDonkeyImport.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "DlgDonkeyImport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CDonkeyImportDlg, CSkinDialog)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CLOSE, OnClose)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDonkeyImportDlg dialog

CDonkeyImportDlg::CDonkeyImportDlg(CWnd* pParent /*=NULL*/)
	: CSkinDialog(CDonkeyImportDlg::IDD, pParent)
{
}

void CDonkeyImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CLOSE, m_wndClose);
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDC_IMPORT, m_wndImport);
	DDX_Control(pDX, IDC_LOG, m_wndLog);
}

/////////////////////////////////////////////////////////////////////////////
// CDonkeyImportDlg message handlers

BOOL CDonkeyImportDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CDonkeyImportDlg"), IDR_MAINFRAME );

	CString str;
	m_wndCancel.GetWindowText( str );
	int nPos = str.Find( '|' );
	if ( nPos > 0 )
	{
		m_sCancel = str.Mid( nPos + 1 );
		m_wndCancel.SetWindowText( str.Left( nPos ) );
	}

	return TRUE;
}

void CDonkeyImportDlg::OnImport()
{
	m_wndImport.EnableWindow( FALSE );
	m_wndCancel.SetWindowText( m_sCancel );
	m_pImporter.Start( &m_wndLog );
	SetTimer( 1, 1000, NULL );
}

void CDonkeyImportDlg::OnCancel()
{
	m_pImporter.Stop();
	CSkinDialog::OnCancel();
}

void CDonkeyImportDlg::OnTimer(UINT_PTR /*nIDEvent*/)
{
	if ( ! m_pImporter.IsThreadAlive() )
	{
		KillTimer( 1 );
		m_wndCancel.EnableWindow( FALSE );
		m_wndClose.ModifyStyle( 0, BS_DEFPUSHBUTTON );
		m_wndClose.ShowWindow( SW_SHOW );
		m_wndClose.SetFocus();
	}
}

void CDonkeyImportDlg::OnClose()
{
	EndDialog( IDOK );
}
