//
// WndSearchMonitor.cpp
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
#include "QuerySearch.h"
#include "WndSearchMonitor.h"
#include "WndSearch.h"
#include "LiveList.h"
#include "XML.h"
#include "SHA.h"
#include "ED2K.h"
#include "TigerTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CSearchMonitorWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CSearchMonitorWnd, CPanelWnd)
	//{{AFX_MSG_MAP(CSearchMonitorWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_SEARCHMONITOR_PAUSE, OnUpdateSearchMonitorPause)
	ON_COMMAND(ID_SEARCHMONITOR_PAUSE, OnSearchMonitorPause)
	ON_COMMAND(ID_SEARCHMONITOR_CLEAR, OnSearchMonitorClear)
	ON_UPDATE_COMMAND_UI(ID_HITMONITOR_SEARCH, OnUpdateSearchMonitorSearch)
	ON_COMMAND(ID_HITMONITOR_SEARCH, OnSearchMonitorSearch)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SEARCHES, OnDblClkList)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSearchMonitorWnd construction

CSearchMonitorWnd::CSearchMonitorWnd()
{
	Create( IDR_SEARCHMONITORFRAME );
}

CSearchMonitorWnd::~CSearchMonitorWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSearchMonitorWnd message handlers

int CSearchMonitorWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	m_wndList.Create( WS_VISIBLE|LVS_ICON|LVS_AUTOARRANGE|LVS_REPORT|LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_SEARCHES );

	m_pSizer.Attach( &m_wndList );
	
	m_wndList.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP,
		LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP );
	
	m_gdiImageList.Create( 16, 16, ILC_MASK|ILC_COLOR16, 1, 1 );
	m_gdiImageList.Add( theApp.LoadIcon( IDR_SEARCHMONITORFRAME ) );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	m_wndList.InsertColumn( 0, _T("Search"), LVCFMT_LEFT, 200, -1 );
	m_wndList.InsertColumn( 1, _T("URN"), LVCFMT_LEFT, 120, 0 );
	m_wndList.InsertColumn( 2, _T("Schema"), LVCFMT_LEFT, 120, 1 );
	
	LoadState( _T("CSearchMonitorWnd"), TRUE );
	
	m_bPaused = FALSE;
	SetTimer( 2, 250, NULL );

	return 0;
}

void CSearchMonitorWnd::OnDestroy() 
{
	KillTimer( 2 );

	CSingleLock pLock( &m_pSection, TRUE );
	m_bPaused = TRUE;

	for ( POSITION pos = m_pQueue.GetHeadPosition() ; pos ; )
	{
		delete (CLiveItem*)m_pQueue.GetNext( pos );
	}
	m_pQueue.RemoveAll();

	pLock.Unlock();

	Settings.SaveList( _T("CSearchMonitorWnd"), &m_wndList );
	SaveState( _T("CSearchMonitorWnd") );

	CPanelWnd::OnDestroy();
}

void CSearchMonitorWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();
	Settings.LoadList( _T("CSearchMonitorWnd"), &m_wndList );
}

void CSearchMonitorWnd::OnSize(UINT nType, int cx, int cy) 
{
	CPanelWnd::OnSize( nType, cx, cy );
	m_pSizer.Resize( cx );
	m_wndList.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
}

void CSearchMonitorWnd::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	TrackPopupMenu( _T("CSearchMonitorWnd"), point, ID_HITMONITOR_SEARCH );
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
		CQuerySearch* pSearch = new CQuerySearch();
		pSearch->m_sSearch = m_wndList.GetItemText( nItem, 0 );
		pSearch->OpenWindow();
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

void CSearchMonitorWnd::OnDblClkList(NMHDR* pNotifyStruct, LRESULT *pResult)
{
	OnSearchMonitorSearch();
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CPanelWnd event handlers

void CSearchMonitorWnd::OnQuerySearch(CQuerySearch* pSearch)
{
	if ( m_bPaused || m_hWnd == NULL ) return;

	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_bPaused ) return;

	CLiveItem* pItem = new CLiveItem( 3, NULL );

	CString strSearch	= pSearch->m_sSearch;
	CString strSchema	= _T("None");
	CString strURN		= _T("None");

	if ( pSearch->m_oSHA1.IsValid() && pSearch->m_oTiger.IsValid() )
	{
		strURN	= _T("bitprint:")
			+ pSearch->m_oSHA1.ToString()
			+ '.'
			+ pSearch->m_oTiger.ToString();
	}
	else if ( pSearch->m_oTiger.IsValid() )
	{
		strURN = _T("tree:tiger/:") + pSearch->m_oTiger.ToString();
	}
	else if ( pSearch->m_oSHA1.IsValid() )
	{
		strURN = _T("sha1:") + pSearch->m_oSHA1.ToString();
	}
	else if ( pSearch->m_oED2K.IsValid() )
	{
		strURN = _T("ed2k:") + pSearch->m_oED2K.ToString();
	}
	else if ( pSearch->m_oBTH.IsValid() )
	{
		strURN = _T("btih:") + pSearch->m_oBTH.ToString();
	}

	if ( pSearch->m_pXML )
	{
		strSearch += ' ';
		strSearch += pSearch->m_pXML->GetRecursiveWords();
		
		strSchema = pSearch->m_pXML->GetAttributeValue( CXMLAttribute::schemaName, _T("") );
		
		int nSlash = strSchema.ReverseFind( '/' );
		if ( nSlash > 0 ) strSchema = strSchema.Mid( nSlash + 1 );
	}

	pItem->Set( 0, strSearch );
	pItem->Set( 1, strURN );
	pItem->Set( 2, strSchema );
		
	m_pQueue.AddTail( pItem );
}

void CSearchMonitorWnd::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent != 2 ) return;

	BOOL bScroll = m_wndList.GetTopIndex() + m_wndList.GetCountPerPage() >= m_wndList.GetItemCount();

	CSingleLock pLock( &m_pSection );

	while ( TRUE )
	{
		pLock.Lock();

		if ( m_pQueue.GetCount() == 0 ) break;
		CLiveItem* pItem = (CLiveItem*)m_pQueue.RemoveHead();

		pLock.Unlock();

		if ( (DWORD)m_wndList.GetItemCount() >= Settings.Search.MonitorQueue && Settings.Search.MonitorQueue > 0 )
		{
			m_wndList.DeleteItem( 0 );
		}

		int nItem = pItem->Add( &m_wndList, -1, 3 );

		delete pItem;
	}

	if ( bScroll ) m_wndList.EnsureVisible( m_wndList.GetItemCount() - 1, FALSE );
}
