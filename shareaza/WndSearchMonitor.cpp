//
// WndSearchMonitor.cpp
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
#include "CoolInterface.h"
#include "LiveList.h"
#include "QuerySearch.h"
#include "Security.h"
#include "Skin.h"
#include "WndBrowseHost.h"
#include "WndSearch.h"
#include "WndSearchMonitor.h"
#include "XML.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const static UINT nImageID[] =
{
	IDR_SEARCHMONITORFRAME,
	NULL
};

IMPLEMENT_SERIAL(CSearchMonitorWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CSearchMonitorWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_SEARCHMONITOR_PAUSE, OnUpdateSearchMonitorPause)
	ON_COMMAND(ID_SEARCHMONITOR_PAUSE, OnSearchMonitorPause)
	ON_COMMAND(ID_SEARCHMONITOR_CLEAR, OnSearchMonitorClear)
	ON_UPDATE_COMMAND_UI(ID_HITMONITOR_SEARCH, OnUpdateSearchMonitorSearch)
	ON_COMMAND(ID_HITMONITOR_SEARCH, OnSearchMonitorSearch)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_BAN, OnUpdateSecurityBan)
	ON_COMMAND(ID_SECURITY_BAN, OnSecurityBan)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_LAUNCH, OnUpdateBrowseLaunch)
	ON_COMMAND(ID_BROWSE_LAUNCH, OnBrowseLaunch)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SEARCHES, OnDblClkList)
	ON_WM_TIMER()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SEARCHES, OnCustomDrawList)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSearchMonitorWnd construction

CSearchMonitorWnd::CSearchMonitorWnd()
	: m_bPaused	( FALSE )
{
	Create( IDR_SEARCHMONITORFRAME );
}

/////////////////////////////////////////////////////////////////////////////
// CSearchMonitorWnd message handlers

int CSearchMonitorWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	m_wndList.Create( WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE |
		LVS_AUTOARRANGE | LVS_REPORT | LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_SEARCHES );
	m_pSizer.Attach( &m_wndList );
	
	m_wndList.SetExtendedStyle(
		LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_SUBITEMIMAGES );

	m_wndList.InsertColumn( 0, _T("Search"), LVCFMT_LEFT, 200, -1 );
	m_wndList.InsertColumn( 1, _T("URN"), LVCFMT_LEFT, 400, 0 );
	m_wndList.InsertColumn( 2, _T("Schema"), LVCFMT_LEFT, 100, 1 );
	m_wndList.InsertColumn( 3, _T("Endpoint"), LVCFMT_LEFT, 100, 2 );
	
	LoadState( _T("CSearchMonitorWnd"), TRUE );
	
	SetTimer( 2, 250, NULL );

	return 0;
}

void CSearchMonitorWnd::OnDestroy() 
{
	KillTimer( 2 );

	{
		CSingleLock pLock( &m_pSection, TRUE );

		m_bPaused = TRUE;

		for ( POSITION pos = m_pQueue.GetHeadPosition() ; pos ; )
		{
			delete m_pQueue.GetNext( pos );
		}
		m_pQueue.RemoveAll();
	}

	Settings.SaveList( _T("CSearchMonitorWnd"), &m_wndList );
	SaveState( _T("CSearchMonitorWnd") );

	CPanelWnd::OnDestroy();
}

void CSearchMonitorWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();

	// Columns
	Settings.LoadList( _T("CSearchMonitorWnd"), &m_wndList );

	// Fonts
	m_wndList.SetFont( &theApp.m_gdiFont );

	// Icons
	CoolInterface.LoadIconsTo( m_gdiImageList, nImageID );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
}

void CSearchMonitorWnd::OnSize(UINT nType, int cx, int cy) 
{
	CPanelWnd::OnSize( nType, cx, cy );
	m_pSizer.Resize( cx );
	m_wndList.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
}

void CSearchMonitorWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{
	Skin.TrackPopupMenu( _T("CSearchMonitorWnd"), point, ID_HITMONITOR_SEARCH );
}

void CSearchMonitorWnd::OnUpdateSearchMonitorSearch(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() == 1 );
}

void CSearchMonitorWnd::OnSearchMonitorSearch() 
{
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );

	if ( nItem >= 0 )
	{
		CQuerySearchPtr pSearch = new CQuerySearch();
		pSearch->m_sSearch = m_wndList.GetItemText( nItem, 0 );

		if ( pSearch->m_sSearch.GetLength() == 0 || 
			 _tcscmp( pSearch->m_sSearch, _T("\\") ) == 0 )
		{
			pSearch->m_sSearch = m_wndList.GetItemText( nItem, 1 );
			
			if ( _tcsicmp( pSearch->m_sSearch, _T("None") ) != 0 && 
				 _tcsncmp( pSearch->m_sSearch, _T("btih:"), 5 ) != 0 )
				pSearch->m_sSearch = _T("urn:") + m_wndList.GetItemText( nItem, 1 );
			else
				pSearch->m_sSearch.Empty();
		}

		if ( ! pSearch->m_sSearch.IsEmpty() )
			CQuerySearch::OpenWindow( pSearch );
	}
}

