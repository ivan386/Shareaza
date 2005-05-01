//
// CtrlLibraryTree.cpp
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
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "CtrlLibraryTree.h"
#include "CtrlLibraryFrame.h"
#include "CtrlCoolTip.h"
#include "SharedFolder.h"
#include "AlbumFolder.h"
#include "Schema.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CLibraryTreeCtrl, CWnd)

BEGIN_MESSAGE_MAP(CLibraryTreeCtrl, CWnd)
	//{{AFX_MSG_MAP(CLibraryTreeCtrl)
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define ITEM_HEIGHT	16


/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeCtrl construction

CLibraryTreeCtrl::CLibraryTreeCtrl()
{
	m_pRoot = new CLibraryTreeItem();
	m_pRoot->m_bExpanded = TRUE;

	m_nTotal		= 0;
	m_nVisible		= 0;
	m_nScroll		= 0;
	m_nSelected		= 0;
	m_pSelFirst		= NULL;
	m_pSelLast		= NULL;
	m_pFocus		= NULL;
	m_bDrag			= FALSE;
	m_pDropItem		= NULL;
	m_nCleanCookie	= 0;
	m_pTip			= NULL;
}

CLibraryTreeCtrl::~CLibraryTreeCtrl()
{
	delete m_pRoot;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeCtrl operations

BOOL CLibraryTreeCtrl::Create(CWnd* pParentWnd)
{
	CRect rect;
	return CWnd::Create( NULL, _T("CLibraryTreeCtrl"),
		WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL, rect, pParentWnd, IDC_LIBRARY_TREE, NULL );
}

void CLibraryTreeCtrl::SetToolTip(CCoolTipCtrl* pTip)
{
	if ( m_pTip ) m_pTip->Hide();
	m_pTip = pTip;
	if ( m_pTip ) m_pTip->SetOwner( this );
}

void CLibraryTreeCtrl::Clear()
{
	if ( m_pRoot->m_nCount == 0 ) return;

	m_pRoot->Clear();

	m_nTotal		= 0;
	m_nSelected		= 0;
	m_pSelFirst		= NULL;
	m_pSelLast		= NULL;
	m_pFocus		= NULL;
	m_pDropItem		= NULL;

	if ( m_pTip ) m_pTip->Hide();

	// NotifySelection(); NOT NOTIFIED
	UpdateScroll();
	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeCtrl expand

BOOL CLibraryTreeCtrl::Expand(CLibraryTreeItem* pItem, TRISTATE bExpand, BOOL bInvalidate)
{
	if ( pItem == NULL ) return FALSE;

	switch ( bExpand )
	{
	case TS_UNKNOWN:
		pItem->m_bExpanded = ! pItem->m_bExpanded;
		break;
	case TS_TRUE:
		if ( pItem->m_bExpanded ) return FALSE;
		pItem->m_bExpanded = TRUE;
		break;
	case TS_FALSE:
		if ( ! pItem->m_bExpanded ) return FALSE;
		pItem->m_bExpanded = FALSE;
		break;
	}

	if ( pItem->m_pPhysical )
	{
		pItem->m_pPhysical->m_bExpanded = pItem->m_bExpanded;
	}
	else
	{
		pItem->m_pVirtual->m_bExpanded = pItem->m_bExpanded;
	}

	if ( ! pItem->IsVisible() ) return FALSE;

	if ( pItem->m_bExpanded )
	{
		m_nTotal += pItem->GetChildCount();
	}
	else
	{
		m_nTotal -= pItem->GetChildCount();
		DeselectAll( NULL, pItem, FALSE );
	}

	pItem->m_bContract1 = pItem->m_bExpanded == TRUE && bExpand == TS_TRUE && bInvalidate == FALSE;

	if ( pItem->m_bContract1 == FALSE )
	{
		for ( CLibraryTreeItem* pParent = pItem ; pParent != NULL ; pParent = pParent->m_pParent )
			pParent->m_bContract1 = FALSE;
	}

	if ( bInvalidate )
	{
		UpdateScroll();
		Invalidate();
	}

	return TRUE;
}

BOOL CLibraryTreeCtrl::CollapseRecursive(CLibraryTreeItem* pItem)
{
	BOOL bChanged = FALSE;

	if ( pItem != m_pRoot && pItem->m_bExpanded && pItem->m_bContract1 )
	{
		bChanged |= Expand( pItem, TS_FALSE, FALSE );
	}

	CLibraryTreeItem** pChild = pItem->m_pList;

	for ( int nCount = pItem->m_nCount ; nCount ; nCount--, pChild++ )
	{
		bChanged |= CollapseRecursive( *pChild );
	}

	return bChanged;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeCtrl selection

BOOL CLibraryTreeCtrl::Select(CLibraryTreeItem* pItem, TRISTATE bSelect, BOOL bInvalidate)
{
	if ( pItem == NULL ) return FALSE;

	switch ( bSelect )
	{
	case TS_UNKNOWN:
		pItem->m_bSelected = ! pItem->m_bSelected;
		break;
	case TS_TRUE:
		if ( pItem->m_bSelected ) return FALSE;
		pItem->m_bSelected = TRUE;
		break;
	case TS_FALSE:
		if ( ! pItem->m_bSelected ) return FALSE;
		pItem->m_bSelected = FALSE;
		break;
	}

	if ( pItem->m_bSelected )
	{
		m_nSelected++;

		if ( m_pSelLast )
		{
			m_pSelLast->m_pSelNext = pItem;
			pItem->m_pSelPrev = m_pSelLast;
			pItem->m_pSelNext = NULL;
			m_pSelLast = pItem;
		}
		else
		{
			m_pSelFirst = m_pSelLast = pItem;
			pItem->m_pSelPrev = pItem->m_pSelNext = NULL;
		}
	}
	else
	{
		m_nSelected--;

		if ( pItem->m_pSelPrev )
			pItem->m_pSelPrev->m_pSelNext = pItem->m_pSelNext;
		else
			m_pSelFirst = pItem->m_pSelNext;

		if ( pItem->m_pSelNext )
			pItem->m_pSelNext = pItem->m_pSelNext->m_pSelPrev = pItem->m_pSelPrev;
		else
			m_pSelLast = pItem->m_pSelPrev;
	}

	if ( pItem->IsVisible() )
	{
		if ( bInvalidate ) Invalidate();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CLibraryTreeCtrl::SelectAll(CLibraryTreeItem* pParent, BOOL bInvalidate)
{
	if ( pParent == NULL ) pParent = m_pRoot;
	else if ( pParent->m_bExpanded == FALSE ) return FALSE;

	CLibraryTreeItem** pChild = pParent->m_pList;
	BOOL bChanged = FALSE;

	for ( int nCount = pParent->m_nCount ; nCount ; nCount--, pChild++ )
	{
		if ( (*pChild)->m_bSelected == FALSE )
		{
			Select( *pChild, TS_TRUE, FALSE );
			bChanged = TRUE;
		}

		if ( (*pChild)->m_bExpanded && (*pChild)->m_nCount )
		{
			bChanged |= SelectAll( *pChild, FALSE );
		}
	}

	if ( bInvalidate && bChanged && pParent == m_pRoot ) Invalidate();

	return bChanged;
}

BOOL CLibraryTreeCtrl::DeselectAll(CLibraryTreeItem* pExcept, CLibraryTreeItem* pParent, BOOL bInvalidate)
{
	if ( pParent == NULL ) pParent = m_pRoot;

	CLibraryTreeItem** pChild = pParent->m_pList;
	BOOL bChanged = FALSE;

	for ( int nCount = pParent->m_nCount ; nCount ; nCount--, pChild++ )
	{
		if ( *pChild != pExcept && (*pChild)->m_bSelected )
		{
			Select( *pChild, TS_FALSE, FALSE );
			bChanged = TRUE;
		}

		if ( (*pChild)->m_nCount ) bChanged |= DeselectAll( pExcept, *pChild, FALSE );
	}

	if ( bInvalidate && bChanged && pParent == m_pRoot ) Invalidate();

	return bChanged;
}

int CLibraryTreeCtrl::GetSelectedCount() const
{
	return m_nSelected;
}

CLibraryTreeItem* CLibraryTreeCtrl::GetFirstSelected() const
{
	return m_pSelFirst;
}

CLibraryTreeItem* CLibraryTreeCtrl::GetLastSelected() const
{
	return m_pSelLast;
}

BOOL CLibraryTreeCtrl::Highlight(CLibraryTreeItem* pItem)
{
	m_pFocus = pItem;

	for ( CLibraryTreeItem* pParent = m_pFocus->m_pParent ; pParent ; pParent = pParent->m_pParent )
	{
		Expand( pParent, TS_TRUE, FALSE );

		pParent->m_bContract2 = pParent->m_bContract1;
		pParent->m_bContract1 = FALSE;
	}

	CollapseRecursive( m_pRoot );

	for ( CLibraryTreeItem* pParent = m_pFocus->m_pParent ; pParent ; pParent = pParent->m_pParent )
	{
		pParent->m_bContract1 = pParent->m_bContract2;
	}

	CRect rcItem, rcClient;

	if ( GetRect( m_pFocus, &rcItem ) )
	{
		GetClientRect( &rcClient );

		if ( rcItem.top <= rcClient.top )
			ScrollBy( rcItem.top - rcClient.top );
		else if ( rcItem.bottom > rcClient.bottom )
			ScrollBy( rcItem.bottom - rcClient.bottom );
	}

	UpdateScroll();
	Invalidate();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeCtrl internal helpers

BOOL CLibraryTreeCtrl::CleanItems(CLibraryTreeItem* pItem, DWORD nCookie, BOOL bVisible)
{
	CLibraryTreeItem** pChild = pItem->m_pList + pItem->m_nCount - 1;
	BOOL bChanged = FALSE;

	for ( int nChild = pItem->m_nCount ; nChild ; nChild--, pChild-- )
	{
		if ( (*pChild)->m_nCleanCookie != nCookie )
		{
			if ( m_pFocus == *pChild ) m_pFocus = NULL;

			if ( (*pChild)->m_bSelected ) Select( *pChild, TS_FALSE, FALSE );
			bChanged |= DeselectAll( NULL, *pChild, FALSE );

			if ( bVisible )
			{
				m_nTotal -= (*pChild)->GetChildCount() + 1;
				bChanged = TRUE;
			}

			delete *pChild;
			MoveMemory( pChild, pChild + 1, 4 * ( pItem->m_nCount - nChild ) );
			pItem->m_nCount--;
		}
	}

	return bChanged;
}

void CLibraryTreeCtrl::NotifySelection()
{
	if (!m_hWnd) return;
	NMHDR pNM = { GetSafeHwnd(), GetDlgCtrlID(), LTN_SELCHANGED };
	GetOwner()->SendMessage( WM_NOTIFY, pNM.idFrom, (LPARAM)&pNM );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeCtrl search

CLibraryTreeItem* CLibraryTreeCtrl::GetFolderItem(LPVOID pSearch, CLibraryTreeItem* pParent)
{
	if ( pParent == NULL ) pParent = m_pRoot;

	CLibraryTreeItem** pChild = pParent->m_pList;

	for ( int nChild = pParent->m_nCount ; nChild ; nChild--, pChild++ )
	{
		if ( pSearch == (*pChild)->m_pPhysical ) return *pChild;
		if ( pSearch == (*pChild)->m_pVirtual  ) return *pChild;

		if ( (*pChild)->m_nCount )
		{
			CLibraryTreeItem* pFound = GetFolderItem( pSearch, *pChild );
			if ( pFound ) return pFound;
		}
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeCtrl message handlers

void CLibraryTreeCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );

	m_nVisible = cy;

	UpdateScroll();
}

void CLibraryTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rc;
	CLibraryTreeItem* pHit = HitTest( point, &rc );
	BOOL bChanged = FALSE;

	SetFocus();

	if ( m_pTip ) m_pTip->Hide();

	if ( pHit && pHit->m_nCount && point.x >= rc.left && point.x < rc.left + 16 )
	{
		bChanged = Expand( pHit, TS_UNKNOWN );
	}
	else if ( nFlags & MK_CONTROL )
	{
		if ( pHit ) bChanged = Select( pHit, TS_UNKNOWN );
	}
	else if ( nFlags & MK_SHIFT )
	{
		if ( pHit ) bChanged = Select( pHit );
	}
	else
	{
		if ( ( nFlags & MK_RBUTTON ) == 0 || ( pHit && pHit->m_bSelected == FALSE ) )
			bChanged = DeselectAll( pHit );
		if ( pHit ) bChanged |= Select( pHit );
	}

	m_pFocus = pHit;

	if ( pHit != NULL )
	{
		if ( m_pFocus->m_pVirtual && ( nFlags & MK_RBUTTON ) == 0 )
		{
			m_bDrag = TRUE;
			m_ptDrag = point;
			SetCapture();
		}
	}

	if ( bChanged ) NotifySelection();
}

void CLibraryTreeCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown( nFlags, point );

	if ( m_pFocus != NULL && m_pFocus->m_nCount )
	{
		if ( Expand( m_pFocus, TS_UNKNOWN ) ) NotifySelection();
	}
}

void CLibraryTreeCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	OnLButtonDown( nFlags, point );
	CWnd::OnRButtonDown( nFlags, point );
}

void CLibraryTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if ( m_bDrag & ( nFlags & MK_LBUTTON ) )
	{
		CSize szDiff = point - m_ptDrag;

		if ( abs( szDiff.cx ) > 5 || abs( szDiff.cy ) > 5 )
		{
			m_bDrag = FALSE;
			StartDragging( point );
		}
	}
	else if ( m_pTip != NULL )
	{
		if ( CLibraryTreeItem* pItem = HitTest( point ) )
		{
			m_pTip->Show( pItem->m_pPhysical ? (LPVOID)pItem->m_pPhysical : (LPVOID)pItem->m_pVirtual );
		}
		else
		{
			m_pTip->Hide();
		}
	}

	CWnd::OnMouseMove( nFlags, point );
}

void CLibraryTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	m_bDrag = FALSE;

	CWnd::OnLButtonUp( nFlags, point );
}

void CLibraryTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CLibraryTreeItem* pTo = NULL;
	BOOL bChanged = FALSE;
	CRect rc;

	if ( m_pTip ) m_pTip->Hide();

	if ( nChar == VK_HOME || ( nChar == VK_UP && m_pFocus == NULL ) )
	{
		if ( m_pRoot->m_nCount ) pTo = m_pRoot->m_pList[0];
	}
	else if ( nChar == VK_END || ( nChar == VK_DOWN && m_pFocus == NULL ) )
	{
		if ( m_pRoot->m_nCount ) pTo = m_pRoot->m_pList[ m_pRoot->m_nCount - 1 ];
	}
	else if ( nChar == VK_UP && m_pFocus != NULL )
	{
		if ( GetRect( m_pFocus, &rc ) )
		{
			CPoint pt( rc.left, ( rc.top + rc.bottom ) / 2 );
			pt.y -= ITEM_HEIGHT;
			pTo = HitTest( pt );
		}
	}
	else if ( nChar == VK_DOWN && m_pFocus != NULL )
	{
		if ( GetRect( m_pFocus, &rc ) )
		{
			CPoint pt( rc.left, ( rc.top + rc.bottom ) / 2 );
			pt.y += ITEM_HEIGHT;
			pTo = HitTest( pt );
		}
	}
	else if ( ( nChar == VK_LEFT || nChar == VK_SUBTRACT ) && m_pFocus != NULL )
	{
		while ( TRUE )
		{
			if ( m_pFocus->m_bExpanded && m_pFocus->m_nCount )
			{
				Expand( m_pFocus, TS_FALSE );
				break;
			}

			if ( m_pFocus->m_pParent == m_pRoot ) break;
			m_pFocus = m_pFocus->m_pParent;

			bChanged |= DeselectAll( m_pFocus );
			bChanged |= Select( m_pFocus );
		}

		Highlight( m_pFocus );
	}
	else if ( ( nChar == VK_RIGHT || nChar == VK_ADD ) && m_pFocus != NULL )
	{
		if ( ! m_pFocus->m_bExpanded && m_pFocus->m_nCount )
		{
			bChanged |= Expand( m_pFocus, TS_TRUE );
		}
	}
	else if ( _istalnum( nChar ) )
	{
		CLibraryTreeItem* pStart	= m_pFocus;
		CLibraryTreeItem* pBase		= pStart ? pStart->m_pParent : m_pRoot;

		for ( int nLoop = 0 ; nLoop < 2 ; nLoop++ )
		{
			CLibraryTreeItem** pChild = pBase->m_pList;

			for ( int nCount = pBase->m_nCount ; nCount ; nCount--, pChild++ )
			{
				if ( pStart != NULL )
				{
					if ( pStart == *pChild ) pStart = NULL;
				}
				else if ( toupper( (*pChild)->m_sText.GetAt( 0 ) ) == (int)nChar )
				{
					DeselectAll( m_pFocus = *pChild, NULL, FALSE );
					Select( m_pFocus, TS_TRUE, FALSE );
					Highlight( m_pFocus );
					NotifySelection();
					return;
				}
			}
		}
	}

	if ( pTo != NULL )
	{
		if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 || m_pFocus == NULL )
		{
			bChanged |= DeselectAll( m_pFocus = pTo );
			bChanged |= Select( m_pFocus );
		}
		else
		{
			bChanged |= Select( m_pFocus = pTo );
		}

		Highlight( m_pFocus );
	}

	if ( bChanged ) NotifySelection();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeCtrl scrolling

void CLibraryTreeCtrl::UpdateScroll()
{
	SCROLLINFO pInfo;

	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;
	pInfo.nMin		= 0;
	pInfo.nMax		= m_nTotal * ITEM_HEIGHT;
	pInfo.nPage		= m_nVisible;
	pInfo.nPos		= m_nScroll = max( 0, min( m_nScroll, pInfo.nMax - (int)pInfo.nPage + 1 ) );

	SetScrollInfo( SB_VERT, &pInfo, TRUE );
}

void CLibraryTreeCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	switch ( nSBCode )
	{
	case SB_BOTTOM:
		ScrollTo( 0xFFFFFFFF );
		break;
	case SB_LINEDOWN:
		ScrollBy( 16 );
		break;
	case SB_LINEUP:
		ScrollBy( -16 );
		break;
	case SB_PAGEDOWN:
		ScrollBy( m_nVisible );
		break;
	case SB_PAGEUP:
		ScrollBy( -m_nVisible );
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		ScrollTo( nPos );
		break;
	case SB_TOP:
		ScrollTo( 0 );
		break;
	}
}

