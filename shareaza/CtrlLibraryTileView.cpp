//
// CtrlLibraryTileView.cpp
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
#include "Library.h"
#include "LibraryFolders.h"
#include "AlbumFolder.h"
#include "Schema.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryTree.h"
#include "CtrlLibraryTileView.h"
#include "DlgFolderProperties.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CLibraryTileView, CLibraryView)
	//{{AFX_MSG_MAP(CLibraryTileView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()
	ON_WM_CHAR()
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_ALBUM_OPEN, OnUpdateLibraryAlbumOpen)
	ON_COMMAND(ID_LIBRARY_ALBUM_OPEN, OnLibraryAlbumOpen)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_ALBUM_DELETE, OnUpdateLibraryAlbumDelete)
	ON_COMMAND(ID_LIBRARY_ALBUM_DELETE, OnLibraryAlbumDelete)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_ALBUM_PROPERTIES, OnUpdateLibraryAlbumProperties)
	ON_COMMAND(ID_LIBRARY_ALBUM_PROPERTIES, OnLibraryAlbumProperties)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryTileView construction

CLibraryTileView::CLibraryTileView()
{
	m_nCommandID	= ID_LIBRARY_VIEW_TILE;
	m_pszToolBar	= _T("CLibraryTileView");
}

CLibraryTileView::~CLibraryTileView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTileView create and destroy

BOOL CLibraryTileView::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CLibraryView::PreCreateWindow( cs );
}

int CLibraryTileView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CLibraryView::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	m_szBlock.cx	= 252;
	m_szBlock.cy	= 56;
	m_nColumns		= 1;
	m_nRows			= 0;
	
	m_pList			= NULL;
	m_nCount		= 0;
	m_nBuffer		= 0;
	m_nScroll		= 0;
	m_nSelected		= 0;
	m_pFocus		= NULL;
	m_pFirst		= NULL;
	m_bDrag			= FALSE;
	
	return 0;
}

