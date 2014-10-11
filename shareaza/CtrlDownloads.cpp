//
// CtrlDownloads.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "FragmentBar.h"
#include "Skin.h"
#include "CoolInterface.h"
#include "CtrlDownloads.h"
#include "WndDownloads.h"
#include "Flags.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

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
	ON_WM_GETDLGCODE()
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
#define DOWNLOAD_COLUMN_COUNTRY		8
#define COLUMNS_TO_SORT				DOWNLOAD_COLUMN_PERCENTAGE - DOWNLOAD_COLUMN_TITLE

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl construction

CDownloadsCtrl::CDownloadsCtrl() :
	m_nGroupCookie( 0 ),
	m_nFocus( 0 ),
	m_bCreateDragImage( FALSE ),
	m_pDragDrop( NULL ),
	m_bDrag( FALSE ),
	m_pDeselect1( NULL ),
	m_pDeselect2( NULL ),
	m_pbSortAscending( NULL ),
	m_bShowSearching( TRUE ),
	m_tSwitchTimer( 0 )
{
	// Try to get the number of lines to scroll when the mouse wheel is rotated
	if( !SystemParametersInfo ( SPI_GETWHEELSCROLLLINES, 0, &m_nScrollWheelLines, 0) )
	{
		m_nScrollWheelLines = 3;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl operations

BOOL CDownloadsCtrl::Create(CWnd* pParentWnd, UINT nID)
{
	CRect rc( 0, 0, 0, 0 );
	return CWnd::CreateEx( 0, NULL, _T("CDownloadsCtrl"),
		WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP, rc, pParentWnd, nID );
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
	
	m_wndTip.Create( this, &Settings.Interface.TipDownloads );
	
	InsertColumn( DOWNLOAD_COLUMN_TITLE, _T("Downloaded File"), LVCFMT_LEFT, 210 );
	InsertColumn( DOWNLOAD_COLUMN_SIZE, _T("Size"), LVCFMT_CENTER, 80 );
	InsertColumn( DOWNLOAD_COLUMN_PROGRESS, _T("Progress"), LVCFMT_CENTER, 130 );
	InsertColumn( DOWNLOAD_COLUMN_SPEED, _T("Speed"), LVCFMT_CENTER, 80 );
	InsertColumn( DOWNLOAD_COLUMN_STATUS, _T("Status"), LVCFMT_CENTER, 80 );
	InsertColumn( DOWNLOAD_COLUMN_CLIENT, _T("Client"), LVCFMT_CENTER, 80 );
	InsertColumn( DOWNLOAD_COLUMN_DOWNLOADED, _T("Downloaded"), LVCFMT_CENTER, 0 );
	InsertColumn( DOWNLOAD_COLUMN_PERCENTAGE, _T("Complete"), LVCFMT_CENTER, 60 );
	InsertColumn( DOWNLOAD_COLUMN_COUNTRY, _T("Country"), LVCFMT_LEFT, 60 );
	
	LoadColumnState();
	
	CoolInterface.LoadProtocolIconsTo( m_pProtocols );

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
	HDITEM pColumn = {};

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
	CString strOrdering, strWidths;

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
		if ( strWidths.GetLength() < 4 || strOrdering.GetLength() < 2 ||
			_stscanf( strWidths.Left( 4 ), _T("%x"), &pItem.cxy ) != 1 ||
			_stscanf( strOrdering.Left( 2 ), _T("%x"), &pItem.iOrder ) != 1 )
			return FALSE;

		strWidths = strWidths.Mid( 4 );
		strOrdering = strOrdering.Mid( 2 );

		m_wndHeader.SetItem( nColumns, &pItem );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl item helpers

bool CDownloadsCtrl::IsFiltered(const CDownload* pDownload)
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( Settings.General.GUIMode == GUI_BASIC )
		return false;

	if ( pDownload->IsCompleted() )
		return false;

	DWORD nFilterMask = Settings.Downloads.FilterMask;

	if ( pDownload->IsDownloading() || pDownload->IsMoving() )
		return ( ( nFilterMask & DLF_ACTIVE ) == 0 );

	if ( pDownload->IsPaused() )
		return ( ( nFilterMask & DLF_PAUSED ) == 0 );

	if ( pDownload->GetEffectiveSourceCount() > 0 )
		return ( ( nFilterMask & DLF_QUEUED ) == 0 );

	return ( ( nFilterMask & DLF_SOURCES ) == 0 );
}

BOOL CDownloadsCtrl::IsExpandable(CDownload* pDownload)
{
	if ( pDownload->IsSeeding() && !Settings.General.DebugBTSources ) return FALSE;
	if ( Settings.Downloads.ShowSources )
	{
		return ( pDownload->GetSourceCount() > 0 );
	}
	else
	{
		for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource->IsConnected() )
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CDownloadsCtrl::SelectTo(int nIndex)
{
	ASSUME_LOCK( Transfers.m_pSection );

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

void CDownloadsCtrl::SelectAll(CDownload* /*pDownload*/, CDownloadSource* /*pSource*/)
{
	ASSUME_LOCK( Transfers.m_pSection );

	BOOL bSelected = FALSE;

	for ( POSITION pos = Downloads.GetIterator() ; pos != NULL ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		// If a download is selected, select all downloads
		if ( pDownload->m_bSelected )
		{
			for ( POSITION pos2 = Downloads.GetIterator() ; pos2 != NULL ; )
			{
				if ( CDownload* pSelectedDownload = Downloads.GetNext( pos2 ) )
					pSelectedDownload->m_bSelected = TRUE;
			}

			bSelected = TRUE;
		}

		// If a source is selected, select all sources for that download
		for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource->m_bSelected )
			{
				for ( POSITION posSource2 = pDownload->GetIterator(); posSource2 ; )
				{
					CDownloadSource* pSource2 = pDownload->GetNext( posSource2 );

					pSource2->m_bSelected = TRUE;
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
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return;

	for ( POSITION pos = Downloads.GetIterator() ; pos != NULL ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		if ( pDownload != pExcept1 ) pDownload->m_bSelected = FALSE;

		for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource != pExcept2 )
				pSource->m_bSelected = FALSE;
		}
	}

	Invalidate();
}

int CDownloadsCtrl::GetSelectedCount()
{
	ASSUME_LOCK( Transfers.m_pSection );
	int nCount = 0;

	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );

		if ( pDownload->m_bSelected )
			nCount ++;

		for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource->m_bSelected )
				nCount ++;
		}
	}

	return nCount;
}

