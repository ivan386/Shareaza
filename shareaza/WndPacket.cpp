//
// WndPacket.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "Neighbours.h"
#include "Neighbour.h"
#include "EDPacket.h"
#include "WndPacket.h"
#include "CoolInterface.h"
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

G2_PACKET CPacketWnd::m_nG2[nTypeG2Size] = {
	G2_PACKET_CACHED_HUB,
	G2_PACKET_CRAWL_ANS,
	G2_PACKET_CRAWL_REQ,
	G2_PACKET_DISCOVERY,
	G2_PACKET_HAW,
	G2_PACKET_HIT,
	G2_PACKET_HIT_WRAP,
	G2_PACKET_KHL,
	G2_PACKET_KHL_ANS,
	G2_PACKET_KHL_REQ,
	G2_PACKET_LNI,
	G2_PACKET_PING,
	G2_PACKET_PONG,
	G2_PACKET_PROFILE_CHALLENGE,
	G2_PACKET_PROFILE_DELIVERY,
	G2_PACKET_PUSH,
	G2_PACKET_QHT,
	G2_PACKET_QUERY,
	G2_PACKET_QUERY_ACK,
	G2_PACKET_QUERY_KEY_ANS,
	G2_PACKET_QUERY_KEY_REQ,
	G2_PACKET_QUERY_WRAP
};

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
	
	m_wndList.Create( WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE |
		LVS_AUTOARRANGE | LVS_REPORT | LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_PACKETS );
	m_pSizer.Attach( &m_wndList );
	
	m_wndList.SetExtendedStyle(
		LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP );

	m_pFont.CreateFontW( -(int)(Settings.Fonts.FontSize - 1), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.PacketDumpFont );

	m_wndList.SetFont( &m_pFont );

	CWnd* pWnd = CWnd::FromHandle( (HWND)m_wndList.SendMessage( LVM_GETHEADER ) );
	pWnd->SetFont( &theApp.m_gdiFont );

	LoadState( _T("CPacketWnd"), TRUE );

	CRect rcList;
	m_wndList.GetClientRect( &rcList );
	m_wndList.InsertColumn( 0, _T("Time"), LVCFMT_CENTER, 60, -1 );
	m_wndList.InsertColumn( 1, _T("Address"), LVCFMT_LEFT, 115, -1 );
	m_wndList.InsertColumn( 2, _T("Protocol"), LVCFMT_CENTER, 55, 0 );
    m_wndList.InsertColumn( 3, _T("Type"), LVCFMT_CENTER, 50, 1 );
	m_wndList.InsertColumn( 4, _T("TTL/Hops"), LVCFMT_CENTER, 60, 2 );
	m_wndList.InsertColumn( 5, _T("Hex"), LVCFMT_LEFT, 50, 3 );
	m_wndList.InsertColumn( 6, _T("ASCII"), LVCFMT_LEFT,
		rcList.Width() - 440 - GetSystemMetrics( SM_CXVSCROLL ) - 1, 4 );
	m_wndList.InsertColumn( 7, _T("G1-ID"), LVCFMT_LEFT, 50, 5 );
	
	m_pCoolMenu		= NULL;
	m_nInputFilter	= 0;
	m_nOutputFilter	= 0;
	m_bPaused		= FALSE;

	for ( int nType = 0 ; nType < nTypeG1Size ; nType++ ) m_bTypeG1[ nType ] = TRUE;
	for ( int nType = 0 ; nType < nTypeG2Size ; nType++ ) m_bTypeG2[ nType ] = TRUE;
	m_bTypeED = TRUE;

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
		delete m_pQueue.GetNext( pos );
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

