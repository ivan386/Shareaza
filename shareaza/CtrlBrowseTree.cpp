//
// CtrlBrowseTree.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
#include "CoolInterface.h"
#include "CtrlBrowseTree.h"
#include "ShellIcons.h"
#include "G2Packet.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "XML.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ITEM_HEIGHT	17
#define WM_UPDATE	(WM_APP+80)

IMPLEMENT_DYNAMIC(CBrowseTreeCtrl, CWnd)

BEGIN_MESSAGE_MAP(CBrowseTreeCtrl, CWnd)
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
	ON_MESSAGE(WM_UPDATE,OnUpdate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeCtrl construction

CBrowseTreeCtrl::CBrowseTreeCtrl()
{
	m_pRoot = new CBrowseTreeItem();
	m_pRoot->m_bExpanded = TRUE;

	m_nTotal		= 0;
	m_nVisible		= 0;
	m_nScroll		= 0;
	m_nSelected		= 0;
	m_pSelFirst		= NULL;
	m_pSelLast		= NULL;
	m_pFocus		= NULL;
	m_nCleanCookie	= 0;
}

CBrowseTreeCtrl::~CBrowseTreeCtrl()
{
	delete m_pRoot;
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeCtrl operations

BOOL CBrowseTreeCtrl::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	return CWnd::Create( NULL, _T("CBrowseTreeCtrl"),
		WS_CHILD|WS_TABSTOP|WS_VSCROLL, rect, pParentWnd, IDC_BROWSE_TREE, NULL );
}

void CBrowseTreeCtrl::Clear(BOOL bGUI)
{
	CSingleLock lRoot( &m_csRoot, TRUE );

	if ( m_pRoot->m_nCount == 0 ) return;

	m_pRoot->Clear();

	m_nTotal		= 0;
	m_nSelected		= 0;
	m_pSelFirst		= NULL;
	m_pSelLast		= NULL;
	m_pFocus		= NULL;

	if ( bGUI )
	{
		PostMessage( WM_UPDATE );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeCtrl expand

BOOL CBrowseTreeCtrl::Expand(CBrowseTreeItem* pItem, TRISTATE bExpand, BOOL bInvalidate)
{
	CSingleLock lRoot( &m_csRoot, TRUE );

	if ( pItem == NULL ) return FALSE;

	switch ( bExpand )
	{
	case TRI_UNKNOWN:
		pItem->m_bExpanded = ! pItem->m_bExpanded;
		break;
	case TRI_TRUE:
		if ( pItem->m_bExpanded ) return FALSE;
		pItem->m_bExpanded = TRUE;
		break;
	case TRI_FALSE:
		if ( ! pItem->m_bExpanded ) return FALSE;
		pItem->m_bExpanded = FALSE;
		break;
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

	pItem->m_bContract1 = pItem->m_bExpanded == TRUE && bExpand == TRI_TRUE && bInvalidate == FALSE;

	if ( pItem->m_bContract1 == FALSE )
	{
		for ( CBrowseTreeItem* pParent = pItem ; pParent != NULL ; pParent = pParent->m_pParent )
			pParent->m_bContract1 = FALSE;
	}

	if ( bInvalidate )
	{
		PostMessage( WM_UPDATE );
	}

	return TRUE;
}

BOOL CBrowseTreeCtrl::CollapseRecursive(CBrowseTreeItem* pItem)
{
	CSingleLock lRoot( &m_csRoot, TRUE );
	BOOL bChanged = FALSE;

	if ( pItem != m_pRoot && pItem->m_bExpanded && pItem->m_bContract1 )
	{
		bChanged |= Expand( pItem, TRI_FALSE, FALSE );
	}

	CBrowseTreeItem** pChild = pItem->m_pList;

	for ( int nCount = pItem->m_nCount ; nCount ; nCount--, pChild++ )
	{
		bChanged |= CollapseRecursive( *pChild );
	}

	return bChanged;
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeCtrl selection

BOOL CBrowseTreeCtrl::Select(CBrowseTreeItem* pItem, TRISTATE bSelect, BOOL bInvalidate)
{
	CSingleLock lRoot( &m_csRoot, TRUE );

	if ( pItem == NULL ) return FALSE;

	switch ( bSelect )
	{
	case TRI_UNKNOWN:
		pItem->m_bSelected = ! pItem->m_bSelected;
		break;
	case TRI_TRUE:
		if ( pItem->m_bSelected ) return FALSE;
		pItem->m_bSelected = TRUE;
		break;
	case TRI_FALSE:
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

BOOL CBrowseTreeCtrl::DeselectAll(CBrowseTreeItem* pExcept, CBrowseTreeItem* pParent, BOOL bInvalidate)
{
	CSingleLock lRoot( &m_csRoot, TRUE );

	if ( pParent == NULL ) pParent = m_pRoot;
	CBrowseTreeItem** pChild = pParent->m_pList;

	BOOL bChanged = FALSE;

	for ( int nCount = pParent->m_nCount ; nCount ; nCount--, pChild++ )
	{
		if ( *pChild != pExcept && (*pChild)->m_bSelected )
		{
			Select( *pChild, TRI_FALSE, FALSE );
			bChanged = TRUE;
		}

		if ( (*pChild)->m_nCount ) bChanged |= DeselectAll( pExcept, *pChild, FALSE );
	}

	if ( bInvalidate && bChanged && pParent == m_pRoot ) Invalidate();

	return bChanged;
}

int CBrowseTreeCtrl::GetSelectedCount() const
{
	return m_nSelected;
}

CBrowseTreeItem* CBrowseTreeCtrl::GetFirstSelected() const
{
	return m_pSelFirst;
}

CBrowseTreeItem* CBrowseTreeCtrl::GetLastSelected() const
{
	return m_pSelLast;
}

BOOL CBrowseTreeCtrl::Highlight(CBrowseTreeItem* pItem)
{
	CSingleLock lRoot( &m_csRoot, TRUE );

	m_pFocus = pItem;

	for ( CBrowseTreeItem* pParent = m_pFocus->m_pParent ; pParent ; pParent = pParent->m_pParent )
	{
		Expand( pParent, TRI_TRUE, FALSE );

		pParent->m_bContract2 = pParent->m_bContract1;
		pParent->m_bContract1 = FALSE;
	}

	CollapseRecursive( m_pRoot );

	for ( CBrowseTreeItem* pParent = m_pFocus->m_pParent ; pParent ; pParent = pParent->m_pParent )
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

	PostMessage( WM_UPDATE );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeCtrl internal helpers

BOOL CBrowseTreeCtrl::CleanItems(CBrowseTreeItem* pItem, DWORD nCookie, BOOL bVisible)
{
	CSingleLock lRoot( &m_csRoot, TRUE );

	CBrowseTreeItem** pChild = pItem->m_pList + pItem->m_nCount - 1;
	BOOL bChanged = FALSE;

	for ( int nChild = pItem->m_nCount ; nChild ; nChild--, pChild-- )
	{
		if ( (*pChild)->m_nCleanCookie != nCookie )
		{
			if ( m_pFocus == *pChild ) m_pFocus = NULL;

			if ( (*pChild)->m_bSelected ) Select( *pChild, TRI_FALSE, FALSE );
			bChanged |= DeselectAll( NULL, *pChild, FALSE );

			if ( bVisible )
			{
				m_nTotal -= (*pChild)->GetChildCount() + 1;
				bChanged = TRUE;
			}

			delete *pChild;
			MoveMemory( pChild, pChild + 1, ( pItem->m_nCount - nChild ) * sizeof( CBrowseTreeItem* ) );
			pItem->m_nCount--;
		}
	}

	return bChanged;
}

void CBrowseTreeCtrl::NotifySelection()
{
	NMHDR pNM = { GetSafeHwnd(), (UINT_PTR)GetDlgCtrlID(), BTN_SELCHANGED };
	GetOwner()->SendMessage( WM_NOTIFY, pNM.idFrom, (LPARAM)&pNM );
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeCtrl message handlers

void CBrowseTreeCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );
	m_nVisible = cy;
	UpdateScroll();
}

void CBrowseTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CSingleLock lRoot( &m_csRoot, TRUE );

	CRect rc;
	CBrowseTreeItem* pHit = HitTest( point, &rc );
	BOOL bChanged = FALSE;

	SetFocus();

	if ( pHit && pHit->m_nCount && point.x >= rc.left && point.x < rc.left + 16 )
	{
		bChanged = Expand( pHit, TRI_UNKNOWN );
	}
	else if ( nFlags & MK_CONTROL )
	{
		if ( pHit ) bChanged = Select( pHit, TRI_UNKNOWN );
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

	if ( pHit ) m_pFocus = pHit;

	if ( bChanged )
	{
		lRoot.Unlock();
		NotifySelection();
	}
}

void CBrowseTreeCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CSingleLock lRoot( &m_csRoot, TRUE );

	OnLButtonDown( nFlags, point );

	if ( m_pFocus != NULL && m_pFocus->m_nCount )
	{
		if ( Expand( m_pFocus, TRI_UNKNOWN ) )
		{
			lRoot.Unlock();
			NotifySelection();
		}
	}
}

void CBrowseTreeCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	OnLButtonDown( nFlags, point );
	CWnd::OnRButtonDown( nFlags, point );
}

void CBrowseTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove( nFlags, point );

	// [+] or [-] Hoverstates
	if ( ! nFlags && point.x < 32 )
	{
		CRect rcRefresh( 1, point.y - 48, 32, point.y + 48 );
		RedrawWindow(rcRefresh);
	}
}

void CBrowseTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();

	CWnd::OnLButtonUp( nFlags, point );
}

void CBrowseTreeCtrl::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	CSingleLock lRoot( &m_csRoot, TRUE );
	CBrowseTreeItem* pTo = NULL;
	BOOL bChanged = FALSE;
	CRect rc;

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
		for (;;)
		{
			if ( m_pFocus->m_bExpanded && m_pFocus->m_nCount )
			{
				Expand( m_pFocus, TRI_FALSE );
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
			bChanged |= Expand( m_pFocus, TRI_TRUE );
		}
	}
	else if ( _istalnum( TCHAR( nChar ) ) )
	{
		CBrowseTreeItem* pStart	= m_pFocus;
		CBrowseTreeItem* pBase	= pStart ? pStart->m_pParent : m_pRoot;

		for ( int nLoop = 0 ; nLoop < 2 ; nLoop++ )
		{
			CBrowseTreeItem** pChild = pBase->m_pList;

			for ( int nCount = pBase->m_nCount ; nCount ; nCount--, pChild++ )
			{
				if ( pStart != NULL )
				{
					if ( pStart == *pChild ) pStart = NULL;
				}
				else if ( toupper( (*pChild)->m_sText.GetAt( 0 ) ) == (int)nChar )
				{
					DeselectAll( m_pFocus = *pChild, NULL, FALSE );
					Select( m_pFocus, TRI_TRUE, FALSE );
					Highlight( m_pFocus );
					lRoot.Unlock();
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

	if ( bChanged )
	{
		lRoot.Unlock();
		NotifySelection();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeCtrl scrolling

void CBrowseTreeCtrl::UpdateScroll()
{
	CSingleLock lRoot( &m_csRoot, TRUE );
	SCROLLINFO pInfo;

	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= SIF_PAGE | SIF_POS | SIF_RANGE;
	pInfo.nMin		= 0;
	pInfo.nMax		= m_nTotal * ITEM_HEIGHT;
	pInfo.nPage		= m_nVisible;
	pInfo.nPos		= m_nScroll = max( 0, min( m_nScroll, pInfo.nMax - (int)pInfo.nPage + 1 ) );

	SetScrollInfo( SB_VERT, &pInfo, IsWindowVisible() );
}

void CBrowseTreeCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	switch ( nSBCode )
	{
	case SB_BOTTOM:
		ScrollTo( 0xFFFFFFFF );
		break;
	case SB_LINEDOWN:
		ScrollBy( 1 );
		break;
	case SB_LINEUP:
		ScrollBy( -1 );
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

BOOL CBrowseTreeCtrl::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	ScrollBy( zDelta * 3 * -ITEM_HEIGHT / WHEEL_DELTA );
	return TRUE;
}

void CBrowseTreeCtrl::ScrollBy(int nDelta)
{
	ScrollTo( max( 0, m_nScroll + nDelta ) );
}

void CBrowseTreeCtrl::ScrollTo(int nPosition)
{
	CSingleLock lRoot( &m_csRoot, TRUE );

	if ( nPosition == m_nScroll ) return;
	m_nScroll = nPosition;

	UpdateScroll();

	CRect rc;
	GetClientRect( &rc );
	RedrawWindow( &rc, NULL, RDW_INVALIDATE );
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeCtrl painting

BOOL CBrowseTreeCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CBrowseTreeCtrl::OnPaint()
{
	CSingleLock lRoot( &m_csRoot, TRUE );
	CPaintDC dc( this );

	CRect rcClient;
	GetClientRect( &rcClient );

	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	CBrowseTreeItem** pChild = m_pRoot->m_pList;

	CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );

	for ( int nCount = m_pRoot->m_nCount ; nCount && pt.y < rcClient.bottom ; nCount--, pChild++ )
	{
		Paint( dc, rcClient, pt, *pChild );
	}

	dc.SelectObject( pOldFont );

	dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );
}

void CBrowseTreeCtrl::Paint(CDC& dc, CRect& rcClient, CPoint& pt, CBrowseTreeItem* pItem)
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
			pItem->Paint( dc, rc, FALSE );
			dc.ExcludeClipRect( &rc );
		}

		if ( pItem->m_bBold ) dc.SelectObject( &CoolInterface.m_fntNormal );
	}

	if ( pItem->m_bExpanded && pItem->m_nCount )
	{
		pt.x += 16;

		CBrowseTreeItem** pChild = pItem->m_pList;

		for ( int nCount = pItem->m_nCount ; nCount ; nCount--, pChild++ )
		{
			Paint( dc, rcClient, pt, *pChild );
			if ( pt.y >= rcClient.bottom ) break;
		}

		pt.x -= 16;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeCtrl hit testing

CBrowseTreeItem* CBrowseTreeCtrl::HitTest(const POINT& point, RECT* pRect) const
{
	CSingleLock lRoot( (CCriticalSection*)&m_csRoot, TRUE );
	CRect rcClient;

	GetClientRect( &rcClient );

	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	CBrowseTreeItem** pChild = m_pRoot->m_pList;

	for ( int nCount = m_pRoot->m_nCount ; nCount && pt.y < rcClient.bottom ; nCount--, pChild++ )
	{
		CBrowseTreeItem* pItem = HitTest( rcClient, pt, *pChild, point, pRect );
		if ( pItem ) return pItem;
	}

	return NULL;
}

CBrowseTreeItem* CBrowseTreeCtrl::HitTest(CRect& rcClient, CPoint& pt, CBrowseTreeItem* pItem, const POINT& point, RECT* pRect) const
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

		CBrowseTreeItem** pChild = pItem->m_pList;

		for ( int nCount = pItem->m_nCount ; nCount ; nCount--, pChild++ )
		{
			if ( CBrowseTreeItem* pHit = HitTest( rcClient, pt, *pChild, point, pRect ) )
				return pHit;
			if ( pt.y >= rcClient.bottom + ITEM_HEIGHT ) break;
		}

		pt.x -= 16;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeCtrl rect lookup

BOOL CBrowseTreeCtrl::GetRect(CBrowseTreeItem* pItem, RECT* pRect)
{
	CSingleLock lRoot( &m_csRoot, TRUE );
	CRect rcClient;

	GetClientRect( &rcClient );

	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	CBrowseTreeItem** pChild = m_pRoot->m_pList;

	for ( int nCount = m_pRoot->m_nCount ; nCount ; nCount--, pChild++ )
	{
		if ( GetRect( pt, *pChild, pItem, pRect ) ) return TRUE;
	}

	return FALSE;
}

BOOL CBrowseTreeCtrl::GetRect(CPoint& pt, CBrowseTreeItem* pItem, CBrowseTreeItem* pFind, RECT* pRect)
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

		CBrowseTreeItem** pChild = pItem->m_pList;

		for ( int nCount = pItem->m_nCount ; nCount ; nCount--, pChild++ )
		{
			if ( GetRect( pt, *pChild, pFind, pRect ) ) return TRUE;
		}

		pt.x -= 16;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeCtrl virtual tree

void CBrowseTreeCtrl::OnTreePacket(CG2Packet* pPacket)
{
	CSingleLock lRoot( &m_csRoot, TRUE );

	Clear( FALSE );

	pPacket->Seek( 0 );
	OnTreePacket( pPacket, pPacket->m_nLength, m_pRoot );
	m_nTotal = m_pRoot->GetChildCount();

	PostMessage( WM_UPDATE );
}

void CBrowseTreeCtrl::OnTreePacket(CG2Packet* pPacket, DWORD nFinish, CBrowseTreeItem* pItem)
{
	BOOL bCompound;
	G2_PACKET nType;
	DWORD nLength;

	while (	pPacket->m_nPosition < nFinish &&
			pPacket->ReadPacket( nType, nLength, &bCompound ) )
	{
		DWORD nNext = pPacket->m_nPosition + nLength;

		if ( ( nType == G2_PACKET_PHYSICAL_FOLDER || nType == G2_PACKET_VIRTUAL_FOLDER ) && bCompound == TRUE )
		{
			CBrowseTreeItem* pChild = new CBrowseTreeItem( pItem );
			OnTreePacket( pPacket, nNext, pChild );
			pChild->m_bExpanded = ( pItem == m_pRoot );
			pItem->Add( pChild );
		}
		else if ( nType == G2_PACKET_DESCRIPTIVE_NAME && bCompound == FALSE )
		{
			pItem->m_sText = pPacket->ReadString( nLength );
		}
		else if ( nType == G2_PACKET_METADATA && bCompound == FALSE )
		{
			CAutoPtr< CXMLElement > pXML( CXMLElement::FromString( pPacket->ReadString( nLength ) ) );
			pItem->AddXML( pXML );
		}
		else if ( nType == G2_PACKET_FILES && bCompound == FALSE )
		{
			if ( pItem->m_pFiles != NULL ) delete [] pItem->m_pFiles;

			pItem->m_nFiles = nLength / 4;
			pItem->m_pFiles = new DWORD[ pItem->m_nFiles ];

			for ( DWORD nCount = pItem->m_nFiles ; nCount ; nCount-- )
			{
				pItem->m_pFiles[ nCount - 1 ] = pPacket->ReadLongBE();
			}
		}

		pPacket->m_nPosition = nNext;
	}
}

LRESULT CBrowseTreeCtrl::OnUpdate(WPARAM, LPARAM)
{
	UpdateScroll();
	Invalidate();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeItem construction

CBrowseTreeItem::CBrowseTreeItem(CBrowseTreeItem* pParent)
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

	m_nCookie		= 0;
	m_bBold			= FALSE;
	m_nIcon16		= -1;

	m_pSchema		= NULL;
	m_pFiles		= NULL;
	m_nFiles		= 0;
}

CBrowseTreeItem::~CBrowseTreeItem()
{
	if ( m_pFiles != NULL )
	{
		delete [] m_pFiles;
	}

	if ( m_pList != NULL )
	{
		Clear();
		delete [] m_pList;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeItem add

CBrowseTreeItem* CBrowseTreeItem::Add(LPCTSTR pszName)
{
	if ( m_nCount == m_nBuffer )
	{
		if ( m_nBuffer ) m_nBuffer += min( m_nBuffer, 16 ); else m_nBuffer = 4;

		CBrowseTreeItem** pList = new CBrowseTreeItem*[ m_nBuffer ];

		if ( m_pList )
		{
			if ( m_nCount ) CopyMemory( pList, m_pList, m_nCount * sizeof( CBrowseTreeItem* ) );
			delete [] m_pList;
		}

		m_pList = pList;
	}

	if ( m_nCount == 0 ) return m_pList[ m_nCount++ ] = new CBrowseTreeItem( this );

	int nFirst = 0;
	for ( int nLast = m_nCount - 1 ; nLast >= nFirst ; )
	{
		int nMiddle = ( nFirst + nLast ) >> 1;

		CBrowseTreeItem* pItem = m_pList[ nMiddle ];

		if ( _tcsicoll( pszName, pItem->m_sText ) >= 0 )
		{
			nFirst = nMiddle + 1;
		}
		else
		{
			nLast = nMiddle - 1;
		}
	}

	MoveMemory( m_pList + nFirst + 1, m_pList + nFirst, ( m_nCount - nFirst ) * sizeof( CBrowseTreeItem* ) );
	m_nCount++;

	return m_pList[ nFirst ] = new CBrowseTreeItem( this );
}

CBrowseTreeItem* CBrowseTreeItem::Add(CBrowseTreeItem* pNewItem)
{
	if ( m_nCount == m_nBuffer )
	{
		if ( m_nBuffer ) m_nBuffer += min( m_nBuffer, 16 ); else m_nBuffer = 4;

		CBrowseTreeItem** pList = new CBrowseTreeItem*[ m_nBuffer ];

		if ( m_pList )
		{
			if ( m_nCount ) CopyMemory( pList, m_pList, m_nCount * sizeof( CBrowseTreeItem* ) );
			delete [] m_pList;
		}

		m_pList = pList;
	}

	if ( m_nCount == 0 ) return m_pList[ m_nCount++ ] = pNewItem;

	int nFirst = 0;
	for ( int nLast = m_nCount - 1 ; nLast >= nFirst ; )
	{
		int nMiddle = ( nFirst + nLast ) >> 1;

		CBrowseTreeItem* pItem = m_pList[ nMiddle ];

		if ( _tcsicoll( pNewItem->m_sText, pItem->m_sText ) >= 0 )
		{
			nFirst = nMiddle + 1;
		}
		else
		{
			nLast = nMiddle - 1;
		}
	}

	MoveMemory( m_pList + nFirst + 1, m_pList + nFirst, ( m_nCount - nFirst ) * sizeof( CBrowseTreeItem* ) );
	m_nCount++;

	return m_pList[ nFirst ] = pNewItem;
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeItem delete

void CBrowseTreeItem::Delete()
{
	m_pParent->Delete( this );
}

void CBrowseTreeItem::Delete(CBrowseTreeItem* pItem)
{
	ASSERT( pItem->m_bSelected == FALSE );

	CBrowseTreeItem** pChild = m_pList;

	for ( int nChild = m_nCount ; nChild ; nChild--, pChild++ )
	{
		if ( *pChild == pItem )
		{
			MoveMemory( pChild, pChild + 1, ( nChild - 1 ) * sizeof( CBrowseTreeItem* ) );
			m_nCount--;
			break;
		}
	}

	delete pItem;
}

void CBrowseTreeItem::Delete(int nItem)
{
	if ( nItem < 0 || nItem >= m_nCount ) return;

	ASSERT( m_pList[ nItem ]->m_bSelected == FALSE );
	delete m_pList[ nItem ];
	MoveMemory( m_pList + nItem, m_pList + nItem + 1, ( m_nCount - nItem - 1 ) * sizeof( CBrowseTreeItem* ) );
	m_nCount--;
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeItem clear

void CBrowseTreeItem::Clear()
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
// CBrowseTreeItem visibility

BOOL CBrowseTreeItem::IsVisible() const
{
	for ( CBrowseTreeItem* pRoot = m_pParent ; pRoot ; pRoot = pRoot->m_pParent )
	{
		if ( ! pRoot->m_bExpanded ) return FALSE;
	}

	return TRUE;
}

int CBrowseTreeItem::GetChildCount() const
{
	int nCount = m_nCount;

	CBrowseTreeItem** pChild = m_pList;

	for ( int nChild = m_nCount ; nChild ; nChild--, pChild++ )
	{
		if ( (*pChild)->m_bExpanded ) nCount += (*pChild)->GetChildCount();
	}

	return nCount;
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseTreeItem paint

void CBrowseTreeItem::Paint(CDC& dc, CRect& rc, BOOL bTarget, COLORREF crBack) const
{
	POINT ptHover;
	RECT  rcTick = { rc.left+2, rc.top+2, rc.left+14, rc.bottom-2 };
	GetCursorPos( &ptHover );
	if ( dc.GetWindow()!= NULL )
		dc.GetWindow()->ScreenToClient( &ptHover );

	if ( crBack == CLR_NONE ) crBack = CoolInterface.m_crWindow;
	dc.FillSolidRect( rc.left, rc.top, 32, 17, crBack );

	if ( m_bExpanded )
	{
		CoolInterface.Draw( &dc, PtInRect(&rcTick,ptHover) ? IDI_MINUS_HOVER : IDI_MINUS,
			16, rc.left, rc.top, crBack );
	}
	else if ( m_nCount )
	{
		CoolInterface.Draw( &dc, PtInRect(&rcTick,ptHover) ? IDI_PLUS_HOVER : IDI_PLUS,
			16, rc.left, rc.top, crBack );
	}

	if ( m_nIcon16 >= 0 )
		// Draw custom icon
		ShellIcons.Draw( &dc, m_nIcon16, 16,
			rc.left + 16, rc.top, crBack, ( m_bSelected || bTarget ) );
	else
		// Draw standard icon
		CoolInterface.Draw( &dc,
			( ( m_bExpanded && m_nCount ) ? IDI_FOLDER_OPEN : IDI_FOLDER_CLOSED ),
			16, rc.left + 16, rc.top, crBack, ( m_bSelected || bTarget ) );

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
// CBrowseTreeItem add XML

void CBrowseTreeItem::AddXML(const CXMLElement* pXML)
{
	if ( ! pXML )
		return;

	CString strURI = pXML->GetAttributeValue( CXMLAttribute::schemaName );

	if ( ( m_pSchema = SchemaCache.Get( strURI ) ) != NULL )
	{
		m_bBold		= CheckURI( strURI, CSchema::uriFavouritesFolder );
		m_nIcon16	= m_pSchema->m_nIcon16;

		if ( CXMLElement* pBody = pXML->GetFirstElement() )
		{
			m_sText = pBody->GetAttributeValue( m_pSchema->GetFirstMemberName() );
		}
	}

	if ( m_sText.IsEmpty() ) m_sText = _T("Unnamed");
}
