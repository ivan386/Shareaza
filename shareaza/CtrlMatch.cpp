//
// CtrlMatch.cpp
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
#include "CtrlMatch.h"
#include "MatchObjects.h"
#include "QueryHit.h"
#include "ShellIcons.h"
#include "CoolInterface.h"
#include "VendorCache.h"
#include "Network.h"
#include "Schema.h"
#include "Skin.h"
#include "WndBaseMatch.h"
#include "Flags.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CMatchCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEWHEEL()
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	ON_NOTIFY(HDN_ITEMCHANGEDW, IDC_MATCH_HEADER, OnChangeHeader)
	ON_NOTIFY(HDN_ITEMCHANGEDA, IDC_MATCH_HEADER, OnChangeHeader)
	ON_NOTIFY(HDN_ENDDRAG, IDC_MATCH_HEADER, OnChangeHeader)
	ON_NOTIFY(HDN_ITEMCLICKW, IDC_MATCH_HEADER, OnClickHeader)
	ON_NOTIFY(HDN_ITEMCLICKA, IDC_MATCH_HEADER, OnClickHeader)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()

#define HEADER_HEIGHT	20
#define ITEM_HEIGHT		17


/////////////////////////////////////////////////////////////////////////////
// CMatchCtrl construction

CMatchCtrl::CMatchCtrl()
	: m_pMatches			( NULL )
	, m_sType				( _T("Search") )
	, m_pSchema				( NULL )
	, m_nTopIndex			( 0 )
	, m_nHitIndex			( 0 )
	, m_nBottomIndex		( 0xFFFFFFFF )
	, m_nFocus				( 0xFFFFFFFF )
	, m_nPageCount			( 1 )
	, m_nCurrentWidth		( 0 )
	, m_nCacheItems			( 0 )
	, m_nTrailWidth			( 0 )
	, m_bSearchLink			( FALSE )
	, m_bTips				( TRUE )
	, m_nScrollWheelLines	( 3 )
	, m_pLastSelectedFile	( NULL )
	, m_pLastSelectedHit	( NULL )

{
	// Try to get the number of lines to scroll when the mouse wheel is rotated
	SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &m_nScrollWheelLines, 0 );
}

CMatchCtrl::~CMatchCtrl()
{
}

void CMatchCtrl::OnSkinChange()
{
	ASSERT_VALID( this );
	SetFont( &CoolInterface.m_fntNormal );

	ASSERT_VALID( &m_wndHeader );
	m_wndHeader.SetFont( &CoolInterface.m_fntNormal );
}

/////////////////////////////////////////////////////////////////////////////
// CMatchCtrl system

BOOL CMatchCtrl::Create(CMatchList* pMatches, CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	m_pMatches = pMatches;
	return CWnd::CreateEx( 0, NULL, _T("CMatchCtrl"), WS_CHILD|WS_TABSTOP|WS_VSCROLL|
		WS_VISIBLE, rect, pParentWnd, IDC_MATCHES, NULL );
}

int CMatchCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CRect rc;
	
	if ( ! m_wndHeader.Create( WS_CHILD|WS_VISIBLE|HDS_BUTTONS|HDS_DRAGDROP|HDS_HOTTRACK|HDS_FULLDRAG,
		rc, this, IDC_MATCH_HEADER ) ) return -1;
	
	if ( ! m_wndTip.Create( this ) ) return -1;
	
	EnableToolTips( TRUE );
	
	InsertColumn( MATCH_COL_NAME, _T("File"), HDF_LEFT, 200 );
	InsertColumn( MATCH_COL_TYPE, _T("Extension"), HDF_CENTER, 80 );
	InsertColumn( MATCH_COL_SIZE, _T("Size"), HDF_CENTER, 60 );
	InsertColumn( MATCH_COL_RATING, _T("Rating"), HDF_CENTER, 12*5 );
	InsertColumn( MATCH_COL_STATUS, _T("Status"), HDF_CENTER, 16*3 );
	InsertColumn( MATCH_COL_COUNT, _T("Host/Count"), HDF_CENTER, 120 );
	InsertColumn( MATCH_COL_SPEED, _T("Speed"), HDF_CENTER, 60 );
	InsertColumn( MATCH_COL_CLIENT, _T("Client"), HDF_CENTER, 80 );
	InsertColumn( MATCH_COL_TIME, _T("Time"), HDF_CENTER, 120 );
	InsertColumn( MATCH_COL_COUNTRY, _T("Country"), HDF_LEFT, 60 );

	m_pStars.Create( 12, 12, ILC_COLOR32|ILC_MASK, 7, 0 ) ||
	m_pStars.Create( 12, 12, ILC_COLOR24|ILC_MASK, 7, 0 ) ||
	m_pStars.Create( 12, 12, ILC_COLOR16|ILC_MASK, 7, 0 );
	m_pStars.Add( CBitmap::FromHandle( Skin.LoadBitmap( IDB_SMALL_STAR ) ), RGB( 0, 255, 0 ) );
	
	LoadColumnState();
	
	UpdateScroll();
	
	OnSkinChange();
	
	return 0;
}

void CMatchCtrl::OnDestroy() 
{
	SaveColumnState();

	m_pStars.DeleteImageList();
	m_wndTip.DestroyWindow();

	CWnd::OnDestroy();
}

void CMatchCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize( nType, cx, cy );
	
	m_nCurrentWidth = cx;

	m_nPageCount = ( cy - HEADER_HEIGHT ) / ITEM_HEIGHT;
	if ( m_nPageCount < 1 ) m_nPageCount = 1;
	m_nBottomIndex = 0xFFFFFFFF;

	UpdateScroll();
}

/////////////////////////////////////////////////////////////////////////////
// CMatchCtrl update

void CMatchCtrl::Update()
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );

	if ( ! m_pMatches->m_bUpdated ) return;

	m_nCacheItems = m_pMatches->m_nItems;
	ScrollTo( GetScrollPos( SB_VERT ) );

	if ( m_pMatches->m_nUpdateMax >= m_nTopIndex &&
		 m_pMatches->m_nUpdateMin <= m_nBottomIndex )
	{
		Invalidate();
	}
	
	m_pMatches->ClearUpdated();
}

void CMatchCtrl::DestructiveUpdate()
{
	m_wndTip.Hide();
}

void CMatchCtrl::SelectSchema(CSchemaPtr pSchema, CSchemaMemberList* pColumns)
{
	SaveColumnState();
	
	m_pSchema = pSchema;
	m_pColumns.RemoveAll();
	
	while ( m_wndHeader.DeleteItem( MATCH_COL_MAX ) );
	int nColumn = MATCH_COL_MAX;
	
	if ( pSchema && pColumns )
	{
		m_pColumns.AddTail( pColumns );
		
		for ( POSITION pos = m_pColumns.GetHeadPosition() ; pos ; nColumn++ )
		{
			CSchemaMemberPtr pMember = m_pColumns.GetNext( pos );
			if ( !pMember->m_bHidden )
				InsertColumn( nColumn, pMember->m_sTitle, pMember->m_nColumnAlign, pMember->m_nColumnWidth );
			else
				nColumn--;
		}
	}
	
	m_pMatches->SelectSchema( pSchema, pColumns );
	
	LoadColumnState();
	
	Update();
}

void CMatchCtrl::SetBrowseMode()
{
	SaveColumnState();
	m_sType = _T("Browse");
	HDITEM pZero = { HDI_WIDTH, 0 };
	m_wndHeader.SetItem( MATCH_COL_STATUS, &pZero );
	m_wndHeader.SetItem( MATCH_COL_COUNT, &pZero );
	m_wndHeader.SetItem( MATCH_COL_SPEED, &pZero );
	m_wndHeader.SetItem( MATCH_COL_CLIENT, &pZero );
	m_wndHeader.SetItem( MATCH_COL_TIME, &pZero );
	m_wndHeader.SetItem( MATCH_COL_COUNTRY, &pZero );
	LoadColumnState();
}

BOOL CMatchCtrl::HitTestHeader(const CPoint& point)
{
	CRect rc;
	m_wndHeader.GetWindowRect( &rc );
	return rc.PtInRect( point );
}

