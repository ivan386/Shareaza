//
// WndPacket.cpp
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
#include "Neighbours.h"
#include "Neighbour.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "EDPacket.h"
#include "WndPacket.h"
#include "CoolMenu.h"
#include "LiveList.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CPacketWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CPacketWnd, CPanelWnd)
	//{{AFX_MSG_MAP(CPacketWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PACKETS, OnCustomDrawList)
	ON_WM_CONTEXTMENU()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_UPDATE_COMMAND_UI(ID_SYSTEM_CLEAR, OnUpdateSystemClear)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE(1, 3200, OnUpdateBlocker)
END_MESSAGE_MAP()

LPCSTR CPacketWnd::m_pszG2[] = { "PI", "PO", "LNI", "KHL", "HAW", "QKR", "QKA", "Q1", "QH1", "Q2", "QH2", "QA", "QHT", "PUSH", "UPROC", "UPROD", NULL };


/////////////////////////////////////////////////////////////////////////////
// CPacketWnd construction

CPacketWnd::CPacketWnd(CChildWnd* pOwner)
{
	m_pOwner	= pOwner;
	m_bPaused	= TRUE;
	Create( IDR_PACKETFRAME );
}

CPacketWnd::~CPacketWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CPacketWnd create

int CPacketWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	m_wndList.Create( WS_VISIBLE|LVS_ICON|LVS_AUTOARRANGE|LVS_REPORT,
		rectDefault, this, IDC_PACKETS );

	m_pSizer.Attach( &m_wndList );
	
	m_wndList.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP,
		LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP );
	
	m_wndList.InsertColumn( 0, _T("Address"), LVCFMT_LEFT, 100, -1 );
	m_wndList.InsertColumn( 1, _T("Protocol"), LVCFMT_CENTER, 40, 0 );
    m_wndList.InsertColumn( 2, _T("Type"), LVCFMT_CENTER, 45, 1 );
	m_wndList.InsertColumn( 3, _T("T/H"), LVCFMT_CENTER, 40, 2 );
	m_wndList.InsertColumn( 4, _T("Hex"), LVCFMT_LEFT, 200, 3 );
	m_wndList.InsertColumn( 5, _T("ASCII"), LVCFMT_LEFT, 120, 4 );
	m_wndList.InsertColumn( 6, _T("G1-ID"), LVCFMT_LEFT, 0, 5 );
	// 210 for full width ID column
	
	m_pFont.CreateFont( -10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, theApp.m_sFont3 );

	m_wndList.SetFont( &m_pFont );

	CWnd* pWnd = CWnd::FromHandle( (HWND)m_wndList.SendMessage( LVM_GETHEADER ) );
	pWnd->SetFont( &theApp.m_gdiFont );
	
	LoadState( _T("CPacketWnd"), TRUE );

	m_pCoolMenu		= NULL;
	m_nInputFilter	= 0;
	m_nOutputFilter	= 0;
	m_bPaused		= FALSE;

	for ( int nType = 0 ; nType < G1_PACKTYPE_MAX ; nType++ ) m_bTypeG1[ nType ] = TRUE;
	for ( int nType = 0 ; nType < 16 ; nType++ ) m_bTypeG2[ nType ] = TRUE;

	SetTimer( 2, 500, NULL );

	return 0;
}

void CPacketWnd::OnDestroy() 
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

	Settings.SaveList( _T("CPacketWnd"), &m_wndList );
	SaveState( _T("CPacketWnd") );

	CPanelWnd::OnDestroy();
}

void CPacketWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();
	Settings.LoadList( _T("CPacketWnd"), &m_wndList );
}

/////////////////////////////////////////////////////////////////////////////
// CPacketWnd operations

