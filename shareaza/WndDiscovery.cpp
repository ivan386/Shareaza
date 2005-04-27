//
// WndDiscovery.cpp
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
#include "Network.h"
#include "DiscoveryServices.h"
#include "LiveList.h"
#include "WndDiscovery.h"
#include "DlgDiscoveryService.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CDiscoveryWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CDiscoveryWnd, CPanelWnd)
	//{{AFX_MSG_MAP(CDiscoveryWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_NOTIFY(NM_DBLCLK, IDC_SERVICES, OnDblClkList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SERVICES, OnSortList)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_QUERY, OnUpdateDiscoveryQuery)
	ON_COMMAND(ID_DISCOVERY_QUERY, OnDiscoveryQuery)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_REMOVE, OnUpdateDiscoveryRemove)
	ON_COMMAND(ID_DISCOVERY_REMOVE, OnDiscoveryRemove)
	ON_COMMAND(ID_DISCOVERY_ADD, OnDiscoveryAdd)
	ON_COMMAND(ID_DISCOVERY_EDIT, OnDiscoveryEdit)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_EDIT, OnUpdateDiscoveryEdit)
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_GNUTELLA, OnUpdateDiscoveryGnutella)
	ON_COMMAND(ID_DISCOVERY_GNUTELLA, OnDiscoveryGnutella)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_WEBCACHE, OnUpdateDiscoveryWebcache)
	ON_COMMAND(ID_DISCOVERY_WEBCACHE, OnDiscoveryWebcache)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_ADVERTISE, OnUpdateDiscoveryAdvertise)
	ON_COMMAND(ID_DISCOVERY_ADVERTISE, OnDiscoveryAdvertise)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_BROWSE, OnUpdateDiscoveryBrowse)
	ON_COMMAND(ID_DISCOVERY_BROWSE, OnDiscoveryBrowse)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_SERVERMET, OnUpdateDiscoveryServerMet)
	ON_COMMAND(ID_DISCOVERY_SERVERMET, OnDiscoveryServerMet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDiscoveryWnd construction

CDiscoveryWnd::CDiscoveryWnd()
{
	Create( IDR_DISCOVERYFRAME );
}

CDiscoveryWnd::~CDiscoveryWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryWnd message handlers

int CDiscoveryWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	m_wndList.Create( WS_VISIBLE|LVS_ICON|LVS_AUTOARRANGE|LVS_REPORT|LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_SERVICES );
	m_pSizer.Attach( &m_wndList );
	
	m_wndList.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP,
		LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP );
	
	m_gdiImageList.Create( 16, 16, ILC_MASK|ILC_COLOR32, 4, 1 );
	m_gdiImageList.Add( theApp.LoadIcon( IDR_HOSTCACHEFRAME ) );
	m_gdiImageList.Add( theApp.LoadIcon( IDI_DISCOVERY_GRAY ) );
	m_gdiImageList.Add( theApp.LoadIcon( IDR_DISCOVERYFRAME ) );
	m_gdiImageList.Add( theApp.LoadIcon( IDI_WEB_URL ) );
	m_gdiImageList.Add( theApp.LoadIcon( IDI_DISCOVERY_BLUE ) );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	m_wndList.InsertColumn( 0, _T("Address"), LVCFMT_LEFT, 260, -1 );
	m_wndList.InsertColumn( 1, _T("Type"), LVCFMT_CENTER, 80, 0 );
	m_wndList.InsertColumn( 2, _T("Last Access"), LVCFMT_CENTER, 130, 1 );
	m_wndList.InsertColumn( 3, _T("Hosts"), LVCFMT_CENTER, 50, 2 );
	m_wndList.InsertColumn( 4, _T("Accesses"), LVCFMT_CENTER, 70, 3 );
	m_wndList.InsertColumn( 5, _T("Updates"), LVCFMT_CENTER, 55, 4 );
	m_wndList.InsertColumn( 6, _T("Failures"), LVCFMT_CENTER, 55, 5 );

	m_wndList.SetFont( &theApp.m_gdiFont );
	
	LoadState( _T("CDiscoveryWnd"), TRUE );

	m_bShowGnutella		= TRUE;
	m_bShowWebCache		= TRUE;
	m_bShowServerMet	= TRUE;

	CWaitCursor pCursor;
	Update();
	
	return 0;
}