void CMatchCtrl::SetSortColumn(int nColumn, BOOL bDirection)
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );
	CWaitCursor pCursor;
	
	m_pMatches->SetSortColumn( nColumn, bDirection );
	Update();
	
	if ( m_bmSortAsc.m_hObject == NULL )
	{
		m_bmSortAsc.LoadMappedBitmap( IDB_SORT_ASC );
		m_bmSortDesc.LoadMappedBitmap( IDB_SORT_DESC );
	}
	
	HDITEM pColumn = { HDI_BITMAP|HDI_FORMAT };
	
	for ( int nCol = 0 ; m_wndHeader.GetItem( nCol, &pColumn ) ; nCol++ )
	{
		if ( nCol == nColumn )
		{
			pColumn.fmt |= HDF_BITMAP|HDF_BITMAP_ON_RIGHT;
			pColumn.hbm = (HBITMAP)( bDirection ? m_bmSortAsc.GetSafeHandle() : m_bmSortDesc.GetSafeHandle() );
		}
		else
		{
			pColumn.fmt &= ~HDF_BITMAP;
			pColumn.hbm = NULL;
		}
		
		m_wndHeader.SetItem( nCol, &pColumn );
	}
}

void CMatchCtrl::SetMessage(UINT nMessageID, BOOL bLink)
{
	SetMessage( LoadString( nMessageID ), bLink );
}

void CMatchCtrl::SetMessage(LPCTSTR pszMessage, BOOL bLink)
{
	if ( m_bSearchLink == bLink && m_sMessage == pszMessage )
		return;
	
	m_bSearchLink = bLink;
	m_sMessage = pszMessage;
	
	if ( m_nCacheItems == 0 )
		Invalidate();
}

void CMatchCtrl::EnableTips(BOOL bTips)
{
	m_bTips = bTips;
}

/////////////////////////////////////////////////////////////////////////////
// CMatchCtrl column utilities

void CMatchCtrl::InsertColumn(int nColumn, LPCTSTR pszCaption, int nFormat, int nWidth)
{
	HDITEM pItem = { HDI_TEXT|HDI_FORMAT|HDI_WIDTH };
	
	pItem.pszText		= (LPTSTR)pszCaption;
	pItem.cchTextMax	= static_cast< int >( _tcslen( pszCaption ) );
	pItem.fmt			= nFormat;
	pItem.cxy			= nWidth;
	
	m_wndHeader.InsertItem( nColumn, &pItem );
}

void CMatchCtrl::SaveColumnState()
{
	HDITEM pItem = { HDI_WIDTH|HDI_ORDER };
	
	CString strOrdering, strWidths, strItem;
	
	for ( int nColumns = 0 ; m_wndHeader.GetItem( nColumns, &pItem ) ; nColumns++ )
	{
		m_wndHeader.GetItem( nColumns, &pItem );
		
		strItem.Format( _T("%.2x"), pItem.iOrder );
		strOrdering += strItem;
		
		strItem.Format( _T("%.4x"), pItem.cxy );
		strWidths += strItem;
	}
	
	int nSort = m_pMatches->m_nSortColumn >= 0 ? 
		( m_pMatches->m_nSortColumn + 1 ) * m_pMatches->m_bSortDir : 0;
	
	LPCTSTR pszName = _T("Null");
	if ( m_pSchema ) pszName = m_pSchema->m_sSingular;
	
	strItem.Format( _T("CMatchCtrl.%s.%s.Ordering"), m_sType, pszName );
	theApp.WriteProfileString( _T("ListStates"), strItem, strOrdering );
	strItem.Format( _T("CMatchCtrl.%s.%s.Widths"), m_sType, pszName );
	theApp.WriteProfileString( _T("ListStates"), strItem, strWidths );
	strItem.Format( _T("CMatchCtrl.%s.%s.Sort"), m_sType, pszName );
	theApp.WriteProfileInt( _T("ListStates"), strItem, nSort );
}

BOOL CMatchCtrl::LoadColumnState()
{
	CString strOrdering, strWidths, strItem;
	
	LPCTSTR pszName = _T("Null");
	if ( m_pSchema ) pszName = m_pSchema->m_sSingular;
	
	strItem.Format( _T("CMatchCtrl.%s.%s.Ordering"), m_sType, pszName );
	strOrdering = theApp.GetProfileString( _T("ListStates"), strItem, _T("") );
	strItem.Format( _T("CMatchCtrl.%s.%s.Widths"), m_sType, pszName );
	strWidths = theApp.GetProfileString( _T("ListStates"), strItem, _T("") );
	strItem.Format( _T("CMatchCtrl.%s.%s.Sort"), m_sType, pszName );
	int nSort = theApp.GetProfileInt( _T("ListStates"), strItem, - MATCH_COL_COUNT - 1 );
	
	HDITEM pItem = { HDI_WIDTH|HDI_ORDER };
	
	if ( _tcsncmp( strWidths, _T("0000"), 4 ) == 0 &&
		 _tcsncmp( strOrdering, _T("00"), 2 ) == 0 )
	{
		strWidths = strWidths.Mid( 4 );
		strOrdering = strOrdering.Mid( 2 );
	}
	
	for ( int nColumns = 0 ; m_wndHeader.GetItem( nColumns, &pItem ) ; nColumns++ )
	{
		if ( strWidths.GetLength() < 4 || strOrdering.GetLength() < 2 ||
			_stscanf( strWidths.Left( 4 ), _T("%x"), &pItem.cxy ) != 1 ||
			_stscanf( strOrdering.Left( 2 ), _T("%x"), &pItem.iOrder ) != 1 )
			return FALSE;
		
		strWidths = strWidths.Mid( 4 );
		strOrdering = strOrdering.Mid( 2 );
		
		m_wndHeader.SetItem( nColumns, &pItem );
	}
	
	SetSortColumn( abs( nSort ) - 1, nSort < 0 );
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMatchCtrl scrolling

void CMatchCtrl::UpdateScroll(DWORD nScroll)
{
	SCROLLINFO pInfo;
	
	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;
	pInfo.nMin		= 0;
	pInfo.nMax		= m_pMatches->m_nItems - 1;
	pInfo.nPage		= m_nPageCount;
	pInfo.nPos		= nScroll < 0xFFFFFFFF ? nScroll : GetScrollPos( SB_VERT );
	pInfo.nPos		= max( 0, min( pInfo.nPos, pInfo.nMax - (int)pInfo.nPage + 1 ) );
	
	SetScrollInfo( SB_VERT, &pInfo, TRUE );
	
	int nColumnWidth = 0;
	
	for ( int nColumn = m_wndHeader.GetItemCount() - 1 ; nColumn >= 0 ; nColumn-- )
	{
		CRect rcCol;
		Header_GetItemRect( m_wndHeader.GetSafeHwnd(), nColumn, &rcCol );
		nColumnWidth = max( nColumnWidth, int(rcCol.right) );
	}
	
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;
	pInfo.nMin		= 0;
	pInfo.nMax		= nColumnWidth - 1;
	pInfo.nPage		= m_nCurrentWidth;
	pInfo.nPos		= GetScrollPos( SB_HORZ );
	pInfo.nPos		= max( 0, min( pInfo.nPos, pInfo.nMax - (int)pInfo.nPage + 1 ) );
	
	SetScrollInfo( SB_HORZ, &pInfo, TRUE );
	
	CRect rc;
	m_wndHeader.GetWindowRect( &rc );
	ScreenToClient( &rc );
	
	if ( rc.left != -pInfo.nPos || rc.Width() != max( m_nCurrentWidth, pInfo.nMax ) )
	{
		m_wndHeader.SetWindowPos( NULL, -pInfo.nPos, 0,
			max( m_nCurrentWidth, pInfo.nMax ), HEADER_HEIGHT, SWP_NOZORDER );
	}
}

void CMatchCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/) 
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
		ScrollBy( m_nPageCount );
		break;
	case SB_PAGEUP:
		ScrollBy( -m_nPageCount );
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

void CMatchCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/) 
{
	SCROLLINFO pInfo;

	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;

	GetScrollInfo( SB_HORZ, &pInfo );
	int nDelta = pInfo.nPos;

	switch ( nSBCode )
	{
	case SB_BOTTOM:
		pInfo.nPos = pInfo.nMax - pInfo.nPage;
		break;
	case SB_LINEDOWN:
		pInfo.nPos ++;
		break;
	case SB_LINEUP:
		pInfo.nPos --;
		break;
	case SB_PAGEDOWN:
		pInfo.nPos += pInfo.nPage;
		break;
	case SB_PAGEUP:
		pInfo.nPos -= pInfo.nPage;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		pInfo.nPos = nPos;
		break;
	case SB_TOP:
		pInfo.nPos = 0;
		break;
	}

	pInfo.nPos = max( 0, min( pInfo.nPos, pInfo.nMax - (int)pInfo.nPage + 1 ) );
	if ( pInfo.nPos == nDelta ) return;

	SetScrollInfo( SB_HORZ, &pInfo, TRUE );

	m_wndHeader.SetWindowPos( NULL, -pInfo.nPos, 0,
		max( m_nCurrentWidth, pInfo.nMax ), HEADER_HEIGHT, SWP_NOZORDER );

	RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW );
}

