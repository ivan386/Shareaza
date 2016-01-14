//
// WndHostCache.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "Network.h"
#include "HostCache.h"
#include "HubHorizon.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "VendorCache.h"
#include "WndHostCache.h"
#include "DlgDonkeyServers.h"
#include "DlgURLCopy.h"
#include "LiveList.h"
#include "Skin.h"
#include "CoolInterface.h"
#include "SchemaCache.h"

#include "Flags.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CHostCacheWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CHostCacheWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_NCMOUSEMOVE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_HOSTS, OnCustomDrawList)
	ON_NOTIFY(NM_DBLCLK, IDC_HOSTS, OnDblClkList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_HOSTS, OnSortList)
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_CONNECT, OnUpdateHostCacheConnect)
	ON_COMMAND(ID_HOSTCACHE_CONNECT, OnHostCacheConnect)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_DISCONNECT, OnUpdateHostCacheDisconnect)
	ON_COMMAND(ID_HOSTCACHE_DISCONNECT, OnHostCacheDisconnect)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_REMOVE, OnUpdateHostCacheRemove)
	ON_COMMAND(ID_HOSTCACHE_REMOVE, OnHostCacheRemove)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_G2_HORIZON, OnUpdateHostcacheG2Horizon)
	ON_COMMAND(ID_HOSTCACHE_G2_HORIZON, OnHostcacheG2Horizon)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_G2_CACHE, OnUpdateHostcacheG2Cache)
	ON_COMMAND(ID_HOSTCACHE_G2_CACHE, OnHostcacheG2Cache)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_G1_CACHE, OnUpdateHostcacheG1Cache)
	ON_COMMAND(ID_HOSTCACHE_G1_CACHE, OnHostcacheG1Cache)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_ED2K_CACHE, OnUpdateHostcacheEd2kCache)
	ON_COMMAND(ID_HOSTCACHE_ED2K_CACHE, OnHostcacheEd2kCache)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_BT_CACHE, OnUpdateHostcacheBTCache)
	ON_COMMAND(ID_HOSTCACHE_BT_CACHE, OnHostcacheBTCache)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_KAD_CACHE, OnUpdateHostcacheKADCache)
	ON_COMMAND(ID_HOSTCACHE_KAD_CACHE, OnHostcacheKADCache)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_DC_CACHE, OnUpdateHostcacheDCCache)
	ON_COMMAND(ID_HOSTCACHE_DC_CACHE, OnHostcacheDCCache)
	ON_COMMAND(ID_HOSTCACHE_IMPORT, OnHostcacheImport)
	ON_COMMAND(ID_HOSTCACHE_ED2K_DOWNLOAD, OnHostcacheEd2kDownload)
	ON_UPDATE_COMMAND_UI(ID_HOSTCACHE_PRIORITY, OnUpdateHostcachePriority)
	ON_COMMAND(ID_HOSTCACHE_PRIORITY, OnHostcachePriority)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_COPY, OnUpdateNeighboursCopy)
	ON_COMMAND(ID_NEIGHBOURS_COPY, OnNeighboursCopy)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CHostCacheWnd construction

CHostCacheWnd::CHostCacheWnd()
	: m_nMode			( PROTOCOLID( Settings.Gnutella.HostCacheView ) )
	, m_bAllowUpdates	( TRUE )
	, m_nCookie			( 0 )
	, m_tLastUpdate		( 0 )
{
	Create( IDR_HOSTCACHEFRAME );
}

/////////////////////////////////////////////////////////////////////////////
// CHostCacheWnd create

int CHostCacheWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );

	m_wndList.Create( WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE |
		LVS_AUTOARRANGE | LVS_REPORT | LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_HOSTS,
#ifdef _DEBUG
		14
#else
		11
#endif
		);
	m_pSizer.Attach( &m_wndList );

	m_wndList.SetExtendedStyle(
		LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_SUBITEMIMAGES );

	m_wndList.InsertColumn( 0, _T("Address"), LVCFMT_LEFT, 140, -1 );
	m_wndList.InsertColumn( 1, _T("Port"), LVCFMT_CENTER, 60, 0 );
	m_wndList.InsertColumn( 2, _T("Client"), LVCFMT_CENTER, 100, 1 );
	m_wndList.InsertColumn( 3, _T("Last Seen"), LVCFMT_CENTER, 130, 2 );
	m_wndList.InsertColumn( 4, _T("Daily Uptime"), LVCFMT_CENTER, 130, 3 );
	m_wndList.InsertColumn( 5, _T("Name"), LVCFMT_LEFT, 130, 4 );
	m_wndList.InsertColumn( 6, _T("Description"), LVCFMT_LEFT, 130, 5 );
	m_wndList.InsertColumn( 7, _T("CurUsers"), LVCFMT_CENTER, 60, 6 );
	m_wndList.InsertColumn( 8, _T("MaxUsers"), LVCFMT_CENTER, 60, 7 );
	m_wndList.InsertColumn( 9, _T("Failures"), LVCFMT_CENTER, 60, 7 );
	m_wndList.InsertColumn( 10, _T("Country"), LVCFMT_LEFT, 40, 10 );