BOOL CDownloadsCtrl::HitTest(const CPoint& point, CDownload** ppDownload, CDownloadSource** ppSource, int* pnIndex, RECT* prcItem)
{
	ASSUME_LOCK( Transfers.m_pSection );

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

		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie )
			continue;

		if ( IsFiltered( pDownload ) )
			continue;

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
		if ( !pDownload->m_bExpanded || ( pDownload->IsSeeding() && !Settings.General.DebugBTSources ) ) continue;

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

		for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( Settings.Downloads.ShowSources || pSource->IsConnected() )
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
	ASSUME_LOCK( Transfers.m_pSection );

	/*int nScroll =*/ GetScrollPos( SB_VERT );
	int nIndex = 0;

	if ( ppDownload != NULL ) *ppDownload = NULL;
	if ( ppSource != NULL ) *ppSource = NULL;

	for ( POSITION posDownload = Downloads.GetIterator() ; posDownload ; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );

		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie )
			continue;

		if ( IsFiltered( pDownload ) )
			continue;

		if ( nIndex++ == nSelect )
		{
			if ( ppDownload != NULL ) *ppDownload = pDownload;
			return TRUE;
		}

		if ( !pDownload->m_bExpanded || ( pDownload->IsSeeding() && !Settings.General.DebugBTSources ) ) continue;

		for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( Settings.Downloads.ShowSources || pSource->IsConnected() )
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
	ASSUME_LOCK( Transfers.m_pSection );

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

		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie )
			continue;

		if ( IsFiltered( pDownload ) )
			continue;

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

		for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( pSource->IsConnected() )
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

BOOL CDownloadsCtrl::DropShowTarget(CList< CDownload* >* /*pSel*/, const CPoint& ptScreen)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

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

BOOL CDownloadsCtrl::DropObjects(CList< CDownload* >* pSel, const CPoint& ptScreen)
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

	if ( nType != 1982 ) CWnd::OnSize( nType, cx, cy );

	GetClientRect( &rcClient );

	HDITEM pColumn = {};
	pColumn.mask = HDI_WIDTH;

	for ( int nColumn = 0 ; m_wndHeader.GetItem( nColumn, &pColumn ) ; nColumn ++ )
		nWidth += pColumn.cxy;

	SCROLLINFO pScroll = {};
	pScroll.cbSize	= sizeof(pScroll);
	pScroll.fMask	= SIF_RANGE|SIF_PAGE;
	pScroll.nMin	= 0;
	pScroll.nMax	= nWidth;
	pScroll.nPage	= rcClient.right;
	SetScrollInfo( SB_HORZ, &pScroll, TRUE );

	int nScroll = GetScrollPos( SB_HORZ );
	m_wndHeader.SetWindowPos( NULL, -nScroll, 0, rcClient.right + nScroll, HEADER_HEIGHT, SWP_SHOWWINDOW );

	CSingleLock pLock( &Transfers.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return;

	for ( POSITION posDownload = Downloads.GetIterator() ; posDownload ; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );

		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie || IsFiltered( pDownload ) )
		{
			pDownload->m_bSelected = FALSE;

			for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
			{
				CDownloadSource* pSource = pDownload->GetNext( posSource );

				pSource->m_bSelected = FALSE;
			}
			continue;
		}

		nHeight ++;

		if ( !pDownload->m_bExpanded || ( pDownload->IsSeeding() && !Settings.General.DebugBTSources ) ) continue;

		if ( Settings.Downloads.ShowSources )
		{
			nHeight += pDownload->GetSourceCount();
		}
		else
		{
			for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
			{
				CDownloadSource* pSource = pDownload->GetNext( posSource );

				if ( Settings.Downloads.ShowSources || pSource->IsConnected() )
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
	CRect rcClient, rcItem;
	CPaintDC dc( this );
	DWORD tNow = GetTickCount();

	CSingleLock pTransfersLock( &Transfers.m_pSection );
	if ( ! pTransfersLock.Lock( 250 ) )
		return;

	if ( tNow - m_tSwitchTimer > 10000 )
	{
		m_tSwitchTimer = tNow;
		m_bShowSearching = !m_bShowSearching;
	}

	if ( Settings.General.LanguageRTL )
		dc.SetTextAlign( TA_RTLREADING );

	GetClientRect( &rcClient );
	rcClient.top += HEADER_HEIGHT;

	rcItem.CopyRect( &rcClient );
	rcItem.left -= GetScrollPos( SB_HORZ );
	rcItem.bottom = rcItem.top + ITEM_HEIGHT;

	int nScroll = GetScrollPos( SB_VERT );
	int nIndex = 0;

	CFont* pfOld	= (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );
	BOOL bFocus		= ( GetFocus() == this );

	for ( POSITION posDownload = Downloads.GetIterator() ; posDownload ; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );

		if ( rcItem.top > rcClient.bottom )
			break;

		if ( m_nGroupCookie != 0 && m_nGroupCookie != pDownload->m_nGroupCookie )
			continue;

		if ( IsFiltered( pDownload ) )
			continue;

		if ( nScroll > 0 )
			--nScroll;
		else
		{
			PaintDownload( dc, rcItem, pDownload, bFocus && ( m_nFocus == nIndex ), m_pDragDrop == pDownload );
			rcItem.OffsetRect( 0, ITEM_HEIGHT );
		}

		++nIndex;

		if ( !pDownload->m_bExpanded || ( pDownload->IsSeeding() && !Settings.General.DebugBTSources ) )
			continue;

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

		for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
		{
			CDownloadSource* pSource = pDownload->GetNext( posSource );

			if ( rcItem.top > rcClient.bottom )
				break;

			if ( Settings.Downloads.ShowSources || pSource->IsConnected() )
			{
				if ( nScroll > 0 )
					--nScroll;
				else
				{
					PaintSource( dc, rcItem, pDownload, pSource, bFocus && ( m_nFocus == nIndex ) );
					rcItem.OffsetRect( 0, ITEM_HEIGHT );
				}

				++nIndex;
			}
		}
	}

	dc.SelectObject( pfOld );

	if ( rcItem.top < rcClient.bottom )
	{
		rcClient.top = rcItem.top;
		dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );
	}
}

