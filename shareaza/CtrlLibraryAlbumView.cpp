//
// CtrlLibraryAlbumView.cpp
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
#include "AlbumFolder.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "ShellIcons.h"
#include "CoolInterface.h"
#include "CtrlLibraryAlbumView.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryTip.h"
#include "DlgFilePropertiesSheet.h"
#include "Skin.h"
#include "XML.h"
#include "ShareazaDataSource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLibraryAlbumView, CLibraryFileView)

BEGIN_MESSAGE_MAP(CLibraryAlbumView, CLibraryFileView)
	//{{AFX_MSG_MAP(CLibraryAlbumView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

LPCTSTR		CLibraryAlbumView::m_pStaticStyle;
COLORREF	CLibraryAlbumView::m_crRows[2];


/////////////////////////////////////////////////////////////////////////////
// CLibraryAlbumView construction

CLibraryAlbumView::CLibraryAlbumView()
{
	m_nCommandID = ID_LIBRARY_VIEW_ALBUM;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryAlbumView create and destroy

BOOL CLibraryAlbumView::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	SelClear( FALSE );
	return CWnd::CreateEx( 0, NULL, _T("CLibraryAlbumView"), WS_CHILD | WS_VSCROLL |
		WS_TABSTOP | WS_GROUP, rect, pParentWnd, IDC_LIBRARY_VIEW );
}

int CLibraryAlbumView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CLibraryFileView::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	m_pList			= NULL;
	m_nCount		= 0;
	m_nBuffer		= 0;
	m_nScroll		= 0;
	
	m_nSelected		= 0;
	m_pFocus		= NULL;
	m_pFirst		= NULL;
	m_bDrag			= FALSE;
	
	CBitmap bmStar;
	bmStar.LoadBitmap( IDB_SMALL_STAR );
	m_pStars.Create( 12, 12, ILC_COLOR32|ILC_MASK, 6, 0 ) ||
	m_pStars.Create( 12, 12, ILC_COLOR24|ILC_MASK, 6, 0 ) ||
	m_pStars.Create( 12, 12, ILC_COLOR16|ILC_MASK, 6, 0 );
	m_pStars.Add( &bmStar, RGB( 0, 255, 0 ) );
	
	return 0;
}