#ifdef _DEBUG
	m_wndList.InsertColumn( 11, _T("Key"), LVCFMT_RIGHT, 0, 7 );
	m_wndList.InsertColumn( 12, _T("Query"), LVCFMT_RIGHT, 0, 8 );
	m_wndList.InsertColumn( 13, _T("Ack"), LVCFMT_RIGHT, 0, 9 );
#endif

	LoadState();

	PostMessage( WM_TIMER, 1 );

	return 0;
}

void CHostCacheWnd::OnDestroy()
{
	HostCache.Save();

	Settings.SaveList( _T("CHostCacheWnd"), &m_wndList );
	SaveState( _T("CHostCacheWnd") );

	CPanelWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CHostCacheWnd operations

void CHostCacheWnd::Update(BOOL bForce)
{
	if ( ! bForce && ! m_bAllowUpdates ) return;

	CHostCacheList* pCache = HostCache.ForProtocol( m_nMode ? m_nMode : PROTOCOL_G2 );
	CSingleLock oLock( &pCache->m_pSection, FALSE );
	if ( ! oLock.Lock( 100 ) ) return;

	m_nCookie = pCache->m_nCookie;

	for ( CHostCacheIterator i = pCache->Begin() ; i != pCache->End() ; ++i )
	{
		CHostCacheHostPtr pHost = (*i);

		if ( m_nMode == PROTOCOL_NULL )
		{
			if ( HubHorizonPool.Find( &pHost->m_pAddress ) == NULL ) continue;
		}

		CLiveItem* pItem = m_wndList.Add( pHost );

		pItem->SetImage( 0, pHost->m_nProtocol );
		pItem->SetMaskOverlay( pHost->m_bPriority );

		if ( pHost->m_sAddress.IsEmpty() )
			pItem->Set( 0, CString( inet_ntoa( pHost->m_pAddress ) ) );
		else if ( pHost->m_pAddress.s_addr == INADDR_ANY )
			pItem->Set( 0, pHost->m_sAddress );
		else
			pItem->Set( 0, pHost->m_sAddress + _T(" (") + CString( inet_ntoa( pHost->m_pAddress ) ) + _T(")") );

		pItem->Format( 1, _T("%hu"), pHost->m_nPort );

		if ( pHost->m_pVendor )
			pItem->Set( 2, pHost->m_pVendor->m_sName );
		else
			pItem->Set( 2, CString( _T("(") ) + protocolNames[ pHost->m_nProtocol ] + _T(")") );

		CTime pTime( (time_t)pHost->Seen() );
		pItem->Set( 3, pTime.Format( _T("%Y-%m-%d %H:%M:%S") ) );

		if ( pHost->m_nDailyUptime )
		{
			pTime = (time_t)pHost->m_nDailyUptime;
			pItem->Set( 4, pTime.Format( _T("%H:%M:%S") ) );

		}
		pItem->Set( 5, pHost->m_sName );
		pItem->Set( 6, pHost->m_sDescription );

		if ( pHost->m_nUserCount ) pItem->Format( 7, _T("%u"), pHost->m_nUserCount );
		if ( pHost->m_nUserLimit ) pItem->Format( 8, _T("%u"), pHost->m_nUserLimit );
		if ( pHost->m_nFailures ) pItem->Format( 9, _T("%u"), pHost->m_nFailures );
		if ( pHost->m_sCountry )
		{
			pItem->Set( 10, pHost->m_sCountry );
			int nFlag = Flags.GetFlagIndex( pHost->m_sCountry );
			if ( nFlag >= 0 )
				pItem->SetImage( 10, PROTOCOL_LAST + nFlag );
		}
#ifdef _DEBUG
		if ( pHost->m_nKeyValue ) pItem->Format( 11, _T("%u"), pHost->m_nKeyValue);
		if ( pHost->m_tQuery ) pItem->Format( 12, _T("%u"), pHost->m_tQuery );
		if ( pHost->m_tAck ) pItem->Format( 13, _T("%u"), pHost->m_tAck);
#endif
	}

	m_wndList.Apply();

	m_tLastUpdate = GetTickCount();				// Update timer
}

CHostCacheHostPtr CHostCacheWnd::GetItem(int nItem)
{
	if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
	{
		CHostCacheHostPtr pHost = (CHostCacheHostPtr)m_wndList.GetItemData( nItem );
		if ( HostCache.Check( pHost ) ) return pHost;
	}

	return NULL;
}

void CHostCacheWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();

	// Columns
	Settings.LoadList( _T("CHostCacheWnd"), &m_wndList );

	// Toolbar
	Skin.CreateToolBar( _T("CHostCacheWnd"), &m_wndToolBar );

	// Fonts
	m_wndList.SetFont( &theApp.m_gdiFont );

	// Icons (merge protocols and flags in one image list)
	CoolInterface.LoadProtocolIconsTo( m_gdiImageList );
	int nImages = m_gdiImageList.GetImageCount();
	int nFlags = Flags.GetCount();
	VERIFY( m_gdiImageList.SetImageCount( nImages + nFlags ) );
	for ( int nFlag = 0 ; nFlag < nFlags ; nFlag++ )
	{
		if ( HICON hIcon = Flags.ExtractIcon( nFlag ) )
		{
			VERIFY( m_gdiImageList.Replace( nImages + nFlag, hIcon ) != -1 );
			VERIFY( DestroyIcon( hIcon ) );
		}
	}
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	if ( Settings.General.GUIMode == GUI_BASIC)
	{
		Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_G2;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHostCacheWnd message handlers

void CHostCacheWnd::OnSize(UINT nType, int cx, int cy)
{
	CPanelWnd::OnSize( nType, cx, cy );
	m_pSizer.Resize( cx );
	SizeListAndBar( &m_wndList, &m_wndToolBar );
}

void CHostCacheWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 && IsPartiallyVisible() )
	{
		PROTOCOLID nEffective = m_nMode ? m_nMode : PROTOCOL_G2;

		if ( nEffective != PROTOCOL_G1 &&
			 nEffective != PROTOCOL_G2 &&
			 nEffective != PROTOCOL_ED2K &&
			 nEffective != PROTOCOL_BT &&
			 nEffective != PROTOCOL_KAD &&
			 nEffective != PROTOCOL_DC )
			nEffective = PROTOCOL_G2;

		CHostCacheList* pCache = HostCache.ForProtocol( nEffective );
		DWORD tTicks = GetTickCount();

		// Wait 10 seconds before refreshing; do not force updates
		if ( ( pCache->m_nCookie != m_nCookie ) && ( ( tTicks - m_tLastUpdate ) > 10000 ) ) Update();
	}
}

