//
// CtrlMonitorBar.cpp
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
#include "Settings.h"
#include "GraphItem.h"
#include "CoolInterface.h"
#include "CtrlMonitorBar.h"
#include "Skin.h"

BEGIN_MESSAGE_MAP(CMonitorBarCtrl, CControlBar)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl construction

CMonitorBarCtrl::CMonitorBarCtrl()
{
	m_pSnapBar[0]	= NULL;
	m_pSnapBar[1]	= NULL;
	m_pTxItem		= new CGraphItem( GRC_TOTAL_BANDWIDTH_OUT, 0, RGB( 0xFF, 0xFF, 0 ) );
	m_pRxItem		= new CGraphItem( GRC_TOTAL_BANDWIDTH_IN, 0, RGB( 0, 0xFF, 0 ) );
	m_nMaximum		= 0;
	m_nCount		= 0;
	m_bTab			= FALSE;
}

CMonitorBarCtrl::~CMonitorBarCtrl()
{
	delete m_pRxItem;
	delete m_pTxItem;
}

/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl system message handlers

BOOL CMonitorBarCtrl::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	CRect rc( 0, 0, 0, 0 );
	dwStyle |= WS_CHILD|WS_CLIPCHILDREN;
	return CWnd::Create( NULL, NULL, dwStyle, rc, pParentWnd, nID, NULL );
}

int CMonitorBarCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CControlBar::OnCreate( lpCreateStruct ) == -1 ) return -1;
	m_dwStyle |= CBRS_BORDER_3D;

	if ( lpCreateStruct->dwExStyle & WS_EX_LAYOUTRTL )
	{
		lpCreateStruct->dwExStyle ^= WS_EX_LAYOUTRTL;
		SetWindowLong( this->m_hWnd, GWL_EXSTYLE, lpCreateStruct->dwExStyle );
	}

	m_hTab = (HICON)LoadImage( AfxGetResourceHandle(),
		MAKEINTRESOURCE(IDI_POINTER_ARROW), IMAGE_ICON, 16, 16, 0 );

	OnSkinChange();

	SetTimer( 1, 50, NULL );

	return 0;
}

void CMonitorBarCtrl::OnDestroy()
{
	KillTimer( 1 );
	CControlBar::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl layout message handlers

CSize CMonitorBarCtrl::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize size( 128, 30 );

	for ( int nSnap = 1 ; nSnap >= 0 ; nSnap-- )
	{
		if ( m_pSnapBar[ nSnap ] != NULL && m_pSnapBar[ nSnap ]->IsVisible() )
		{
			size.cy = m_pSnapBar[ nSnap ]->CalcFixedLayout( FALSE, TRUE ).cy;
			break;
		}
	}

	return size;
}