void CLibraryAlbumView::OnDestroy() 
{
	Clear();
	CLibraryFileView::OnDestroy();
	m_pStars.DeleteImageList();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryAlbumView view operations

void CLibraryAlbumView::Update()
{
	CLibraryTreeItem* pFolders = GetFolderSelection();
	
	m_pStaticStyle = m_pStyle;
	m_pStyle = NULL;
	BOOL bGhostFolder	= FALSE;

	if ( pFolders != NULL && pFolders->m_pVirtual != NULL &&
		 pFolders->m_pSelNext == NULL )
	{
		CAlbumFolder* pFolder = pFolders->m_pVirtual;
		
		if ( CheckURI( pFolder->m_sSchemaURI, CSchema::uriMusicAlbum ) )
		{
			m_pStyle = CSchema::uriMusicAlbum;
		}
		else if ( CheckURI( pFolder->m_sSchemaURI, CSchema::uriMusicArtist ) )
		{
			m_pStyle = CSchema::uriMusicArtist;
		}
		else if ( CheckURI( pFolder->m_sSchemaURI, CSchema::uriGhostFolder ) )
		{
			bGhostFolder = TRUE;
		}
	}
	
	CSchemaPtr pSchema	= SchemaCache.Get( Settings.Library.FilterURI );
	DWORD nCookie		= GetFolderCookie();
	BOOL bChanged		= m_pStyle != m_pStaticStyle;
	
	if ( Settings.Library.ShowVirtual )	pSchema = NULL;

	CLibraryAlbumTrack** pList = m_pList + m_nCount - 1;
	
	for ( int nItem = m_nCount ; nItem ; nItem--, pList-- )
	{
		CLibraryAlbumTrack* pTrack	= *pList;
		CLibraryFile* pFile			= Library.LookupFile( pTrack->m_nIndex );
		
		if ( pFile != NULL && pFile->m_nSelectCookie == nCookie &&
			 ( pFile->IsAvailable() || bGhostFolder ) &&
			 ( ! pSchema || pSchema->Equals( pFile->m_pSchema ) ||
			 ( ! pFile->m_pMetadata && pSchema->FilterType( pFile->m_sName ) ) ) )
		{
			bChanged |= pTrack->Update( pFile );
			pFile->m_nListCookie = nCookie;
		}
		else
		{
			if ( pTrack == m_pFocus ) m_pFocus = NULL;
			if ( pTrack == m_pFirst ) m_pFirst = NULL;
			if ( pTrack->m_bSelected ) Select( pTrack, TRI_FALSE );
			delete pTrack;
			MoveMemory( pList, pList + 1, ( m_nCount - nItem ) * sizeof *pList );
			m_nCount--;
			bChanged = TRUE;
		}
	}
	
	if ( bChanged )
	{
		CRect rcClient;
		GetClientRect( &rcClient );
		int nMax	= m_nCount * m_szTrack.cy;
		m_nScroll	= max( 0, min( m_nScroll, nMax - rcClient.Height() + 1 ) );
	}
	
	for ( POSITION pos = LibraryMaps.GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = LibraryMaps.GetNextFile( pos );
		
		if ( pFile->m_nSelectCookie == nCookie &&
 			 pFile->m_nListCookie != nCookie &&
			 ( pFile->IsAvailable() || bGhostFolder ) &&
			 ( ! pSchema || pSchema->Equals( pFile->m_pSchema ) ||
			 ( ! pFile->m_pMetadata && pSchema->FilterType( pFile->m_sName ) ) ) )
		{
			CLibraryAlbumTrack* pTrack = new CLibraryAlbumTrack( pFile );
			
			if ( m_nCount == m_nBuffer )
			{
				m_nBuffer += 64;
				CLibraryAlbumTrack** pNewList = new CLibraryAlbumTrack*[ m_nBuffer ];
				if ( m_nCount ) CopyMemory( pNewList, m_pList, m_nCount * sizeof( CLibraryAlbumTrack* ) );
				delete [] m_pList;
				m_pList = pNewList;
			}
			
			m_pList[ m_nCount++ ] = pTrack;
			pFile->m_nListCookie = nCookie;
			bChanged = TRUE;
		}
	}
	
	if ( bChanged )
	{
		m_pStaticStyle = m_pStyle;
		qsort( m_pList, m_nCount, sizeof *m_pList, SortList );
		UpdateScroll();
	}
}

BOOL CLibraryAlbumView::Select(DWORD nObject)
{
	CRect rcClient, rcItem;
	
	CLibraryAlbumTrack** pList = m_pList + m_nCount - 1;
	
    int nItem = m_nCount;
	for ( ; nItem ; nItem--, pList-- )
	{
		CLibraryAlbumTrack* pTrack = *pList;
		if ( pTrack->m_nIndex == nObject ) break;
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

void CLibraryAlbumView::SelectAll()
{
	CLibraryAlbumTrack** pList = m_pList;
	for ( int nItem = 0 ; nItem < m_nCount ; nItem++, pList++ )
	{
		Select( *pList, TRI_TRUE );
	}

	Invalidate();
}

DWORD_PTR CLibraryAlbumView::HitTestIndex(const CPoint& point) const
{
	CLibraryAlbumTrack* pTrack = HitTest( point );
	return ( pTrack ) ? pTrack->m_nIndex : 0;
}

int CLibraryAlbumView::SortList(LPCVOID pA, LPCVOID pB)
{
	CLibraryAlbumTrack* ppA = *(CLibraryAlbumTrack**)pA;
	CLibraryAlbumTrack* ppB = *(CLibraryAlbumTrack**)pB;
	
	if ( m_pStaticStyle == CSchema::uriMusicAlbum )
	{
		if ( ppA->m_nTrack != ppB->m_nTrack )
		{
			return ( ppA->m_nTrack < ppB->m_nTrack ) ? -1 : 1;
		}
		else
		{
			return _tcsicoll( ppA->m_sTitle, ppB->m_sTitle );
		}
	}
	else if ( m_pStaticStyle == CSchema::uriMusicArtist )
	{
		int nCompare = _tcsicoll( ppA->m_sAlbum, ppB->m_sAlbum );
		
		if ( nCompare )
		{
			return nCompare;
		}
		else
		{
			return _tcsicoll( ppA->m_sTitle, ppB->m_sTitle );
		}
	}
	else
	{
		int nCompare = _tcsicoll( ppA->m_sArtist, ppB->m_sArtist );
		
		if ( nCompare )
		{
			return nCompare;
		}
		else if ( ( nCompare = _tcsicoll( ppA->m_sAlbum, ppB->m_sAlbum ) ) != 0 )
		{
			return nCompare;
		}
		else
		{
			return _tcsicoll( ppA->m_sTitle, ppB->m_sTitle );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryAlbumView item list management operations

void CLibraryAlbumView::Clear()
{
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
	
	SelClear();
	m_pSelTrack.RemoveAll();
}

int CLibraryAlbumView::GetTrackIndex(CLibraryAlbumTrack* pTrack) const
{
	CLibraryAlbumTrack** pList = m_pList;
	
	for ( int nItem = 0 ; nItem < m_nCount ; nItem++, pList++ )
	{
		if ( *pList == pTrack ) return nItem;
	}
	
	return -1;
}

BOOL CLibraryAlbumView::Select(CLibraryAlbumTrack* pTrack, TRISTATE bSelect)
{
	switch ( bSelect )
	{
	case TRI_UNKNOWN:
		pTrack->m_bSelected = ! pTrack->m_bSelected;
		break;
	case TRI_FALSE:
		if ( pTrack->m_bSelected == FALSE ) return FALSE;
		pTrack->m_bSelected = FALSE;
		break;
	case TRI_TRUE:
		if ( pTrack->m_bSelected == TRUE ) return FALSE;
		pTrack->m_bSelected = TRUE;
		break;
	}
	
	if ( pTrack->m_bSelected )
	{
		if ( m_pSelTrack.Find( pTrack ) == NULL )
		{
			m_nSelected++;
			SelAdd( pTrack->m_nIndex );
			m_pSelTrack.AddTail( pTrack );
		}
	}
	else
	{
		if ( POSITION pos = m_pSelTrack.Find( pTrack ) )
		{
			m_nSelected--;
			SelRemove( pTrack->m_nIndex );
			m_pSelTrack.RemoveAt( pos );
		}
	}
	
	return TRUE;
}

BOOL CLibraryAlbumView::DeselectAll(CLibraryAlbumTrack* pTrack)
{
	CLibraryAlbumTrack** pList = m_pList + m_nCount - 1;
	BOOL bChanged = FALSE;
	
	for ( int nItem = m_nCount ; nItem ; nItem--, pList-- )
	{
		if ( *pList != pTrack )
		{
			if ( (*pList)->m_bSelected ) bChanged = Select( *pList, TRI_FALSE );
		}
	}
	
	return bChanged;
}

BOOL CLibraryAlbumView::SelectTo(CLibraryAlbumTrack* pTrack)
{
	BOOL bChanged = FALSE;
	
	if ( pTrack )
	{
		m_pFocus = pTrack;
		
		int nFirst	= GetTrackIndex( m_pFirst );
		int nFocus	= GetTrackIndex( m_pFocus );
		
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
		
		if ( rcItem.top < rcClient.top + m_szTrack.cy )
		{
			ScrollBy( rcItem.top - rcClient.top - m_szTrack.cy );
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

void CLibraryAlbumView::SelectTo(int nDelta)
{
	if ( m_nCount == 0 ) return;
	
	int nFocus = GetTrackIndex( m_pFocus );
	
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
// CLibraryAlbumView message handlers

void CLibraryAlbumView::OnSize(UINT nType, int cx, int cy) 
{
	CLibraryFileView::OnSize( nType, cx, cy );
	
	m_szTrack.cx = cx;
	m_szTrack.cy = 22;
	m_nRows = cy / m_szTrack.cy - 1;

	UpdateScroll();
}

void CLibraryAlbumView::UpdateScroll()
{
	SCROLLINFO pInfo;
	CRect rc;
	
	GetClientRect( &rc );
	
	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;
	pInfo.nMin		= 0;
	pInfo.nMax		= m_nCount * m_szTrack.cy;
	pInfo.nPage		= rc.Height() - m_szTrack.cy;
	pInfo.nPos		= m_nScroll = max( 0, min( m_nScroll, pInfo.nMax - (int)pInfo.nPage + 1 ) );
	
	SetScrollInfo( SB_VERT, &pInfo, TRUE );
	
	Invalidate();
}

void CLibraryAlbumView::OnVScroll(UINT nSBCode, UINT /*nPos*/, CScrollBar* /*pScrollBar*/) 
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
		ScrollBy( m_szTrack.cy );
		break;
	case SB_LINEUP:
		ScrollBy( -m_szTrack.cy );
		break;
	case SB_PAGEDOWN:
		ScrollBy( rc.Height() );
		break;
	case SB_PAGEUP:
		ScrollBy( -rc.Height() );
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		{
			SCROLLINFO pScroll = { sizeof(SCROLLINFO), SIF_TRACKPOS };
			GetScrollInfo( SB_VERT, &pScroll );
			ScrollTo( pScroll.nTrackPos );
		}
		break;
	case SB_TOP:
		ScrollTo( 0 );
		break;
	}
}

BOOL CLibraryAlbumView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if ( CLibraryView::OnMouseWheel( nFlags, zDelta, pt ) )
		return TRUE;

	ScrollBy( zDelta * -m_szTrack.cy / WHEEL_DELTA * 5 );
	return TRUE;
}

void CLibraryAlbumView::ScrollBy(int nDelta)
{
	ScrollTo( max( 0, m_nScroll + nDelta ) );
}

void CLibraryAlbumView::ScrollTo(int nPosition)
{
	if ( nPosition == m_nScroll ) return;
	m_nScroll = nPosition;
	
	UpdateScroll();
	RedrawWindow( NULL, NULL, RDW_INVALIDATE );
}

void CLibraryAlbumView::OnPaint() 
{
	CPaintDC dc( this );
	
	CRect rcBuffer( 0, 0, m_szTrack.cx, m_szTrack.cy );
	CDC* pBuffer = CoolInterface.GetBuffer( dc, m_szTrack );
	if ( Settings.General.LanguageRTL ) pBuffer->SetTextAlign( TA_RTLREADING );
	
	CFont* pOldFont = (CFont*)pBuffer->SelectObject( &CoolInterface.m_fntNormal );
	pBuffer->SetBkMode( OPAQUE );
	pBuffer->SetBkColor( CoolInterface.m_crWindow );
	pBuffer->SetTextColor( CoolInterface.m_crText );
	
	m_crRows[0] = CCoolInterface::CalculateColour( CoolInterface.m_crWindow, Skin.m_crSchemaRow[0], 128 );
	m_crRows[1] = CCoolInterface::CalculateColour( CoolInterface.m_crWindow, Skin.m_crSchemaRow[1], 128 );
	
	CRect rcClient, rcTrack;
	GetClientRect( &rcClient );
	
	rcTrack = rcBuffer;
	rcTrack.OffsetRect( rcClient.left, 0 );
	
	CLibraryAlbumTrack** pList = m_pList;

	pBuffer->SelectObject( &CoolInterface.m_fntBold );
	CRect rcLine(rcTrack);
	rcLine.left += 22;
	rcLine.right -= 78;
	pBuffer->FillSolidRect( &rcBuffer, CoolInterface.m_crWindow );
	if ( m_pStyle == CSchema::uriMusicAlbum )
	{
		// Track, Title, Length, Bitrate
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine,  0,   5, IDS_LIBRARY_ALBUM_TRACK, TRUE );
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine,  5,  84, IDS_LIBRARY_ALBUM_TITLE );
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine, 84,  92, IDS_LIBRARY_ALBUM_LENGTH, TRUE );
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine, 92, 100, IDS_LIBRARY_ALBUM_BITRATE, TRUE );
		
		rcLine.left = rcLine.right;
		rcLine.right += 78;
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine, 0, 100, IDS_LIBRARY_ALBUM_RATING );
	}
	else if ( m_pStyle == CSchema::uriMusicArtist )
	{
		// Album, Title, Length, Bitrate
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine,  0,  30, IDS_LIBRARY_ALBUM_ALBUM );
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine, 30,  84, IDS_LIBRARY_ALBUM_TITLE );
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine, 84,  92, IDS_LIBRARY_ALBUM_LENGTH, TRUE );
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine, 92, 100, IDS_LIBRARY_ALBUM_BITRATE, TRUE );

		rcLine.left = rcLine.right;
		rcLine.right += 78;
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine, 0, 100, IDS_LIBRARY_ALBUM_RATING );
	}
	else
	{
		// Artist, Album, Title, Length, Bitrate
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine,  0,  25, IDS_LIBRARY_ALBUM_ARTIST );
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine, 25,  50, IDS_LIBRARY_ALBUM_ALBUM );
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine, 50,  84, IDS_LIBRARY_ALBUM_TITLE );
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine, 84,  92, IDS_LIBRARY_ALBUM_LENGTH, TRUE );
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine, 92, 100, IDS_LIBRARY_ALBUM_BITRATE, TRUE );

		rcLine.left = rcLine.right;
		rcLine.right += 78;
		CLibraryAlbumTrack::PaintText( pBuffer, rcLine, 0, 100, IDS_LIBRARY_ALBUM_RATING );
	}

	pBuffer->SelectObject( &CoolInterface.m_fntNormal );

	dc.BitBlt( rcTrack.left, rcTrack.top, rcBuffer.right, rcBuffer.bottom,
		pBuffer, 0, 0, SRCCOPY );
	dc.ExcludeClipRect( &rcTrack );
	rcTrack.OffsetRect( 0, rcClient.top - m_nScroll );
	rcTrack.OffsetRect( 0, m_szTrack.cy );

	for ( int nItem = 0 ; nItem < m_nCount && rcTrack.top < rcClient.bottom ; nItem++, pList++ )
	{
		CLibraryAlbumTrack* pTrack = *pList;
		
		if ( rcTrack.bottom >= rcClient.top && dc.RectVisible( &rcTrack ) )
		{
			pBuffer->FillSolidRect( &rcBuffer, CoolInterface.m_crWindow );
			pTrack->Paint( this, pBuffer, rcBuffer, nItem );
			dc.BitBlt( rcTrack.left, rcTrack.top, rcBuffer.right, rcBuffer.bottom,
				pBuffer, 0, 0, SRCCOPY );
			dc.ExcludeClipRect( &rcTrack );
		}
		
		rcTrack.OffsetRect( 0, m_szTrack.cy );
	}
	
	pBuffer->SelectObject( pOldFont );
	dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );
}

