//
// CtrlDownloads.cpp
//
//	Date:			"$Date: 2005/04/06 14:43:45 $"
//	Revision:		"$Revision: 1.30 $"
//  Last change by:	"$Author: rolandas $"
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
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "FragmentBar.h"
#include "Skin.h"
#include "CtrlDownloads.h"
#include "WndDownloads.h"

IMPLEMENT_DYNAMIC(CDownloadsCtrl, CWnd)

BEGIN_MESSAGE_MAP(CDownloadsCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_NOTIFY(HDN_ITEMCLICKA, AFX_IDW_PANE_FIRST, OnSortPanelItems)
	ON_NOTIFY(HDN_ITEMCLICKW, AFX_IDW_PANE_FIRST, OnSortPanelItems)
	ON_NOTIFY(HDN_ITEMCHANGEDW, AFX_IDW_PANE_FIRST, OnChangeHeader)
	ON_NOTIFY(HDN_ITEMCHANGEDA, AFX_IDW_PANE_FIRST, OnChangeHeader)
	ON_NOTIFY(HDN_ENDDRAG, AFX_IDW_PANE_FIRST, OnChangeHeader)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

#define HEADER_HEIGHT				20
#define ITEM_HEIGHT					17

#define DOWNLOAD_COLUMN_TITLE		0
#define DOWNLOAD_COLUMN_SIZE		1
#define DOWNLOAD_COLUMN_PROGRESS	2	
#define DOWNLOAD_COLUMN_SPEED		3
#define DOWNLOAD_COLUMN_STATUS		4
#define DOWNLOAD_COLUMN_CLIENT		5
#define DOWNLOAD_COLUMN_DOWNLOADED	6
#define DOWNLOAD_COLUMN_PERCENTAGE  7
#define COLUMNS_TO_SORT				DOWNLOAD_COLUMN_PERCENTAGE - DOWNLOAD_COLUMN_TITLE

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl construction

CDownloadsCtrl::CDownloadsCtrl()
{
}

CDownloadsCtrl::~CDownloadsCtrl()
{
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl operations

BOOL CDownloadsCtrl::Create(CWnd* pParentWnd, UINT nID)
{
	CRect rect( 0, 0, 0, 0 );
	return CWnd::Create( NULL, NULL, WS_CHILD|WS_CLIPSIBLINGS, rect, pParentWnd, nID, NULL );
}

BOOL CDownloadsCtrl::Update()
{
	OnSize( 1982, 0, 0 );
	return TRUE;
}

BOOL CDownloadsCtrl::Update(int nGroupCookie)
{
	m_nGroupCookie = nGroupCookie;
	OnSize( 1982, 0, 0 );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl system message handlers

int CDownloadsCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	CRect rect( 0, 0, 0, 0 );

	if ( Settings.Downloads.SortColumns )
		m_wndHeader.Create( WS_CHILD|HDS_BUTTONS|HDS_DRAGDROP|HDS_HOTTRACK|HDS_FULLDRAG, rect, this, AFX_IDW_PANE_FIRST );
	else
        m_wndHeader.Create( WS_CHILD|HDS_DRAGDROP|HDS_HOTTRACK|HDS_FULLDRAG, rect, this, AFX_IDW_PANE_FIRST );
	
	m_wndHeader.SetFont( &theApp.m_gdiFont );
	
	m_wndTip.Create( this, &Settings.Interface.TipDownloads );
	
	InsertColumn( DOWNLOAD_COLUMN_TITLE, _T("Downloaded File"), LVCFMT_LEFT, 210 );
	InsertColumn( DOWNLOAD_COLUMN_SIZE, _T("Size"), LVCFMT_CENTER, 80 );
	InsertColumn( DOWNLOAD_COLUMN_PROGRESS, _T("Progress"), LVCFMT_CENTER, 130 );
	InsertColumn( DOWNLOAD_COLUMN_SPEED, _T("Speed"), LVCFMT_CENTER, 80 );
	InsertColumn( DOWNLOAD_COLUMN_STATUS, _T("Status"), LVCFMT_CENTER, 80 );
	InsertColumn( DOWNLOAD_COLUMN_CLIENT, _T("Client"), LVCFMT_CENTER, 80 );
	InsertColumn( DOWNLOAD_COLUMN_DOWNLOADED, _T("Downloaded"), LVCFMT_CENTER, 0 );
	InsertColumn( DOWNLOAD_COLUMN_PERCENTAGE, _T("Complete"), LVCFMT_CENTER, 60 );
	
	LoadColumnState();
	
	CBitmap bmImages;
	bmImages.LoadBitmap( IDB_PROTOCOLS );
	m_pProtocols.Create( 16, 16, ILC_COLOR16|ILC_MASK, 7, 1 );
	m_pProtocols.Add( &bmImages, RGB( 0, 255, 0 ) );
	
	m_nGroupCookie		= 0;
	m_nFocus			= 0;
	m_bCreateDragImage	= FALSE;
	m_pDragDrop			= NULL;
	m_bDrag				= FALSE;
	m_pDeselect1		= NULL;
	m_pDeselect2		= NULL;
	
	m_pbSortAscending	= new BOOL[COLUMNS_TO_SORT + 1];
	for (int i=DOWNLOAD_COLUMN_TITLE; i <= DOWNLOAD_COLUMN_PERCENTAGE; i++)
		m_pbSortAscending[i]=TRUE;

	return 0;
}

void CDownloadsCtrl::OnDestroy()
{
	delete[] m_pbSortAscending;
	SaveColumnState();
	CWnd::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl column helpers

void CDownloadsCtrl::InsertColumn(int nColumn, LPCTSTR pszCaption, int nFormat, int nWidth)
{
	HDITEM pColumn;
	
	ZeroMemory( &pColumn, sizeof(pColumn) );
	
	pColumn.mask	= HDI_FORMAT | HDI_LPARAM | HDI_TEXT | HDI_WIDTH;
	pColumn.cxy		= nWidth;
	pColumn.pszText	= (LPTSTR)pszCaption;
	pColumn.fmt		= nFormat;
	pColumn.lParam	= nColumn;
	
	m_wndHeader.InsertItem( m_wndHeader.GetItemCount(), &pColumn );
}

void CDownloadsCtrl::SaveColumnState()
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
	
	theApp.WriteProfileString( _T("ListStates"), _T("CDownloadCtrl.Ordering"), strOrdering );
	theApp.WriteProfileString( _T("ListStates"), _T("CDownloadCtrl.Widths"), strWidths );
}

BOOL CDownloadsCtrl::LoadColumnState()
{
	CString strOrdering, strWidths, strItem;
	
	strOrdering = theApp.GetProfileString( _T("ListStates"), _T("CDownloadCtrl.Ordering"), _T("") );
	strWidths = theApp.GetProfileString( _T("ListStates"), _T("CDownloadCtrl.Widths"), _T("") );
	
	HDITEM pItem = { HDI_WIDTH|HDI_ORDER };
	
	if ( _tcsncmp( strWidths, _T("0000"), 4 ) == 0 &&
		 _tcsncmp( strOrdering, _T("00"), 2 ) == 0 )
	{
		strWidths = strWidths.Mid( 4 );
		strOrdering = strOrdering.Mid( 2 );
	}
	
	for ( int nColumns = 0 ; m_wndHeader.GetItem( nColumns, &pItem ) ; nColumns++ )
	{
		if ( strWidths.GetLength() < 4 || strOrdering.GetLength() < 2 ) return FALSE;
		
		_stscanf( strWidths.Left( 4 ), _T("%x"), &pItem.cxy );
		_stscanf( strOrdering.Left( 2 ), _T("%x"), &pItem.iOrder );
		
		strWidths = strWidths.Mid( 4 );
		strOrdering = strOrdering.Mid( 2 );
		
		m_wndHeader.SetItem( nColumns, &pItem );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl item helpers

BOOL CDownloadsCtrl::IsFiltered(CDownload* pDownload)
{
	DWORD nFilterMask = Settings.Downloads.FilterMask;
	if ( Settings.General.GUIMode == GUI_BASIC ) nFilterMask = 0xFFFFFFFF;

	if ( pDownload->IsMoving() )
	{
		return ( ( nFilterMask & DLF_ACTIVE ) == 0 );
	}
	else if ( pDownload->IsPaused() )
	{
		return ( ( nFilterMask & DLF_PAUSED ) == 0 );
	}
	else if ( pDownload->IsDownloading() )
	{
		return ( ( nFilterMask & DLF_ACTIVE ) == 0 );
	}
	else if ( pDownload->GetSourceCount() > 0 )
	{
		if ( pDownload->IsDownloading() )
		{
			return ( ( nFilterMask & DLF_ACTIVE ) == 0 );
		}
		else
		{
			return ( ( nFilterMask & DLF_QUEUED ) == 0 );
		}
	}
	else if ( pDownload->m_nSize == SIZE_UNKNOWN )
	{
		return ( ( nFilterMask & DLF_SOURCES ) == 0 );
	}
	else
	{
		return ( ( nFilterMask & DLF_SOURCES ) == 0 );
	}
	
	return FALSE;
}

BOOL CDownloadsCtrl::IsExpandable(CDownload* pDownload)
{
	if ( Settings.Downloads.ShowSources )
	{
		return ( pDownload->GetSourceCount() > 0 );
	}
	else
	{
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
		{
			if ( pSource->m_pTransfer != NULL && pSource->m_pTransfer->m_nState > dtsConnecting )
			{
				return TRUE;
			}
		}
		
		return FALSE;
	}
}

void CDownloadsCtrl::SelectTo(int nIndex)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	BOOL bShift		= GetAsyncKeyState( VK_SHIFT ) & 0x8000;
	BOOL bControl	= GetAsyncKeyState( VK_CONTROL ) & 0x8000;
	BOOL bRight		= GetAsyncKeyState( VK_RBUTTON ) & 0x8000;
	
	if ( ! bShift && ! bControl && ! bRight )
	{
		if ( m_pDeselect1 == NULL && m_pDeselect2 == NULL ) DeselectAll();
	}
	
	Update();
	
	INT nMin, nMax;
	GetScrollRange( SB_VERT, &nMin, &nMax );
	nIndex = max( 0, min( nIndex, nMax - 1 ) );
	
	CDownloadSource* pSource;
	CDownload* pDownload;
	
	if ( bShift )
	{
		if ( m_nFocus < nIndex )
		{
			for ( m_nFocus ++ ; m_nFocus <= nIndex ; m_nFocus ++ )
			{
				GetAt( m_nFocus, &pDownload, &pSource );
				if ( pDownload != NULL ) pDownload->m_bSelected = TRUE;
				if ( pSource != NULL ) pSource->m_bSelected = TRUE;
			}
		}
		else if ( m_nFocus > nIndex )
		{
			for ( m_nFocus -- ; m_nFocus >= nIndex ; m_nFocus -- )
			{
				GetAt( m_nFocus, &pDownload, &pSource );
				if ( pDownload != NULL ) pDownload->m_bSelected = TRUE;
				if ( pSource != NULL ) pSource->m_bSelected = TRUE;
			}
		}
		
		m_nFocus = nIndex;
	}
	else
	{
		m_nFocus = nIndex;
		GetAt( m_nFocus, &pDownload, &pSource );
		
		if ( bControl )
		{
			if ( pDownload != NULL ) pDownload->m_bSelected = ! pDownload->m_bSelected;
			if ( pSource != NULL ) pSource->m_bSelected = ! pSource->m_bSelected;
		}
		else
		{
			if ( pDownload != NULL ) pDownload->m_bSelected = TRUE;
			if ( pSource != NULL ) pSource->m_bSelected = TRUE;
		}
	}
	
	CRect rcClient;
	GetClientRect( &rcClient );
	
	int nScroll = GetScrollPos( SB_VERT );
	int nHeight = ( rcClient.bottom - HEADER_HEIGHT ) / ITEM_HEIGHT - 1;
	nHeight = max( 0, nHeight );
	
	if ( m_nFocus < nScroll )
	{
		SetScrollPos( SB_VERT, m_nFocus );
		Update();
	}
	else if ( m_nFocus > nScroll + nHeight )
	{
		SetScrollPos( SB_VERT, max( 0, m_nFocus - nHeight ) );
		Update();
	}
	else
	{
		Invalidate();
	}
}

void CDownloadsCtrl::SelectAll(CDownload* pDownload, CDownloadSource* pSource)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	BOOL bSelected = FALSE;

	for ( POSITION pos = Downloads.GetIterator() ; pos != NULL ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		// If a download is selected, select all downloads
		if ( pDownload != NULL && pDownload->m_bSelected )
		{
			for ( POSITION pos2 = Downloads.GetIterator() ; pos2 != NULL ; )
			{
				CDownload* pDownload = Downloads.GetNext( pos2 );

				if ( pDownload != NULL ) pDownload->m_bSelected = TRUE;
			}

			bSelected = TRUE;
		}

		// If a source is selected, select all sources for that download
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource != NULL ; pSource = pSource->m_pNext )
		{
			if ( pSource != NULL && pSource->m_bSelected )
			{
				for ( CDownloadSource* pSource2 = pDownload->GetFirstSource() ; pSource2 != NULL ; pSource2 = pSource2->m_pNext )
				{
					if ( pSource2 != NULL ) pSource2->m_bSelected = TRUE;
				}

				bSelected = TRUE;
				break;
			}
		}
	}

	// If nothing is selected, select all downloads
	if ( bSelected != TRUE )
	{
		for ( POSITION pos = Downloads.GetIterator() ; pos != NULL ; )
		{
			CDownload* pDownload = Downloads.GetNext( pos );
			
			if ( pDownload != NULL ) pDownload->m_bSelected = TRUE;
		}
	}

	Invalidate();
}

void CDownloadsCtrl::DeselectAll(CDownload* pExcept1, CDownloadSource* pExcept2)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = Downloads.GetIterator() ; pos != NULL ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		if ( pDownload != pExcept1 ) pDownload->m_bSelected = FALSE;
		
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource != NULL ; pSource = pSource->m_pNext )
		{
			if ( pSource != pExcept2 ) pSource->m_bSelected = FALSE;
		}
	}
	
	Invalidate();
}