void CDownloadsCtrl::PaintDownload(CDC& dc, const CRect& rcRow, CDownload* pDownload, BOOL bFocus, BOOL bDrop)
{
	COLORREF crNatural		= m_bCreateDragImage ? DRAG_COLOR_KEY : CoolInterface.m_crWindow;
	COLORREF crBack			= pDownload->m_bSelected ? CoolInterface.m_crHighlight : crNatural;
	COLORREF crText			= CoolInterface.m_crText;
	COLORREF crLeftAligned	= crBack ;
	COLORREF crBorder		= pDownload->m_bSelected ? CoolInterface.m_crFragmentBorderSelected : CoolInterface.m_crFragmentBorder;
	COLORREF crBorderSimple	= pDownload->m_bSelected ? CoolInterface.m_crFragmentBorderSimpleBarSelected : CoolInterface.m_crFragmentBorderSimpleBar;

	if ( IsExpandable( pDownload ) )
		dc.SelectObject( &CoolInterface.m_fntBold ) ;

	if ( bDrop )
	{
		CRect rcDrop( rcRow.left, rcRow.top, rcRow.right, rcRow.top + 2 );
		dc.Draw3dRect( &rcDrop, 0, 0 );
		dc.ExcludeClipRect( &rcDrop );
	}

	dc.SetBkColor( crBack );
	dc.SetBkMode( OPAQUE );

	// Modify Text color if required
	if ( pDownload->IsCompleted() )
	{
		if ( pDownload->m_bSelected )
		{
 			if ( pDownload->m_bVerify == TRI_FALSE )
 				crText = CoolInterface.m_crTransferVerifyFailSelected;
			else if ( pDownload->IsSeeding() && ( pDownload->m_nSize > pDownload->m_nTorrentUploaded ) )
	 			crText = CoolInterface.m_crTransferVerifyPassSelected;
	  		else
	 			crText = CoolInterface.m_crTransferCompletedSelected;
		}
		else
		{
 			if ( pDownload->m_bVerify == TRI_FALSE )
 				crText = CoolInterface.m_crTransferVerifyFail;
	 		else if ( pDownload->IsSeeding() && ( pDownload->m_nSize > pDownload->m_nTorrentUploaded ) )
	 			crText = CoolInterface.m_crTransferVerifyPass;
	  		else
	 			crText = CoolInterface.m_crTransferCompleted;
		}
	}
	else if ( pDownload->m_bSelected )
		crText = CoolInterface.m_crHiText;

	dc.SetTextColor( crText );

	int nTextLeft = rcRow.right, nTextRight = rcRow.left;
	HDITEM pColumn = {};

	pColumn.mask = HDI_FORMAT | HDI_LPARAM;

	int nRating			= pDownload->GetReviewAverage();

	if ( ( nRating == 0 ) && ( pDownload->GetReviewCount() > 0 ) )
	{
		// There are reviews but no ratings- give it an "average" rating
		nRating = 3;
	}

	for ( int nColumn = 0 ; m_wndHeader.GetItem( nColumn, &pColumn ) ; nColumn++ )
	{
		CString strText;
		CRect rcCell;
		BOOL bDisplayText	= TRUE;

		m_wndHeader.GetItemRect( nColumn, &rcCell );
		rcCell.left		+= rcRow.left;
		rcCell.right	+= rcRow.left;
		rcCell.top		= rcRow.top;
		rcCell.bottom	= rcRow.bottom;

		crLeftAligned = ( rcRow.left == rcCell.left ? crNatural : crBack ) ;

		POINT ptHover;
		RECT  rcTick = { rcCell.left+2, rcCell.top+2, rcCell.left+14, rcCell.bottom-2 };
		GetCursorPos(&ptHover);
		ScreenToClient(&ptHover);

		switch ( pColumn.lParam )
		{
		case DOWNLOAD_COLUMN_TITLE:
			dc.FillSolidRect( rcCell.left, rcCell.bottom - 1, 32, 1, crLeftAligned );
			if ( IsExpandable( pDownload ) )
			{
				if ( pDownload->m_bExpanded )
				{
					CoolInterface.Draw( &dc, 
						( PtInRect(&rcTick,ptHover) ? IDI_MINUS_HOVER : IDI_MINUS ),
						16, rcCell.left, rcCell.top, crLeftAligned );
				}
				else 
				{
					CoolInterface.Draw( &dc, 
						( PtInRect(&rcTick,ptHover) ? IDI_PLUS_HOVER : IDI_PLUS ),
						16, rcCell.left, rcCell.top, crLeftAligned );
				}
			}
			else
				dc.FillSolidRect( rcCell.left, rcCell.top, 16, 16, crLeftAligned );
			rcCell.left += 16;

			// Draw file icon
			ShellIcons.Draw( &dc, ShellIcons.Get( pDownload->m_sName, 16 ), 16,
				rcCell.left, rcCell.top, crLeftAligned, pDownload->m_bSelected );
			
			// Add rating overlay
			switch ( nRating )
			{
			case 0:		// No reviews or no reviews with ratings
				break;
			case 1:		// Ratings suggest fake file
				CoolInterface.Draw( &dc, IDI_RATING_FAKE, 16,
					rcCell.left, rcCell.top, CLR_NONE, pDownload->m_bSelected );
				break;
			case 2:	
			case 3:	
			case 4:		// Ratings suggest average file
				CoolInterface.Draw( &dc, IDI_RATING_AVERAGE, 16,
					rcCell.left, rcCell.top, CLR_NONE, pDownload->m_bSelected );
				break;
			default:	// Ratings suggest good file
				CoolInterface.Draw( &dc, IDI_RATING_GOOD, 16,
					rcCell.left, rcCell.top, CLR_NONE, pDownload->m_bSelected );
				break;
			}

			rcCell.left += 16;
			dc.FillSolidRect( rcCell.left, rcCell.top, 1, rcCell.Height(), crLeftAligned );
			rcCell.left += 1;

			strText = pDownload->GetDisplayName();
			break;

		case DOWNLOAD_COLUMN_SIZE:
			if ( pDownload->m_nSize < SIZE_UNKNOWN )
				strText = Settings.SmartVolume( pDownload->m_nSize );
			else
				LoadString( strText, IDS_STATUS_UNKNOWN );
			break;

		case DOWNLOAD_COLUMN_PROGRESS:
			if ( rcCell.Width() > 75 )
			{
				bDisplayText = FALSE;
				dc.Draw3dRect( &rcCell, crBack, crBack );
				rcCell.DeflateRect( 1, 1 );
				dc.Draw3dRect( &rcCell, crBack, crBack );
				rcCell.DeflateRect( 0, 1 );

				if ( Settings.Downloads.SimpleBar )
				{
					dc.Draw3dRect( &rcCell, crBorderSimple, crBorderSimple );
					rcCell.DeflateRect( 1, 1 );
					CFragmentBar::DrawDownloadSimple( &dc, &rcCell, pDownload, crNatural );
				}
				else
				{
					dc.Draw3dRect( &rcCell, crBorder, crBorder );
					rcCell.DeflateRect( 1, 1 );
					CFragmentBar::DrawDownload( &dc, &rcCell, pDownload, crNatural );
				}
			}
			else if ( ( pDownload->m_nSize < SIZE_UNKNOWN ) && ( pDownload->m_nSize > 0 ) )
				if ( rcCell.Width() > 50 )
					strText.Format( _T("%.2f%%"), pDownload->GetProgress() );
  				else
					strText.Format( _T("%i%%"), int( pDownload->GetProgress() ) );
			break;

		case DOWNLOAD_COLUMN_SPEED:
			if ( pDownload->IsTrying() )
			{
				DWORD nSpeed = pDownload->GetAverageSpeed();
				if ( nSpeed )
					strText = Settings.SmartSpeed( nSpeed );
			}
			break;

		case DOWNLOAD_COLUMN_STATUS:
			if ( m_bShowSearching && pDownload->IsSearching() )
				LoadString( strText, IDS_STATUS_SEARCHING );
			else
				strText = pDownload->GetDownloadStatus();
			break;

		case DOWNLOAD_COLUMN_CLIENT:
			strText = pDownload->GetDownloadSources();
			break;

		case DOWNLOAD_COLUMN_DOWNLOADED:
			strText = Settings.SmartVolume( pDownload->GetVolumeComplete() );
			break;

		case DOWNLOAD_COLUMN_PERCENTAGE:
			if ( ( pDownload->m_nSize < SIZE_UNKNOWN ) && ( pDownload->m_nSize > 0 ) )
			{
				if ( rcCell.Width() > 50 )
				{
 					if ( pDownload->IsSeeding() )
 						strText.Format( _T("%.2f%%"), pDownload->GetRatio() );
 					else
 						strText.Format( _T("%.2f%%"), pDownload->GetProgress() );
  				}
  				else
  				{
 					if ( pDownload->IsSeeding() )
 						strText.Format( _T("%i%%"), int( pDownload->GetRatio() ) );
 					else
 						strText.Format( _T("%i%%"), int( pDownload->GetProgress() ) );
				}
			}
			else
				LoadString( strText, IDS_STATUS_UNKNOWN );
			break;
		}
		
		nTextLeft	= min( nTextLeft, int(rcCell.left) );
		nTextRight	= max( nTextRight, int(rcCell.right) );
		
		if ( ! bDisplayText ) continue;
		
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
		dc.Draw3dRect( &rcFocus, CoolInterface.m_crHiBorder, CoolInterface.m_crHiBorder );
	}

	dc.SelectObject( &CoolInterface.m_fntNormal ) ;
}

