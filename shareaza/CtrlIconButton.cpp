//
// CtrlIconButton.cpp
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
#include "CtrlIconButton.h"
#include "CoolInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CIconButtonCtrl, CWnd)

BEGIN_MESSAGE_MAP(CIconButtonCtrl, CWnd)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_ENABLE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CIconButtonCtrl construction

CIconButtonCtrl::CIconButtonCtrl()
{
	m_pImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 0 );

	m_bCapture	= FALSE;
	m_bDown		= FALSE;
	m_bCursor	= FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CIconButtonCtrl operations

BOOL CIconButtonCtrl::Create(const RECT& rect, CWnd* pParentWnd, UINT nControlID, DWORD dwStyle)
{
	if ( CWnd::CreateEx( 0, NULL, _T(""), dwStyle | WS_CHILD | WS_VISIBLE,
		rect, pParentWnd, nControlID ) )
	{
		// Fix
		GetParent()->IsDlgButtonChecked( nControlID );
		return TRUE;
	}
	return FALSE;
}

void CIconButtonCtrl::SetText(LPCTSTR pszText)
{
	CString strText;
	GetWindowText( strText );
	if ( strText == pszText ) return;
	SetWindowText( pszText );
	Invalidate();
}

void CIconButtonCtrl::SetIcon(HICON hIcon, BOOL bMirrored)
{
	if ( hIcon )
	{
		if ( bMirrored )
		{
			hIcon = CreateMirroredIcon( hIcon );
			ASSERT( hIcon != NULL );
		}
		if ( hIcon )
		{
			if ( m_pImageList.GetImageCount() )
			{
				ASSERT( m_pImageList.GetImageCount() == 1 );
				VERIFY( m_pImageList.Remove( 0 ) );
			}
			VERIFY( m_pImageList.Add( hIcon ) != -1 );
			VERIFY( DestroyIcon( hIcon ) );

			RemoveStyle();
		}
	}
}

void CIconButtonCtrl::SetCoolIcon(UINT nIconID, BOOL bMirrored)
{
	SetIcon( CoolInterface.ExtractIcon( nIconID, bMirrored ), FALSE );
}

void CIconButtonCtrl::SetIcon(UINT nIconID, BOOL bMirrored)
{
	SetIcon( AfxGetApp()->LoadIcon( nIconID ), bMirrored );
}

void CIconButtonCtrl::SetHandCursor(BOOL bCursor)
{
	m_bCursor = bCursor;
}

BOOL CIconButtonCtrl::RemoveStyle()
{
	return CoolInterface.EnableTheme( this, FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CIconButtonCtrl mouse message handlers

void CIconButtonCtrl::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if ( ! IsWindowEnabled() ) return;

	CRect rc;
	GetClientRect( &rc );

	if ( m_bDown )
	{
		BOOL bInside = rc.PtInRect( point );
		if ( bInside == m_bCapture ) return;

		m_bCapture = bInside;
	}
	else if ( m_bCapture )
	{
		if ( rc.PtInRect( point ) ) return;

		ReleaseCapture();
		m_bCapture = FALSE;
	}
	else
	{
		SetCapture();
		if ( m_bCursor ) ::SetCursor( theApp.LoadCursor( IDC_HAND ) );
		m_bCapture = TRUE;
	}

	Invalidate();
}

void CIconButtonCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	if ( ! IsWindowEnabled() ) return;

	SetFocus();
	SetCapture();
	m_bDown = TRUE;

	Invalidate();
}

void CIconButtonCtrl::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	if ( m_bDown )
	{
		ReleaseCapture();
		m_bDown = FALSE;

		if ( m_bCapture )
		{
			RedrawWindow();

			GetParent()->SendMessage( WM_COMMAND, MAKELONG( GetDlgCtrlID(), 0 ),
				(LPARAM)GetSafeHwnd() );
		}

		m_bCapture = FALSE;

		Invalidate();
	}
}

void CIconButtonCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown( nFlags, point );
}

/////////////////////////////////////////////////////////////////////////////
// CIconButtonCtrl paint message handlers