BOOL CMatchCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// Scroll window under cursor
	if ( CWnd* pWnd = WindowFromPoint( pt ) )
	{
		if ( pWnd != this )
		{
			if ( pWnd == FindWindowEx(
				GetParent()->GetSafeHwnd(), NULL, NULL, _T("CPanelCtrl") ) )
			{
				return pWnd->PostMessage( WM_MOUSEWHEEL, MAKEWPARAM( nFlags, zDelta ), MAKELPARAM( pt.x, pt.y ) );
			}
		}
	}

	ScrollBy( zDelta / WHEEL_DELTA * -m_nScrollWheelLines );
	return TRUE;
}

void CMatchCtrl::ScrollBy(int nDelta)
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );

	int nIndex = GetScrollPos( SB_VERT ) + nDelta;
	nIndex = max( 0, nIndex );
	ScrollTo( nIndex );
}

void CMatchCtrl::ScrollTo(DWORD nIndex)
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );

	DWORD nLimit = m_pMatches->m_nItems;
	if ( nLimit > (DWORD)m_nPageCount ) nLimit -= m_nPageCount;
	else nLimit = 0;
	nIndex = min( nIndex, nLimit );
	
	DWORD nScroll = 0;
	
	m_nTopIndex = 0;
	m_nHitIndex = 0;
	m_nBottomIndex = 0xFFFFFFFF;
	
	CMatchFile** ppFile = m_pMatches->m_pFiles;
	
	for ( DWORD nFiles = 0 ; nFiles < m_pMatches->m_nFiles ; nFiles++, ppFile++ )
	{
		DWORD nCount = (*ppFile)->GetItemCount();
		if ( ! nCount ) continue;
		
		m_nTopIndex = nFiles;
		
		if ( nIndex < nCount )
		{
			m_nHitIndex = nIndex;
			nScroll += nIndex;
			break;
		}
		
		nIndex -= nCount;
		nScroll += nCount;
	}
	
	UpdateScroll( nScroll );
	
	CRect rc;
	GetClientRect( &rc );
	rc.top += HEADER_HEIGHT;
	
	// RedrawWindow( &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW );
	RedrawWindow( &rc, NULL, RDW_INVALIDATE );
}

/////////////////////////////////////////////////////////////////////////////
// CMatchCtrl painting

BOOL CMatchCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

void CMatchCtrl::OnPaint() 
{
	CSingleLock pLock( &m_pMatches->m_pSection );
	
	if ( ! pLock.Lock( 80 ) )
	{
		PostMessage( WM_TIMER, 1 );
		return;
	}
	
	CRect rcClient, rcItem;
	CPaintDC dc( this );
	if ( Settings.General.LanguageRTL ) dc.SetTextAlign( TA_RTLREADING );
	
	GetClientRect( &rcClient );
	rcClient.top += HEADER_HEIGHT;
	
	dc.SetViewportOrg( -GetScrollPos( SB_HORZ ), 0 );
	
	INT nZeroInt, nColWidth;
	GetScrollRange( SB_HORZ, &nZeroInt, &nColWidth );
	rcClient.right = max( rcClient.right, LONG(nColWidth) );
	
	CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );
	
	m_nTrailWidth = dc.GetTextExtent( _T("\x2026") ).cx;
	
	rcItem.SetRect( rcClient.left, rcClient.top, rcClient.right, 0 );
	rcItem.top -= m_nHitIndex * ITEM_HEIGHT;
	rcItem.bottom = rcItem.top + ITEM_HEIGHT;
	
	CMatchFile** ppFile = m_pMatches->m_pFiles + m_nTopIndex;
	BOOL bFocus = ( GetFocus() == this );
	
    DWORD nIndex = m_nTopIndex;
	for (	;
			nIndex < m_pMatches->m_nFiles && rcItem.top < rcClient.bottom ;
			nIndex++, ppFile++ )
	{
		CMatchFile* pFile = *ppFile;
		int nCount = pFile->GetFilteredCount();
		
		// Don't paint the file if it has been filtered.
		if ( ! nCount ) continue;
		
		if ( rcItem.top >= rcClient.top && dc.RectVisible( &rcItem ) )
		{
			DrawItem( dc, rcItem, pFile, NULL, bFocus && ( nIndex == m_nFocus ) );
		}
		
		rcItem.top += ITEM_HEIGHT;
		rcItem.bottom += ITEM_HEIGHT;
		
		if ( nCount > 1 && pFile->m_bExpanded )
		{
			for ( CQueryHit* pHit = pFile->GetHits() ; pHit ; pHit = pHit->m_pNext )
			{
				// Don't paint filtered hits.
				if ( ! pHit->m_bFiltered ) continue;
				
				if ( rcItem.top >= rcClient.top && dc.RectVisible( &rcItem ) )
				{
					DrawItem( dc, rcItem, pFile, pHit, FALSE );
				}
				
				rcItem.top += ITEM_HEIGHT;
				rcItem.bottom += ITEM_HEIGHT;
				
				if ( rcItem.top >= rcClient.bottom ) break;
			}
		}
	}
	
	m_nBottomIndex = nIndex + 1;
	
	if ( m_pMatches->m_nFilteredFiles == 0 && m_sMessage.GetLength() )
	{
		dc.SetViewportOrg( 0, 0 );
		GetClientRect( &rcClient );
		rcClient.top += HEADER_HEIGHT;
		DrawEmptyMessage( dc, rcClient );
	}
	
	dc.SelectObject( pOldFont );
	
	rcItem.bottom = rcClient.bottom;
	
	if ( dc.RectVisible( &rcItem ) )
	{
		dc.FillSolidRect( &rcItem, CoolInterface.m_crWindow );
	}
}