void CLibraryTileView::OnDestroy() 
{
	Clear();
	CLibraryView::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTileView view operations

BOOL CLibraryTileView::CheckAvailable(CLibraryTreeItem* pSel)
{
	m_bAvailable = FALSE;
	
	if ( pSel != NULL && pSel->m_pSelNext == NULL && pSel->m_pVirtual != NULL )
	{
		m_bAvailable = ( pSel->m_pVirtual->GetFileCount() == 0 );
	}
	
	return m_bAvailable;
}

void CLibraryTileView::Update()
{
	CLibraryTreeItem* pFolders	= GetFolderSelection();
	CAlbumFolder* pFolder		= NULL;

	if ( pFolders == NULL || pFolders->m_pVirtual == NULL )
	{
		pFolder = Library.GetAlbumRoot();
	}
	else
	{
		if (	pFolders == NULL || pFolders->m_pSelNext != NULL ||
				pFolders->m_pVirtual == NULL ||
				pFolders->m_pVirtual->GetFileCount() > 0 )
		{
			if ( m_nCount > 0 )
			{
				Clear();
				Invalidate();
			}

			return;
		}

		pFolder = pFolders->m_pVirtual;
	}

	DWORD nCookie = GetFolderCookie();
	BOOL bChanged = FALSE;

	CLibraryTileItem** pList = m_pList + m_nCount - 1;

	for ( int nItem = m_nCount ; nItem ; nItem--, pList-- )
	{
		CLibraryTileItem* pTile	= *pList;

		if ( pFolder->CheckFolder( pTile->m_pFolder ) )
		{
			bChanged |= pTile->Update();
			pTile->m_pFolder->m_nListCookie = nCookie;
		}
		else
		{
			if ( pTile->m_bSelected ) Select( pTile, TS_FALSE );
			if ( pTile == m_pFocus ) m_pFocus = NULL;
			if ( pTile == m_pFirst ) m_pFirst = NULL;
			
			delete pTile;
			MoveMemory( pList, pList + 1, 4 * ( m_nCount - nItem ) );
			m_nCount--;

			bChanged = TRUE;
		}
	}

	if ( bChanged )
	{
		CRect rcClient;
		GetClientRect( &rcClient );
		int nMax	= ( ( m_nCount + m_nColumns - 1 ) / m_nColumns ) * m_szBlock.cy;
		m_nScroll	= max( 0, min( m_nScroll, nMax - rcClient.Height() + 1 ) );
	}

	for ( POSITION pos = pFolder->GetFolderIterator() ; pos ; )
	{
		CAlbumFolder* pChild = pFolder->GetNextFolder( pos );
		
		if ( pChild->m_nListCookie != nCookie )
		{
			CLibraryTileItem* pTile = new CLibraryTileItem( pChild );
			
			if ( m_nCount == m_nBuffer )
			{
				m_nBuffer += 64;
				CLibraryTileItem** pList = new CLibraryTileItem*[ m_nBuffer ];
				if ( m_nCount ) CopyMemory( pList, m_pList, 4 * m_nCount );
				if ( m_pList ) delete [] m_pList;
				m_pList = pList;
			}

			m_pList[ m_nCount++ ] = pTile;
			pChild->m_nListCookie = nCookie;
			bChanged = TRUE;
		}
	}

	if ( bChanged )
	{
		qsort( m_pList, m_nCount, 4, SortList );
		UpdateScroll();
	}
}

BOOL CLibraryTileView::Select(DWORD nObject)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTileView item list management operations

void CLibraryTileView::Clear()
{
	for ( int nItem = 0 ; nItem < m_nCount ; nItem++ )
	{
		delete m_pList[ nItem ];
	}

	if ( m_pList ) delete [] m_pList;

	m_pList		= NULL;
	m_nCount	= 0;
	m_nBuffer	= 0;
	m_nScroll	= 0;
	m_nSelected	= 0;
	m_pFocus	= NULL;
	m_pFirst	= NULL;

	m_pSelTile.RemoveAll();
	SelClear();
}

int CLibraryTileView::GetTileIndex(CLibraryTileItem* pTile) const
{
	CLibraryTileItem** pList = m_pList;

	for ( int nItem = 0 ; nItem < m_nCount ; nItem++, pList++ )
	{
		if ( *pList == pTile ) return nItem;
	}

	return -1;
}

BOOL CLibraryTileView::Select(CLibraryTileItem* pTile, TRISTATE bSelect)
{
	switch ( bSelect )
	{
	case TS_UNKNOWN:
		pTile->m_bSelected = ! pTile->m_bSelected;
		break;
	case TS_FALSE:
		if ( pTile->m_bSelected == FALSE ) return FALSE;
		pTile->m_bSelected = FALSE;
		break;
	case TS_TRUE:
		if ( pTile->m_bSelected == TRUE ) return FALSE;
		pTile->m_bSelected = TRUE;
		break;
	}

	if ( pTile->m_bSelected )
	{
		m_nSelected++;
		m_pSelTile.AddTail( pTile );
		SelAdd( (DWORD)pTile->m_pFolder );
	}
	else
	{
		m_nSelected--;
		if ( POSITION pos = m_pSelTile.Find( pTile ) )
		{
			m_pSelTile.RemoveAt( pos );
			SelRemove( (DWORD)pTile->m_pFolder );
		}
	}

	return TRUE;
}

BOOL CLibraryTileView::DeselectAll(CLibraryTileItem* pTile)
{
	CLibraryTileItem** pList = m_pList + m_nCount - 1;
	BOOL bChanged = FALSE;

	for ( int nItem = m_nCount ; nItem ; nItem--, pList-- )
	{
		if ( *pList != pTile )
		{
			if ( (*pList)->m_bSelected ) bChanged = Select( *pList, TS_FALSE );
		}
	}

	return bChanged;
}

BOOL CLibraryTileView::SelectTo(CLibraryTileItem* pTile)
{
	BOOL bChanged = FALSE;

	if ( pTile )
	{
		m_pFocus = pTile;

		int nFirst	= GetTileIndex( m_pFirst );
		int nFocus	= GetTileIndex( m_pFocus );
		
		if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
		{
			bChanged = Select( m_pFocus, TS_UNKNOWN );
		}
		else if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			bChanged = DeselectAll();

			if ( nFirst >= 0 && nFocus >= 0 )
			{
				if ( nFirst <= nFocus )
				{
					for ( ; nFirst <= nFocus ; nFirst++ ) Select( m_pList[ nFirst ], TS_TRUE );
				}
				else
				{
					for ( ; nFocus <= nFirst ; nFocus++ ) Select( m_pList[ nFocus ], TS_TRUE );
				}

				bChanged = TRUE;
			}
			else
			{
				bChanged |= Select( m_pFocus, TS_TRUE );
			}
		}
		else
		{
			if ( m_pFocus->m_bSelected == FALSE ) bChanged = DeselectAll( m_pFocus );
			bChanged |= Select( m_pFocus );
		}

		if ( m_nSelected == 1 && m_pFocus->m_bSelected ) m_pFirst = m_pFocus;

		Highlight( m_pFocus );
	}
	else if (	( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 &&
				( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) == 0 )
	{
		bChanged = DeselectAll();
	}

	if ( m_nSelected == 0 ) m_pFirst = NULL;

	return bChanged;
}

void CLibraryTileView::SelectTo(int nDelta)
{
	if ( m_nCount == 0 ) return;

	int nFocus = GetTileIndex( m_pFocus );

	if ( nFocus < 0 )
	{
		nFocus = 0;
	}
	else
	{
		nFocus += nDelta;
		if ( nFocus < 0 ) nFocus = 0;
		if ( nFocus >= m_nCount ) nFocus = m_nCount - 1;
	}

	if ( SelectTo( m_pList[ nFocus ] ) ) Invalidate();
}

void CLibraryTileView::Highlight(CLibraryTileItem* pItem)
{
	CRect rcClient, rcItem;

	GetClientRect( &rcClient );
	GetItemRect( pItem, &rcItem );

	if ( rcItem.top < rcClient.top )
	{
		ScrollBy( rcItem.top - rcClient.top );
	}
	else if ( rcItem.bottom > rcClient.bottom )
	{
		ScrollBy( rcItem.bottom - rcClient.bottom );
	}
}

int CLibraryTileView::SortList(LPCVOID pA, LPCVOID pB)
{
	CLibraryTileItem* ppA = *(CLibraryTileItem**)pA;
	CLibraryTileItem* ppB = *(CLibraryTileItem**)pB;
	return _tcsicoll( ppA->m_sTitle, ppB->m_sTitle );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTileView message handlers

void CLibraryTileView::OnSize(UINT nType, int cx, int cy) 
{
	CLibraryView::OnSize( nType, cx, cy );
	
	m_nColumns	= cx / m_szBlock.cx;
	m_nRows		= cy / m_szBlock.cy + 1;
	
	m_nColumns = max( m_nColumns, 1 );
	
	UpdateScroll();
}

void CLibraryTileView::UpdateScroll()
{
	if ( m_nColumns == 0 ) return;

	SCROLLINFO pInfo;
	CRect rc;

	GetClientRect( &rc );

	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;
	pInfo.nMin		= 0;
	pInfo.nMax		= ( ( m_nCount + m_nColumns - 1 ) / m_nColumns ) * m_szBlock.cy;
	pInfo.nPage		= rc.Height();;
	pInfo.nPos		= m_nScroll = max( 0, min( m_nScroll, pInfo.nMax - (int)pInfo.nPage + 1 ) );
	
	SetScrollInfo( SB_VERT, &pInfo, TRUE );

	Invalidate();
}

void CLibraryTileView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CRect rc;
	GetClientRect( &rc );

	SetFocus();

	switch ( nSBCode )
	{
	case SB_BOTTOM:
		ScrollTo( 0xFFFFFF );
		break;
	case SB_LINEDOWN:
		ScrollBy( 32 );
		break;
	case SB_LINEUP:
		ScrollBy( -32 );
		break;
	case SB_PAGEDOWN:
		ScrollBy( rc.Height() );
		break;
	case SB_PAGEUP:
		ScrollBy( -rc.Height() );
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

BOOL CLibraryTileView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	ScrollBy( zDelta * -m_szBlock.cy / WHEEL_DELTA / 2 );
	return TRUE;
}

void CLibraryTileView::ScrollBy(int nDelta)
{
	ScrollTo( max( 0, m_nScroll + nDelta ) );
}

void CLibraryTileView::ScrollTo(int nPosition)
{
	if ( nPosition == m_nScroll ) return;
	m_nScroll = nPosition;

	UpdateScroll();
	RedrawWindow( NULL, NULL, RDW_INVALIDATE );
}

void CLibraryTileView::OnPaint() 
{
	CPaintDC dc( this );
	
	CDC* pBuffer = CoolInterface.GetBuffer( dc, m_szBlock );
	CRect rcBuffer( 0, 0, m_szBlock.cx, m_szBlock.cy );
	
	CFont* pOldFont = (CFont*)pBuffer->SelectObject( &CoolInterface.m_fntNormal );
	pBuffer->SetBkMode( OPAQUE );
	pBuffer->SetBkColor( CoolInterface.m_crWindow );
	pBuffer->SetTextColor( CoolInterface.m_crText );
	
	CDC dcMem;
	dcMem.CreateCompatibleDC( &dc );
	
	CRect rcClient;
	GetClientRect( &rcClient );
	CPoint pt( rcClient.left, rcClient.top - m_nScroll );
	
	CLibraryTileItem** pList = m_pList;
	
	for ( int nItem = m_nCount ; nItem && pt.y < rcClient.bottom ; nItem--, pList++ )
	{
		CLibraryTileItem* pTile = *pList;
		
		CRect rcBlock( pt.x, pt.y, pt.x + m_szBlock.cx, pt.y + m_szBlock.cy );
		
		if ( rcBlock.bottom >= rcClient.top && dc.RectVisible( &rcBlock ) )
		{
			pBuffer->FillSolidRect( &rcBuffer, CoolInterface.m_crWindow );
			pTile->Paint( pBuffer, rcBuffer, &dcMem );
			dc.BitBlt( rcBlock.left, rcBlock.top, m_szBlock.cx, m_szBlock.cy,
				pBuffer, 0, 0, SRCCOPY );
			dc.ExcludeClipRect( &rcBlock );
		}
		
		pt.x += m_szBlock.cx;
		
		if ( pt.x + m_szBlock.cx > rcClient.right )
		{
			pt.x = rcClient.left;
			pt.y += m_szBlock.cy;
		}
	}
	
	pBuffer->SelectObject( pOldFont );
	dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );
}

CLibraryTileItem* CLibraryTileView::HitTest(const CPoint& point) const
{
	CRect rcClient;
	GetClientRect( &rcClient );
	
	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	CLibraryTileItem** pList = m_pList;

	for ( int nItem = m_nCount ; nItem && pt.y < rcClient.bottom ; nItem--, pList++ )
	{
		CLibraryTileItem* pTile = *pList;

		CRect rcBlock( pt.x, pt.y, pt.x + m_szBlock.cx, pt.y + m_szBlock.cy );

		if ( rcBlock.PtInRect( point ) ) return pTile;

		pt.x += m_szBlock.cx;

		if ( pt.x + m_szBlock.cx > rcClient.right )
		{
			pt.x = rcClient.left;
			pt.y += m_szBlock.cy;
		}
	}

	return NULL;
}

BOOL CLibraryTileView::GetItemRect(CLibraryTileItem* pTile, CRect* pRect)
{
	CRect rcClient;
	GetClientRect( &rcClient );
	
	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	CLibraryTileItem** pList = m_pList;

	for ( int nItem = m_nCount ; nItem ; nItem--, pList++ )
	{
		CRect rcBlock( pt.x, pt.y, pt.x + m_szBlock.cx, pt.y + m_szBlock.cy );

		if ( pTile == *pList )
		{
			*pRect = rcBlock;
			return TRUE;
		}

		pt.x += m_szBlock.cx;

		if ( pt.x + m_szBlock.cx > rcClient.right )
		{
			pt.x = rcClient.left;
			pt.y += m_szBlock.cy;
		}
	}

	return FALSE;
}

void CLibraryTileView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CLibraryTileItem* pHit = HitTest( point );

	if ( SelectTo( pHit ) ) Invalidate();

	SetFocus();
	SetCapture();

	if ( pHit && ( nFlags & MK_RBUTTON ) == 0 && Settings.Library.ShowVirtual )
	{
		m_bDrag = TRUE;
		m_ptDrag = point;
	}

	CLibraryView::OnLButtonDown( nFlags, point );
}

