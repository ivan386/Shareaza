//
// DCClients.cpp
//
// Copyright (c) Shareaza Development Team, 2010-2014.
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
#include "Connection.h"
#include "DCClient.h"
#include "DCClients.h"
#include "DCNeighbour.h"
#include "Downloads.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadTransferDC.h"
#include "GProfile.h"
#include "Neighbours.h"
#include "Network.h"
#include "TRansfers.h"
#include "ShareazaURL.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDCClients DCClients;

CDCClients::CDCClients()
{
}

CDCClients::~CDCClients()
{
	Clear();
}

void CDCClients::Clear()
{
	CSingleLock oLock( &m_pSection, TRUE );

	while ( ! m_pList.IsEmpty() )
	{
		CDCClient* pClient = m_pList.GetHead();

		oLock.Unlock();

		pClient->Remove();
		
		oLock.Lock();
	}
}

void CDCClients::Add(CDCClient* pClient)
{
	CQuickLock oLock( m_pSection );

	if ( m_pList.Find( pClient ) == NULL )
		m_pList.AddTail( pClient );
}

void CDCClients::Remove(CDCClient* pClient)
{
	CQuickLock oLock( m_pSection );

	if ( POSITION pos = m_pList.Find( pClient ) )
		m_pList.RemoveAt( pos );
}

int CDCClients::GetCount() const
{
	CQuickLock oLock( m_pSection );

	return (int)m_pList.GetCount();
}

void CDCClients::OnRun()
{
	CSingleLock oLock( &m_pSection, TRUE );

	for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
	{
		CDCClient* pClient = m_pList.GetNext( pos );

		if ( ! pClient->IsValid() )
		{
			oLock.Unlock();

			{
				CSingleLock oTransfersLock( &Transfers.m_pSection, FALSE );
				if ( ! oTransfersLock.Lock( 250 ) )
					return;

				pClient->OnRun();
			}

			oLock.Lock();
		}
	}
}

CString CDCClients::CreateNick(LPCTSTR szNick)
{
	CString sNick = ( szNick && *szNick )? szNick : MyProfile.GetNick();

	// Replace bad symbols
	sNick.Replace( _T(' '), _T('_') );
	sNick.Replace( _T('|'), _T('_') );
	sNick.Replace( _T('&'), _T('_') );
	sNick.Replace( _T('$'), _T('_') );

	// Generate nick if not set yet
	if ( sNick.IsEmpty() )
		sNick.Format( CLIENT_NAME_T _T("%04u"), GetRandomNum( 0u, 9999u ) );

	// Minimum length is 2
	while ( sNick.GetLength() < 2 ) sNick += _T("_");

	// Maximum length is 32
	return sNick.Left( 32 );
}

CDCClient* CDCClients::GetClient(const CString& sNick) const
{
	ASSUME_LOCK( m_pSection );

	if ( sNick.IsEmpty() )
		return NULL;

	Hashes::Guid oGUID;
	CDCClients::CreateGUID( sNick, oGUID );

	for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
	{
		CDCClient* pClient = m_pList.GetNext( pos );

		if ( validAndEqual( pClient->m_oGUID, oGUID ) )
		{
			return pClient;
		}
	}
	return NULL;
}

CDCNeighbour* CDCClients::GetHub(const CString& sNick) const
{
	ASSUME_LOCK( Network.m_pSection );

	for ( POSITION pos = Neighbours.GetIterator(); pos; )
	{
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );

		if ( pNeighbour->m_nProtocol == PROTOCOL_DC )
		{
			if ( static_cast< CDCNeighbour*>( pNeighbour )->GetUser( sNick ) )
			{
				return static_cast< CDCNeighbour*>( pNeighbour );
			}
		}
	}
	return NULL;
}

void CDCClients::CreateGUID(const CString& sNick, Hashes::Guid& oGUID)
{
	CMD5 pMD5;
	pMD5.Add( (LPCTSTR)sNick, sNick.GetLength() * sizeof( TCHAR ) );
	pMD5.Finish();
	pMD5.GetHash( &oGUID[ 0 ] );
	oGUID.validate();
}

BOOL CDCClients::ConnectTo(const IN_ADDR* pAddress, WORD nPort, CDCNeighbour* pHub, const CString& sRemoteNick)
{
	CSingleLock oDCLock( &m_pSection, FALSE );
	if ( ! oDCLock.Lock( 250 ) )
		// DC++ core overload
		return FALSE;

	// Try existing client first
	if ( CDCClient* pClient = GetClient( sRemoteNick ) )
	{
		if ( pClient->IsValid() )
			// Already connected
			return TRUE;

		return pClient->ConnectTo( pAddress, nPort );
	}

	// Create new one
	if ( CDCClient* pClient = new CDCClient( &pHub->m_pHost.sin_addr, ntohs( pHub->m_pHost.sin_port ), pHub->m_sNick, sRemoteNick ) )
	{
		return pClient->ConnectTo( pAddress, nPort );
	}

	return FALSE;
}

