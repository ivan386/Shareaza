//
// CtrlCoolTip.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "CoolInterface.h"
#include "CtrlCoolTip.h"
#include "GraphLine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CCoolTipCtrl, CWnd)

BEGIN_MESSAGE_MAP(CCoolTipCtrl, CWnd)
	//{{AFX_MSG_MAP(CCoolTipCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

LPCTSTR CCoolTipCtrl::m_hClass = NULL;

#define TIP_TIMER		100
#define TIP_OFFSET_X	0
#define TIP_OFFSET_Y	24
#define TIP_MARGIN		6
#define TIP_TEXTHEIGHT	14


/////////////////////////////////////////////////////////////////////////////
// CCoolTipCtrl construction

CCoolTipCtrl::CCoolTipCtrl()
{
	m_pbEnable	= NULL;
	m_pContext	= NULL;
	m_hAltWnd	= NULL;
	m_bTimer	= FALSE;
	m_bVisible	= FALSE;
	m_tOpen		= 0;

	if ( m_hClass == NULL ) m_hClass = AfxRegisterWndClass( CS_SAVEBITS );
}

CCoolTipCtrl::~CCoolTipCtrl()
{
	if ( m_hWnd != NULL ) DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CCoolTipCtrl operations

BOOL CCoolTipCtrl::Create(CWnd* pParentWnd, BOOL* pbEnable)
{
	CRect rc( 0, 0, 0, 0 );

	DWORD dwStyleEx = WS_EX_TOPMOST | ( theApp.m_bRTL ? WS_EX_LAYOUTRTL : 0 );
	if ( ! CWnd::CreateEx( dwStyleEx, m_hClass, NULL, WS_POPUP|WS_DISABLED,
		rc, pParentWnd, 0, NULL ) ) return FALSE;

	SetOwner( pParentWnd );
	m_pbEnable = pbEnable;

	return TRUE;
}

void CCoolTipCtrl::Show(void* pContext, HWND hAltWnd)
{
	if ( pContext == NULL ) return;
	if ( AfxGetMainWnd() != GetForegroundWindow() ) return;
	if ( m_pbEnable != NULL && *m_pbEnable == 0 ) return;

	CPoint point;
	GetCursorPos( &point );

	m_hAltWnd = hAltWnd;

	if ( m_bVisible )
	{
		if ( pContext == m_pContext ) return;

		Hide();

		m_pContext = pContext;

		ShowImpl();
	}
	else if ( point != m_pOpen )
	{
		m_pContext	= pContext;
		m_pOpen		= point;
		m_tOpen		= GetTickCount() + Settings.Interface.TipDelay;

		if ( ! m_bTimer )
		{
			SetTimer( 1, TIP_TIMER, NULL );
			m_bTimer = TRUE;
		}
	}
}

void CCoolTipCtrl::Hide()
{
	m_pContext	= NULL;
	m_tOpen		= 0;

	if ( m_bVisible )
	{
		OnHide();

		ShowWindow( SW_HIDE );
		ModifyStyleEx( WS_EX_LAYERED, 0 );
		m_bVisible = FALSE;
		GetCursorPos( &m_pOpen );
	}

	if ( m_bTimer )
	{
		KillTimer( 1 );
		m_bTimer = FALSE;
	}
}

void CCoolTipCtrl::ShowImpl()
{
	/*
	if ( m_bVisible ) return;

	m_sz.cx = m_sz.cy = 0;

	if ( ! OnPrepare() ) return;

	CRect rc( m_pOpen.x + TIP_OFFSET_X, m_pOpen.y + TIP_OFFSET_Y, 0, 0 );
	rc.right = rc.left + m_sz.cx + TIP_MARGIN * 2;
	rc.bottom = rc.top + m_sz.cy + TIP_MARGIN * 2;

	if ( rc.right >= GetSystemMetrics( SM_CXSCREEN ) )
	{
		rc.OffsetRect( GetSystemMetrics( SM_CXSCREEN ) - rc.right - 4, 0 );
	}

	if ( rc.bottom >= GetSystemMetrics( SM_CYSCREEN ) )
	{
		rc.OffsetRect( 0, - ( m_sz.cy + TIP_MARGIN * 2 + TIP_OFFSET_Y + 4 ) );
	}
	*/
	if ( m_bVisible ) return;

	m_sz.cx = m_sz.cy = 0;

	if ( ! OnPrepare() ) return;

	HMONITOR hMonitor = NULL;
	MONITORINFO mi = {0};
	CRect rcMonitor( 0, 0, 0, 0 );
	CRect rc( m_pOpen.x + TIP_OFFSET_X, m_pOpen.y + TIP_OFFSET_Y, 0, 0 );
	rc.right = rc.left + m_sz.cx + TIP_MARGIN * 2;
	rc.bottom = rc.top + m_sz.cy + TIP_MARGIN * 2;

	if ( ( theApp.m_dwWindowsVersion >= 5 ) && (GetSystemMetrics( SM_CMONITORS ) > 1) && (theApp.m_pfnMonitorFromRect) )
	{
		mi.cbSize = sizeof(MONITORINFO);

		hMonitor = theApp.m_pfnMonitorFromRect( rc, MONITOR_DEFAULTTONEAREST );
		//hMonitor = MonitorFromPoint( m_pOpen, MONITOR_DEFAULTTONEAREST );
		if (NULL != hMonitor)
		{
			if ( theApp.m_pfnGetMonitorInfoA(hMonitor, &mi) )
				rcMonitor = mi.rcWork;
			else
				hMonitor = NULL; // Fall back to GetSystemMetrics
		}

	}

	if ( NULL == hMonitor )
	{
		// Unimon system or something is wrong with multimon

		rcMonitor.right = GetSystemMetrics( SM_CXSCREEN );
		rcMonitor.bottom = GetSystemMetrics( SM_CYSCREEN );
	}

	if ( rc.right >= rcMonitor.right)
	{
		rc.OffsetRect( rcMonitor.right - rc.right - 4, 0 );
	}

	if ( rc.bottom >= rcMonitor.bottom )
	{
		rc.OffsetRect( 0, - ( m_sz.cy + TIP_MARGIN * 2 + TIP_OFFSET_Y + 4 ) );
	}


	m_bVisible = TRUE;

	OnShow();

	if ( Settings.Interface.TipAlpha == 255 || theApp.m_pfnSetLayeredWindowAttributes == NULL )
	{
		ModifyStyleEx( WS_EX_LAYERED, 0 );
	}
	else
	{
		ModifyStyleEx( 0, WS_EX_LAYERED );
		theApp.m_pfnSetLayeredWindowAttributes( GetSafeHwnd(),
			0, (BYTE)Settings.Interface.TipAlpha, LWA_ALPHA );
	}

	SetWindowPos( &wndTop, rc.left, rc.top, rc.Width(), rc.Height(),
		SWP_SHOWWINDOW|SWP_NOACTIVATE );
	UpdateWindow();

	if ( ! m_bTimer )
	{
		SetTimer( 1, TIP_TIMER, NULL );
		m_bTimer = TRUE;
	}
}

void CCoolTipCtrl::CalcSizeHelper()
{
	CClientDC dc( this );

	m_sz.cx = m_sz.cy = 0;

	CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntBold );

	OnCalcSize( &dc );

	dc.SelectObject( pOldFont );
}

