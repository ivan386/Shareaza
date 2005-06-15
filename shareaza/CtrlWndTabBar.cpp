//
// CtrlWndTabBar.cpp
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
#include "CtrlWndTabBar.h"
#include "CoolInterface.h"
#include "CoolMenu.h"
#include "Skin.h"

#include "WndMain.h"
#include "WndPlugin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CWndTabBar, CControlBar)
	//{{AFX_MSG_MAP(CWndTabBar)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_MBUTTONUP()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWndTabBar construction

CWndTabBar::CWndTabBar()
{
	m_nMaximumWidth	= 140;
	m_nCookie		= 0;
	m_bTimer		= FALSE;
	m_bMenuGray		= FALSE;
	m_nMessage		= 0;
}

CWndTabBar::~CWndTabBar()
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		delete (TabItem*)m_pItems.GetNext( pos );
	}
	m_pItems.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CWndTabBar operations

BOOL CWndTabBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	CRect rc;
	dwStyle |= WS_CHILD;
	return CWnd::Create( NULL, NULL, dwStyle, rc, pParentWnd, nID, NULL );
}

void CWndTabBar::SetMessage(UINT nMessageID)
{
	if ( m_nMessage == nMessageID ) return;

	if ( m_nMessage = nMessageID )
		Skin.LoadString( m_sMessage, m_nMessage );
	else
		m_sMessage.Empty();

	if ( m_pItems.IsEmpty() ) Invalidate();
}

void CWndTabBar::SetMessage(LPCTSTR pszText)
{
	if ( m_sMessage == pszText ) return;
	m_nMessage = 0;
	m_sMessage = pszText;
	if ( m_pItems.IsEmpty() ) Invalidate();
}

void CWndTabBar::SetMaximumWidth(int nWidth)
{
	if ( m_nMaximumWidth == nWidth ) return;
	m_nMaximumWidth = nWidth;
	Invalidate();
}

void CWndTabBar::SetWatermark(HBITMAP hBitmap)
{
	if ( m_bmImage.m_hObject ) m_bmImage.DeleteObject();
	if ( hBitmap ) m_bmImage.Attach( hBitmap );

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		TabItem* pItem = (TabItem*)m_pItems.GetNext( pos );
		pItem->m_nImage = -1;
	}

	m_pIcons.RemoveAll();
	m_pImages.DeleteImageList();
	m_pImages.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 1 );
}

/////////////////////////////////////////////////////////////////////////////
// CWndTabBar message handlers

int CWndTabBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CControlBar::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_pImages.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 1 );

	m_mnuChild.LoadMenu( IDR_CHILDFRAME );
	CoolMenu.AddMenu( &m_mnuChild );

	m_dwStyle |= CBRS_BORDER_3D;

	return 0;
}

CSize CWndTabBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize size( 32767, 26 );

	if ( CWnd* pParent = AfxGetMainWnd() )
	{
		CRect rc;
		pParent->GetWindowRect( &rc );
		if ( rc.Width() > 32 ) size.cx = rc.Width() + 2;
	}

	return size;
}

void CWndTabBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	if ( ! IsWindow( m_hWnd ) ) return;
	if ( ! pTarget->IsKindOf( RUNTIME_CLASS(CMainWnd) ) ) return;

	CMainWnd* pMainWnd			= (CMainWnd*)pTarget;
	CWindowManager* pManager	= &pMainWnd->m_pWindows;

	BOOL bChanged = FALSE;
	m_nCookie++;

	CChildWnd* pActive = pManager->GetActive();

	if ( pActive && pActive->m_bGroupMode )
	{
		if ( pActive->m_pGroupParent ) pActive = pActive->m_pGroupParent;
	}

	for ( POSITION posChild = pManager->GetIterator() ; posChild ; )
	{
		CChildWnd* pChild = pManager->GetNext( posChild );

		BOOL bFound = FALSE;
		CString strCaption;

		if ( pChild->m_bTabMode ) continue;

		pChild->GetWindowText( strCaption );

		if ( pChild->m_bPanelMode )
		{
			if ( strCaption.Find( _T("Search : ") ) == 0 ) strCaption = strCaption.Mid( 9 );
			if ( strCaption.Find( _T("Browse Host : ") ) == 0 ) strCaption = strCaption.Mid( 14 );
		}

		for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
		{
			TabItem* pItem = (TabItem*)m_pItems.GetNext( pos );

			if ( pItem->m_hWnd == pChild->GetSafeHwnd() )
			{
				bFound = TRUE;
				pItem->m_nCookie = m_nCookie;

				if ( strCaption != pItem->m_sCaption )
				{
					pItem->m_nImage = ImageIndexForWindow( pChild );
					pItem->m_sCaption = strCaption;
					bChanged = TRUE;
				}
				else if ( pItem->m_nImage < 0 )
				{
					pItem->m_nImage = ImageIndexForWindow( pChild );
					bChanged = TRUE;
				}

				if ( pChild->IsWindowVisible() != pItem->m_bVisible )
				{
					pItem->m_bVisible = pChild->IsWindowVisible();
					bChanged = TRUE;
				}

				if ( pChild->m_bAlert != pItem->m_bAlert )
				{
					pItem->m_bAlert = pChild->m_bAlert;
					bChanged = TRUE;
				}

				if ( pChild == pActive && m_pSelected != pItem )
				{
					m_pSelected	= pItem;
					bChanged	= TRUE;
				}
				else if ( m_pSelected == pItem && pChild != pActive )
				{
					m_pSelected	= NULL;
					bChanged	= TRUE;
				}

				break;
			}
		}

		if ( bFound ) continue;

		TabItem* pItem = new TabItem( pChild, m_nCookie, strCaption );
		if ( pActive == pChild ) m_pSelected = pItem;
		pItem->m_nImage = ImageIndexForWindow( pChild );
		m_pItems.AddTail( pItem );
		bChanged = TRUE;
	}

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		POSITION posOld = pos;
		TabItem* pItem = (TabItem*)m_pItems.GetNext( pos );

		if ( pItem->m_nCookie != m_nCookie )
		{
			if ( m_pSelected == pItem ) m_pSelected = NULL;
			delete pItem;
			m_pItems.RemoveAt( posOld );
			bChanged = TRUE;
		}
	}

	if ( bChanged ) Invalidate();
}

int CWndTabBar::ImageIndexForWindow(CWnd* pChild)
{
	CRuntimeClass* pClass = pChild->GetRuntimeClass();
	WORD nImage;

	if ( pClass != RUNTIME_CLASS(CPluginWnd) )
	{
		if ( m_pIcons.Lookup( pClass, nImage ) ) return (int)nImage;
		nImage = m_pImages.Add( pChild->GetIcon( FALSE ) );
		m_pIcons.SetAt( pClass, nImage );
	}
	else
	{
		HICON hIcon = pChild->GetIcon( FALSE );
		if ( m_pIcons.Lookup( (LPVOID)hIcon, nImage ) ) return (int)nImage;
		nImage = m_pImages.Add( hIcon );
		m_pIcons.SetAt( (LPVOID)hIcon, nImage );
	}

	return (int)nImage;
}

CWndTabBar::TabItem* CWndTabBar::HitTest(const CPoint& point, CRect* pItemRect) const
{
	if ( m_pItems.IsEmpty() ) return NULL;

	CRect rc;
	GetClientRect( &rc );
	CalcInsideRect( rc, FALSE );

	rc.left -= m_cyTopBorder;
	rc.top -= m_cxLeftBorder;
	rc.right += m_cyBottomBorder;
	rc.bottom += m_cxRightBorder;

	CRect rcItem( rc.left + 3, rc.top + 1, 0, rc.bottom - 1 );
	rcItem.right = ( rc.Width() - 3 * m_pItems.GetCount() ) / m_pItems.GetCount() + 3;
	rcItem.right = min( rcItem.right, LONG(m_nMaximumWidth) );

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		TabItem* pItem = (TabItem*)m_pItems.GetNext( pos );
		if ( rcItem.PtInRect( point ) )
		{
			if ( pItemRect ) *pItemRect = rcItem;
			return pItem;
		}
		rcItem.OffsetRect( rcItem.Width() + 3, 0 );
	}

	return NULL;
}

int CWndTabBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	CRect rcItem;
	TabItem* pItem = HitTest( point, &rcItem );

	if ( pItem == NULL ) return -1;
	if ( ! pTI ) return 1;

	pTI->uFlags		= 0;
	pTI->hwnd		= GetSafeHwnd();
	pTI->uId		= (UINT)pItem->m_hWnd;
	pTI->rect		= rcItem;
	pTI->lpszText	= _tcsdup( pItem->m_sCaption );

	return pTI->uId;
}

void CWndTabBar::DoPaint(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CDC* pOutDC = pDC;
	CRect rc;

	GetClientRect( &rc );

	if ( m_bmImage.m_hObject != NULL )
	{
		pDC = CoolInterface.GetBuffer( *pDC, rc.Size() );
		CoolInterface.DrawWatermark( pDC, &rc, &m_bmImage );
	}

	DrawBorders( pDC, rc );

	CFont* pOldFont = (CFont*)pDC->SelectObject( &CoolInterface.m_fntNormal );

	if ( m_pItems.GetCount() )
	{
		m_nCloseImage = CoolInterface.ImageForID( ID_CHILD_CLOSE );

		CRect rcItem( rc.left + 3, rc.top + 1, 0, rc.bottom - 1 );
		rcItem.right = ( rc.Width() - 3 * m_pItems.GetCount() ) / m_pItems.GetCount() + 3;
		rcItem.right = min( rcItem.right, LONG(m_nMaximumWidth) );

		for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
		{
			TabItem* pItem = (TabItem*)m_pItems.GetNext( pos );

			pItem->Paint( this, pDC, &rcItem, m_pSelected == pItem, m_pHot == pItem,
				pDC != pOutDC );
			pDC->ExcludeClipRect( &rcItem );

			rcItem.OffsetRect( rcItem.Width() + 3, 0 );
		}

		if ( pDC == pOutDC ) pDC->FillSolidRect( &rc, CoolInterface.m_crMidtone );
	}
	else
	{
		CSize  sz = pDC->GetTextExtent( m_sMessage );
		CPoint pt = rc.CenterPoint();
		pt.x -= sz.cx / 2; pt.y -= sz.cy / 2 + 1;

		pDC->SetBkColor( CoolInterface.m_crMidtone );
		pDC->SetTextColor( CoolInterface.m_crDisabled );

		if ( pDC == pOutDC )
		{
			pDC->SetBkMode( OPAQUE );
			pDC->ExtTextOut( pt.x, pt.y, ETO_CLIPPED|ETO_OPAQUE, &rc, m_sMessage, NULL );
		}
		else
		{
			pDC->SetBkMode( TRANSPARENT );
			pDC->ExtTextOut( pt.x, pt.y, ETO_CLIPPED, &rc, m_sMessage, NULL );
		}

		m_rcMessage.SetRect( pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy );
	}

	pDC->SelectObject( pOldFont );

	if ( pDC != pOutDC )
	{
		GetClientRect( &rc );
		pOutDC->BitBlt( 0, 0, rc.Width(), rc.Height(), pDC, 0, 0, SRCCOPY );
		pDC->SelectClipRgn( NULL );
	}
}

void CWndTabBar::OnMouseMove(UINT nFlags, CPoint point)
{
	TabItem* pItem = HitTest( point );

	if ( pItem != m_pHot )
	{
		m_pHot = pItem;
		Invalidate();
	}

	if ( ! m_bTimer && m_pHot )
	{
		SetTimer( 1, 100, NULL );
		m_bTimer = TRUE;
	}
	else if ( m_bTimer && ! m_pHot )
	{
		KillTimer( 1 );
		m_bTimer = FALSE;
	}

	CControlBar::OnMouseMove( nFlags, point );
}

void CWndTabBar::OnTimer(UINT nIDEvent)
{
	if ( nIDEvent == 1 )
	{
		CRect rcWindow;
		CPoint point;

		GetClientRect( &rcWindow );
		ClientToScreen( &rcWindow );
		GetCursorPos( &point );

		if ( ! rcWindow.PtInRect( point ) )
		{
			KillTimer( nIDEvent );
			m_pHot		= NULL;
			m_bTimer	= FALSE;
			Invalidate();
		}
	}

	CControlBar::OnTimer( nIDEvent );
}

