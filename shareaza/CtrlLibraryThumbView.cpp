//
// CtrlLibraryThumbView.cpp
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
#include "Library.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "ImageFile.h"
#include "ThumbCache.h"
#include "ShellIcons.h"
#include "CoolInterface.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "CtrlLibraryThumbView.h"
#include "ShareazaDataSource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLibraryThumbView, CLibraryFileView)

BEGIN_MESSAGE_MAP(CLibraryThumbView, CLibraryFileView)
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
	ON_WM_SETFOCUS()
	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()

#define CX	( (int)Settings.Library.ThumbSize + 32 )
#define CY	( (int)Settings.Library.ThumbSize + 44 )

/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbView construction

CLibraryThumbView::CLibraryThumbView()
{
	m_nCommandID = ID_LIBRARY_VIEW_THUMBNAIL;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbView create and destroy

BOOL CLibraryThumbView::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	SelClear( FALSE );
	return CWnd::CreateEx( 0, NULL, _T("CLibraryThumbView"), WS_CHILD | WS_VSCROLL |
		WS_TABSTOP | WS_GROUP, rect, pParentWnd, IDC_LIBRARY_VIEW );
}

int CLibraryThumbView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CLibraryFileView::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_nInvalidate	= 0;

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
	CSingleLock pLock( &Library.m_pSection, TRUE );

	CSchemaPtr pSchema	= SchemaCache.Get( Settings.Library.FilterURI );
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
			if ( pThumb->m_bSelected ) Select( pThumb, TRI_FALSE );
			if ( pThumb == m_pFocus ) m_pFocus = NULL;
			if ( pThumb == m_pFirst ) m_pFirst = NULL;

			delete pThumb;
			MoveMemory( pList, pList + 1, ( m_nCount - nItem ) * sizeof *pList );
			m_nCount--;

			bChanged = TRUE;
		}
	}

	if ( bChanged )
	{
		CRect rcClient;
		GetClientRect( &rcClient );
		int nMax	= ( ( m_nCount + m_nColumns - 1 ) / m_nColumns ) * CY;
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
				CLibraryThumbItem** pNewList = new CLibraryThumbItem*[ m_nBuffer ];
				if ( m_pList )
				{
					if ( m_nCount ) CopyMemory( pNewList, m_pList, m_nCount * sizeof( CLibraryThumbItem* ) );
					delete [] m_pList;
				}
				m_pList = pNewList;
			}

			m_pList[ m_nCount++ ] = pThumb;
			pFile->m_nListCookie = nCookie;
			bChanged = TRUE;
		}
	}

	if ( bChanged )
	{
		qsort( m_pList, m_nCount, sizeof *m_pList, SortList );
		UpdateScroll();
		StartThread();
	}
}

