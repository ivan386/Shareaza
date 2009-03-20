//
// DlgMessage.cpp
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

#include "stdafx.h"
#include "resource.h"
#include "DlgSkinDialog.h"
#include "DlgMessage.h"
#include "Skin.h"


// CMessageDlg dialog

IMPLEMENT_DYNAMIC(CMessageDlg, CSkinDialog)

CMessageDlg::CMessageDlg(CWnd* pParent /*=NULL*/)
	: CSkinDialog( CMessageDlg::IDD, pParent )
	, m_nType( MB_OK )
	, m_bRemember( FALSE )
	, m_pnDefault( NULL )
{
}

void CMessageDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INFO_ICON, m_Icon);
	DDX_Text(pDX, IDC_INFO_TEXT, m_sText);
	DDX_Check(pDX, IDC_INFO_REMEMBER, m_bRemember);
}

BOOL CMessageDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CMessageDlg") );

	CWnd* pSplit = GetDlgItem( IDC_INFO_SPLIT );
	CWnd* pDefault = GetDlgItem( IDC_INFO_REMEMBER );
	pSplit->ShowWindow( m_pnDefault ? SW_NORMAL : SW_HIDE );
	pDefault->ShowWindow( m_pnDefault ? SW_NORMAL : SW_HIDE );
	if ( m_pnDefault == NULL )
	{
		// Resize window
		CRect rcWindow, rcSplit;
		GetWindowRect( &rcWindow );
		pSplit->GetWindowRect( &rcSplit );
		SetWindowPos( NULL, 0, 0, rcWindow.Width(), rcWindow.Height() -
			( rcWindow.bottom - rcSplit.top ),
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
	}

	switch ( m_nType & MB_ICONMASK )
	{
	case MB_ICONHAND:
		m_Icon.SetIcon( ::LoadIcon( NULL, IDI_HAND ) );
		break;
	case MB_ICONQUESTION:
		m_Icon.SetIcon( ::LoadIcon( NULL, IDI_QUESTION ) );
		break;
	case MB_ICONEXCLAMATION:
		m_Icon.SetIcon( ::LoadIcon( NULL, IDI_EXCLAMATION ) );
		break;
	case MB_ICONASTERISK:
		m_Icon.SetIcon( ::LoadIcon( NULL, IDI_ASTERISK ) );
		break;
	}

	CButton* pButton1 = static_cast< CButton* >( GetDlgItem( IDC_INFO_BUTTON1 ) );
	CButton* pButton2 = static_cast< CButton* >( GetDlgItem( IDC_INFO_BUTTON2 ) );
	CButton* pButton3 = static_cast< CButton* >( GetDlgItem( IDC_INFO_BUTTON3 ) );

	switch ( m_nType & MB_TYPEMASK )
	{
	case MB_OK:
		pButton1->SetWindowText( _T("Ok") );
		pButton1->ShowWindow( SW_NORMAL );
		pButton2->ShowWindow( SW_HIDE );
		pButton3->ShowWindow( SW_HIDE );
		break;
	case MB_OKCANCEL:
		pButton1->SetWindowText( _T("Cancel") );
		pButton1->ShowWindow( SW_NORMAL );
		pButton2->SetWindowText( _T("Ok") );
		pButton2->ShowWindow( SW_NORMAL );
		pButton3->ShowWindow( SW_HIDE );
		break;
	case MB_ABORTRETRYIGNORE:
		pButton1->SetWindowText( _T("Ignore") );
		pButton1->ShowWindow( SW_NORMAL );
		pButton2->SetWindowText( _T("Retry") );
		pButton2->ShowWindow( SW_NORMAL );
		pButton3->SetWindowText( _T("Abort") );
		pButton3->ShowWindow( SW_NORMAL );
		break;
	case MB_YESNOCANCEL:
		pButton1->SetWindowText( _T("Cancel") );
		pButton1->ShowWindow( SW_NORMAL );
		pButton2->SetWindowText( _T("No") );
		pButton2->ShowWindow( SW_NORMAL );
		pButton3->SetWindowText( _T("Yes") );
		pButton3->ShowWindow( SW_NORMAL );
		break;
	case MB_YESNO:
		pButton1->SetWindowText( _T("No") );
		pButton1->ShowWindow( SW_NORMAL );
		pButton2->SetWindowText( _T("Yes") );
		pButton2->ShowWindow( SW_NORMAL );
		pButton3->ShowWindow( SW_HIDE );
		break;
	case MB_RETRYCANCEL:
		pButton1->SetWindowText( _T("Cancel") );
		pButton1->ShowWindow( SW_NORMAL );
		pButton2->SetWindowText( _T("Retry") );
		pButton2->ShowWindow( SW_NORMAL );
		pButton3->ShowWindow( SW_HIDE );
		break;
	case MB_CANCELTRYCONTINUE:
		pButton1->SetWindowText( _T("Continue") );
		pButton1->ShowWindow( SW_NORMAL );
		pButton2->SetWindowText( _T("Try Again") );
		pButton2->ShowWindow( SW_NORMAL );
		pButton3->SetWindowText( _T("Cancel") );
		pButton3->ShowWindow( SW_NORMAL );
		break;
	}

	return TRUE;
}