void CSearchMonitorWnd::OnUpdateSearchMonitorPause(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bPaused );
}

void CSearchMonitorWnd::OnSearchMonitorPause() 
{
	m_bPaused = ! m_bPaused;
}

void CSearchMonitorWnd::OnSearchMonitorClear() 
{
	m_wndList.DeleteAllItems();
}

void CSearchMonitorWnd::OnDblClkList(NMHDR* /*pNotifyStruct*/, LRESULT *pResult)
{
	OnSearchMonitorSearch();
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CPanelWnd event handlers

void CSearchMonitorWnd::OnQuerySearch(const CQuerySearch* pSearch)
{
	if ( m_bPaused || m_hWnd == NULL )
		return;

	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return;

	if ( m_bPaused )
		return;

	CLiveItem* pItem = new CLiveItem( 4, NULL );

	CString strSearch = pSearch->m_sSearch;
	CString strSchema;
	CString strURN;
	CString strNode;
	if ( pSearch->m_pEndpoint.sin_addr.s_addr )
		strNode.Format( _T("%hs:%u"),
			inet_ntoa( pSearch->m_pEndpoint.sin_addr ),
			ntohs( pSearch->m_pEndpoint.sin_port ) );

	if ( pSearch->HasHash() )
		strURN = pSearch->GetShortURN();
	else
		strURN = _T("None");

	if ( pSearch->m_bWhatsNew )
		strSearch = _T("What's New?");

	if ( pSearch->m_pXML )
	{
		strSearch += _T('«');
		strSearch += pSearch->m_pXML->GetRecursiveWords();
		strSearch += _T('»');
	}

	if ( pSearch->m_pSchema )
		strSchema = pSearch->m_pSchema->m_sTitle;
	else
		strSchema = _T("None");

	pItem->Set( 0, strSearch );
	pItem->Set( 1, strURN );
	pItem->Set( 2, strSchema );
	pItem->Set( 3, strNode );
		
	m_pQueue.AddTail( pItem );
}

void CSearchMonitorWnd::OnTimer(UINT_PTR nIDEvent) 
{
	if ( nIDEvent != 2 ) return;

	BOOL bScroll = m_wndList.GetTopIndex() + m_wndList.GetCountPerPage() >= m_wndList.GetItemCount();

	for (;;)
	{
		CLiveItem* pItem;

		{
			CSingleLock pLock( &m_pSection );
			if ( ! pLock.Lock( 250 ) )
				break;

			if ( m_pQueue.GetCount() == 0 )
				break;

			pItem = m_pQueue.RemoveHead();
		}

		if ( (DWORD)m_wndList.GetItemCount() >= Settings.Search.MonitorQueue && Settings.Search.MonitorQueue > 0 )
		{
			m_wndList.DeleteItem( 0 );
		}

		/*int nItem =*/ pItem->Add( &m_wndList, -1, 4 );

		delete pItem;
	}

	if ( bScroll ) m_wndList.EnsureVisible( m_wndList.GetItemCount() - 1, FALSE );
}

void CSearchMonitorWnd::OnUpdateSecurityBan(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetNextItem( -1, LVNI_SELECTED ) >= 0 );
}

void CSearchMonitorWnd::OnSecurityBan()
{
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem >= 0 )
	{
		SOCKADDR_IN pHost = { 0 };
		pHost.sin_family = AF_INET;
		CString strNode = m_wndList.GetItemText( nItem, 3 );
		int nPos = strNode.Find( _T(':') );
		pHost.sin_addr.s_addr = inet_addr( CT2CA( (LPCTSTR)strNode.Left( nPos ) ) );
		pHost.sin_port = htons( (WORD)_tstoi( strNode.Mid( nPos + 1 ) ) );
		Security.Ban( &pHost.sin_addr, banSession );
	}
}

void CSearchMonitorWnd::OnUpdateBrowseLaunch(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetNextItem( -1, LVNI_SELECTED ) >= 0 );
}

void CSearchMonitorWnd::OnBrowseLaunch()
{
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem >= 0 )
	{
		SOCKADDR_IN pHost = { 0 };
		pHost.sin_family = AF_INET;
		CString strNode = m_wndList.GetItemText( nItem, 3 );
		int nPos = strNode.Find( _T(':') );
		pHost.sin_addr.s_addr = inet_addr( CT2CA( (LPCTSTR)strNode.Left( nPos ) ) );
		pHost.sin_port = htons( (WORD)_tstoi( strNode.Mid( nPos + 1 ) ) );
		new CBrowseHostWnd( PROTOCOL_ANY, &pHost );
	}
}

void CSearchMonitorWnd::OnCustomDrawList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	//NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	*pResult = CDRF_DODEFAULT;
}