BOOL CWndTabBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( m_pItems.GetCount() == 0 && m_nMessage == IDS_TABBAR_CONNECTED  )
	{
		CPoint point;
		GetCursorPos( &point );
		ScreenToClient( &point );

		if ( m_rcMessage.PtInRect( point ) )
		{
			SetCursor( theApp.LoadCursor( IDC_HAND ) );
			return TRUE;
		}
	}

	return CControlBar::OnSetCursor( pWnd, nHitTest, message );
}

void CWndTabBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rc;

	if ( TabItem* pItem = HitTest( point, &rc ) )
	{
		CChildWnd* pChild = (CChildWnd*)CWnd::FromHandle( pItem->m_hWnd );

        if ( m_pSelected == pItem &&
			 Settings.General.GUIMode != GUI_WINDOWED &&
			 point.x >= rc.right - 18 )
		{
			pChild->PostMessage( WM_SYSCOMMAND, SC_CLOSE, 0 );
			return;
		}
		else if ( pChild->IsIconic() )
		{
			pChild->ShowWindow( SW_SHOWNORMAL );
		}
		else if ( m_pSelected == pItem && ! pChild->m_bPanelMode )
		{
			pChild->ShowWindow( SW_HIDE );
			pChild->ShowWindow( SW_MINIMIZE );
			return;
		}

		pChild->MDIActivate();

		return;
	}
	else if ( m_nMessage == IDS_TABBAR_CONNECTED )
	{
		if ( m_pItems.GetCount() == 0 && m_rcMessage.PtInRect( point ) )
		{
			return;
		}
	}

	CControlBar::OnLButtonDown( nFlags, point );
}

void CWndTabBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	/*
	if ( TabItem* pItem = HitTest( point ) )
	{
		CMDIChildWnd* pChild = (CMDIChildWnd*)CWnd::FromHandle( pItem->m_hWnd );
		pChild->PostMessage( WM_SYSCOMMAND, pChild->IsZoomed() ? SC_RESTORE : SC_MAXIMIZE );
		return;
	}
	*/

	OnLButtonDown( nFlags, point );
}

void CWndTabBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect rc;

	if ( TabItem* pItem = HitTest( point, &rc ) )
	{
	}
	else if ( m_nMessage == IDS_TABBAR_CONNECTED )
	{
		if ( m_pItems.GetCount() == 0 && m_rcMessage.PtInRect( point ) )
		{
			GetOwner()->PostMessage( WM_COMMAND, ID_NETWORK_SEARCH );
		}
	}

	CControlBar::OnLButtonUp( nFlags, point );
}

void CWndTabBar::OnMButtonUp(UINT nFlags, CPoint point)
{
	if ( TabItem* pItem = HitTest( point ) )
	{
		CChildWnd* pChild = (CChildWnd*)CWnd::FromHandle( pItem->m_hWnd );
		pChild->PostMessage( WM_SYSCOMMAND, SC_CLOSE, 0 );
	}

	CControlBar::OnMButtonUp( nFlags, point );
}