int CDownloadsCtrl::GetSelectedCount()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	int nCount = 0;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) nCount ++;
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
			if ( pSource->m_bSelected ) nCount ++;
	}
	
	pLock.Unlock();
	
	return nCount;
}

BOOL CDownloadsCtrl::HitTest(const CPoint& point, CDownload** ppDownload, CDownloadSource** ppSource, int* pnIndex, RECT* prcItem)
{
	CRect rcClient, rcItem;
	
	GetClientRect( &rcClient );
	rcClient.top += HEADER_HEIGHT;
	
	rcItem.CopyRect( &rcClient );
	rcItem.left -= GetScrollPos( SB_HORZ );
	rcItem.bottom = rcItem.top + ITEM_HEIGHT;
	
	int nScroll = GetScrollPos( SB_VERT );
	int nIndex = 0;
	
	if ( ppDownload != NULL ) *ppDownload = NULL;
	if ( ppSource != NULL ) *ppSource = NULL;
	
	for ( POSITION posDownload = Downloads.GetIterator() ; posDownload && rcItem.top < rcClient.bottom ; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );
		
		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie ) continue;
		if ( IsFiltered( pDownload ) ) continue;
		
		if ( nScroll > 0 )
		{
			nScroll --;
		}
		else
		{
			if ( rcItem.PtInRect( point ) )
			{
				if ( ppDownload != NULL ) *ppDownload = pDownload;
				if ( pnIndex != NULL ) *pnIndex = nIndex;
				if ( prcItem != NULL ) *prcItem = rcItem;
				return TRUE;
			}
			rcItem.OffsetRect( 0, ITEM_HEIGHT );
		}
		
		nIndex ++;
		if ( ! pDownload->m_bExpanded ) continue;
		
		if ( Settings.Downloads.ShowSources )
		{
			int nSources = pDownload->GetSourceCount();
			
			if ( nScroll >= nSources )
			{
				nScroll -= nSources;
				nIndex += nSources;
				continue;
			}
		}
		
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
		{
			if ( Settings.Downloads.ShowSources || ( pSource->m_pTransfer != NULL && pSource->m_pTransfer->m_nState > dtsConnecting ) )
			{
				if ( nScroll > 0 )
				{
					nScroll --;
				}
				else
				{
					if ( rcItem.PtInRect( point ) )
					{
						if ( ppSource != NULL ) *ppSource = pSource;
						if ( pnIndex != NULL ) *pnIndex = nIndex;
						if ( prcItem != NULL ) *prcItem = rcItem;
						return TRUE;
					}
					rcItem.OffsetRect( 0, ITEM_HEIGHT );
				}
				
				nIndex ++;
			}
		}
	}
	
	return FALSE;
}