void CCoolTipCtrl::AddSize(CDC* pDC, LPCTSTR pszText, int nBase)
{
	CSize szText = pDC->GetTextExtent( pszText, static_cast< int >( _tcslen( pszText ) ) );
	szText.cx += nBase;
	m_sz.cx = max( m_sz.cx, szText.cx );
}

void CCoolTipCtrl::GetPaintRect(RECT* pRect)
{
	pRect->left = 0;
	pRect->top = 0;
	pRect->right = m_sz.cx;
	pRect->bottom = m_sz.cy;
}

void CCoolTipCtrl::DrawText(CDC* pDC, POINT* pPoint, LPCTSTR pszText, int nBase)
{
	DWORD dwFlags = ( theApp.m_bRTL ? ETO_RTLREADING : 0 );
	CSize sz = pDC->GetTextExtent( pszText, static_cast< int >( _tcslen( pszText ) ) );

	if ( nBase ) pPoint->x += nBase;
	CRect rc( pPoint->x - 2, pPoint->y - 2, pPoint->x + sz.cx + 2, pPoint->y + sz.cy + 2 );

	pDC->SetBkColor( CoolInterface.m_crTipBack );
	pDC->ExtTextOut( pPoint->x, pPoint->y, ETO_CLIPPED|ETO_OPAQUE|dwFlags, &rc, pszText, static_cast< UINT >( _tcslen( pszText ) ), NULL );
	pDC->ExcludeClipRect( &rc );

	if ( nBase ) pPoint->x -= nBase;
}

