//
// CtrlLibraryThumbView.cpp
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
#include "SharedFile.h"
#include "SharedFolder.h"
#include "ImageServices.h"
#include "ThumbCache.h"
#include "ShellIcons.h"
#include "CoolInterface.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "CtrlLibraryThumbView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLibraryThumbView, CLibraryFileView)

BEGIN_MESSAGE_MAP(CLibraryThumbView, CLibraryFileView)
	//{{AFX_MSG_MAP(CLibraryThumbView)
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
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define THUMB_ICON	48


/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbView construction

CLibraryThumbView::CLibraryThumbView()
{
	m_nCommandID = ID_LIBRARY_VIEW_THUMBNAIL;
}

CLibraryThumbView::~CLibraryThumbView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbView create and destroy

BOOL CLibraryThumbView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= WS_VSCROLL;
	return CLibraryFileView::PreCreateWindow( cs );
}

int CLibraryThumbView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CLibraryFileView::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_hThread		= NULL;
	m_bThread		= FALSE;
	m_nInvalidate	= 0;
	m_bRush			= FALSE;

	m_szThumb		= CSize( Settings.Library.ThumbSize, Settings.Library.ThumbSize );
	m_szBlock.cx	= m_szThumb.cx + 32;
	m_szBlock.cy	= m_szThumb.cy + 44;
	m_nColumns		= 0;
	m_nRows			= 0;
	
	m_pList			= NULL;
	m_nCount		= 0;
	m_nBuffer		= 0;
	m_nScroll		= 0;
	m_nSelected		= 0;
	m_pFocus		= NULL;
	m_pFirst		= NULL;
	m_bDrag			= FALSE;

	SetTimer( 1, 500, NULL );

	return 0;
}