BOOL CLibraryThumbView::Select(DWORD nObject)
{
	CRect rcClient, rcItem;

	CLibraryThumbItem** pList = m_pList + m_nCount - 1;

    int nItem = m_nCount;
	for ( ; nItem ; nItem--, pList-- )
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

void CLibraryThumbView::SelectAll()
{
	CLibraryThumbItem** pList = m_pList;
	for ( int nItem = 0 ; nItem < m_nCount; nItem++, pList++ )
	{
		Select( *pList, TRI_TRUE );
	}

	Invalidate();
}

DWORD_PTR CLibraryThumbView::HitTestIndex(const CPoint& point) const
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

	if ( m_pList )
	{
		for ( int nItem = 0 ; nItem < m_nCount ; nItem++ )
		{
			delete m_pList[ nItem ];
		}
		delete [] m_pList;
	}

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
	case TRI_UNKNOWN:
		pThumb->m_bSelected = ! pThumb->m_bSelected;
		break;
	case TRI_FALSE:
		if ( pThumb->m_bSelected == FALSE ) return FALSE;
		pThumb->m_bSelected = FALSE;
		break;
	case TRI_TRUE:
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
			if ( (*pList)->m_bSelected ) bChanged = Select( *pList, TRI_FALSE );
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
			bChanged = Select( m_pFocus, TRI_UNKNOWN );
		}
		else if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			bChanged = DeselectAll();

			if ( nFirst >= 0 && nFocus >= 0 )
			{
				if ( nFirst <= nFocus )
				{
					for ( ; nFirst <= nFocus ; nFirst++ ) Select( m_pList[ nFirst ], TRI_TRUE );
				}
				else
				{
					for ( ; nFocus <= nFirst ; nFocus++ ) Select( m_pList[ nFocus ], TRI_TRUE );
				}

				bChanged = TRUE;
			}
			else
			{
				bChanged |= Select( m_pFocus, TRI_TRUE );
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

	if ( SelectTo( m_pList[ nFocus ] ) )
	{
		Invalidate();
		CLibraryFileView::CheckDynamicBar();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbView message handlers

void CLibraryThumbView::OnSize(UINT nType, int cx, int cy)
{
	CLibraryFileView::OnSize( nType, cx, cy );

	m_nColumns	= cx / CX;
	m_nRows		= cy / CY + 1;

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
	pInfo.nMax		= ( ( m_nCount + m_nColumns - 1 ) / m_nColumns ) * CY;
	pInfo.nPage		= rc.Height();;
	pInfo.nPos		= m_nScroll = max( 0, min( m_nScroll, pInfo.nMax - (int)pInfo.nPage + 1 ) );

	SetScrollInfo( SB_VERT, &pInfo, TRUE );

	Invalidate();
}

void CLibraryThumbView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
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
	if ( CLibraryFileView::OnMouseWheel( nFlags, zDelta, pt ) )
		return TRUE;

	ScrollBy( zDelta * -CY / WHEEL_DELTA / 2 );
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

void CLibraryThumbView::OnTimer(UINT_PTR /*nIDEvent*/)
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( m_nInvalidate && ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) == 0 )
	{
		Invalidate();
		m_nInvalidate = 0;
	}
}

void CLibraryThumbView::OnPaint()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CPaintDC dc( this );

	CRect rcClient;
	GetClientRect( &rcClient );
	CPoint pt( rcClient.left, rcClient.top - m_nScroll );

	CLibraryThumbItem** pList = m_pList;

	for ( int nItem = m_nCount ; nItem && pt.y < rcClient.bottom ; nItem--, pList++ )
	{
		CLibraryThumbItem* pThumb = *pList;

		CRect rcBlock( pt.x, pt.y, pt.x + CX, pt.y + CY );

		if ( rcBlock.bottom >= rcClient.top && dc.RectVisible( &rcBlock ) )
		{
			pThumb->Paint( &dc, rcBlock );
		}

		pt.x += CX;

		if ( pt.x + CX > rcClient.right )
		{
			pt.x = rcClient.left;
			pt.y += CY;
		}
	}

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

		CRect rcBlock( pt.x, pt.y, pt.x + CX, pt.y + CY );

		if ( rcBlock.PtInRect( point ) ) return pThumb;

		pt.x += CX;

		if ( pt.x + CX > rcClient.right )
		{
			pt.x = rcClient.left;
			pt.y += CY;
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
		CRect rcBlock( pt.x, pt.y, pt.x + CX, pt.y + CY );

		if ( pThumb == *pList )
		{
			*pRect = rcBlock;
			return TRUE;
		}

		pt.x += CX;

		if ( pt.x + CX > rcClient.right )
		{
			pt.x = rcClient.left;
			pt.y += CY;
		}
	}

	return FALSE;
}

void CLibraryThumbView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CLibraryThumbItem* pHit = HitTest( point );

	if ( SelectTo( pHit ) )
	{
		Invalidate();
		CLibraryFileView::CheckDynamicBar();
	}

	SetFocus();

	if ( pHit && ( nFlags & MK_RBUTTON ) == 0 )
	{
		m_bDrag = TRUE;
		m_ptDrag = point;
	}

	CLibraryFileView::OnLButtonDown( nFlags, point );
}