void CCoolTipCtrl::DrawRule(CDC* pDC, POINT* pPoint, BOOL bPos)
{
	pPoint->y += 5;
	if ( bPos )
	{
		pDC->Draw3dRect( pPoint->x, pPoint->y,
			m_sz.cx + ( TIP_MARGIN - 3 ) - pPoint->x, 1, CoolInterface.m_crTipBorder,
			CoolInterface.m_crTipBorder );
		pDC->ExcludeClipRect( pPoint->x, pPoint->y,
			m_sz.cx + ( TIP_MARGIN - 3 ), pPoint->y + 1 );
	}
	else
	{
		pDC->Draw3dRect( -( TIP_MARGIN - 3 ), pPoint->y,
			m_sz.cx + ( TIP_MARGIN - 3 ) * 2, 1, CoolInterface.m_crTipBorder,
			CoolInterface.m_crTipBorder );
		pDC->ExcludeClipRect( -( TIP_MARGIN - 3 ), pPoint->y,
			m_sz.cx + ( TIP_MARGIN - 3 ), pPoint->y + 1 );
	}
	pPoint->y += 6;
}

BOOL CCoolTipCtrl::WindowFromPointBelongsToOwner(const CPoint& point)
{
	CRect rc;
	GetOwner()->GetWindowRect( &rc );

	if ( ! rc.PtInRect( point ) ) return FALSE;

	CWnd* pWnd = WindowFromPoint( point );

	while ( pWnd )
	{
		if ( pWnd == GetOwner() ) return TRUE;
		if ( m_hAltWnd != NULL && pWnd->GetSafeHwnd() == m_hAltWnd ) return TRUE;
		if ( !IsWindow( pWnd->m_hWnd ) ) return FALSE;
		pWnd = pWnd->GetParent();
	}

	return FALSE;
}

CLineGraph* CCoolTipCtrl::CreateLineGraph()
{
	CLineGraph* pGraph = new CLineGraph();

	pGraph->m_bShowLegend	= FALSE;
	pGraph->m_bShowAxis		= FALSE;
	pGraph->m_nMinGridVert	= 16;

	pGraph->m_crBack = CoolInterface.CalculateColour(
		RGB( 255, 255, 255 ), CoolInterface.m_crTipBack, 80 );
	pGraph->m_crGrid = CoolInterface.CalculateColour(
		CoolInterface.m_crTipBorder, pGraph->m_crBack, 180 );

	return pGraph;
}

/////////////////////////////////////////////////////////////////////////////
// CCoolTipCtrl message handlers

int CCoolTipCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	m_bTimer = FALSE;
	return 0;
}

void CCoolTipCtrl::OnDestroy()
{
	if ( m_bTimer ) KillTimer( 1 );
	m_bTimer = FALSE;
	if ( m_bVisible ) Hide();
	CWnd::OnDestroy();
}

BOOL CCoolTipCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CCoolTipCtrl::OnPaint()
{
	if ( ! IsWindow( GetSafeHwnd() ) || ! IsWindowVisible() ) return;

	CPaintDC dc( this );
	CRect rc;

	GetClientRect( &rc );

	CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntBold );

	dc.Draw3dRect( &rc, CoolInterface.m_crTipBorder, CoolInterface.m_crTipBorder );
	dc.SetViewportOrg( TIP_MARGIN, TIP_MARGIN );
	rc.DeflateRect( 1, 1 );

	OnPaint( &dc );

	dc.SetViewportOrg( 0, 0 );
	dc.FillSolidRect( &rc, CoolInterface.m_crTipBack );
	dc.SelectObject( pOldFont );
}

void CCoolTipCtrl::OnMouseMove(UINT /*nFlags*/, CPoint /*point*/)
{
	Hide();
}

void CCoolTipCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Hide();
	CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CCoolTipCtrl::OnTimer(UINT_PTR /*nIDEvent*/)
{
	CPoint point;
	GetCursorPos( &point );

	if ( WindowFromPointBelongsToOwner( point ) )
	{
		CWnd* pWnd = GetForegroundWindow();

		if ( pWnd != this && pWnd != AfxGetMainWnd() )
		{
			if ( m_bVisible ) Hide();
			return;
		}
	}
	else
	{
		if ( m_bVisible ) Hide();
		return;
	}

	if ( ! m_bVisible && m_tOpen && GetTickCount() >= m_tOpen )
	{
		m_tOpen = 0;
		if ( point == m_pOpen || m_hAltWnd != NULL ) ShowImpl();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCoolTipCtrl events

BOOL CCoolTipCtrl::OnPrepare()
{
	return FALSE;
}

void CCoolTipCtrl::OnCalcSize(CDC* /*pDC*/)
{
}

void CCoolTipCtrl::OnShow()
{
}

void CCoolTipCtrl::OnHide()
{
}

void CCoolTipCtrl::OnPaint(CDC* /*pDC*/)
{
}