void CHostCacheWnd::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		if ( m_wndList.GetItemOverlayMask( (int)pDraw->nmcd.dwItemSpec ) )
		{
			SelectObject( pDraw->nmcd.hdc, CoolInterface.m_fntBold );
			pDraw->clrText = CoolInterface.m_crTextLink;
			*pResult = CDRF_NEWFONT;
		}
		else
			*pResult = CDRF_DODEFAULT;
	}
}

void CHostCacheWnd::OnDblClkList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnHostCacheConnect();
	*pResult = 0;
}

void CHostCacheWnd::OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNotifyStruct;

	m_wndList.Sort( pNMListView->iSubItem );

	*pResult = 0;
}

void CHostCacheWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// do not update the list while user navigates through context menu
	m_bAllowUpdates = FALSE;
	Skin.TrackPopupMenu( _T("CHostCacheWnd"), point, ID_HOSTCACHE_CONNECT );
	m_bAllowUpdates = TRUE;
}

void CHostCacheWnd::OnNcMouseMove(UINT /*nHitTest*/, CPoint /*point*/)
{
	// do not update for at least 5 sec while mouse is moving ouside host cache window
	m_bAllowUpdates = FALSE;
}

void CHostCacheWnd::OnUpdateHostCacheConnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ( m_wndList.GetSelectedCount() > 0 ) );
}

void CHostCacheWnd::OnHostCacheConnect()
{
	POSITION pos = m_wndList.GetFirstSelectedItemPosition();
	while ( pos )
	{
		int nItem = m_wndList.GetNextSelectedItem( pos );
		if ( CHostCacheHostPtr pHost = GetItem( nItem ) )
		{
			pHost->ConnectTo();
		}
	}
}