void CLibraryTileView::OnMouseMove(UINT nFlags, CPoint point) 
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

	CLibraryView::OnMouseMove( nFlags, point );
}

void CLibraryTileView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseCapture();
	m_bDrag = FALSE;

	if ( ( nFlags & (MK_SHIFT|MK_CONTROL) ) == 0 && m_pFocus && m_pFocus->m_bSelected )
	{
		if ( DeselectAll( m_pFocus ) ) Invalidate();
	}
}

void CLibraryTileView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	SendMessage( WM_COMMAND, ID_LIBRARY_ALBUM_OPEN );
}

void CLibraryTileView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	OnLButtonDown( nFlags, point );
	CLibraryView::OnRButtonDown( nFlags, point );
}

void CLibraryTileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch ( nChar )
	{
	case VK_LEFT:
		SelectTo( - 1 );
		break;
	case VK_RIGHT:
		SelectTo( 1 );
		break;
	case VK_UP:
		SelectTo( -m_nColumns );
		break;
	case VK_DOWN:
		SelectTo( m_nColumns );
		break;
	case VK_PRIOR:
		SelectTo( m_nRows * -m_nColumns );
		break;
	case VK_NEXT:
		ScrollBy( m_nRows * m_nColumns );
		break;
	case VK_HOME:
		SelectTo( -m_nCount );
		break;
	case VK_END:
		SelectTo( m_nCount );
		break;
	case VK_RETURN:
		PostMessage( WM_COMMAND, ID_LIBRARY_ALBUM_OPEN );
		break;
	}

	CLibraryView::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CLibraryTileView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ( _istalnum( nChar ) )
	{
		CLibraryTileItem* pStart = m_pFocus;
				
		for ( int nLoop = 0 ; nLoop < 2 ; nLoop++ )
		{
			CLibraryTileItem** pChild = m_pList;

			for ( int nCount = m_nCount ; nCount ; nCount--, pChild++ )
			{
				if ( pStart != NULL )
				{
					if ( pStart == *pChild ) pStart = NULL;
				}
				else if ( toupper( (*pChild)->m_sTitle.GetAt( 0 ) ) == toupper( (int)nChar ) )
				{
					DeselectAll( m_pFocus = *pChild );
					Select( m_pFocus, TS_TRUE );
					Highlight( m_pFocus );
					Invalidate();
					return;
				}
			}
		}
	}
	
	CLibraryView::OnChar( nChar, nRepCnt, nFlags );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTileView drag setup