void CPacketWnd::SmartDump(const CPacket* pPacket, const SOCKADDR_IN* pAddress, BOOL bUDP, BOOL bOutgoing, DWORD nNeighbourUnique)
{
	ASSERT( pPacket );
	ASSERT( pAddress );

	if ( m_bPaused || m_hWnd == NULL ) return;

	if ( nNeighbourUnique )
	{
		if ( bOutgoing )
		{
			if ( m_nOutputFilter && m_nOutputFilter != nNeighbourUnique ) return;
		}
		else
		{
			if ( m_nInputFilter && m_nInputFilter != nNeighbourUnique ) return;
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
		for ( int nType = 0 ; nType < nTypeG2Size ; nType++ )
		{
			if ( pPacketG2->IsType( m_nG2[ nType ] ) )
			{
				if ( ! m_bTypeG2[ nType ] ) return;
				break;
			}
		}
	}
	else if ( pPacketED )
	{
		// TODO: Filter ED2K packets
		if ( !m_bTypeED ) return;
	}
	
	CSingleLock pLock( &m_pSection, TRUE );
	
	if ( m_bPaused ) return;
	
	CLiveItem* pItem = new CLiveItem( 8, 0 );
	
	pItem->m_nParam = bOutgoing;

	CTime pNow( CTime::GetCurrentTime() );
	CString strNow;
	strNow.Format( _T("%0.2i:%0.2i:%0.2i"),
		pNow.GetHour(), pNow.GetMinute(), pNow.GetSecond() );
	pItem->Set( 0, strNow );
	
	if ( ! bUDP )
	{
		pItem->Set( 1, CString( inet_ntoa( pAddress->sin_addr ) ) );
		if ( pPacketG2 )
			pItem->Set( 2, _T("G2 TCP") );
		else if ( pPacketG1 )
			pItem->Set( 2, _T("G1 TCP") );
		else if ( pPacketED )
			pItem->Set( 2, _T("ED2K TCP") );
	}
	else
	{
		pItem->Set( 1, _T("(") + CString( inet_ntoa( pAddress->sin_addr ) ) + _T(")") );
		if ( pPacketG2 )
			pItem->Set( 2, _T("G2 UDP") );
		else if ( pPacketG1 )
			pItem->Set( 2, _T("G1 UDP") );
		else if ( pPacketED )
			pItem->Set( 2, _T("ED2K UDP") );
	}
	
	pItem->Set( 3, pPacket->GetType() );
	pItem->Set( 5, pPacket->ToHex() );
	pItem->Set( 6, pPacket->ToASCII() );
	
	if ( pPacketG1 )
	{
		pItem->Format( 4, _T("%u/%u"), unsigned( pPacketG1->m_nTTL ), unsigned( pPacketG1->m_nHops ) );
		pItem->Set( 7, pPacketG1->GetGUID() );
	}
	
	m_pQueue.AddTail( pItem );
}

void CPacketWnd::OnTimer(UINT_PTR nIDEvent) 
{
	if ( nIDEvent != 2 ) return;

	BOOL bScroll = m_wndList.GetTopIndex() + m_wndList.GetCountPerPage() >= m_wndList.GetItemCount();

	CSingleLock pLock( &m_pSection );
	BOOL bAny = FALSE;

	for (;;)
	{
		pLock.Lock();

		if ( m_pQueue.GetCount() == 0 ) break;
		CLiveItem* pItem = m_pQueue.RemoveHead();

		pLock.Unlock();

		if ( ! bAny )
		{
			bAny = TRUE;
		}

		if ( (DWORD)m_wndList.GetItemCount() >= Settings.Search.MonitorQueue && Settings.Search.MonitorQueue > 0 )
		{
			m_wndList.DeleteItem( 0 );
		}

		/*int nItem =*/ pItem->Add( &m_wndList, -1, 8 );

		delete pItem;
	}

	if ( bAny )
	{
		if ( bScroll ) m_wndList.EnsureVisible( m_wndList.GetItemCount() - 1, FALSE );
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
				pDraw->clrText = CoolInterface.m_crNetworkUp ;
			else
				pDraw->clrText = CoolInterface.m_crNetworkDown ;
		}
		*pResult = CDRF_DODEFAULT;
	}
}

void CPacketWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	CMenu pMenu, pHosts[2], pTypes1, pTypes2, pTypes3;

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

	for ( int nType = 0 ; nType < nTypeG1Size ; nType++ )
	{
		if ( m_bTypeG1[ nType ] )
			pTypes1.AppendMenu( MF_STRING|MF_CHECKED, 3000 + nType, CG1Packet::m_pszPackets[ nType ] );
		else
			pTypes1.AppendMenu( MF_STRING, 3000 + nType, CG1Packet::m_pszPackets[ nType ] );
	}

	pTypes2.CreatePopupMenu();

	for ( int nType = 0 ; nType < nTypeG2Size ; nType++ )
	{
		CStringA tmp;
		tmp.Append( (LPCSTR)&m_nG2[ nType ], G2_TYPE_LEN( m_nG2[ nType ] ) );
		if ( m_bTypeG2[ nType ] )
			pTypes2.AppendMenu( MF_STRING|MF_CHECKED, 3100 + nType, (LPCTSTR)CA2T( tmp ) );
		else
			pTypes2.AppendMenu( MF_STRING, 3100 + nType, (LPCTSTR)CA2T( tmp ) );
	}

	pTypes3.CreatePopupMenu();
	if ( m_bTypeED )
			pTypes3.AppendMenu( MF_STRING|MF_CHECKED, 3200, CString( "All" ) );
		else
			pTypes3.AppendMenu( MF_STRING, 3200, CString( "All" ) );

	pMenu.CreatePopupMenu();
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pHosts[0].GetSafeHmenu(), _T("Incoming") );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pHosts[1].GetSafeHmenu(), _T("Outgoing") );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pTypes1.GetSafeHmenu(), _T("G1 Types") );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pTypes2.GetSafeHmenu(), _T("G2 Types") );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pTypes3.GetSafeHmenu(), _T("ED2K Types") );
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
	else if ( nCmd >= 3000 && nCmd < 3000 + nTypeG1Size )
	{
		nCmd -= 3000;

		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			for ( int nType = 0 ; nType < nTypeG1Size ; nType++ )
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
	else if ( nCmd >= 3100 && nCmd < 3100 + nTypeG2Size )
	{
		nCmd -= 3100;

		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			for ( int nType = 0 ; nType < nTypeG2Size ; nType++ )
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
	else if ( nCmd == 3200 )
	{
		nCmd -= 3200;

		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			m_bTypeED = ( nCmd == nCmd ) ? TRUE : FALSE;
		}
		else
		{
			m_bTypeED = ! m_bTypeED;
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

void CPacketWnd::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	if ( m_pCoolMenu ) m_pCoolMenu->OnMeasureItem( lpMeasureItemStruct );
}

void CPacketWnd::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct) 
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