BOOL CIconButtonCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CIconButtonCtrl::OnPaint()
{
	CPaintDC dc( this );
	COLORREF crBack;
	CString strText;
	CPoint ptIcon;
	CRect rc;

	GetClientRect( &rc );
	GetWindowText( strText );

	if ( strText.GetLength() )
	{
		ptIcon.x = rc.left + 3;
	}
	else
	{
		ptIcon.x = ( rc.left + rc.right ) / 2 - 8;
	}

	ptIcon.y = ( rc.top + rc.bottom ) / 2 - 8;

	if ( m_bDown && m_bCapture )
	{
		crBack = CoolInterface.m_crBackCheckSel;
		dc.Draw3dRect( &rc, CoolInterface.m_crBorder, CoolInterface.m_crBorder );
		rc.DeflateRect( 1, 1 );

		ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
			ptIcon.x, ptIcon.y, 0, 0, crBack, CLR_NONE, ILD_NORMAL );
		dc.ExcludeClipRect( ptIcon.x, ptIcon.y, ptIcon.x + 16, ptIcon.y + 16 );
	}
	else if ( m_bDown != m_bCapture )
	{
		crBack = CoolInterface.m_crBackSel;
		dc.Draw3dRect( &rc, CoolInterface.m_crBorder, CoolInterface.m_crBorder );
		rc.DeflateRect( 1, 1 );

		ptIcon.Offset( -1, -1 );
		dc.FillSolidRect( ptIcon.x, ptIcon.y, 18, 2, crBack );
		dc.FillSolidRect( ptIcon.x, ptIcon.y + 2, 2, 16, crBack );

		ptIcon.Offset( 2, 2 );
		dc.SetTextColor( CoolInterface.m_crShadow );
		ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
			ptIcon.x, ptIcon.y, 0, 0, crBack, CLR_NONE, ILD_MASK );

		ptIcon.Offset( -2, -2 );
		ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
			ptIcon.x, ptIcon.y, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL );

		dc.ExcludeClipRect( ptIcon.x, ptIcon.y, ptIcon.x + 18, ptIcon.y + 18 );
		ptIcon.Offset( 1, 1 );
	}
	else if ( GetFocus() == this && IsWindowEnabled() )
	{
		crBack = CoolInterface.m_crBackNormal;
		dc.Draw3dRect( &rc, CoolInterface.m_crBorder, CoolInterface.m_crBorder );
		rc.DeflateRect( 1, 1 );

		ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
			ptIcon.x, ptIcon.y, 0, 0, crBack, CLR_NONE, ILD_NORMAL );
		dc.ExcludeClipRect( ptIcon.x, ptIcon.y, ptIcon.x + 16, ptIcon.y + 16 );
	}
	else if ( IsWindowEnabled() )
	{
		crBack = CoolInterface.m_crBackNormal;
		dc.Draw3dRect( &rc, CoolInterface.m_crShadow, CoolInterface.m_crShadow );
		rc.DeflateRect( 1, 1 );

		ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
			ptIcon.x, ptIcon.y, 0, 0, crBack, CoolInterface.m_crShadow, ILD_BLEND50 );
		dc.ExcludeClipRect( ptIcon.x, ptIcon.y, ptIcon.x + 16, ptIcon.y + 16 );
	}
	else
	{
		crBack = CoolInterface.m_crMidtone;
		dc.Draw3dRect( &rc, CoolInterface.m_crShadow, CoolInterface.m_crShadow );
		rc.DeflateRect( 1, 1 );

		dc.SetTextColor( CoolInterface.m_crDisabled );
		dc.SetBkColor( crBack );

//		ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
//			ptIcon.x, ptIcon.y, 0, 0, crBack, CLR_NONE, ILD_MASK );

		ImageList_DrawEx( m_pImageList.m_hImageList, 0, dc.GetSafeHdc(),
			ptIcon.x, ptIcon.y, 0, 0, crBack, CoolInterface.m_crDisabled, ILD_BLEND50 );

		dc.ExcludeClipRect( ptIcon.x, ptIcon.y, ptIcon.x + 16, ptIcon.y + 16 );
	}

	if ( strText.GetLength() )
	{
		rc.left += 21;

		CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );

		dc.SetBkColor( crBack );
		dc.SetTextColor( IsWindowEnabled() ? CoolInterface.m_crCmdText : CoolInterface.m_crDisabled );
		dc.ExtTextOut( rc.left + 2, ptIcon.y + 1, ETO_CLIPPED|ETO_OPAQUE, &rc, strText, NULL );
		dc.SelectObject( pOldFont );

		rc.right = rc.left;
		rc.left -= 21;
	}

	dc.FillSolidRect( &rc, crBack );
}

void CIconButtonCtrl::OnEnable(BOOL bEnable)
{
	CWnd::OnEnable( bEnable );
	Invalidate();
}

void CIconButtonCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus( pOldWnd );
	Invalidate();
}

void CIconButtonCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus( pNewWnd );
	Invalidate();
}

UINT CIconButtonCtrl::OnGetDlgCode()
{
	return ( GetStyle() & BS_DEFPUSHBUTTON ) ? DLGC_DEFPUSHBUTTON : DLGC_UNDEFPUSHBUTTON;
}

BOOL CIconButtonCtrl::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_SPACE )
	{
		GetParent()->PostMessage( WM_COMMAND, MAKELONG( GetDlgCtrlID(), BN_CLICKED ), (LPARAM)GetSafeHwnd() );
		return TRUE;
	}
	return CWnd::PreTranslateMessage( pMsg );
}