BOOL CLibraryTreeCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	ScrollBy( zDelta * 3 * -ITEM_HEIGHT / WHEEL_DELTA );
	return TRUE;
}

void CLibraryTreeCtrl::ScrollBy(int nDelta)
{
	ScrollTo( max( 0, m_nScroll + nDelta ) );
}

void CLibraryTreeCtrl::ScrollTo(int nPosition)
{
	if ( nPosition == m_nScroll ) return;
	m_nScroll = nPosition;

	UpdateScroll();

	CRect rc;
	GetClientRect( &rc );
	RedrawWindow( &rc, NULL, RDW_INVALIDATE );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeCtrl painting

BOOL CLibraryTreeCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CLibraryTreeCtrl::OnPaint()
{
	CPaintDC dc( this );

	CRect rcClient;
	GetClientRect( &rcClient );

	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	CLibraryTreeItem** pChild = m_pRoot->m_pList;

	CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );

	for ( int nCount = m_pRoot->m_nCount ; nCount && pt.y < rcClient.bottom ; nCount--, pChild++ )
	{
		Paint( dc, rcClient, pt, *pChild );
	}

	dc.SelectObject( pOldFont );

	dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );
}

void CLibraryTreeCtrl::Paint(CDC& dc, CRect& rcClient, CPoint& pt, CLibraryTreeItem* pItem)
{
	CRect rc( pt.x, pt.y, pt.x, pt.y + ITEM_HEIGHT );
	pt.y += ITEM_HEIGHT;

	if ( rc.top >= rcClient.bottom )
	{
		return;
	}
	else if ( rc.bottom >= rcClient.top )
	{
		if ( pItem->m_bBold ) dc.SelectObject( &CoolInterface.m_fntBold );

		rc.right += 32 + dc.GetTextExtent( pItem->m_sText ).cx + 6;

		if ( dc.RectVisible( &rc ) )
		{
			pItem->Paint( dc, rc, m_pDropItem == pItem );
			dc.ExcludeClipRect( &rc );
		}

		if ( pItem->m_bBold ) dc.SelectObject( &CoolInterface.m_fntNormal );
	}

	if ( pItem->m_bExpanded && pItem->m_nCount )
	{
		pt.x += 16;

		CLibraryTreeItem** pChild = pItem->m_pList;

		for ( int nCount = pItem->m_nCount ; nCount ; nCount--, pChild++ )
		{
			Paint( dc, rcClient, pt, *pChild );
			if ( pt.y >= rcClient.bottom ) break;
		}

		pt.x -= 16;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeCtrl hit testing

CLibraryTreeItem* CLibraryTreeCtrl::HitTest(const POINT& point, RECT* pRect) const
{
	CRect rcClient;
	GetClientRect( &rcClient );

	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	CLibraryTreeItem** pChild = m_pRoot->m_pList;

	for ( int nCount = m_pRoot->m_nCount ; nCount && pt.y < rcClient.bottom ; nCount--, pChild++ )
	{
		CLibraryTreeItem* pItem = HitTest( rcClient, pt, *pChild, point, pRect );
		if ( pItem ) return pItem;
	}

	return NULL;
}

CLibraryTreeItem* CLibraryTreeCtrl::HitTest(CRect& rcClient, CPoint& pt, CLibraryTreeItem* pItem, const POINT& point, RECT* pRect) const
{
	CRect rc( rcClient.left, pt.y, rcClient.right, pt.y + ITEM_HEIGHT );
	pt.y += ITEM_HEIGHT;

	if ( rc.top >= rcClient.bottom + ITEM_HEIGHT )
	{
		return NULL;
	}
	else if ( rc.bottom >= rcClient.top - ITEM_HEIGHT )
	{
		if ( rc.PtInRect( point ) )
		{
			if ( pRect )
			{
				CopyMemory( pRect, &rc, sizeof(RECT) );
				pRect->left = pt.x;
			}
			return pItem;
		}
	}

	if ( pItem->m_bExpanded && pItem->m_nCount )
	{
		pt.x += 16;

		CLibraryTreeItem** pChild = pItem->m_pList;

		for ( int nCount = pItem->m_nCount ; nCount ; nCount--, pChild++ )
		{
			CLibraryTreeItem* pItem = HitTest( rcClient, pt, *pChild, point, pRect );
			if ( pItem ) return pItem;
			if ( pt.y >= rcClient.bottom + ITEM_HEIGHT ) break;
		}

		pt.x -= 16;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeCtrl rect lookup

BOOL CLibraryTreeCtrl::GetRect(CLibraryTreeItem* pItem, RECT* pRect)
{
	CRect rcClient;
	GetClientRect( &rcClient );

	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	CLibraryTreeItem** pChild = m_pRoot->m_pList;

	for ( int nCount = m_pRoot->m_nCount ; nCount ; nCount--, pChild++ )
	{
		if ( GetRect( pt, *pChild, pItem, pRect ) ) return TRUE;
	}

	return FALSE;
}

BOOL CLibraryTreeCtrl::GetRect(CPoint& pt, CLibraryTreeItem* pItem, CLibraryTreeItem* pFind, RECT* pRect)
{
	if ( pItem == pFind )
	{
		pRect->left		= pt.x;
		pRect->top		= pt.y;
		pRect->right	= pt.x;
		pRect->bottom	= pt.y = pRect->top + ITEM_HEIGHT;

		CClientDC dc( this );
		CFont* pOld = (CFont*)dc.SelectObject( pItem->m_bBold ?
			&CoolInterface.m_fntBold : &CoolInterface.m_fntNormal );
		pRect->right += 33 + dc.GetTextExtent( pItem->m_sText ).cx + 4;
		dc.SelectObject( pOld );

		return TRUE;
	}
	else
	{
		pt.y += ITEM_HEIGHT;
	}

	if ( pItem->m_bExpanded && pItem->m_nCount )
	{
		pt.x += 16;

		CLibraryTreeItem** pChild = pItem->m_pList;

		for ( int nCount = pItem->m_nCount ; nCount ; nCount--, pChild++ )
		{
			if ( GetRect( pt, *pChild, pFind, pRect ) ) return TRUE;
		}

		pt.x -= 16;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeCtrl drag setup

#define MAX_DRAG_SIZE	256
#define MAX_DRAG_SIZE_2	128

void CLibraryTreeCtrl::StartDragging(CPoint& ptMouse)
{
	CImageList* pImage = CreateDragImage( ptMouse );
	if ( pImage == NULL ) return;

	ReleaseCapture();
	ClientToScreen( &ptMouse );

	CLibraryFrame* pFrame	= (CLibraryFrame*)GetOwner();
	CLibraryList* pList		= new CLibraryList( m_nSelected );

	for (	CLibraryTreeItem* pItem = m_pSelFirst ; pItem ;
			pItem = pItem->m_pSelNext )
	{
		if ( pItem->m_pVirtual ) pList->AddTail( (DWORD)pItem->m_pVirtual );
	}

	pFrame->DragObjects( pList, pImage, ptMouse );
}

CImageList* CLibraryTreeCtrl::CreateDragImage(const CPoint& ptMouse)
{
	CRect rcClient, rcOne, rcAll( 32000, 32000, -32000, -32000 );

	GetClientRect( &rcClient );

	for (	CLibraryTreeItem* pItem = m_pSelFirst ; pItem ;
			pItem = pItem->m_pSelNext )
	{
		GetRect( pItem, &rcOne );

		if ( rcOne.IntersectRect( &rcClient, &rcOne ) )
		{
			rcAll.left		= min( rcAll.left, rcOne.left );
			rcAll.top		= min( rcAll.top, rcOne.top );
			rcAll.right		= max( rcAll.right, rcOne.right );
			rcAll.bottom	= max( rcAll.bottom, rcOne.bottom );
		}
	}

	BOOL bClipped = rcAll.Height() > MAX_DRAG_SIZE;

	if ( bClipped )
	{
		rcAll.left		= max( rcAll.left, ptMouse.x - MAX_DRAG_SIZE_2 );
		rcAll.right		= max( rcAll.right, ptMouse.x + MAX_DRAG_SIZE_2 );
		rcAll.top		= max( rcAll.top, ptMouse.y - MAX_DRAG_SIZE_2 );
		rcAll.bottom	= max( rcAll.bottom, ptMouse.y + MAX_DRAG_SIZE_2 );
	}

	CClientDC dcClient( this );
	CDC dcMem, dcDrag;
	CBitmap bmDrag;

	if ( ! dcMem.CreateCompatibleDC( &dcClient ) )
		return NULL;
	if ( ! dcDrag.CreateCompatibleDC( &dcClient ) )
		return NULL;
	if ( ! bmDrag.CreateCompatibleBitmap( &dcClient, rcAll.Width(), rcAll.Height() ) )
		return NULL;

	CBitmap *pOldDrag = dcDrag.SelectObject( &bmDrag );

	dcDrag.FillSolidRect( 0, 0, rcAll.Width(), rcAll.Height(), RGB( 250, 255, 250 ) );

	CRgn pRgn;

	if ( bClipped )
	{
		CPoint ptMiddle( ptMouse.x - rcAll.left, ptMouse.y - rcAll.top );
		pRgn.CreateEllipticRgn(	ptMiddle.x - MAX_DRAG_SIZE_2, ptMiddle.y - MAX_DRAG_SIZE_2,
								ptMiddle.x + MAX_DRAG_SIZE_2, ptMiddle.y + MAX_DRAG_SIZE_2 );
		dcDrag.SelectClipRgn( &pRgn );
	}

	CFont* pOldFont = (CFont*)dcDrag.SelectObject( &CoolInterface.m_fntNormal );

	for ( CLibraryTreeItem* pItem = m_pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		GetRect( pItem, &rcOne );
		CRect rcDummy;

		if ( rcDummy.IntersectRect( &rcAll, &rcOne ) )
		{
			rcOne.OffsetRect( -rcAll.left, -rcAll.top );
			pItem->Paint( dcDrag, rcOne, FALSE, RGB( 250, 255, 250 ) );
		}
	}

	dcDrag.SelectObject( pOldFont );
	dcDrag.SelectObject( pOldDrag );
	dcDrag.DeleteDC();

	CImageList* pAll = new CImageList();
	pAll->Create( rcAll.Width(), rcAll.Height(), ILC_COLOR16|ILC_MASK, 1, 1 );
	pAll->Add( &bmDrag, RGB( 250, 255, 250 ) );

	bmDrag.DeleteObject();

	pAll->BeginDrag( 0, ptMouse - rcAll.TopLeft() );

	return pAll;
}


/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeItem construction

CLibraryTreeItem::CLibraryTreeItem(CLibraryTreeItem* pParent)
{
	m_pParent		= pParent;
	m_pList			= NULL;
	m_nCount		= 0;
	m_nBuffer		= 0;
	m_pSelPrev		= NULL;
	m_pSelNext		= NULL;
	m_nCleanCookie	= 0;

	m_bExpanded		= FALSE;
	m_bSelected		= FALSE;
	m_bContract1	= FALSE;
	m_bContract2	= FALSE;

	m_pPhysical		= NULL;
	m_pVirtual		= NULL;
	m_nCookie		= 0;
	m_bBold			= FALSE;
    m_bShared		= TRUE;
	m_bCollection	= FALSE;
	m_nIcon16		= -1;
}

CLibraryTreeItem::~CLibraryTreeItem()
{
	if ( m_pList )
	{
		Clear();
		delete [] m_pList;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeItem add

CLibraryTreeItem* CLibraryTreeItem::Add(LPCTSTR pszName)
{
	if ( m_nCount == m_nBuffer )
	{
		if ( m_nBuffer ) m_nBuffer += min( m_nBuffer, 16 ); else m_nBuffer = 4;

		CLibraryTreeItem** pList = new CLibraryTreeItem*[ m_nBuffer ];

		if ( m_nCount ) CopyMemory( pList, m_pList, m_nCount * 4 );
		if ( m_pList ) delete [] m_pList;

		m_pList = pList;
	}

	if ( m_nCount == 0 ) return m_pList[ m_nCount++ ] = new CLibraryTreeItem( this );

    int nFirst = 0;
	for ( int nLast = m_nCount - 1 ; nLast >= nFirst ; )
	{
		int nMiddle = ( nFirst + nLast ) >> 1;

		CLibraryTreeItem* pItem = m_pList[ nMiddle ];

		if ( _tcsicoll( pszName, pItem->m_sText ) >= 0 )
		{
			nFirst = nMiddle + 1;
		}
		else
		{
			nLast = nMiddle - 1;
		}
	}

	MoveMemory( m_pList + nFirst + 1, m_pList + nFirst, ( m_nCount - nFirst ) << 2 );
	m_nCount++;

	return m_pList[ nFirst ] = new CLibraryTreeItem( this );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeItem delete

void CLibraryTreeItem::Delete()
{
	m_pParent->Delete( this );
}

void CLibraryTreeItem::Delete(CLibraryTreeItem* pItem)
{
	ASSERT( pItem->m_bSelected == FALSE );

	CLibraryTreeItem** pChild = m_pList;

	for ( int nChild = m_nCount ; nChild ; nChild--, pChild++ )
	{
		if ( *pChild == pItem )
		{
			MoveMemory( pChild, pChild + 1, 4 * ( nChild - 1 ) );
			m_nCount--;
			break;
		}
	}

	delete pItem;
}

void CLibraryTreeItem::Delete(int nItem)
{
	if ( nItem < 0 || nItem >= m_nCount ) return;

	ASSERT( m_pList[ nItem ]->m_bSelected == FALSE );
	delete m_pList[ nItem ];
	MoveMemory( m_pList + nItem, m_pList + nItem + 1, 4 * ( m_nCount - nItem - 1 ) );
	m_nCount--;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeItem clear

void CLibraryTreeItem::Clear()
{
	if ( m_pList )
	{
		for ( int nChild = 0 ; nChild < m_nCount ; nChild++ ) delete m_pList[ nChild ];
		delete [] m_pList;
	}

	m_pList		= NULL;
	m_nCount	= 0;
	m_nBuffer	= 0;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeItem visibility

BOOL CLibraryTreeItem::IsVisible() const
{
	for ( CLibraryTreeItem* pRoot = m_pParent ; pRoot ; pRoot = pRoot->m_pParent )
	{
		if ( ! pRoot->m_bExpanded ) return FALSE;
	}

	return TRUE;
}

int CLibraryTreeItem::GetChildCount() const
{
	int nCount = m_nCount;

	CLibraryTreeItem** pChild = m_pList;

	for ( int nChild = m_nCount ; nChild ; nChild--, pChild++ )
	{
		if ( (*pChild)->m_bExpanded ) nCount += (*pChild)->GetChildCount();
	}

	return nCount;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeItem paint

void CLibraryTreeItem::Paint(CDC& dc, CRect& rc, BOOL bTarget, COLORREF crBack) const
{
	if ( crBack == CLR_NONE ) crBack = CoolInterface.m_crWindow;

	if ( m_nCount )
	{
		ImageList_DrawEx( ShellIcons.GetHandle( 16 ),
			m_bExpanded ? SHI_MINUS : SHI_PLUS,
			dc.GetSafeHdc(), rc.left, rc.top, 16, 16,
			crBack, CLR_NONE, ILD_NORMAL );
	}
	else
	{
		dc.FillSolidRect( rc.left, rc.top, 16, 16, crBack );
	}

	int nImage = ( m_bExpanded && m_nCount ) ? SHI_FOLDER_OPEN : SHI_FOLDER_CLOSED;
	if ( m_nIcon16 >= 0 ) nImage = m_nIcon16;

	UINT nIconStyle = ( m_bSelected || bTarget ) ? ILD_SELECTED : ILD_NORMAL;

	if ( ! m_bShared ) nIconStyle |= INDEXTOOVERLAYMASK( SHI_O_LOCKED );
	if ( m_bCollection ) nIconStyle |= INDEXTOOVERLAYMASK( SHI_O_COLLECTION );

	ImageList_DrawEx( ShellIcons.GetHandle( 16 ), nImage,
		dc.GetSafeHdc(), rc.left + 16, rc.top, 16, 16,
		crBack, CLR_DEFAULT, nIconStyle );

	crBack = ( m_bSelected || bTarget ) ? CoolInterface.m_crHighlight : crBack;
	COLORREF crText = ( m_bSelected || bTarget ) ? CoolInterface.m_crHiText : CoolInterface.m_crText;

	dc.SetTextColor( crText );
	dc.SetBkColor( crBack );
	dc.SetBkMode( OPAQUE );

	rc.left += 32;
	dc.ExtTextOut( rc.left + 3, rc.top + 1, ETO_OPAQUE|ETO_CLIPPED, &rc,
		m_sText, NULL );
	rc.left -= 32;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeItem get child files

int CLibraryTreeItem::GetFileList(CLibraryList* pList, BOOL bRecursive) const
{
	if ( LibraryFolders.CheckFolder( m_pPhysical, TRUE ) )
	{
		return m_pPhysical->GetFileList( pList, bRecursive );
	}
	else if ( LibraryFolders.CheckAlbum( m_pVirtual ) )
	{
		return m_pVirtual->GetFileList( pList, bRecursive );
	}

	return 0;
}