BOOL CDownloadsCtrl::GetAt(int nSelect, CDownload** ppDownload, CDownloadSource** ppSource)
{
	int nScroll = GetScrollPos( SB_VERT );
	int nIndex = 0;
	
	if ( ppDownload != NULL ) *ppDownload = NULL;
	if ( ppSource != NULL ) *ppSource = NULL;
	
	for ( POSITION posDownload = Downloads.GetIterator() ; posDownload ; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );
		
		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie ) continue;
		if ( IsFiltered( pDownload ) ) continue;
		
		if ( nIndex++ == nSelect )
		{
			if ( ppDownload != NULL ) *ppDownload = pDownload;
			return TRUE;
		}
		
		if ( ! pDownload->m_bExpanded ) continue;
		
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
		{
			if ( Settings.Downloads.ShowSources || ( pSource->m_pTransfer != NULL && pSource->m_pTransfer->m_nState > dtsConnecting ) )
			{
				if ( nIndex++ == nSelect )
				{
					if ( ppSource != NULL ) *ppSource = pSource;
					return TRUE;
				}
			}
		}
	}
	
	return FALSE;
}

BOOL CDownloadsCtrl::GetRect(CDownload* pSelect, RECT* prcItem)
{
	CRect rcClient, rcItem;
	
	GetClientRect( &rcClient );
	rcClient.top += HEADER_HEIGHT;
	
	rcItem.CopyRect( &rcClient );
	rcItem.left -= GetScrollPos( SB_HORZ );
	rcItem.bottom = rcItem.top + ITEM_HEIGHT;
	
	int nScroll = GetScrollPos( SB_VERT );
	rcItem.OffsetRect( 0, ITEM_HEIGHT * -nScroll );
	
	for ( POSITION posDownload = Downloads.GetIterator() ; posDownload ; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );
		
		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie ) continue;
		if ( IsFiltered( pDownload ) ) continue;
		
		if ( pDownload == pSelect )
		{
			*prcItem = rcItem;
			return TRUE;
		}
		
		rcItem.OffsetRect( 0, ITEM_HEIGHT );
		
		if ( ! pDownload->m_bExpanded ) continue;
		
		if ( Settings.Downloads.ShowSources )
		{
			int nSources = pDownload->GetSourceCount();
			rcItem.OffsetRect( 0, ITEM_HEIGHT * nSources );
			continue;
		}
		
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
		{
			if ( pSource->m_pTransfer != NULL && pSource->m_pTransfer->m_nState > dtsConnecting )
			{
				rcItem.OffsetRect( 0, ITEM_HEIGHT );
			}
		}
	}
	
	return FALSE;
}

void CDownloadsCtrl::MoveSelected(int nDelta)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CList<CDownload*> pList;
	POSITION pos;
	
	for ( pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) pList.AddTail( pDownload );
	}
	
	pos = nDelta > 0 ? pList.GetTailPosition() : pList.GetHeadPosition();
	
	while ( pos )
	{
		CDownload* pDownload = nDelta > 0 ? pList.GetPrev( pos ) : pList.GetNext( pos );
		Downloads.Move( pDownload, nDelta );
	}
	
	Update();
}

BOOL CDownloadsCtrl::DropShowTarget(CPtrList* pSel, const CPoint& ptScreen)
{
	CPoint ptLocal( ptScreen );
	CRect rcClient;
	
	ScreenToClient( &ptLocal );
	GetClientRect( &rcClient );
	
	BOOL bLocal = rcClient.PtInRect( ptLocal );
	CDownload* pHit = NULL;
	
	if ( bLocal ) HitTest( ptLocal, &pHit, NULL, NULL, NULL );
	
	if ( pHit != m_pDragDrop )
	{
		CImageList::DragShowNolock( FALSE );
		m_pDragDrop = pHit;
		RedrawWindow();
		CImageList::DragShowNolock( TRUE );
	}
	
	return bLocal;
}

