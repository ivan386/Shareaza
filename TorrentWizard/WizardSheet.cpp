//
// WizardSheet.cpp
//
// Copyright (c) Shareaza Development Team, 2007-2009.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "StdAfx.h"
#include "TorrentWizard.h"
#include "WizardSheet.h"

#include "PageWelcome.h"
#include "PageSingle.h"
#include "PagePackage.h"
#include "PageTracker.h"
#include "PageComment.h"
#include "PageOutput.h"
#include "PageFinished.h"
#include "PageCommandline.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CWizardSheet

IMPLEMENT_DYNAMIC(CWizardSheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CWizardSheet, CPropertySheet)
	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardSheet run wizard

BOOL CWizardSheet::Run(CWnd* pParent)
{
	CWizardSheet		pSheet( pParent, 0 );
	CWelcomePage		pWelcome;
	CSinglePage			pSingle;
	CPackagePage		pPackage;
	CTrackerPage		pTracker;
	CCommentPage		pComment;
	COutputPage			pOutput;
	CFinishedPage		pFinished;
	CCommandlinePage	pCommandline;

	if ( theApp.m_bCommandLine ) 
	{
		pSheet.AddPage( &pCommandline );
	}
	else
	{
		pSheet.AddPage( &pWelcome );
		pSheet.AddPage( &pSingle );
		pSheet.AddPage( &pPackage );
		pSheet.AddPage( &pTracker );
		pSheet.AddPage( &pOutput );
		pSheet.AddPage( &pComment );
		pSheet.AddPage( &pFinished );
	}


	if ( pSheet.DoModal() != IDOK ) return FALSE;
	
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CWizardSheet construction

CWizardSheet::CWizardSheet(CWnd *pParentWnd, UINT iSelectPage)
{
	m_bmHeader.LoadBitmap( IDB_WIZARD );

	m_psh.dwFlags &= ~PSP_HASHELP;

	Construct( _T(""), pParentWnd, iSelectPage );
	SetWizardMode();
}

/////////////////////////////////////////////////////////////////////////////
// CWizardSheet page lookup

CWizardPage* CWizardSheet::GetPage(CRuntimeClass* pClass)
{
	for ( int nPage = 0 ; nPage < GetPageCount() ; nPage++ )
	{
		CWizardPage* pPage = (CWizardPage*)CPropertySheet::GetPage( nPage );
		ASSERT_KINDOF(CWizardPage, pPage);
		if ( pPage->IsKindOf( pClass ) ) return pPage;
	}
	
	return NULL;
}

void CWizardSheet::DoReset()
{
	for ( int nPage = 0 ; nPage < GetPageCount() ; nPage++ )
	{
		CWizardPage* pPage = (CWizardPage*)CPropertySheet::GetPage( nPage );
		ASSERT_KINDOF(CWizardPage, pPage);
		if ( pPage->m_hWnd != NULL ) pPage->OnReset();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWizardSheet message handlers

BOOL CWizardSheet::OnInitDialog() 
{
	CPropertySheet::OnInitDialog();
	CRect rc;
	
	SetIcon( theApp.LoadIcon( IDR_MAINFRAME ), TRUE );
	SetFont( &theApp.m_fntNormal );
	
	ModifyStyle( 0, WS_MINIMIZEBOX );
	
	GetDlgItem( 0x3023 )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.OffsetRect( 16 - rc.left, -1 );
	GetDlgItem( 0x3023 )->MoveWindow( &rc );
	
	GetDlgItem( 0x3024 )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.OffsetRect( 100 - rc.left, -1 );
	GetDlgItem( 0x3024 )->MoveWindow( &rc );
	GetDlgItem( 0x3025 )->MoveWindow( &rc );
	
	GetDlgItem( 2 )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.OffsetRect( 414 - rc.left, -1 );
	GetDlgItem( 2 )->MoveWindow( &rc );
	GetDlgItem( 2 )->SetWindowText( _T("E&xit") );
	
	if ( GetDlgItem( 0x0009 ) ) GetDlgItem( 0x0009 )->ShowWindow( SW_HIDE );
	if ( GetDlgItem( 0x3026 ) ) GetDlgItem( 0x3026 )->ShowWindow( SW_HIDE );
	
	return TRUE;
}

BOOL CWizardSheet::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	CWnd* pWnd = GetActivePage();
	
	if ( pWnd != NULL )
	{
		if ( GetWindowLongPtr( pWnd->GetSafeHwnd(), GWLP_USERDATA ) == TRUE )
		{
			pWnd = NULL;
		}
		else
		{
			SetWindowLongPtr( pWnd->GetSafeHwnd(), GWLP_USERDATA, TRUE );
			pWnd->SetFont( &theApp.m_fntNormal, FALSE );
			pWnd = pWnd->GetWindow( GW_CHILD );
		}
	}
	
	while ( pWnd != NULL )
	{
		TCHAR szName[32];

		GetClassName( pWnd->GetSafeHwnd(), szName, 32 );

		if ( !_tcscmp( szName, _T("Static") ) )
		{
			pWnd->SetFont( &theApp.m_fntNormal, FALSE );
			
		}
		else if ( _tcscmp( szName, _T("RICHEDIT") ) )
		{
			pWnd->SetFont( &theApp.m_fntNormal, TRUE );
		}

		pWnd = pWnd->GetNextWindow();
	}
	
	return CPropertySheet::OnChildNotify( message, wParam, lParam, pLResult );
}

void CWizardSheet::OnSize(UINT nType, int cx, int cy) 
{
	CPropertySheet::OnSize( nType, cx, cy );
	
	if ( CWnd* pWnd = GetWindow( GW_CHILD ) )
	{
		GetClientRect( &m_rcPage );
	
		BITMAP bm;
		m_bmHeader.GetBitmap( &bm );
		
		m_rcPage.top += bm.bmHeight + 1;
		m_rcPage.bottom -= bm.bmHeight - 2;

		pWnd->SetWindowPos( NULL, m_rcPage.left, m_rcPage.top, m_rcPage.Width(),
			m_rcPage.Height(), SWP_NOSIZE );
	}
}

void CWizardSheet::OnPaint() 
{
	CPaintDC dc( this );
	dc.SetBkMode( TRANSPARENT );
	dc.SetTextColor( RGB( 255, 255, 255 ) );

	CRect rc;
	GetClientRect( &rc );

	BITMAP bm;
	m_bmHeader.GetBitmap( &bm );

	CRect rcHeader = rc;
	rcHeader.bottom = rcHeader.top + bm.bmHeight;

	CDC mdc;
	mdc.CreateCompatibleDC( &dc );
	CBitmap* pOldBitmap = (CBitmap*)mdc.SelectObject( &m_bmHeader );
	dc.BitBlt( rcHeader.left, rcHeader.top, rcHeader.Width(), rcHeader.Height(),
		&mdc, 0, 0, SRCCOPY );
	mdc.SelectObject( pOldBitmap );
	mdc.DeleteDC();

	dc.Draw3dRect( rc.left, rc.top + rcHeader.Height() - 2, rc.Width() + 1, 2,
		RGB( 128, 128, 128 ), RGB( 255, 255, 255 ) );

	dc.Draw3dRect( rc.left, rc.bottom - 48, rc.Width() + 1, 2,
		RGB( 128, 128, 128 ), RGB( 255, 255, 255 ) );

	rcHeader.left += 180;
	CFont* pOldFont = (CFont*)dc.SelectObject( &theApp.m_fntHeader );
	dc.DrawText( theApp.m_sName, &rcHeader, DT_VCENTER | DT_CENTER | DT_SINGLELINE );

	rcHeader.left = rcHeader.right - 50;
	rcHeader.bottom = rcHeader.top + 20;
	dc.SelectObject( &theApp.m_fntTiny );
	dc.DrawText( theApp.m_sVersion, &rcHeader, DT_VCENTER | DT_CENTER | DT_SINGLELINE );

	dc.SelectObject( pOldFont );
}

/////////////////////////////////////////////////////////////////////////////
// CWizardPage

IMPLEMENT_DYNCREATE(CWizardPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CWizardPage, CPropertyPage)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardPage construction

CWizardPage::CWizardPage(UINT nID) : CPropertyPage( nID )
{
	m_crWhite = RGB( 255, 255, 255 );
	m_brWhite.CreateSolidBrush( m_crWhite );
}

/////////////////////////////////////////////////////////////////////////////
// CWizardPage message handlers

HBRUSH CWizardPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT /*nCtlColor*/) 
{
	if ( pWnd != NULL && pWnd->GetDlgCtrlID() == IDC_TITLE )
	{
		pDC->SelectObject( &theApp.m_fntBold );
	}

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

CWizardPage* CWizardPage::GetPage(CRuntimeClass* pClass)
{
	return GetSheet()->GetPage( pClass );
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
		
		for (;;)
		{
			int nPos = strText.Find( pszSearch );
			if ( nPos < 0 ) break;
			strText	= strText.Left( nPos ) + CString( pszReplace )
					+ strText.Mid( nPos + static_cast< int >( _tcslen( pszSearch ) ) );
		}
		
		pChild->SetWindowText( strText );
	}
}
