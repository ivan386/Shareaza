//
// WizardSheet.cpp
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
#include "Settings.h"
#include "Skin.h"
#include "SkinWindow.h"
#include "GProfile.h"

#include "WizardSheet.h"
#include "WizardWelcomePage.h"
#include "WizardInterfacePage.h"
#include "WizardConnectionPage.h"
#include "WizardSharePage.h"
#include "WizardProfilePage.h"
#include "WizardNetworksPage.h"
#include "WizardFinishedPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CWizardSheet

BEGIN_MESSAGE_MAP(CWizardSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CWizardSheet)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCACTIVATE()
	ON_WM_NCPAINT()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_NCMOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardSheet run wizard

BOOL CWizardSheet::RunWizard(CWnd* pParent)
{
	BOOL bSuccess = FALSE;
	
	for ( int nAttempt = 0 ; nAttempt < 2 ; nAttempt ++ )
	{
		CWizardSheet pSheet( pParent, nAttempt ? 3 : 0 );
		
		CWizardWelcomePage		pWelcome;
		CWizardConnectionPage	pConnection;
		CWizardSharePage		pShare;
		CWizardProfilePage		pProfile;
		CWizardNetworksPage		pNetworks;
		CWizardFinishedPage		pFinished;
		
		pSheet.AddPage( &pWelcome );
		pSheet.AddPage( &pConnection );
		pSheet.AddPage( &pShare );
		pSheet.AddPage( &pProfile );
		pSheet.AddPage( &pNetworks );
		pSheet.AddPage( &pFinished );
		
		bSuccess |= ( pSheet.DoModal() == IDOK );
		Settings.Save();
		
		if ( pProfile.m_pWorld != NULL ) break;
		if ( MyProfile.GetNick().GetLength() && MyProfile.GetLocation().GetLength() ) break;
	}
	
	return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////
// CWizardSheet construction

CWizardSheet::CWizardSheet(CWnd *pParentWnd, UINT iSelectPage)
{
	m_pSkin = NULL;
	m_psh.dwFlags &= ~PSP_HASHELP;
	
	Construct( _T(""), pParentWnd, iSelectPage );
	SetWizardMode();
}

CWizardSheet::~CWizardSheet()
{
}

/////////////////////////////////////////////////////////////////////////////
// CWizardSheet message handlers

BOOL CWizardSheet::OnInitDialog() 
{
	CPropertySheet::OnInitDialog();
	CRect rc;
	
	SetIcon( theApp.LoadIcon( IDR_MAINFRAME ), TRUE );
	SetFont( &theApp.m_gdiFont );
	
	GetClientRect( &rc );
	
	/*
	if ( m_pSkin = Skin.GetWindowSkin( _T("CWizardSheet") ) )
	{
		m_pSkin->CalcWindowRect( &rc );
		SetWindowPos( NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE );
	}
	*/
	
	GetDlgItem( 0x3023 )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.OffsetRect( 95 + 6 - rc.left, 0 );
	GetDlgItem( 0x3023 )->MoveWindow( &rc );
	
	GetDlgItem( 0x3024 )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.OffsetRect( 95 + 88 - rc.left, 0 );
	GetDlgItem( 0x3024 )->MoveWindow( &rc );
	GetDlgItem( 0x3025 )->MoveWindow( &rc );
	
	GetDlgItem( 2 )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.OffsetRect( 95 + 170 - rc.left, 0 );
	GetDlgItem( 2 )->MoveWindow( &rc );
	GetDlgItem( 2 )->SetWindowText( _T("E&xit") );
	
	if ( GetDlgItem( 0x0009 ) ) GetDlgItem( 0x0009 )->ShowWindow( SW_HIDE );
	if ( GetDlgItem( 0x3026 ) ) GetDlgItem( 0x3026 )->ShowWindow( SW_HIDE );
	
	m_bmHeader.LoadBitmap( IDB_WIZARD );
	
	return TRUE;
}

BOOL CWizardSheet::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	CWnd* pWnd = GetActivePage();

	if ( pWnd != NULL )
	{
		if ( GetWindowLong( pWnd->GetSafeHwnd(), GWL_USERDATA ) == TRUE )
		{
			pWnd = NULL;
		}
		else
		{
			SetWindowLong( pWnd->GetSafeHwnd(), GWL_USERDATA, TRUE );
			pWnd->SetFont( &theApp.m_gdiFont, FALSE );
			pWnd = pWnd->GetWindow( GW_CHILD );
		}
	}

	while ( pWnd != NULL )
	{
		TCHAR szName[32];

		GetClassName( pWnd->GetSafeHwnd(), szName, 32 );

		if ( !_tcscmp( szName, _T("Static") ) )
		{
			pWnd->SetFont( &theApp.m_gdiFont, FALSE );
			
		}
		else if ( _tcscmp( szName, _T("RICHEDIT") ) )
		{
			pWnd->SetFont( &theApp.m_gdiFont, TRUE );
		}

		pWnd = pWnd->GetNextWindow();
	}
	
	return CPropertySheet::OnChildNotify( message, wParam, lParam, pLResult );
}

void CWizardSheet::OnSize(UINT nType, int cx, int cy) 
{
	if ( m_pSkin ) m_pSkin->OnSize( this );

	CPropertySheet::OnSize( nType, cx, cy );
	
	if ( CWnd* pWnd = GetWindow( GW_CHILD ) )
	{
		GetClientRect( &m_rcPage );

		m_rcPage.top += 51;	// 50
		m_rcPage.bottom -= 48;

		pWnd->SetWindowPos( NULL, m_rcPage.left, m_rcPage.top, m_rcPage.Width(),
			m_rcPage.Height(), SWP_NOSIZE );
	}
}