BOOL CDownloadsCtrl::DropObjects(CPtrList* pSel, const CPoint& ptScreen)
{
	CPoint ptLocal( ptScreen );
	CRect rcClient;
	
	ScreenToClient( &ptLocal );
	GetClientRect( &rcClient );
	
	m_pDragDrop = NULL;
	
	if ( pSel == NULL || ! rcClient.PtInRect( ptLocal ) ) return FALSE;
	
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownload* pHit = NULL;
	
	HitTest( ptLocal, &pHit, NULL, NULL, NULL );
	
	for ( POSITION pos = pSel->GetHeadPosition() ; pos ; )
	{
		CDownload* pDownload = (CDownload*)pSel->GetNext( pos );
		
		if ( Downloads.Check( pDownload ) && pDownload != pHit )
		{
			Downloads.Reorder( pDownload, pHit );
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl presentation message handlers

void CDownloadsCtrl::OnSize(UINT nType, int cx, int cy)
{
	int nWidth = 0, nHeight = 0;
	CRect rcClient;
	HDITEM pColumn;
	
	if ( nType != 1982 ) CWnd::OnSize( nType, cx, cy );
	
	GetClientRect( &rcClient );
	
	ZeroMemory( &pColumn, sizeof(pColumn) );
	pColumn.mask = HDI_WIDTH;
	
	for ( int nColumn = 0 ; m_wndHeader.GetItem( nColumn, &pColumn ) ; nColumn ++ )
		nWidth += pColumn.cxy;
	
	SCROLLINFO pScroll;
	ZeroMemory( &pScroll, sizeof(pScroll) );
	pScroll.cbSize	= sizeof(pScroll);
	pScroll.fMask	= SIF_RANGE|SIF_PAGE;
	pScroll.nMin	= 0;
	pScroll.nMax	= nWidth;
	pScroll.nPage	= rcClient.right;
	SetScrollInfo( SB_HORZ, &pScroll, TRUE );
	
	int nScroll = GetScrollPos( SB_HORZ );
	m_wndHeader.SetWindowPos( NULL, -nScroll, 0, rcClient.right + nScroll, HEADER_HEIGHT, SWP_SHOWWINDOW );
	
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION posDownload = Downloads.GetIterator() ; posDownload ; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );
		
		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie || IsFiltered( pDownload ) )
		{
			pDownload->m_bSelected = FALSE;
			for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
				pSource->m_bSelected = FALSE;
			continue;
		}
		
		nHeight ++;
		
		if ( ! pDownload->m_bExpanded )
		{
		}
		else if ( Settings.Downloads.ShowSources )
		{
			nHeight += pDownload->GetSourceCount();
		}
		else
		{
			for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
			{
				if ( Settings.Downloads.ShowSources || ( pSource->m_pTransfer != NULL && pSource->m_pTransfer->m_nState > dtsConnecting ) )
				{
					nHeight ++;
				}
			}
		}
	}
	
	pLock.Unlock();
	
	ZeroMemory( &pScroll, sizeof(pScroll) );
	pScroll.cbSize	= sizeof(pScroll);
	pScroll.fMask	= SIF_RANGE|SIF_PAGE;
	pScroll.nMin	= 0;
	pScroll.nMax	= nHeight;
	pScroll.nPage	= ( rcClient.bottom - HEADER_HEIGHT ) / ITEM_HEIGHT + 1;
	SetScrollInfo( SB_VERT, &pScroll, TRUE );
	
	m_nFocus = min( m_nFocus, max( 0, nHeight - 1 ) );
	
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl painting

void CDownloadsCtrl::OnPaint()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CRect rcClient, rcItem;
	CPaintDC dc( this );
	
	GetClientRect( &rcClient );
	rcClient.top += HEADER_HEIGHT;
	
	rcItem.CopyRect( &rcClient );
	rcItem.left -= GetScrollPos( SB_HORZ );
	rcItem.bottom = rcItem.top + ITEM_HEIGHT;
	
	int nScroll = GetScrollPos( SB_VERT );
	int nIndex = 0;
	
	CFont* pfOld	= (CFont*)dc.SelectObject( &theApp.m_gdiFont );
	BOOL bFocus		= ( GetFocus() == this );
	
	for ( POSITION posDownload = Downloads.GetIterator() ; posDownload && rcItem.top < rcClient.bottom ; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );
		
		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie ) continue;
		if ( IsFiltered( pDownload ) ) continue;
		
		if ( nScroll > 0 )
		{
			nScroll --;
		}
		else
		{
			PaintDownload( dc, rcItem, pDownload, bFocus && ( m_nFocus == nIndex ), m_pDragDrop == pDownload );
			rcItem.OffsetRect( 0, ITEM_HEIGHT );
		}
		
		nIndex ++;
		
		if ( ! pDownload->m_bExpanded ) continue;
		
		if ( Settings.Downloads.ShowSources )
		{
			int nSources = pDownload->GetSourceCount();
			
			if ( nScroll >= nSources )
			{
				nScroll -= nSources;
				nIndex += nSources;
				continue;
			}
		}
		
		for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource && rcItem.top < rcClient.bottom ; pSource = pSource->m_pNext )
		{
			if ( Settings.Downloads.ShowSources || ( pSource->m_pTransfer != NULL && pSource->m_pTransfer->m_nState > dtsConnecting ) )
			{
				if ( nScroll > 0 )
				{
					nScroll --;
				}
				else
				{
					PaintSource( dc, rcItem, pDownload, pSource, bFocus && ( m_nFocus == nIndex ) );
					rcItem.OffsetRect( 0, ITEM_HEIGHT );
				}
				
				nIndex ++;
			}
		}
	}
	
	dc.SelectObject( pfOld );
	
	rcClient.top = rcItem.top;
	if ( rcClient.top < rcClient.bottom )
		dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );
}