void CMessageDlg::OnOK()
{
}

void CMessageDlg::OnCancel()
{
}

BEGIN_MESSAGE_MAP(CMessageDlg, CSkinDialog)
	ON_BN_CLICKED(IDC_INFO_BUTTON1, &CMessageDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_INFO_BUTTON2, &CMessageDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_INFO_BUTTON3, &CMessageDlg::OnBnClickedButton3)
END_MESSAGE_MAP()

// CMessageDlg message handlers

void CMessageDlg::OnBnClickedButton1()
{
	UpdateData();

	switch ( m_nType & MB_TYPEMASK )
	{
	case MB_OK:
		EndDialog( IDOK );
		break;
	case MB_OKCANCEL:
	case MB_YESNOCANCEL:
	case MB_RETRYCANCEL:
		EndDialog( IDCANCEL );
		break;
	case MB_ABORTRETRYIGNORE:
		EndDialog( IDIGNORE );
		break;
	case MB_YESNO:
		EndDialog( IDNO );
		break;
	case MB_CANCELTRYCONTINUE:
		EndDialog( IDCONTINUE );
		break;
	}
}

void CMessageDlg::OnBnClickedButton2()
{
	UpdateData();

	switch ( m_nType & MB_TYPEMASK )
	{
	case MB_OKCANCEL:
		EndDialog( IDOK );
		break;
	case MB_ABORTRETRYIGNORE:
	case MB_RETRYCANCEL:
		EndDialog( IDRETRY );
		break;
	case MB_YESNOCANCEL:
		EndDialog( IDNO );
		break;
	case MB_YESNO:
		EndDialog( IDYES );
		break;
	case MB_CANCELTRYCONTINUE:
		EndDialog( IDTRYAGAIN );
		break;
	}
}

void CMessageDlg::OnBnClickedButton3()
{
	UpdateData();

	switch ( m_nType & MB_TYPEMASK )
	{
	case MB_ABORTRETRYIGNORE:
		EndDialog( IDABORT );
		break;
	case MB_YESNOCANCEL:
		EndDialog( IDYES );
		break;
	case MB_CANCELTRYCONTINUE:
		EndDialog( IDCANCEL );
		break;
	}
}