void CHostCacheWnd::OnUpdateHostCacheDisconnect(CCmdUI* pCmdUI)
{
	if ( m_nMode == PROTOCOL_NULL || m_nMode == PROTOCOL_G1 ||
		m_nMode == PROTOCOL_G2 || m_nMode == PROTOCOL_ED2K )
	{
		// Lock Network objects until we are finished with them
		// Note - This needs to be locked before the HostCache object to avoid
		// deadlocks with the network thread
		CQuickLock oNetworkLock( Network.m_pSection );

		// Lock HostCache objects until we are finished with them
		CQuickLock oHostCacheLock( HostCache.ForProtocol(
			m_nMode ? m_nMode : PROTOCOL_G2 )->m_pSection );

		POSITION pos = m_wndList.GetFirstSelectedItemPosition();
		while ( pos )
		{
			int nItem = m_wndList.GetNextSelectedItem( pos );
			if ( CHostCacheHostPtr pHost = GetItem( nItem ) )
			{
				CNeighbour* pNeighbour = Neighbours.Get( pHost->m_pAddress );
				if ( pNeighbour )
				{
					pCmdUI->Enable( TRUE );
					return;
				}
			}
		}
	}
	pCmdUI->Enable( FALSE );
}

void CHostCacheWnd::OnHostCacheDisconnect()
{
	if ( m_nMode == PROTOCOL_NULL || m_nMode == PROTOCOL_G1 ||
		m_nMode == PROTOCOL_G2 || m_nMode == PROTOCOL_ED2K )
	{
		// Lock Network objects until we are finished with them
		// Note - This needs to be locked before the HostCache object to avoid
		// deadlocks with the network thread
		CQuickLock oNetworkLock( Network.m_pSection );

		// Lock HostCache objects until we are finished with them
		CQuickLock oHostCacheLock( HostCache.ForProtocol(
			m_nMode ? m_nMode : PROTOCOL_G2 )->m_pSection );

		POSITION pos = m_wndList.GetFirstSelectedItemPosition();
		while ( pos )
		{
			int nItem = m_wndList.GetNextSelectedItem( pos );
			if ( CHostCacheHostPtr pHost = GetItem( nItem ) )
			{
				CNeighbour* pNeighbour = Neighbours.Get( pHost->m_pAddress );
				if ( pNeighbour )
					pNeighbour->Close();
			}
		}
	}
}

void CHostCacheWnd::OnUpdateHostcachePriority(CCmdUI* pCmdUI)
{
	if ( m_wndList.GetSelectedCount() == 0 )
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( FALSE );
		return;
	}

	CHostCacheList* pList = HostCache.ForProtocol( m_nMode ? m_nMode : PROTOCOL_G2 );
	CQuickLock oLock( pList->m_pSection );

	pCmdUI->Enable( TRUE );

	POSITION pos = m_wndList.GetFirstSelectedItemPosition();
	while ( pos )
	{
		int nItem = m_wndList.GetNextSelectedItem( pos );
		if ( CHostCacheHostPtr pHost = GetItem( nItem ) )
		{
			if ( pHost->m_bPriority )
			{
				pCmdUI->SetCheck( TRUE );
				return;
			}
		}
	}

	pCmdUI->SetCheck( FALSE );
}

void CHostCacheWnd::OnHostcachePriority()
{
	CHostCacheList* pList = HostCache.ForProtocol( m_nMode ? m_nMode : PROTOCOL_G2 );
	CQuickLock oLock( pList->m_pSection );

	POSITION pos = m_wndList.GetFirstSelectedItemPosition();
	while ( pos )
	{
		int nItem = m_wndList.GetNextSelectedItem( pos );
		if ( CHostCacheHostPtr pHost = GetItem( nItem ) )
		{
			pHost->m_bPriority = ! pHost->m_bPriority;
		}
	}

	pList->m_nCookie ++;

	InvalidateRect( NULL );
	Update();
}

void CHostCacheWnd::OnUpdateNeighboursCopy(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() == 1 );
}

