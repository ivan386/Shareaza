//
// BTClients.cpp
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
#include "Network.h"
#include "Transfers.h"
#include "BTClients.h"
#include "BTClient.h"
#include "GProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CBTClients BTClients;


//////////////////////////////////////////////////////////////////////
// CBTClients construction

CBTClients::CBTClients()
{
}

CBTClients::~CBTClients()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CBTClients clear

void CBTClients::Clear()
{
	CSingleLock oLock( &m_pListSection, TRUE );
	while ( ! m_pList.IsEmpty() )
	{
		oLock.Unlock();
		m_pList.GetHead()->Close();
		oLock.Lock();
	}
}

//////////////////////////////////////////////////////////////////////
// CBTClients accept new connections

BOOL CBTClients::OnAccept(CConnection* pConnection)
{
	if ( ! Network.IsConnected() || ( Settings.Connection.RequireForTransfers && ! Settings.BitTorrent.EnableToday ) )
	{
		theApp.Message( MSG_ERROR, IDS_BT_CLIENT_DROP_CONNECTED, (LPCTSTR)pConnection->m_sAddress );
		return FALSE;
	}

	CSingleLock pLock( &Transfers.m_pSection );
	if ( pLock.Lock( 250 ) )
	{
		if ( CBTClient* pClient = new CBTClient() )
		{
			pClient->AttachTo( pConnection );
		}
	}
	else
		theApp.Message( MSG_ERROR, _T("Rejecting %s connection from %s, network core overloaded."), protocolNames[ PROTOCOL_BT ], (LPCTSTR)pConnection->m_sAddress );

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CBTClients add and remove

void CBTClients::Add(CBTClient* pClient)
{
	CQuickLock oLock( m_pListSection );

	ASSERT( m_pList.Find( pClient ) == NULL );
	m_pList.AddHead( pClient );
}

void CBTClients::Remove(CBTClient* pClient)
{
	CQuickLock oLock( m_pListSection );

	POSITION pos = m_pList.Find( pClient );
	ASSERT( pos != NULL );
	m_pList.RemoveAt( pos );
}