INT_PTR CMessageDlg::DoModal()
{
	if ( m_pnDefault && *m_pnDefault != 0 )
	{
		switch ( m_nType & MB_TYPEMASK )
		{
		case MB_OK:
			// 0 - ask, 1 - ok
			if ( *m_pnDefault == 1 )
				return IDOK;
			break;
		case MB_OKCANCEL:
			// 0 - ask, 1 - ok, 2 - cancel
			if ( *m_pnDefault == 1 )
				return IDOK;
			else if ( *m_pnDefault == 2 )
				return IDCANCEL;
			break;
		case MB_ABORTRETRYIGNORE:
			// 0 - ask, 1 - abort, 2 - retry, 3 - ignore
			if ( *m_pnDefault == 1 )
				return IDABORT;
			else if ( *m_pnDefault == 2 )
				return IDRETRY;
			else if ( *m_pnDefault == 3 )
				return IDIGNORE;
			break;
		case MB_YESNOCANCEL:
			// 0 - ask, 1 - no, 2 - yes, 3 - cancel
			if ( *m_pnDefault == 1 )
				return IDNO;
			else if ( *m_pnDefault == 2 )
				return IDYES;
			else if ( *m_pnDefault == 3 )
				return IDCANCEL;
			break;
		case MB_YESNO:
			// 0 - ask, 1 - no, 2 - yes
			if ( *m_pnDefault == 1 )
				return IDNO;
			else if ( *m_pnDefault == 2 )
				return IDYES;
			break;
		case MB_RETRYCANCEL:
			// 0 - ask, 1 - retry, 2 - cancel
			if ( *m_pnDefault == 1 )
				return IDRETRY;
			else if ( *m_pnDefault == 2 )
				return IDCANCEL;
			break;
		case MB_CANCELTRYCONTINUE:
			// 0 - ask, 1 - cancel, 2 - try again, 3 - continue
			if ( *m_pnDefault == 1 )
				return IDCANCEL;
			else if ( *m_pnDefault == 2 )
				return IDTRYAGAIN;
			else if ( *m_pnDefault == 3 )
				return IDCONTINUE;
			break;
		}
	}

	if ( m_pnDefault )
		*m_pnDefault = 0;

	INT_PTR nResult = CSkinDialog::DoModal();

	if ( m_pnDefault && m_bRemember )
	{
		switch ( m_nType & MB_TYPEMASK )
		{
		case MB_OK:
			if ( nResult == IDOK )
				*m_pnDefault = 1;
			break;
		case MB_OKCANCEL:
			if ( nResult == IDOK )
				*m_pnDefault = 1;
			else if ( nResult == IDCANCEL )
				*m_pnDefault = 2;
			break;
		case MB_ABORTRETRYIGNORE:
			if ( nResult == IDABORT )
				*m_pnDefault = 1;
			else if ( nResult == IDRETRY )
				*m_pnDefault = 2;
			else if ( nResult == IDIGNORE )
				*m_pnDefault = 3;
			break;
		case MB_YESNOCANCEL:
			if ( nResult == IDNO )
				*m_pnDefault = 1;
			else if ( nResult == IDYES )
				*m_pnDefault = 2;
			else if ( nResult == IDCANCEL )
				*m_pnDefault = 3;
			break;
		case MB_YESNO:
			if ( nResult == IDNO )
				*m_pnDefault = 1;
			else if ( nResult == IDYES )
				*m_pnDefault = 2;
			break;
		case MB_RETRYCANCEL:
			if ( nResult == IDRETRY )
				*m_pnDefault = 1;
			else if ( nResult == IDCANCEL )
				*m_pnDefault = 2;
			break;
		case MB_CANCELTRYCONTINUE:
			if ( nResult == IDCANCEL )
				*m_pnDefault = 1;
			else if ( nResult == IDTRYAGAIN )
				*m_pnDefault = 2;
			else if ( nResult == IDCONTINUE )
				*m_pnDefault = 3;
			break;
		}
	}

	return nResult;
}

int MsgBox(LPCTSTR lpszText, UINT nType, UINT /*nIDHelp*/, DWORD* pnDefault)
{
	CMessageDlg dlg;
	dlg.m_nType = nType;
	dlg.m_sText = lpszText;
	dlg.m_pnDefault = pnDefault;
	return dlg.DoModal();
}

int MsgBox(UINT nIDPrompt, UINT nType, UINT nIDHelp, DWORD* pnDefault)
{
	CString strText;
	Skin.LoadString( strText, nIDPrompt );
	return MsgBox( (LPCTSTR)strText, nType, nIDHelp, pnDefault );
}