void CHostCacheWnd::OnNeighboursCopy()
{
	CQuickLock oLock( HostCache.ForProtocol( m_nMode ? m_nMode : PROTOCOL_G2 )->m_pSection );

	CString strURL;

	CHostCacheHostPtr pHost = GetItem( m_wndList.GetNextItem( -1, LVNI_SELECTED ) );
	if ( ! pHost ) return;

	if ( pHost->m_nProtocol == PROTOCOL_G1 || pHost->m_nProtocol == PROTOCOL_G2 )
	{
		strURL.Format( _T("gnutella:host:%s:%u"),
			(LPCTSTR)pHost->Address(), pHost->m_nPort );
	}
	else if ( pHost->m_nProtocol == PROTOCOL_ED2K )
	{
		strURL.Format( _T("ed2k://|server|%s|%u|/"),
			(LPCTSTR)pHost->Address(), pHost->m_nPort );
	}
	else if ( pHost->m_nProtocol == PROTOCOL_KAD )
	{
		strURL.Format( _T("ed2k://|kad|%s|%u|/"),
			(LPCTSTR)pHost->Address(), pHost->m_nUDPPort );
	}
	else if ( pHost->m_nProtocol == PROTOCOL_DC )
	{
		strURL.Format( _T("dchub://%s:%u/"),
			(LPCTSTR)pHost->Address(), pHost->m_nUDPPort );
	}
	else if ( pHost->m_nProtocol == PROTOCOL_BT )
	{
		strURL.Format( _T("shareaza:btnode:%s:%u"),
			(LPCTSTR)pHost->Address(), pHost->m_nUDPPort );
	}

	theApp.SetClipboardText( strURL );
}

void CHostCacheWnd::OnUpdateHostCacheRemove(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CHostCacheWnd::OnHostCacheRemove()
{
	CQuickLock oLock( HostCache.ForProtocol( m_nMode ? m_nMode : PROTOCOL_G2 )->m_pSection );

	POSITION pos = m_wndList.GetFirstSelectedItemPosition();
	while ( pos )
	{
		int nItem = m_wndList.GetNextSelectedItem( pos );
		if ( CHostCacheHostPtr pHost = GetItem( nItem ) )
		{
			HostCache.Remove( pHost );
		}
	}

	m_wndList.ClearSelection();

	Update();
}

void CHostCacheWnd::OnUpdateHostcacheG2Horizon(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_nMode == PROTOCOL_NULL );
}

void CHostCacheWnd::OnHostcacheG2Horizon()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_NULL;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnUpdateHostcacheG2Cache(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_nMode == PROTOCOL_G2 );
}

void CHostCacheWnd::OnHostcacheG2Cache()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_G2;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnUpdateHostcacheG1Cache(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_nMode == PROTOCOL_G1 );
}

void CHostCacheWnd::OnHostcacheG1Cache()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_G1;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnUpdateHostcacheEd2kCache(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_nMode == PROTOCOL_ED2K );
}

void CHostCacheWnd::OnHostcacheEd2kCache()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_ED2K;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnUpdateHostcacheBTCache(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_nMode == PROTOCOL_BT );
}

void CHostCacheWnd::OnHostcacheBTCache()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_BT;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnUpdateHostcacheKADCache(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_nMode == PROTOCOL_KAD );
}

void CHostCacheWnd::OnHostcacheKADCache()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_KAD;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnUpdateHostcacheDCCache(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_nMode == PROTOCOL_DC );
}

void CHostCacheWnd::OnHostcacheDCCache()
{
	Settings.Gnutella.HostCacheView = m_nMode = PROTOCOL_DC;
	m_wndList.DeleteAllItems();
	Update( TRUE );
}

void CHostCacheWnd::OnHostcacheImport()
{
	// TODO: Localize it
	CFileDialog dlg( TRUE, _T("met"), NULL, OFN_HIDEREADONLY,
		_T("eDonkey2000 MET files|*.met|")
		_T("Kademlia Nodes files|nodes.dat|")
		_T("DC++ hub lists|*.xml.bz2|") +
		SchemaCache.GetFilter( CSchema::uriAllFiles ) +
		_T("|"), this );

	if ( dlg.DoModal() != IDOK ) return;

	CWaitCursor pCursor;
	HostCache.Import( dlg.GetPathName() );

	Update( TRUE );
}

void CHostCacheWnd::OnHostcacheEd2kDownload()
{
	CDonkeyServersDlg dlg;
	if ( dlg.DoModal() == IDOK ) Update( TRUE );
}

BOOL CHostCacheWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_TIMER )
	{
		// switch updates when window is inactive
		m_bAllowUpdates = IsActive();
	}
	else if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == 'A' && GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
		{
			for ( int nItem = m_wndList.GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
			{
				m_wndList.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
			}
			return TRUE;
		}
		else if ( pMsg->wParam == VK_DELETE )
		{
			OnHostCacheRemove();
			return TRUE;
		}
	}
	else if ( pMsg->message == WM_MOUSEWHEEL )
	{
		m_bAllowUpdates = FALSE;
	}
	return CPanelWnd::PreTranslateMessage( pMsg );
}

void CHostCacheWnd::RecalcLayout(BOOL bNotify)
{
	m_bAllowUpdates = FALSE;

	CPanelWnd::RecalcLayout(bNotify);
}