void CDownloadsCtrl::PaintDownload(CDC& dc, const CRect& rcRow, CDownload* pDownload, BOOL bFocus, BOOL bDrop)
{
	COLORREF crNatural	= m_bCreateDragImage ? RGB( 250, 255, 250 ) : CoolInterface.m_crWindow;
	COLORREF crBack		= pDownload->m_bSelected ? CoolInterface.m_crBackSel : crNatural;
	
	if ( bDrop )
	{
		CRect rcDrop( rcRow.left, rcRow.top, rcRow.right, rcRow.top + 2 );
		dc.Draw3dRect( &rcDrop, 0, 0 );
		dc.ExcludeClipRect( &rcDrop );
	}
	
	dc.SetBkColor( crBack );
	dc.SetBkMode( OPAQUE );
	
	if ( pDownload->m_bVerify == TS_FALSE )
		dc.SetTextColor( RGB( 255, 0, 0 ) );
	else if ( pDownload->m_bSelected )
		dc.SetTextColor( CoolInterface.m_crText );
	else if ( pDownload->m_bVerify == TS_TRUE )
	{
		if( pDownload->m_bBTH && ( pDownload->m_nTorrentUploaded < pDownload->m_nTorrentDownloaded ) )
			dc.SetTextColor( CoolInterface.m_crText );
		else
			dc.SetTextColor( RGB( 0, 127, 0 ) );
	}
	else
		dc.SetTextColor( CoolInterface.m_crText );
	
	int nTextLeft = rcRow.right, nTextRight = rcRow.left;
	HDITEM pColumn;
	
	ZeroMemory( &pColumn, sizeof(pColumn) );
	pColumn.mask = HDI_FORMAT | HDI_LPARAM;
	
	int nTransfers	= pDownload->GetTransferCount();
	int nSources	= pDownload->GetSourceCount();
	
	for ( int nColumn = 0 ; m_wndHeader.GetItem( nColumn, &pColumn ) ; nColumn++ )
	{
		CString strText;
		CRect rcCell;
		CString strSource;
		
		m_wndHeader.GetItemRect( nColumn, &rcCell );
		rcCell.left		+= rcRow.left;
		rcCell.right	+= rcRow.left;
		rcCell.top		= rcRow.top;
		rcCell.bottom	= rcRow.bottom;
		
		switch ( pColumn.lParam )
		{
		case DOWNLOAD_COLUMN_TITLE:
			dc.FillSolidRect( rcCell.left, rcCell.bottom - 1, 32, 1, crNatural );
			if ( IsExpandable( pDownload ) )
			{
				ImageList_DrawEx( ShellIcons.GetHandle( 16 ), pDownload->m_bExpanded ? SHI_MINUS : SHI_PLUS, dc.GetSafeHdc(),
						rcCell.left, rcCell.top, 16, 16, crNatural, CLR_DEFAULT, ILD_NORMAL );
			}
			else
				dc.FillSolidRect( rcCell.left, rcCell.top, 16, 16, crNatural );
			rcCell.left += 16;
			ImageList_DrawEx( ShellIcons.GetHandle( 16 ), ShellIcons.Get( pDownload->m_sRemoteName, 16 ), dc.GetSafeHdc(),
					rcCell.left, rcCell.top, 16, 16, crNatural, CLR_DEFAULT, pDownload->m_bSelected ? ILD_SELECTED : ILD_NORMAL );
			rcCell.left += 16;
			dc.FillSolidRect( rcCell.left, rcCell.top, 1, rcCell.Height(), crNatural );
			rcCell.left += 1;
			
			strText = pDownload->GetDisplayName();
			break;
			
		case DOWNLOAD_COLUMN_SIZE:
			if ( pDownload->m_nSize < SIZE_UNKNOWN )
				strText = Settings.SmartVolume( pDownload->m_nSize, FALSE );
			else
				LoadString( strText, IDS_STATUS_UNKNOWN );
			break;
			
		case DOWNLOAD_COLUMN_PROGRESS:
			dc.Draw3dRect( &rcCell, crBack, crBack );
			rcCell.DeflateRect( 1, 1 );
			dc.Draw3dRect( &rcCell, crBack, crBack );
			rcCell.DeflateRect( 0, 1 );
			dc.Draw3dRect( &rcCell, RGB( 50, 50, 50 ), RGB( 50, 50, 50 ) );
			rcCell.DeflateRect( 1, 1 );
			CFragmentBar::DrawDownload( &dc, &rcCell, pDownload, crNatural );
			break;
			
		case DOWNLOAD_COLUMN_SPEED:
			if ( ! pDownload->IsMoving() )
			{
				if ( DWORD nSpeed = pDownload->GetAverageSpeed() * 8 )
					strText = Settings.SmartVolume( nSpeed, FALSE, TRUE );
			}
			break;
			
		case DOWNLOAD_COLUMN_STATUS:
			strText = GetDownloadStatus( pDownload );
			break;
			
		case DOWNLOAD_COLUMN_CLIENT:
			if ( pDownload->IsCompleted() )
			{
				if ( pDownload->m_bVerify == TS_TRUE )
					LoadString( strText, IDS_STATUS_VERIFIED );
				else if ( pDownload->m_bVerify == TS_FALSE )
					LoadString( strText, IDS_STATUS_UNVERIFIED );
			}
			else if ( nSources > 0 )
			{
				LoadSourcesString( strSource,  nSources );
				strText.Format( _T("(%i %s)"), nSources, strSource );
			}
			else
				LoadString( strText, IDS_STATUS_NOSOURCES );
			break;
		case DOWNLOAD_COLUMN_DOWNLOADED:
			strText = Settings.SmartVolume( pDownload->GetVolumeComplete(), FALSE );
			break;
		case DOWNLOAD_COLUMN_PERCENTAGE:
			if ( ( pDownload->m_nSize < SIZE_UNKNOWN ) && ( pDownload->m_nSize > 0 ) )
			{
				strText.Format( _T("%.2f%%"), pDownload->GetProgress() * 100.0 );
			}
			else
				LoadString( strText, IDS_STATUS_UNKNOWN );
			break;
		}
		
		nTextLeft	= min( nTextLeft, int(rcCell.left) );
		nTextRight	= max( nTextRight, int(rcCell.right) );
		
		if ( pColumn.lParam == DOWNLOAD_COLUMN_PROGRESS ) continue;
		
		if ( rcCell.Width() < 8 ) strText.Empty();
		
		if ( dc.GetTextExtent( strText ).cx > rcCell.Width() - 8 )
		{
			while ( dc.GetTextExtent( strText + _T('\x2026') ).cx > ( rcCell.Width() - 8 ) && strText.GetLength() > 0 )
			{
				strText.Truncate( strText.GetLength() - 1 );
			}
			
			if ( strText.GetLength() > 0 ) strText += _T('\x2026');
		}
		
		int nWidth		= dc.GetTextExtent( strText ).cx;
		int nPosition	= 0;
		
		switch ( pColumn.fmt & LVCFMT_JUSTIFYMASK )
		{
		default:
			nPosition = ( rcCell.left + 4 );
			break;
		case LVCFMT_CENTER:
			nPosition = ( ( rcCell.left + rcCell.right ) / 2 ) - ( nWidth / 2 );
			break;
		case LVCFMT_RIGHT:
			nPosition = ( rcCell.right - 4 - nWidth );
			break;
		}
		
		dc.SetBkColor( crBack );
		dc.ExtTextOut( nPosition, rcCell.top + 2, ETO_CLIPPED|ETO_OPAQUE,
			&rcCell, strText, NULL );
	}
	
	if ( nTextRight < rcRow.right )
	{
		CRect rcBlank( nTextRight, rcRow.top, rcRow.right, rcRow.bottom );
		dc.FillSolidRect( &rcBlank, crBack );
	}
	
	if ( bFocus )
	{
		CRect rcFocus( nTextLeft, rcRow.top, max( int(rcRow.right), nTextRight ), rcRow.bottom );
		dc.Draw3dRect( &rcFocus, CoolInterface.m_crBorder, CoolInterface.m_crBorder );
	}
}

