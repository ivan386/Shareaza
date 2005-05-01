//
// WndSettingsPage.cpp
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
#include "WndSettingsSheet.h"
#include "WndSettingsPage.h"

#include <afxpriv.h>
#include <..\src\mfc\afximpl.h>
//#include "C:\Development\VisualStudio2003\Vc7\atlmfc\src\mfc\afximpl.h"

#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSettingsPage, CDialog)

BEGIN_MESSAGE_MAP(CSettingsPage, CDialog)
	//{{AFX_MSG_MAP(CSettingsPage)
	//}}AFX_MSG_MAP
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSettingsPage construction

CSettingsPage::CSettingsPage(UINT nIDTemplate, LPCTSTR pszCaption) : CDialog( nIDTemplate )
{
	//{{AFX_DATA_INIT(CSettingsPage)
	//}}AFX_DATA_INIT

	if ( pszCaption != NULL ) m_sCaption = pszCaption;
	else if ( m_lpszTemplateName != NULL ) LoadDefaultCaption();
}

CSettingsPage::~CSettingsPage()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSettingsPage operations

BOOL CSettingsPage::LoadDefaultCaption()
{
	CDialogTemplate pTemplate;
	DLGTEMPLATE* pData;
	LPWORD pWord;

	if ( ! pTemplate.Load( m_lpszTemplateName ) ) return FALSE;

	pData = (DLGTEMPLATE*)GlobalLock( pTemplate.m_hTemplate );

	if ( ((DLGTEMPLATEEX*)pData)->signature == 0xFFFF )
	{
		pWord = (WORD*)( (DLGTEMPLATEEX*)pData + 1 );
	}
	else
	{
		pWord = (WORD*)( pData + 1 );
	}

	if ( *pWord == 0xFFFF )
	{
		pWord += 2;
	}
	else
	{
		while ( *pWord++ );
	}

	if ( *pWord == 0xFFFF )
	{
		pWord += 2;
	}
	else
	{
		while ( *pWord++ );
	}

	m_sCaption = (wchar_t*)pWord;

	GlobalUnlock( pTemplate.m_hTemplate );

	return m_sCaption.GetLength() > 0;
}

BOOL CSettingsPage::Create(CRect& rcPage, CWnd* pSheetWnd)
{
	ASSERT_VALID(this);
	ASSERT( m_lpszTemplateName != NULL );

	CDialogTemplate pTemplate;
	LPDLGTEMPLATE pData;

	if ( ! pTemplate.Load( m_lpszTemplateName ) ) return FALSE;
	pData = (LPDLGTEMPLATE)GlobalLock( pTemplate.m_hTemplate );

	if ( ((DLGTEMPLATEEX*)pData)->signature == 0xFFFF )
	{
		DLGTEMPLATEEX* pEx = (DLGTEMPLATEEX*)pData;
		pEx->style		= WS_CHILDWINDOW|WS_OVERLAPPED|DS_3DLOOK|DS_SETFONT|DS_CONTROL;
		pEx->exStyle	= WS_EX_LEFT|WS_EX_LTRREADING|WS_EX_RIGHTSCROLLBAR|WS_EX_WINDOWEDGE|WS_EX_CONTROLPARENT;
	}
	else
	{
		pData->style			= WS_CHILDWINDOW|WS_OVERLAPPED|DS_3DLOOK|DS_SETFONT|DS_CONTROL;
		pData->dwExtendedStyle	= WS_EX_LEFT|WS_EX_LTRREADING|WS_EX_RIGHTSCROLLBAR|WS_EX_WINDOWEDGE|WS_EX_CONTROLPARENT;
	}

	GlobalUnlock( pTemplate.m_hTemplate );
	CreateIndirect( pTemplate.m_hTemplate, pSheetWnd );
	SetFont( &theApp.m_gdiFont );

	MoveWindow( rcPage );

	return ( m_hWnd != NULL );
}

CSettingsPage* CSettingsPage::GetPage(CRuntimeClass* pClass) const
{
	return GetSheet()->GetPage( pClass );
}

/////////////////////////////////////////////////////////////////////////////
// CSettingsPage message handlers

BOOL CSettingsPage::OnInitDialog()
{
	CDialog::OnInitDialog();

	Skin.Apply( NULL, this );

	return TRUE;
}

void CSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CSettingsPage)
	//}}AFX_DATA_MAP
}

void CSettingsPage::SetModified(BOOL bChanged)
{
	ASSERT_VALID(this);
	GetSheet()->SetModified( bChanged );
}

BOOL CSettingsPage::OnApply()
{
	ASSERT_VALID(this);
	OnOK();
	return TRUE;
}

void CSettingsPage::OnReset()
{
	ASSERT_VALID(this);
	OnCancel();
}

void CSettingsPage::OnOK()
{
	ASSERT_VALID(this);
}

void CSettingsPage::OnCancel()
{
	ASSERT_VALID(this);
}

BOOL CSettingsPage::OnSetActive()
{
	ASSERT_VALID(this);
	return TRUE;
}

BOOL CSettingsPage::OnKillActive()
{
	ASSERT_VALID(this);
	if ( ! UpdateData() ) return FALSE;
	return TRUE;
}

BOOL CSettingsPage::OnEraseBkgnd(CDC* pDC)
{
	CRect rc;
	GetClientRect( &rc );
	pDC->FillSolidRect( &rc, Skin.m_crDialog );
	return TRUE;
}

HBRUSH CSettingsPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC )
	{
		pDC->SetBkColor( Skin.m_crDialog );
		hbr = Skin.m_brDialog;
	}

	return hbr;
}