void CDownloadsCtrl::PaintSource(CDC& dc, const CRect& rcRow, CDownload* pDownload, CDownloadSource* pSource, BOOL bFocus)
{
	int nFlagImage = Flags.GetFlagIndex(pSource->m_sCountry);
	COLORREF crNatural		= m_bCreateDragImage ? DRAG_COLOR_KEY : CoolInterface.m_crWindow;
	COLORREF crBack			= pSource->m_bSelected ? CoolInterface.m_crHighlight : crNatural;
	COLORREF crLeftAligned	= crBack;
	COLORREF crBorder		= pSource->m_bSelected ? CoolInterface.m_crFragmentBorderSelected : CoolInterface.m_crFragmentBorder;

	dc.SetBkColor( crBack );
	dc.SetBkMode( OPAQUE );
	
	if ( pSource->m_bSelected )
		dc.SetTextColor( CoolInterface.m_crHiText );
	else
		dc.SetTextColor( CoolInterface.m_crTransferSource );
	
	int nTextLeft = rcRow.right, nTextRight = rcRow.left;
	HDITEM pColumn = {};
	
	pColumn.mask = HDI_FORMAT | HDI_LPARAM;
	
	for ( int nColumn = 0 ; m_wndHeader.GetItem( nColumn, &pColumn ) ; nColumn++ )
	{
		CString strText;
		CRect rcCell;
		BOOL bDisplayText	= TRUE;
		
		m_wndHeader.GetItemRect( nColumn, &rcCell );
		rcCell.left		+= rcRow.left;
		rcCell.right	+= rcRow.left;
		rcCell.top		= rcRow.top;
		rcCell.bottom	= rcRow.bottom;
		
		crLeftAligned = ( rcRow.left == rcCell.left ? crNatural : crBack ) ;

		switch ( pColumn.lParam )
		{
		case DOWNLOAD_COLUMN_TITLE:
			dc.FillSolidRect( rcCell.left, rcCell.top, 24, rcCell.Height(), crLeftAligned );
			rcCell.left += 24;
			dc.FillSolidRect( rcCell.left, rcCell.bottom - 1, 16, 1, crLeftAligned );
			ImageList_DrawEx( m_pProtocols, pSource->m_nProtocol, dc.GetSafeHdc(),
					rcCell.left, rcCell.top, 16, 16, crLeftAligned, CLR_DEFAULT, pSource->m_bSelected ? ILD_SELECTED : ILD_NORMAL );
			rcCell.left += 16;
			dc.FillSolidRect( rcCell.left, rcCell.top, 1, rcCell.Height(), crLeftAligned );
			rcCell.left += 1;

			// Is this a firewalled eDonkey client
			if ( pSource->m_nProtocol == PROTOCOL_DC )
			{
				strText.Format( _T("%s:%u"),
					(LPCTSTR)CString( inet_ntoa( pSource->m_pServerAddress ) ),
					pSource->m_nServerPort );
			}
			else if ( pSource->m_nProtocol == PROTOCOL_ED2K && pSource->m_bPushOnly == TRUE )
			{
				strText.Format( _T("%lu@%s:%u"),
					pSource->m_pAddress.S_un.S_addr,
					(LPCTSTR)CString( inet_ntoa( pSource->m_pServerAddress ) ),
					pSource->m_nServerPort );
			}
			// Or an active transfer
			else if ( ! pSource->IsIdle() )
			{
				strText.Format( _T("%s:%u"),
					(LPCTSTR)pSource->GetAddress(),
					ntohs( pSource->GetPort() ) );
			}
			// Or just queued
			else
			{
				strText.Format( _T("%s:%u"),
					(LPCTSTR)CString( inet_ntoa( pSource->m_pAddress ) ),
					pSource->m_nPort );
			}

			// Add the Nickname if there is one and they are being shown
			if ( Settings.Search.ShowNames && !pSource->m_sNick.IsEmpty() )
				strText = pSource->m_sNick + _T(" (") + strText + _T(")");

			// Indicate if this is a firewalled client
			if ( pSource->m_bPushOnly )
				strText += _T(" (push)");

			break;

		case DOWNLOAD_COLUMN_SIZE:
			if ( ! pSource->IsIdle() )
				if ( pSource->GetState() > dtsHeaders && pSource->m_oAvailable.empty() )
				{
					if ( pSource->m_pDownload->m_nSize != SIZE_UNKNOWN )
						strText = Settings.SmartVolume( pSource->m_pDownload->m_nSize );
				}
				else
					strText = Settings.SmartVolume( pSource->m_oAvailable.length_sum() );
			break;
			
		case DOWNLOAD_COLUMN_PROGRESS:
			if ( rcCell.Width() > 75 )
			{
				bDisplayText = FALSE;
				dc.Draw3dRect( &rcCell, crBack, crBack );
				rcCell.DeflateRect( 1, 1 );
				dc.Draw3dRect( &rcCell, crBack, crBack );
				rcCell.DeflateRect( 0, 1 );
				dc.Draw3dRect( &rcCell, crBorder, crBorder );
				rcCell.DeflateRect( 1, 1 );
				pSource->Draw( &dc, &rcCell, CoolInterface.m_crTransferRanges );
			}
			else if ( ! pSource->IsIdle() )
				if ( pSource->GetState() > dtsHeaders && pSource->m_oAvailable.empty() )
					rcCell.Width() > 50 ? strText = _T("100.00%") : strText = _T("100%");
				else if ( rcCell.Width() > 50 )
					strText.Format( _T("%.2f%%"),
						float( pSource->m_oAvailable.length_sum() * 10000 / pSource->m_pDownload->m_nSize ) / 100 );
  				else
					strText.Format( _T("%i%%"), int( pSource->m_oAvailable.length_sum() * 100 / pSource->m_pDownload->m_nSize ) );
			break;
			
		case DOWNLOAD_COLUMN_SPEED:
			if ( ! pSource->IsIdle() )
			{
				DWORD nSpeed = pSource->GetMeasuredSpeed();
				if ( nSpeed )
					strText = Settings.SmartSpeed( nSpeed );
			}
			break;
			
		case DOWNLOAD_COLUMN_STATUS:
			if ( ! pSource->IsIdle() )
				strText = pSource->GetState( FALSE );
			else if ( pSource->m_tAttempt && pDownload->IsTrying() )
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
			if ( ! pSource->IsIdle() )
				strText = Settings.SmartVolume( pSource->GetDownloaded() );
			break;
		case DOWNLOAD_COLUMN_PERCENTAGE:
			if ( ! pSource->IsIdle() && pSource->GetDownloaded() > 0 &&
				pDownload->m_nSize < SIZE_UNKNOWN && pDownload->m_nSize > 0 )
			{
				if ( rcCell.Width() > 50 )
					strText.Format( _T("%.2f%%"), float( pSource->GetDownloaded() * 10000 / pSource->m_pDownload->m_nSize ) / 100.0f );
				else
					strText.Format( _T("%i%%"), int( pSource->GetDownloaded() * 100 / pSource->m_pDownload->m_nSize ) );
			}
			break;
		case DOWNLOAD_COLUMN_COUNTRY:
			dc.FillSolidRect( rcCell.left, rcCell.top, 20, rcCell.Height(), crBack );
			rcCell.left += 2;
			if ( nFlagImage >= 0 )
				Flags.Draw( nFlagImage, dc.GetSafeHdc(), rcCell.left, rcCell.top + 2, CLR_NONE, CLR_DEFAULT, pSource->m_bSelected ? ILD_SELECTED : ILD_NORMAL );

			rcCell.left += 16;
			dc.FillSolidRect( rcCell.left, rcCell.top, 1, rcCell.Height(), crNatural );

			strText = pSource->m_sCountry;
			break;
		}
		
		nTextLeft	= min( nTextLeft, int(rcCell.left) );
		nTextRight	= max( nTextRight, int(rcCell.right) );
		
		if ( ! bDisplayText ) continue;
		
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
		dc.Draw3dRect( &rcFocus, CoolInterface.m_crHiBorder, CoolInterface.m_crHiBorder );
	}
}

