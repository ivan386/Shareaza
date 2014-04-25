//
// WndPanel.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "WndPanel.h"
#include "CoolInterface.h"
#include "Skin.h"
#include "SkinWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPanelWnd, CChildWnd)

BEGIN_MESSAGE_MAP(CPanelWnd, CChildWnd)
	//{{AFX_MSG_MAP(CPanelWnd)
	ON_WM_NCPAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCACTIVATE()
	ON_WM_SIZE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()

#define CAPTION_HEIGHT	20
#define CLOSEBOX		10


/////////////////////////////////////////////////////////////////////////////
// CPanelWnd construction

CPanelWnd::CPanelWnd(BOOL bTabMode, BOOL bGroupMode)
{
	m_bPanelMode = Settings.General.GUIMode != GUI_WINDOWED;

	if ( m_bPanelMode )
	{
		m_bTabMode |= bTabMode;
		m_bGroupMode |= bGroupMode;
	}

	m_bPanelClose = ( m_bPanelMode && ! m_bTabMode );
}

void CPanelWnd::OnSkinChange()
{
	// Disable theme in tab mode
	CoolInterface.EnableTheme( this, ( Skin.GetWindowSkin( this ) == NULL ) &&
		( Settings.General.GUIMode == GUI_WINDOWED ) );

	CChildWnd::OnSkinChange();
}

/////////////////////////////////////////////////////////////////////////////
// CPanelWnd message handlers

void CPanelWnd::OnSize(UINT nType, int cx, int cy)
{
	if ( m_bPanelMode && ! m_pSkin && ! IsIconic() )
	{
		CRect rc;
		GetWindowRect( &rc );
		rc.OffsetRect( -rc.left, -rc.top );
		rc.right++; rc.bottom++;
		SetWindowRgn( CreateRectRgnIndirect( &rc ), TRUE );
	}

	CChildWnd::OnSize( nType, cx, cy );
}

void CPanelWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	if ( m_bPanelMode && m_pSkin == NULL )
	{
		NCCALCSIZE_PARAMS* pSize = (NCCALCSIZE_PARAMS*)lpncsp;
			
		BITMAP info = { 0, 0, CAPTION_HEIGHT };
		if ( Skin.m_bmPanelMark.m_hObject) Skin.m_bmPanelMark.GetBitmap( &info );

		pSize->rgrc[0].top += info.bmHeight;
		return;
	}

	CChildWnd::OnNcCalcSize( bCalcValidRects, lpncsp );
}

LRESULT CPanelWnd::OnNcHitTest(CPoint point)
{
	if ( m_bPanelMode && ! m_pSkin )
	{
		BITMAP info = { 0, 0, CAPTION_HEIGHT };
		if ( Skin.m_bmPanelMark.m_hObject) Skin.m_bmPanelMark.GetBitmap( &info );

		CRect rc;
		GetWindowRect( &rc );
		rc.bottom = rc.top + info.bmHeight;

		return rc.PtInRect( point ) ? HTCAPTION : HTCLIENT;
	}

	return CChildWnd::OnNcHitTest( point );
}

void CPanelWnd::OnNcPaint()
{
	if ( m_bPanelMode && ! m_pSkin )
	{
		CWindowDC dc( this );
		PaintCaption( dc );
	}
	else
	{
		CChildWnd::OnNcPaint();
	}
}

BOOL CPanelWnd::OnNcActivate(BOOL bActive)
{
	if ( m_bPanelMode && ! m_pSkin )
	{
		CWindowDC dc( this );
		PaintCaption( dc );
		return TRUE;
	}

	return CChildWnd::OnNcActivate( bActive );
}

LRESULT CPanelWnd::OnSetText(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if ( m_pSkin )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible ) ModifyStyle( WS_VISIBLE, 0 );
		LONG lResult = static_cast< DWORD >( Default() );
		if ( bVisible ) ModifyStyle( 0, WS_VISIBLE );

		if ( m_pSkin ) m_pSkin->OnSetText( this );

		return lResult;
	}
	else if ( m_bPanelMode )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible ) ModifyStyle( WS_VISIBLE, 0 );
		LONG lResult = static_cast< DWORD >( Default() );
		if ( bVisible ) ModifyStyle( 0, WS_VISIBLE );

		CWindowDC dc( this );
		PaintCaption( dc );

		return lResult;
	}
	else
	{
		return Default();
	}
}