void CDownloadsCtrl::PaintSource(CDC& dc, const CRect& rcRow, CDownload* pDownload, CDownloadSource* pSource, BOOL bFocus)
{
	COLORREF crNatural	= m_bCreateDragImage ? RGB( 250, 255, 250 ) : CoolInterface.m_crWindow;
	COLORREF crBack		= pSource->m_bSelected ? CoolInterface.m_crBackSel : crNatural;
	
	dc.SetBkColor( crBack );
	dc.SetBkMode( OPAQUE );
	
	if ( pSource->m_bSelected )
		dc.SetTextColor( CoolInterface.m_crText );
	else
		dc.SetTextColor( CoolInterface.m_crText );
	
	int nTextLeft = rcRow.right, nTextRight = rcRow.left;
	HDITEM pColumn;
	
	ZeroMemory( &pColumn, sizeof(pColumn) );
	pColumn.mask = HDI_FORMAT | HDI_LPARAM;
	
	for ( int nColumn = 0 ; m_wndHeader.GetItem( nColumn, &pColumn ) ; nColumn++ )
	{
		CString strText;
		CRect rcCell;
		
		m_wndHeader.GetItemRect( nColumn, &rcCell );
		rcCell.left		+= rcRow.left;
		rcCell.right	+= rcRow.left;
		rcCell.top		= rcRow.top;
		rcCell.bottom	= rcRow.bottom;
		
		switch ( pColumn.lParam )
		{
		case DOWNLOAD_COLUMN_TITLE:
			dc.FillSolidRect( rcCell.left, rcCell.top, 24, rcCell.Height(), crNatural );
			rcCell.left += 24;
			dc.FillSolidRect( rcCell.left, rcCell.bottom - 1, 16, 1, crNatural );
			ImageList_DrawEx( m_pProtocols, pSource->m_nProtocol, dc.GetSafeHdc(),
					rcCell.left, rcCell.top, 16, 16, crNatural, CLR_DEFAULT, pSource->m_bSelected ? ILD_SELECTED : ILD_NORMAL );
			rcCell.left += 16;
			dc.FillSolidRect( rcCell.left, rcCell.top, 1, rcCell.Height(), crNatural );
			rcCell.left += 1;
			
			if ( pSource->m_pTransfer != NULL )
			{
				if ( Settings.Search.ShowNames && pSource->m_sNick.GetLength() )
					strText = pSource->m_sNick + _T(" (") + pSource->m_pTransfer->m_sAddress + _T(")");
				else
					strText = pSource->m_pTransfer->m_sAddress;
			}
			else
			{
				if ( Settings.Search.ShowNames && pSource->m_sNick.GetLength() )
					strText = pSource->m_sNick + _T(" (") + inet_ntoa( pSource->m_pAddress ) + _T(")");
				else
				{
					if( ( pSource->m_nProtocol == PROTOCOL_ED2K ) && ( pSource->m_bPushOnly == TRUE ) )
					{
						strText.Format( _T("%lu@%s"), pSource->m_pAddress.S_un.S_addr, 
							(LPCTSTR)CString( inet_ntoa( (IN_ADDR&)pSource->m_pServerAddress) ) );
					}
					else
					{
						strText = inet_ntoa( pSource->m_pAddress );
					}
				}
			}
			if( pSource->m_bPushOnly )
			{
				strText += _T(" (push)");
			}
			break;
			
		case DOWNLOAD_COLUMN_SIZE:
			if ( pSource->m_pTransfer != NULL )
				strText = Settings.SmartVolume( pSource->m_pTransfer->m_nDownloaded, FALSE );
			break;
			
		case DOWNLOAD_COLUMN_PROGRESS:
			dc.Draw3dRect( &rcCell, crBack, crBack );
			rcCell.DeflateRect( 1, 1 );
			dc.Draw3dRect( &rcCell, crBack, crBack );
			rcCell.DeflateRect( 0, 1 );
			dc.Draw3dRect( &rcCell, RGB( 50, 50, 50 ), RGB( 50, 50, 50 ) );
			rcCell.DeflateRect( 1, 1 );
			CFragmentBar::DrawSource( &dc, &rcCell, pSource, crNatural );
			break;
			
		case DOWNLOAD_COLUMN_SPEED:
			if ( pSource->m_pTransfer != NULL )
			{
				if ( DWORD nSpeed = pSource->m_pTransfer->GetMeasuredSpeed() * 8 )
					strText = Settings.SmartVolume( nSpeed, FALSE, TRUE );
			}
			break;
			
		case DOWNLOAD_COLUMN_STATUS:
			if ( pSource->m_pTransfer != NULL )
				strText = pSource->m_pTransfer->GetStateText( FALSE );
			else if ( pSource->m_tAttempt )
			{
				DWORD nTime = GetTickCount();
				
				if ( pSource->m_tAttempt >= nTime )
				{
					nTime = ( pSource->m_tAttempt - nTime ) / 1000;
					strText.Format( _T("%.2u:%.2u"), nTime / 60, nTime % 60 );
				}
			}
			break;
			
		case DOWNLOAD_COLUMN_CLIENT:
			strText = pSource->m_sServer;
			break;
		case DOWNLOAD_COLUMN_DOWNLOADED:
			if ( pSource->m_pTransfer != NULL )
				strText = Settings.SmartVolume( pSource->m_pTransfer->m_nDownloaded, FALSE );
			break;
		case DOWNLOAD_COLUMN_PERCENTAGE:
			if ( ( pDownload->m_nSize < SIZE_UNKNOWN ) && ( pDownload->m_nSize > 0 ) && ( pSource->m_pTransfer ) )
				strText.Format( _T("%i%%"), ((int) ( (double)( pSource->m_pTransfer->m_nDownloaded ) / (double)( pSource->m_pDownload->m_nSize ) * 100 )) );
			else
				strText = _T("-");
			break;

		}
		
		nTextLeft	= min( nTextLeft, int(rcCell.left) );
		nTextRight	= max( nTextRight, int(rcCell.right) );
		
		if ( pColumn.lParam == DOWNLOAD_COLUMN_PROGRESS ) continue;
		
		if ( rcCell.Width() < 8 ) strText.Empty();
		
		if ( dc.GetTextExtent( strText ).cx > rcCell.Width() - 8 )
		{
			while ( dc.GetTextExtent( strText + _T('\x2026') ).cx > ( rcCell.Width() - 8 ) && strText.GetLength() > 0 )
			{
				strText.Truncate( strText.GetLength() - 1 );
			}
			
			if ( strText.GetLength() > 0 ) strText += _T('\x2026');
		}
		
		int nWidth		= dc.GetTextExtent( strText ).cx;
		int nPosition	= 0;
		
		switch ( pColumn.fmt & LVCFMT_JUSTIFYMASK )
		{
		default:
			nPosition = ( rcCell.left + 4 );
			break;
		case LVCFMT_CENTER:
			nPosition = ( ( rcCell.left + rcCell.right ) / 2 ) - ( nWidth / 2 );
			break;
		case LVCFMT_RIGHT:
			nPosition = ( rcCell.right - 4 - nWidth );
			break;
		}
		
		dc.SetBkColor( crBack );
		dc.ExtTextOut( nPosition, rcCell.top + 2, ETO_CLIPPED|ETO_OPAQUE,
			&rcCell, strText, NULL );
	}
	
	if ( nTextRight < rcRow.right )
	{
		CRect rcBlank( nTextRight, rcRow.top, rcRow.right, rcRow.bottom );
		dc.FillSolidRect( &rcBlank, crBack );
	}
	
	if ( bFocus )
	{
		CRect rcFocus( nTextLeft, rcRow.top, max( int(rcRow.right), nTextRight ), rcRow.bottom );
		dc.Draw3dRect( &rcFocus, CoolInterface.m_crBorder, CoolInterface.m_crBorder );
	}
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl interaction message handlers

void CDownloadsCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO pInfo;
	
	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;
	
	GetScrollInfo( SB_VERT, &pInfo );
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
	
	SetScrollInfo( SB_VERT, &pInfo, TRUE );
	Invalidate();
}

void CDownloadsCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
	
	CRect rcClient;
	GetClientRect( &rcClient );
	
	m_wndHeader.SetWindowPos( NULL, -pInfo.nPos, 0,
		rcClient.right + pInfo.nPos, HEADER_HEIGHT, SWP_NOZORDER );
	
	RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW );
}

BOOL CDownloadsCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	OnVScroll( SB_THUMBPOSITION, (int)( GetScrollPos( SB_VERT ) - zDelta / WHEEL_DELTA * 3 ), NULL );
	return TRUE;
}

void CDownloadsCtrl::OnChangeHeader(NMHDR* pNotifyStruct, LRESULT* pResult)
{
	Update();
}

CString CDownloadsCtrl::GetDownloadStatus(CDownload *pDownload)
{
	CString strText;
	int nSources = pDownload->GetSourceCount();

	if ( pDownload->IsCompleted() )
		if( pDownload->IsSeeding() )
		{
			if ( pDownload->m_bTorrentTrackerError )
				LoadString( strText, IDS_STATUS_TRACKERDOWN );
			else
				LoadString( strText, IDS_STATUS_SEEDING );
		}
		else
			LoadString( strText, IDS_STATUS_COMPLETED );
	else if ( pDownload->IsPaused() )
	{
		if ( pDownload->m_bDiskFull )
			if( pDownload->IsMoving() )
				LoadString( strText, IDS_STATUS_CANTMOVE );
			else
				LoadString( strText, IDS_STATUS_FILEERROR );
		else
			LoadString( strText, IDS_STATUS_PAUSED );
	}
	else if ( pDownload->IsMoving() )
		LoadString( strText, IDS_STATUS_MOVING );
	else if ( pDownload->GetProgress() == 1.0f && pDownload->IsStarted() )
		LoadString( strText, IDS_STATUS_VERIFYING );
	else if ( pDownload->IsDownloading() )
	{
		DWORD nTime = pDownload->GetTimeRemaining();
	
		if ( nTime == 0xFFFFFFFF )
			LoadString( strText, IDS_STATUS_ACTIVE );
		else
		{
			if ( nTime > 86400 )
				strText.Format( _T("%i:%.2i:%.2i:%.2i"), nTime / 86400, ( nTime / 3600 ) % 24, ( nTime / 60 ) % 60, nTime % 60 );
			else
				strText.Format( _T("%i:%.2i:%.2i"), nTime / 3600, ( nTime / 60 ) % 60, nTime % 60 );
		}
	}
	else if ( ! pDownload->IsTrying() )
		LoadString( strText, IDS_STATUS_QUEUED );
	else if ( nSources > 0 )
		LoadString( strText, IDS_STATUS_PENDING );
	else if ( pDownload->m_bBTH )
	{
		if ( pDownload->IsTasking() )
			LoadString( strText, IDS_STATUS_CREATING );
		else if ( pDownload->m_bTorrentTrackerError )
			LoadString( strText, IDS_STATUS_TRACKERDOWN );
		else
			LoadString( strText, IDS_STATUS_TORRENT );
	}
	else
		LoadString( strText, IDS_STATUS_SEARCHING );

	return strText;
}