void CMatchCtrl::DrawItem(CDC& dc, CRect& rcRow, CMatchFile* pFile, CQueryHit* pHit, BOOL bFocus)
{
	TCHAR szBuffer[64];
	
	int nColumns	= m_wndHeader.GetItemCount();
	int nHits		= pHit ? 0 : pFile->GetFilteredCount();
	
	LPCTSTR pszName	= pHit ? pHit->m_sName : pFile->m_sName;
	LPCTSTR pszType	= _tcsrchr( pszName, '.' );
	int nNameLen	= static_cast< int >( pszType ? pszType - pszName : _tcslen( pszName ) );
	
	BOOL bSelected	= pHit ? pHit->m_bSelected : pFile->m_bSelected;
	COLORREF crWnd	= CoolInterface.m_crWindow;

	COLORREF crText	= bSelected ? CoolInterface.m_crHiText : CoolInterface.m_crText ;
	COLORREF crBack	= crWnd ;
	COLORREF crLeftAligned = crBack ;

	if ( pFile->m_bCollection )
	{	// Pale blue background for collections
		if ( CoolInterface.m_crSearchCollection ) crWnd = crBack = CoolInterface.m_crSearchCollection ;
		else crWnd = crBack = CCoolInterface::CalculateColour( crBack, RGB( 0, 0, 255 ), 25 );
	}
	else if ( pFile->m_bTorrent )
	{	// Pale red background for torrents
		if ( CoolInterface.m_crSearchTorrent ) crWnd = crBack = CoolInterface.m_crSearchTorrent ;
		else crWnd = crBack = CCoolInterface::CalculateColour( crBack, RGB( 255, 0, 0 ), 10 );
	}
	else if ( ( pFile->m_nRated > 1 ) && ( ( pFile->m_nRating / pFile->m_nRated ) >= 5 ) )
	{	// Gold highlight for highly rated files
		if ( CoolInterface.m_crSearchHighrated ) crWnd = crBack = CoolInterface.m_crSearchHighrated ;
		else crWnd = crBack = CCoolInterface::CalculateColour( crBack, RGB( 255, 255, 0 ), 20 );
	}

	if ( pFile->GetLibraryStatus() == TRI_FALSE )
	{
		// Green if already in the library
		crText = pHit ? CoolInterface.m_crSearchExistsHit : CoolInterface.m_crSearchExists;
		if ( bSelected ) crText = CoolInterface.m_crSearchExistsSelected;
	}
	else if ( pFile->GetLibraryStatus() == TRI_TRUE )
	{
		// Brown/Orange if a ghost rating is in the library
		crText = CoolInterface.m_crSearchGhostrated;
	}
	else if ( pFile->m_bDownload || ( pHit && pHit->m_bDownload ) )
	{
		// Blue if chosen for download
		crText = pHit ? CoolInterface.m_crSearchQueuedHit : CoolInterface.m_crSearchQueued;
		if ( bSelected ) crText = CoolInterface.m_crSearchQueuedSelected;
	}

	if ( bSelected )
	{
		crBack = CoolInterface.m_crHighlight;
	}
	else if ( ( pHit && ( pHit->m_bBogus || pHit->m_sURL.IsEmpty() || ! pHit->m_bMatched ) ) ||
			  ( ! pHit && ! pFile->m_bOneValid ) ||
			  ( pHit && pHit->m_bPush == TRI_TRUE && Network.IsStable() == FALSE ) )
	{
		// Greyed Out if Unstable (or Brown if also Ghostrated)
		crText = pFile->GetLibraryStatus() == TRI_TRUE ? CoolInterface.m_crSearchGhostrated : CoolInterface.m_crSearchNull;
	}
	
	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( crBack );
	
	dc.SelectObject( Settings.Search.HighlightNew && ( pHit ? pHit->m_bNew : pFile->m_bNew )
		? &CoolInterface.m_fntBold : &CoolInterface.m_fntNormal );
	
	for ( int nColumn = 0 ; nColumn < nColumns ; nColumn++ )
	{
		HDITEM pColumn = { HDI_FORMAT|HDI_WIDTH|HDI_ORDER };
		CRect rcCol;
		
		Header_GetItem( m_wndHeader.GetSafeHwnd(), nColumn, &pColumn );
		Header_GetItemRect( m_wndHeader.GetSafeHwnd(), nColumn, &rcCol );
		
		int nLeft = rcCol.left;
		rcCol.top = rcRow.top;
		rcCol.bottom = rcRow.bottom;
		
		POINT ptHover;
		RECT  rcTick = { rcCol.left+2, rcCol.top+2, rcCol.left+14, rcCol.bottom-2 };
		GetCursorPos(&ptHover);
		ScreenToClient(&ptHover);

		LPCTSTR pszText	= _T("");
		int nText		= -1;
		int nPosition;
		
		dc.SetTextColor( crText );
		
		switch ( nColumn )
		{
		case MATCH_COL_NAME:
			if ( rcCol.Width() < 32 ) break;

			pszText = pszName;
			nText	= nNameLen;

			crLeftAligned = ( rcRow.left == rcCol.left ? crWnd : crBack ) ;

			if ( ! pHit && nHits > 1 )
			{
				// Draw [+] or [-]
				if ( pFile->m_bExpanded )
				{
					CoolInterface.Draw( &dc, PtInRect(&rcTick,ptHover) ? IDI_MINUS_HOVER : IDI_MINUS,
						16, rcCol.left, rcCol.top, crLeftAligned );
				}
				else
				{
					CoolInterface.Draw( &dc, PtInRect(&rcTick,ptHover) ? IDI_PLUS_HOVER : IDI_PLUS,
						16, rcCol.left, rcCol.top, crLeftAligned );
				}

				// Draw file icon
				ShellIcons.Draw( &dc, pFile->m_nShellIndex, 16, rcCol.left + 16, rcCol.top,
					crLeftAligned, bSelected );

				dc.FillSolidRect( rcCol.left, rcCol.top + 16, 32, ITEM_HEIGHT - 16, crLeftAligned );
				
				rcCol.left += 32;
			}
			else
			{
				dc.FillSolidRect( rcCol.left, rcCol.top, ( pHit ? 24 : 16 ),
					ITEM_HEIGHT, crLeftAligned );
				rcCol.left += ( pHit ? 24 : 16 );
				
				// Draw file icon
				ShellIcons.Draw( &dc, pFile->m_nShellIndex, 16, rcCol.left, rcCol.top,
					crLeftAligned, bSelected );

				// Draw partial status
				if ( ! pFile->m_bDRM &&
					( ( pHit && pHit->m_nPartial ) ||
					( nHits == 1 && pFile->GetBestPartial() ) ) )
				{
					CoolInterface.Draw( &dc, IDI_PARTIAL, 16,
						rcCol.left, rcCol.top, CLR_NONE, bSelected, FALSE );
				}

				dc.FillSolidRect( rcCol.left, rcCol.top + 16, 16, ITEM_HEIGHT - 16, crLeftAligned );

				rcCol.left += 16;
			}

			if ( pFile->m_bDRM )
				// Draw DRM status
				CoolInterface.Draw( &dc, IDI_COMMERCIAL, 16,
					rcCol.left - 16, rcCol.top, CLR_NONE, bSelected );

			dc.FillSolidRect( rcCol.left, rcCol.top, 1, ITEM_HEIGHT, crLeftAligned );
			rcCol.left += 1;
			
			if ( bSelected && bFocus )
			{
				CRect rcFocus( &rcRow );
				rcFocus.left = rcCol.left;
				dc.Draw3dRect( &rcFocus, CoolInterface.m_crHiBorder, CoolInterface.m_crHiBorder );
				dc.ExcludeClipRect( rcFocus.left, rcFocus.top, rcFocus.right, rcFocus.top + 1 );
				dc.ExcludeClipRect( rcFocus.left, rcFocus.bottom - 1, rcFocus.right, rcFocus.bottom );
				dc.ExcludeClipRect( rcFocus.left, rcFocus.top + 1, rcFocus.left + 1, rcFocus.bottom - 1 );
				dc.ExcludeClipRect( rcFocus.right - 1, rcFocus.top + 1, rcFocus.right, rcFocus.bottom - 1 );
			}
			
			break;

		case MATCH_COL_TYPE:
			if ( pszType ) pszText = pszType + 1;
			break;

		case MATCH_COL_SIZE:
			pszText = pFile->m_sSize;
			break;
		
		case MATCH_COL_STATUS:
			if ( pHit )
			{
				DrawStatus( dc, rcCol, pFile, pHit, bSelected, crBack );
			}
			else if ( nHits == 1 )
			{
				DrawStatus( dc, rcCol, pFile, pFile->GetBest(), bSelected, crBack );
			}
			else
			{
				DrawStatus( dc, rcCol, pFile, NULL, bSelected, crBack );
			}
			break;
			
		case MATCH_COL_RATING:
			if ( pHit )
			{
				DrawRating( dc, rcCol, pHit->m_nRating, bSelected, crBack );
			}
			else if ( nHits == 1 )
			{
				DrawRating( dc, rcCol, pFile->GetBestRating(), bSelected, crBack );
			}
			else
			{
				DrawRating( dc, rcCol,
					pFile->m_nRated ? pFile->m_nRating / pFile->m_nRated : 0,
					bSelected, crBack );
			}
			break;
			
		case MATCH_COL_COUNT:
			if ( nHits == 1 || pHit != NULL )
			{
				const CQueryHit* ppHit = ( nHits == 1 || pHit == NULL ) ? pFile->GetBest() : pHit;
				CString strTemp;

				if ( Settings.Search.ShowNames && !ppHit->m_sNick.IsEmpty() )
				{
					strTemp = ppHit->m_sNick;

					if ( ppHit->GetSources() > 1 )
						_sntprintf( szBuffer, _countof( szBuffer ), _T("%s+%lu"), (LPCTSTR)strTemp, ppHit->GetSources() - 1 );
					else
						_sntprintf( szBuffer, _countof( szBuffer ), _T("%s"), (LPCTSTR)strTemp );
				}
				else if ( ( ppHit->m_nProtocol == PROTOCOL_ED2K ) && ( ppHit->m_bPush == TRI_TRUE ) )
				{
					/* strText.Format( _T("%lu@%s"), ppHit->m_oClientID.begin()[2], (LPCTSTR)CString( inet_ntoa( (IN_ADDR&)*ppHit->m_oClientID.begin() ) ) ); */
					strTemp.Format( _T("(%s)"), (LPCTSTR)CString( inet_ntoa( (IN_ADDR&)*ppHit->m_oClientID.begin() ) ) );

					if ( ppHit->GetSources() > 1 )
						_sntprintf( szBuffer, _countof( szBuffer ), _T("%s+%lu"), (LPCTSTR)strTemp, ppHit->GetSources() - 1 );
					else
						_sntprintf( szBuffer, _countof( szBuffer ), _T("%s"), (LPCTSTR)strTemp );
				}
				else if ( ppHit->m_pAddress.S_un.S_addr )
				{
					if ( ppHit->GetSources() > 1 )
					{
						_sntprintf( szBuffer, _countof( szBuffer ), _T("%s+%lu"), (LPCTSTR)CString( inet_ntoa( ppHit->m_pAddress ) ), ppHit->GetSources() - 1 );
					}
					else
					{
						_tcscpy( szBuffer, (LPCTSTR)CString( inet_ntoa( ppHit->m_pAddress ) ) );
					}
				}
				else
				{
					if ( ppHit->GetSources() )
					{
						CString strSource, strText;
						LoadSourcesString( strSource, pFile->m_nFiltered );
						int nAltSources = pFile->m_nSources - pFile->m_nFiltered;
						if ( nAltSources > 0 )
						{
							strText.Format( _T("%u %s+%u"), pFile->m_nFiltered, (LPCTSTR)strSource, (DWORD)nAltSources );
						}
						else
						{
							strText.Format( _T("%u %s"), pFile->m_nFiltered, (LPCTSTR)strSource );
						}
						_tcsncpy( szBuffer, (LPCTSTR)strText, _countof( szBuffer ) );
					}
					else
					{
						// Not used?
						_tcscpy( szBuffer, _T("(Firewalled)") );
					}
				}
			}
			else
			{
				CString strSource, strText;
				LoadSourcesString( strSource, pFile->m_nFiltered );
				int nAltSources = pFile->m_nSources - pFile->m_nFiltered;
				if ( nAltSources > 0 )
				{
					strText.Format( _T("%u %s+%u"), pFile->m_nFiltered, (LPCTSTR)strSource, (DWORD)nAltSources );
				}
				else
				{
					strText.Format( _T("%u %s"), pFile->m_nFiltered, (LPCTSTR)strSource );
				}
				_tcsncpy( szBuffer, (LPCTSTR)strText, _countof( szBuffer ) );
			}
			szBuffer[ _countof( szBuffer ) - 1 ] = 0;
			pszText = szBuffer;
			break;
			
		case MATCH_COL_SPEED:
			if ( pHit )
			{
				pszText = pHit->m_sSpeed;
			}
			else
			{
				pszText = pFile->m_sSpeed;
			}
			break;

		case MATCH_COL_CLIENT:
			if ( pHit )
			{
				pszText = pHit->m_pVendor->m_sName;
			}
			else if ( nHits == 1 )
			{
				pszText = pFile->GetBestVendorName();
			}
			break;

		case MATCH_COL_TIME:
			{
				SYSTEMTIME st;
				if ( pFile->m_pTime.GetAsSystemTime( st ) )
				{
					// Caching for slow GetDateFormat/GetTimeFormat functions
					// using fact that neighbour hits got at same time
					static TCHAR szBufferCache[ 64 ] = {};
					static SYSTEMTIME stCache = {};
					st.wMilliseconds = 0;	// round to seconds
					if ( memcmp( &stCache, &st, sizeof( SYSTEMTIME ) ) == 0 )
						// Use cache
						pszText = szBufferCache;
					else
					{
						// Get new date
						int nChars = GetDateFormat( LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, szBuffer, _countof( szBuffer ) );
						szBuffer[ nChars - 1 ] = _T(' ');
						GetTimeFormat( LOCALE_USER_DEFAULT, 0, &st, NULL, szBuffer + nChars, _countof( szBuffer ) - nChars );
						pszText = szBuffer;

						// Save to cache
						_tcscpy( szBufferCache, szBuffer );
						memcpy( &stCache, &st, sizeof( SYSTEMTIME ) ); 
					}
				}
			}
			break;

		case MATCH_COL_COUNTRY:
			if ( pHit )
			{
				DrawCountry( dc, rcCol, pHit->m_sCountry, bSelected, crBack );
				pszText = pHit->m_sCountry;
			}
			else if ( nHits == 1 )
			{
				DrawCountry( dc, rcCol, pFile->GetBestCountry(), bSelected, crBack );
				pszText = pFile->GetBestCountry();
			}
			break;

		default:
			if ( pFile->m_pColumns == NULL ) break;
			pszText = pFile->m_pColumns[ nColumn - MATCH_COL_MAX ];
			nText = static_cast< int >( _tcslen( pszText ) );
			nText = min( nText, 128 );
			break;

		}

		if ( nText < 0 ) nText = static_cast< int >( _tcslen( pszText ) );
		int nWidth = 0;
		int nTrail = 0;
		const int nColWidth = rcCol.Width() - 4;
		if ( nColWidth > 4 )
		{
			while ( nText )
			{
				nWidth = dc.GetTextExtent( pszText, nText ).cx + nTrail;
				if ( nWidth <= nColWidth )
					break;
				nTrail = m_nTrailWidth;
				nText--;
			}
		}
		else
		{
			nTrail = m_nTrailWidth;
			nWidth = nText = 0;
		}
		
		switch ( pColumn.fmt & HDF_JUSTIFYMASK )
		{
		default:
			nPosition = rcCol.left + 4;
			break;
		case HDF_CENTER:
			nPosition = ( rcCol.left + rcCol.right ) / 2 - nWidth / 2;
			break;
		case HDF_RIGHT:
			nPosition = rcCol.right - 4 - nWidth;
			break;
		}

		dc.SetBkColor( crBack );

		if ( nTrail )
		{
			CString strTrail;
			LPTSTR pszTrail = strTrail.GetBuffer( nText + 1 );
			CopyMemory( pszTrail, pszText, nText * sizeof(TCHAR) );
			pszTrail[ nText ] = _T('\x2026');
			strTrail.ReleaseBuffer( nText + 1 );
			dc.ExtTextOut( nPosition, rcCol.top + 2, ETO_CLIPPED|ETO_OPAQUE,
				&rcCol, strTrail, nText + 1, NULL );
		}
		else
		{
			dc.ExtTextOut( nPosition, rcCol.top + 2, ETO_CLIPPED|ETO_OPAQUE,
				&rcCol, pszText, nText, NULL );
		}

		dc.ExcludeClipRect( nLeft, rcCol.top, rcCol.right, rcCol.bottom  );
	}

	dc.FillSolidRect( &rcRow, crBack );
}

