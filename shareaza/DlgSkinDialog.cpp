//
// DlgSkinDialog.cpp
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
#include "DlgSkinDialog.h"
#include "CoolInterface.h"
#include "Skin.h"
#include "SkinWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSkinDialog, CDialog)

BEGIN_MESSAGE_MAP(CSkinDialog, CDialog)
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCACTIVATE()
	ON_WM_NCPAINT()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_NCMOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_SETTEXT, &CSkinDialog::OnSetText)
	ON_WM_CTLCOLOR()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_CREATE()
	ON_WM_HELPINFO()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSkinDialog dialog

CSkinDialog::CSkinDialog(UINT nResID, CWnd* pParent, BOOL bAutoBanner)
	: CDialog		( nResID, pParent )
	, m_pSkin		( NULL )
	, m_bAutoBanner	( bAutoBanner )
{
}

void CSkinDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	if ( m_oBanner.m_hWnd ) DDX_Control(pDX, IDC_BANNER, m_oBanner);
}

/////////////////////////////////////////////////////////////////////////////
// CSkinDialog operations

int CSkinDialog::GetBannerHeight() const
{
	if ( CStatic* pBanner = (CStatic*)GetDlgItem( IDC_BANNER ) )
	{
		BITMAP bm = {};
		GetObject( pBanner->GetBitmap(), sizeof( BITMAP ), &bm );
		return bm.bmHeight;
	}
	return 0;
}

void CSkinDialog::EnableBanner(BOOL bEnable)
{
	if ( ! bEnable && m_oBanner.m_hWnd )
	{
		int nBannerHeight = GetBannerHeight();

		// Remove banner
		m_oBanner.DestroyWindow();

		// Move all controls up
		for ( CWnd* pChild = GetWindow( GW_CHILD ); pChild;
			pChild = pChild->GetNextWindow() )
		{
			CRect rc;
			pChild->GetWindowRect( &rc );
			ScreenToClient( &rc );
			rc.MoveToY( rc.top - nBannerHeight );
			pChild->MoveWindow( &rc );
		}

		// Resize window
		CRect rcWindow;
		GetWindowRect( &rcWindow );
		SetWindowPos( NULL, 0, 0, rcWindow.Width(), rcWindow.Height() - nBannerHeight,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
	}
	else if ( bEnable && ! m_oBanner.m_hWnd )
	{	
		if ( HBITMAP hBitmap = Skin.GetWatermark( _T("Banner"), TRUE ) )
		{
			BITMAP bm = {};
			GetObject( hBitmap, sizeof( BITMAP ), &bm );

			// Resize window
			CRect rcWindow;
			GetWindowRect( &rcWindow );
			SetWindowPos( NULL, 0, 0, rcWindow.Width(), rcWindow.Height() + bm.bmHeight,
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

			// Move all controls down
			for ( CWnd* pChild = GetWindow( GW_CHILD ); pChild;
				pChild = pChild->GetNextWindow() )
			{
				CRect rc;
				pChild->GetWindowRect( &rc );
				ScreenToClient( &rc );
				rc.MoveToY( rc.top + bm.bmHeight );
				pChild->MoveWindow( &rc );
			}

			// Add banner
			CRect rcBanner;
			GetClientRect( &rcBanner );
			if ( Settings.General.LanguageRTL )
			{
				rcBanner.left -= bm.bmWidth - rcBanner.Width();
			}
			rcBanner.right = rcBanner.left + bm.bmWidth;
			rcBanner.bottom = rcBanner.top + bm.bmHeight;
			VERIFY( m_oBanner.Create( NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP |
				SS_REALSIZEIMAGE, rcBanner, this, IDC_BANNER ) );
			m_oBanner.SetBitmap( hBitmap );
		}
	}
}

void CSkinDialog::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType)
{
	if ( ! theApp.m_bClosing && m_pSkin )
		m_pSkin->CalcWindowRect( lpClientRect );
	else
		CDialog::CalcWindowRect( lpClientRect, nAdjustType );
}

void CSkinDialog::RemoveSkin()
{
	if (  m_pSkin )
	{
		m_pSkin = NULL;
		CoolInterface.EnableTheme( this, TRUE );
	}
}

BOOL CSkinDialog::SkinMe(LPCTSTR pszSkin, UINT nIcon, BOOL bLanguage)
{
	if ( m_bAutoBanner )
		EnableBanner( TRUE );

	CString strSkin;
	if ( pszSkin )
		strSkin = pszSkin;
	else
		strSkin = GetRuntimeClass()->m_lpszClassName;
	
	CRect rc;
	GetClientRect( &rc );

	m_pSkin = ::Skin.GetWindowSkin( strSkin );
	if ( ! m_pSkin )
		m_pSkin = ::Skin.GetWindowSkin( this );

	if ( m_pSkin )
		CoolInterface.EnableTheme( this, FALSE );

	BOOL bSuccess = FALSE;
	if ( bLanguage )
		bSuccess = ::Skin.Apply( strSkin, this, nIcon );

	if ( nIcon )
		CoolInterface.SetIcon( nIcon, m_pSkin && Settings.General.LanguageRTL, FALSE, this );

	if ( m_pSkin )
		ModifyStyle( WS_CAPTION, 0 );
	else
		ModifyStyle( 0, WS_CAPTION );

	CalcWindowRect( &rc );
	SetWindowRgn( NULL, FALSE );
	SetWindowPos( NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED );

	if ( m_pSkin )
		m_pSkin->OnSize( this );

	return bSuccess || ( m_pSkin != NULL );
}

BOOL CSkinDialog::SelectCaption(CWnd* pWnd, int nIndex)
{
	return ::Skin.SelectCaption( pWnd, nIndex );
}

/////////////////////////////////////////////////////////////////////////////
// CSkinDialog message handlers

void CSkinDialog::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	if ( ! theApp.m_bClosing && m_pSkin )
		m_pSkin->OnNcCalcSize( this, bCalcValidRects, lpncsp );
	else
		CDialog::OnNcCalcSize( bCalcValidRects, lpncsp );
}

