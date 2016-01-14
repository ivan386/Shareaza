//
// BTPacket.cpp
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
#include "BENode.h"
#include "BTClient.h"
#include "BTPacket.h"
#include "Buffer.h"
#include "Datagrams.h"
#include "Download.h"
#include "Downloads.h"
#include "GProfile.h"
#include "HostCache.h"
#include "Network.h"
#include "Security.h"
#include "Settings.h"
#include "Statistics.h"
#include "Transfers.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDHT DHT;

extern "C"
{
	// Sources: https ://github.com/jech/dht
	// Info : http ://www.pps.univ-paris-diderot.fr/~jch/software/bittorrent/

	#pragma warning(push,2)

	#ifndef _CRT_SECURE_NO_WARNINGS
		#define _CRT_SECURE_NO_WARNINGS
	#endif

	#include "dht/dht.h"
	#include "dht/dht.c"

	#pragma warning(pop)

	// Callback functions:

	int dht_blacklisted(const struct sockaddr *sa, int salen)
	{
		if ( salen != sizeof( SOCKADDR_IN ) )
			// IPv6 not supported
			return 1;

		const SOCKADDR_IN* pHost = (const SOCKADDR_IN*)sa;

		return ( pHost->sin_port == 0 ||
			Network.IsFirewalledAddress( &pHost->sin_addr, Settings.Connection.IgnoreOwnIP ) ||
			Network.IsReserved( &pHost->sin_addr ) ||
			Security.IsDenied( &pHost->sin_addr ) ) ? 1 : 0;
	}

	void dht_hash(void *hash_return, int hash_size, const void *v1, int len1, const void *v2, int len2, const void *v3, int len3)
	{
		CMD5 md5;
		md5.Add( v1, len1);
		md5.Add( v2, len2);
		md5.Add( v3, len3);
		md5.Finish();
		CMD5::Digest pDataMD5;
		md5.GetHash( (unsigned char*)&pDataMD5[ 0 ] );
		if ( hash_size > 16 )
			memset( (char*)hash_return + 16, 0, hash_size - 16 );
		memcpy( hash_return, &pDataMD5[ 0 ], ( hash_size > 16 ) ? 16 : hash_size );
	}

	int dht_random_bytes(void *buf, size_t size)
	{
		return CryptGenRandom( theApp.m_hCryptProv, (DWORD)size, (BYTE*)buf ) ? 0 : -1;
	}

	int dht_sendto(int /*s*/, const void *buf, int len, int /*flags*/, const struct sockaddr *to, int tolen)
	{
		if ( tolen != sizeof( SOCKADDR_IN ) )
			// IPv6 not supported
			return -1;

		const SOCKADDR_IN* pHost = (const SOCKADDR_IN*)to;

		// TODO: Remove extra copy
		CBTPacket* pPacket = CBTPacket::New( BT_PACKET_EXTENSION, BT_EXTENSION_NOP, (const BYTE*)buf, len );

		return Datagrams.Send( pHost, pPacket ) ?  len : -1;
	}
}

CDHT::CDHT()
	: m_bConnected( false )
{
}

void CDHT::Connect()
{
	ASSUME_LOCK( Network.m_pSection );

	if ( m_bConnected || ! Settings.BitTorrent.EnableToday || ! Settings.BitTorrent.EnableDHT )
		return;

	Hashes::BtGuid oID = MyProfile.oGUIDBT;
	if ( dht_init( 0, -1, &oID[ 0 ], theApp.m_pBTVersion ) >= 0 )
	{
		CQuickLock oLock( HostCache.BitTorrent.m_pSection );

		int nCount = 0;
		for ( CHostCacheIterator i = HostCache.BitTorrent.Begin() ; i != HostCache.BitTorrent.End() && nCount < 100; ++i )
		{
			CHostCacheHostPtr pCache = (*i);

			if ( pCache->m_oBtGUID )
			{
				SOCKADDR_IN sa = { AF_INET, htons( pCache->m_nPort ), pCache->m_pAddress };
				dht_insert_node( &pCache->m_oBtGUID[ 0 ], (sockaddr*)&sa, sizeof( SOCKADDR_IN ) );
				nCount++;
			}
		}

		m_bConnected = true;
	}
}