void CLibraryThumbView::OnDestroy() 
{
	KillTimer( 1 );
	Clear();
	CLibraryFileView::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbView view operations

void CLibraryThumbView::Update()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	CSchema* pSchema	= SchemaCache.Get( Settings.Library.FilterURI );
	DWORD nCookie		= GetFolderCookie();
	BOOL bChanged		= FALSE;
	
	if ( Settings.Library.ShowVirtual ) pSchema = NULL;
	
	CLibraryThumbItem** pList = m_pList + m_nCount - 1;
	
	for ( int nItem = m_nCount ; nItem ; nItem--, pList-- )
	{
		CLibraryThumbItem* pThumb	= *pList;
		CLibraryFile* pFile			= Library.LookupFile( pThumb->m_nIndex );
		
		if ( pFile != NULL && pFile->m_nSelectCookie == nCookie &&
			 ( ! pSchema || pSchema->Equals( pFile->m_pSchema ) ||
			 ( ! pFile->m_pMetadata && pSchema->FilterType( pFile->m_sName ) ) ) )
		{
			bChanged |= pThumb->Update( pFile );
			
			pFile->m_nListCookie = nCookie;
		}
		else
		{
			if ( pThumb->m_bSelected ) Select( pThumb, TS_FALSE );
			if ( pThumb == m_pFocus ) m_pFocus = NULL;
			if ( pThumb == m_pFirst ) m_pFirst = NULL;
			
			delete pThumb;
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

	for ( POSITION pos = LibraryMaps.GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = LibraryMaps.GetNextFile( pos );
		
		if ( pFile->m_nSelectCookie == nCookie &&
 			 pFile->m_nListCookie != nCookie &&
			 ( ! pSchema || pSchema->Equals( pFile->m_pSchema ) ||
			 ( ! pFile->m_pMetadata && pSchema->FilterType( pFile->m_sName ) ) ) )
		{
			CLibraryThumbItem* pThumb = new CLibraryThumbItem( pFile );
			
			if ( m_nCount == m_nBuffer )
			{
				m_nBuffer += 64;
				CLibraryThumbItem** pList = new CLibraryThumbItem*[ m_nBuffer ];
				if ( m_nCount ) CopyMemory( pList, m_pList, 4 * m_nCount );
				if ( m_pList ) delete [] m_pList;
				m_pList = pList;
			}
			
			m_pList[ m_nCount++ ] = pThumb;
			pFile->m_nListCookie = nCookie;
			bChanged = TRUE;
		}
	}
	
	if ( bChanged )
	{
		qsort( m_pList, m_nCount, 4, SortList );
		UpdateScroll();
		StartThread();
	}
}

BOOL CLibraryThumbView::Select(DWORD nObject)
{
	CRect rcClient, rcItem;
	
	CLibraryThumbItem** pList = m_pList + m_nCount - 1;

	for ( int nItem = m_nCount ; nItem ; nItem--, pList-- )
	{
		CLibraryThumbItem* pThumb = *pList;
		if ( pThumb->m_nIndex == nObject ) break;
	}

	if ( nItem == 0 ) return FALSE;

	m_pFocus = *pList;
	DeselectAll( m_pFocus );
	Select( m_pFocus );
	Invalidate();
	
	GetClientRect( &rcClient );
	GetItemRect( m_pFocus, &rcItem );
	
	if ( rcItem.top < rcClient.top )
	{
		ScrollBy( rcItem.top - rcClient.top );
	}
	else if ( rcItem.bottom > rcClient.bottom )
	{
		ScrollBy( rcItem.bottom - rcClient.bottom );
	}
	
	return TRUE;
}

DWORD CLibraryThumbView::HitTestIndex(const CPoint& point) const
{
	CLibraryThumbItem* pThumb = HitTest( point );
	return ( pThumb ) ? pThumb->m_nIndex : 0;
}

int CLibraryThumbView::SortList(LPCVOID pA, LPCVOID pB)
{
	CLibraryThumbItem* ppA = *(CLibraryThumbItem**)pA;
	CLibraryThumbItem* ppB = *(CLibraryThumbItem**)pB;
	return _tcsicoll( ppA->m_sText, ppB->m_sText );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbView item list management operations

void CLibraryThumbView::Clear()
{
	StopThread();

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
	m_pSelThumb.RemoveAll();
	SelClear();
}

int CLibraryThumbView::GetThumbIndex(CLibraryThumbItem* pThumb) const
{
	CLibraryThumbItem** pList = m_pList;

	for ( int nItem = 0 ; nItem < m_nCount ; nItem++, pList++ )
	{
		if ( *pList == pThumb ) return nItem;
	}

	return -1;
}

BOOL CLibraryThumbView::Select(CLibraryThumbItem* pThumb, TRISTATE bSelect)
{
	switch ( bSelect )
	{
	case TS_UNKNOWN:
		pThumb->m_bSelected = ! pThumb->m_bSelected;
		break;
	case TS_FALSE:
		if ( pThumb->m_bSelected == FALSE ) return FALSE;
		pThumb->m_bSelected = FALSE;
		break;
	case TS_TRUE:
		if ( pThumb->m_bSelected == TRUE ) return FALSE;
		pThumb->m_bSelected = TRUE;
		break;
	}

	if ( pThumb->m_bSelected )
	{
		m_nSelected++;
		m_pSelThumb.AddTail( pThumb );
		SelAdd( pThumb->m_nIndex );
	}
	else
	{
		m_nSelected--;
		if ( POSITION pos = m_pSelThumb.Find( pThumb ) )
		{
			m_pSelThumb.RemoveAt( pos );
			SelRemove( pThumb->m_nIndex );
		}
	}

	return TRUE;
}

BOOL CLibraryThumbView::DeselectAll(CLibraryThumbItem* pThumb)
{
	CLibraryThumbItem** pList = m_pList + m_nCount - 1;
	BOOL bChanged = FALSE;

	for ( int nItem = m_nCount ; nItem ; nItem--, pList-- )
	{
		if ( *pList != pThumb )
		{
			if ( (*pList)->m_bSelected ) bChanged = Select( *pList, TS_FALSE );
		}
	}

	return bChanged;
}

BOOL CLibraryThumbView::SelectTo(CLibraryThumbItem* pThumb)
{
	BOOL bChanged = FALSE;

	if ( pThumb )
	{
		m_pFocus = pThumb;

		int nFirst	= GetThumbIndex( m_pFirst );
		int nFocus	= GetThumbIndex( m_pFocus );
		
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

		CRect rcClient, rcItem;

		GetClientRect( &rcClient );
		GetItemRect( m_pFocus, &rcItem );

		if ( rcItem.top < rcClient.top )
		{
			ScrollBy( rcItem.top - rcClient.top );
		}
		else if ( rcItem.bottom > rcClient.bottom )
		{
			ScrollBy( rcItem.bottom - rcClient.bottom );
		}
	}
	else if (	( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 &&
				( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) == 0 )
	{
		bChanged = DeselectAll();
	}

	if ( m_nSelected == 0 ) m_pFirst = NULL;

	return bChanged;
}

void CLibraryThumbView::SelectTo(int nDelta)
{
	if ( m_nCount == 0 ) return;

	int nFocus = GetThumbIndex( m_pFocus );

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

/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbView message handlers

void CLibraryThumbView::OnSize(UINT nType, int cx, int cy) 
{
	CLibraryFileView::OnSize( nType, cx, cy );
	
	m_nColumns	= cx / m_szBlock.cx;
	m_nRows		= cy / m_szBlock.cy + 1;

	UpdateScroll();
}

void CLibraryThumbView::UpdateScroll()
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

void CLibraryThumbView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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

BOOL CLibraryThumbView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	ScrollBy( zDelta * -m_szBlock.cy / WHEEL_DELTA / 2 );
	return TRUE;
}

void CLibraryThumbView::ScrollBy(int nDelta)
{
	ScrollTo( max( 0, m_nScroll + nDelta ) );
}

void CLibraryThumbView::ScrollTo(int nPosition)
{
	if ( nPosition == m_nScroll ) return;
	m_nScroll = nPosition;

	UpdateScroll();
	RedrawWindow( NULL, NULL, RDW_INVALIDATE );
}

void CLibraryThumbView::OnTimer(UINT nIDEvent) 
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_nInvalidate && ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) == 0 )
	{
		Invalidate();
		m_nInvalidate = 0;
	}
}

void CLibraryThumbView::OnPaint() 
{
	CSingleLock pLock( &m_pSection, TRUE );
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

	CLibraryThumbItem** pList = m_pList;
	m_bRush = FALSE;

	for ( int nItem = m_nCount ; nItem && pt.y < rcClient.bottom ; nItem--, pList++ )
	{
		CLibraryThumbItem* pThumb = *pList;

		CRect rcBlock( pt.x, pt.y, pt.x + m_szBlock.cx, pt.y + m_szBlock.cy );

		if ( rcBlock.bottom >= rcClient.top && dc.RectVisible( &rcBlock ) )
		{
			pBuffer->FillSolidRect( &rcBuffer, CoolInterface.m_crWindow );
			pThumb->Paint( pBuffer, rcBuffer, m_szThumb, &dcMem );
			dc.BitBlt( rcBlock.left, rcBlock.top, m_szBlock.cx, m_szBlock.cy,
				pBuffer, 0, 0, SRCCOPY );
			dc.ExcludeClipRect( &rcBlock );
			if ( pThumb->m_nThumb == CLibraryThumbItem::thumbWaiting ) m_bRush = TRUE;
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

CLibraryThumbItem* CLibraryThumbView::HitTest(const CPoint& point) const
{
	CRect rcClient;
	GetClientRect( &rcClient );
	
	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	CLibraryThumbItem** pList = m_pList;

	for ( int nItem = m_nCount ; nItem && pt.y < rcClient.bottom ; nItem--, pList++ )
	{
		CLibraryThumbItem* pThumb = *pList;

		CRect rcBlock( pt.x, pt.y, pt.x + m_szBlock.cx, pt.y + m_szBlock.cy );

		if ( rcBlock.PtInRect( point ) ) return pThumb;

		pt.x += m_szBlock.cx;

		if ( pt.x + m_szBlock.cx > rcClient.right )
		{
			pt.x = rcClient.left;
			pt.y += m_szBlock.cy;
		}
	}

	return NULL;
}

BOOL CLibraryThumbView::GetItemRect(CLibraryThumbItem* pThumb, CRect* pRect)
{
	CRect rcClient;
	GetClientRect( &rcClient );
	
	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	CLibraryThumbItem** pList = m_pList;

	for ( int nItem = m_nCount ; nItem ; nItem--, pList++ )
	{
		CRect rcBlock( pt.x, pt.y, pt.x + m_szBlock.cx, pt.y + m_szBlock.cy );

		if ( pThumb == *pList )
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

void CLibraryThumbView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CLibraryThumbItem* pHit = HitTest( point );

	if ( SelectTo( pHit ) ) Invalidate();

	SetFocus();
	SetCapture();

	if ( pHit && ( nFlags & MK_RBUTTON ) == 0 )
	{
		m_bDrag = TRUE;
		m_ptDrag = point;
	}

	CLibraryFileView::OnLButtonDown( nFlags, point );
}

void CLibraryThumbView::OnMouseMove(UINT nFlags, CPoint point) 
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

	CLibraryFileView::OnMouseMove( nFlags, point );
}

void CLibraryThumbView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseCapture();
	m_bDrag = FALSE;

	if ( ( nFlags & (MK_SHIFT|MK_CONTROL) ) == 0 && m_pFocus && m_pFocus->m_bSelected )
	{
		if ( DeselectAll( m_pFocus ) ) Invalidate();
	}
}

void CLibraryThumbView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	SendMessage( WM_COMMAND, ID_LIBRARY_LAUNCH );
}

void CLibraryThumbView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	OnLButtonDown( nFlags, point );
	CLibraryFileView::OnRButtonDown( nFlags, point );
}

void CLibraryThumbView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
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
		SelectTo( m_nRows * m_nColumns );
		break;
	case VK_HOME:
		SelectTo( -m_nCount );
		break;
	case VK_END:
		SelectTo( m_nCount );
		break;
	}

	CLibraryFileView::OnKeyDown( nChar, nRepCnt, nFlags );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbView drag setup