void CDiscoveryWnd::OnDestroy() 
{
	CSingleLock pLock( &Network.m_pSection );
	if ( pLock.Lock( 250 ) ) DiscoveryServices.Save();

	Settings.SaveList( _T("CDiscoveryWnd"), &m_wndList );		
	SaveState( _T("CDiscoveryWnd") );

	CPanelWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryWnd operations

void CDiscoveryWnd::Update()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );
	CLiveList pLiveList( 7 );

	for ( POSITION pos = DiscoveryServices.GetIterator() ; pos ; )
	{
		CDiscoveryService* pService = DiscoveryServices.GetNext( pos );

		CLiveItem* pItem = NULL;

		if ( pService->m_nType == CDiscoveryService::dsGnutella )
		{
			if ( ! m_bShowGnutella ) continue;
			pItem = pLiveList.Add( pService );
			pItem->Set( 1, _T("Bootstrap") );
			pItem->m_nImage = 0;
		}
		else if ( pService->m_nType == CDiscoveryService::dsWebCache )
		{
			if ( ! m_bShowWebCache ) continue;
			pItem = pLiveList.Add( pService );
			pItem->Set( 1, _T("GWebCache") );
			if ( pService->m_bGnutella2 && pService->m_bGnutella1 )
			{
				pItem->m_nImage = 2;			// Multi-coloured icon
			}
			else
			{
				if ( pService->m_bGnutella2 )
					pItem->m_nImage = 4;		// Blue icon
				else if ( pService->m_bGnutella1 )
					pItem->m_nImage = 1;		// Grey icon
				else 
					pItem->m_nImage = 3;		// Blank
			}
		}
		else if ( pService->m_nType == CDiscoveryService::dsServerMet )
		{
			if ( ! m_bShowServerMet ) continue;
			pItem = pLiveList.Add( pService );
			pItem->Set( 1, _T("Server.met") );
			pItem->m_nImage = 3;
		}
		else
		{
			continue;
		}
		
		pItem->Set( 0, pService->m_sAddress );
		
		if ( pService->m_tAccessed )
		{
			CTime pTime( (time_t)pService->m_tAccessed );
			pItem->Set( 2, pTime.Format( _T("%Y-%m-%d %H:%M:%S") ) );
		}
		else
		{
			pItem->Set( 2, _T("0 - Never") );
		}
		
		if ( pService->m_nType == CDiscoveryService::dsWebCache ||
			 pService->m_nType == CDiscoveryService::dsServerMet )
		{
			pItem->Format( 4, _T("%u"), pService->m_nAccesses );
			pItem->Format( 6, _T("%u"), pService->m_nFailures );
			
			if ( pService->m_tAccessed )
			{
				pItem->Format( 3, _T("%u"), pService->m_nHosts );
				pItem->Format( 5, _T("%u"), pService->m_nUpdates );
			}
		}
	}
	
	pLiveList.Apply( &m_wndList, TRUE );
}

CDiscoveryService* CDiscoveryWnd::GetItem(int nItem)
{
	if ( nItem >= 0 && m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
	{
		CDiscoveryService* pService = (CDiscoveryService*)m_wndList.GetItemData( nItem );
		if ( DiscoveryServices.Check( pService ) ) return pService;
	}
	
	return NULL;
}

void CDiscoveryWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();
	Settings.LoadList( _T("CDiscoveryWnd"), &m_wndList, 3 );
}

void CDiscoveryWnd::OnSize(UINT nType, int cx, int cy) 
{
	CPanelWnd::OnSize(nType, cx, cy);
	m_pSizer.Resize( cx );
	m_wndList.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
}

void CDiscoveryWnd::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 1 ) Update();
}

void CDiscoveryWnd::OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnDiscoveryEdit();
	*pResult = 0;
}

void CDiscoveryWnd::OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNotifyStruct;
	CLiveList::Sort( &m_wndList, pNMListView->iSubItem );
	*pResult = 0;
}

void CDiscoveryWnd::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	TrackPopupMenu( _T("CDiscoveryWnd"), point, ID_DISCOVERY_EDIT );
}

void CDiscoveryWnd::OnUpdateDiscoveryQuery(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() == 1 );
}

void CDiscoveryWnd::OnDiscoveryQuery() 
{
	CSingleLock pLock( &Network.m_pSection, TRUE );
	
	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		CDiscoveryService* pService = GetItem( nItem );
		
		if ( pService != NULL )
		{
			pService->Execute( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ?
				CDiscoveryServices::wcmCaches : CDiscoveryServices::wcmHosts );
			break;
		}
	}
}

void CDiscoveryWnd::OnUpdateDiscoveryAdvertise(CCmdUI* pCmdUI) 
{
	CSingleLock pLock( &Network.m_pSection );

	if ( pLock.Lock( 100 ) )
	{
		CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
		pCmdUI->Enable( pService && pService->m_nType == CDiscoveryService::dsWebCache );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

void CDiscoveryWnd::OnDiscoveryAdvertise() 
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );

	if ( pService )
	{
		pService->Execute( CDiscoveryServices::wcmSubmit );
	}
}

void CDiscoveryWnd::OnUpdateDiscoveryBrowse(CCmdUI* pCmdUI) 
{
	OnUpdateDiscoveryAdvertise( pCmdUI );
}

void CDiscoveryWnd::OnDiscoveryBrowse() 
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
	CString strURL;

	if ( pService && pService->m_nType == CDiscoveryService::dsWebCache )
		strURL = pService->m_sAddress;

	pLock.Unlock();

	if ( strURL.GetLength() )
	{
		ShellExecute( GetSafeHwnd(), _T("open"), strURL, NULL, NULL, SW_SHOWNORMAL );
	}
}

void CDiscoveryWnd::OnUpdateDiscoveryRemove(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CDiscoveryWnd::OnDiscoveryRemove() 
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		CDiscoveryService* pService = GetItem( nItem );
		if ( pService ) pService->Remove();
	}

	Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryEdit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() == 1 );
}

void CDiscoveryWnd::OnDiscoveryEdit() 
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
	if ( ! pService ) return;

	pLock.Unlock();

	CDiscoveryServiceDlg dlg( NULL, pService );

	if ( dlg.DoModal() == IDOK ) Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryGnutella(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bShowGnutella );
}

void CDiscoveryWnd::OnDiscoveryGnutella() 
{
	m_bShowGnutella = ! m_bShowGnutella;
	Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryWebcache(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bShowWebCache );
}

void CDiscoveryWnd::OnDiscoveryWebcache() 
{
	m_bShowWebCache = ! m_bShowWebCache;
	Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryServerMet(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bShowServerMet );
}

void CDiscoveryWnd::OnDiscoveryServerMet() 
{
	m_bShowServerMet = ! m_bShowServerMet;
	Update();
}

void CDiscoveryWnd::OnDiscoveryAdd() 
{
	CDiscoveryServiceDlg dlg;

	if ( dlg.DoModal() == IDOK ) Update();
}