void CPacketWnd::Process(const CNeighbour* pNeighbour, const IN_ADDR* pUDP, BOOL bOutgoing, const CPacket* pPacket)
{
	if ( m_bPaused || m_hWnd == NULL ) return;

	if ( pNeighbour )
	{
		if ( bOutgoing )
		{
			if ( m_nOutputFilter && m_nOutputFilter != pNeighbour->m_nUnique ) return;
		}
		else
		{
			if ( m_nInputFilter && m_nInputFilter != pNeighbour->m_nUnique ) return;
		}
	}
	else
	{
		if ( bOutgoing )
		{
			if ( m_nOutputFilter ) return;
		}
		else
		{
			if ( m_nInputFilter ) return;
		}
	}

	CG1Packet* pPacketG1 = ( pPacket->m_nProtocol == PROTOCOL_G1 ) ? (CG1Packet*)pPacket : NULL;
	CG2Packet* pPacketG2 = ( pPacket->m_nProtocol == PROTOCOL_G2 ) ? (CG2Packet*)pPacket : NULL;
	CEDPacket* pPacketED = ( pPacket->m_nProtocol == PROTOCOL_ED2K ) ? (CEDPacket*)pPacket : NULL;

	if ( pPacketG1 )
	{
		if ( ! m_bTypeG1[ pPacketG1->m_nTypeIndex ] ) return;
	}
	else if ( pPacketG2 )
	{
		for ( int nType = 0 ; m_pszG2[ nType ] ; nType++ )
		{
			if ( strcmp( pPacketG2->m_sType, m_pszG2[ nType ] ) == 0 )
			{
				if ( ! m_bTypeG2[ nType ] ) return;
				break;
			}
		}
	}
	else if ( pPacketED )
	{
		// TODO: Filter ED2K packets
	}
	
	CSingleLock pLock( &m_pSection, TRUE );
	
	if ( m_bPaused ) return;
	
	CLiveItem* pItem = new CLiveItem( 7, 0 );
	
	pItem->m_nParam = bOutgoing;
	
	if ( pNeighbour )
	{
		pItem->Set( 0, pNeighbour->m_sAddress );
	}
	else
	{
		pItem->Set( 0, _T("(") + CString( inet_ntoa( *pUDP ) ) + _T(")") );
	}
	
	if ( pPacketG2 )
		pItem->Set( 1, _T("G2") );
	else if ( pPacketG1 )
		pItem->Set( 1, _T("G1") );
	else if ( pPacketED )
		pItem->Set( 1, _T("ED2K") );
	
	pItem->Set( 2, pPacket->GetType() );
	pItem->Set( 4, pPacket->ToHex() );
	pItem->Set( 5, pPacket->ToASCII() );
	
	if ( pPacketG1 )
	{
		pItem->Format( 3, _T("%u/%u"), unsigned( pPacketG1->m_nTTL ), unsigned( pPacketG1->m_nHops ) );
		pItem->Set( 6, pPacketG1->GetGUID() );
	}
	
	m_pQueue.AddTail( pItem );
}

void CPacketWnd::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent != 2 ) return;

	BOOL bScroll = m_wndList.GetTopIndex() + m_wndList.GetCountPerPage() >= m_wndList.GetItemCount();

	CSingleLock pLock( &m_pSection );
	BOOL bAny = FALSE;

	while ( TRUE )
	{
		pLock.Lock();

		if ( m_pQueue.GetCount() == 0 ) break;
		CLiveItem* pItem = (CLiveItem*)m_pQueue.RemoveHead();

		pLock.Unlock();

		if ( ! bAny )
		{
			bAny = TRUE;
			m_wndList.ModifyStyle( WS_VISIBLE , 0);
		}

		if ( (DWORD)m_wndList.GetItemCount() >= Settings.Search.MonitorQueue && Settings.Search.MonitorQueue > 0 )
		{
			m_wndList.DeleteItem( 0 );
		}

		int nItem = pItem->Add( &m_wndList, -1, 7 );

		delete pItem;
	}

	if ( bAny )
	{
		if ( bScroll ) m_wndList.EnsureVisible( m_wndList.GetItemCount() - 1, FALSE );
		m_wndList.ShowWindow( SW_SHOW );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPacketWnd message handlers

void CPacketWnd::OnSize(UINT nType, int cx, int cy) 
{
	CPanelWnd::OnSize( nType, cx, cy );
	m_pSizer.Resize( cx );
	m_wndList.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
}

void CPacketWnd::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		if ( m_nInputFilter != 1 && m_nOutputFilter != 1 )
		{
			if ( pDraw->nmcd.lItemlParam )
				pDraw->clrText = RGB( 127, 0, 0 );
			else
				pDraw->clrText = RGB( 0, 0, 127 );
		}
		*pResult = CDRF_DODEFAULT;
	}
}

