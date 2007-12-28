//
// DlgSkinDialog.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
	//{{AFX_MSG_MAP(CSkinDialog)
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
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_WM_CTLCOLOR()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_CREATE()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSkinDialog dialog

CSkinDialog::CSkinDialog(UINT nResID, CWnd* pParent) : CDialog( nResID, pParent )
{
	//{{AFX_DATA_INIT(CSkinDialog)
	//}}AFX_DATA_INIT
	m_pSkin = NULL;
}

void CSkinDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSkinDialog)
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CSkinDialog operations

BOOL CSkinDialog::SkinMe(LPCTSTR pszSkin, UINT nIcon, BOOL bLanguage)
{
	BOOL bSuccess = FALSE;
	CString strSkin;
	CRect rc;

	GetClientRect( &rc );

	if ( pszSkin == NULL )
		strSkin = GetRuntimeClass()->m_lpszClassName;
	else
		strSkin = pszSkin;

	m_pSkin = ::Skin.GetWindowSkin( strSkin );
	if ( NULL == m_pSkin ) m_pSkin = ::Skin.GetWindowSkin( this );

	if ( bLanguage )
	{
		bSuccess = ::Skin.Apply( strSkin, this, nIcon, &m_wndToolTip );
	}
	if ( nIcon )
	{
		CoolInterface.SetIcon( nIcon, m_pSkin && Settings.General.LanguageRTL, FALSE, this );
	}

	if ( m_pSkin != NULL )
	{
		if ( GetStyle() & WS_CAPTION ) ModifyStyle( WS_CAPTION, 0 );

		m_pSkin->CalcWindowRect( &rc );

		SetWindowRgn( NULL, FALSE );
		SetWindowPos( NULL, 0, 0, rc.Width(), rc.Height(),
			SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED );

		m_pSkin->OnSize( this );
	}
	else
	{
		if ( ( GetStyle() & WS_CAPTION ) == 0 ) ModifyStyle( 0, WS_CAPTION );

		CalcWindowRect( &rc );
		SetWindowRgn( NULL, FALSE );
		SetWindowPos( NULL, 0, 0, rc.Width(), rc.Height(),
			SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED );
	}

	return bSuccess || m_pSkin != NULL;
}

BOOL CSkinDialog::SelectCaption(CWnd* pWnd, int nIndex)
{
	return ::Skin.SelectCaption( pWnd, nIndex );
}

/////////////////////////////////////////////////////////////////////////////
// CSkinDialog message handlers

void CSkinDialog::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	if ( m_pSkin )
		m_pSkin->OnNcCalcSize( this, bCalcValidRects, lpncsp );
	else
		CDialog::OnNcCalcSize( bCalcValidRects, lpncsp );
}

ONNCHITTESTRESULT CSkinDialog::OnNcHitTest(CPoint point)
{
	if ( m_pSkin )
		return m_pSkin->OnNcHitTest( this, point, ( GetStyle() & WS_THICKFRAME ) ? TRUE : FALSE );
	else
		return CDialog::OnNcHitTest( point );
}

BOOL CSkinDialog::OnNcActivate(BOOL bActive)
{
	if ( m_pSkin )
	{
		m_pSkin->OnNcActivate( this, IsWindowEnabled() && ( bActive || ( m_nFlags & WF_STAYACTIVE ) ) );
		return TRUE;
	}
	else
	{
		return CDialog::OnNcActivate( bActive );
	}
}

void CSkinDialog::OnNcPaint()
{
	if ( m_pSkin )
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
	if ( m_pSkin && m_pSkin->OnEraseBkgnd( this, pDC ) ) return TRUE;

	CRect rc;
	GetClientRect( &rc );
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

	if ( theApp.m_pfnGetMonitorInfoA != NULL ) //If GetMonitorInfo() is available
	{
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		theApp.m_pfnGetMonitorInfoA( theApp.m_pfnMonitorFromWindow( GetSafeHwnd(), MONITOR_DEFAULTTOPRIMARY ), &oMonitor );

		if ( abs( lpwndpos->x - oMonitor.rcWork.left ) < SNAP_SIZE )
			lpwndpos->x = oMonitor.rcWork.left;
		if ( abs( lpwndpos->y - oMonitor.rcWork.top ) < SNAP_SIZE )
			lpwndpos->y = oMonitor.rcWork.top;
		if ( abs( lpwndpos->x + lpwndpos->cx - oMonitor.rcWork.right ) < SNAP_SIZE )
			lpwndpos->x = oMonitor.rcWork.right - lpwndpos->cx;
		if ( abs( lpwndpos->y + lpwndpos->cy - oMonitor.rcWork.bottom ) < SNAP_SIZE )
			lpwndpos->y = oMonitor.rcWork.bottom - lpwndpos->cy;
	}
	else
	{
		CRect rcWork;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWork, 0 );

		if ( abs( lpwndpos->x ) <= ( rcWork.left + SNAP_SIZE ) )
		{
			lpwndpos->x = rcWork.left;
		}
		else if (	( lpwndpos->x + lpwndpos->cx ) >= ( rcWork.right - SNAP_SIZE ) &&
					( lpwndpos->x + lpwndpos->cx ) <= ( rcWork.right + SNAP_SIZE ) )
		{
			lpwndpos->x = rcWork.right - lpwndpos->cx;
		}

		if ( abs( lpwndpos->y ) <= ( rcWork.top + SNAP_SIZE ) )
		{
			lpwndpos->y = rcWork.top;
		}
		else if (	( lpwndpos->y + lpwndpos->cy ) >= ( rcWork.bottom - SNAP_SIZE ) &&
					( lpwndpos->y + lpwndpos->cy ) <= ( rcWork.bottom + SNAP_SIZE ) )
		{
			lpwndpos->y = rcWork.bottom-lpwndpos->cy;
		}
	}
}

int CSkinDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	if ( Settings.General.LanguageRTL ) ModifyStyleEx( 0, WS_EX_LAYOUTRTL|WS_EX_RTLREADING, 0 );
	return 0;
}

BOOL CSkinDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_wndToolTip.Create( this );
	m_wndToolTip.Activate( TRUE );
	m_wndToolTip.SetMaxTipWidth( 200 );
	// Show the tooltip for 20 seconds
	m_wndToolTip.SetDelayTime( TTDT_AUTOPOP, 20 * 1000 );

	if ( Settings.General.LanguageRTL )
	{
		CStatic* pBanner = (CStatic*)GetDlgItem( IDC_BANNER );
		if ( pBanner )
		{
			if ( pBanner->GetBitmap() == NULL ) return TRUE;
			CRect rcClient, rc;
			pBanner->GetWindowRect( &rc );

			GetWindowRect( &rcClient );
			ScreenToClient( &rcClient );
			pBanner->SetWindowPos( NULL, rcClient.left + rcClient.Width() - rc.Width(), 
				0, 0, 0, SWP_NOSIZE|SWP_NOZORDER );
		}
	}
	return TRUE; 
}

BOOL CSkinDialog::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	return FALSE;
}

BOOL CSkinDialog::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST )
	{
		MSG msg;
		CopyMemory( &msg, pMsg, sizeof(MSG) );
		HWND hWndParent = ::GetParent( msg.hwnd );

		while ( hWndParent && hWndParent != m_hWnd )
		{
			msg.hwnd = hWndParent;
			hWndParent = ::GetParent( hWndParent );
		}

		if ( msg.hwnd )
		{
			m_wndToolTip.RelayEvent( &msg );
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}