void CMatchCtrl::DrawStatus(CDC& dc, CRect& rcCol, CMatchFile* pFile, CQueryHit* pHit, BOOL bSelected, COLORREF crBack)
{
	if ( rcCol.Width() < 16 * 3 ) return;
	
	int nLeft = rcCol.left;
	
	if ( rcCol.Width() > 16 * 6 )
		nLeft = ( rcCol.left + rcCol.right ) / 2 - ( 16 * 6 ) / 2;
	
	int nPos = nLeft;
	TRISTATE bState;
	
	if ( ( bState = pHit ? pHit->m_bBusy : pFile->m_bBusy ) != FALSE )
	{
		CoolInterface.Draw( &dc, ( bState == TRI_TRUE ? IDI_BUSY : IDI_TICK ), 16,
			nPos, rcCol.top, crBack, bSelected );
	}
	else
	{
		dc.FillSolidRect( nPos, rcCol.top, 16, 16, crBack );
	}
	
	nPos += 16;
	
	if ( ( bState = pHit ? pHit->m_bPush : pFile->m_bPush ) != FALSE )
	{
		CoolInterface.Draw( &dc, ( bState == TRI_TRUE ? IDI_FIREWALLED : IDI_TICK ), 16,
			nPos, rcCol.top, crBack, bSelected );
	}
	else
	{
		dc.FillSolidRect( nPos, rcCol.top, 16, 16, crBack );
	}
	
	nPos += 16;
	
	if ( ( bState = pHit ? pHit->m_bStable : pFile->m_bStable ) != FALSE )
	{
		CoolInterface.Draw( &dc, ( bState == TRI_TRUE ? IDI_TICK : IDI_UNSTABLE ), 16,
			nPos, rcCol.top, crBack, bSelected );
	}
	else
	{
		dc.FillSolidRect( nPos, rcCol.top, 16, 16, crBack );
	}
	
	nPos += 16;

	if ( nPos + 16 < rcCol.right )
	{
		if ( pHit ? pHit->m_bPreview : pFile->m_bPreview )
		{
			CoolInterface.Draw( &dc, IDI_PREVIEW, 16, nPos, rcCol.top, crBack, bSelected );
		}
		else
		{
			dc.FillSolidRect( nPos, rcCol.top, 16, 16, crBack );
		}

		nPos += 16;
	}
	
	if ( nPos + 16 < rcCol.right && pHit )
	{
		if ( pHit->m_bBrowseHost )
		{
			CoolInterface.Draw( &dc, IDI_BROWSE, 16, nPos, rcCol.top, crBack, bSelected );
		}
		else
		{
			dc.FillSolidRect( nPos, rcCol.top, 16, 16, crBack );
		}
		
		nPos += 16;
	}
	
	if ( nPos + 16 < rcCol.right && pHit )
	{
		if ( pHit->m_bChat )
		{
			CoolInterface.Draw( &dc, IDR_CHATFRAME, 16,
				nPos, rcCol.top, crBack, bSelected );
		}
		else
		{
			dc.FillSolidRect( nPos, rcCol.top, 16, 16, crBack );
		}

		nPos += 16;
	}
	
	dc.ExcludeClipRect( nLeft, rcCol.top, nPos, rcCol.top + 16 );
}