int CDownloadsCtrl::GetClientStatus(CDownload *pDownload)
{
	int nSources = pDownload->GetSourceCount();
			
	if ( pDownload->IsCompleted() )
		return -1;
	else if ( nSources >= 1 )
		return nSources;
	else
		return nSources;

	return nSources;
}

void CDownloadsCtrl::BubbleSortDownloads(int nColumn)
{
	POSITION pos;
	int nTransfers, j, pass;
	CDownload *x, *y;
	BOOL bSwitch=TRUE,  bSort, bOK;
	CString s, t;	

	bSort=m_pbSortAscending[nColumn];
	//for (j=DOWNLOAD_COLUMN_TITLE; j <= DOWNLOAD_COLUMN_PERCENTAGE; j++)
	//	m_pbSortAscending[j]=FALSE;
	m_pbSortAscending[nColumn]= !bSort;

	nTransfers = Downloads.GetCount();
	for (pass=0; (pass < nTransfers-1) && (bSwitch == TRUE); pass++)
	{
		bSwitch=FALSE;
		pos=Downloads.GetIterator();
		for (j=0 ; (j < nTransfers-pass-1) && (pos != NULL) ; j++ )
		{
			x = Downloads.GetNext( pos );
			y = Downloads.GetNext( pos );
			s = x->GetDisplayName();
			t = y->GetDisplayName();
			bOK = FALSE;
			if ( m_pbSortAscending[nColumn] == FALSE ) 
			{
				switch ( nColumn )
				{
					case DOWNLOAD_COLUMN_TITLE:
						if ( x->GetDisplayName().CompareNoCase( y->GetDisplayName() ) > 0 )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_SIZE:
						if ( x->m_nSize > y->m_nSize )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_PROGRESS:
						if ( x->GetProgress() > y->GetProgress() )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_SPEED:
						if ( x->GetMeasuredSpeed() > y->GetMeasuredSpeed() )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_STATUS:
						if ( GetDownloadStatus( x ).CompareNoCase(GetDownloadStatus(y)) > 0 )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_CLIENT:
						if ( GetClientStatus( x ) > GetClientStatus(y) )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_DOWNLOADED:
						if ( x->GetVolumeComplete() > y->GetVolumeComplete() )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_PERCENTAGE:
						if ( ((double)(x->GetVolumeComplete() ) / (double)(x->m_nSize)) > ((double)(y->GetVolumeComplete() ) / (double)(y->m_nSize)) )
							bOK = TRUE;
						break;
				}
				if (bOK)
				{
					bSwitch = TRUE;
					Downloads.Swap( y, x );
				}
			}
			else 
			{
				switch ( nColumn )
				{
					case DOWNLOAD_COLUMN_TITLE:
						if ( x->GetDisplayName().CompareNoCase(y->GetDisplayName()) < 0 )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_SIZE:
						if ( x->m_nSize < y->m_nSize )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_PROGRESS:
						if ( x->GetProgress() < y->GetProgress() )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_SPEED:
						if ( x->GetMeasuredSpeed() < y->GetMeasuredSpeed() )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_STATUS:
						if ( GetDownloadStatus(x).CompareNoCase( GetDownloadStatus(y) ) < 0 )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_CLIENT:
						if ( GetClientStatus(x) < GetClientStatus(y) )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_DOWNLOADED:
						if ( x->GetVolumeComplete() < y->GetVolumeComplete() )
							bOK = TRUE;
						break;
					case DOWNLOAD_COLUMN_PERCENTAGE:
						if ( ((double)(x->GetVolumeComplete() ) / (double)(x->m_nSize)) < ((double)(y->GetVolumeComplete() ) / (double)(y->m_nSize)) )
							bOK = TRUE;
						break;

				}
				if (bOK)
				{
					bSwitch = TRUE;
					Downloads.Swap( x, y );
				}
			}

			if ( pos != NULL )
				Downloads.GetPrevious( pos );
		}
	}
}

void CDownloadsCtrl::OnSortPanelItems(NMHDR* pNotifyStruct, LRESULT* pResult)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	NMLISTVIEW *pLV = (NMLISTVIEW *) pNotifyStruct;
	BubbleSortDownloads( pLV->iItem );
	Invalidate();
}


void CDownloadsCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownloadSource* pSource;
	CDownload* pDownload;
	
	m_wndTip.Hide();
	
	switch ( nChar )
	{
	case VK_HOME:
		SelectTo( 0 );
		return;
	case VK_END:
		{
			INT nMin, nMax;
			GetScrollRange( SB_VERT, &nMin, &nMax );
			SelectTo( max( 0, nMax - 1 ) );
		}
		return;
	case VK_UP:
		if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
			MoveSelected( -1 );
		else
			SelectTo( m_nFocus - 1 );
		return;
	case VK_DOWN:
		if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
			MoveSelected( 1 );
		else
			SelectTo( m_nFocus + 1 );
		return;
	case VK_PRIOR:
		SelectTo( m_nFocus - 10 );
		return;
	case VK_NEXT:
		SelectTo( m_nFocus + 10 );
		return;
	case VK_LEFT:
	case VK_SUBTRACT:
		if ( GetAt( m_nFocus, &pDownload, &pSource ) )
		{
			if ( pSource != NULL ) pDownload = pSource->m_pDownload;
			if ( pDownload->m_bExpanded == TRUE )
			{
				pDownload->m_bExpanded = FALSE;
				Update();
			}
		}
		return;
	case VK_RIGHT:
	case VK_ADD:
		if ( GetAt( m_nFocus, &pDownload, NULL ) && pDownload != NULL && pDownload->m_bExpanded == FALSE )
		{
			pDownload->m_bExpanded = TRUE;
			Update();
		}
		return;
	case 'A':
		if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
			SelectAll();
		return;
	case VK_DELETE:
		GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_CLEAR );
		return;
	case VK_RETURN:			// If the enter key is pressed activate the function relevant to the current focus
		OnEnterKey();		// Run the function that does the actions on the download window when enter key is pressed
		return;
	}
	
	CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CDownloadsCtrl::OnEnterKey()
{
		CDownloadSource* pSource;
		CDownload* pDownload;

		GetAt( m_nFocus, &pDownload, &pSource );								// Get the data for the current focus
		if ( pDownload != NULL )												// If the selected object is a download...
		{
			if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )						// And the control key is pressed...
			{
				GetOwner()->PostMessage( WM_TIMER, 5 );
				GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_LAUNCH );		// Launch the current file
			}
			else if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )					// Or the shift key is pressed...
			{
				GetOwner()->PostMessage( WM_TIMER, 5 );
				GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_ENQUEUE );	// Add the current file to playlist
			}
		}
		else if ( pSource != NULL )												// If the selected object is a download source...
		{
			GetOwner()->PostMessage( WM_TIMER, 5 );
			GetOwner()->PostMessage( WM_COMMAND, ID_TRANSFERS_CONNECT );		// Connect to the source
		}
}

void CDownloadsCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownloadSource* pSource;
	CDownload* pDownload;
	CRect rcItem;
	int nIndex;
	
	SetFocus();
	m_wndTip.Hide();
	
	if ( HitTest( point, &pDownload, &pSource, &nIndex, &rcItem ) )
	{
		if ( point.x <= rcItem.left + 16 )
		{
			if ( pDownload != NULL && IsExpandable( pDownload ) )
			{
				pDownload->m_bExpanded = ! pDownload->m_bExpanded;
				
				if ( ! pDownload->m_bExpanded )
				{
					for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource != NULL ; pSource = pSource->m_pNext )
						pSource->m_bSelected = FALSE;
				}
				
				Update();
			}
		}
		else
		{
			if ( pDownload != NULL && pDownload->m_bSelected )
			{
				if ( ( nFlags & ( MK_SHIFT | MK_CONTROL | MK_RBUTTON ) ) == 0 )
				{
					m_pDeselect1 = pDownload;
				}
			}
			else if ( pSource != NULL && pSource->m_bSelected )
			{
				if ( ( nFlags & ( MK_SHIFT | MK_CONTROL | MK_RBUTTON ) ) == 0 )
				{
					m_pDeselect2 = pSource;
				}
			}
			else if ( nFlags & MK_RBUTTON )
			{
				DeselectAll();
			}
			
			SelectTo( nIndex );
		}
	}
	else if ( ( nFlags & ( MK_SHIFT | MK_CONTROL ) ) == 0 )
	{
		DeselectAll();
		Update();
	}
	
	if ( ( nFlags & MK_LBUTTON ) && GetSelectedCount() > 0 )
	{
		m_bDrag = TRUE;
		m_ptDrag = point;
	}
}

void CDownloadsCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_wndTip.Hide();
	OnLButtonDown( nFlags, point );
	CWnd::OnRButtonDown( nFlags, point );
}

void CDownloadsCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownloadSource* pSource;
	CDownload* pDownload;
	CRect rcItem;
	
	SetFocus();
	
	if ( HitTest( point, &pDownload, &pSource, NULL, &rcItem ) )
	{
		if ( pDownload != NULL && point.x <= rcItem.left + 16 )
		{
			if ( IsExpandable( pDownload ) )
			{
				pDownload->m_bExpanded = ! pDownload->m_bExpanded;
				
				if ( ! pDownload->m_bExpanded )
				{
					for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource != NULL ; pSource = pSource->m_pNext )
						pSource->m_bSelected = FALSE;
				}
				
				Update();
			}
		}
		else if ( pDownload != NULL )
		{
			GetOwner()->PostMessage( WM_TIMER, 5 );
			GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_LAUNCH );
		}
		else if ( pSource != NULL )
		{
			GetOwner()->PostMessage( WM_TIMER, 5 );
			GetOwner()->PostMessage( WM_COMMAND, ID_TRANSFERS_CONNECT );
		}
	}
	
	CWnd::OnLButtonDblClk( nFlags, point );
}

void CDownloadsCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove( nFlags, point );
	
	if ( ( nFlags & ( MK_LBUTTON|MK_RBUTTON) ) == 0 )
	{
		CDownloadSource* pSource;
		CDownload* pDownload;
		CRect rcItem;
		
		if ( HitTest( point, &pDownload, &pSource, NULL, &rcItem ) )
		{
			if ( pDownload != NULL && point.x > rcItem.left + 16 )
			{
				m_wndTip.Show( pDownload );
				return;
			}
			else if ( pSource != NULL )
			{
				m_wndTip.Show( pSource );
				return;
			}
		}
	}
	else if ( m_bDrag )
	{
		if ( abs( point.x - m_ptDrag.x ) >= GetSystemMetrics( SM_CXDRAG ) ||
			 abs( point.y - m_ptDrag.y ) >= GetSystemMetrics( SM_CYDRAG ) )
		{
			OnBeginDrag( point );
			m_bDrag = FALSE;
			
		}
	}
	
	m_wndTip.Hide();
}

void CDownloadsCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bDrag = FALSE;
	
	if ( m_pDeselect1 != NULL )
	{
		DeselectAll( m_pDeselect1 );
		m_pDeselect1 = NULL;
	}
	else if ( m_pDeselect2 != NULL )
	{
		DeselectAll( NULL, m_pDeselect2 );
		m_pDeselect2 = NULL;
	}
	
	CWnd::OnLButtonUp( nFlags, point );
}

void CDownloadsCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_bDrag = FALSE;
	
	if ( m_pDeselect1 != NULL )
	{
		DeselectAll( m_pDeselect1 );
		m_pDeselect1 = NULL;
	}
	else if ( m_pDeselect2 != NULL )
	{
		DeselectAll( NULL, m_pDeselect2 );
		m_pDeselect2 = NULL;
	}
	
	CWnd::OnRButtonUp( nFlags, point );
}

void CDownloadsCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus( pOldWnd );
	Invalidate();
}

void CDownloadsCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus( pNewWnd );
	Invalidate();
}

void CDownloadsCtrl::OnBeginDrag(CPoint ptAction)
{
	m_wndTip.Hide();
	m_pDeselect1 = NULL;
	m_pDeselect2 = NULL;
	
	CPtrList* pSel = new CPtrList();
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) pSel->AddTail( pDownload );
	}
	
	if ( pSel->GetCount() == 0 )
	{
		delete pSel;
		return;
	}
	
	m_bCreateDragImage = TRUE;
	CImageList* pDragImage = CreateDragImage( pSel, ptAction );
	m_bCreateDragImage = FALSE;
	
	if ( pDragImage == NULL )
	{
		delete pSel;
		return;
	}
	
	m_pDragDrop = NULL;
	UpdateWindow();
	
	ClientToScreen( &ptAction );
	
	CDownloadsWnd* pWindow = (CDownloadsWnd*)GetOwner();
	ASSERT_KINDOF( CDownloadsWnd, pWindow );
	pWindow->DragDownloads( pSel, pDragImage, ptAction );
}

#define MAX_DRAG_SIZE	128
#define MAX_DRAG_SIZE_2	(MAX_DRAG_SIZE/2)

CImageList* CDownloadsCtrl::CreateDragImage(CPtrList* pSel, const CPoint& ptMouse)
{
	CRect rcClient, rcOne, rcAll( 32000, 32000, -32000, -32000 );
	
	GetClientRect( &rcClient );
	
	for ( POSITION pos = pSel->GetHeadPosition() ; pos ; )
	{
		CDownload* pDownload = (CDownload*)pSel->GetNext( pos );
		GetRect( pDownload, &rcOne );
		
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
	
	for ( POSITION pos = pSel->GetHeadPosition() ; pos ; )
	{
		CDownload* pDownload = (CDownload*)pSel->GetNext( pos );
		GetRect( pDownload, &rcOne );
		CRect rcDummy, rcOut( &rcOne );
		rcOut.OffsetRect( -rcAll.left, -rcAll.top );
		
		if ( rcDummy.IntersectRect( &rcAll, &rcOne ) )
		{
			dcDrag.FillSolidRect( &rcOut, RGB( 250, 255, 250 ) );
			PaintDownload( dcDrag, rcOut, pDownload, FALSE, FALSE );
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