LRESULT CSkinDialog::OnNcHitTest(CPoint point)
{
	if ( ! theApp.m_bClosing && m_pSkin )
		return m_pSkin->OnNcHitTest( this, point, ( GetStyle() & WS_THICKFRAME ) ? TRUE : FALSE );
	else
		return CDialog::OnNcHitTest( point );
}

BOOL CSkinDialog::OnNcActivate(BOOL bActive)
{
	if ( ! theApp.m_bClosing && m_pSkin )
	{
		m_pSkin->OnNcActivate( this, IsWindowEnabled() && ( bActive || ( m_nFlags & WF_STAYACTIVE ) ) );
		return TRUE;
	}
	else
		return CDialog::OnNcActivate( bActive );
}

void CSkinDialog::OnNcPaint()
{
	if ( ! theApp.m_bClosing && m_pSkin )
		m_pSkin->OnNcPaint( this );
	else
		CDialog::OnNcPaint();
}

void CSkinDialog::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDown( this, nHitTest, point ) ) return;
	CDialog::OnNcLButtonDown(nHitTest, point);
}

void CSkinDialog::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonUp( this, nHitTest, point ) ) return;
	CDialog::OnNcLButtonUp( nHitTest, point );
}

void CSkinDialog::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDblClk( this, nHitTest, point ) ) return;
	CDialog::OnNcLButtonDblClk( nHitTest, point );
}

void CSkinDialog::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	if ( m_pSkin ) m_pSkin->OnNcMouseMove( this, nHitTest, point );
	CDialog::OnNcMouseMove( nHitTest, point );
}

void CSkinDialog::OnSize(UINT nType, int cx, int cy)
{
	if ( m_pSkin ) m_pSkin->OnSize( this );
	CDialog::OnSize( nType, cx, cy );
}

LRESULT CSkinDialog::OnSetText(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if ( m_pSkin )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible ) ModifyStyle( WS_VISIBLE, 0 );
		LRESULT lResult = Default();
		if ( bVisible ) ModifyStyle( 0, WS_VISIBLE );
		if ( m_pSkin ) m_pSkin->OnSetText( this );
		return lResult;
	}
	else
	{
		return Default();
	}
}

BOOL CSkinDialog::OnEraseBkgnd(CDC* pDC)
{
	if ( ! theApp.m_bClosing && m_pSkin && m_pSkin->OnEraseBkgnd( this, pDC ) )
		return TRUE;

	CRect rc;
	GetClientRect( &rc );
	rc.bottom = GetBannerHeight();
	pDC->FillSolidRect( &rc, RGB( 0, 0, 0 ) );

	GetClientRect( &rc );
	rc.top = GetBannerHeight();
	pDC->FillSolidRect( &rc, Skin.m_crDialog );

	return TRUE;
}

HBRUSH CSkinDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC )
	{
		pDC->SetBkColor( Skin.m_crDialog );
		hbr = Skin.m_brDialog;
	}

	return hbr;
}

#define SNAP_SIZE 6

void CSkinDialog::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CDialog::OnWindowPosChanging( lpwndpos );

	HMONITOR hMonitor = MonitorFromWindow( GetSafeHwnd(),
		MONITOR_DEFAULTTOPRIMARY );

	MONITORINFO oMonitor = {0};
	oMonitor.cbSize = sizeof( MONITORINFO );
	GetMonitorInfo( hMonitor, &oMonitor );

	if ( abs( lpwndpos->x - oMonitor.rcWork.left ) < SNAP_SIZE )
		lpwndpos->x = oMonitor.rcWork.left;
	if ( abs( lpwndpos->y - oMonitor.rcWork.top ) < SNAP_SIZE )
		lpwndpos->y = oMonitor.rcWork.top;
	if ( abs( lpwndpos->x + lpwndpos->cx - oMonitor.rcWork.right ) < SNAP_SIZE )
		lpwndpos->x = oMonitor.rcWork.right - lpwndpos->cx;
	if ( abs( lpwndpos->y + lpwndpos->cy - oMonitor.rcWork.bottom ) < SNAP_SIZE )
		lpwndpos->y = oMonitor.rcWork.bottom - lpwndpos->cy;

	if ( m_oBanner.m_hWnd )
	{	
		if ( HBITMAP hBitmap = m_oBanner.GetBitmap() )
		{
			BITMAP bm = {};
			GetObject( hBitmap, sizeof( BITMAP ), &bm );
			CRect rcBanner;
			GetClientRect( &rcBanner );
			if ( Settings.General.LanguageRTL )
			{
				rcBanner.left -= bm.bmWidth - rcBanner.Width();
			}
			rcBanner.right = rcBanner.left + bm.bmWidth;
			rcBanner.bottom = rcBanner.top + bm.bmHeight;
			m_oBanner.MoveWindow( rcBanner );
		}
	}
}

int CSkinDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	if ( Settings.General.LanguageRTL )
		ModifyStyleEx( 0, WS_EX_LAYOUTRTL | WS_EX_RTLREADING );
	else
		ModifyStyleEx( WS_EX_LAYOUTRTL | WS_EX_RTLREADING, 0 );

	return 0;
}

BOOL CSkinDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;
}

BOOL CSkinDialog::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	return FALSE;
}