void CMatchCtrl::DrawRating(CDC& dc, CRect& rcCol, int nRating, BOOL bSelected, COLORREF crBack)
{
	if ( nRating > 1 && nRating <= 6 )
	{
		CPoint pt( rcCol.left, rcCol.top + 2 );
		
		if ( rcCol.Width() >= 12 * 5 )
		{
			pt.x += rcCol.Width() / 2;
			pt.x -= 6 * ( --nRating );
		}
		else
		{
			nRating = min( nRating - 1, rcCol.Width() / 12 );
		}
		
		while ( nRating-- )
		{
			ImageList_DrawEx( m_pStars, 0, dc, pt.x, pt.y, 12, 12, crBack,
				crBack, bSelected ? ILD_BLEND50 : ILD_NORMAL );
			dc.ExcludeClipRect( pt.x, pt.y, pt.x + 12, pt.y + 12 );
			pt.x += 12;
		}
	}
	else if ( nRating == 1 && rcCol.Width() > 12 )
	{
		CPoint pt( ( rcCol.left + rcCol.right ) / 2 - 6, rcCol.top + 2 );
		ImageList_DrawEx( m_pStars, 6, dc, pt.x, pt.y, 12, 12, crBack,
			crBack, bSelected ? ILD_BLEND50 : ILD_NORMAL );
		dc.ExcludeClipRect( pt.x, pt.y, pt.x + 12, pt.y + 12 );
	}
	
	dc.FillSolidRect( &rcCol, crBack );
}

void CMatchCtrl::DrawCountry(CDC& dc, CRect& rcCol, const CString& sCountry, BOOL bSelected, COLORREF crBack)
{
	int nFlagIndex = Flags.GetFlagIndex( sCountry );
	// If the column is very narrow then don't draw the flag.
	if ( nFlagIndex >= 0 && rcCol.Width() >= 22 )
	{
		CPoint pt( rcCol.left, rcCol.top - 1 );
		Flags.Draw( nFlagIndex, dc, pt.x, pt.y + 2, crBack, crBack, bSelected ? ILD_BLEND50 : ILD_NORMAL );
		dc.ExcludeClipRect( pt.x, pt.y + 2, pt.x + 16, pt.y + 16 );
		dc.FillSolidRect( &rcCol, crBack );
		rcCol.left += 16;
	}
}