CLibraryAlbumTrack* CLibraryAlbumView::HitTest(const CPoint& point, CRect* pRect) const
{
	CRect rcClient, rcTrack( 0, 0, m_szTrack.cx, m_szTrack.cy * 2 );
	GetClientRect( &rcClient );
	rcTrack.OffsetRect( rcClient.left, rcClient.top - m_nScroll );
	
	CLibraryAlbumTrack** pList = m_pList;
	
	for ( int nItem = m_nCount ; nItem && rcTrack.top < rcClient.bottom ; nItem--, pList++ )
	{
		CLibraryAlbumTrack* pTrack = *pList;

		if ( rcTrack.PtInRect( point ) )
		{
			if ( pRect ) *pRect = rcTrack;
			return pTrack;
		}
		
		rcTrack.OffsetRect( 0, m_szTrack.cy );
	}
	
	return NULL;
}

BOOL CLibraryAlbumView::GetItemRect(CLibraryAlbumTrack* pTrack, CRect* pRect)
{
	CRect rcClient, rcTrack( 0, 0, m_szTrack.cx, m_szTrack.cy );
	GetClientRect( &rcClient );
	rcClient.top += m_szTrack.cy;
	rcTrack.OffsetRect( rcClient.left, rcClient.top - m_nScroll );
	
	CLibraryAlbumTrack** pList = m_pList;
	
	for ( int nItem = m_nCount ; nItem ; nItem--, pList++ )
	{
		if ( *pList == pTrack )
		{
			*pRect = rcTrack;
			return TRUE;
		}

		rcTrack.OffsetRect( 0, m_szTrack.cy );
	}
	
	return FALSE;
}