BOOL CDCClients::Connect(const IN_ADDR* pHubAddress, WORD nHubPort, const CString& sRemoteNick, BOOL& bSuccess)
{
	bSuccess = FALSE;

	if ( pHubAddress->s_addr == INADDR_ANY || nHubPort == 0 || sRemoteNick.IsEmpty() )
		return FALSE;

	CSingleLock oNetLock( &Network.m_pSection, FALSE );
	if ( ! oNetLock.Lock( 250 ) )
		// Network core overload
		return FALSE;

	CSingleLock oDCLock( &m_pSection, FALSE );
	if ( ! oDCLock.Lock( 250 ) )
		// DC++ core overload
		return FALSE;

	// Check existing clients
	CDCClient* pClient = GetClient( sRemoteNick );
	if ( pClient && pClient->IsValid() )
	{
		// Check if client can process new transfer requests
		if ( pClient->IsIdle() )
		{
			// Lets start new download using this already connected client
			pClient->OnPush();
		}
		return TRUE;
	}

	if ( ! pClient )
		pClient = new CDCClient( pHubAddress, nHubPort, NULL, sRemoteNick );
	if ( ! pClient )
		// Out of memory
		return FALSE;

	bSuccess = pClient->Connect();

	return TRUE;
}

BOOL CDCClients::OnAccept(CConnection* pConnection)
{
	if ( ! Network.IsConnected() || ( Settings.Connection.RequireForTransfers && ! Settings.DC.EnableToday ) )
	{
		theApp.Message( MSG_ERROR, _T("Refusing %s client link from %s because network is disabled."), protocolNames[ PROTOCOL_DC ], (LPCTSTR)pConnection->m_sAddress );
		return FALSE;
	}

	CSingleLock oTransfersLock( &Transfers.m_pSection );
	if ( oTransfersLock.Lock( 250 ) )
	{
		CSingleLock oDCLock( &m_pSection );
		if ( oDCLock.Lock( 250 ) )
		{
			if ( CDCClient* pClient = new CDCClient() )
			{
				pClient->AttachTo( pConnection );
			}
		}
		else
			theApp.Message( MSG_ERROR, _T("Rejecting %s connection from %s, network core overloaded."), protocolNames[ PROTOCOL_DC ], (LPCTSTR)pConnection->m_sAddress );
	}
	else
		theApp.Message( MSG_ERROR, _T("Rejecting %s connection from %s, network core overloaded."), protocolNames[ PROTOCOL_DC ], (LPCTSTR)pConnection->m_sAddress );

	return FALSE;
}

BOOL CDCClients::Merge(CDCClient* pClient)
{
	CQuickLock oLock( m_pSection );

	for ( POSITION pos = m_pList.GetHeadPosition(); pos; )
	{
		CDCClient* pOther = m_pList.GetNext( pos );

		if ( pOther != pClient && validAndEqual( pOther->m_oGUID, pClient->m_oGUID ) )
		{
			pClient->Merge( pOther );
			return TRUE;
		}
	}

	return FALSE;
}

std::string CDCClients::MakeKey(const std::string& aLock)
{
	if ( aLock.size() < 3 )
		return std::string();

	auto_array< BYTE > temp( new BYTE[ aLock.size() ] );
	size_t extra = 0;
	BYTE v1 = (BYTE)( (BYTE)aLock[ 0 ] ^ 5 );
	v1 = (BYTE)( ( ( v1 >> 4 ) | ( v1 << 4 ) ) & 0xff );
	temp[ 0 ] = v1;
	for ( size_t i = 1; i < aLock.size(); i++ )
	{
		v1 = (BYTE)( (BYTE)aLock[ i ] ^ (BYTE)aLock[ i - 1 ] );
		v1 = (BYTE)( ( ( v1 >> 4 ) | ( v1 << 4 ) ) & 0xff );
		temp[ i ] = v1;
		if ( IsExtra( temp[ i ] ) )
			extra++;
	}
	temp[ 0 ] = (BYTE)( temp[ 0 ] ^ temp[ aLock.size() - 1 ] );
	if ( IsExtra( temp[ 0 ] ) )
		extra++;

	return KeySubst( &temp[ 0 ], aLock.size(), extra );
}

std::string CDCClients::KeySubst(const BYTE* aKey, size_t len, size_t n)
{
	auto_array< BYTE > temp( new BYTE[ len + n * 9 ] );
	size_t j = 0;	
	for ( size_t i = 0; i < len; i++ )
	{
		if ( IsExtra( aKey[ i ] ) )
		{
			temp[ j++ ] = '/'; temp[ j++ ] = '%'; temp[ j++ ] = 'D';
			temp[ j++ ] = 'C'; temp[ j++ ] = 'N';
			switch ( aKey[ i ] )
			{
			case 0:   temp[ j++ ] = '0'; temp[ j++ ] = '0'; temp[ j++ ] = '0'; break;
			case 5:   temp[ j++ ] = '0'; temp[ j++ ] = '0'; temp[ j++ ] = '5'; break;
			case 36:  temp[ j++ ] = '0'; temp[ j++ ] = '3'; temp[ j++ ] = '6'; break;
			case 96:  temp[ j++ ] = '0'; temp[ j++ ] = '9'; temp[ j++ ] = '6'; break;
			case 124: temp[ j++ ] = '1'; temp[ j++ ] = '2'; temp[ j++ ] = '4'; break;
			case 126: temp[ j++ ] = '1'; temp[ j++ ] = '2'; temp[ j++ ] = '6'; break;
			}
			temp[ j++ ] = '%'; temp[ j++ ] = '/';
		}
		else
			temp[ j++ ] = aKey[ i ];
	}
	return std::string( (const char*)&temp[ 0 ], j );
}

BOOL CDCClients::IsExtra(BYTE b)
{
	return ( b == 0 || b == 5 || b == 124 || b == 96 || b == 126 || b == 36 );
}