void CMatchCtrl::DrawEmptyMessage(CDC& dc, CRect& rcClient)
{
	CPoint ptText;
	CRect rcText;
	CSize szText;
	
	rcText.SetRect( rcClient.left, 16, rcClient.right, 0 );
	rcText.bottom = ( rcClient.top + rcClient.bottom ) / 2;
	rcText.top = rcText.bottom - rcText.top;

	if ( ! m_bSearchLink ) rcText.OffsetRect( 0, rcText.Height() / 2 );

	dc.SetBkMode( TRANSPARENT );
	dc.SetBkColor( CoolInterface.m_crWindow );
	dc.SetTextColor( CoolInterface.m_crText );
	dc.SelectObject( &CoolInterface.m_fntNormal );
	
	szText		= dc.GetTextExtent( m_sMessage );
	ptText.x	= ( rcText.left + rcText.right ) / 2 - szText.cx / 2;
	ptText.y	= ( rcText.top + rcText.bottom ) / 2 - szText.cy / 2;
	
	dc.ExtTextOut( ptText.x, ptText.y, ETO_CLIPPED|ETO_OPAQUE, &rcText, m_sMessage, NULL );
	dc.ExcludeClipRect( &rcText );

	if ( m_bSearchLink )
	{
		const CString strText = LoadString( IDS_SEARCH_AGAIN );

		rcText.OffsetRect( 0, rcText.Height() );

		dc.SelectObject( &CoolInterface.m_fntUnder );
		dc.SetTextColor( CoolInterface.m_crTextLink );

		szText		= dc.GetTextExtent( strText );
		ptText.x	= ( rcText.left + rcText.right ) / 2 - szText.cx / 2;
		ptText.y	= ( rcText.top + rcText.bottom ) / 2 - szText.cy / 2;

		dc.ExtTextOut( ptText.x, ptText.y, ETO_CLIPPED|ETO_OPAQUE, &rcText, strText, NULL );
		dc.ExcludeClipRect( &rcText );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMatchCtrl mouse based interaction

BOOL CMatchCtrl::HitTest(const CPoint& point, CMatchFile** poFile, CQueryHit** poHit, DWORD* pnIndex, CRect* pRect)
{
	CSingleLock pLock( &m_pMatches->m_pSection );
	CRect rcClient, rcItem;
	
	if ( poFile ) *poFile = NULL;
	if ( poHit ) *poHit = NULL;
	if ( pnIndex ) *pnIndex = 0xFFFFFFFF;
	
	if ( ! pLock.Lock( 10 ) ) return FALSE;
	
	GetClientRect( &rcClient );
	rcClient.top += HEADER_HEIGHT;
	
	rcItem.SetRect( rcClient.left, rcClient.top, rcClient.right, 0 );
	rcItem.top -= m_nHitIndex * ITEM_HEIGHT;
	rcItem.bottom = rcItem.top + ITEM_HEIGHT;
	
	CMatchFile** ppFile = m_pMatches->m_pFiles + m_nTopIndex;
	
	for (	DWORD nIndex = m_nTopIndex ;
			nIndex < m_pMatches->m_nFiles && rcItem.top < rcClient.bottom ;
			nIndex++, ppFile++ )
	{
		CMatchFile* pFile = *ppFile;
		int nCount = pFile->GetFilteredCount();
		
		if ( ! nCount ) continue;
		
		if ( rcItem.top >= rcClient.top && rcItem.PtInRect( point ) )
		{
			*poFile = pFile;
			if ( pnIndex ) *pnIndex = nIndex;
			if ( pRect ) *pRect = rcItem;
			return TRUE;
		}
		
		rcItem.top += ITEM_HEIGHT;
		rcItem.bottom += ITEM_HEIGHT;
		
		if ( nCount > 1 && pFile->m_bExpanded )
		{
			for ( CQueryHit* pHit = pFile->GetHits() ; pHit ; pHit = pHit->m_pNext )
			{
				if ( ! pHit->m_bFiltered ) continue;
				
				if ( rcItem.top >= rcClient.top && rcItem.PtInRect( point ) )
				{
					*poFile = pFile;
					*poHit = pHit;
					if ( pnIndex ) *pnIndex = nIndex;
					if ( pRect ) *pRect = rcItem;
					return TRUE;
				}
				
				rcItem.top += ITEM_HEIGHT;
				rcItem.bottom += ITEM_HEIGHT;
				
				if ( rcItem.top >= rcClient.bottom ) break;
			}
		}
	}
	
	return FALSE;
}

BOOL CMatchCtrl::GetItemRect(CMatchFile* pFindFile, CQueryHit* pFindHit, CRect* pRect)
{
	CSingleLock pLock( &m_pMatches->m_pSection );
	CRect rcClient, rcItem;
	
	if ( ! pLock.Lock( 10 ) ) return FALSE;
	
	GetClientRect( &rcClient );
	rcClient.top += HEADER_HEIGHT;
	
	rcItem.SetRect( rcClient.left, rcClient.top, rcClient.right, rcClient.top + ITEM_HEIGHT );
	rcItem.top -= m_nHitIndex * ITEM_HEIGHT;
	rcItem.bottom = rcItem.top + ITEM_HEIGHT;
	
	if ( m_nTopIndex > 0 )
	{
		CMatchFile** ppFile = m_pMatches->m_pFiles + m_nTopIndex - 1;
		
		for ( DWORD nIndex = m_nTopIndex ; nIndex ; nIndex--, ppFile-- )
		{
			CMatchFile* pFile = *ppFile;
			int nCount = pFile->GetFilteredCount();
			
			if ( ! nCount ) continue;
			
			rcItem.top -= ITEM_HEIGHT;
			rcItem.bottom -= ITEM_HEIGHT;
			
			if ( nCount > 1 && pFile->m_bExpanded )
			{
				for ( CQueryHit* pHit = pFile->GetHits() ; pHit ; pHit = pHit->m_pNext )
				{
					if ( ! pHit->m_bFiltered ) continue;
					
					rcItem.top -= ITEM_HEIGHT;
					rcItem.bottom -= ITEM_HEIGHT;
				}
			}
		}
	}
	
	CMatchFile** ppFile = ppFile = m_pMatches->m_pFiles;
	
	for ( DWORD nIndex = m_pMatches->m_nFiles ; nIndex ; nIndex--, ppFile++ )
	{
		CMatchFile* pFile = *ppFile;
		int nCount = pFile->GetFilteredCount();
		
		if ( ! nCount ) continue;
		
		if ( pFile == pFindFile )
		{
			*pRect = rcItem;
			return TRUE;
		}
		
		rcItem.top += ITEM_HEIGHT;
		rcItem.bottom += ITEM_HEIGHT;
		
		if ( nCount > 1 && pFile->m_bExpanded )
		{
			for ( CQueryHit* pHit = pFile->GetHits() ; pHit ; pHit = pHit->m_pNext )
			{
				if ( ! pHit->m_bFiltered ) continue;
				
				if ( pHit == pFindHit )
				{
					*pRect = rcItem;
					return TRUE;
				}
				
				rcItem.top += ITEM_HEIGHT;
				rcItem.bottom += ITEM_HEIGHT;
			}
		}
	}
	
	return FALSE;
}

void CMatchCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );
	CMatchFile* pFile;
	CQueryHit* pHit;
	DWORD nIndex;
	CRect rcItem;
	
	SetFocus();
	SetCapture();
	m_wndTip.Hide();
	
	HitTest( point, &pFile, &pHit, &nIndex, &rcItem );
	
	if ( pFile != NULL && pHit == NULL && pFile->GetFilteredCount() > 1 )
	{
		CRect rcHeader;
		Header_GetItemRect( m_wndHeader.GetSafeHwnd(), 0, &rcHeader );
		
		point.x += GetScrollPos( SB_HORZ );
		
		if ( point.x >= rcHeader.left && point.x <= rcHeader.left + 16 )
		{
			pFile->Expand( ! pFile->m_bExpanded );
			NotifySelection();
			Update();
			return;
		}

		point.x -= GetScrollPos( SB_HORZ );
	}
	
	BOOL bChanged = FALSE;
	
	if ( ( nFlags & MK_SHIFT ) == 0 && ( nFlags & MK_CONTROL ) == 0 &&
		 ( nFlags & MK_RBUTTON ) == 0 )
	{
		bChanged |= m_pMatches->ClearSelection();
		m_nFocus = nIndex;
	}

	if ( pFile != NULL )
	{
		if ( nFlags & MK_SHIFT )
		{
			if ( m_pLastSelectedFile == pFile && m_pLastSelectedHit == pHit )
			{
				bChanged |= m_pMatches->ClearSelection();
				m_nFocus = nIndex;
				m_pMatches->Select( pFile, pHit, TRUE );
			}
			else if ( m_pLastSelectedFile )
			{
				if ( ! m_pLastSelectedFile->m_bExpanded )
				{
					m_pLastSelectedHit = NULL;
				}

				bChanged |= m_pMatches->ClearSelection();
				m_nFocus = nIndex;

				BOOL bEnd = FALSE;
				BOOL bFileFound = FALSE;
				BOOL bHitFound = FALSE;
				CMatchFile** ppCurFile = m_pMatches->m_pFiles;
				for ( DWORD i = 0 ; i < m_pMatches->m_nFiles && ! bEnd ; i++, ppCurFile++ )
				{
					if ( *ppCurFile == pFile || *ppCurFile == m_pLastSelectedFile )
					{
						if ( ( *ppCurFile == m_pLastSelectedFile && ! m_pLastSelectedHit ) ||
							 ( *ppCurFile == pFile && ! pHit ) )
						{
							if ( bFileFound )
							{
								bEnd = TRUE;
							}
							bHitFound = TRUE;
						}
						bFileFound = TRUE;
					}
					if ( (*ppCurFile)->GetFilteredCount() )
					{
						if ( bFileFound && bHitFound )
						{
							m_pMatches->Select( *ppCurFile, NULL, TRUE );
						}
						if ( (*ppCurFile)->m_bExpanded && ! bEnd )
						{
							for ( CQueryHit* pCurHit = (*ppCurFile)->GetHits() ; pCurHit ;
								pCurHit = pCurHit->m_pNext )
							{
								if ( pCurHit == pHit || pCurHit == m_pLastSelectedHit )
								{
									if ( bHitFound || pHit == m_pLastSelectedHit )
									{
										bEnd = TRUE;
									}
									bHitFound = TRUE;
								}
								if ( bFileFound && bHitFound && pCurHit->m_bFiltered )
								{
									m_pMatches->Select( *ppCurFile, pCurHit, TRUE );
								}
								if ( bEnd )
								{
									break;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			m_pLastSelectedFile = pFile;
			m_pLastSelectedHit = pHit;

			BOOL bSelected = ( pHit != NULL ) ? pHit->m_bSelected : pFile->m_bSelected;
			
			if ( nFlags & MK_RBUTTON )
			{
				if ( ! bSelected )
				{
					m_pMatches->ClearSelection();
					m_pMatches->Select( pFile, pHit, TRUE );
					m_nFocus = nIndex;
				}
			}
			else
			{
				bChanged |= m_pMatches->Select( pFile, pHit, ! bSelected );
			}
		}
	}
	else
	{
		m_pLastSelectedFile = NULL;
		m_pLastSelectedHit = NULL;
	}
	
	if ( bChanged ) NotifySelection();
	Update();
}

void CMatchCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	CWnd::OnMouseMove( nFlags, point );

	CRect rcCol;
	
	GetClientRect( &rcCol );
	rcCol.top += HEADER_HEIGHT;
	
	if ( m_bTips && rcCol.PtInRect( point ) && point.x >= rcCol.left + 16 )
	{
		CSingleLock pLock( &m_pMatches->m_pSection );
		if ( ! pLock.Lock( 100 ) ) return;
		
		CMatchFile* pFile;
		CQueryHit* pHit;
		
		if ( HitTest( point, &pFile, &pHit ) && PixelTest( point ) )
		{
			m_wndTip.Show( pFile, pHit );
			return;
		}
	}
	
	m_wndTip.Hide();

	// [+] or [-] Hoverstates
	if ( point.x < rcCol.left + 18 )
	{
		CRect rcRefresh( 1, rcCol.top - 32, 18, rcCol.bottom + 32 );
		RedrawWindow(rcRefresh);
	}
}

BOOL CMatchCtrl::PixelTest(const CPoint& point)
{
	POINT pNESW[4] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
	CClientDC dc( this );
	COLORREF crEmpty;
	CRect rc;
	
	crEmpty = CoolInterface.m_crWindow;
	if ( dc.GetPixel( point ) != crEmpty ) return TRUE;
	GetClientRect( &rc );
	
	for ( int nDirection = 0 ; nDirection < 4 ; nDirection++ )
	{
		CPoint pt( point );

		for ( int nLength = 16 ; nLength ; nLength-- )
		{
			pt += pNESW[ nDirection ];
			if ( ! rc.PtInRect( pt ) ) break;
			if ( dc.GetPixel( pt ) != crEmpty ) return TRUE;
		}
	}
	
	return FALSE;
}

void CMatchCtrl::OnLButtonUp(UINT /*nFlags*/, CPoint point) 
{
	ReleaseCapture();
	
	if ( m_pMatches->m_nFilteredFiles == 0 && m_bSearchLink )
	{
		CRect rc;
		
		GetClientRect( &rc );
		rc.top += HEADER_HEIGHT;
		
		rc.left		= ( rc.left + rc.right ) / 2 - 64;
		rc.right	= rc.left + 128;
		rc.top		= ( rc.top + rc.bottom ) / 2;
		rc.bottom	= rc.top + 16;
		
		if ( rc.PtInRect( point ) )
		{
			GetOwner()->PostMessage( WM_COMMAND, ID_SEARCH_SEARCH );
			return;
		}
	}
}

void CMatchCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if ( point.x < 16 )
	{
		OnLButtonDown( nFlags, point );
	}
	else
	{
		CMatchFile* pFile	= NULL;
		CQueryHit* pHit		= NULL;
		CRect rcItem;
		
		if ( HitTest( point, &pFile, &pHit, NULL, &rcItem ) )
		{
			// TODO: Check if its on an action icon and take the appropriate action
		}
		
		GetOwner()->PostMessage( WM_COMMAND, ID_SEARCH_DOWNLOAD );
	}
}

void CMatchCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	OnLButtonDown( nFlags, point );
	CWnd::OnRButtonDown( nFlags, point );
}

void CMatchCtrl::OnRButtonUp(UINT nFlags, CPoint point) 
{
	OnLButtonUp( nFlags, point );
	CWnd::OnRButtonUp( nFlags, point );
}

BOOL CMatchCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if ( m_pMatches->m_nFilteredFiles == 0 && m_bSearchLink )
	{
		CPoint point;
		CRect rc;
		
		GetClientRect( &rc );
		rc.top += HEADER_HEIGHT;
		
		rc.left		= ( rc.left + rc.right ) / 2 - 64;
		rc.right	= rc.left + 128;
		rc.top		= ( rc.top + rc.bottom ) / 2;
		rc.bottom	= rc.top + 16;
		ClientToScreen( &rc );
		
		GetCursorPos( &point );
		
		if ( rc.PtInRect( point ) )
		{
			SetCursor( theApp.LoadCursor( IDC_HAND ) );
			return TRUE;
		}
	}
	
	return CWnd::OnSetCursor( pWnd, nHitTest, message );
}

/////////////////////////////////////////////////////////////////////////////
// CMatchCtrl key based interaction

void CMatchCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	BOOL bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0x8000;
	BOOL bControl = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) == 0x8000;
	
	m_wndTip.Hide();
	
	switch ( nChar )
	{
	case 'A':
	case 'a':
		if ( bControl ) SelectAll();
		return;
	case VK_ESCAPE:
		if ( m_pMatches->ClearSelection() ) NotifySelection();
		Update();
		return;
	case VK_HOME:
		MoveFocus( -(int)m_pMatches->m_nItems, bShift );
		return;
	case VK_END:
		MoveFocus( m_pMatches->m_nItems, bShift );
		return;
	case VK_PRIOR:
		MoveFocus( -m_nPageCount, bShift );
		return;
	case VK_NEXT:
		MoveFocus( m_nPageCount, bShift );
		return;
	case VK_UP:
		MoveFocus( -1, bShift );
		return;
	case VK_DOWN:
		MoveFocus( 1, bShift );
		return;
	case VK_RETURN:
		GetOwner()->PostMessage( WM_COMMAND, ID_SEARCH_DOWNLOAD );
		return;
	case VK_DELETE:
		DoDelete();
		return;
	case VK_LEFT:
	case VK_SUBTRACT:
		DoExpand( FALSE );
		break;
	case VK_RIGHT:
	case VK_ADD:
		DoExpand( TRUE );
		break;
	}
	
	CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CMatchCtrl::MoveFocus(int nDelta, BOOL bShift)
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );
	
	if ( m_pMatches->m_nFiles == 0 || nDelta == 0 ) return;
	
	if ( m_nFocus >= m_pMatches->m_nFiles )
	{
		m_nFocus = nDelta > 0 ? 0 : m_pMatches->m_nFiles - 1;
	}
	
	CMatchFile** ppFile = m_pMatches->m_pFiles + m_nFocus;
	int nSign = ( nDelta > 0 ) ? 1 : -1;

	for ( ;	m_nFocus < m_pMatches->m_nFiles
		  ;	m_nFocus += nSign, ppFile += nSign )
	{
		CMatchFile* pFile = *ppFile;
		if ( pFile->GetItemCount() ) break;
	}
	
	if ( m_nFocus >= m_pMatches->m_nFiles )
	{
		m_nFocus = nDelta > 0 ? m_pMatches->m_nFiles - 1 : 0;
		return;
	}
	
	CMatchFile* pFocus = NULL;
	nDelta += nSign;
	
	for (	DWORD nPosition = m_nFocus ;
			nPosition < m_pMatches->m_nFiles && nDelta != 0 ;
			nPosition += nSign, ppFile += nSign )
	{
		CMatchFile* pFile = *ppFile;

		if ( pFile->GetItemCount() )
		{
			m_nFocus	= nPosition;
			pFocus		= pFile;
			nDelta		-= nSign;
		}
	}
	
	if ( pFocus != NULL )
	{
		CRect rcItem, rcClient;
		BOOL bChanged = FALSE;
				
		if ( ! bShift ) bChanged |= m_pMatches->ClearSelection();
		bChanged |= m_pMatches->Select( pFocus, NULL, TRUE );
				
		if ( GetItemRect( pFocus, NULL, &rcItem ) )
		{
			GetClientRect( &rcClient );
			rcClient.top += HEADER_HEIGHT;
			
			if ( rcItem.top < rcClient.top )
			{
				ScrollBy( ( rcItem.top - rcClient.top - ITEM_HEIGHT + 1 ) / ITEM_HEIGHT );
			}
			else if ( rcItem.bottom > rcClient.bottom )
			{
				ScrollBy( ( rcItem.bottom - rcClient.bottom + ITEM_HEIGHT - 1 ) / ITEM_HEIGHT );
			}
		}
		
		if ( bChanged ) NotifySelection();
		Update();
	}
}

