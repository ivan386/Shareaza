//
// WizardSheet.cpp
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
#include "CoolInterface.h"
#include "GProfile.h"
#include "WndMain.h"
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

IMPLEMENT_DYNAMIC(CWizardSheet, CPropertySheetAdv)

BEGIN_MESSAGE_MAP(CWizardSheet, CPropertySheetAdv)
	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardSheet run wizard

BOOL CWizardSheet::RunWizard(CWnd* pParent)
{
	BOOL bSuccess = FALSE;

	CWizardSheet pSheet( pParent );

	CWizardWelcomePage		pWelcome;
	CWizardConnectionPage	pConnection;
	CWizardSharePage		pShare;
	CWizardProfilePage		pProfile;
	CWizardInterfacePage	pInterface;
	CWizardNetworksPage		pNetworks;
	CWizardFinishedPage		pFinished;

	pSheet.AddPage( &pWelcome );
	pSheet.AddPage( &pConnection );
	pSheet.AddPage( &pShare );
	pSheet.AddPage( &pProfile );
	pSheet.AddPage( &pInterface );
	pSheet.AddPage( &pNetworks );
	pSheet.AddPage( &pFinished );

	bSuccess = ( pSheet.DoModal() == IDOK );

	CWaitCursor pCursor;
	CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();

	if ( pInterface.m_bExpert && Settings.General.GUIMode == GUI_BASIC )
	{
		Settings.General.GUIMode = GUI_TABBED;
		pMainWnd->SetGUIMode( Settings.General.GUIMode, FALSE );
	}
	else if ( ! pInterface.m_bExpert && Settings.General.GUIMode != GUI_BASIC )
	{
		Settings.General.GUIMode = GUI_BASIC;
		pMainWnd->SetGUIMode( Settings.General.GUIMode, FALSE );
	}

	Settings.Save();

	return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////
// CWizardSheet construction

CWizardSheet::CWizardSheet(CWnd *pParentWnd, UINT iSelectPage)
{
	Construct( _T(""), pParentWnd, iSelectPage );

	SetWizardMode();
}

/////////////////////////////////////////////////////////////////////////////
// CWizardSheet message handlers

BOOL CWizardSheet::OnInitDialog()
{
	CPropertySheetAdv::OnInitDialog();
	CRect rc;

	CString strMessage;

	SetIcon( theApp.LoadIcon( IDR_MAINFRAME ), TRUE );
	SetFont( &theApp.m_gdiFont );

	GetClientRect( &rc );

	GetDlgItem( ID_WIZBACK )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.OffsetRect( 95 + 6 - rc.left, 0 );
	GetDlgItem( ID_WIZBACK )->MoveWindow( &rc );

	GetDlgItem( ID_WIZNEXT )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.OffsetRect( 95 + 88 - rc.left, 0 );
	GetDlgItem( ID_WIZNEXT )->MoveWindow( &rc );

	LoadString( strMessage, IDS_GENERAL_FINISH );
	GetDlgItem( ID_WIZFINISH )->MoveWindow( &rc );
	GetDlgItem( ID_WIZFINISH )->SetWindowText( strMessage );

	LoadString( strMessage, IDS_GENERAL_BACK );
	if ( GetDlgItem( ID_WIZBACK ) )
		GetDlgItem( ID_WIZBACK )->SetWindowText( L"< " + strMessage );
	LoadString( strMessage, IDS_GENERAL_NEXT );
	if ( GetDlgItem( ID_WIZNEXT ) )
		GetDlgItem( ID_WIZNEXT )->SetWindowText( strMessage + L" >" );

	GetDlgItem( IDCANCEL )->GetWindowRect( &rc );
	ScreenToClient( &rc );
	rc.OffsetRect( 95 + 170 - rc.left, 0 );
	GetDlgItem( IDCANCEL )->MoveWindow( &rc );
	LoadString( strMessage, IDS_WIZARD_EXIT );
	GetDlgItem( IDCANCEL )->SetWindowText( strMessage );

	if ( GetDlgItem( IDHELP ) ) GetDlgItem( IDHELP )->ShowWindow( SW_HIDE );
	// ATL_IDC_STATIC1?
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
			pWnd->SetFont( &theApp.m_gdiFont, FALSE );
			pWnd = pWnd->GetWindow( GW_CHILD );
		}
	}

	while ( pWnd != NULL )
	{
		TCHAR szName[32];

		GetClassName( pWnd->GetSafeHwnd(), szName, 32 );

		if ( _tcscmp( szName, _T("Static") ) == 0 )
		{
			pWnd->SetFont( &theApp.m_gdiFont, FALSE );

		}
		else if ( _tcscmp( szName, _T("RICHEDIT") ) != 0 )
		{
			pWnd->SetFont( &theApp.m_gdiFont, TRUE );
		}

		pWnd = pWnd->GetNextWindow();
	}

	return CPropertySheetAdv::OnChildNotify( message, wParam, lParam, pLResult );
}