void CDHT::Disconnect()
{
	ASSUME_LOCK( Network.m_pSection );

	if ( m_bConnected )
	{
		int nCount = 100, nZero = 0;
		CAutoVectorPtr< SOCKADDR_IN > pHosts( new SOCKADDR_IN[ nCount ] );
		CAutoVectorPtr< unsigned char > pIDs( new unsigned char[ nCount * Hashes::BtGuid::byteCount ] );
		if ( dht_get_nodes( pHosts, pIDs, &nCount, NULL, NULL, &nZero ) >= 0 )
		{
			CQuickLock oLock( HostCache.BitTorrent.m_pSection );

			for ( int i = 0; i < nCount; ++i )
			{
				if ( CHostCacheHostPtr pCache = HostCache.BitTorrent.Add( &pHosts[ i ].sin_addr, pHosts[ i].sin_port ) )
				{
					CopyMemory( &pCache->m_oBtGUID[ 0 ], &pIDs[ i * Hashes::BtGuid::byteCount ], Hashes::BtGuid::byteCount );
					pCache->m_oBtGUID.validate();
				}
			}
		}
		dht_uninit();

		m_bConnected = false;
	}
}

void CDHT::Search(const Hashes::BtHash& oBTH, bool bAnnounce)
{
	if ( ! m_bConnected || ! Settings.BitTorrent.EnableToday || ! Settings.BitTorrent.EnableDHT )
		return;

	CSingleLock oLock( &Network.m_pSection, FALSE );
	if ( oLock.Lock( 250 ) )
	{
		dht_search( &oBTH[ 0 ], ( bAnnounce ? Network.GetPort() : 0 ), AF_INET, &CDHT::OnEvent, NULL );
	}
}

bool CDHT::Ping(const IN_ADDR* pAddress, WORD nPort)
{
	if ( ! m_bConnected || ! Settings.BitTorrent.EnableToday || ! Settings.BitTorrent.EnableDHT )
		return false;

	CSingleLock oNetworkLock( &Network.m_pSection, FALSE );
	if ( ! oNetworkLock.Lock( 250 ) )
		return false;
	
	{
		CSingleLock oLock( &HostCache.BitTorrent.m_pSection, FALSE );
		if ( oLock.Lock( 250 ) )
		{
			if ( CHostCacheHostPtr pCache = HostCache.BitTorrent.Add( pAddress, nPort ) )
			{
				pCache->m_tConnect = static_cast< DWORD >( time( NULL ) );
				if ( pCache->m_tAck == 0 )
					pCache->m_tAck = pCache->m_tConnect;

				HostCache.BitTorrent.m_nCookie++;
			}
		}
	}

	SOCKADDR_IN sa = { AF_INET, htons( nPort ), *pAddress };
	return dht_ping_node( (sockaddr*)&sa, sizeof( sa ) ) != -1;
}

void CDHT::OnRun()
{
	ASSUME_LOCK( Network.m_pSection );

	if ( ! Settings.BitTorrent.EnableToday || ! Settings.BitTorrent.EnableDHT )
	{
		if ( m_bConnected )
		{
			Disconnect();
		}
		return;
	}
	else
	{
		if ( ! m_bConnected )
		{
			Connect();
		}
	}

	int nNodes = dht_nodes( AF_INET, NULL, NULL, NULL, NULL );
	if ( nNodes == 0 )
	{
		// Need a node
		CSingleLock oLock( &HostCache.BitTorrent.m_pSection, FALSE );
		if ( oLock.Lock( 250 ) )
		{
			DWORD tNow = static_cast< DWORD >( time( NULL ) );

			// Ping most recent node
			for ( CHostCacheIterator i = HostCache.BitTorrent.Begin(); i != HostCache.BitTorrent.End(); ++i )
			{
				CHostCacheHostPtr pCache = (*i);

				if ( pCache->CanConnect( tNow ) )
				{
					pCache->ConnectTo( TRUE );
					break;
				}
			}
		}
	}

	time_t tosleep = 0;
	dht_periodic( NULL, 0, NULL, 0, &tosleep, &CDHT::OnEvent, NULL );
}