void CDownloadsCtrl::OnSkinChange()
{
	m_wndHeader.SetFont( &CoolInterface.m_fntNormal );

	CoolInterface.LoadProtocolIconsTo( m_pProtocols );
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadsCtrl interaction message handlers

void CDownloadsCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
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

void CDownloadsCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
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

BOOL CDownloadsCtrl::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	OnVScroll( SB_THUMBPOSITION, (int)( GetScrollPos( SB_VERT ) - zDelta / WHEEL_DELTA * m_nScrollWheelLines ), NULL );
	return TRUE;
}

void CDownloadsCtrl::OnChangeHeader(NMHDR* /*pNotifyStruct*/, LRESULT* /*pResult*/)
{
	Update();
}

void CDownloadsCtrl::BubbleSortDownloads(int nColumn)  // BinaryInsertionSortDownloads(int nColumn)
{

	m_pbSortAscending[nColumn]= !m_pbSortAscending[nColumn];

	if (Downloads.GetCount() < 2) return;

	POSITION pos = Downloads.GetIterator(), pos_y = pos;
	Downloads.GetNext(pos);

	while (pos != NULL)
	{
		POSITION pos_x = pos;
		CDownload *x = Downloads.GetNext(pos);
		
		BOOL bOK = FALSE, bRlBk = TRUE;
		CDownload *y = NULL;
		while (bRlBk && (pos_y != NULL))
		{
			y = Downloads.GetPrevious(pos_y);
			if ( m_pbSortAscending[nColumn] == FALSE )
			{
				switch ( nColumn )
				{
					case DOWNLOAD_COLUMN_TITLE:
						if ( x->GetDisplayName().CompareNoCase( y->GetDisplayName() ) < 0 )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_SIZE:
						if ( x->m_nSize < y->m_nSize )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_PROGRESS:
						if ( x->GetProgress() < y->GetProgress() )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_SPEED:
						if ( x->GetMeasuredSpeed() < y->GetMeasuredSpeed() )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_STATUS:
						if ( x->GetDownloadStatus().CompareNoCase( y->GetDownloadStatus() ) < 0 )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_CLIENT:
						if ( x->GetClientStatus() < y->GetClientStatus() )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_DOWNLOADED:
						if ( x->GetVolumeComplete() < y->GetVolumeComplete() )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_PERCENTAGE:
						if ( ((double)(x->GetVolumeComplete() ) / (double)(x->m_nSize)) < ((double)(y->GetVolumeComplete() ) / (double)(y->m_nSize)) )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
				}//end switch
			}
			else
			{
				switch ( nColumn )
				{
					case DOWNLOAD_COLUMN_TITLE:
						if ( x->GetDisplayName().CompareNoCase(y->GetDisplayName()) > 0 )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_SIZE:
						if ( x->m_nSize > y->m_nSize )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_PROGRESS:
						if ( x->GetProgress() > y->GetProgress() )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_SPEED:
						if ( x->GetMeasuredSpeed() > y->GetMeasuredSpeed() )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_STATUS:
						if ( x->GetDownloadStatus().CompareNoCase( y->GetDownloadStatus() ) > 0 )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_CLIENT:
						if ( x->GetClientStatus() > y->GetClientStatus() )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_DOWNLOADED:
						if ( x->GetVolumeComplete() > y->GetVolumeComplete() )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
					case DOWNLOAD_COLUMN_PERCENTAGE:
						if ( ((double)(x->GetVolumeComplete() ) / (double)(x->m_nSize)) > ((double)(y->GetVolumeComplete() ) / (double)(y->m_nSize)) )
							bOK = TRUE;
						else
							bRlBk = FALSE;
						break;
				}//end switch                 
			}//end if else
		}//end while bRlBk
		if (bOK)
		{
			Downloads.Reorder(x, y);
			if (!bRlBk) Downloads.Move(x,1);
			if (pos == NULL) break;
			pos_y = pos;
            Downloads.GetPrevious(pos_y);
		}
		else
			pos_y = pos_x;
	}//end while pos
}

