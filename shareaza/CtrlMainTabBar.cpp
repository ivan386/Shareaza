//
// CtrlMainTabBar.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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
#include "CtrlMainTabBar.h"
#include "Skin.h"
#include "SkinWindow.h"

IMPLEMENT_DYNAMIC(CMainTabBarCtrl, CControlBar)

BEGIN_MESSAGE_MAP(CMainTabBarCtrl, CControlBar)
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_TIMER()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl construction

CMainTabBarCtrl::CMainTabBarCtrl()
{
	m_pSkin		= NULL;
	m_pHover	= NULL;
	m_pDown		= NULL;
}

CMainTabBarCtrl::~CMainTabBarCtrl()
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		delete (TabItem*)m_pItems.GetNext( pos );
	}
	
	if ( m_dcSkin.m_hDC != NULL )
	{
		if ( m_hOldSkin != NULL ) m_dcSkin.SelectObject( CBitmap::FromHandle( m_hOldSkin ) );
		m_bmSkin.DeleteObject();
		m_dcSkin.DeleteDC();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl operations

BOOL CMainTabBarCtrl::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	CRect rc;
	dwStyle |= WS_CHILD;
	return CWnd::Create( NULL, NULL, dwStyle, rc, pParentWnd, nID, NULL );
}

BOOL CMainTabBarCtrl::HasLocalVersion()
{
	return ( m_pSkin != NULL ) && ( m_pSkin->m_sLanguage == Settings.General.Language );
}

void CMainTabBarCtrl::OnSkinChange()
{
	if ( m_pItems.GetCount() == 0 )
	{
		m_pItems.AddTail( new TabItem( this, _T("_ID_TAB_HOME") ) );
		m_pItems.AddTail( new TabItem( this, _T("_ID_TAB_LIBRARY") ) );
		m_pItems.AddTail( new TabItem( this, _T("_ID_TAB_MEDIA") ) );
		m_pItems.AddTail( new TabItem( this, _T("_ID_TAB_SEARCH") ) );
		m_pItems.AddTail( new TabItem( this, _T("_ID_TAB_TRANSFERS") ) );
		m_pItems.AddTail( new TabItem( this, _T("_ID_TAB_NETWORK") ) );
	}
	
	if ( m_dcSkin.m_hDC != NULL )
	{
		if ( m_hOldSkin != NULL ) m_dcSkin.SelectObject( CBitmap::FromHandle( m_hOldSkin ) );
		m_bmSkin.DeleteObject();
		m_dcSkin.DeleteDC();
	}
	
	m_pSkin = Skin.GetWindowSkin( this );
	
	if ( m_pSkin != NULL )
	{
		BITMAP pInfo;
		CDC dcScreen;
		
		dcScreen.Attach( ::GetDC( 0 ) );
		m_pSkin->Prepare( &dcScreen );
		m_pSkin->m_bmSkin.GetBitmap( &pInfo );
		m_dcSkin.CreateCompatibleDC( &dcScreen );
		m_bmSkin.CreateCompatibleBitmap( &dcScreen, pInfo.bmWidth, pInfo.bmHeight );
		::ReleaseDC( 0, dcScreen.Detach() );
		
		m_hOldSkin = (HBITMAP)m_dcSkin.SelectObject( &m_bmSkin )->GetSafeHandle();
		m_dcSkin.BitBlt( 0, 0, pInfo.bmWidth, pInfo.bmHeight, &m_pSkin->m_dcSkin, 0, 0, SRCCOPY );
		m_dcSkin.SelectObject( CBitmap::FromHandle( m_hOldSkin ) );
		
		m_hOldSkin = NULL;

#ifndef _DEBUG
        for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
		{
			TabItem* pItem = (TabItem*)m_pItems.GetNext( pos );
			pItem->Skin( m_pSkin, &m_dcSkin, &m_bmSkin );
		}
		
		m_hOldSkin = (HBITMAP)m_dcSkin.SelectObject( &m_bmSkin )->GetSafeHandle();
#endif
	}
	
	CMDIFrameWnd* pOwner = (CMDIFrameWnd*)GetOwner();
	
	if ( pOwner != NULL && pOwner->IsKindOf( RUNTIME_CLASS(CMDIFrameWnd) ) )
	{
		if ( ! pOwner->IsIconic() ) pOwner->RecalcLayout();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl message handlers

int CMainTabBarCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CControlBar::OnCreate( lpCreateStruct ) == -1 ) return -1;
	m_dwStyle |= CBRS_BORDER_3D;
	SetTimer( 1, 250, NULL );
	return 0;
}

void CMainTabBarCtrl::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	BOOL bChanged = FALSE;
	
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		TabItem* pItem = (TabItem*)m_pItems.GetNext( pos );
		bChanged |= pItem->Update( pTarget );
	}
	
	if ( bChanged ) Invalidate();
}

