//
// BTClients.cpp
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
#include "Transfers.h"
#include "BTClients.h"
#include "BTClient.h"
#include "BTTrackerRequest.h"
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
	m_bShutdown = FALSE;
}

CBTClients::~CBTClients()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CBTClients clear

void CBTClients::Clear()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->Close();
	}
	
	ShutdownRequests();
}


//////////////////////////////////////////////////////////////////////
// CBTClients GUID->SHA1 filter	

  //Note: This was removed and placed in the transfer, after a request from people running trackers.
  // They wanted a per-download generated ID, not static. (Do not retain between sessions.)

  //Note 2: Official spec says "Generate per download", which is as per above.
  // Unofficial spec says "Generate at startup", which is how it used to be. Which is correct?

/*
SHA1* CBTClients::GetGUID()
{
	(GGUID&)m_pGUID = MyProfile.GUID;

	for ( int nByte = 16 ; nByte < 20 ; nByte++ )
	{
		m_pGUID.n[ nByte ]	= MyProfile.GUID.n[ nByte % 16 ]
							^ MyProfile.GUID.n[ 15 - ( nByte % 16 ) ];
	}

	return &m_pGUID;
}
*/

//////////////////////////////////////////////////////////////////////
// CBTClients accept new connections

BOOL CBTClients::OnAccept(CConnection* pConnection)
{
	ASSERT( pConnection != NULL );
	
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	CBTClient* pClient = new CBTClient();
	pClient->AttachTo( pConnection );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClients add and remove

void CBTClients::Add(CBTClient* pClient)
{
	ASSERT( m_pList.Find( pClient ) == NULL );
	m_pList.AddHead( pClient );
}

void CBTClients::Remove(CBTClient* pClient)
{
	POSITION pos = m_pList.Find( pClient );
	ASSERT( pos != NULL );
	m_pList.RemoveAt( pos );
}

//////////////////////////////////////////////////////////////////////
// CBTClients request thread management

void CBTClients::Add(CBTTrackerRequest* pRequest)
{
	CSingleLock pLock( &m_pSection, TRUE );
	ASSERT( ! m_bShutdown );
	m_pRequests.AddTail( pRequest );
}

void CBTClients::Remove(CBTTrackerRequest* pRequest)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( POSITION pos = m_pRequests.Find( pRequest ) ) m_pRequests.RemoveAt( pos );
	if ( ! m_bShutdown || m_pRequests.GetCount() > 0 ) return;
	pLock.Unlock();
	m_pShutdown.SetEvent();
}

void CBTClients::ShutdownRequests()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	if ( m_pRequests.GetCount() == 0 ) return;
	m_bShutdown = TRUE;
	pLock.Unlock();
	
	if ( WaitForSingleObject( m_pShutdown, 5000 ) == WAIT_OBJECT_0 ) return;
	
	while ( TRUE )
	{
		pLock.Lock();
		if ( m_pRequests.GetCount() == 0 ) break;
		CBTTrackerRequest* pRequest = (CBTTrackerRequest*)m_pRequests.RemoveHead();
		HANDLE hThread = pRequest->m_hThread;
		pLock.Unlock();
		CHttpRequest::CloseThread( &hThread, _T("CBTTrackerRequest") );
	}
}