void CLibraryAlbumView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();
	
	CRect rcTrack;
	CLibraryAlbumTrack* pHit = HitTest( point, &rcTrack );
	
	if ( pHit != NULL && pHit->HitTestRating( rcTrack, point ) )
	{
		pHit->LockRating();
		m_pRating = NULL;
		Invalidate();
		return;
	}
	
	if ( SelectTo( pHit ) )
	{
		Invalidate();
		CLibraryFileView::CheckDynamicBar();
	}
	
	if ( pHit && ( nFlags & MK_RBUTTON ) == 0 )
	{
		m_bDrag = TRUE;
		m_ptDrag = point;
	}
	
	CLibraryFileView::OnLButtonDown( nFlags, point );
}

void CLibraryAlbumView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if ( m_bDrag && ( nFlags & MK_LBUTTON ) )
	{
		CLibraryFileView::OnMouseMove( nFlags, point );

		CSize szDiff = point - m_ptDrag;
		
		if ( abs( szDiff.cx ) > 5 || abs( szDiff.cy ) > 5 )
		{
			m_bDrag = FALSE;
			StartDragging( point );
		}

		CLibraryFileView::OnMouseMove( nFlags, point );

		return;
	}
	else
		m_bDrag = FALSE;
	
	// CLibraryFileView::OnMouseMove( nFlags, point );	Overridden below!
	CLibraryView::OnMouseMove( nFlags, point );
	
	CLibraryTipCtrl* pTip = ((CLibraryFrame*)GetOwner())->GetToolTip();
	CRect rcTrack;
	
	if ( CLibraryAlbumTrack* pTrack = HitTest( point, &rcTrack ) )
	{
		pTip->Show( pTrack->m_nIndex );
		
		if ( pTrack->HitTestRating( rcTrack, point ) )
		{
			m_pRating = pTrack;
			Invalidate();
			SetCursor( theApp.LoadCursor( IDC_HAND ) );
		}
		else if ( m_pRating != NULL )
		{
			m_pRating = NULL;
			Invalidate();
		}
	}
	else
	{
		pTip->Hide();
		
		if ( m_pRating != NULL )
		{
			m_pRating = NULL;
			Invalidate();
		}
	}
}