void CDHT::OnPacket(const SOCKADDR_IN* pHost, CBTPacket* pPacket)
{
	ASSUME_LOCK( Network.m_pSection );

	if ( ! m_bConnected || ! Settings.BitTorrent.EnableToday || ! Settings.BitTorrent.EnableDHT )
		return;

	// TODO: Remove extra copy
	CBuffer pBufffer;
	pPacket->ToBuffer( &pBufffer, false );
	pBufffer.Add( "", 1 );	// zero terminated

	time_t tosleep = 0;
	dht_periodic( pBufffer.m_pBuffer, pBufffer.m_nLength - 1, (sockaddr*)pHost, sizeof( SOCKADDR_IN ), &tosleep, &CDHT::OnEvent, NULL );
}

void CDHT::OnEvent(void* /*closure*/, int evt, const unsigned char* info_hash, const void* data, size_t data_len)
{
	switch ( evt )
	{
	case DHT_EVENT_ADDED:
	case DHT_EVENT_SENT:
		if ( data_len == sizeof( SOCKADDR_IN ) )
		{
			const SOCKADDR_IN* pHost = (const SOCKADDR_IN*)data;

			CQuickLock oLock( HostCache.BitTorrent.m_pSection );

			// Node just added
			if ( CHostCacheHostPtr pCache = HostCache.BitTorrent.Add( &pHost->sin_addr, htons( pHost->sin_port ) ) )
			{
				CopyMemory( &pCache->m_oBtGUID[ 0 ], info_hash, Hashes::BtGuid::byteCount );
				pCache->m_oBtGUID.validate();

				HostCache.BitTorrent.m_nCookie++;
			}
		}
		break;

	case DHT_EVENT_REPLY:
		if ( data_len == sizeof( SOCKADDR_IN ) )
		{
			// Assume UDP is stable
			Datagrams.SetStable();

			const SOCKADDR_IN* pHost = (const SOCKADDR_IN*)data;

			CQuickLock oLock( HostCache.BitTorrent.m_pSection );

			// Got reply from node
			if ( CHostCacheHostPtr pCache = HostCache.BitTorrent.OnSuccess( &pHost->sin_addr, htons( pHost->sin_port ) ) )
			{
				pCache->m_tAck = 0;

				HostCache.BitTorrent.m_nCookie++;
			}
		}
		break;

	case DHT_EVENT_REMOVED:
		if ( data_len == sizeof( SOCKADDR_IN ) )
		{
			const SOCKADDR_IN* pHost = (const SOCKADDR_IN*)data;
			
			CQuickLock oLock( HostCache.BitTorrent.m_pSection );

			HostCache.BitTorrent.Remove( &pHost->sin_addr );

			HostCache.BitTorrent.m_nCookie++;
		}
		break;

	case DHT_EVENT_VALUES:
		{
			CSingleLock oLock( &Transfers.m_pSection, FALSE );
			if ( oLock.Lock( 250 ) )
			{
				Hashes::BtHash oHash;
				CopyMemory( &oHash[ 0 ], info_hash, Hashes::BtHash::byteCount );
				oHash.validate();

				if ( CDownload* pDownload = Downloads.FindByBTH( oHash ) )
				{
					size_t nCount = data_len / 6;
					for ( size_t i = 0; i < nCount; ++i )
					{
						const char* p = &((const char*)data)[ i * 6 ];
						pDownload->AddSourceBT( Hashes::BtGuid(), (IN_ADDR*)p, ntohs( *(WORD*)(p + 4) ) );
					}
				}
			}
		}
		break;
	case DHT_EVENT_VALUES6:
	case DHT_EVENT_SEARCH_DONE:
	case DHT_EVENT_SEARCH_DONE6:
	default:
		break;
	}
}

CBTPacket::CBTPacketPool CBTPacket::POOL;

//////////////////////////////////////////////////////////////////////
// CBTPacket construction

CBTPacket::CBTPacket()
	: CPacket		( PROTOCOL_BT )
	, m_nType		( BT_PACKET_EXTENSION )
	, m_nExtension	( BT_EXTENSION_NOP )
	, m_pNode		( new CBENode() )
{
}