void CMonitorBarCtrl::OnSize(UINT nType, int cx, int cy)
{
	CControlBar::OnSize( nType, cx, cy );
	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl timer

void CMonitorBarCtrl::OnTimer(UINT nIDEvent)
{
	if ( m_nCount++ & 1 )
	{
		m_pTxItem->Update();
		m_pRxItem->Update();

		m_nMaximum		= m_pTxItem->GetMaximum();
		DWORD nSecond	= m_pRxItem->GetMaximum();
		m_nMaximum = max( m_nMaximum, nSecond );
	}

	m_nMaximum = max( m_nMaximum, Settings.Connection.InSpeed  * 1024 );
	m_nMaximum = max( m_nMaximum, Settings.Connection.OutSpeed * 1024 );

	if ( IsWindowVisible() ) Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl display

void CMonitorBarCtrl::OnSkinChange()
{
	HBITMAP hWatermark = Skin.GetWatermark( _T("CMonitorBar") );
	if ( m_bmWatermark.m_hObject != NULL ) m_bmWatermark.DeleteObject();
	if ( hWatermark != NULL ) m_bmWatermark.Attach( hWatermark );

	if ( m_hWnd != NULL && IsWindowVisible() ) Invalidate();
}

void CMonitorBarCtrl::DoPaint(CDC* pDC)
{
	CRect rcClient;
	GetClientRect( &rcClient );

	CDC* pMemDC = CoolInterface.GetBuffer( *pDC, rcClient.Size() );
	if ( theApp.m_bRTL ) theApp.m_pfnSetLayout( pMemDC->m_hDC, 0 );

	if ( ! CoolInterface.DrawWatermark( pMemDC, &rcClient, &m_bmWatermark ) )
		pMemDC->FillSolidRect( &rcClient, CoolInterface.m_crMidtone );
	DrawBorders( pMemDC, rcClient );

	for ( int nY = rcClient.top + 4 ; nY < rcClient.bottom - 4 ; nY += 2 )
	{
		pMemDC->Draw3dRect( rcClient.left + 3, nY, 4, 1,
			CoolInterface.m_crDisabled, CoolInterface.m_crDisabled );
	}

	m_pTxItem->SetHistory( rcClient.Width(), TRUE );
	m_pRxItem->SetHistory( rcClient.Width(), TRUE );

	CRect rcHistory( rcClient.left + 10, rcClient.top + 2, rcClient.right - 15, rcClient.bottom - 6 );
	PaintHistory( pMemDC, &rcHistory );

	CRect rcCurrent( rcClient.right - 7, rcClient.top + 2, rcClient.right - 2, rcClient.bottom - 6 );
	PaintCurrent( pMemDC, &rcCurrent, m_pRxItem );
	rcCurrent.OffsetRect( -6, 0 );
	PaintCurrent( pMemDC, &rcCurrent, m_pTxItem );

	m_rcTrack.SetRect( rcClient.left + 6, rcClient.bottom - 8, rcClient.right, rcClient.bottom - 2 );
	PaintTab( pMemDC );

	GetClientRect( &rcClient );
	pDC->BitBlt( rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(),
		pMemDC, 0, 0, SRCCOPY );
	if ( theApp.m_bRTL ) theApp.m_pfnSetLayout( pMemDC->m_hDC, LAYOUT_RTL );
}

/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl painting components

void CMonitorBarCtrl::PaintHistory(CDC* pDC, CRect* prc)
{
	CRect rc( prc );

	pDC->Draw3dRect( &rc, GetSysColor( COLOR_3DSHADOW ), GetSysColor( COLOR_3DHIGHLIGHT ) );
	rc.DeflateRect( 1, 1 );
	pDC->FillSolidRect( &rc, Settings.Live.BandwidthScale > 100 ? RGB( 80, 0, 0 ) : 0 );

	if ( m_bTab )
	{
		CString str;

		if ( Settings.Live.BandwidthScale > 100 )
		{
			str = _T("MAX");
		}
		else
		{
			DWORD nRate = max( Settings.Connection.InSpeed, Settings.Connection.OutSpeed );
			nRate = nRate * Settings.Live.BandwidthScale / 100;
			str.Format( _T("%s (%i%%)"),
				(LPCTSTR)Settings.SmartVolume( nRate, TRUE, TRUE ),
				Settings.Live.BandwidthScale );
		}

		CFont* pfOld = (CFont*)pDC->SelectObject( &CoolInterface.m_fntNormal );
		pDC->SetBkMode( TRANSPARENT );
		pDC->SetTextColor( RGB( 255, 0, 0 ) );
		pDC->DrawText( str, &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE );
		pDC->SelectObject( pfOld );
		return;
	}

	if ( m_nMaximum == 0 ) return;

	DWORD nMax = min( m_pTxItem->m_nLength, (DWORD)rc.Width() );
	int nX = rc.right - 1;

	for ( DWORD nPos = 0 ; nPos < nMax ; nPos++, nX-- )
	{
		DWORD nTxValue = m_pTxItem->GetValueAt( nPos );
		DWORD nRxValue = m_pRxItem->GetValueAt( nPos );

		nTxValue = rc.bottom - nTxValue * rc.Height() / m_nMaximum;
		nRxValue = rc.bottom - nRxValue * rc.Height() / m_nMaximum;

		if ( nTxValue < nRxValue )
		{
			if ( nTxValue < (DWORD)rc.bottom )
			{
				pDC->FillSolidRect( nX, nTxValue + 1, 1, rc.bottom - nTxValue - 1, RGB( 0xBB, 0xBB, 0 ) );
				pDC->SetPixel( nX, nTxValue, RGB( 0xFF, 0xFF, 0 ) );
			}
			if ( nRxValue < (DWORD)rc.bottom )
			{
				pDC->FillSolidRect( nX, nRxValue + 1, 1, rc.bottom - nRxValue - 1, RGB( 0, 0xBB, 0 ) );
				pDC->SetPixel( nX, nRxValue, RGB( 0, 0xFF, 0 ) );
			}
		}
		else
		{
			if ( nRxValue < (DWORD)rc.bottom )
			{
				pDC->FillSolidRect( nX, nRxValue + 1, 1, rc.bottom - nRxValue - 1, RGB( 0, 0xBB, 0 ) );
				pDC->SetPixel( nX, nRxValue, RGB( 0, 0xFF, 0 ) );
			}
			if ( nTxValue < (DWORD)rc.bottom )
			{
				pDC->FillSolidRect( nX, nTxValue + 1, 1, rc.bottom - nTxValue - 1, RGB( 0xBB, 0xBB, 0 ) );
				pDC->SetPixel( nX, nTxValue, RGB( 0xFF, 0xFF, 0 ) );
			}
		}
	}
}

void CMonitorBarCtrl::PaintCurrent(CDC* pDC, CRect* prc, CGraphItem* pItem)
{
	CRect rc( prc );

	pDC->Draw3dRect( &rc, GetSysColor( COLOR_3DSHADOW ), GetSysColor( COLOR_3DHIGHLIGHT ) );
	rc.DeflateRect( 1, 1 );
	pDC->FillSolidRect( &rc, Settings.Live.BandwidthScale > 100 ? RGB( 80, 0, 0 ) : 0 );

	if ( m_nMaximum == 0 || pItem->m_nLength < 1 ) return;

	DWORD nValue = (DWORD)pItem->GetValue( pItem->m_nCode );
	nValue = nValue * rc.Height() / m_nMaximum;
	pDC->FillSolidRect( rc.left, rc.bottom - nValue, rc.Width(), nValue, pItem->m_nColour );
}

void CMonitorBarCtrl::PaintTab(CDC* pDC)
{
	float nPosition = 0;

	if ( Settings.Live.BandwidthScale > 100 )
		nPosition = 1.0f;
	else
		nPosition = (float)Settings.Live.BandwidthScale / 110.0f;

	m_rcTab.left	= m_rcTrack.left + (int)( nPosition * ( m_rcTrack.Width() - 16 ) );
	m_rcTab.right	= m_rcTab.left + 16;
	m_rcTab.top		= m_rcTrack.top;
	m_rcTab.bottom	= m_rcTrack.bottom;

	DrawIconEx( pDC->GetSafeHdc(), m_rcTab.left, m_rcTab.top, m_hTab, 16, 16, 0, NULL, DI_NORMAL );
}

/////////////////////////////////////////////////////////////////////////////
// CMonitorBarCtrl tracking

BOOL CMonitorBarCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	GetCursorPos( &point );
	ScreenToClient( &point );

	if ( m_rcTrack.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}
	else
	{
		return CControlBar::OnSetCursor( pWnd, nHitTest, message );
	}
}

void CMonitorBarCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ( m_rcTrack.PtInRect( point ) )
	{
		MSG* pMsg = &AfxGetThreadState()->m_msgCur;
		CRect rcTrack( &m_rcTrack );
		CPoint point;

		ClientToScreen( &rcTrack );
		ClipCursor( &rcTrack );
		ScreenToClient( &rcTrack );

		rcTrack.DeflateRect( m_rcTab.Width() / 2, 0 );

		m_bTab = TRUE;
		Invalidate();

		while ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
		{
			while ( ::PeekMessage( pMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) );

			if ( ! AfxGetThread()->PumpMessage() )
			{
				AfxPostQuitMessage( 0 );
				break;
			}

			GetCursorPos( &point );
			ScreenToClient( &point );

			int nPosition = (DWORD)( 110.0f * (float)( point.x - rcTrack.left ) / (float)rcTrack.Width() );
			if ( nPosition < 0 ) nPosition = 0;
			else if ( nPosition >= 105 ) nPosition = 101;
			else if ( nPosition >= 100 ) nPosition = 100;

			if ( nPosition != (int)Settings.Live.BandwidthScale )
			{
				Settings.Live.BandwidthScale = (DWORD)nPosition;
				Invalidate();
			}
		}

		m_bTab = FALSE;
		ReleaseCapture();
		ClipCursor( NULL );
		Invalidate();
	}
	else
	{
		CControlBar::OnLButtonDown( nFlags, point );
	}
}