void CWndTabBar::OnRButtonUp(UINT nFlags, CPoint point)
{
	CRect rcItem;

	if ( TabItem* pItem = HitTest( point, &rcItem ) )
	{
		CChildWnd* pChild = (CChildWnd*)CWnd::FromHandle( pItem->m_hWnd );
		if ( pChild->IsWindowVisible() ) pChild->MDIActivate();

		ClientToScreen( &rcItem );

		CMenu* pMenu = m_mnuChild.GetSubMenu( pChild->m_bPanelMode ? 1 : 0 );

		BOOL bCanRestore	= pChild->IsIconic() || pChild->IsZoomed();
		UINT nCommand		= 0;

		/*
		MENUITEMINFO pInfo;
		pInfo.cbSize	= sizeof(pInfo);
		pInfo.fMask		= MIIM_STATE;
		GetMenuItemInfo( pMenu->GetSafeHmenu(), ID_CHILD_RESTORE, FALSE, &pInfo );
		pInfo.fState = ( pInfo.fState & (~MFS_DEFAULT) ) | ( bCanRestore ? MFS_DEFAULT : 0 );
		SetMenuItemInfo( pMenu->GetSafeHmenu(), ID_CHILD_RESTORE, FALSE, &pInfo );
		*/

		pMenu->EnableMenuItem( ID_CHILD_RESTORE, MF_BYCOMMAND |
			( bCanRestore ? 0 : MF_GRAYED ) );
		pMenu->EnableMenuItem( ID_CHILD_MINIMISE, MF_BYCOMMAND |
			( pChild->IsIconic() ? MF_GRAYED : 0 ) );
		pMenu->EnableMenuItem( ID_CHILD_MAXIMISE, MF_BYCOMMAND |
			( pChild->IsZoomed() ? MF_GRAYED : 0 ) );

		m_bMenuGray = TRUE;
		Invalidate();

		if ( rcItem.bottom > GetSystemMetrics( SM_CYSCREEN ) / 2 )
		{
			nCommand = pMenu->TrackPopupMenu( TPM_RETURNCMD|TPM_RIGHTBUTTON|
				TPM_LEFTALIGN|TPM_BOTTOMALIGN, theApp.m_bRTL ? rcItem.right : rcItem.left, 
				rcItem.top + 1, this, NULL );
		}
		else
		{
			CoolMenu.RegisterEdge( theApp.m_bRTL ? rcItem.right : rcItem.left, 
				rcItem.bottom - 1, rcItem.Width() );
			nCommand = pMenu->TrackPopupMenu( TPM_RETURNCMD|TPM_RIGHTBUTTON|
				TPM_LEFTALIGN|TPM_TOPALIGN, theApp.m_bRTL ? rcItem.right : rcItem.left, 
				rcItem.bottom - 1, this, NULL );
		}

		m_bMenuGray = FALSE;
		Invalidate();

		switch ( nCommand )
		{
		case ID_CHILD_RESTORE:
			pChild->PostMessage( WM_SYSCOMMAND, SC_RESTORE, 0 );
			break;
		case ID_CHILD_MINIMISE:
			pChild->PostMessage( WM_SYSCOMMAND, SC_MINIMIZE, 0 );
			break;
		case ID_CHILD_MAXIMISE:
			pChild->PostMessage( WM_SYSCOMMAND, SC_MAXIMIZE, 0 );
			break;
		case ID_CHILD_CLOSE:
			pChild->PostMessage( WM_SYSCOMMAND, SC_CLOSE, 0 );
			break;
		}

		return;
	}

	CControlBar::OnRButtonUp( nFlags, point );
}

void CWndTabBar::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	CoolMenu.OnMeasureItem( lpMeasureItemStruct );
}

void CWndTabBar::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CoolMenu.OnDrawItem( lpDrawItemStruct );
}


/////////////////////////////////////////////////////////////////////////////
// CWndTabBar::TabItem construction

CWndTabBar::TabItem::TabItem(CChildWnd* pWnd, DWORD nCookie, LPCTSTR pszCaption)
{
	m_hWnd		= pWnd->GetSafeHwnd();
	m_pClass	= pWnd->GetRuntimeClass();
	m_nImage	= -1;
	m_bVisible	= pWnd->IsWindowVisible();
	m_bAlert	= pWnd->m_bAlert;
	m_nCookie	= nCookie;

	if ( pszCaption != NULL )
		m_sCaption = pszCaption;
	else
		pWnd->GetWindowText( m_sCaption );
}

CWndTabBar::TabItem::~TabItem()
{
}

/////////////////////////////////////////////////////////////////////////////
// CWndTabBar::TabItem paint