CBTPacket::~CBTPacket()
{
}

void CBTPacket::Reset()
{
	CPacket::Reset();

	m_nType			= BT_PACKET_EXTENSION;
	m_nExtension	= BT_EXTENSION_NOP;
	m_pNode.reset	( new CBENode() );
}

CBTPacket* CBTPacket::New(BYTE nType, BYTE nExtension, const BYTE* pBuffer, DWORD nLength)
{
	CBTPacket* pPacket = (CBTPacket*)POOL.New();
	ASSERT( pPacket && pPacket->m_pNode.get() );
	if ( pPacket )
	{
		pPacket->m_nType = nType;
		pPacket->m_nExtension = nExtension;
		if ( pBuffer && nLength )
		{
			DWORD nRead = 0;
			if ( IsEncoded( nType ) )
			{
				pPacket->m_pNode.reset( CBENode::Decode( pBuffer, nLength, &nRead ) );
				if ( ! pPacket->m_pNode.get() )
				{
					pPacket->Release();
					return NULL;
				}
			}
			// Read rest of packet (if any) as raw data
			if ( nLength > nRead )
			{
				if ( ! pPacket->Write( pBuffer + nRead, nLength - nRead ) )
				{
					pPacket->Release();
					return NULL;
				}
			}
		}
	}
	return pPacket;
}

bool CBTPacket::HasEncodedData() const
{
	return m_pNode.get() && ! m_pNode->IsType( CBENode::beNull );
}

//////////////////////////////////////////////////////////////////////
// CBTPacket serialize

void CBTPacket::ToBuffer(CBuffer* pBuffer, bool /*bTCP*/)
{
	if ( m_nType == BT_PACKET_KEEPALIVE )
	{
		// Keep-Alive packet
		DWORD nZero = 0;
		pBuffer->Add( &nZero, 4 );
	}
	else if ( m_nType == BT_PACKET_EXTENSION && m_nExtension == BT_EXTENSION_NOP )
	{
		// Packet without header
		if ( HasEncodedData() )
		{
			m_pNode->Encode( pBuffer );
		}
		pBuffer->Add( m_pBuffer, m_nLength );
	}
	else
	{
		// Full packet

		// Reserve memory for packet length field
		DWORD nZero = 0;
		pBuffer->Add( &nZero, 4 );
		DWORD nOldLength = pBuffer->m_nLength;

		// Add packet type
		pBuffer->Add( &m_nType, 1 );

		if ( m_nType == BT_PACKET_EXTENSION )
		{
			// Add packet extension
			pBuffer->Add( &m_nExtension, 1 );
		}

		// Add bencoded data
		if ( HasEncodedData() )
		{
			m_pNode->Encode( pBuffer );
		}

		// Add payload
		pBuffer->Add( m_pBuffer, m_nLength );

		// Set packet total length
		*(DWORD*)( pBuffer->m_pBuffer + nOldLength - 4 ) =
			swapEndianess( pBuffer->m_nLength - nOldLength );
	}

	ASSERT( pBuffer->m_nLength );
}

//////////////////////////////////////////////////////////////////////
// CBTPacket un-serialize