void CDownloadsCtrl::OnSortPanelItems(NMHDR* pNotifyStruct, LRESULT* /*pResult*/)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	NMLISTVIEW *pLV = (NMLISTVIEW *) pNotifyStruct;
	BubbleSortDownloads( pLV->iItem );
	Invalidate();
}


void CDownloadsCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return;

	CDownloadSource* pSource;
	CDownload* pDownload;
	
	m_wndTip.Hide();

	bool bControl = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0;
	bool bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;

	switch ( nChar )
	{
	case VK_HOME:
		if ( bControl )
		{
			GetAt( m_nFocus, &pDownload, &pSource );
			Downloads.Move( pDownload, -2 );
		}
		SelectTo( 0 );
		return;
	case VK_END:
		if ( bControl )
		{
			GetAt( m_nFocus, &pDownload, &pSource );
			Downloads.Move( pDownload, 2 );
		}
		INT nMin, nMax;
		GetScrollRange( SB_VERT, &nMin, &nMax );
		SelectTo( max( 0, nMax - 1 ) );
		return;
	case VK_UP:
		if ( ! bControl )
			SelectTo( m_nFocus - 1 );
		return;
	case VK_DOWN:
		if ( ! bControl )
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
		if ( bControl )
			SelectAll();
		return;
	case 'C':
		if ( bControl )
			GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_COPY );
		return;
	case 'V':
		if ( bControl )
			GetOwner()->PostMessage( WM_COMMAND, ID_TOOLS_DOWNLOAD );
		return;
	case VK_INSERT:
		if ( bControl )
			GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_COPY );
		else if ( bShift )
			GetOwner()->PostMessage( WM_COMMAND, ID_TOOLS_DOWNLOAD );
		return;
	case 'E':
		if ( bControl )
		{
			GetOwner()->PostMessage( WM_TIMER, 5 );
			GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_ENQUEUE );	// Add the current file to playlist
		}
		return;
	case 'R':
		if ( bControl )
			GetOwner()->PostMessage( WM_COMMAND, ID_DOWNLOADS_VIEW_REVIEWS );
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
			if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )						// And the control key is pressed...
			{
				GetOwner()->PostMessage( WM_TIMER, 5 );
				GetOwner()->PostMessage( WM_COMMAND, pDownload->IsCompleted() ?
					ID_DOWNLOADS_LAUNCH_COMPLETE : ID_DOWNLOADS_LAUNCH_COPY );	// Launch the current file
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
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return;

	CDownloadSource* pSource;
	CDownload* pDownload;
	CRect rcItem;
	int nIndex;
	
	SetFocus();
	m_wndTip.Hide();
	
	if ( HitTest( point, &pDownload, &pSource, &nIndex, &rcItem ) )
	{
		int nTitleStarts = GetExpandableColumnX();
		if ( point.x > nTitleStarts && point.x <= nTitleStarts + rcItem.left + 16 )
		{
			if ( pDownload != NULL && IsExpandable( pDownload ) )
			{
				pDownload->m_bExpanded = ! pDownload->m_bExpanded;
				
				if ( ! pDownload->m_bExpanded )
				{
					for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
					{
						CDownloadSource* pDownloadSource = pDownload->GetNext( posSource );

						pDownloadSource->m_bSelected = FALSE;
					}
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
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return;

	CDownloadSource* pSource;
	CDownload* pDownload;
	CRect rcItem;
	
	SetFocus();
	
	if ( HitTest( point, &pDownload, &pSource, NULL, &rcItem ) )
	{
		int nTitleStarts = GetExpandableColumnX();
		if ( pDownload != NULL && point.x > nTitleStarts && point.x <= nTitleStarts + rcItem.left + 16 )
		{
			if ( IsExpandable( pDownload ) )
			{
				pDownload->m_bExpanded = ! pDownload->m_bExpanded;
				
				if ( ! pDownload->m_bExpanded )
				{
					for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
					{
						CDownloadSource* pDownloadSource = pDownload->GetNext( posSource );

						pDownloadSource->m_bSelected = FALSE;
					}
				}
				
				Update();
			}
		}
		else if ( pDownload != NULL )
		{
			GetOwner()->PostMessage( WM_TIMER, 5 );
			GetOwner()->PostMessage( WM_COMMAND, pDownload->IsCompleted() ?
				ID_DOWNLOADS_LAUNCH_COMPLETE : ID_DOWNLOADS_LAUNCH_COPY );
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
		CSingleLock pLock( &Transfers.m_pSection );
		if ( ! pLock.Lock( 250 ) )
			return;

		CDownloadSource* pSource;
		CDownload* pDownload;
		CRect rcItem;
		
		if ( HitTest( point, &pDownload, &pSource, NULL, &rcItem ) )
		{
			// [+] or [-] Hoverstates
			if ( point.x < rcItem.left + 18 )
			{
				CRect rcRefresh( 1, rcItem.top - 32, 18, rcItem.bottom + 32 );
				RedrawWindow(rcRefresh);
			}

			if ( pDownload != NULL && point.x > rcItem.left + 18 )
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
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return;

	m_wndTip.Hide();
	m_pDeselect1 = NULL;
	m_pDeselect2 = NULL;
	
	CList< CDownload* >* pSel = new CList< CDownload* >;
	
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		if ( pDownload->m_bSelected ) pSel->AddTail( pDownload );
	}
	
	if ( pSel->IsEmpty() )
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

CImageList* CDownloadsCtrl::CreateDragImage(CList< CDownload* >* pSel, const CPoint& ptMouse)
{
	ASSUME_LOCK( Transfers.m_pSection );

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
	
	dcDrag.FillSolidRect( 0, 0, rcAll.Width(), rcAll.Height(), DRAG_COLOR_KEY );
	
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
			dcDrag.FillSolidRect( &rcOut, DRAG_COLOR_KEY );
			PaintDownload( dcDrag, rcOut, pDownload, FALSE, FALSE );
		}
	}
	
	dcDrag.SelectObject( pOldFont );
    dcDrag.SelectObject( pOldDrag );
    dcDrag.DeleteDC();
	
	CImageList* pAll = new CImageList();
	pAll->Create( rcAll.Width(), rcAll.Height(), ILC_COLOR32|ILC_MASK, 1, 1 ) ||
	pAll->Create( rcAll.Width(), rcAll.Height(), ILC_COLOR24|ILC_MASK, 1, 1 ) ||
	pAll->Create( rcAll.Width(), rcAll.Height(), ILC_COLOR16|ILC_MASK, 1, 1 );
	pAll->Add( &bmDrag, DRAG_COLOR_KEY ); 
	
	bmDrag.DeleteObject();
	
	pAll->BeginDrag( 0, ptMouse - rcAll.TopLeft() );
	
	return pAll;
}

int CDownloadsCtrl::GetExpandableColumnX() const
{
	HDITEM pColumn = {};
	int nTitleStarts = 0;
	
	pColumn.mask = HDI_LPARAM | HDI_WIDTH;

	for ( int nColumn = 0 ; m_wndHeader.GetItem( m_wndHeader.OrderToIndex( nColumn ), &pColumn ) ; nColumn++ )
	{
		if ( pColumn.lParam == DOWNLOAD_COLUMN_TITLE ) break;
		else nTitleStarts += pColumn.cxy;
	}
	return nTitleStarts;
}

UINT CDownloadsCtrl::OnGetDlgCode()
{
	return DLGC_WANTARROWS;
}