#define MAX_DRAG_SIZE	256
#define MAX_DRAG_SIZE_2	128

void CLibraryThumbView::StartDragging(CPoint& ptMouse)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	CImageList* pImage = CreateDragImage( ptMouse );
	if ( ! pImage ) return;

	ReleaseCapture();
	ClientToScreen( &ptMouse );
	DragObjects( pImage, ptMouse );
}

CImageList* CLibraryThumbView::CreateDragImage(const CPoint& ptMouse)
{
	CRect rcClient, rcOne, rcAll( 32000, 32000, -32000, -32000 );

	GetClientRect( &rcClient );

	for ( POSITION pos = m_pSelThumb.GetHeadPosition() ; pos ; )
	{
		CLibraryThumbItem* pThumb = (CLibraryThumbItem*)m_pSelThumb.GetNext( pos );
		GetItemRect( pThumb, &rcOne );
		
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

	dcDrag.FillSolidRect( 0, 0, rcAll.Width(), rcAll.Height(), RGB( 0, 255, 0 ) );
	
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

	for ( pos = m_pSelThumb.GetHeadPosition() ; pos ; )
	{
		CLibraryThumbItem* pThumb = (CLibraryThumbItem*)m_pSelThumb.GetNext( pos );
		GetItemRect( pThumb, &rcOne );
		CRect rcDummy;
		
		if ( rcDummy.IntersectRect( &rcAll, &rcOne ) )
		{
			pBuffer->FillSolidRect( &rcBuffer, RGB( 0, 255, 0 ) );
			pThumb->Paint( pBuffer, rcBuffer, m_szThumb, &dcMem );
			dcDrag.BitBlt( rcOne.left - rcAll.left, rcOne.top - rcAll.top,
				m_szBlock.cx, m_szBlock.cy, pBuffer, 0, 0, SRCCOPY );
		}
	}

	pBuffer->SelectObject( pOldFont );

	dcDrag.SelectObject( pOldDrag );

	dcDrag.DeleteDC();
	
	CImageList* pAll = new CImageList();
	pAll->Create( rcAll.Width(), rcAll.Height(), ILC_COLOR16|ILC_MASK, 1, 1 );
	pAll->Add( &bmDrag, RGB( 0, 255, 0 ) ); 

	bmDrag.DeleteObject();

	pAll->BeginDrag( 0, ptMouse - rcAll.TopLeft() );

	return pAll;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbView thread builder

void CLibraryThumbView::StartThread()
{
	if ( m_hThread != NULL && m_bThread ) return;
	
	CSingleLock pLock( &m_pSection, TRUE );

	CLibraryThumbItem** pList = m_pList;
	int nCount = 0;

	for ( int nItem = m_nCount ; nItem ; nItem--, pList++ )
	{
		if ( (*pList)->m_nThumb == CLibraryThumbItem::thumbWaiting ) nCount++;
	}

	if ( nCount == 0 ) return;
	
	m_bThread	= TRUE;
	CWinThread* pThread = AfxBeginThread( ThreadStart, this, THREAD_PRIORITY_IDLE );
	m_hThread	= pThread->m_hThread;
}

void CLibraryThumbView::StopThread()
{
	if ( m_hThread == NULL ) return;
	
	m_bThread = FALSE;
	
	for ( int nAttempt = 100 ; nAttempt > 0 ; nAttempt-- )
	{
		DWORD nCode;

		if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
		if ( nCode != STILL_ACTIVE ) break;
		Sleep( 100 );
	}
	
	if ( nAttempt == 0 )
	{
		TerminateThread( m_hThread, 0 );
		theApp.Message( MSG_DEBUG, _T("WARNING: Terminating CLibraryThumbView thread.") );
		Sleep( 100 );
	}
	
	m_hThread = NULL;
}

UINT CLibraryThumbView::ThreadStart(LPVOID pParam)
{
	CLibraryThumbView* pView = (CLibraryThumbView*)pParam;
	pView->OnRun();
	return 0;
}

void CLibraryThumbView::OnRun()
{
	CSingleLock pLock( &m_pSection );
	CImageServices pServices;
	CThumbCache pCache;
	
	while ( m_bThread )
	{
		CLibraryThumbItem* pThumb = NULL;
		DWORD nIndex = 0;
		CString strPath;
		BOOL bCache;
		
		pLock.Lock();
		
		CLibraryThumbItem** pList = m_pList;
		for ( int nItem = m_nCount ; nItem ; nItem--, pList++ )
		{
			if ( (*pList)->m_nThumb == CLibraryThumbItem::thumbWaiting )
			{
				if ( CLibraryFile* pFile = Library.LookupFile( (*pList)->m_nIndex, TRUE ) )
				{
					pThumb	= *pList;
					nIndex	= pFile->m_nIndex;
					strPath	= pFile->GetPath();
					bCache	= pFile->m_bCachedPreview;
					Library.Unlock();
					break;
				}
			}
		}
		
		pLock.Unlock();
		
		if ( pThumb == NULL ) break;
		pThumb = NULL;
		
		DWORD tNow = GetTickCount();
		
		CImageFile pFile( &pServices );
		BOOL bSuccess = FALSE;
		
		if ( pCache.Load( strPath, &m_szThumb, nIndex, &pFile ) )
		{
			bSuccess = TRUE;
		}
		else if ( pFile.LoadFromFile( strPath, FALSE, TRUE ) && pFile.EnsureRGB() )
		{
			int nSize = m_szThumb.cy * pFile.m_nWidth / pFile.m_nHeight;
			
			if ( ! m_bThread ) break;
			
			if ( nSize > m_szThumb.cx )
			{
				nSize = m_szThumb.cx * pFile.m_nHeight / pFile.m_nWidth;
				pFile.Resample( m_szThumb.cx, nSize );
			}
			else
			{
				pFile.Resample( nSize, m_szThumb.cy );
			}
			
			if ( ! m_bThread ) break;
			
			pCache.Store( strPath, &m_szThumb, nIndex, &pFile );
			
			bSuccess = TRUE;
		}
		
		pLock.Lock();
		
		pList = m_pList;
		for ( nItem = m_nCount ; nItem ; nItem--, pList++ )
		{
			if ( (*pList)->m_nIndex == nIndex )
			{
				pThumb = *pList;
				break;
			}
		}
		
		if ( pThumb )
		{
			if ( pThumb->m_bmThumb.m_hObject ) pThumb->m_bmThumb.DeleteObject();
			
			if ( bSuccess )
			{
				pThumb->m_bmThumb.Attach( pFile.CreateBitmap() );
				pThumb->m_szThumb.cx = pFile.m_nWidth;
				pThumb->m_szThumb.cy = pFile.m_nHeight;
				pThumb->m_nThumb = CLibraryThumbItem::thumbValid;
			}
			else
			{
				pThumb->m_nThumb = CLibraryThumbItem::thumbError;
			}
			
			m_nInvalidate++;
		}
		
		pLock.Unlock();
		
		if ( bSuccess && ! bCache )
		{
			if ( CLibraryFile* pFile = Library.LookupFile( nIndex, TRUE ) )
			{
				pFile->m_bCachedPreview = TRUE;
				Library.Unlock();
			}
		}
		
		if ( ! m_bRush )
		{
			DWORD tDelay = GetTickCount() - tNow;
			if ( tDelay > 400 ) tDelay = 400;
			if ( tDelay < 20 ) tDelay = 20;
			
			while ( tDelay && m_bThread )
			{
				DWORD tNow = min( tDelay, 50 );
				tDelay -= tNow;
				Sleep( tNow );
			}
		}
	}
	
	pServices.Cleanup();
	m_bThread = FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbItem construction

CLibraryThumbItem::CLibraryThumbItem(CLibraryFile* pFile)
{
	m_nIndex	= pFile->m_nIndex;
	m_nCookie	= pFile->m_nUpdateCookie;
	m_sText		= pFile->m_sName;
	m_bShared	= pFile->IsShared();
	m_bSelected	= FALSE;
	m_nThumb	= 0;
	m_nShell	= ShellIcons.Get( m_sText, THUMB_ICON );
}

CLibraryThumbItem::~CLibraryThumbItem()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbItem operations

BOOL CLibraryThumbItem::Update(CLibraryFile* pFile)
{
	BOOL bShared = pFile->IsShared();

	if ( m_nCookie == pFile->m_nUpdateCookie && m_bShared == bShared ) return FALSE;

	m_nCookie	= pFile->m_nUpdateCookie;
	m_sText		= pFile->m_sName;
	m_bShared	= bShared;
	
	m_nThumb	= thumbWaiting;
	if ( m_bmThumb.m_hObject ) m_bmThumb.DeleteObject();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbItem paint

void CLibraryThumbItem::Paint(CDC* pDC, const CRect& rcBlock, const CSize& szThumb, CDC* pMemDC)
{
	CRect rcThumb;

	rcThumb.left	= ( rcBlock.left + rcBlock.right ) / 2 - szThumb.cx / 2;
	rcThumb.right	= rcThumb.left + szThumb.cx;
	rcThumb.top		= rcBlock.top + 7;
	rcThumb.bottom	= rcThumb.top + szThumb.cy;

	CRect rcFrame( &rcThumb );

	if ( m_bSelected )
	{
		for ( int nBorder = 3 ; nBorder ; nBorder-- )
		{
			rcFrame.InflateRect( 1, 1 );
			pDC->Draw3dRect( &rcFrame, CoolInterface.m_crHighlight, CoolInterface.m_crHighlight );
		}
	}
	else
	{
		rcFrame.InflateRect( 1, 1 );
		pDC->Draw3dRect( &rcFrame, CoolInterface.m_crMargin, CoolInterface.m_crMargin );
	}
	
	if ( m_bmThumb.m_hObject != NULL )
	{
		pMemDC->SelectObject( &m_bmThumb );

		CPoint ptImage(	( rcThumb.left + rcThumb.right ) / 2 - m_szThumb.cx / 2,
						( rcThumb.top + rcThumb.bottom ) / 2 - m_szThumb.cy / 2 );

		pDC->BitBlt( ptImage.x, ptImage.y, m_szThumb.cx, m_szThumb.cy,
			pMemDC, 0, 0, SRCCOPY );
	}
	else
	{
		if ( m_nThumb == thumbWaiting )
			pDC->FillSolidRect( &rcThumb, RGB( 255, 255, 255 ) );
		else
			pDC->FillSolidRect( &rcThumb, CoolInterface.m_crBackNormal );

		ImageList_DrawEx( ShellIcons.GetHandle( THUMB_ICON ), m_nShell, pDC->GetSafeHdc(),
			( rcThumb.left + rcThumb.right ) / 2 - THUMB_ICON / 2,
			( rcThumb.top + rcThumb.bottom ) / 2 - THUMB_ICON / 2,
			THUMB_ICON, THUMB_ICON, CLR_NONE, CLR_NONE, ILD_NORMAL );
	}

	if ( m_bSelected )
	{
		pDC->SetBkColor( CoolInterface.m_crHighlight );
		pDC->SetTextColor( CoolInterface.m_crHiText );
	}
	else if ( ! m_bShared )
	{
		pDC->SetBkColor( CoolInterface.m_crWindow );
		pDC->SetTextColor( CoolInterface.m_crHighlight );
	}
	else
	{
		pDC->SetBkColor( CoolInterface.m_crWindow );
		pDC->SetTextColor( CoolInterface.m_crText );
	}

	rcThumb.top		= rcThumb.bottom + 4;
	rcThumb.bottom	= rcThumb.top + 27;
	rcThumb.left	-= 3;
	rcThumb.right	+= 3;

	CSize szText = pDC->GetTextExtent( m_sText );

	if ( szText.cx < rcThumb.Width() - 4 )
	{
		rcThumb.left = ( rcThumb.left + rcThumb.right ) / 2 - szText.cx / 2 - 2;
		rcThumb.right = rcThumb.left + szText.cx + 4;
		rcThumb.bottom = rcThumb.top + szText.cy + 2;
		pDC->FillSolidRect( &rcThumb, pDC->GetBkColor() );
		pDC->ExtTextOut( rcThumb.left + 2, rcThumb.top, ETO_CLIPPED|ETO_OPAQUE,
			&rcThumb, m_sText, NULL );
	}
	else
	{
		int nSaveX		= rcThumb.right;
		int nSaveY		= rcThumb.bottom;
		int nHeight		= pDC->DrawText( m_sText, &rcThumb, DT_CENTER|DT_WORDBREAK|DT_CALCRECT|DT_NOPREFIX );
		rcThumb.bottom	= min( nSaveY, rcThumb.top + nHeight + 2 );
		rcThumb.right	= nSaveX;
		pDC->FillSolidRect( &rcThumb, pDC->GetBkColor() );
		pDC->DrawText( m_sText, &rcThumb, DT_CENTER|DT_WORDBREAK|DT_NOPREFIX );
	}
}