void CWizardSheet::OnSize(UINT nType, int cx, int cy)
{
	CPropertySheetAdv::OnSize( nType, cx, cy );

	if ( CWnd* pWnd = GetWindow( GW_CHILD ) )
	{
		GetClientRect( &m_rcPage );

		m_rcPage.top += 51;	// 50
		m_rcPage.bottom -= 48;

		pWnd->SetWindowPos( NULL, m_rcPage.left, m_rcPage.top, m_rcPage.Width(),
			m_rcPage.Height(), SWP_NOSIZE );
	}
}

void CWizardSheet::OnPaint()
{
	CPaintDC dc( this );

	CRect rc;
	GetClientRect( &rc );

	HBITMAP hHeader = Skin.GetWatermark( _T("Banner"), TRUE );

	CDC mdc;
	mdc.CreateCompatibleDC( &dc );
	HBITMAP hOldBitmap = (HBITMAP)mdc.SelectObject( hHeader );
	dc.BitBlt( 0, 0, 438, 50, &mdc, 0, 0, SRCCOPY );
	mdc.SelectObject( hOldBitmap );
	mdc.DeleteDC();

	dc.Draw3dRect( 0, 50, rc.Width() + 1, 1,
		RGB( 128, 128, 128 ), RGB( 128, 128, 128 ) );

	dc.Draw3dRect( 0, rc.bottom - 48, rc.Width() + 1, 2,
		RGB( 128, 128, 128 ), RGB( 255, 255, 255 ) );

	dc.FillSolidRect( rc.left, rc.bottom - 46, rc.Width(), 46, CoolInterface.m_crSysBtnFace );
}

/////////////////////////////////////////////////////////////////////////////
// CWizardPage

IMPLEMENT_DYNCREATE(CWizardPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CWizardPage, CPropertyPageAdv)
	ON_WM_SIZE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardPage construction

CWizardPage::CWizardPage(UINT nID) : CPropertyPageAdv( nID )
{
}

/////////////////////////////////////////////////////////////////////////////
// CWizardPage message handlers

void CWizardPage::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPageAdv::OnSize(nType, cx, cy);

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
	const size_t nLen = _tcslen( pszSearch );

	for ( CWnd* pChild = GetWindow( GW_CHILD ) ; pChild ; pChild = pChild->GetNextWindow() )
	{
		TCHAR szName[32];
		GetClassName( pChild->GetSafeHwnd(), szName, 32 );

		if ( _tcscmp( szName, _T("Static") ) != 0 ) continue;

		CString strText;
		pChild->GetWindowText( strText );

		for (;;)
		{
			int nPos = strText.Find( pszSearch );
			if ( nPos < 0 ) break;
			strText	= strText.Left( nPos ) + pszReplace + strText.Mid( nPos + (int)nLen );
		}

		pChild->SetWindowText( strText );
	}
}

BOOL CWizardPage::IsConnectionCapable()
{
	return ( !theApp.m_bLimitedConnections || Settings.General.IgnoreXPsp2 )	// The connection rate limiting (XPsp2) makes multi-network performance awful
		&& ( Settings.Connection.InSpeed > 256 )								// Must have a decent connection to be worth it. (Or extra traffic will slow downloads)
		&& ( Settings.GetOutgoingBandwidth() > 16 );							// If your outbound bandwidth is too low, the ED2K ratio will throttle you anyway
}