void CLibraryAlbumView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bDrag = FALSE;
	
	if ( ( nFlags & (MK_SHIFT|MK_CONTROL) ) == 0 && m_pFocus && m_pFocus->m_bSelected )
	{
		if ( DeselectAll( m_pFocus ) ) Invalidate();
	}

	CLibraryFileView::OnLButtonUp( nFlags, point );
}

void CLibraryAlbumView::OnLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/) 
{
	SendMessage( WM_COMMAND, ID_LIBRARY_LAUNCH );
}

void CLibraryAlbumView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	OnLButtonDown( nFlags, point );
	CLibraryFileView::OnRButtonDown( nFlags, point );
}

void CLibraryAlbumView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	BOOL bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0x8000;
	BOOL bControl = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) == 0x8000;

	switch ( nChar )
	{
	case VK_LEFT:
	case VK_UP:
		SelectTo( -1 );
		break;
	case VK_DOWN:
	case VK_RIGHT:
		SelectTo( 1 );
		break;
	case VK_PRIOR:
		SelectTo( -m_nRows );
		break;
	case VK_NEXT:
		SelectTo( m_nRows );
		break;
	case VK_HOME:
		SelectTo( -m_nCount );
		break;
	case VK_END:
		SelectTo( m_nCount );
		break;
	default:
		if ( ! bShift && ! bControl && _istalnum( TCHAR( nChar ) ) )
		{
			CLibraryAlbumTrack* pStart	= m_pFocus;
			
			for ( int nLoop = 0 ; nLoop < 2 ; nLoop++ )
			{
				CLibraryAlbumTrack** pChild = m_pList;
				
				for ( int nCount = m_nCount ; nCount ; nCount--, pChild++ )
				{
					if ( pStart != NULL )
					{
						if ( pStart == *pChild ) pStart = NULL;
					}
					else
					{
						LPCTSTR psz = NULL;
						
						if ( m_pStyle == CSchema::uriMusicAlbum )
							psz = (*pChild)->m_sTitle;
						else if ( m_pStyle == CSchema::uriMusicArtist )
							psz = (*pChild)->m_sAlbum;
						else
							psz = (*pChild)->m_sArtist;
						
						if ( psz && *psz && toupper( *psz ) == (int)nChar )
						{
							if ( SelectTo( *pChild ) ) Invalidate();
							return;
						}
					}
				}
			}
		}
	}
	
	CLibraryFileView::OnKeyDown( nChar, nRepCnt, nFlags );
}