void CMatchCtrl::DoDelete()
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );
	BOOL bChanged = FALSE;

	m_pLastSelectedFile = NULL;
	m_pLastSelectedHit = NULL;

	for ( POSITION pos = m_pMatches->m_pSelectedFiles.GetHeadPosition() ; pos ; )
	{
		CMatchFile* pFile = (CMatchFile*)m_pMatches->m_pSelectedFiles.GetNext( pos );
		bChanged |= m_pMatches->Select( pFile, NULL, FALSE );
		pFile->SetBogus( TRUE );
	}

	for ( POSITION pos = m_pMatches->m_pSelectedHits.GetHeadPosition() ; pos ; )
	{
		CQueryHit* pHit = (CQueryHit*)m_pMatches->m_pSelectedHits.GetNext( pos );
		m_pMatches->Select( NULL, pHit, FALSE );
		pHit->m_bBogus = TRUE;
	}
	
	m_pMatches->Filter();
	
	m_wndTip.Hide();
	if ( bChanged ) NotifySelection();
	Update();
}

void CMatchCtrl::DoExpand(BOOL bExpand)
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );
	BOOL bChanged = FALSE;
	
	for ( POSITION pos = m_pMatches->m_pSelectedFiles.GetHeadPosition() ; pos ; )
	{
		CMatchFile* pFile = (CMatchFile*)m_pMatches->m_pSelectedFiles.GetNext( pos );

		bChanged |= pFile->Expand( bExpand );
	}
	
	m_wndTip.Hide();
	if ( bChanged ) NotifySelection();
	Update();
}

void CMatchCtrl::SelectAll()
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );
	BOOL bChanged = FALSE;

	bChanged |= m_pMatches->ClearSelection();

	CMatchFile** ppCurFile = m_pMatches->m_pFiles;
	for ( DWORD i = 0 ; i < m_pMatches->m_nFiles ; i++, ppCurFile++ )
	{
		if ( (*ppCurFile)->GetFilteredCount() )
		{
			m_pMatches->Select( *ppCurFile, NULL, TRUE );
			if ( (*ppCurFile)->m_bExpanded )
			{
				for ( CQueryHit* pCurHit = (*ppCurFile)->GetHits() ; pCurHit ;
					pCurHit = pCurHit->m_pNext )
				{
					if ( pCurHit->m_bFiltered )
					{
						m_pMatches->Select( *ppCurFile, pCurHit, TRUE );
					}
				}
			}
		}
	}

	m_wndTip.Hide();
	if ( bChanged ) NotifySelection();
	Update();
}

void CMatchCtrl::NotifySelection()
{
	GetOwner()->PostMessage( WM_COMMAND, MAKELONG( GetDlgCtrlID(), LBN_SELCHANGE ), (LPARAM)GetSafeHwnd() );
}

/////////////////////////////////////////////////////////////////////////////
// CMatchCtrl header interaction

void CMatchCtrl::OnClickHeader(NMHDR* pNotifyStruct, LRESULT* /*pResult*/)
{
	HD_NOTIFY* pNotify = (HD_NOTIFY*)pNotifyStruct;
	
	if ( m_pMatches->m_nSortColumn == pNotify->iItem )
	{
		if ( m_pMatches->m_bSortDir == 1 )
		{
			SetSortColumn( -1 );
		}
		else
		{
			SetSortColumn( pNotify->iItem, FALSE );
		}
	}
	else
	{
		SetSortColumn( pNotify->iItem, TRUE );
	}
}

void CMatchCtrl::OnChangeHeader(NMHDR* /*pNotifyStruct*/, LRESULT* /*pResult*/)
{
	UpdateScroll();
	Invalidate();
}

void CMatchCtrl::OnTimer(UINT_PTR /*nIDEvent*/) 
{
	Invalidate();
}

void CMatchCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus( pOldWnd );
	Invalidate();
}

void CMatchCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus( pNewWnd );
	Invalidate();
}

UINT CMatchCtrl::OnGetDlgCode()
{
	return DLGC_WANTARROWS;
}