BOOL CWizardSheet::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CWizardSheet::OnPaint() 
{
	CPaintDC dc( this );
	CRect rc;

	GetClientRect( &rc );
	
	CDC mdc;
	mdc.CreateCompatibleDC( &dc );
	CBitmap* pOldBitmap = (CBitmap*)mdc.SelectObject( &m_bmHeader );
	dc.BitBlt( 0, 0, 438, 50, &mdc, 0, 0, SRCCOPY );
	mdc.SelectObject( pOldBitmap );
	mdc.DeleteDC();

	dc.Draw3dRect( 0, 50, rc.Width() + 1, 1,
		RGB( 128, 128, 128 ), RGB( 128, 128, 128 ) );

	dc.Draw3dRect( 0, rc.bottom - 48, rc.Width() + 1, 2,
		RGB( 128, 128, 128 ), RGB( 255, 255, 255 ) );

	dc.FillSolidRect( rc.left, rc.bottom - 46, rc.Width(), 46, GetSysColor( COLOR_BTNFACE ) );
}

/////////////////////////////////////////////////////////////////////////////
// CWizardSheet skin support

void CWizardSheet::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	if ( m_pSkin )
		m_pSkin->OnNcCalcSize( this, bCalcValidRects, lpncsp );
	else
		CPropertySheet::OnNcCalcSize( bCalcValidRects, lpncsp );
}

UINT CWizardSheet::OnNcHitTest(CPoint point) 
{
	if ( m_pSkin )
		return m_pSkin->OnNcHitTest( this, point, FALSE );
	else
		return CPropertySheet::OnNcHitTest( point );
}

BOOL CWizardSheet::OnNcActivate(BOOL bActive) 
{
	if ( m_pSkin )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible ) ModifyStyle( WS_VISIBLE, 0 );
		BOOL bResult = CPropertySheet::OnNcActivate( bActive );
		if ( bVisible ) ModifyStyle( 0, WS_VISIBLE );
		m_pSkin->OnNcActivate( this, bActive || ( m_nFlags & WF_STAYACTIVE ) );
		return bResult;
	}
	else
	{
		return CPropertySheet::OnNcActivate( bActive );
	}
}

void CWizardSheet::OnNcPaint() 
{
	if ( m_pSkin )
		m_pSkin->OnNcPaint( this );
	else
		CPropertySheet::OnNcPaint();
}

void CWizardSheet::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDown( this, nHitTest, point ) ) return;
	CPropertySheet::OnNcLButtonDown(nHitTest, point);
}

void CWizardSheet::OnNcLButtonUp(UINT nHitTest, CPoint point) 
{
	if ( m_pSkin && m_pSkin->OnNcLButtonUp( this, nHitTest, point ) ) return;
	CPropertySheet::OnNcLButtonUp( nHitTest, point );
}

void CWizardSheet::OnNcLButtonDblClk(UINT nHitTest, CPoint point) 
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDblClk( this, nHitTest, point ) ) return;
	CPropertySheet::OnNcLButtonDblClk( nHitTest, point );
}

void CWizardSheet::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	if ( m_pSkin ) m_pSkin->OnNcMouseMove( this, nHitTest, point );
	CPropertySheet::OnNcMouseMove( nHitTest, point );
}

LONG CWizardSheet::OnSetText(WPARAM wParam, LPARAM lParam)
{
	if ( m_pSkin )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible ) ModifyStyle( WS_VISIBLE, 0 );
		LONG lResult = Default();
		if ( bVisible ) ModifyStyle( 0, WS_VISIBLE );
		if ( m_pSkin ) m_pSkin->OnSetText( this );
		return lResult;
	}
	else
	{
		return Default();
	}
}


/////////////////////////////////////////////////////////////////////////////
// CWizardPage

IMPLEMENT_DYNCREATE(CWizardPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CWizardPage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardPage)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardPage construction

CWizardPage::CWizardPage(UINT nID) : CPropertyPage( nID )
{
	m_crWhite = RGB( 255, 255, 255 );
	m_brWhite.CreateSolidBrush( m_crWhite );
}

CWizardPage::~CWizardPage()
{
}

/////////////////////////////////////////////////////////////////////////////
// CWizardPage message handlers

HBRUSH CWizardPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	pDC->SetBkColor( m_crWhite );
	return (HBRUSH)m_brWhite.GetSafeHandle();
}

void CWizardPage::OnSize(UINT nType, int cx, int cy) 
{
	CPropertyPage::OnSize(nType, cx, cy);
	
	CWizardSheet* pSheet = (CWizardSheet*)GetParent();

	if ( cx != pSheet->m_rcPage.Width() )
	{
		MoveWindow( &pSheet->m_rcPage );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWizardPage operations

CWizardSheet* CWizardPage::GetSheet()
{
	return (CWizardSheet*)GetParent();
}

void CWizardPage::SetWizardButtons(DWORD dwFlags)
{
	GetSheet()->SetWizardButtons( dwFlags );
}

void CWizardPage::StaticReplace(LPCTSTR pszSearch, LPCTSTR pszReplace)
{
	for ( CWnd* pChild = GetWindow( GW_CHILD ) ; pChild ; pChild = pChild->GetNextWindow() )
	{
		TCHAR szName[32];
		GetClassName( pChild->GetSafeHwnd(), szName, 32 );

		if ( _tcscmp( szName, _T("Static") ) ) continue;
		
		CString strText;
		pChild->GetWindowText( strText );

		while ( TRUE )
		{
			int nPos = strText.Find( pszSearch );
			if ( nPos < 0 ) break;
			strText	= strText.Left( nPos ) + CString( pszReplace )
					+ strText.Mid( nPos + _tcslen( pszSearch ) );
		}

		pChild->SetWindowText( strText );
	}
}