CBTPacket* CBTPacket::ReadBuffer(CBuffer* pBuffer)
{
	DWORD nLength = (DWORD) - 1;
	bool bKeepAlive = false;
	bool bValid = true;

	// Skip subsequent keep-alive packets
	do
	{
		if ( pBuffer->m_nLength < sizeof( DWORD ) )
			bValid = false;
		else
		{
			nLength = transformFromBE( pBuffer->ReadDWORD() );
			if ( pBuffer->m_nLength - sizeof( DWORD ) < nLength )
				bValid = false;
		}

		if ( !bKeepAlive && nLength == 0 )
			bKeepAlive = true;

		if ( bValid )
			pBuffer->Remove( sizeof( DWORD ) );		// remove size marker
	}
	while ( bKeepAlive && bValid && nLength == 0 );

	CBTPacket* pPacket = NULL;
	if ( bKeepAlive )
	{
		pPacket = CBTPacket::New( BT_PACKET_KEEPALIVE );
	}
	else if ( bValid )
	{
		if ( pBuffer->m_pBuffer[ 0 ] == BT_PACKET_EXTENSION )
		{
			// Read extension packet
			pPacket = CBTPacket::New( BT_PACKET_EXTENSION,
				pBuffer->m_pBuffer[ 1 ], pBuffer->m_pBuffer + 2, nLength - 2 );
		}
		else
		{
			// Read standard packet
			pPacket = CBTPacket::New( pBuffer->m_pBuffer[ 0 ],
				BT_EXTENSION_NOP, pBuffer->m_pBuffer + 1, nLength - 1 );
		}

		pBuffer->Remove( nLength );
	}

	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CBTPacket debugging
void CBTPacket::SmartDump(const SOCKADDR_IN* pAddress, BOOL bUDP, BOOL bOutgoing, DWORD_PTR nNeighbourUnique)
{
	switch ( m_nType )
	{
	case BT_PACKET_REQUEST:
	case BT_PACKET_PIECE:
	case BT_PACKET_HAVE:
		// Ignore uninterested packets
		return;
	}

	CPacket::SmartDump( pAddress, bUDP, bOutgoing, nNeighbourUnique );
}

CString CBTPacket::GetType() const
{
	CString sType;
	switch ( m_nType )
	{
	case BT_PACKET_CHOKE:
		sType = _T("Choke");
		break;
	case BT_PACKET_UNCHOKE:
		sType = _T("Unchoke");
		break;
	case BT_PACKET_INTERESTED:
		sType = _T("Interested");
		break;
	case BT_PACKET_NOT_INTERESTED:
		sType = _T("NotInterested");
		break;
	case BT_PACKET_HAVE:
		sType = _T("Have");
		break;
	case BT_PACKET_BITFIELD:
		sType = _T("Bitfield");
		break;
	case BT_PACKET_REQUEST:
		sType = _T("Request");
		break;
	case BT_PACKET_PIECE:
		sType = _T("Piece");
		break;
	case BT_PACKET_CANCEL:
		sType = _T("Cancel");
		break;
	case BT_PACKET_DHT_PORT:
		sType = _T("DHT port");
		break;
	case BT_PACKET_EXTENSION:
		switch( m_nExtension )
		{
		case BT_EXTENSION_HANDSHAKE:
			sType = _T("Handshake");
			break;
		case BT_EXTENSION_NOP:
			sType = _T("DHT");
			break;
		default:
			sType.Format( _T("Ext %02u"), m_nExtension );
		}
		break;
	case BT_PACKET_HANDSHAKE:
		sType = _T("ExtHandshake");
		break;
	case BT_PACKET_SOURCE_REQUEST:
		sType = _T("SrcRequest");
		break;
	case BT_PACKET_SOURCE_RESPONSE:
		sType = _T("SrcResponse");
		break;
	case BT_PACKET_KEEPALIVE:
		sType = _T("Keep-Alive");
		break;
	default:
		sType.Format( _T("%u"), m_nType );
	}
	return sType;
}

CString CBTPacket::ToHex() const
{
	return CPacket::ToHex();
}

CString CBTPacket::ToASCII() const
{
	switch ( m_nType )
	{
	case BT_PACKET_DHT_PORT:
		{
			CString sPort;
			sPort.Format( _T("port: %u"), ntohs( *(WORD*)m_pBuffer ) );
			return sPort;
		}
	case BT_PACKET_BITFIELD:
		{
			CString sLength;
			sLength.Format( _T("length: %u bytes"), m_nLength );
			return sLength;
		}
	}
	return HasEncodedData() ? m_pNode->Encode() : CPacket::ToASCII();
}

BOOL CBTPacket::OnPacket(const SOCKADDR_IN* pHost)
{
	ASSERT( m_nType == BT_PACKET_EXTENSION );
	ASSERT( m_nExtension == BT_EXTENSION_NOP );

	Statistics.Current.BitTorrent.Incoming++;

	SmartDump( pHost, TRUE, FALSE );

	if ( ! m_pNode->IsType( CBENode::beDict ) )
		return FALSE;

	if ( ! Settings.BitTorrent.EnableToday || ! Settings.BitTorrent.EnableDHT )
		return TRUE;

	{
		CQuickLock oLock( HostCache.BitTorrent.m_pSection );

		if ( CHostCacheHostPtr pCache = HostCache.BitTorrent.OnSuccess( &pHost->sin_addr, htons( pHost->sin_port ) ) )
		{
			// Get version
			const CBENode* pVersion = m_pNode->GetNode( BT_DICT_VENDOR );
			if ( pVersion && pVersion->IsType( CBENode::beString ) )
			{
				pCache->m_sName = CBTClient::GetAzureusStyleUserAgent( (LPBYTE)pVersion->m_pValue, 4 );
			}

			HostCache.BitTorrent.m_nCookie++;
		}
	}

	const CBENode* pYourIP = m_pNode->GetNode( BT_DICT_YOURIP );
	if ( pYourIP && pYourIP->IsType( CBENode::beString ) )
	{
		if ( pYourIP->m_nValue == 4 )
		{
			// IPv4
			Network.AcquireLocalAddress( *(const IN_ADDR*)pYourIP->m_pValue );
		}
	}

	DHT.OnPacket( pHost, this );

	return TRUE;
/*
	// Get packet type and transaction id
	const CBENode* pType = m_pNode->GetNode( BT_DICT_TYPE );
	const CBENode* pTransID = m_pNode->GetNode( BT_DICT_TRANSACT_ID );
	if ( ! pType ||
		 ! pType->IsType( CBENode::beString ) ||
		 ! pTransID ||
		 ! pTransID->IsType( CBENode::beString ) )
		 return FALSE;

	CString sType = pType->GetString();
	if ( sType == BT_DICT_QUERY )
	{
		// Query message
		const CBENode* pQueryMethod = m_pNode->GetNode( BT_DICT_QUERY );
		if ( ! pQueryMethod ||
			 ! pQueryMethod->IsType( CBENode::beString ) )
			return FALSE;

		CString sQueryMethod = pQueryMethod->GetString();
		if ( sQueryMethod == BT_DICT_PING )
		{
			// Ping
			return OnPing( pHost );
		}
		else if ( sQueryMethod == BT_DICT_FIND_NODE )
		{
			// TODO: Find node
		}
		else if ( sQueryMethod == BT_DICT_GET_PEERS )
		{
			// TODO: Get peers
		}
		else if ( sQueryMethod == BT_DICT_ANNOUNCE_PEER )
		{
			// TODO: Announce peer
		}
		// else if ( sQueryMethod == BT_DICT_ERROR_LONG ) - ???

		return TRUE;
	}
	else if ( sType == BT_DICT_RESPONSE )
	{
		// Response message
		const CBENode* pResponse = m_pNode->GetNode( BT_DICT_RESPONSE );
		if ( ! pResponse ||
			 ! pResponse->IsType( CBENode::beDict ) )
			 return FALSE;

		Hashes::BtGuid oNodeGUID;
		const CBENode* pNodeID = pResponse->GetNode( BT_DICT_ID );
		if ( ! pNodeID ||
			 ! pNodeID->GetString( oNodeGUID ) )
			return FALSE;

		pCache->m_oBtGUID = oNodeGUID;
		pCache->m_sDescription = oNodeGUID.toString();

		// TODO: Check queries pool for pTransID

		// Save access token
		const CBENode* pToken = pResponse->GetNode( BT_DICT_TOKEN );
		if ( pToken && pToken->IsType( CBENode::beString ) )
		{
			pCache->m_Token.SetSize( (INT_PTR)pToken->m_nValue );
			CopyMemory( pCache->m_Token.GetData(), pToken->m_pValue, (size_t)pToken->m_nValue );
		}

		const CBENode* pPeers = pResponse->GetNode( BT_DICT_VALUES );
		if ( pPeers && pPeers->IsType( CBENode::beList) )
		{
		}

		const CBENode* pNodes = pResponse->GetNode( BT_DICT_NODES );
		if ( pNodes && pNodes->IsType( CBENode::beString ) )
		{
		}

		return TRUE;
	}
	else if ( sType == BT_DICT_ERROR )
	{
		// Error message
		const CBENode* pError = m_pNode->GetNode( BT_DICT_ERROR );
		if ( ! pError ||
			 ! pError->IsType( CBENode::beList ) )
			return FALSE;

		return OnError( pHost );
	}

	return FALSE;*/
}

//BOOL CBTPacket::OnPing(const SOCKADDR_IN* pHost)
//{
//	const CBENode* pTransID = m_pNode->GetNode( BT_DICT_TRANSACT_ID );
//
//	const CBENode* pQueryData = m_pNode->GetNode( BT_DICT_DATA );
//	if ( ! pQueryData ||
//		 ! pQueryData->IsType( CBENode::beDict ) )
//		return FALSE;
//
//	Hashes::BtGuid oNodeGUID;
//	const CBENode* pNodeID = pQueryData->GetNode( BT_DICT_ID );
//	if ( ! pNodeID ||
//		 ! pNodeID->GetString( oNodeGUID ) )
//		return FALSE;
//
//	{
//		CQuickLock oLock( HostCache.BitTorrent.m_pSection );
//
//		CHostCacheHostPtr pCache = HostCache.BitTorrent.Add( &pHost->sin_addr, htons( pHost->sin_port ) );
//		if ( pCache )
//		{
//			pCache->m_oBtGUID = oNodeGUID;
//			pCache->m_sDescription = oNodeGUID.toString();
//
//			HostCache.BitTorrent.m_nCookie++;
//		}
//	}
//
//	// Send pong
//
//	CBTPacket* pPacket = CBTPacket::New();
//	CBENode* pRoot = pPacket->m_pNode.get();
//	ASSERT( pRoot );
//
//	pRoot->Add( BT_DICT_RESPONSE )->Add( BT_DICT_ID )->SetString( MyProfile.oGUIDBT );
//	pRoot->Add( BT_DICT_TYPE )->SetString( BT_DICT_RESPONSE );
//	pRoot->Add( BT_DICT_TRANSACT_ID )->SetString( (LPCSTR)pTransID->m_pValue, (size_t)pTransID->m_nValue );
//	pRoot->Add( BT_DICT_VENDOR )->SetString( theApp.m_pBTVersion, 4 );
//
//	Datagrams.Send( pHost, pPacket );
//
//	return TRUE;
//}

//BOOL CBTPacket::OnError(const SOCKADDR_IN* /*pHost*/)
//{
//	return TRUE;
//}

/*BOOL CBTPacket::Ping(const SOCKADDR_IN* pHost)
{
	CBTPacket* pPingPacket = CBTPacket::New();
	CBENode* pPing = pPingPacket->m_pNode.get();
	pPing->Add( BT_DICT_DATA )->Add( BT_DICT_ID )->SetString( MyProfile.oGUIDBT );
	pPing->Add( BT_DICT_TYPE )->SetString( BT_DICT_QUERY );
	pPing->Add( BT_DICT_TRANSACT_ID )->SetString( "1234" ); // TODO
	pPing->Add( BT_DICT_QUERY )->SetString( BT_DICT_PING );
	pPing->Add( BT_DICT_VENDOR )->SetString( theApp.m_pBTVersion, 4 );
	return Datagrams.Send( pHost, pPingPacket );
}*/

/*BOOL CBTPacket::GetPeers(const SOCKADDR_IN* pHost, const Hashes::BtGuid& oNodeGUID, const Hashes::BtHash& oGUID)
{
	CBENode pGetPeers;
	CBENode* pGetPeersData = pGetPeers.Add( BT_DICT_DATA );
	pGetPeersData->Add( BT_DICT_ID )->SetString( oNodeGUID );
	pGetPeersData->Add( "info_hash" )->SetString( oGUID );
	pGetPeers.Add( BT_DICT_TYPE )->SetString( BT_DICT_QUERY );
	pGetPeers.Add( BT_DICT_TRANSACT_ID )->SetString( "4567" ); // TODO
	pGetPeers.Add( BT_DICT_QUERY )->SetString( BT_DICT_GET_PEERS );
	pGetPeers.Add( BT_DICT_VENDOR )->SetString( theApp.m_pBTVersion, 4 );
	CBuffer pOutput;
	pGetPeers.Encode( &pOutput );
	return Datagrams.Send( pHost, pOutput );
}*/