void CLibraryThumbView::OnMouseMove(UINT nFlags, CPoint point)
{
	if ( m_bDrag && ( nFlags & MK_LBUTTON ) )
	{
		CSize szDiff = point - m_ptDrag;

		if ( abs( szDiff.cx ) > 5 || abs( szDiff.cy ) > 5 )
		{
			m_bDrag = FALSE;
			StartDragging( point );
		}
	}
	else
		m_bDrag = FALSE;

	CLibraryFileView::OnMouseMove( nFlags, point );
}

void CLibraryThumbView::OnLButtonUp(UINT nFlags, CPoint /*point*/)
{
	m_bDrag = FALSE;

	if ( ( nFlags & (MK_SHIFT|MK_CONTROL) ) == 0 && m_pFocus && m_pFocus->m_bSelected )
	{
		if ( DeselectAll( m_pFocus ) ) Invalidate();
	}
}

void CLibraryThumbView::OnLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/)
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

HBITMAP CLibraryThumbView::CreateDragImage(const CPoint& ptMouse, CPoint& ptMiddle)
{
	CRect rcClient, rcOne, rcAll( 32000, 32000, -32000, -32000 );

	GetClientRect( &rcClient );

	for ( POSITION pos = m_pSelThumb.GetHeadPosition() ; pos ; )
	{
		CLibraryThumbItem* pThumb = m_pSelThumb.GetNext( pos );
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

	dcDrag.FillSolidRect( 0, 0, rcAll.Width(), rcAll.Height(), DRAG_COLOR_KEY );

	CRgn pRgn;

	ptMiddle.SetPoint( ptMouse.x - rcAll.left, ptMouse.y - rcAll.top );
	if ( bClipped )
	{
		pRgn.CreateEllipticRgn(	ptMiddle.x - MAX_DRAG_SIZE_2, ptMiddle.y - MAX_DRAG_SIZE_2,
								ptMiddle.x + MAX_DRAG_SIZE_2, ptMiddle.y + MAX_DRAG_SIZE_2 );
		dcDrag.SelectClipRgn( &pRgn );
	}

	CDC* pBuffer = CoolInterface.GetBuffer( dcClient, CSize( CX, CY ) );
	CRect rcBuffer( 0, 0, CX, CY );

	CFont* pOldFont = (CFont*)pBuffer->SelectObject( &CoolInterface.m_fntNormal );

	for ( POSITION pos = m_pSelThumb.GetHeadPosition() ; pos ; )
	{
		CLibraryThumbItem* pThumb = m_pSelThumb.GetNext( pos );
		GetItemRect( pThumb, &rcOne );
		CRect rcDummy;

		if ( rcDummy.IntersectRect( &rcAll, &rcOne ) )
		{
			pBuffer->FillSolidRect( &rcBuffer, DRAG_COLOR_KEY );
			pThumb->Paint( pBuffer, rcBuffer );
			dcDrag.BitBlt( rcOne.left - rcAll.left, rcOne.top - rcAll.top,
				CX, CY, pBuffer, 0, 0, SRCCOPY );
		}
	}

	pBuffer->SelectObject( pOldFont );

	dcDrag.SelectObject( pOldDrag );

	dcDrag.DeleteDC();

	return (HBITMAP) bmDrag.Detach();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryThumbView thread builder

void CLibraryThumbView::StartThread()
{
	if ( IsThreadAlive() )
		return;

	CSingleLock pLock( &Library.m_pSection, TRUE );

	CLibraryThumbItem** pList = m_pList;
	int nCount = 0;

	for ( int nItem = m_nCount ; nItem ; nItem--, pList++ )
	{
		if ( (*pList)->m_nThumb == CLibraryThumbItem::thumbWaiting ) nCount++;
	}

	if ( nCount == 0 ) // all thumbnails extracted
		return;

	BeginThread( "CtrlLibraryThumbView" );
}

void CLibraryThumbView::StopThread()
{
	CloseThread();
}

void CLibraryThumbView::OnRun()
{
	CSingleLock oLock( &Library.m_pSection, FALSE );

	while ( IsThreadEnabled() )
	{
		// Get next file without thumbnail

		bool bWaiting = false;
		DWORD nIndex = 0;
		CString strPath;

		if ( ! oLock.Lock( 100 ) )
			// Library is busy
			continue;

		for ( int i = 0 ; i < m_nCount; ++i )
		{
			if ( m_pList[ i ]->m_nThumb == CLibraryThumbItem::thumbWaiting )
			{
				bWaiting = true;
				if ( CLibraryFile* pFile = Library.LookupFile( m_pList[ i ]->m_nIndex ) )
				{
					nIndex	= pFile->m_nIndex;
					strPath	= pFile->GetPath();
					break;
				}
			}
		}

		oLock.Unlock();

		if ( ! bWaiting )
			// Complete
			break;

		if ( ! nIndex )
		{
			// Too busy
			Sleep( 250 );
			continue;
		}

		// Load thumbnail from file and save it to cache

		CImageFile pFile;
		BOOL bSuccess = CThumbCache::Cache( strPath, &pFile );

		if ( ! oLock.Lock( 100 ) )
			// Library is busy
			continue;

		// Save thumbnail to item
		for ( int i = 0 ; i < m_nCount ; ++i )
		{
			if ( m_pList[ i ]->m_nIndex == nIndex )
			{
				if ( m_pList[ i ]->m_bmThumb.m_hObject )
					m_pList[ i ]->m_bmThumb.DeleteObject();

				if ( bSuccess )
				{
					m_pList[ i ]->m_bmThumb.Attach( pFile.CreateBitmap() );
					m_pList[ i ]->m_nThumb = CLibraryThumbItem::thumbValid;
				}
				else
					m_pList[ i ]->m_nThumb = CLibraryThumbItem::thumbError;

				m_nInvalidate++;
				break;
			}
		}

		oLock.Unlock();
	}
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
	m_nThumb	= thumbWaiting;
	m_nShell	= ShellIcons.Get( pFile->GetPath(), 48 );
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

void CLibraryThumbItem::Paint(CDC* pDC, const CRect& rcBlock)
{
	CRect rcThumb;
	rcThumb.left	= ( rcBlock.left + rcBlock.right - Settings.Library.ThumbSize ) / 2;
	rcThumb.right	= rcThumb.left + Settings.Library.ThumbSize;
	rcThumb.top		= rcBlock.top + 7;
	rcThumb.bottom	= rcThumb.top + Settings.Library.ThumbSize;

	// Draw thumbnail

	CoolInterface.DrawThumbnail( pDC, rcThumb, ( m_nThumb == thumbWaiting ),
		m_bSelected, m_bmThumb, m_nShell, -1 );

	// Draw label

	CFont* pOldFont = pDC->SelectObject( &CoolInterface.m_fntNormal );
	const UINT nStyle = DT_CENTER | DT_TOP | DT_WORDBREAK | DT_EDITCONTROL |
		DT_NOPREFIX | DT_END_ELLIPSIS;
	CRect rcText( rcBlock.left + 4, rcThumb.bottom + 4,
		rcBlock.right - 4, rcBlock.bottom );
	if ( m_bSelected )
	{
		pDC->SetBkColor( CoolInterface.m_crHighlight );
		pDC->SetTextColor( CoolInterface.m_crHiText );

		CRect rcCalc( rcText );
		int nHeight = pDC->DrawText( m_sText, &rcCalc, nStyle | DT_CALCRECT );
		if ( nHeight > rcText.Height() )
			rcText.bottom = rcText.top + nHeight;
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
	pDC->FillSolidRect( &rcText, pDC->GetBkColor() );
	pDC->DrawText( m_sText, &rcText, nStyle );
	pDC->ExcludeClipRect( &rcText );
	pDC->SelectObject( pOldFont );

	// Draw background

	pDC->FillSolidRect( &rcBlock, CoolInterface.m_crWindow );
	pDC->ExcludeClipRect( &rcBlock );
}

UINT CLibraryThumbView::OnGetDlgCode()
{
	return DLGC_WANTARROWS;
}