void CPacketWnd::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	CMenu pMenu, pHosts[2], pTypes1, pTypes2;

	for ( int nGroup = 0 ; nGroup < 2 ; nGroup++ )
	{
		UINT nID = nGroup ? 2000 : 1000;

		pHosts[nGroup].CreatePopupMenu();

		AddNeighbour( pHosts, nGroup, nID++, 1, _T("Disable") );
		AddNeighbour( pHosts, nGroup, nID++, 0, _T("Any Neighbour") );
		pHosts[nGroup].AppendMenu( MF_SEPARATOR, ID_SEPARATOR );

		for ( POSITION pos = Neighbours.GetIterator() ; pos ; nID++ )
		{
			CNeighbour* pNeighbour = Neighbours.GetNext( pos );
			if ( pNeighbour->m_nState < nrsConnected ) continue;
			AddNeighbour( pHosts, nGroup, nID, pNeighbour->m_nUnique, pNeighbour->m_sAddress );
		}

		if ( ( nID % 1000 ) == 2 )
			pHosts[nGroup].AppendMenu( MF_STRING|MF_GRAYED, 999, _T("No Neighbours") );
	}
	
	pTypes1.CreatePopupMenu();

	for ( int nType = 0 ; nType < G1_PACKTYPE_MAX ; nType++ )
	{
		if ( m_bTypeG1[ nType ] )
			pTypes1.AppendMenu( MF_STRING|MF_CHECKED, 3000 + nType, CG1Packet::m_pszPackets[ nType ] );
		else
			pTypes1.AppendMenu( MF_STRING, 3000 + nType, CG1Packet::m_pszPackets[ nType ] );
	}

	pTypes2.CreatePopupMenu();

	for ( int nType = 0 ; m_pszG2[ nType ] ; nType++ )
	{
		if ( m_bTypeG2[ nType ] )
			pTypes2.AppendMenu( MF_STRING|MF_CHECKED, 3100 + nType, CString( m_pszG2[ nType ] ) );
		else
			pTypes2.AppendMenu( MF_STRING, 3100 + nType, CString( m_pszG2[ nType ] ) );
	}

	pMenu.CreatePopupMenu();
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT)pHosts[0].GetSafeHmenu(), _T("Incoming") );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT)pHosts[1].GetSafeHmenu(), _T("Outgoing") );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT)pTypes1.GetSafeHmenu(), _T("G1 Types") );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT)pTypes2.GetSafeHmenu(), _T("G2 Types") );
	pMenu.AppendMenu( MF_SEPARATOR, ID_SEPARATOR );
	pMenu.AppendMenu( MF_STRING | ( m_bPaused ? MF_CHECKED : 0 ), 1, _T("&Pause Display") );
	pMenu.AppendMenu( MF_STRING, ID_SYSTEM_CLEAR, _T("&Clear Buffer") );

	m_pCoolMenu = new CCoolMenu();
	m_pCoolMenu->AddMenu( &pMenu, TRUE );
	m_pCoolMenu->SetWatermark( Skin.GetWatermark( _T("CCoolMenu") ) );

	pLock.Unlock();

	UINT nCmd = pMenu.TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD,
		point.x, point.y, this );

	delete m_pCoolMenu;
	m_pCoolMenu = NULL;

	DWORD* pModify = NULL;

	if ( nCmd == 1 )
	{
		m_bPaused = ! m_bPaused;
		return;
	}
	else if ( nCmd == ID_SYSTEM_CLEAR )
	{
		m_wndList.DeleteAllItems();
		return;
	}
	else if ( nCmd >= 3000 && nCmd < 3000 + G1_PACKTYPE_MAX )
	{
		nCmd -= 3000;

		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			for ( int nType = 0 ; nType < G1_PACKTYPE_MAX ; nType++ )
			{
				m_bTypeG1[ nType ] = ( nCmd == (UINT)nType ) ? TRUE : FALSE;
			}
		}
		else
		{
			m_bTypeG1[ nCmd ] = ! m_bTypeG1[ nCmd ];
		}

		return;
	}
	else if ( nCmd >= 3100 && nCmd < 3100 + 16 )
	{
		nCmd -= 3100;

		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			for ( int nType = 0 ; nType < 16 ; nType++ )
			{
				m_bTypeG2[ nType ] = ( nCmd == (UINT)nType ) ? TRUE : FALSE;
			}
		}
		else
		{
			m_bTypeG2[ nCmd ] = ! m_bTypeG2[ nCmd ];
		}

		return;
	}
	else if ( nCmd >= 1000 && nCmd < 2000 )
	{
		pModify = &m_nInputFilter;
		nCmd -= 1000;
	}
	else if ( nCmd >= 2000 && nCmd < 3000 )
	{
		pModify = &m_nOutputFilter;
		nCmd -= 2000;
	}
	else
	{
		return;
	}

	if ( nCmd == 0 )
	{
		*pModify = 1;
	}
	else if ( nCmd == 1 )
	{
		*pModify = 0;
	}
	else
	{
		pLock.Lock();
		nCmd -= 2;

		for ( POSITION pos = Neighbours.GetIterator() ; pos ; nCmd-- )
		{
			CNeighbour* pNeighbour = Neighbours.GetNext( pos );
			if ( ! nCmd )
			{
				*pModify = pNeighbour->m_nUnique;
				break;
			}
		}
	}

	Invalidate();
}

void CPacketWnd::AddNeighbour(CMenu* pMenus, int nGroup, UINT nID, DWORD nTarget, LPCTSTR pszText)
{
	UINT nChecked = ( ( nGroup == 1 && m_nOutputFilter == nTarget ) ||
		 ( nGroup == 0 && m_nInputFilter == nTarget ) )
		 ? MF_CHECKED : 0;

	pMenus[nGroup].AppendMenu( MF_STRING|nChecked, nID, pszText );
}

void CPacketWnd::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	if ( m_pCoolMenu ) m_pCoolMenu->OnMeasureItem( lpMeasureItemStruct );
}

void CPacketWnd::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if ( m_pCoolMenu ) m_pCoolMenu->OnDrawItem( lpDrawItemStruct );
}

void CPacketWnd::OnUpdateBlocker(CCmdUI* pCmdUI)
{
	if ( m_pCoolMenu ) pCmdUI->Enable( pCmdUI->m_nID != 999 );
	else pCmdUI->ContinueRouting();
}

void CPacketWnd::OnUpdateSystemClear(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( TRUE );	
}