#define MAX_DRAG_SIZE	256
#define MAX_DRAG_SIZE_2	128

void CLibraryTileView::StartDragging(CPoint& ptMouse)
{
	CImageList* pImage = CreateDragImage( ptMouse );
	if ( ! pImage ) return;

	ReleaseCapture();
	ClientToScreen( &ptMouse );
	DragObjects( pImage, ptMouse );
}

CImageList* CLibraryTileView::CreateDragImage(const CPoint& ptMouse)
{
	CRect rcClient, rcOne, rcAll( 32000, 32000, -32000, -32000 );

	GetClientRect( &rcClient );

	for ( POSITION pos = m_pSelTile.GetHeadPosition() ; pos ; )
	{
		CLibraryTileItem* pTile = (CLibraryTileItem*)m_pSelTile.GetNext( pos );
		GetItemRect( pTile, &rcOne );
		
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

	CDC* pBuffer = CoolInterface.GetBuffer( dcClient, m_szBlock );
	CRect rcBuffer( 0, 0, m_szBlock.cx, m_szBlock.cy );

	CFont* pOldFont = (CFont*)pBuffer->SelectObject( &CoolInterface.m_fntNormal );

	for ( POSITION pos = m_pSelTile.GetHeadPosition() ; pos ; )
	{
		CLibraryTileItem* pTile = (CLibraryTileItem*)m_pSelTile.GetNext( pos );
		GetItemRect( pTile, &rcOne );
		CRect rcDummy;
		
		if ( rcDummy.IntersectRect( &rcAll, &rcOne ) )
		{
			pBuffer->FillSolidRect( &rcBuffer, RGB( 250, 255, 250 ) );
			pTile->Paint( pBuffer, rcBuffer, &dcMem );
			dcDrag.BitBlt( rcOne.left - rcAll.left, rcOne.top - rcAll.top,
				m_szBlock.cx, m_szBlock.cy, pBuffer, 0, 0, SRCCOPY );
		}
	}

	pBuffer->SelectObject( pOldFont );

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
// CLibraryTileItem construction

CLibraryTileItem::CLibraryTileItem(CAlbumFolder* pFolder)
{
	m_pFolder	= pFolder;
	m_nCookie	= 0xFFFFFFFF;
	m_bSelected	= FALSE;
	Update();
}

CLibraryTileItem::~CLibraryTileItem()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTileItem operations

BOOL CLibraryTileItem::Update()
{
	if ( m_pFolder->m_nUpdateCookie == m_nCookie ) return FALSE;
	
	m_nCookie		= m_pFolder->m_nUpdateCookie;
	m_sTitle		= m_pFolder->m_sName;
	m_nIcon32		= m_pFolder->m_pSchema ? m_pFolder->m_pSchema->m_nIcon32 : -1;
	m_nIcon48		= m_pFolder->m_pSchema ? m_pFolder->m_pSchema->m_nIcon48 : -1;
	m_bCollection	= m_pFolder->m_bCollSHA1;
	
	CSchema* pSchema = m_pFolder->m_pSchema;
	
	if ( pSchema != NULL && m_pFolder->m_pXML != NULL )
	{
		m_sSubtitle1	= pSchema->m_sTileLine1;
		m_sSubtitle2	= pSchema->m_sTileLine2;
		
		pSchema->ResolveTokens( m_sSubtitle1, m_pFolder->m_pXML );
		pSchema->ResolveTokens( m_sSubtitle2, m_pFolder->m_pXML );
	}
	else
	{
		m_sSubtitle1.Empty();
		m_sSubtitle2.Empty();
	}
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTileItem paint

void CLibraryTileItem::Paint(CDC* pDC, const CRect& rcBlock, CDC* pMemDC)
{
	CRect rc( &rcBlock );
	
	UINT nStyle = 0;
	
	if ( m_bSelected ) nStyle |= ILD_SELECTED;
	if ( m_bCollection ) nStyle |= INDEXTOOVERLAYMASK(SHI_O_COLLECTION);
	
	if ( m_nIcon48 >= 0 )
	{
		ImageList_DrawEx( ShellIcons.GetHandle( 48 ), m_nIcon48, *pDC,
			rc.left + 5, rc.top + 4, 48, 48, CLR_NONE, CLR_DEFAULT, nStyle );
	}
	else if ( m_nIcon32 >= 0 )
	{
		ImageList_DrawEx( ShellIcons.GetHandle( 32 ), m_nIcon32, *pDC,
			rc.left + 5 + 8, rc.top + 4 + 8, 32, 32, CLR_NONE, CLR_DEFAULT, nStyle );
	}
	
	rc.left += 48 + 5;
	rc.DeflateRect( 10, 5 );
	
	if ( m_bSelected )
	{
		pDC->SetBkColor( CoolInterface.m_crHighlight );
		pDC->SetTextColor( CoolInterface.m_crHiText );
		pDC->SetBkMode( OPAQUE );
	}
	else
	{
		pDC->SetBkColor( CoolInterface.m_crWindow );
		pDC->SetTextColor( CoolInterface.m_crText );
		pDC->SetBkMode( TRANSPARENT );
	}
	
	int nX = rc.left + 2;
	int nY = ( rc.top + rc.bottom ) / 2;
	int nH = pDC->GetTextExtent( _T("Xy") ).cy;
	
	if ( m_sSubtitle1.GetLength() > 0 )
	{
		CRect rcUnion( nX, nY, nX, nY );
		
		if ( m_sSubtitle2.GetLength() > 0 )
		{
			nY -= ( nH * 3 ) / 2;
			if ( m_bCollection ) pDC->SelectObject( &CoolInterface.m_fntBold );
			DrawText( pDC, &rc, nX, nY, m_sTitle, &rcUnion );
			if ( m_bCollection ) pDC->SelectObject( &CoolInterface.m_fntNormal );
			if ( ! m_bSelected ) pDC->SetTextColor( CoolInterface.m_crDisabled );
			DrawText( pDC, &rc, nX, nY + nH, m_sSubtitle1, &rcUnion );
			DrawText( pDC, &rc, nX, nY + nH + nH, m_sSubtitle2, &rcUnion );
		}
		else
		{
			if ( m_bCollection ) pDC->SelectObject( &CoolInterface.m_fntBold );
			DrawText( pDC, &rc, nX, nY - nH, m_sTitle, &rcUnion );
			if ( m_bCollection ) pDC->SelectObject( &CoolInterface.m_fntNormal );
			if ( ! m_bSelected ) pDC->SetTextColor( CoolInterface.m_crDisabled );
			DrawText( pDC, &rc, nX, nY, m_sSubtitle1, &rcUnion );
		}
		
		pDC->FillSolidRect( &rcUnion, pDC->GetBkColor() );
	}
	else
	{
		nY -= nH / 2;
		if ( m_bCollection ) pDC->SelectObject( &CoolInterface.m_fntBold );
		DrawText( pDC, &rc, nX, nY, m_sTitle );
		if ( m_bCollection ) pDC->SelectObject( &CoolInterface.m_fntNormal );
	}
	
	pDC->SelectClipRgn( NULL );
}

void CLibraryTileItem::DrawText(CDC* pDC, const CRect* prcClip, int nX, int nY, LPCTSTR pszText, CRect* prcUnion)
{
	CSize sz = pDC->GetTextExtent( pszText, _tcslen( pszText ) );
	CRect rc( nX - 2, nY - 1, nX + sz.cx + 2, nY + sz.cy + 1 );
	
	rc.IntersectRect( &rc, prcClip );
	
	pDC->ExtTextOut( nX, nY, ETO_CLIPPED|ETO_OPAQUE, &rc, pszText, _tcslen( pszText ), NULL );
	pDC->ExcludeClipRect( rc.left, rc.top, rc.right, rc.bottom );
	
	if ( prcUnion != NULL ) prcUnion->UnionRect( prcUnion, &rc );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTileView command handlers

void CLibraryTileView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	Skin.TrackPopupMenu( m_pszToolBar, point, ID_LIBRARY_ALBUM_OPEN );
}

void CLibraryTileView::OnUpdateLibraryAlbumOpen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_nSelected == 1 );
}

void CLibraryTileView::OnLibraryAlbumOpen() 
{
	if ( m_pSelTile.GetCount() == 0 ) return;
	CLibraryTileItem* pItem = (CLibraryTileItem*)m_pSelTile.GetHead();
	GetFrame()->Display( pItem->m_pFolder );
}

void CLibraryTileView::OnUpdateLibraryAlbumDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_nSelected > 0 );
}