CSize CMainTabBarCtrl::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CRect rcBackground;
	CSize size( 0, 28 );
	
	if ( m_pSkin != NULL && m_pSkin->GetAnchor( _T("Background"), rcBackground ) )
	{
		size = rcBackground.Size();
	}
	
	if ( bStretch )
	{
		size.cx = 32000;
		
		if ( CWnd* pParent = GetOwner() )
		{
			CRect rc;
			pParent->GetWindowRect( &rc );
			if ( rc.Width() > 32 ) size.cx = rc.Width() + 2;
		}
	}
	
	return size;
}

CMainTabBarCtrl::TabItem* CMainTabBarCtrl::HitTest(const CPoint& point) const
{
	CPoint ptLocal( point );
	CRect rcClient;
	
	GetClientRect( &rcClient );
	CalcInsideRect( rcClient, FALSE );
	rcClient.left -= m_cyTopBorder;
	rcClient.top -= m_cxLeftBorder;
	rcClient.right += m_cyBottomBorder;
	rcClient.bottom += m_cxRightBorder;
	ptLocal.Offset( -rcClient.left, -rcClient.top );
	
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		TabItem* pItem = (TabItem*)m_pItems.GetNext( pos );
		if ( pItem->HitTest( ptLocal ) ) return pItem;
	}
	
	return NULL;
}

int CMainTabBarCtrl::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	TabItem* pItem = HitTest( point );
	
	if ( pItem == NULL ) return -1;
	if ( pTI == NULL ) return 1;
	
	pTI->uFlags		= 0;
	pTI->hwnd		= GetSafeHwnd();
	pTI->uId		= (UINT)pItem->m_nID;
	pTI->rect		= pItem->m_rc;
	pTI->lpszText	= LPSTR_TEXTCALLBACK;
	
	CString strTip;
	
	if ( LoadString( strTip, pTI->uId ) )
	{
		if ( LPCTSTR pszBreak = _tcschr( strTip, '\n' ) )
		{
			pTI->lpszText = _tcsdup( pszBreak + 1 );
		}
		else
		{
			strTip = strTip.SpanExcluding( _T(".") );
			pTI->lpszText = _tcsdup( strTip );
		}
	}
	
	return pTI->uId;
}

void CMainTabBarCtrl::DoPaint(CDC* pDC)
{
	ASSERT_VALID( this );
	ASSERT_VALID( pDC );
	
	CRect rc;
	GetClientRect( &rc );
	
	if ( m_pSkin == NULL )
	{
		DrawBorders( pDC, rc );
		pDC->FillSolidRect( &rc, CoolInterface.m_crMidtone );
		return;
	}
	else if ( m_hOldSkin == NULL )
	{
        for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
		{
			TabItem* pItem = (TabItem*)m_pItems.GetNext( pos );
			pItem->Skin( m_pSkin, &m_dcSkin, &m_bmSkin );
		}
		
		m_hOldSkin = (HBITMAP)m_dcSkin.SelectObject( &m_bmSkin )->GetSafeHandle();
	}
	
	CDC* pBuffer = CoolInterface.GetBuffer( *pDC, rc.Size() );
	
	DrawBorders( pBuffer, rc );
	
	if ( ! CoolInterface.DrawWatermark( pBuffer, &rc, &m_pSkin->m_bmWatermark ) )
	{
		pBuffer->FillSolidRect( &rc, CoolInterface.m_crMidtone );
	}
	
	CPoint ptOffset = rc.TopLeft();
	
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		TabItem* pItem = (TabItem*)m_pItems.GetNext( pos );
		pItem->Paint( pBuffer, &m_dcSkin, ptOffset, 
			( m_pHover == pItem && m_pDown == NULL ) || ( m_pDown == pItem ),
			( m_pDown == pItem ) && ( m_pHover == pItem ) );
	}
	
	GetClientRect( &rc );
	pDC->BitBlt( 0, 0, rc.Width(), rc.Height(), pBuffer, 0, 0, SRCCOPY );
}

BOOL CMainTabBarCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( nHitTest == HTCLIENT )
	{
		CPoint point;
		GetCursorPos( &point );
		ScreenToClient( &point );
		
		if ( TabItem* pItem = HitTest( point ) )
		{
			if ( pItem->m_bEnabled )
			{
				SetCursor( theApp.LoadCursor( IDC_HAND ) );
				return TRUE;
			}
		}
	}
	
	return CControlBar::OnSetCursor( pWnd, nHitTest, message );
}

void CMainTabBarCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	TabItem* pItem = HitTest( point );
	
	if ( pItem != m_pHover )
	{
		m_pHover = pItem;
		Invalidate();
	}
}

void CMainTabBarCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown( nFlags, point );
}

void CMainTabBarCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_pHover = HitTest( point );
	
	if ( m_pHover != NULL && m_pHover->m_bEnabled )
	{
		m_pDown = m_pHover;
	}
	
	if ( m_pHover == NULL )
	{
		CControlBar::OnLButtonDown( nFlags, point );
	}
	else
	{
    	SetCapture();
	}
	
	Invalidate();
}

void CMainTabBarCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ( m_pDown != NULL && m_pHover == m_pDown )
	{
		GetOwner()->PostMessage( WM_COMMAND, m_pDown->m_nID );
	}
	
	m_pDown = NULL;
	ReleaseCapture();
	Invalidate();
	
	if ( m_pHover == NULL ) CControlBar::OnLButtonUp( nFlags, point );
}

void CMainTabBarCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ( m_pDown != NULL || m_pHover != NULL )
	{
		m_pDown = m_pHover = NULL;
		ReleaseCapture();
		Invalidate();
	}
	
	CControlBar::OnRButtonDown( nFlags, point );
}

void CMainTabBarCtrl::OnTimer(UINT nIDEvent)
{
	if ( m_pHover != NULL && m_pDown == NULL )
	{
		CPoint point;
		CRect rect;
		
		GetCursorPos( &point );
		GetWindowRect( &rect );
		
		if ( rect.PtInRect( point ) == FALSE )
		{
			m_pHover = NULL;
			Invalidate();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl::TabItem construction

CMainTabBarCtrl::TabItem::TabItem(CMainTabBarCtrl* pCtrl, LPCTSTR pszName)
{
	m_pCtrl		= pCtrl;
	m_nID		= CoolInterface.NameToID( pszName + 1 );
	m_sName		= pszName;
	m_bEnabled	= TRUE;
	m_bSelected	= FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl::TabItem skin change

void CMainTabBarCtrl::TabItem::Skin(CSkinWindow* pSkin, CDC* pdcCache, CBitmap* pbmCache)
{
	static LPCTSTR pszState[] = { _T(".Checked"), _T(".Down"), _T(".Hover"), _T(".Up"), _T(".Disabled"), NULL };
	
	m_rc.SetRectEmpty();
	pSkin->GetAnchor( m_sName, m_rc );
	
	for ( int nState = 0 ; pszState[ nState ] != NULL ; nState++ )
	{
		CRect* pRect = &m_rcSrc[ nState ];
		pRect->SetRectEmpty();
		
		if ( pSkin->GetPart( m_sName + pszState[ nState ], *pRect ) )
		{
			CBitmap* pOld = (CBitmap*)pdcCache->SelectObject( pbmCache );
			
			if ( pSkin->m_bmWatermark.m_hObject != NULL )
			{
				pdcCache->IntersectClipRect( pRect );
				CoolInterface.DrawWatermark( pdcCache, pRect, &pSkin->m_bmWatermark,
					m_rc.left, m_rc.top );
				pdcCache->SelectClipRgn( NULL );
			}
			else
			{
				pdcCache->FillSolidRect( pRect, CoolInterface.m_crMidtone );
			}
			
			pdcCache->SelectObject( pOld );
			
			pSkin->PreBlend( pbmCache, *pRect, *pRect );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl::TabItem GUI update

BOOL CMainTabBarCtrl::TabItem::Update(CFrameWnd* pWnd)
{
	BOOL bEnabled	= m_bEnabled;
	BOOL bSelected	= m_bSelected;
	
	DoUpdate( pWnd, TRUE );
	
	return ( bEnabled != m_bEnabled || bSelected != m_bSelected );
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl::TabItem hit test

BOOL CMainTabBarCtrl::TabItem::HitTest(const CPoint& point) const
{
	return m_rc.PtInRect( point );
}

/////////////////////////////////////////////////////////////////////////////
// CMainTabBarCtrl::TabItem paint

void CMainTabBarCtrl::TabItem::Paint(CDC* pDstDC, CDC* pSrcDC, const CPoint& ptOffset, BOOL bHover, BOOL bDown)
{
	CRect* pPart = NULL;
	
	if ( bDown )
		pPart = &m_rcSrc[1];
	else if ( m_bSelected )
		pPart = &m_rcSrc[0];
	else if ( ! m_bEnabled )
		pPart = &m_rcSrc[4];
	else if ( bHover )
		pPart = &m_rcSrc[2];
	else
		pPart = &m_rcSrc[3];
	
	CRect rcTarget( m_rc );
	rcTarget += ptOffset;
	
	pDstDC->BitBlt( rcTarget.left, rcTarget.top, rcTarget.Width(), rcTarget.Height(),
		pSrcDC, pPart->left, pPart->top, SRCCOPY );
}

void CMainTabBarCtrl::TabItem::Enable(BOOL bEnable)
{
	m_bEnabled = bEnable;
}

void CMainTabBarCtrl::TabItem::SetCheck(BOOL bCheck)
{
	m_bSelected = bCheck;
}