void CPanelWnd::PaintCaption(CDC& dc)
{
	CString strCaption;
	CRect rc, rcWnd;
	
	BITMAP info = { 0, 0, CAPTION_HEIGHT };
	if ( Skin.m_bmPanelMark.m_hObject) Skin.m_bmPanelMark.GetBitmap( &info );

	GetWindowRect( &rcWnd );
	rc.SetRect( 0, 0, rcWnd.Width(), info.bmHeight );
	GetWindowText( strCaption );

	CSize size = rc.Size();
	CDC* pBuffer = CoolInterface.GetBuffer( dc, size );

	if ( ! CoolInterface.DrawWatermark( pBuffer, &rc, &Skin.m_bmPanelMark, 0, 0 ) )
	{
		pBuffer->FillSolidRect( &rc, Skin.m_crPanelBack );
	}

	int nIconY = rc.Height() / 2 - 8;
	DrawIconEx( pBuffer->GetSafeHdc(), 4, nIconY,
		GetIcon( FALSE ), 16, 16, 0, NULL, DI_NORMAL );

	CFont* pOldFont	= (CFont*)pBuffer->SelectObject( &CoolInterface.m_fntCaption );
	CSize szCaption	= pBuffer->GetTextExtent( strCaption );

	pBuffer->SetBkMode( TRANSPARENT );

	if ( Skin.m_crPanelBorder != CLR_NONE )
	{
		pBuffer->SetTextColor( Skin.m_crPanelBorder );
		pBuffer->ExtTextOut( 8 + 16 - 1, rc.Height() / 2 - szCaption.cy / 2 - 1,
			ETO_CLIPPED, &rc, strCaption, NULL );
		pBuffer->ExtTextOut( 8 + 16 + 1, rc.Height() / 2 - szCaption.cy / 2 - 1,
			ETO_CLIPPED, &rc, strCaption, NULL );
		pBuffer->ExtTextOut( 8 + 16, rc.Height() / 2 - szCaption.cy / 2 - 1 - 1,
			ETO_CLIPPED, &rc, strCaption, NULL );
		pBuffer->ExtTextOut( 8 + 16, rc.Height() / 2 - szCaption.cy / 2 - 1 + 1,
			ETO_CLIPPED, &rc, strCaption, NULL );
	}

	pBuffer->SetTextColor( Skin.m_crPanelText );
	pBuffer->ExtTextOut( 8 + 16, rc.Height() / 2 - szCaption.cy / 2 - 1,
		ETO_CLIPPED, &rc, strCaption, NULL );

	if ( m_bPanelClose )
	{
		pBuffer->SelectObject( &theApp.m_gdiFont );
		CString strText	= _T("Close");
		CSize szText	= pBuffer->GetTextExtent( strText );

		m_rcClose.SetRect( rc.right - szText.cx - 8, rc.top, rc.right, rc.bottom );
		pBuffer->ExtTextOut( m_rcClose.left + 2,
			( m_rcClose.top + m_rcClose.bottom ) / 2 - szText.cy / 2 - 1,
			ETO_CLIPPED, &m_rcClose, strText, NULL );
		m_rcClose.OffsetRect( rcWnd.left, rcWnd.top );
	}

	pBuffer->SelectObject( pOldFont );

	dc.BitBlt( rc.left, rc.top, rc.Width(), rc.Height(), pBuffer, 0, 0, SRCCOPY );

	dc.SelectStockObject( SYSTEM_FONT ); // GDI font leak fix
	dc.SelectStockObject( NULL_BRUSH );	 // GDI brush leak fix
}

BOOL CPanelWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( nHitTest == HTCAPTION && m_bGroupMode && m_pGroupParent )
	{
		SetCursor( AfxGetApp()->LoadStandardCursor( IDC_SIZENS ) );
		return TRUE;
	}
	else if ( nHitTest == HTCAPTION && m_bPanelClose && m_pSkin == NULL )
	{
		CPoint pt;
		GetCursorPos( &pt );

		if ( Settings.General.LanguageRTL )
		{
			CRect rc;
			pWnd->GetWindowRect( &rc );
			pt.x = 2 * rc.left + rc.Width() - pt.x;
		}

		if ( m_rcClose.PtInRect( pt ) )
		{
			SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
			return TRUE;
		}
	}

	return CChildWnd::OnSetCursor( pWnd, nHitTest, message );
}

void CPanelWnd::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	if ( nHitTest == HTCAPTION && m_bGroupMode && m_pGroupParent )
	{
		PanelSizeLoop();
		return;
	}
	else if ( nHitTest == HTCAPTION && m_bPanelClose && m_pSkin == NULL )
	{
		if ( Settings.General.LanguageRTL )
		{
			CRect rc;
			GetWindowRect( &rc );
			point.x = 2 * rc.left + rc.Width() - point.x;
		}

		if ( m_rcClose.PtInRect( point ) )
		{
			PostMessage( WM_SYSCOMMAND, SC_CLOSE );
			return;
		}
	}

	CChildWnd::OnNcLButtonDown( nHitTest, point );
}

void CPanelWnd::PanelSizeLoop()
{
	MSG* pMsg = &AfxGetThreadState()->m_msgCur;

	float nOffset = 10;
	CPoint point;
	CRect rcMDI;

	SendMessage( WM_ENTERSIZEMOVE );

	GetParent()->GetWindowRect( &rcMDI );
	GetParent()->SetCapture();
	ClipCursor( &rcMDI );

	while ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
	{
		while ( ::PeekMessage( pMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) );

		if ( ! AfxGetThread()->PumpMessage() )
		{
			AfxPostQuitMessage( 0 );
			break;
		}

		GetCursorPos( &point );
		if ( point.y < rcMDI.top ) point.y = rcMDI.top;
		if ( point.y > rcMDI.bottom ) point.y = rcMDI.bottom;

		float nSplitter = (float)( point.y - rcMDI.top ) / (float)rcMDI.Height();

		if ( nOffset == 10 ) nOffset = m_pGroupParent->m_nGroupSize - nSplitter;
		nSplitter += nOffset;

		if ( nSplitter < 0.1f ) nSplitter = 0.1f;
		if ( nSplitter > 0.9f ) nSplitter = 0.9f;

		if ( nSplitter >= 0.47f && nSplitter <= 0.53f ) nSplitter = 0.5f;

		if ( nSplitter != m_pGroupParent->m_nGroupSize )
		{
			m_pGroupParent->m_nGroupSize = nSplitter;
			GetParent()->SendMessage( WM_SIZE, 1982 );
		}
	}

	ReleaseCapture();
	ClipCursor( NULL );
	SendMessage( WM_EXITSIZEMOVE );
}