UINT CLibraryAlbumView::OnGetDlgCode()
{
	return DLGC_WANTARROWS;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryAlbumView drag setup

HBITMAP CLibraryAlbumView::CreateDragImage(const CPoint& ptMouse, CPoint& ptMiddle)
{
	CRect rcClient, rcOne, rcAll( 32000, 32000, -32000, -32000 );
	
	GetClientRect( &rcClient );
	
	for ( POSITION pos = m_pSelTrack.GetHeadPosition() ; pos ; )
	{
		CLibraryAlbumTrack* pTrack = m_pSelTrack.GetNext( pos );
		GetItemRect( pTrack, &rcOne );
		
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
	CBitmap bmDrag;
	CDC dcDrag;
	
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
	
	CDC* pBuffer = CoolInterface.GetBuffer( dcClient, m_szTrack );
	CRect rcBuffer( 0, 0, m_szTrack.cx, m_szTrack.cy );
	
	CFont* pOldFont = (CFont*)pBuffer->SelectObject( &CoolInterface.m_fntNormal );
	
	for ( POSITION pos = m_pSelTrack.GetHeadPosition() ; pos ; )
	{
		CLibraryAlbumTrack* pTrack = m_pSelTrack.GetNext( pos );
		GetItemRect( pTrack, &rcOne );
		CRect rcDummy;
		
		if ( rcDummy.IntersectRect( &rcAll, &rcOne ) )
		{
			pBuffer->FillSolidRect( &rcBuffer, DRAG_COLOR_KEY );
			pTrack->Paint( this, pBuffer, rcBuffer, -1 );
			dcDrag.BitBlt( rcOne.left - rcAll.left, rcOne.top - rcAll.top,
				m_szTrack.cx, m_szTrack.cy, pBuffer, 0, 0, SRCCOPY );
		}
	}
	
	pBuffer->SelectObject( pOldFont );
	dcDrag.SelectObject( pOldDrag );
	dcDrag.DeleteDC();

	return (HBITMAP) bmDrag.Detach();
}


/////////////////////////////////////////////////////////////////////////////
// CLibraryAlbumTrack construction

CLibraryAlbumTrack::CLibraryAlbumTrack(CLibraryFile* pFile)
{
	m_nIndex	= pFile->m_nIndex;
	m_nCookie	= pFile->m_nUpdateCookie - 1;
	m_bSelected	= FALSE;
	
	Update( pFile );
}

CLibraryAlbumTrack::~CLibraryAlbumTrack()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryAlbumTrack operations

BOOL CLibraryAlbumTrack::Update(CLibraryFile* pFile)
{
	BOOL bShared = pFile->IsShared();
	
	if ( m_nCookie == pFile->m_nUpdateCookie && m_bShared == bShared ) return FALSE;
	
	m_nCookie	= pFile->m_nUpdateCookie;
	m_bShared	= bShared;
	m_nShell	= ShellIcons.Get( pFile->GetPath(), 16 );
	m_nRating	= pFile->m_nRating;
	m_bComments	= pFile->m_sComments.GetLength() > 0;
	m_nTrack	= 0;
	m_sTrack.Empty();
	m_sTitle.Empty();
	m_sArtist.Empty();
	m_sAlbum.Empty();
	m_nLength	= 0;
	m_sLength.Empty();
	m_nBitrate	= 0;
	m_sBitrate.Empty();
	
	if ( pFile->m_pMetadata && pFile->IsSchemaURI( CSchema::uriAudio ) )
	{
		CString str = pFile->m_pMetadata->GetAttributeValue( _T("track") );
		LPCTSTR psz = str;

		while ( *psz == '0' ) psz++;
		if ( *psz ) _stscanf( psz, _T("%i"), &m_nTrack );
		
		m_sTitle	= pFile->m_pMetadata->GetAttributeValue( _T("title") );
		m_sArtist	= pFile->m_pMetadata->GetAttributeValue( _T("artist") );
		m_sAlbum	= pFile->m_pMetadata->GetAttributeValue( _T("album") );
		
		_stscanf( pFile->m_pMetadata->GetAttributeValue( _T("seconds") ), _T("%i"), &m_nLength );
		_stscanf( pFile->m_pMetadata->GetAttributeValue( _T("bitrate") ), _T("%i"), &m_nBitrate );
	}
	
	int nDash = pFile->m_sName.Find( '-' );
	
	if ( nDash > 0 )
	{
		if ( m_sArtist.IsEmpty() ) m_sArtist = pFile->m_sName.Left( nDash );
		nDash = pFile->m_sName.ReverseFind( '-' );
		if ( m_sTitle.IsEmpty() )
		{
			m_sTitle = pFile->m_sName.Mid( nDash + 1 );
			nDash = m_sTitle.ReverseFind( '.' );
			if ( nDash >= 0 ) m_sTitle = m_sTitle.Left( nDash );
		}
	}
	else if ( m_sTitle.IsEmpty() )
	{
		m_sTitle = pFile->m_sName;
		nDash = m_sTitle.ReverseFind( '.' );
		if ( nDash >= 0 ) m_sTitle = m_sTitle.Left( nDash );
	}
	
	m_sTitle.TrimLeft();
	m_sTitle.TrimRight();
	m_sArtist.TrimLeft();
	m_sArtist.TrimRight();
	m_sAlbum.TrimLeft();
	m_sAlbum.TrimRight();
	
	if ( m_nTrack > 0 )
	{
		m_sTrack.Format( _T("%i"), m_nTrack );
	}
	
	if ( m_nLength > 0 )
	{
		m_sLength.Format( _T("%i:%.2i"), m_nLength / 60, m_nLength % 60 );
	}
	
	if ( m_nBitrate > 0 )
	{
		m_sBitrate.Format( _T("%ik"), m_nBitrate );
	}
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryAlbumTrack paint

void CLibraryAlbumTrack::Paint(CLibraryAlbumView* pView, CDC* pDC, const CRect& rcTrack, int nCount)
{
	COLORREF crBack1 = CLibraryAlbumView::m_crRows[ nCount & 1 ];
	COLORREF crBack2 = m_bSelected ? CoolInterface.m_crHighlight : crBack1;
	
	CRect rcLine( &rcTrack );
	rcLine.DeflateRect( 1, 1 );
	rcLine.left ++; rcLine.right --;
	
	CRect rcTemp( rcLine.left, rcLine.top, rcLine.left + 22, rcLine.bottom );
	rcLine.left += 22;
	
	pDC->SetBkColor( m_bSelected ? crBack2 : crBack1 );
	
	ShellIcons.Draw( pDC, m_nShell, 16, rcTemp.left + 3,
		( rcTemp.top + rcTemp.bottom ) / 2 - 8, CLR_NONE, m_bSelected );
	
	pDC->SetTextColor( m_bSelected ? CoolInterface.m_crHiText : CoolInterface.m_crText );
	
	rcTemp.SetRect( rcLine.right - 78, rcLine.top, rcLine.right, rcLine.bottom );
	rcLine.right -= 78;
	
	CPoint ptStar( rcTemp.left + 3, ( rcTemp.top + rcTemp.bottom ) / 2 - 6 );
	PaintText( pDC, rcTemp, 0, 100, NULL );
	
	if ( pView->m_pRating == this && m_nSetRating < 7 )
	{
		for ( int nRating = 2 ; nRating <= 6 ; nRating++ )
		{
			ImageList_DrawEx( pView->m_pStars, m_nSetRating >= nRating ? 2 : 1,
				*pDC, ptStar.x, ptStar.y, 12, 12, crBack2, crBack2,
				m_nSetRating >= nRating ? ILD_NORMAL : ILD_BLEND50 );
			ptStar.x += 12;
		}
	}
	else
	{
		for ( int nRating = 2 ; nRating <= 6 ; nRating++ )
		{
			ImageList_DrawEx( pView->m_pStars, m_nRating >= nRating ? 0 : 1,
				*pDC, ptStar.x, ptStar.y, 12, 12, crBack2, crBack2,
				m_nRating >= nRating ? ILD_NORMAL : ILD_BLEND50 );
			ptStar.x += 12;
		}
	}
	
	if ( pView->m_pRating == this && m_nSetRating == 7 )
	{
		ImageList_DrawEx( pView->m_pStars, 5,
			*pDC, ptStar.x, ptStar.y, 12, 12, crBack2, crBack2, ILD_NORMAL );
	}
	else
	{
		ImageList_DrawEx( pView->m_pStars, m_bComments ? 3 : 4,
			*pDC, ptStar.x, ptStar.y, 12, 12, crBack2, crBack2,
			m_bComments ? ILD_NORMAL : ILD_BLEND50 );
	}
	
	if ( pView->m_pStyle == CSchema::uriMusicAlbum )
	{
		// Track, Title, Length, Bitrate
		PaintText( pDC, rcLine, 0, 5, &m_sTrack, TRUE );
		PaintText( pDC, rcLine, 5, 84, &m_sTitle );
		PaintText( pDC, rcLine, 84, 92, &m_sLength, TRUE );
		PaintText( pDC, rcLine, 92, 100, &m_sBitrate, TRUE );
	}
	else if ( pView->m_pStyle == CSchema::uriMusicArtist )
	{
		// Album, Title, Length, Bitrate
		PaintText( pDC, rcLine, 0, 30, &m_sAlbum );
		PaintText( pDC, rcLine, 30, 84, &m_sTitle );
		PaintText( pDC, rcLine, 84, 92, &m_sLength, TRUE );
		PaintText( pDC, rcLine, 92, 100, &m_sBitrate, TRUE );
	}
	else
	{
		// Artist, Album, Title, Length, Bitrate
		PaintText( pDC, rcLine, 0, 25, &m_sArtist );
		PaintText( pDC, rcLine, 25, 50, &m_sAlbum );
		PaintText( pDC, rcLine, 50, 84, &m_sTitle );
		PaintText( pDC, rcLine, 84, 92, &m_sLength, TRUE );
		PaintText( pDC, rcLine, 92, 100, &m_sBitrate, TRUE );
	}
}

void CLibraryAlbumTrack::PaintText(CDC* pDC, const CRect& rcTrack, int nFrom, int nTo, const CString* pstr, BOOL bCenter)
{
	CRect rcText;
	
	rcText.left		= rcTrack.left + rcTrack.Width() * nFrom / 100 + 1;
	rcText.right	= rcTrack.left + rcTrack.Width() * nTo / 100 - 1;
	rcText.top		= rcTrack.top;
	rcText.bottom	= rcTrack.bottom;
	
	COLORREF crOld = pDC->GetPixel( rcText.left, rcText.top );
	
	if ( pstr != NULL )
	{
		if ( bCenter || pstr->GetLength() )
		{
			CSize szText = pDC->GetTextExtent( *pstr );
			int nText = pstr->GetLength();
			
			if ( szText.cx + 8 > rcText.Width() )
			{
				while ( nText > 0 )
				{
					nText--;
					szText = pDC->GetTextExtent( *pstr, nText );
					szText.cx += 8;
					if ( szText.cx + 8 <= rcText.Width() ) break;
				}
				
				CString str = pstr->Left( nText ) + _T('\x2026');
				
				pDC->ExtTextOut( bCenter ? ( ( rcText.left + rcText.right ) / 2 - szText.cx / 2 ) : ( rcText.left + 4 ),
					( rcText.top + rcText.bottom ) / 2 - szText.cy / 2 - 1,
					ETO_CLIPPED|ETO_OPAQUE, &rcText, str, NULL );
			}
			else
			{
				pDC->ExtTextOut( bCenter ? ( ( rcText.left + rcText.right ) / 2 - szText.cx / 2 ) : ( rcText.left + 4 ),
					( rcText.top + rcText.bottom ) / 2 - szText.cy / 2 - 1,
					ETO_CLIPPED|ETO_OPAQUE, &rcText, *pstr, nText, NULL );
			}
		}
		else
		{
			CSize szText = pDC->GetTextExtent( _T("?") );
			
			pDC->ExtTextOut( ( rcText.left + 4 ),
				( rcText.top + rcText.bottom ) / 2 - szText.cy / 2 - 1,
				ETO_CLIPPED|ETO_OPAQUE, &rcText, _T("?"), 1, NULL );
		}
	}
	else
	{
		pDC->ExtTextOut( rcText.left, rcText.top, ETO_OPAQUE, &rcText, NULL, 0, NULL );
	}

	pDC->SetPixel( rcText.left, rcText.top, crOld );
	pDC->SetPixel( rcText.right - 1, rcText.top, crOld );
	pDC->SetPixel( rcText.left, rcText.bottom - 1, crOld );
	pDC->SetPixel( rcText.right - 1, rcText.bottom - 1, crOld );
}

void CLibraryAlbumTrack::PaintText(CDC* pDC, const CRect& rcTrack, int nFrom, int nTo, int nID, BOOL bCenter)
{
	CString str;
	LoadString( str, nID );
	PaintText( pDC, rcTrack, nFrom, nTo, &str, bCenter );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryAlbumTrack mouse rating

BOOL CLibraryAlbumTrack::HitTestRating(const CRect& rcBlock, const CPoint& point)
{
	if ( point.x < rcBlock.right - 79 ) return FALSE;
	
	if ( point.x >= rcBlock.right - 3 - 12 )
	{
		m_nSetRating = 7;
		return TRUE;
	}
	
	int nPos = rcBlock.right - 72;
	
	for ( m_nSetRating = 1 ; m_nSetRating <= 6 ; m_nSetRating++ )
	{
		if ( point.x < nPos ) break;
		nPos += 12;
	}
	
	m_nSetRating = min( m_nSetRating, 6 );
	
	return TRUE;
}

BOOL CLibraryAlbumTrack::LockRating()
{
	if ( m_nSetRating == 7 )
	{
		CFilePropertiesSheet dlg;
		dlg.Add( m_nIndex );
		return dlg.DoModal( 2 ) == IDOK;
	}
	else if ( m_nSetRating >= 0 && m_nSetRating <= 6 )
	{
		CQuickLock oLock( Library.m_pSection );
		CLibraryFile* pFile = Library.LookupFile( m_nIndex );
		if ( pFile == NULL ) return FALSE;
		
		pFile->m_nRating = m_nRating = ( m_nSetRating > 1 ? m_nSetRating : 0 );
		pFile->ModifyMetadata();
		
		Library.Update();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