void CLibraryTileView::OnLibraryAlbumDelete() 
{
	if ( m_nSelected == 0 ) return;
	
	CString strFormat, strMessage;
	Skin.LoadString( strFormat, IDS_LIBRARY_FOLDER_DELETE );
	strMessage.Format( strFormat, m_nSelected );

	if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_OKCANCEL ) != IDOK ) return;

	{
		CQuickLock oLock( Library.m_pSection );

		for ( POSITION pos = m_pSelTile.GetHeadPosition() ; pos ; )
		{
			CLibraryTileItem* pTile = (CLibraryTileItem*)m_pSelTile.GetNext( pos );
			CAlbumFolder* pFolder = pTile->m_pFolder;
			if ( LibraryFolders.CheckAlbum( pFolder ) ) pFolder->Delete();
		}

	}
	PostUpdate();
}

void CLibraryTileView::OnUpdateLibraryAlbumProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_nSelected == 1 );
}

void CLibraryTileView::OnLibraryAlbumProperties() 
{
	if ( m_pSelTile.GetCount() == 0 ) return;
	CLibraryTileItem* pItem = (CLibraryTileItem*)m_pSelTile.GetHead();

	CAlbumFolder* pFolder = pItem->m_pFolder;
	CFolderPropertiesDlg dlg( NULL, pFolder );

	if ( dlg.DoModal() == IDOK ) GetFrame()->Display( pFolder );
}
