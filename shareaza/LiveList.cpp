//
// LiveList.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "LiveList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CLiveList construction

IMPLEMENT_DYNAMIC( CLiveList, CObject )

CLiveList::CLiveList(int nColumns) :
	m_nColumns ( nColumns )
{
}

CLiveList::~CLiveList()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CLiveList clear

void CLiveList::Clear()
{
	ASSERT_VALID( this );

	CQuickLock oLock( m_pSection );

	for ( POSITION pos = m_pItems.GetStartPosition() ; pos ; )
	{
		CLiveItem* pItem;
		DWORD_PTR nParam;

		m_pItems.GetNextAssoc( pos, nParam, pItem );
		ASSERT_VALID( pItem );
		delete pItem;
	}
	m_pItems.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CLiveList add

CLiveItem* CLiveList::Add(DWORD_PTR nParam)
{
	ASSERT_VALID( this );

	CLiveItem* pItem = new CLiveItem( m_nColumns, nParam );
	ASSERT_VALID( pItem );
	m_pItems.SetAt( nParam, pItem );
	return pItem;
}

CLiveItem* CLiveList::Add(LPVOID pParam)
{
	ASSERT_VALID( this );

	return Add( (DWORD_PTR)pParam );
}

//////////////////////////////////////////////////////////////////////
// CLiveList apply

void CLiveList::Apply(CListCtrl* pCtrl, BOOL bSort)
{
	ASSERT_VALID( this );

	CQuickLock oLock( m_pSection );

	BOOL bModified = FALSE;

	for ( int nItem = 0 ; nItem < pCtrl->GetItemCount() ; nItem++ )
	{
		DWORD nParam		= (DWORD)pCtrl->GetItemData( nItem );
		CLiveItem* pItem;

		if ( m_pItems.Lookup( nParam, pItem ) )
		{
			ASSERT_VALID( pItem );
			if ( pItem->Update( pCtrl, nItem, m_nColumns ) ) bModified = TRUE;

			delete pItem;
			m_pItems.RemoveKey( nParam );
		}
		else
		{
			pCtrl->DeleteItem( nItem-- );
			bModified = TRUE;
		}
	}

	int nCount = pCtrl->GetItemCount();

	for ( POSITION pos = m_pItems.GetStartPosition() ; pos ; )
	{
		CLiveItem* pItem;
		DWORD_PTR nParam;

		m_pItems.GetNextAssoc( pos, nParam, pItem );
		ASSERT_VALID( pItem );
		pItem->Add( pCtrl, nCount++, m_nColumns );
		bModified = TRUE;

		delete pItem;
	}

	m_pItems.RemoveAll();

	if ( bModified && bSort ) Sort( pCtrl, -1 );
}

//////////////////////////////////////////////////////////////////////
// CLiveItem construction

IMPLEMENT_DYNAMIC( CLiveItem, CObject )

CLiveItem::CLiveItem(int nColumns, DWORD_PTR nParam) :
	m_pColumn		( new CString[ nColumns ] ),
	m_nParam		( nParam ),
	m_nImage		( 0 ),
	m_nMaskOverlay	( 0 ),
	m_nMaskState	( 0 )
{
}

CLiveItem::~CLiveItem()
{
	delete [] m_pColumn;
}

//////////////////////////////////////////////////////////////////////
// CLiveItem set

void CLiveItem::Set(int nColumn, LPCTSTR pszText)
{
	ASSERT_VALID( this );
	ASSERT( pszText );

	m_pColumn[ nColumn ] = pszText;
}

//////////////////////////////////////////////////////////////////////
// CLiveItem format

void CLiveItem::Format(int nColumn, LPCTSTR pszFormat, ...)
{
	ASSERT_VALID( this );

	TCHAR szBuffer[1024];
	va_list pArgs;

	va_start( pArgs, pszFormat );
	_vsntprintf( szBuffer, sizeof( szBuffer ) / sizeof( TCHAR ), pszFormat, pArgs );
	szBuffer[ sizeof( szBuffer ) / sizeof( TCHAR ) - 1 ] = 0;
	va_end( pArgs );

	m_pColumn[ nColumn ] = szBuffer;
}

//////////////////////////////////////////////////////////////////////
// CLiveItem add

int CLiveItem::Add(CListCtrl* pCtrl, int nItem, int nColumns)
{
	ASSERT_VALID( this );
	ASSERT_VALID( pCtrl );

	LV_ITEM pItem = {};
	pItem.mask		= LVIF_PARAM|LVIF_TEXT|LVIF_IMAGE|LVIF_STATE;
	pItem.iItem		= nItem >= 0 ? nItem : pCtrl->GetItemCount();
	pItem.lParam	= (LPARAM)m_nParam;
	pItem.iImage	= m_nImage;
	pItem.state		= INDEXTOOVERLAYMASK( m_nMaskOverlay ) | INDEXTOSTATEIMAGEMASK( m_nMaskState );
	pItem.stateMask	= LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK;
	pItem.pszText	= (LPTSTR)(LPCTSTR)m_pColumn[0];
	pItem.iItem		= pCtrl->InsertItem( &pItem );
	ASSERT( pItem.iItem != -1 );
	pItem.mask		= LVIF_TEXT;

	for ( pItem.iSubItem = 1 ; pItem.iSubItem < nColumns ; pItem.iSubItem++ )
	{
		pItem.pszText = (LPTSTR)(LPCTSTR)m_pColumn[ pItem.iSubItem ];
		VERIFY( pCtrl->SetItem( &pItem ) );
	}

	return pItem.iItem;
}

//////////////////////////////////////////////////////////////////////
// CLiveItem update

BOOL CLiveItem::Update(CListCtrl* pCtrl, int nItem, int nColumns)
{
	ASSERT_VALID( this );
	ASSERT_VALID( pCtrl );

	BOOL bModified = FALSE;

	LV_ITEM pItem = {};
	pItem.mask		= LVIF_PARAM|LVIF_IMAGE|LVIF_STATE;
	pItem.iItem		= nItem;
	pItem.stateMask	= LVIS_OVERLAYMASK|LVIS_STATEIMAGEMASK;

	if ( ! pCtrl->GetItem( &pItem ) || pItem.lParam != (LPARAM)m_nParam ) return FALSE;

	if ( m_nImage != pItem.iImage )
	{
		pItem.iImage = m_nImage;
		bModified = TRUE;
	}

	if ( ( pItem.state & (LVIS_OVERLAYMASK|LVIS_STATEIMAGEMASK) ) != ( INDEXTOOVERLAYMASK( m_nMaskOverlay ) | INDEXTOSTATEIMAGEMASK( m_nMaskState ) ) )
	{
		pItem.state		= INDEXTOOVERLAYMASK( m_nMaskOverlay ) | INDEXTOSTATEIMAGEMASK( m_nMaskState );
		pItem.stateMask	= LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK;
		bModified = TRUE;
	}

	if ( bModified ) VERIFY( pCtrl->SetItem( &pItem ) );

	for ( int nColumn = 0 ; nColumn < nColumns ; nColumn++ )
	{
		if ( pCtrl->GetItemText( nItem, nColumn ) != m_pColumn[ nColumn ] )
		{
			VERIFY( pCtrl->SetItemText( nItem, nColumn, m_pColumn[ nColumn ] ) );
			bModified = TRUE;
		}
	}

	return bModified;
}

BOOL CLiveItem::SetImage(CListCtrl* pCtrl, int nParam, int nColumn, int nImageIndex)
{
	ASSERT_VALID( this );
	ASSERT_VALID( pCtrl );

	BOOL bModified = FALSE;
	LV_FINDINFO pFind;
	int nItem;

	pFind.flags		= LVFI_PARAM;
	pFind.lParam	= nParam;
	nItem = pCtrl->FindItem( &pFind );

	LV_ITEM pItem = {};
	pItem.mask	= LVIF_IMAGE;
	pItem.iItem	= nItem;
	pItem.iSubItem = nColumn;

	if ( ! pCtrl->GetItem( &pItem ) ) return FALSE;

	if ( pItem.iImage != nImageIndex )
		bModified = TRUE;

	if ( bModified )
	{
		pItem.iImage = nImageIndex;
		VERIFY( pCtrl->SetItem( &pItem ) );
	}

	return bModified;
}

//////////////////////////////////////////////////////////////////////
// CLiveList sort method

CBitmap CLiveList::m_bmSortAsc;
CBitmap CLiveList::m_bmSortDesc;

void CLiveList::Sort(CListCtrl* pCtrl, int nColumn, BOOL bGraphic)
{
	ASSERT_VALID( pCtrl );

	int nOldColumn	= (int)GetWindowLongPtr( pCtrl->GetSafeHwnd(), GWLP_USERDATA );
	BOOL bWaiting	= FALSE;

	if ( nColumn == -1 )
	{
		nColumn = nOldColumn;
	}
	else
	{
		if ( nColumn == abs( nOldColumn ) - 1 )
		{
			if ( nOldColumn > 0 )
				nColumn = 0 - nOldColumn;
			else
				nColumn = 0;
		}
		else
		{
			nColumn++;
		}

		SetWindowLongPtr( pCtrl->GetSafeHwnd(), GWLP_USERDATA, nColumn );

		bWaiting = TRUE;
		theApp.BeginWaitCursor();
	}

#ifdef IDB_SORT_ASC
	if ( bGraphic )
	{
		if ( m_bmSortAsc.m_hObject == NULL )
		{
			m_bmSortAsc.LoadMappedBitmap( IDB_SORT_ASC );
			m_bmSortDesc.LoadMappedBitmap( IDB_SORT_DESC );
		}

		CHeaderCtrl* pHeader = (CHeaderCtrl*)CWnd::FromHandle( (HWND)pCtrl->SendMessage( LVM_GETHEADER ) );
		ASSERT_VALID( pHeader );
		for ( int nCol = 0 ; ; nCol++ )
		{
			HDITEM pColumn = {};
			pColumn.mask = HDI_BITMAP|HDI_FORMAT;

			if ( ! pHeader->GetItem( nCol, &pColumn ) ) break;

			if ( nCol == abs( nColumn ) - 1 )
			{
				pColumn.fmt |= HDF_BITMAP|HDF_BITMAP_ON_RIGHT;
				pColumn.hbm = (HBITMAP)( nColumn > 0 ? m_bmSortAsc.GetSafeHandle() : m_bmSortDesc.GetSafeHandle() );
			}
			else
			{
				pColumn.fmt &= ~HDF_BITMAP;
				pColumn.hbm = NULL;
			}

			VERIFY( pHeader->SetItem( nCol, &pColumn ) );
		}
	}
#endif

	if ( nColumn ) pCtrl->SendMessage( LVM_SORTITEMS, (WPARAM)pCtrl, (LPARAM)SortCallback );

	if ( bWaiting ) theApp.EndWaitCursor();
}

//////////////////////////////////////////////////////////////////////
// CLiveList sort callback

int CALLBACK CLiveList::SortCallback(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CListCtrl* pList	= (CListCtrl*)lParamSort;
	ASSERT_VALID( pList );
	int nColumn			= (int)GetWindowLongPtr( pList->GetSafeHwnd(), GWLP_USERDATA );
	LV_FINDINFO pFind;
	int nA, nB;

	pFind.flags		= LVFI_PARAM;
	pFind.lParam	= lParam1;
	nA = pList->FindItem( &pFind );
	pFind.lParam	= lParam2;
	nB = pList->FindItem( &pFind );

	CString sA, sB;

	BOOL bInv	= ( nColumn > 0 ) ? TRUE : FALSE;
	nColumn		= ( bInv ? nColumn : -nColumn ) - 1;

	sA	= pList->GetItemText( nA, nColumn );
	sB	= pList->GetItemText( nB, nColumn );

	return bInv ? SortProc( sB, sA ) : SortProc( sA, sB );
}

//////////////////////////////////////////////////////////////////////
// Parse IP addresses like "xxx.xxx.xxx.xxx\0" and "xxx.xxx.xxx.xxx/"
// (IP stored in host byte order)

inline BOOL atoip (LPCTSTR c, DWORD& addr)
{
	DWORD digit = 0;
	DWORD num = 0;
	addr = 0;
	for ( ; ; c++ )
	{
		if ( *c >= _T('0') && *c <= _T('9') )
		{
			num = num * 10 + ( *c - _T('0') );
			if ( num > 255 )
				break;				// too big octet
		}
		else if ( *c == _T('.') || *c == _T('\0') || *c == _T('/') )
		{
			addr = ( addr << 8 ) | num;
			num = 0;
			digit++;
			if ( digit == 4 )
			{
				if ( *c == _T('.') )
					break;			// too long
		else
					return TRUE;	// it's IP!
			}
			if ( *c == _T('\0') || *c == _T('/') )
				break;				// too short
		}
		else
			break;
	}
	addr = 0xffffffff;
	return FALSE;					// invalid symbol
}

int CLiveList::SortProc(LPCTSTR sA, LPCTSTR sB, BOOL bNumeric)
{
	DWORD ipA, ipB;
	if ( atoip( sA, ipA ) && atoip( sB, ipB ) )
	{
		TCHAR* pA = (TCHAR*)_tcschr( sA, '/' );
		TCHAR* pB = (TCHAR*)_tcschr( sB, '/' );
		DWORD maskA = 0xffffffff, maskB = 0xffffffff;
		if ( ( ! pA || atoip( pA + 1, maskA ) ) && 
			 ( ! pB || atoip( pB + 1, maskB ) ) )
		{
			QWORD nA = ( ( (QWORD) ipA ) << 32 ) | maskA;
			QWORD nB = ( ( (QWORD) ipB ) << 32 ) | maskB;
			if ( nA < nB )
				return -1;
			else if ( nA > nB )
				return 1;
			else
				return 0;
		}
	}
	if ( bNumeric || ( IsNumber( sA ) && IsNumber( sB ) ) )
		{
		double nA = 0, nB = 0;

			if ( *sA == '(' || *sA == 'Q' )
				_stscanf( sA+1, _T("%lf"), &nA );
			else
				_stscanf( sA, _T("%lf (%lf)"), &nA, &nA );

			if ( *sB == '(' || *sB == 'Q' )
				_stscanf( sB+1, _T("%lf"), &nB );
			else
				_stscanf( sB, _T("%lf (%lf)"), &nB, &nB );

			if ( _tcsstr( sA, _T(" K") ) ) nA *= 1024;
			if ( _tcsstr( sA, _T(" M") ) ) nA *= 1024*1024;
			if ( _tcsstr( sA, _T(" G") ) ) nA *= 1024*1024*1024;
			if ( _tcsstr( sA, _T(" T") ) ) nA *= 1099511627776.0f;

			if ( _tcsstr( sB, _T(" K") ) ) nB *= 1024;
			if ( _tcsstr( sB, _T(" M") ) ) nB *= 1024*1024;
			if ( _tcsstr( sB, _T(" G") ) ) nB *= 1024*1024*1024;
			if ( _tcsstr( sB, _T(" T") ) ) nB *= 1099511627776.0f;

		if ( nA < nB )
			return -1;
		else if ( nA > nB )
			return 1;
		else
			return 0;
	}
	else
	{
		return _tcsicoll( sA, sB );
	}
}

BOOL CLiveList::IsNumber(LPCTSTR pszString)
{
	if ( ! *pszString ) return FALSE;

	// TODO: Is this the best way to do this?
	if ( *pszString == '(' && _tcsstr( pszString, _T(" source") ) != NULL ) return TRUE;
	if ( *pszString == 'Q' && _istdigit( pszString[1] ) ) return TRUE;

	BOOL bSpace = FALSE;
	int nNonDigit = 0;

	for ( ; *pszString ; pszString++ )
	{
		if ( _istdigit( *pszString ) || *pszString == '.' )
		{
			// if ( bSpace ) return FALSE;
		}
		else if ( *pszString == ' ' )
		{
			if ( bSpace ) return FALSE;
			bSpace = TRUE;
		}
		else if ( *pszString == 'k' && ( ! pszString[1] || pszString[1] == '~' ) )
		{
			// Allows ###k and ###k~
			return TRUE;
		}
		else if ( *pszString == '(' || *pszString == ')' )
		{
		}
		else
		{
			if ( ! bSpace ) return FALSE;
			if ( ++nNonDigit > 4 ) return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLiveList dragging helper

HBITMAP CLiveList::CreateDragImage(CListCtrl* pList, const CPoint& ptMouse, CPoint& ptMiddle)
{
	ASSERT_VALID( pList );

	CRect rcClient, rcOne, rcAll( 32000, 32000, -32000, -32000 );
	int nIndex;

	if ( pList->GetSelectedCount() == 0 ) return NULL;

	pList->SetFocus();
	pList->GetClientRect( &rcClient );

	for ( nIndex = -1 ; ( nIndex = pList->GetNextItem( nIndex, LVNI_SELECTED ) ) >= 0 ; )
	{
		pList->GetItemRect( nIndex, rcOne, LVIR_BOUNDS );

		if ( rcOne.IntersectRect( &rcClient, &rcOne ) )
		{
			rcAll.left		= min( rcAll.left, rcOne.left );
			rcAll.top		= min( rcAll.top, rcOne.top );
			rcAll.right		= max( rcAll.right, rcOne.right );
			rcAll.bottom	= max( rcAll.bottom, rcOne.bottom );
		}

		pList->SetItemState( nIndex, 0, LVIS_FOCUSED );
	}

	BOOL bClipped = rcAll.Height() > MAX_DRAG_SIZE;

	if ( bClipped )
	{
		rcAll.left		= max( rcAll.left, ptMouse.x - MAX_DRAG_SIZE_2 );
		rcAll.right		= max( rcAll.right, ptMouse.x + MAX_DRAG_SIZE_2 );
		rcAll.top		= max( rcAll.top, ptMouse.y - MAX_DRAG_SIZE_2 );
		rcAll.bottom	= max( rcAll.bottom, ptMouse.y + MAX_DRAG_SIZE_2 );
	}

	CClientDC dcClient( pList );
	CBitmap bmAll, bmDrag;
	CDC dcAll, dcDrag;

	if ( ! dcAll.CreateCompatibleDC( &dcClient ) )
		return NULL;
	if ( ! bmAll.CreateCompatibleBitmap( &dcClient, rcClient.Width(), rcClient.Height() ) )
		return NULL;

	if ( ! dcDrag.CreateCompatibleDC( &dcClient ) )
		return NULL;
	if ( ! bmDrag.CreateCompatibleBitmap( &dcClient, rcAll.Width(), rcAll.Height() ) )
		return NULL;

	CBitmap *pOldAll = dcAll.SelectObject( &bmAll );

	dcAll.FillSolidRect( &rcClient, DRAG_COLOR_KEY );

	COLORREF crBack = pList->GetBkColor();
	pList->SetBkColor( DRAG_COLOR_KEY );
	pList->SendMessage( WM_PAINT, (WPARAM)dcAll.GetSafeHdc() );
	pList->SetBkColor( crBack );

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

	for ( nIndex = -1 ; ( nIndex = pList->GetNextItem( nIndex, LVNI_SELECTED ) ) >= 0 ; )
	{
		pList->GetItemRect( nIndex, rcOne, LVIR_BOUNDS );

		if ( rcOne.IntersectRect( &rcAll, &rcOne ) )
		{
			dcDrag.BitBlt( rcOne.left - rcAll.left, rcOne.top - rcAll.top,
				rcOne.Width(), rcOne.Height(), &dcAll, rcOne.left, rcOne.top, SRCCOPY );
		}
	}

	dcDrag.SelectObject( pOldDrag );
	dcAll.SelectObject( pOldAll );

	dcDrag.DeleteDC();
	bmAll.DeleteObject();
	dcAll.DeleteDC();

	return (HBITMAP) bmDrag.Detach ();
}

CImageList* CLiveList::CreateDragImage(CListCtrl* pList, const CPoint& ptMouse)
{
	ASSERT_VALID( pList );

	CPoint ptOffset( 0, 0 );
	CBitmap bmDrag;
	bmDrag.Attach( CreateDragImage( pList, ptMouse, ptOffset) );
	BITMAP bmpInfo;
	bmDrag.GetBitmap( &bmpInfo );
	CImageList* pAll = new CImageList();
	pAll->Create( bmpInfo.bmWidth, bmpInfo.bmHeight, ILC_COLOR32|ILC_MASK, 1, 1 ) ||
	pAll->Create( bmpInfo.bmWidth, bmpInfo.bmHeight, ILC_COLOR24|ILC_MASK, 1, 1 ) ||
	pAll->Create( bmpInfo.bmWidth, bmpInfo.bmHeight, ILC_COLOR16|ILC_MASK, 1, 1 );
	pAll->Add( &bmDrag, DRAG_COLOR_KEY );
	bmDrag.DeleteObject();
	pAll->BeginDrag( 0, ptOffset );

	return pAll;
}