void CWndTabBar::TabItem::Paint(CWndTabBar* pBar, CDC* pDC, CRect* pRect, BOOL bSelected, BOOL bHot, BOOL bTransparent)
{
	CRect rc( pRect );
	COLORREF crBack;

	if ( bSelected && pBar->m_bMenuGray )
	{
		crBack = CoolInterface.m_crBackNormal;
		pDC->Draw3dRect( &rc, CoolInterface.m_crDisabled, CoolInterface.m_crDisabled );
	}
	else if ( bHot || ( bSelected && m_bVisible ) )
	{
		crBack = ( bHot && bSelected ) ? CoolInterface.m_crBackCheckSel : CoolInterface.m_crBackSel;
		pDC->Draw3dRect( &rc, CoolInterface.m_crBorder, CoolInterface.m_crBorder );
	}
	else
	{
		crBack = bTransparent ? CLR_NONE : CoolInterface.m_crMidtone;
		if ( crBack != CLR_NONE ) pDC->Draw3dRect( &rc, crBack, crBack );
	}

	rc.DeflateRect( 1, 1 );
	if ( crBack != CLR_NONE ) pDC->SetBkColor( crBack );

	CPoint ptImage( rc.left + 2, rc.top + 1 );

	if ( bSelected )
	{
		ImageList_DrawEx( pBar->m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(),
			ptImage.x, ptImage.y, 0, 0, crBack, CLR_NONE, ILD_NORMAL );
		pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
	}
	else if ( bHot )
	{
		ptImage.Offset( -1, -1 );

		if ( crBack != CLR_NONE )
		{
			pDC->FillSolidRect( ptImage.x, ptImage.y, 18, 2, crBack );
			pDC->FillSolidRect( ptImage.x, ptImage.y + 2, 2, 16, crBack );
		}

		ptImage.Offset( 2, 2 );
		pDC->SetTextColor( CoolInterface.m_crShadow );
		ImageList_DrawEx( pBar->m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(),
			ptImage.x, ptImage.y, 0, 0, crBack, CLR_NONE, ILD_MASK );

		ptImage.Offset( -2, -2 );
		ImageList_DrawEx( pBar->m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(),
			ptImage.x, ptImage.y, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL );

		pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 18, ptImage.y + 18 );

		ptImage.Offset( 1, 1 );
	}
	else
	{
		ImageList_DrawEx( pBar->m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(),
			ptImage.x, ptImage.y, 0, 0, crBack, CoolInterface.m_crShadow, ILD_BLEND50 );
		pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
	}

	rc.left += 20;

	if ( bSelected && Settings.General.GUIMode != GUI_WINDOWED )
	{
		ptImage.x = rc.right - 18;
		ImageList_DrawEx( CoolInterface.m_pImages, pBar->m_nCloseImage,
			pDC->GetSafeHdc(), ptImage.x, ptImage.y, 0, 0, crBack, CoolInterface.m_crShadow,
			ILD_BLEND50 );
		pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
		rc.right -= 20;
		if ( crBack != CLR_NONE ) pDC->FillSolidRect( rc.right, rc.top, 20, rc.Height(), crBack );
	}

	CString strText = m_sCaption;

	if ( pDC->GetTextExtent( strText ).cx > rc.Width() )
	{
		while ( pDC->GetTextExtent( strText + _T('\x2026') ).cx > rc.Width() && strText.GetLength() )
		{
			strText = strText.Left( strText.GetLength() - 1 );
		}

		if ( strText.GetLength() ) strText += _T('\x2026');
	}

	rc.left -= 20;

	if ( crBack != CLR_NONE ) pDC->SetBkColor( crBack );

	if ( m_bAlert )
	{
		pDC->SetTextColor( RGB( 255, 0, 0 ) );
	}
	else if ( ! m_bVisible && ! bHot )
	{
		pDC->SetTextColor( CoolInterface.m_crDisabled );
	}
	else if ( bSelected || bHot )
	{
		pDC->SetTextColor( CoolInterface.m_crCmdTextSel );
	}
	else
	{
		pDC->SetTextColor( CoolInterface.m_crCmdText );
	}

	if ( crBack != CLR_NONE )
	{
		pDC->SetBkMode( OPAQUE );
		pDC->ExtTextOut( rc.left + 20, rc.top + 2, ETO_CLIPPED|ETO_OPAQUE, &rc, strText, NULL );
	}
	else
	{
		pDC->SetBkMode( TRANSPARENT );
		pDC->ExtTextOut( rc.left + 20, rc.top + 2, ETO_CLIPPED, &rc, strText, NULL );
	}
}
