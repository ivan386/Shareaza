//
// CoolMenu.cpp
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
#include "CoolInterface.h"
#include "CoolMenu.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CM_DISABLEDBLEND	ILD_BLEND25
#define CM_ICONWIDTH		16
#define CM_ICONHEIGHT		16

CCoolMenu CoolMenu;


//////////////////////////////////////////////////////////////////////
// CCoolMenu construction

CCoolMenu::CCoolMenu()
{
	m_nCheckIcon	= 0;
	m_bEnable		= TRUE;
	m_bUnhook		= FALSE;
}

CCoolMenu::~CCoolMenu()
{
	SetWatermark( NULL );
	if ( m_bUnhook ) EnableHook( FALSE );
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu modern version check

BOOL CCoolMenu::IsModernVersion()
{
	OSVERSIONINFO pVersion;
	pVersion.dwOSVersionInfoSize = sizeof(pVersion);
	GetVersionEx( &pVersion );

	return theApp.GetProfileInt( _T(""), _T("CoolMenuEnable"), TRUE ) &&
		( pVersion.dwMajorVersion >= 5 ||
		( pVersion.dwMajorVersion == 4 && pVersion.dwMinorVersion >= 10 ) );
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu add menu

BOOL CCoolMenu::AddMenu(CMenu* pMenu, BOOL bChild)
{
	if ( ! m_bEnable ) return FALSE;

	for ( int i = 0 ; i < (int)pMenu->GetMenuItemCount() ; i++ )
	{
		TCHAR szBuffer[128];
		MENUITEMINFO mii;

		ZeroMemory( &mii, sizeof(mii) );
		mii.cbSize		= sizeof(mii);
		mii.fMask		= MIIM_DATA|MIIM_ID|MIIM_TYPE|MIIM_SUBMENU;
		mii.dwTypeData	= szBuffer;
		mii.cch			= 128;

		GetMenuItemInfo( pMenu->GetSafeHmenu(), i, MF_BYPOSITION, &mii );

		if ( mii.fType & (MF_OWNERDRAW|MF_SEPARATOR) )
		{
			mii.fType |= MF_OWNERDRAW;
			if ( mii.fType & MF_SEPARATOR ) mii.dwItemData = 0;
			SetMenuItemInfo( pMenu->GetSafeHmenu(), i, MF_BYPOSITION, &mii );
			continue;
		}

		mii.fType		|= MF_OWNERDRAW;
		mii.dwItemData	= ( (DWORD)pMenu->GetSafeHmenu() << 16 ) | ( mii.wID & 0xFFFF );

		CString strText = szBuffer;
		m_pStrings.SetAt( mii.dwItemData, strText );

		if ( bChild ) SetMenuItemInfo( pMenu->GetSafeHmenu(), i, MF_BYPOSITION, &mii );

		if ( mii.hSubMenu != NULL ) AddMenu( pMenu->GetSubMenu( i ), TRUE );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu watermark

void CCoolMenu::SetWatermark(HBITMAP hBitmap)
{
	if ( m_bmWatermark.m_hObject != NULL )
	{
		m_dcWatermark.SelectObject( CBitmap::FromHandle( m_hOldMark ) );
		m_bmWatermark.DeleteObject();
		m_dcWatermark.DeleteDC();
	}

	if ( hBitmap != NULL )
	{
		CDC dc;
		dc.Attach( GetDC( 0 ) );
		if ( theApp.m_bRTL ) SetLayout( dc.m_hDC, LAYOUT_BITMAPORIENTATIONPRESERVED );
		m_dcWatermark.CreateCompatibleDC( &dc );
		ReleaseDC( 0, dc.Detach() );

		m_bmWatermark.Attach( hBitmap );
		m_hOldMark = (HBITMAP)m_dcWatermark.SelectObject( &m_bmWatermark )->GetSafeHandle();

		BITMAP pInfo;
		m_bmWatermark.GetBitmap( &pInfo );
		m_czWatermark.cx = pInfo.bmWidth;
		m_czWatermark.cy = pInfo.bmHeight;
	}
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu measure item

void CCoolMenu::OnMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if ( lpMeasureItemStruct->itemID == ID_SEPARATOR )
	{
		lpMeasureItemStruct->itemWidth	= 16;
		lpMeasureItemStruct->itemHeight	= 2;
	}
	else
	{
		CString strText;
		CDC dc;

		m_pStrings.Lookup( lpMeasureItemStruct->itemData, strText );

		dc.Attach( GetDC( 0 ) );

		CFont* pOld = (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );
		CSize sz = dc.GetTextExtent( strText );
		dc.SelectObject( pOld );

		ReleaseDC( 0, dc.Detach() );

		lpMeasureItemStruct->itemWidth	= sz.cx + 32;
		lpMeasureItemStruct->itemHeight	= 23;
	}

	if ( m_hMsgHook == NULL ) lpMeasureItemStruct->itemHeight ++;
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu draw item

void CCoolMenu::OnDrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rcItem, rcText;
	CString strText;
	int nIcon = -1;
	CDC dc;

	BOOL	bSelected	= lpDrawItemStruct->itemState & ODS_SELECTED;
	BOOL	bChecked	= lpDrawItemStruct->itemState & ODS_CHECKED;
	BOOL	bDisabled	= lpDrawItemStruct->itemState & ODS_GRAYED;
	BOOL	bKeyboard	= FALSE;
	BOOL	bEdge		= TRUE;

	dc.Attach( lpDrawItemStruct->hDC );

	if ( CWnd* pWnd = dc.GetWindow() )
	{
		CRect rcScreen( &lpDrawItemStruct->rcItem );
		CPoint ptCursor;

		GetCursorPos( &ptCursor );
		pWnd->ClientToScreen( &rcScreen );

		bKeyboard = ! rcScreen.PtInRect( ptCursor );
	}

	rcItem.CopyRect( &lpDrawItemStruct->rcItem );
	rcItem.OffsetRect( -rcItem.left, -rcItem.top );
	if ( m_hMsgHook != NULL ) rcItem.bottom += ( bEdge = m_bPrinted );

	rcText.CopyRect( &rcItem );
	rcText.left += 32;
	rcText.right -= 2;

	CDC* pDC = CoolInterface.GetBuffer( dc, rcItem.Size() );

	if ( m_bmWatermark.m_hObject != NULL )
	{
		DrawWatermark( pDC, &rcItem, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top );
	}
	else
	{
		pDC->FillSolidRect( rcItem.left, rcItem.top, 24, rcItem.Height(), CoolInterface.m_crMargin );
		pDC->FillSolidRect( rcItem.left + 24, rcItem.top, rcItem.Width() - 24, rcItem.Height(), CoolInterface.m_crBackNormal );
	}

	if ( m_pStrings.Lookup( lpDrawItemStruct->itemData, strText ) == FALSE )
	{
		int nMiddle = rcText.top + 1;

		pDC->FillSolidRect( rcText.left, nMiddle, rcText.Width() + 2, 1, CoolInterface.m_crDisabled );

		dc.BitBlt( lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top,
			rcItem.Width(), rcItem.Height(), pDC, 0, 0, SRCCOPY );
		dc.Detach();

		return;
	}

	if ( bSelected )
	{
		if ( ! bDisabled )
		{
			pDC->Draw3dRect( rcItem.left + 1, rcItem.top + 1,
				rcItem.Width() - 2, rcItem.Height() - 1 - bEdge,
				CoolInterface.m_crBorder, CoolInterface.m_crBorder );
			pDC->FillSolidRect( rcItem.left + 2, rcItem.top + 2,
				rcItem.Width() - 4, rcItem.Height() - 3 - bEdge,
				CoolInterface.m_crBackSel );

			pDC->SetBkColor( CoolInterface.m_crBackSel );
		}
		else if ( bKeyboard )
		{
			pDC->Draw3dRect( rcItem.left + 1, rcItem.top + 1,
				rcItem.Width() - 2, rcItem.Height() - 1 - bEdge,
				CoolInterface.m_crBorder, CoolInterface.m_crBorder );
			pDC->FillSolidRect( rcItem.left + 2, rcItem.top + 2,
				rcItem.Width() - 4, rcItem.Height() - 3 - bEdge,
				CoolInterface.m_crBackNormal );

			pDC->SetBkColor( CoolInterface.m_crBackNormal );
		}
	}
	else
	{
		pDC->SetBkColor( CoolInterface.m_crBackNormal );
	}

	if ( bChecked )
	{
		pDC->Draw3dRect( rcItem.left + 2, rcItem.top + 2, 20, rcItem.Height() - 3 - bEdge,
			CoolInterface.m_crBorder, CoolInterface.m_crBorder );
		pDC->FillSolidRect( rcItem.left + 3, rcItem.top + 3, 18, rcItem.Height() - 5 - bEdge,
			( bSelected && !bDisabled ) ? CoolInterface.m_crBackCheckSel : CoolInterface.m_crBackCheck );
	}

	nIcon = CoolInterface.ImageForID( (DWORD)lpDrawItemStruct->itemID );

	if ( bChecked && nIcon < 0 ) nIcon = m_nCheckIcon;

	if ( nIcon >= 0 )
	{
		CPoint pt( rcItem.left + 4, rcItem.top + 4 );

		if ( bDisabled )
		{
			ImageList_DrawEx( CoolInterface.m_pImages.m_hImageList, nIcon, pDC->GetSafeHdc(),
				pt.x, pt.y, 0, 0, CLR_NONE, CoolInterface.m_crDisabled, CM_DISABLEDBLEND );
		}
		else if ( bChecked )
		{
			CoolInterface.m_pImages.Draw( pDC, nIcon, pt, ILD_NORMAL );
		}
		else if ( bSelected )
		{
			pt.Offset( 1, 1 );
			pDC->SetTextColor( CoolInterface.m_crShadow );
			CoolInterface.m_pImages.Draw( pDC, nIcon, pt, ILD_MASK );
			pt.Offset( -2, -2 );
			CoolInterface.m_pImages.Draw( pDC, nIcon, pt, ILD_NORMAL );
		}
		else
		{
			ImageList_DrawEx( CoolInterface.m_pImages.m_hImageList, nIcon, pDC->GetSafeHdc(),
				pt.x, pt.y, 0, 0, CLR_NONE, CoolInterface.m_crMargin, ILD_BLEND25 );
		}
	}

	CFont* pOld = (CFont*)pDC->SelectObject(
					( lpDrawItemStruct->itemState & ODS_DEFAULT ) && ! bDisabled ?
					&CoolInterface.m_fntBold : &CoolInterface.m_fntNormal );

	pDC->SetBkMode( TRANSPARENT );
	pDC->SetTextColor( bDisabled ? CoolInterface.m_crDisabled :
		( bSelected ? CoolInterface.m_crCmdTextSel : CoolInterface.m_crCmdText ) );
	DrawMenuText( pDC, &rcText, strText );

	pDC->SelectObject( pOld );

	dc.BitBlt( lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top,
		rcItem.Width(), rcItem.Height(), pDC, 0, 0, SRCCOPY );
	dc.Detach();
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu formatted text helper

void CCoolMenu::DrawMenuText(CDC* pDC, CRect* pRect, const CString& strText)
{
	theApp.m_bMenuWasVisible = TRUE;
	int nPos = strText.Find( '\t' );

	if ( nPos >= 0 )
	{
		pRect->right -= 8;
		pDC->DrawText( strText.Left( nPos ), pRect, DT_SINGLELINE|DT_VCENTER|DT_LEFT );
		pDC->DrawText( strText.Mid( nPos + 1 ), pRect, DT_SINGLELINE|DT_VCENTER|DT_RIGHT );
		pRect->right += 8;
	}
	else
	{
		pDC->DrawText( strText, pRect, DT_SINGLELINE|DT_VCENTER|DT_LEFT );
	}
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu watermark helper

void CCoolMenu::DrawWatermark(CDC* pDC, CRect* pRect, int nOffX, int nOffY)
{
	for ( int nY = pRect->top - nOffY ; nY < pRect->bottom ; nY += m_czWatermark.cy )
	{
		if ( nY + m_czWatermark.cy < pRect->top ) continue;

		for ( int nX = pRect->left - nOffX ; nX < pRect->right ; nX += m_czWatermark.cx )
		{
			if ( nX + m_czWatermark.cx < pRect->left ) continue;

			pDC->BitBlt( nX, nY, m_czWatermark.cx, m_czWatermark.cy, &m_dcWatermark, 0, 0, SRCCOPY );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu border effects

HHOOK	CCoolMenu::m_hMsgHook	= NULL;
LPCTSTR CCoolMenu::wpnOldProc	= _T("RAZA_MenuOldWndProc");
BOOL	CCoolMenu::m_bPrinted	= TRUE;
int		CCoolMenu::m_nEdgeLeft	= 0;
int		CCoolMenu::m_nEdgeTop	= 0;
int		CCoolMenu::m_nEdgeSize	= 0;

void CCoolMenu::EnableHook()
{
	ASSERT( m_hMsgHook == NULL );
	ASSERT( m_bUnhook == FALSE );

	m_bEnable = IsModernVersion();
	if ( ! m_bEnable ) return;

	m_bUnhook = TRUE;
	EnableHook( TRUE );
}

void CCoolMenu::EnableHook(BOOL bEnable)
{
	if ( bEnable == ( m_hMsgHook != NULL ) ) return;

	if ( bEnable )
	{
		m_hMsgHook = SetWindowsHookEx( WH_CALLWNDPROC, MsgHook,
			AfxGetInstanceHandle(), GetCurrentThreadId() );
	}
	else
	{
		UnhookWindowsHookEx( m_hMsgHook );
		m_hMsgHook = NULL;
	}
}

void CCoolMenu::RegisterEdge(int nLeft, int nTop, int nLength)
{
	m_nEdgeLeft	= nLeft;
	m_nEdgeTop	= nTop;
	m_nEdgeSize	= nLength;
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu message hook and subclassed window procedure

LRESULT CALLBACK CCoolMenu::MsgHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT* pCWP = (CWPSTRUCT*)lParam;

	while ( nCode == HC_ACTION )
	{
		if ( pCWP->message != WM_CREATE && pCWP->message != 0x01E2 ) break;

		TCHAR szClassName[16];
		int nClassName = GetClassName( pCWP->hwnd, szClassName, 16 );
		if ( nClassName != 6 || _tcscmp( szClassName, _T("#32768") ) != 0 ) break;

		if ( ::GetProp( pCWP->hwnd, wpnOldProc ) != NULL ) break;

		HWND hWndFore = GetForegroundWindow();
		if ( hWndFore != NULL && CWnd::FromHandlePermanent( hWndFore ) == NULL ) break;

		WNDPROC pWndProc = (WNDPROC)(LONG_PTR)::GetWindowLong( pCWP->hwnd, GWL_WNDPROC );
		if ( pWndProc == NULL ) break;
		ASSERT( pWndProc != MenuProc );

		if ( ! SetProp( pCWP->hwnd, wpnOldProc, pWndProc ) ) break;

		if ( ! SetWindowLong( pCWP->hwnd, GWL_WNDPROC, (DWORD)(DWORD_PTR)MenuProc ) )
		{
			::RemoveProp( pCWP->hwnd, wpnOldProc );
			break;
		}

		break;
	}

	return CallNextHookEx( CCoolMenu::m_hMsgHook, nCode, wParam, lParam );
}

LRESULT CALLBACK CCoolMenu::MenuProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC pWndProc = (WNDPROC)::GetProp( hWnd, wpnOldProc );

	switch ( uMsg )
	{
	case WM_NCCALCSIZE:
		{
			NCCALCSIZE_PARAMS* pCalc = (NCCALCSIZE_PARAMS*)lParam;
			pCalc->rgrc[0].left ++;
			pCalc->rgrc[0].top ++;
			pCalc->rgrc[0].right --;
			pCalc->rgrc[0].bottom --;
		}
		return 0;

	case WM_WINDOWPOSCHANGING:
		if ( WINDOWPOS* pWndPos = (WINDOWPOS*)lParam )
		{
			DWORD nStyle	= GetWindowLong( hWnd, GWL_STYLE );
			DWORD nExStyle	= GetWindowLong( hWnd, GWL_EXSTYLE );
			CRect rc( 0, 0, 32, 32 );

			AdjustWindowRectEx( &rc, nStyle, FALSE, nExStyle );

			pWndPos->cx -= ( rc.Width() - 34 );
			pWndPos->cy -= ( rc.Height() - 34 ) - 1;

			if ( pWndPos->x != m_nEdgeLeft || pWndPos->y != m_nEdgeTop )
				pWndPos->x ++;
		}
		break;

	case WM_PRINT:
		if ( ( lParam & PRF_CHECKVISIBLE ) && ! IsWindowVisible( hWnd ) ) return 0;
		if ( lParam & PRF_NONCLIENT )
		{
			CWnd* pWnd = CWnd::FromHandle( hWnd );
			CDC* pDC = CDC::FromHandle( (HDC)wParam );
			CRect rc;

			pWnd->GetWindowRect( &rc );
			BOOL bEdge = ( rc.left == m_nEdgeLeft && rc.top == m_nEdgeTop );
			rc.OffsetRect( -rc.left, -rc.top );

			pDC->Draw3dRect( &rc, CoolInterface.m_crDisabled, CoolInterface.m_crDisabled );
			if ( bEdge ) pDC->FillSolidRect( rc.left + 1, rc.top, min( rc.Width(), m_nEdgeSize ) - 2, 1, CoolInterface.m_crBackNormal );
		}
		if ( lParam & PRF_CLIENT )
		{
			CWnd* pWnd = CWnd::FromHandle( hWnd );
			CDC* pDC = CDC::FromHandle( (HDC)wParam );
			CBitmap bmBuf, *pbmOld;
			CDC dcBuf;
			CRect rc;

			pWnd->GetClientRect( &rc );
			dcBuf.CreateCompatibleDC( pDC );
			bmBuf.CreateCompatibleBitmap( pDC, rc.Width(), rc.Height() );
			pbmOld = (CBitmap*)dcBuf.SelectObject( &bmBuf );

			m_bPrinted = TRUE;
			dcBuf.FillSolidRect( &rc, GetSysColor( COLOR_MENU ) );
			SendMessage( hWnd, WM_PRINTCLIENT, (WPARAM)dcBuf.GetSafeHdc(), 0 );

			pDC->BitBlt( 1, 1, rc.Width(), rc.Height(), &dcBuf, 0, 0, SRCCOPY );
			dcBuf.SelectObject( pbmOld );
		}
		return 0;

	case WM_NCPAINT:
		{
			CWnd* pWnd = CWnd::FromHandle( hWnd );
			CWindowDC dc( pWnd );
			CRect rc;

			pWnd->GetWindowRect( &rc );
			BOOL bEdge = ( rc.left == m_nEdgeLeft && rc.top == m_nEdgeTop );
			rc.OffsetRect( -rc.left, -rc.top );

			dc.Draw3dRect( &rc, CoolInterface.m_crDisabled, CoolInterface.m_crDisabled );
			if ( bEdge ) dc.FillSolidRect( rc.left + 1, rc.top, min( rc.Width(), m_nEdgeSize ) - 2, 1, CoolInterface.m_crBackNormal );
		}
		return 0;

	case WM_PAINT:
		m_bPrinted = FALSE;
		break;

	case WM_NCDESTROY:
		::RemoveProp( hWnd, wpnOldProc );
		break;
	}

	return CallWindowProc( pWndProc, hWnd, uMsg, wParam, lParam );
}
