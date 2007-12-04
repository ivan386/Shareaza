//
// Datagrams.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "Statistics.h"
#include "Network.h"
#include "Datagrams.h"
#include "Datagram.h"
#include "DatagramPart.h"
#include "Buffer.h"

#include "Handshakes.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "RouteCache.h"
#include "LocalSearch.h"
#include "SearchManager.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "GProfile.h"
#include "CrawlSession.h"

#include "GGEP.h"
#include "G1Neighbour.h"
#include "G2Neighbour.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "EDClients.h"
#include "EDPacket.h"
#include "BENode.h"
#include "BTClient.h"
#include "Security.h"
#include "HostCache.h"
#include "DiscoveryServices.h"
#include "QueryKeys.h"
#include "LibraryMaps.h"
#include "VendorCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define HASH_SIZE		32
#define HASH_MASK		31

#define METER_MINIMUM	100
#define METER_LENGTH	24
#define METER_PERIOD	2000
#define METER_SECOND	1000

CDatagrams Datagrams;


//////////////////////////////////////////////////////////////////////
// CDatagrams construction

CDatagrams::CDatagrams()
{
	m_hSocket	= INVALID_SOCKET;
	m_nSequence	= 0;
	m_bStable	= FALSE;

	ZeroMemory( &m_mInput, sizeof(m_mInput) );
	ZeroMemory( &m_mOutput, sizeof(m_mOutput) );

	m_nInBandwidth	= m_nInFrags	= m_nInPackets	= 0;
	m_nOutBandwidth	= m_nOutFrags	= m_nOutPackets	= 0;
}

CDatagrams::~CDatagrams()
{
	Disconnect();
}

//////////////////////////////////////////////////////////////////////
// CDatagrams listen

BOOL CDatagrams::Listen()
{
	if ( m_hSocket != INVALID_SOCKET ) return FALSE;

	m_hSocket = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( m_hSocket == INVALID_SOCKET ) return FALSE;

	const BOOL bEnable = TRUE;
	VERIFY( setsockopt( m_hSocket, SOL_SOCKET, SO_BROADCAST,
		(char*)&bEnable, sizeof( bEnable ) ) == 0 );

	SOCKADDR_IN saHost;

	if ( Network.Resolve( Settings.Connection.InHost, Settings.Connection.InPort, &saHost ) )
	{
		// Inbound resolved
		if ( ! Settings.Connection.InBind ) 
			saHost.sin_addr.S_un.S_addr = 0;
		else
		{
			// Set the exclusive address option
			BOOL bVal = TRUE;
			setsockopt( m_hSocket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&bVal, sizeof(bVal) );
		}
	}
	else if ( Network.Resolve( Settings.Connection.OutHost, Settings.Connection.InPort, &saHost ) )
	{
		// Outbound resolved
	}
	else
	{
		saHost = Network.m_pHost;
		if ( ! Settings.Connection.InBind ) 
			saHost.sin_addr.S_un.S_addr = 0;
		else
		{
			// Set the exclusive address option
			BOOL bVal = TRUE;
			setsockopt( m_hSocket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&bVal, sizeof(bVal) );
		}
	}

	if ( bind( m_hSocket, (SOCKADDR*)&saHost, sizeof(saHost) ) == 0 )
	{
		theApp.Message( MSG_DEFAULT, IDS_NETWORK_LISTENING_UDP,
			(LPCTSTR)CString( inet_ntoa( saHost.sin_addr ) ), htons( saHost.sin_port ) );
	}

	WSAEventSelect( m_hSocket, Network.m_pWakeup, FD_READ );

	m_nBufferBuffer	= Settings.Gnutella2.UdpBuffers; // 256;
	m_pBufferBuffer	= new CBuffer[ m_nBufferBuffer ];
	m_pBufferFree	= m_pBufferBuffer;
	m_nBufferFree	= m_nBufferBuffer;

	CBuffer* pBuffer = m_pBufferBuffer;

	for ( DWORD nPos = m_nBufferBuffer ; nPos ; nPos--, pBuffer++ )
	{
		pBuffer->m_pNext = ( nPos == 1 ) ? NULL : ( pBuffer + 1 );
	}

	m_nInputBuffer	= Settings.Gnutella2.UdpInFrames; // 128;
	m_pInputBuffer	= new CDatagramIn[ m_nInputBuffer ];
	m_pInputFree	= m_pInputBuffer;

	CDatagramIn* pDGI = m_pInputBuffer;

	for ( DWORD nPos = m_nInputBuffer ; nPos ; nPos--, pDGI++ )
	{
		pDGI->m_pNextHash = ( nPos == 1 ) ? NULL : ( pDGI + 1 );
	}

	m_nOutputBuffer	= Settings.Gnutella2.UdpOutFrames; // 128;
	m_pOutputBuffer	= new CDatagramOut[ m_nOutputBuffer ];
	m_pOutputFree	= m_pOutputBuffer;

	CDatagramOut* pDGO = m_pOutputBuffer;

	for ( DWORD nPos = m_nOutputBuffer ; nPos ; nPos--, pDGO++ )
	{
		pDGO->m_pNextHash = ( nPos == 1 ) ? NULL : ( pDGO + 1 );
	}

	ZeroMemory( m_pInputHash,  sizeof(CDatagramIn*) * HASH_SIZE );
	ZeroMemory( m_pOutputHash, sizeof(CDatagramIn*) * HASH_SIZE );

	m_pInputFirst	= m_pInputLast	= NULL;
	m_pOutputFirst	= m_pOutputLast	= NULL;

	m_tLastWrite = 0;

	m_nInFrags	= m_nInPackets = 0;
	m_nOutFrags	= m_nOutPackets = 0;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams disconnect

void CDatagrams::Disconnect()
{
	if ( m_hSocket == INVALID_SOCKET ) return;

	closesocket( m_hSocket );
	m_hSocket = INVALID_SOCKET;

	delete [] m_pOutputBuffer;
	m_pOutputBuffer = NULL;
	m_nOutputBuffer = 0;
	m_pOutputFirst = m_pOutputLast = m_pOutputFree = NULL;

	delete [] m_pInputBuffer;
	m_pInputBuffer = NULL;
	m_nInputBuffer = 0;
	m_pInputFirst = m_pInputLast = m_pInputFree = NULL;

	delete [] m_pBufferBuffer;
	m_pBufferBuffer = NULL;
	m_nBufferBuffer = 0;

	m_nInBandwidth	= m_nInFrags	= m_nInPackets	= 0;
	m_nOutBandwidth	= m_nOutFrags	= m_nOutPackets	= 0;
	m_bStable = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams send

BOOL CDatagrams::Send(IN_ADDR* pAddress, WORD nPort, CPacket* pPacket, BOOL bRelease, LPVOID pToken, BOOL bAck)
{
	SOCKADDR_IN pHost;

	pHost.sin_family	= PF_INET;
	pHost.sin_addr		= *pAddress;
	pHost.sin_port		= htons( nPort );

	return Send( &pHost, pPacket, bRelease, pToken, bAck );
}

BOOL CDatagrams::Send(SOCKADDR_IN* pHost, CPacket* pPacket, BOOL bRelease, LPVOID pToken, BOOL bAck)
{
	ASSERT( pHost != NULL && pPacket != NULL );

	if ( m_hSocket == INVALID_SOCKET || Security.IsDenied( &pHost->sin_addr ) )
	{
		if ( bRelease ) pPacket->Release();
		return FALSE;
	}

	if ( pPacket->m_nProtocol == PROTOCOL_ED2K )
	{
		CBuffer pBuffer;

		((CEDPacket*)pPacket)->ToBufferUDP( &pBuffer );
		pPacket->SmartDump( pHost, TRUE, TRUE );
		if ( bRelease ) pPacket->Release();

		if ( ntohs( pHost->sin_port ) != 4669 )	// Hack
		{
			sendto( m_hSocket, (LPSTR)pBuffer.m_pBuffer, pBuffer.m_nLength, 0,
				(SOCKADDR*)pHost, sizeof(SOCKADDR_IN) );
		}

		return TRUE;
	}
	else if ( pPacket->m_nProtocol == PROTOCOL_G1 )
	{
		// Quick hack
		CBuffer pBuffer;

		((CG1Packet*)pPacket)->ToBuffer( &pBuffer );
		pPacket->SmartDump( pHost, TRUE, TRUE );
		if ( bRelease ) pPacket->Release();

		sendto( m_hSocket, (LPSTR)pBuffer.m_pBuffer, pBuffer.m_nLength, 0,
			(SOCKADDR*)pHost, sizeof(SOCKADDR_IN) );

		m_nOutPackets++;

		return TRUE;
	}
	else if ( pPacket->m_nProtocol != PROTOCOL_G2 )
	{
		if ( bRelease ) pPacket->Release();
		return FALSE;
	}

	if ( m_pOutputFree == NULL || m_pBufferFree == NULL )
	{
		if ( m_pOutputLast == NULL )
		{
			if ( bRelease ) pPacket->Release();
			theApp.Message( MSG_DEBUG, _T("CDatagrams output frames exhausted.") );
			return FALSE;
		}
		Remove( m_pOutputLast );
	}

	if ( m_pBufferFree == NULL )
	{
		if ( bRelease ) pPacket->Release();
		theApp.Message( MSG_DEBUG, _T("CDatagrams output frames really exhausted.") );
		return FALSE;
	}

	CDatagramOut* pDG = m_pOutputFree;
	m_pOutputFree = m_pOutputFree->m_pNextHash;

	if ( m_nInFrags < 1 ) bAck = FALSE;

	pDG->Create( pHost, (CG2Packet*)pPacket, m_nSequence++, m_pBufferFree, bAck );

	m_pBufferFree = m_pBufferFree->m_pNext;
	m_nBufferFree--;

	pDG->m_pToken		= pToken;
	pDG->m_pNextTime	= NULL;
	pDG->m_pPrevTime	= m_pOutputFirst;

	if ( m_pOutputFirst )
		m_pOutputFirst->m_pNextTime = pDG;
	else
		m_pOutputLast = pDG;

	m_pOutputFirst = pDG;

	BYTE nHash	= BYTE( ( pHost->sin_addr.S_un.S_un_b.s_b1
				+ pHost->sin_addr.S_un.S_un_b.s_b2
				+ pHost->sin_addr.S_un.S_un_b.s_b3
				+ pHost->sin_addr.S_un.S_un_b.s_b4
				+ pHost->sin_port
				+ pDG->m_nSequence ) & 0xff );

	CDatagramOut** pHash = m_pOutputHash + ( nHash & HASH_MASK );

	if ( *pHash ) (*pHash)->m_pPrevHash = &pDG->m_pNextHash;
	pDG->m_pNextHash = *pHash;
	pDG->m_pPrevHash = pHash;
	*pHash = pDG;

	m_nOutPackets++;

	pPacket->SmartDump( pHost, TRUE, TRUE );

#ifdef DEBUG_UDP
	theApp.Message( MSG_DEBUG, _T("UDP: Queued SGP (#%i) x%i for %s:%lu"),
		pDG->m_nSequence, pDG->m_nCount,
		(LPCTSTR)CString( inet_ntoa( pDG->m_pHost.sin_addr ) ),
		htons( pDG->m_pHost.sin_port ) );
#endif

	if ( bRelease ) pPacket->Release();

	TryWrite();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams purge outbound fragments with a specified token

void CDatagrams::PurgeToken(LPVOID pToken)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	int nCount = 0;

	for ( CDatagramOut* pDG = m_pOutputLast ; pDG ; )
	{
		CDatagramOut* pNext = pDG->m_pNextTime;

		if ( pDG->m_pToken == pToken )
		{
			Remove( pDG );
			nCount++;
		}

		pDG = pNext;
	}

	if ( nCount ) theApp.Message( MSG_DEBUG, _T("CDatagrams::PurgeToken() = %i"), nCount );
}

//////////////////////////////////////////////////////////////////////
// CDatagrams run event handler

void CDatagrams::OnRun()
{
	if ( m_hSocket == INVALID_SOCKET ) return;

	TryWrite();
	ManageOutput();

	do
	{
		ManagePartials();
	}
	while ( TryRead() );

	Measure();
}

//////////////////////////////////////////////////////////////////////
// CDatagrams measure

void CDatagrams::Measure()
{
	DWORD tCutoff		= GetTickCount() - METER_PERIOD;
	DWORD* pInHistory	= m_mInput.pHistory;
	DWORD* pInTime		= m_mInput.pTimes;
	DWORD* pOutHistory	= m_mOutput.pHistory;
	DWORD* pOutTime		= m_mOutput.pTimes;
	DWORD nInput		= 0;
	DWORD nOutput		= 0;

	for ( int tNow = METER_LENGTH ; tNow ; tNow-- )
	{
		if ( *pInTime >= tCutoff ) nInput += *pInHistory;
		if ( *pOutTime >= tCutoff ) nOutput += *pOutHistory;
		pInHistory++, pInTime++;
		pOutHistory++, pOutTime++;
	}

	m_nInBandwidth	= m_mInput.nMeasure		= nInput * 1000 / METER_PERIOD;
	m_nOutBandwidth	= m_mOutput.nMeasure	= nOutput * 1000 / METER_PERIOD;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams write datagrams

BOOL CDatagrams::TryWrite()
{
	DWORD tNow		= GetTickCount();
	DWORD nLimit	= 0xFFFFFFFF;
	DWORD nTotal	= 0;

	if ( Settings.Live.BandwidthScale <= 100 )
	{
		DWORD tCutoff	= tNow - METER_SECOND;
		DWORD* pHistory	= m_mOutput.pHistory;
		DWORD* pTime	= m_mOutput.pTimes;
		DWORD nUsed		= 0;

		for ( int nSeek = METER_LENGTH ; nSeek ; nSeek--, pHistory++, pTime++ )
		{
			if ( *pTime >= tCutoff ) nUsed += *pHistory;
		}

		nLimit = Settings.Connection.OutSpeed * 128;
		if ( Settings.Bandwidth.UdpOut != 0 ) nLimit = Settings.Bandwidth.UdpOut;

		if ( Settings.Live.BandwidthScale < 100 )
		{
			nLimit = nLimit * Settings.Live.BandwidthScale / 100;
		}

		nLimit = ( nUsed >= nLimit ) ? 0 : ( nLimit - nUsed );
	}

	DWORD nLastHost = 0;

	while ( nLimit > 0 )
	{
        CDatagramOut* pDG = m_pOutputFirst;
		for ( ; pDG ; pDG = pDG->m_pPrevTime )
		{
			BYTE* pPacket;
			DWORD nPacket;

			if ( nLastHost == pDG->m_pHost.sin_addr.S_un.S_addr )
			{
				// Same host, skip it
			}
			else if ( pDG->GetPacket( tNow, &pPacket, &nPacket, m_nInFrags > 0 ) )
			{
				sendto( m_hSocket, (LPCSTR)pPacket, nPacket, 0,
					(SOCKADDR*)&pDG->m_pHost, sizeof(SOCKADDR_IN) );

				nLastHost = pDG->m_pHost.sin_addr.S_un.S_addr;

				if ( nLimit >= nPacket )
					nLimit -= nPacket;
				else
					nLimit = 0;

				m_tLastWrite = GetTickCount();
				nTotal += nPacket;
				m_nOutFrags++;

#ifdef DEBUG_UDP
				SGP_HEADER* pTemp = (SGP_HEADER*)pPacket;
				theApp.Message( MSG_DEBUG, _T("UDP: Sending (#%i) %i of %i to %s:%lu"),
					pDG->m_nSequence, pTemp->nPart, pTemp->nCount,
					(LPCTSTR)CString( inet_ntoa( pDG->m_pHost.sin_addr ) ),
					htons( pDG->m_pHost.sin_port ) );
#endif
				if( ! pDG->m_bAck ) Remove( pDG );

				break;
			}
		}

		if ( pDG == NULL ) break;
	}

	if ( m_mOutput.pHistory && nTotal )
	{
		if ( tNow - m_mOutput.tLastSlot < METER_MINIMUM )
		{
			m_mOutput.pHistory[ m_mOutput.nPosition ]	+= nTotal;
		}
		else
		{
			m_mOutput.nPosition = ( m_mOutput.nPosition + 1 ) % METER_LENGTH;
			m_mOutput.pTimes[ m_mOutput.nPosition ]		= tNow;
			m_mOutput.pHistory[ m_mOutput.nPosition ]	= nTotal;
			m_mOutput.tLastSlot = tNow;
		}
	}

	m_mOutput.nTotal += nTotal;
	Statistics.Current.Bandwidth.Outgoing += nTotal;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams manage output queue datagrams

void CDatagrams::ManageOutput()
{
	DWORD tNow = GetTickCount();

	for ( CDatagramOut* pDG = m_pOutputLast ; pDG ; )
	{
		CDatagramOut* pNext = pDG->m_pNextTime;

		if ( tNow - pDG->m_tSent > Settings.Gnutella2.UdpOutExpire )
		{
			Remove( pDG );
		}

		pDG = pNext;
	}
}

//////////////////////////////////////////////////////////////////////
// CDatagrams remove output datagrams

void CDatagrams::Remove(CDatagramOut* pDG)
{
	if ( pDG->m_pBuffer )
	{
		pDG->m_pBuffer->m_pNext = m_pBufferFree;
		m_pBufferFree = pDG->m_pBuffer;
		m_pBufferFree->Clear();
		pDG->m_pBuffer = NULL;
		m_nBufferFree++;
	}

	if ( pDG->m_pNextHash ) pDG->m_pNextHash->m_pPrevHash = pDG->m_pPrevHash;
	*(pDG->m_pPrevHash) = pDG->m_pNextHash;

	if ( pDG->m_pNextTime )
		pDG->m_pNextTime->m_pPrevTime = pDG->m_pPrevTime;
	else
		m_pOutputFirst = pDG->m_pPrevTime;

	if ( pDG->m_pPrevTime )
		pDG->m_pPrevTime->m_pNextTime = pDG->m_pNextTime;
	else
		m_pOutputLast = pDG->m_pNextTime;

	pDG->m_pNextHash = m_pOutputFree;
	m_pOutputFree = pDG;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams read datagram

BOOL CDatagrams::TryRead()
{
	if ( m_hSocket == INVALID_SOCKET )
		return FALSE;

	std::vector< BYTE > pBuffer( 65536 );	// Maximal UDP size 64KB
	SOCKADDR_IN pFrom = {};
	int nFromLen = sizeof( pFrom );
	int nLength	= recvfrom( m_hSocket, (char*)&pBuffer[ 0 ], (int)pBuffer.size(), 0,
		(SOCKADDR*)&pFrom, &nFromLen );

	if ( nLength < 1 )
		return FALSE;

	pBuffer.resize( nLength );

	if ( m_mInput.pHistory )
	{
		DWORD tNow = GetTickCount();
		if ( tNow - m_mInput.tLastSlot < METER_MINIMUM )
		{
			m_mInput.pHistory[ m_mInput.nPosition ] += nLength;
		}
		else
		{
			m_mInput.nPosition = ( m_mInput.nPosition + 1 ) % METER_LENGTH;
			m_mInput.pTimes[ m_mInput.nPosition ]	= tNow;
			m_mInput.pHistory[ m_mInput.nPosition ]	= nLength;
			m_mInput.tLastSlot = tNow;
		}
	}

	m_mInput.nTotal += nLength;
	Statistics.Current.Bandwidth.Incoming += nLength;

	if ( Security.IsAccepted( &pFrom.sin_addr ) &&
		 ! Network.IsFirewalledAddress( &pFrom.sin_addr, TRUE ) )
	{
		OnDatagram( &pFrom, &pBuffer [ 0 ], nLength );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams datagram handler

BOOL CDatagrams::OnDatagram(SOCKADDR_IN* pHost, BYTE* pBuffer, DWORD nLength)
{
	BOOL bHandled = FALSE;
	GNUTELLAPACKET* pG1UDP = (GNUTELLAPACKET*)pBuffer;
	ED2K_UDP_HEADER* pMULE = (ED2K_UDP_HEADER*)pBuffer;
	SGP_HEADER* pSGP = (SGP_HEADER*)pBuffer;

	// Detect Gnutella 1 packets

	if ( nLength >= sizeof(GNUTELLAPACKET) &&
		( sizeof(GNUTELLAPACKET) + pG1UDP->m_nLength ) == nLength )
	{
		CG1Packet* pG1Packet = CG1Packet::New( pG1UDP );
		if ( pG1Packet )
		{
			bHandled = OnPacket( pHost, pG1Packet );

			pG1Packet->Release();

			if ( bHandled )
				return TRUE;
		}
	}

	// Detect ED2K and KAD packets

	if ( nLength > sizeof(ED2K_UDP_HEADER) )
	{
		switch ( pMULE->nProtocol )
		{
		case ED2K_PROTOCOL_EDONKEY:
		case ED2K_PROTOCOL_EMULE:
		case ED2K_PROTOCOL_EMULE_PACKED:
		case ED2K_PROTOCOL_KAD:
		case ED2K_PROTOCOL_KAD_PACKED:
		case ED2K_PROTOCOL_REVCONNECT:
		case ED2K_PROTOCOL_REVCONNECT_PACKED:
			{
				CEDPacket* pPacket = CEDPacket::New( pMULE, nLength );
				if ( pPacket && !pPacket->InflateOrRelease() )
				{
					bHandled = EDClients.OnUDP( pHost, pPacket );

					pPacket->Release();

					if ( bHandled )
						return TRUE;
				}
			}
		}
	}

	// Detect Gnutella 2 packets

	if ( nLength >= sizeof(SGP_HEADER) &&
		( *(DWORD*)pSGP->szTag & 0x00ffffff ) == ( *(DWORD*)SGP_TAG_2 & 0x00ffffff ) &&
		pSGP->nPart && ( ! pSGP->nCount || pSGP->nPart <= pSGP->nCount ) )
	{
		if ( pSGP->nCount )
		{
			bHandled = OnReceiveSGP( pHost, pSGP, nLength - sizeof(SGP_HEADER) );
		}
		else
		{
			bHandled = OnAcknowledgeSGP( pHost, pSGP, nLength - sizeof(SGP_HEADER) );
		}

		if ( bHandled )
			return TRUE;
	}

	// Detect BitTorrent packets

	if ( nLength > 16 )
	{
		CBuffer pInput;
		pInput.Add( pBuffer, nLength );
		CBENode* pRoot = CBENode::Decode( &pInput );
		if ( pRoot )
		{
			bHandled = OnReceiveBT( pHost, pRoot );

			delete pRoot;

			if ( bHandled )
				return TRUE;
		}
	}

	// Report unknown packets

	CString strText, strTmp;
	strText.Format( _T("UDP: Recieved unknown packet (%i bytes) from %s"),
		nLength, (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );
	for ( DWORD i = 0; i < nLength && i < 80; i++ )
	{
		if ( ! i )
			strText += _T(": ");
		strText += ( ( pBuffer[ i ] < ' ' ) ? '.' : (char)pBuffer[ i ] );
	}
	theApp.Message( MSG_DEBUG, _T("%s"), strText );

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams BitTorrent receive handler

BOOL CDatagrams::OnReceiveBT(const SOCKADDR_IN* pHost, const CBENode* pRoot)
{
	theApp.Message( MSG_DEBUG, _T("Recieved UDP BitTorrent packet from %s: %s"),
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), (LPCTSTR)pRoot->Encode() );

	m_nInPackets++;

	// TODO: pPacket->SmartDump( pHost, TRUE, FALSE );

	BOOL bHandled = FALSE;
	CBENode pReply;
	
	if ( pRoot->IsType( CBENode::beDict ) )
	{
		// Get version
		CBENode* pVersion = pRoot->GetNode( "v" );
		if ( pVersion && pVersion->IsType( CBENode::beString ) )
		{
		}

		// Get packet type and transaction id
		CBENode* pType = pRoot->GetNode( "y" );
		CBENode* pTransID = pRoot->GetNode( "t" );
		if ( pType && pType->IsType( CBENode::beString ) &&
			pTransID && pTransID->IsType( CBENode::beString ) )
		{
			if ( pType->GetString() == "q" )
			{
				// Query message
				CBENode* pQueryMethod = pRoot->GetNode( "q" );
				CBENode* pQueryData = pRoot->GetNode( "a" );
				if ( pQueryMethod && pQueryMethod->IsType( CBENode::beString ) &&
					pQueryData && pQueryData->IsType( CBENode::beDict ) )
				{
					if ( pQueryMethod->GetString() == "ping" )
					{
						// Ping
						CBENode* pNodeID = pQueryData->GetNode( "id" );
						if ( pNodeID && pNodeID->IsType( CBENode::beString ) &&
							pNodeID->m_nValue == 20 )
						{
							Hashes::BtGuid oNodeGUID;
							CopyMemory( &oNodeGUID[0], pNodeID->m_pValue, 20 );
							oNodeGUID.validate();

							// Send "Pong" reply
							CBENode* pReplyData = pReply.Add( "r" );
							Hashes::BtGuid oMyGUID( MyProfile.oGUIDBT );
							pReplyData->Add( "id" )->SetString( &oMyGUID[0], oMyGUID.byteCount );
							pReply.Add( "y" )->SetString( "r" );
							pReply.Add( "t" )->SetString( pTransID->m_pValue, (size_t)pTransID->m_nValue );

							bHandled = TRUE;
						}
					}
					else if ( pQueryMethod->GetString() == "find_node" )
					{
						// Find node
					}
					else if ( pQueryMethod->GetString() == "get_peers" )
					{
						// Get peers
					}
					else if ( pQueryMethod->GetString() == "announce_peer" )
					{
						// Announce peer
					}
					// else if ( pQueryMethod->GetString() == "error" ) - ???
					// else Reply: "204 Method Unknown"
				}
			}
			else if ( pType->GetString() == "r" )
			{
				// Response message
				CBENode* pResponse = pRoot->GetNode( "r" );
				if ( pResponse && pResponse->IsType( CBENode::beDict ) )
				{
					CBENode* pNodeID = pResponse->GetNode( "id" );
					if ( pNodeID && pNodeID->IsType( CBENode::beString ) &&
						pNodeID->m_nValue == 20 )
					{
						Hashes::BtGuid oNodeGUID;
						CopyMemory( &oNodeGUID[0], pNodeID->m_pValue, 20 );
						oNodeGUID.validate();

						// Check queries pool for pTransID

						// Save access token
						CBENode* pToken = pResponse->GetNode( "token" );
						if ( pToken && pToken->IsType( CBENode::beString ) )
						{
						}

						CBENode* pPeers = pResponse->GetNode( "values" );
						if ( pPeers && pPeers->IsType( CBENode::beList) )
						{
						}

						CBENode* pNodes = pResponse->GetNode( "nodes" );
						if ( pNodes && pNodes->IsType( CBENode::beString ) )
						{
						}

						bHandled = TRUE;
					}
				}
			}
			else if ( pType->GetString() == "e" )
			{
				// Error message
				CBENode* pError = pRoot->GetNode( "e" );
				if ( pError && pError->IsType( CBENode::beList ) )
				{
				}
			}
		}
	}

	if ( bHandled )
	{
		// Send reply if any
		if ( ! pReply.IsType( CBENode::beNull ) )
		{
			pReply.Add( "v" )->SetString( theApp.m_pBTVersion, 4 );

			CBuffer pOutput;
			pReply.Encode( &pOutput );
			sendto( m_hSocket, (LPSTR)pOutput.m_pBuffer, pOutput.m_nLength, 0,
				(SOCKADDR*)pHost, sizeof(SOCKADDR_IN) );

			theApp.Message( MSG_DEBUG, _T("UDP: Sended BitTorrent packet to %s: %s"),
				(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), (LPCTSTR)pReply.Encode() );
		}
		return TRUE;
	}
	// else reply "203 Protocol Error"

	return FALSE;
}

/*void CDatagrams::DHTPing(const SOCKADDR_IN* pHost)
{
	CBENode pPing;
	CBENode* pPingData = pPing.Add( "a" );
	Hashes::BtGuid oMyGUID( MyProfile.oGUIDBT );
	pPingData->Add( "id" )->SetString( &oMyGUID[0], oMyGUID.byteCount );
	pPing.Add( "y" )->SetString( "q" );
	pPing.Add( "t" )->SetString( "1234" ); // TODO
	pPing.Add( "q" )->SetString( "ping" );
	pPing.Add( "v" )->SetString( theApp.m_pBTVersion, 4 );
	CBuffer pPingOutput;
	pPing.Encode( &pPingOutput );

	sendto( m_hSocket, (LPSTR)pPingOutput.m_pBuffer, pPingOutput.m_nLength, 0,
		(SOCKADDR*)pHost, sizeof(SOCKADDR_IN) );

	theApp.Message( MSG_DEBUG, _T("UDP: Sended BitTorrent ping packet to %s: %s"),
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), (LPCTSTR)pPing.Encode() );
}*/

/*void CDatagrams::DHTGetPeers(const SOCKADDR_IN* pHost, const Hashes::BtGuid& oNodeGUID, const Hashes::BtHash& oGUID)
{
	CBENode pGetPeers;
	CBENode* pGetPeersData = pGetPeers.Add( "a" );
	pGetPeersData->Add( "id" )->SetString( &oNodeGUID[0], oNodeGUID.byteCount );
	pGetPeersData->Add( "info_hash" )->SetString( &oGUID[0], oGUID.byteCount );
	pGetPeers.Add( "y" )->SetString( "q" );
	pGetPeers.Add( "t" )->SetString( "4567" ); // TODO
	pGetPeers.Add( "q" )->SetString( "get_peers" );
	pGetPeers.Add( "v" )->SetString( theApp.m_pBTVersion, 4 );
	CBuffer pGetPeersOutput;
	pGetPeers.Encode( &pGetPeersOutput );

	sendto( m_hSocket, (LPSTR)pGetPeersOutput.m_pBuffer, pGetPeersOutput.m_nLength, 0,
		(SOCKADDR*)pHost, sizeof(SOCKADDR_IN) );

	theApp.Message( MSG_DEBUG, _T("UDP: Sended BitTorrent get peers packet to %s: %s"),
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), (LPCTSTR)pGetPeers.Encode() );
}*/

//////////////////////////////////////////////////////////////////////
// CDatagrams SGP receive handler

BOOL CDatagrams::OnReceiveSGP(SOCKADDR_IN* pHost, SGP_HEADER* pHeader, DWORD nLength)
{
#ifdef DEBUG_UDP
	theApp.Message( MSG_DEBUG, _T("UDP: Received SGP (#%i) %i of %i from %s"),
		pHeader->nSequence, pHeader->nPart, pHeader->nCount,
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );
#endif

	m_nInFrags++;

	if ( pHeader->nFlags & SGP_ACKNOWLEDGE )
	{
		SGP_HEADER pAck;

		strncpy( pAck.szTag, SGP_TAG_2, 3 );
		pAck.nFlags		= 0;
		pAck.nSequence	= pHeader->nSequence;
		pAck.nPart		= pHeader->nPart;
		pAck.nCount		= 0;

		sendto( m_hSocket, (LPCSTR)&pAck, sizeof(pAck), 0,
			(SOCKADDR*)pHost, sizeof(SOCKADDR_IN) );
	}

	BYTE nHash	= BYTE( ( pHost->sin_addr.S_un.S_un_b.s_b1
				+ pHost->sin_addr.S_un.S_un_b.s_b2
				+ pHost->sin_addr.S_un.S_un_b.s_b3
				+ pHost->sin_addr.S_un.S_un_b.s_b4
				+ pHost->sin_port
				+ pHeader->nSequence ) & 0xff );

	CDatagramIn** pHash = m_pInputHash + ( nHash & HASH_MASK );

    CDatagramIn* pDG = *pHash;
	for ( ; pDG ; pDG = pDG->m_pNextHash )
	{
		if (	pDG->m_pHost.sin_addr.S_un.S_addr == pHost->sin_addr.S_un.S_addr &&
				pDG->m_pHost.sin_port == pHost->sin_port &&
				pDG->m_nSequence == pHeader->nSequence &&
				pDG->m_nCount == pHeader->nCount )
		{
			if ( pDG->Add( pHeader->nPart, &pHeader[1], nLength ) )
			{
				if ( CG2Packet* pPacket = pDG->ToG2Packet() )
				{
					try
					{
						OnPacket( pHost, pPacket );
					}
					catch ( CException* pException )
					{
						pException->Delete();
					}

					pPacket->Release();
				}

				// Keep it to check sequence numbers
				// Remove( pDG );
			}

			return TRUE;
		}
	}

	while ( m_pInputFree == NULL || m_nBufferFree < pHeader->nCount )
	{
		if ( m_pInputLast == NULL ) return FALSE;
		Remove( m_pInputLast );
	}

	if ( m_nBufferFree < pHeader->nCount ) return FALSE;

	pDG = m_pInputFree;

	pDG->Create( pHost, pHeader->nFlags, pHeader->nSequence, pHeader->nCount );

	for ( WORD nPart = 0 ; nPart < pDG->m_nCount ; nPart++ )
	{
		ASSERT( pDG->m_pBuffer[ nPart ] == NULL );
		pDG->m_pBuffer[ nPart ] = m_pBufferFree;
		m_pBufferFree = m_pBufferFree->m_pNext;
		m_nBufferFree--;
	}

	if ( pDG->Add( pHeader->nPart, &pHeader[1], nLength ) )
	{
		if ( CG2Packet* pPacket = pDG->ToG2Packet() )
		{
			try
			{
				OnPacket( pHost, pPacket );
			}
			catch ( CException* pException )
			{
				pException->Delete();
			}
			pPacket->Release();
		}

		// Don't remove it, keep it to check sequence numbers
		// Remove( pDG, TRUE );
	}

	// Always add it to the list

	pDG->m_pNextTime = NULL;
	pDG->m_pPrevTime = m_pInputFirst;

	if ( m_pInputFirst )
		m_pInputFirst->m_pNextTime = pDG;
	else
		m_pInputLast = pDG;

	m_pInputFirst = pDG;
	m_pInputFree = pDG->m_pNextHash;

	if ( *pHash ) (*pHash)->m_pPrevHash = &pDG->m_pNextHash;
	pDG->m_pNextHash = *pHash;
	pDG->m_pPrevHash = pHash;
	*pHash = pDG;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams SGP acknowledgement handler

BOOL CDatagrams::OnAcknowledgeSGP(SOCKADDR_IN* pHost, SGP_HEADER* pHeader, DWORD /*nLength*/)
{
#ifdef DEBUG_UDP
	theApp.Message( MSG_DEBUG, _T("UDP: Received SGP ack (#%i) %i from %s"),
		pHeader->nSequence, pHeader->nPart, (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );
#endif

	BYTE nHash	= BYTE( ( pHost->sin_addr.S_un.S_un_b.s_b1
				+ pHost->sin_addr.S_un.S_un_b.s_b2
				+ pHost->sin_addr.S_un.S_un_b.s_b3
				+ pHost->sin_addr.S_un.S_un_b.s_b4
				+ pHost->sin_port
				+ pHeader->nSequence ) & 0xff );

	CDatagramOut** pHash = m_pOutputHash + ( nHash & HASH_MASK );

	for ( CDatagramOut* pDG = *pHash ; pDG ; pDG = pDG->m_pNextHash )
	{
		if (	pDG->m_pHost.sin_addr.S_un.S_addr == pHost->sin_addr.S_un.S_addr &&
				pDG->m_pHost.sin_port == pHost->sin_port &&
				pDG->m_nSequence == pHeader->nSequence )
		{
			if ( pDG->Acknowledge( pHeader->nPart ) ) Remove( pDG );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams manage partial datagrams

void CDatagrams::ManagePartials()
{
	DWORD tNow = GetTickCount();

	for ( CDatagramIn* pDG = m_pInputLast ; pDG ; )
	{
		CDatagramIn* pNext = pDG->m_pNextTime;

		if ( tNow - pDG->m_tStarted > Settings.Gnutella2.UdpInExpire )
		{
			Remove( pDG );
		}

		pDG = pNext;
	}
}

//////////////////////////////////////////////////////////////////////
// CDatagrams remove a partiallly received datagram

void CDatagrams::Remove(CDatagramIn* pDG, BOOL bReclaimOnly)
{
	for ( int nPart = 0 ; nPart < pDG->m_nCount ; nPart++ )
	{
		if ( pDG->m_pBuffer[ nPart ] )
		{
			pDG->m_pBuffer[ nPart ]->m_pNext = m_pBufferFree;
			m_pBufferFree = pDG->m_pBuffer[ nPart ];
			m_pBufferFree->Clear();
			pDG->m_pBuffer[ nPart ] = NULL;
			m_nBufferFree++;
		}
	}

	if ( bReclaimOnly ) return;

	if ( pDG->m_pNextHash ) pDG->m_pNextHash->m_pPrevHash = pDG->m_pPrevHash;
	*(pDG->m_pPrevHash) = pDG->m_pNextHash;

	if ( pDG->m_pNextTime )
		pDG->m_pNextTime->m_pPrevTime = pDG->m_pPrevTime;
	else
		m_pInputFirst = pDG->m_pPrevTime;

	if ( pDG->m_pPrevTime )
		pDG->m_pPrevTime->m_pNextTime = pDG->m_pNextTime;
	else
		m_pInputLast = pDG->m_pNextTime;

	pDG->m_pNextHash = m_pInputFree;
	m_pInputFree = pDG;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams G1UDP packet handler

BOOL CDatagrams::OnPacket(SOCKADDR_IN* pHost, CG1Packet* pPacket)
{
	pPacket->SmartDump( pHost, TRUE, FALSE );

	m_nInPackets++;

	switch ( pPacket->m_nType )
	{
		case G1_PACKET_PING:
			return OnPing( pHost, pPacket );
		case G1_PACKET_PONG:
			return OnPong( pHost, pPacket );
		default:
			pPacket->Debug( CString( _T("Received unexpected UDP packet from ") ) + 
				CString( inet_ntoa( pHost->sin_addr ) ) );
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams G2UDP packet handler

BOOL CDatagrams::OnPacket(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	pPacket->SmartDump( pHost, TRUE, FALSE );

	m_nInPackets++;

	// Is it neigbour's packet or stranger's packet?
	CNeighbour* pNeighbour = Neighbours.Get( &( pHost->sin_addr ) );
//	CG1Neighbour* pNeighbour1 = static_cast< CG1Neighbour* >
//		( ( pNeighbour && pNeighbour->m_nProtocol == PROTOCOL_G1 ) ? pNeighbour : NULL );
	CG2Neighbour* pNeighbour2 = static_cast< CG2Neighbour* >
		( ( pNeighbour && pNeighbour->m_nProtocol == PROTOCOL_G2 ) ? pNeighbour : NULL );

	if ( Network.RoutePacket( pPacket ) ) return TRUE;

	switch( pPacket->m_nType )
	{
	case G2_PACKET_QUERY:
		return OnQuery( pHost, pPacket );
	case G2_PACKET_QUERY_KEY_REQ:
		return OnQueryKeyRequest( pHost, pPacket );
	case G2_PACKET_HIT:
		return OnCommonHit( pHost, pPacket );
	case G2_PACKET_HIT_WRAP:
		return OnCommonHit( pHost, pPacket );
	case G2_PACKET_QUERY_WRAP:
		// G2_PACKET_QUERY_WRAP deprecated and ignored
		break;
	case G2_PACKET_QUERY_ACK:
		return OnQueryAck( pHost, pPacket );
	case G2_PACKET_QUERY_KEY_ANS:
		return OnQueryKeyAnswer( pHost, pPacket );
	case G2_PACKET_PING:
		// Pass packet handling to neighbour if any
		return pNeighbour2 ? pNeighbour2->OnPing( pPacket, FALSE ) : OnPing( pHost, pPacket );
	case G2_PACKET_PONG:
		// Pass packet handling to neighbour if any
		return pNeighbour2 ? pNeighbour2->OnPong( pPacket, FALSE ) : OnPong( pHost, pPacket );
	case G2_PACKET_PUSH:
		return OnPush( pHost, pPacket );
	case G2_PACKET_CRAWL_REQ:
		return OnCrawlRequest( pHost, pPacket );
	case G2_PACKET_CRAWL_ANS:
		return OnCrawlAnswer( pHost, pPacket );
	case G2_PACKET_KHL_ANS:
		return OnKHLA( pHost, pPacket );
	case G2_PACKET_KHL_REQ:
		return OnKHLR( pHost, pPacket );
	case G2_PACKET_DISCOVERY:
		return OnDiscovery( pHost, pPacket );
	case G2_PACKET_KHL:
		return OnKHL( pHost, pPacket );
	default:
		pPacket->Debug( CString( _T("Received unexpected UDP packet from ") ) + 
			CString( inet_ntoa( pHost->sin_addr ) ) );
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams PING packet handler for G1UDP

BOOL CDatagrams::OnPing(SOCKADDR_IN* pHost, CG1Packet* pPacket)
{
	BOOL bSCP = FALSE;
	// If this ping packet strangely has length, and the remote computer does GGEP blocks
	if ( pPacket->m_nLength )
	{
		CGGEPBlock pGGEP;
		// There is a GGEP block here, and checking and adjusting the TTL and hops counts worked
		if ( pGGEP.ReadFromPacket( pPacket ) )
		{
			if ( CGGEPItem* pItem = pGGEP.Find( GGEP_HEADER_SUPPORT_CACHE_PONGS ) )
			{
				bSCP = TRUE;
			}
		}
	}

	CGGEPBlock pGGEP;
	// Received SCP GGEP, send 5 random hosts from the cache
	// Since we do not provide leaves, ignore the preference data
	if ( bSCP )
	{
		CGGEPItem* pItem = pGGEP.Add( GGEP_HEADER_PACKED_IPPORTS );
		DWORD nCount = min( DWORD(50), HostCache.Gnutella1.CountHosts() );
		WORD nPos = 0;
		pItem->UnsetCOBS();
		pItem->UnsetSmall();

		// Create 5 random positions from 0 to 50 in the descending order
		std::vector< WORD > pList;
		pList.reserve( Settings.Gnutella1.MaxHostsInPongs );
		for ( WORD nNo = 0 ; nNo < Settings.Gnutella1.MaxHostsInPongs ; nNo++ )
		{
			pList.push_back( (WORD)( ( nCount + 1 ) * rand() / ( RAND_MAX + (float)nCount ) ) );
		}
		std::sort( pList.begin(), pList.end(), CompareNums() );

		nCount = Settings.Gnutella1.MaxHostsInPongs;

		CQuickLock oLock( HostCache.Gnutella1.m_pSection );

		for ( CHostCacheIterator i = HostCache.Gnutella1.Begin() ;
			i != HostCache.Gnutella1.End() && nCount && !pList.empty(); ++i )
		{
			CHostCacheHost* pHost = (*i);

			nPos = pList.back(); // take the smallest value;
			pList.pop_back(); // remove it
			for ( ; i != HostCache.Gnutella1.End() && nPos-- ; ++i ) pHost = (*i);

			if ( i != HostCache.Gnutella1.End() &&
				pHost->m_bCheckedLocally && pHost->m_nFailures == 0 )
			{
				pItem->Write( (void*)&pHost->m_pAddress, 4 );
				pItem->Write( (void*)&pHost->m_nPort, 2 );
				theApp.Message( MSG_DEBUG, _T("Sending G1 host through pong (%s:%i)"), 
					(LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&pHost->m_pAddress ) ), pHost->m_nPort ); 
				nCount--;
			}
			if ( nCount == 0 ) break;
		}

		if ( nCount == (DWORD)Settings.Gnutella1.MaxHostsInPongs ) bSCP = FALSE; // the cache is empty
	}

	// Make a new pong packet, the response to a ping
	CG1Packet* pPong = CG1Packet::New(			// Gets it quickly from the Gnutella packet pool
		G1_PACKET_PONG,							// We're making a pong packet
		pPacket->m_nHops,						// Give it TTL same as HOP count of received PING packet
		pPacket->m_oGUID);						// Give it the same GUID as the ping

	// Get statistics about how many files we are sharing
	QWORD nMyVolume;
	DWORD nMyFiles;
	LibraryMaps.GetStatistics( &nMyFiles, &nMyVolume );

	// Start the pong's payload with the IP address and port number from the Network object (do)
	pPong->WriteShortLE( htons( Network.m_pHost.sin_port ) );
	pPong->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );

	// Then, write in the information about how many files we are sharing
	pPong->WriteLongLE( nMyFiles );
	pPong->WriteLongLE( (DWORD)nMyVolume );

	if ( !pGGEP.IsEmpty() ) pGGEP.Write( pPong );

	// Send the pong packet to the remote computer we are currently looping on
	Send( pHost, pPong );
	theApp.Message( MSG_DEBUG, _T("G1UDP: Sent Pong to %s"), (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams PING packet handler

BOOL CDatagrams::OnPing(SOCKADDR_IN* pHost, CG2Packet* /*pPacket*/)
{
	Send( pHost, CG2Packet::New( G2_PACKET_PONG ) );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams PONG packet handler

BOOL CDatagrams::OnPong(SOCKADDR_IN* pHost, CG1Packet* pPacket)
{
	Statistics.Current.Gnutella1.PongsReceived++;
	// If the pong is too short, or the pong is too long and settings say we should watch that
	if ( pPacket->m_nLength < 14 || ( pPacket->m_nLength > 14 && Settings.Gnutella1.StrictPackets && !Settings.Gnutella1.EnableGGEP ) )
	{
		// Pong packets should be 14 bytes long, drop this strangely sized one
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_SIZE_PONG, (LPCTSTR)inet_ntoa( pHost->sin_addr ) );
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE; // Don't disconnect from the remote computer, though
	}

	// Read information from the pong packet
	WORD nPort     = pPacket->ReadShortLE(); // 2 bytes, port number (do) of us? the remote computer? the computer that sent the packet?
	DWORD nAddress = pPacket->ReadLongLE();  // 4 bytes, IP address
	DWORD nFiles   = pPacket->ReadLongLE();  // 4 bytes, the number of files the source computer is sharing
	DWORD nVolume  = pPacket->ReadLongLE();  // 4 bytes, the total size of all those files
	UNUSED_ALWAYS(nFiles);
	UNUSED_ALWAYS(nVolume);

	CDiscoveryService * pService = DiscoveryServices.GetByAddress( &(pHost->sin_addr) , ntohs(pHost->sin_port), 3 );

	// If that IP address is in our list of computers to not talk to, except ones in UHC list in discovery
	if ( pService == NULL && Security.IsDenied( (IN_ADDR*)&nAddress ) )
	{
		// Record the packet as dropped, do nothing else, and leave now
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE;
	}

	// If the pong is bigger than 14 bytes, and the remote compuer told us in the handshake it supports GGEP blocks
	if ( pPacket->m_nLength > 14 && Settings.Gnutella1.EnableGGEP )
	{
		CGGEPBlock pGGEP;
		// There is a GGEP block here, and checking and adjusting the TTL and hops counts worked
		if ( pGGEP.ReadFromPacket( pPacket ) )
		{

			CGGEPItem* pUP = pGGEP.Find( GGEP_HEADER_UP_SUPPORT );
			CGGEPItem* pVC = pGGEP.Find( GGEP_HEADER_VENDOR_INFO, 4 );
			CString sVendorCode;
			if ( pVC != NULL )
			{
				CHAR szaVendor[ 4 ];
				pVC->Read( szaVendor,4 );
				TCHAR szVendor[5] = { szaVendor[0], szaVendor[1], szaVendor[2], szaVendor[3], 0 };

				sVendorCode.Format( _T("%s"), (LPCTSTR)szVendor);
			}

			if ( pUP != NULL )
			{
				sVendorCode.Trim( _T(" ") );
				HostCache.Gnutella1.Add( (IN_ADDR*)&nAddress, nPort, 0, (LPCTSTR)sVendorCode );
			}


			int nCount = 0;
			CGGEPItem* pIPPs = pGGEP.Find( GGEP_HEADER_PACKED_IPPORTS, 6 );

			// We got a response to SCP extension, add hosts to cache if IPP extension exists
			if ( pIPPs != NULL )
			{
				// The first four bytes represent the IP address and the last two represent the port
				// The length of the number of bytes of IPP must be divisible by 6
				if ( ( pIPPs->m_nLength - pIPPs->m_nPosition ) % 6 == 0 )
				{
					while ( pIPPs->m_nPosition != pIPPs->m_nLength )
					{
						DWORD nIPPAddress = 0;
						WORD nIPPPort = 0;
						pIPPs->Read( (void*)&nIPPAddress, 4 );
						pIPPs->Read( (void*)&nIPPPort, 2 );
						if ( nPort != 0 )
						{
							CHostCacheHost * pCachedHost;
							theApp.Message( MSG_DEBUG, _T("Got Gnutella hosts through UDP pong (%s:%i)"), 
								(LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&nIPPAddress ) ), nIPPPort ); 
							pCachedHost = HostCache.Gnutella1.Add( (IN_ADDR*)&nIPPAddress, nIPPPort, 0, NULL );
							// Add to separate cache to have a quick access only to GDNAs

							if ( pCachedHost != NULL ) nCount++;
						}
					}
				}

				if ( pService != NULL )
				{
					pService->OnSuccess();
					pService->m_nHosts = nCount;
				}
			}

		}
		else
		{
			// It's not, drop the packet, but stay connected
			theApp.Message( MSG_ERROR, IDS_PROTOCOL_GGEP_REQUIRED, (LPCTSTR)inet_ntoa( pHost->sin_addr ) );
			Statistics.Current.Gnutella1.Dropped++;
			return TRUE;
		}
	}

	return TRUE;
}

BOOL CDatagrams::OnPong(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;

	BOOL bRelayed = FALSE;
	G2_PACKET nType;
	DWORD nLength;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;
		if ( nType == G2_PACKET_RELAY ) bRelayed = TRUE;
		pPacket->m_nPosition = nOffset;
	}

	if ( bRelayed && ! Network.IsConnectedTo( &pHost->sin_addr ) )
		SetStable();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams QUERY packet handler

BOOL CDatagrams::OnQuery(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	CQuerySearch* pSearch = CQuerySearch::FromPacket( pPacket, pHost );

	if ( pSearch == NULL || ! pSearch->m_bUDP )
	{
		if ( pSearch ) delete pSearch;
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_QUERY,
			(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	if ( Security.IsDenied( &pSearch->m_pEndpoint.sin_addr ) || !Settings.Gnutella2.EnableToday )
	{
		delete pSearch;
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	if ( ! Network.QueryKeys->Check( pSearch->m_pEndpoint.sin_addr.S_un.S_addr, pSearch->m_nKey ) )
	{
		DWORD nKey = Network.QueryKeys->Create( pSearch->m_pEndpoint.sin_addr.S_un.S_addr );

		CString strNode = inet_ntoa( pSearch->m_pEndpoint.sin_addr );
		theApp.Message( MSG_DEBUG, _T("Issuing correction for node %s's query key for %s"),
			(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), (LPCTSTR)strNode );

		CG2Packet* pAnswer = CG2Packet::New( G2_PACKET_QUERY_KEY_ANS, TRUE );
		pAnswer->WritePacket( G2_PACKET_QUERY_KEY, 4 );
		pAnswer->WriteLongBE( nKey );

		if ( pHost->sin_addr.S_un.S_addr != pSearch->m_pEndpoint.sin_addr.S_un.S_addr )
		{
			pAnswer->WritePacket( G2_PACKET_SEND_ADDRESS, 4 );
			pAnswer->WriteLongLE( pHost->sin_addr.S_un.S_addr );
		}

		Send( &pSearch->m_pEndpoint, pAnswer, TRUE );

		delete pSearch;
		return TRUE;
	}
	
	if ( ! Network.QueryRoute->Add( pSearch->m_oGUID, &pSearch->m_pEndpoint ) )
	{
		CG2Packet* pAnswer = CG2Packet::New( G2_PACKET_QUERY_ACK, TRUE );
		pAnswer->WritePacket( G2_PACKET_QUERY_DONE, 8 );
		pAnswer->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
		pAnswer->WriteShortBE( htons( Network.m_pHost.sin_port ) );
		pAnswer->WriteShortBE( 0 );
		pAnswer->WriteByte( 0 );
		pAnswer->Write( pSearch->m_oGUID );
		Send( &pSearch->m_pEndpoint, pAnswer, TRUE );

		delete pSearch;
		Statistics.Current.Gnutella2.Dropped++;
		return TRUE;
	}

	Neighbours.RouteQuery( pSearch, pPacket, NULL, TRUE );

	Network.OnQuerySearch( pSearch );

	CLocalSearch pLocal( pSearch, &pSearch->m_pEndpoint );
	pLocal.Execute();
	
	Send( &pSearch->m_pEndpoint, Neighbours.CreateQueryWeb( pSearch->m_oGUID ), TRUE );
	
	delete pSearch;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams QUERY ACK packet handler

BOOL CDatagrams::OnQueryAck(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	{
		CQuickLock oLock( HostCache.Gnutella2.m_pSection );

		CHostCacheHost* pCache = HostCache.Gnutella2.Add( &pHost->sin_addr, htons( pHost->sin_port ) );
		if ( pCache ) pCache->m_tAck = pCache->m_nFailures = 0;
	}

	Hashes::Guid oGUID;
	
	if ( SearchManager.OnQueryAck( pPacket, pHost, oGUID ) )
	{
		CNeighbour* pNeighbour = NULL;
		SOCKADDR_IN pEndpoint;

		if ( Network.QueryRoute->Lookup( oGUID, &pNeighbour, &pEndpoint ) )
		{
			// TODO: Add a "FR" from tag

			if ( pNeighbour != NULL && pNeighbour->m_nNodeType == ntLeaf )
			{
				pNeighbour->Send( pPacket, FALSE, FALSE );
			}
			else
			{
				// Don't route it on via UDP
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams HIT packet handler

BOOL CDatagrams::OnCommonHit(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	int nHops = 0;
	CQueryHit* pHits = CQueryHit::FromPacket( pPacket, &nHops );

	if ( pHits == NULL )
	{
		pPacket->Debug( _T("Malformed Hit") );
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT,
			(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	// The sender IP of this hit should match the Node Address contained in the packet
	// If it doesn't we'll drop it.
	if ( pHits->m_pAddress.S_un.S_addr != pHost->sin_addr.S_un.S_addr )
	{
		pPacket->Debug( _T("Hit sender IP does not match \"NA\"") );
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT,
			(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );
		Statistics.Current.Gnutella2.Dropped++;
		pHits->Delete();
		return FALSE;
	}

	if ( Security.IsDenied( &pHits->m_pAddress ) )
	{
		pPacket->Debug( _T("Security manager denied Hit") );
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT,
			(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );
		Statistics.Current.Gnutella2.Dropped++;
		pHits->Delete();
		return FALSE;
	}

	Network.NodeRoute->Add( pHits->m_oClientID, pHost );

	// Don't route exceeded hits
	if ( nHops <= (int)Settings.Gnutella1.MaximumTTL &&
		SearchManager.OnQueryHits( pHits ) )
	{
		Network.RouteHits( pHits, pPacket );
	}

	Network.OnQueryHits( pHits );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams QUERY KEY REQUEST packet handler

BOOL CDatagrams::OnQueryKeyRequest(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	if ( ! Neighbours.IsG2Hub() )
	{
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	DWORD nRequestedAddress = pHost->sin_addr.S_un.S_addr;
	WORD nRequestedPort = ntohs( pHost->sin_port );
	DWORD nSendingAddress = pHost->sin_addr.S_un.S_addr;

	if ( pPacket->m_bCompound )
	{
		G2_PACKET nType;
		DWORD nLength;

		while ( pPacket->ReadPacket( nType, nLength ) )
		{
			DWORD nOffset = pPacket->m_nPosition + nLength;

			if ( nType == G2_PACKET_REQUEST_ADDRESS && nLength >= 6 )
			{
				nRequestedAddress	= pPacket->ReadLongLE();
				nRequestedPort		= pPacket->ReadShortBE();
			}
			else if ( nType == G2_PACKET_SEND_ADDRESS && nLength >= 4 )
			{
				nSendingAddress		= pPacket->ReadLongLE();
			}

			pPacket->m_nPosition = nOffset;
		}
	}

	CString strNode = inet_ntoa( *(IN_ADDR*)&nRequestedAddress );
	theApp.Message( MSG_DEBUG, _T("Node %s asked for a query key for node %s:%i"),
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), (LPCTSTR)strNode, nRequestedPort );

	if ( Network.IsFirewalledAddress( &nRequestedAddress, TRUE ) || 0 == nRequestedPort ) return TRUE;

	DWORD nKey = Network.QueryKeys->Create( nRequestedAddress );

	CG2Packet* pAnswer = CG2Packet::New( G2_PACKET_QUERY_KEY_ANS, TRUE );

	pAnswer->WritePacket( G2_PACKET_QUERY_KEY, 4 );
	pAnswer->WriteLongBE( nKey );

	if ( nRequestedAddress != nSendingAddress )
	{
		pAnswer->WritePacket( G2_PACKET_SEND_ADDRESS, 4 );
		pAnswer->WriteLongLE( nSendingAddress );
	}

	Send( (IN_ADDR*)&nRequestedAddress, nRequestedPort, pAnswer, TRUE );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams QUERY KEY ANSWER packet handler

BOOL CDatagrams::OnQueryKeyAnswer(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound )
	{
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	DWORD nKey = 0, nAddress = 0;

	G2_PACKET nType;
	DWORD nLength;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;

		if ( nType == G2_PACKET_QUERY_KEY && nLength >= 4 )
		{
			nKey = pPacket->ReadLongBE();
		}
		else if ( nType == G2_PACKET_SEND_ADDRESS && nLength >= 4 )
		{
			nAddress = pPacket->ReadLongLE();
		}

		pPacket->m_nPosition = nOffset;
	}

	theApp.Message( MSG_DEBUG, _T("Got a query key for %s:%lu: 0x%x"),
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), htons( pHost->sin_port ), nKey );

	{
		CQuickLock oLock( HostCache.Gnutella2.m_pSection );

		CHostCacheHost* pCache = HostCache.Gnutella2.Add(
			&pHost->sin_addr, htons( pHost->sin_port ) );
		if ( pCache != NULL ) pCache->SetKey( nKey );
	}

	if ( nAddress != 0 && ! Network.IsSelfIP( *(IN_ADDR*)&nAddress ) )
	{
		if ( CNeighbour* pNeighbour = Neighbours.Get( (IN_ADDR*)&nAddress ) )
		{
			BYTE* pOut = pPacket->WriteGetPointer( 11, 0 );

			if ( pOut == NULL )
			{
				theApp.Message( MSG_ERROR, _T("Memory allocation error in CDatagrams::OnQueryKeyAnswer()") );
				return TRUE;
			}

			*pOut++ = 0x50;
			*pOut++ = 6;
			*pOut++ = 'Q';
			*pOut++ = 'N';
			*pOut++ = 'A';
			*pOut++ = pHost->sin_addr.S_un.S_un_b.s_b1;
			*pOut++ = pHost->sin_addr.S_un.S_un_b.s_b2;
			*pOut++ = pHost->sin_addr.S_un.S_un_b.s_b3;
			*pOut++ = pHost->sin_addr.S_un.S_un_b.s_b4;

			if ( pPacket->m_bBigEndian )
			{
				*pOut++ = (BYTE)( pHost->sin_port & 0xFF );
				*pOut++ = (BYTE)( pHost->sin_port >> 8 );
			}
			else
			{
				*pOut++ = (BYTE)( pHost->sin_port >> 8 );
				*pOut++ = (BYTE)( pHost->sin_port & 0xFF );
			}

			pNeighbour->Send( pPacket, FALSE, FALSE );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams PUSH packet handler

BOOL CDatagrams::OnPush(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	DWORD nLength = pPacket->GetRemaining();

	if ( ! pPacket->SkipCompound( nLength, 6 ) )
	{
		theApp.Message( MSG_ERROR, _T("G2UDP: Invalid PUSH packet received from %s"), 
			(LPCTSTR)inet_ntoa( pHost->sin_addr ) );
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	DWORD nAddress	= pPacket->ReadLongLE();
	WORD nPort		= pPacket->ReadShortBE();

	if ( Security.IsDenied( (IN_ADDR*)&nAddress ) ||
		 Network.IsFirewalledAddress( &nAddress ) )
	{
		Statistics.Current.Gnutella2.Dropped++;
		return TRUE;
	}

	Handshakes.PushTo( (IN_ADDR*)&nAddress, nPort );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams CRAWL packet handler

BOOL CDatagrams::OnCrawlRequest(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound )
	{
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	BOOL bWantLeaves	= FALSE;
	BOOL bWantNames		= FALSE;
	BOOL bWantGPS		= FALSE;
	BOOL bWantREXT		= FALSE;
	BOOL bIsHub			= ( ! Neighbours.IsG2Leaf() ) && ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() );

	G2_PACKET nType;
	DWORD nLength;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		DWORD nNext = pPacket->m_nPosition + nLength;

		if ( nType == G2_PACKET_CRAWL_RLEAF )
		{
			bWantLeaves = TRUE;
		}
		else if ( nType == G2_PACKET_CRAWL_RNAME )
		{
			bWantNames = TRUE;
		}
		else if ( nType == G2_PACKET_CRAWL_RGPS )
		{
			bWantGPS = TRUE;
		}
		else if ( nType == G2_PACKET_CRAWL_REXT )
		{
			bWantREXT = TRUE;
		}

		pPacket->m_nPosition = nNext;
	}

	pPacket = CG2Packet::New( G2_PACKET_CRAWL_ANS, TRUE );

	CString strNick;
	DWORD nGPS = 0;
	CString vendorCode;
	CString currentVersion;

	if ( bWantNames ) strNick = MyProfile.GetNick().Left( 255 ); //trim if over 255 characters

	if ( bWantGPS ) nGPS = MyProfile.GetPackedGPS();

	if ( bWantREXT )
	{
		vendorCode = VENDOR_CODE;
		currentVersion = Settings.SmartAgent();
	}

	pPacket->WritePacket(
		G2_PACKET_SELF,
		16 + ( strNick.GetLength() ? pPacket->GetStringLen( strNick ) + 6 : 0 ) +
			( nGPS ? 5 + 4 : 0 ) + (vendorCode.GetLength() ? pPacket->GetStringLen( vendorCode ) + 3 : 0 ) +
		(currentVersion.GetLength() ? pPacket->GetStringLen( currentVersion ) + 4 : 0 ) +
		(bIsHub ? 5 : 6),
		TRUE );

	pPacket->WritePacket( G2_PACKET_NODE_ADDRESS, 6 );
	pPacket->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
	pPacket->WriteShortBE( htons( Network.m_pHost.sin_port ) );

	pPacket->WritePacket( G2_PACKET_HUB_STATUS, 2 );
	pPacket->WriteShortBE( WORD( Neighbours.GetCount( PROTOCOL_G2, -1, ntLeaf ) ) );

	if ( strNick.GetLength() )
	{
		pPacket->WritePacket( G2_PACKET_NAME, pPacket->GetStringLen( strNick) );
		pPacket->WriteString( strNick, FALSE );
	}
	if ( vendorCode.GetLength() )
	{
		pPacket->WritePacket( G2_PACKET_VENDOR, pPacket->GetStringLen( vendorCode) );
		pPacket->WriteString( vendorCode, FALSE );
	}
	if ( currentVersion.GetLength() )
	{
		pPacket->WritePacket( G2_PACKET_VERSION, pPacket->GetStringLen( currentVersion) );
		pPacket->WriteString( currentVersion, FALSE );
	}

	if ( bIsHub )
	{
		pPacket->WritePacket( G2_PACKET_HUB, 0 );
	}
	else
	{
		pPacket->WritePacket( G2_PACKET_LEAF, 0 );
	}

	if ( nGPS )
	{
		pPacket->WritePacket( G2_PACKET_GPS, 4 );
		pPacket->WriteLongBE( nGPS );
	}

	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );
		if ( pNeighbour->m_nState < nrsConnected ) continue;

		int nExtraLen = 0;
		strNick.Empty();
		nGPS = 0;

		if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
		{
			if ( CGProfile* pProfile = ((CG2Neighbour*)pNeighbour)->m_pProfile )
			{
				if ( bWantNames ) strNick = pProfile->GetNick().Left( 255 ); //Trim if over 255 characters

				if ( bWantGPS ) nGPS = pProfile->GetPackedGPS();

				if ( strNick.GetLength() ) nExtraLen += 6 + pPacket->GetStringLen( strNick );
				if ( nGPS ) nExtraLen += 9;
			}
		}

		if ( pNeighbour->m_nProtocol == PROTOCOL_G2 &&
			 pNeighbour->m_nNodeType != ntLeaf )
		{
			pPacket->WritePacket( G2_PACKET_NEIGHBOUR_HUB, 16 + nExtraLen, TRUE );

			pPacket->WritePacket( G2_PACKET_NODE_ADDRESS, 6 );
			pPacket->WriteLongLE( pNeighbour->m_pHost.sin_addr.S_un.S_addr );
			pPacket->WriteShortBE( htons( pNeighbour->m_pHost.sin_port ) );

			pPacket->WritePacket( G2_PACKET_HUB_STATUS, 2 );
			pPacket->WriteShortBE( (WORD)((CG2Neighbour*)pNeighbour)->m_nLeafCount );
		}
		else if ( pNeighbour->m_nNodeType == ntLeaf && bWantLeaves )
		{
			pPacket->WritePacket( G2_PACKET_NEIGHBOUR_LEAF, 10 + nExtraLen, TRUE );

			pPacket->WritePacket( G2_PACKET_NODE_ADDRESS, 6 );
			pPacket->WriteLongLE( pNeighbour->m_pHost.sin_addr.S_un.S_addr );
			pPacket->WriteShortBE( htons( pNeighbour->m_pHost.sin_port ) );
		}
		else
		{
			nExtraLen = 0;
		}

		if ( nExtraLen > 0 )
		{
			if ( strNick.GetLength() )
			{
				pPacket->WritePacket( G2_PACKET_NAME, pPacket->GetStringLen( strNick ) );
				pPacket->WriteString( strNick, FALSE );
			}

			if ( nGPS )
			{
				pPacket->WritePacket( G2_PACKET_GPS, 4 );
				pPacket->WriteLongBE( nGPS );
			}
		}
	}

	Send( pHost, pPacket );

	return TRUE;
}

BOOL CDatagrams::OnCrawlAnswer(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	CrawlSession.OnCrawl( pHost, pPacket );
	return TRUE;
}

BOOL CDatagrams::OnDiscovery(SOCKADDR_IN* pHost, CG2Packet* /*pPacket*/)
{
	Send( pHost, CG2Neighbour::CreateKHLPacket( NULL, TRUE ), TRUE, 0, FALSE );

	return TRUE;
}

BOOL CDatagrams::OnKHL(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	return CG2Neighbour::ParseKHLPacket( pPacket, pHost );
}

// KHLA - KHL(Known Hub List) Answer, go over G2 UDP packet more like Gnutella2 version of UDPHC
// Better put cache as security to prevent attack, such as flooding cache with invalid host addresses.
BOOL CDatagrams::OnKHLA(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound )
	{
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE; // if it is not Compound packet, it is basically malformed packet
	}

	CDiscoveryService * pService = DiscoveryServices.GetByAddress( &(pHost->sin_addr) , ntohs(pHost->sin_port), 4 );

	if (	pService == NULL &&
		(	Network.IsFirewalledAddress( (LPVOID*)&pHost->sin_addr, TRUE ) ||
			Network.IsReserved( &pHost->sin_addr ) ||
			Security.IsDenied( &pHost->sin_addr ) )
		)
	{
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	G2_PACKET nType, nInner;
	DWORD nLength, nInnerLength, tAdjust = 0;
	BOOL bCompound;
	int nCount = 0;

	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	while ( pPacket->ReadPacket( nType, nLength, &bCompound ) )
	{
		DWORD nNext = pPacket->m_nPosition + nLength;

		if ( nType == G2_PACKET_NEIGHBOUR_HUB ||
			nType == G2_PACKET_CACHED_HUB )
		{
			DWORD nAddress = 0, tSeen = tNow;
			WORD nPort = 0;
			CString strVendor;

			if ( bCompound || G2_PACKET_NEIGHBOUR_HUB == nType )
			{
				while ( pPacket->m_nPosition < nNext && pPacket->ReadPacket( nInner, nInnerLength ) )
				{
					DWORD nNextX = pPacket->m_nPosition + nInnerLength;

					if ( nInner == G2_PACKET_NODE_ADDRESS && nInnerLength >= 6 )
					{
						nAddress = pPacket->ReadLongLE();
						nPort = pPacket->ReadShortBE();
					}
					else if ( nInner == G2_PACKET_VENDOR && nInnerLength >= 4 )
					{
						strVendor = pPacket->ReadString( 4 );
					}
					else if ( nInner == G2_PACKET_TIMESTAMP && nInnerLength >= 4 )
					{
						tSeen = pPacket->ReadLongBE() + tAdjust;
					}

					pPacket->m_nPosition = nNextX;
				}

				nLength = nNext - pPacket->m_nPosition;
			}

			if ( nLength >= 6 )
			{
				nAddress = pPacket->ReadLongLE();
				nPort = pPacket->ReadShortBE();
				if ( nLength >= 10 ) tSeen = pPacket->ReadLongBE() + tAdjust;
			}

			CHostCacheHost* pCached = HostCache.Gnutella2.Add(
				(IN_ADDR*)&nAddress, nPort, tSeen, strVendor );
			if ( pCached != NULL )
			{
				nCount++;
			}

		}
		else if ( nType == G2_PACKET_TIMESTAMP && nLength >= 4 )
		{
			tAdjust = (LONG)tNow - (LONG)pPacket->ReadLongBE();
		}

		pPacket->m_nPosition = nNext;
	}

	if ( pService != NULL )
	{
		pService->OnSuccess();
		pService->m_nHosts = nCount;
	}

	return TRUE;
}

// KHLR - KHL(Known Hub List) request, go over UDP packet more like UDPHC for G1.
BOOL CDatagrams::OnKHLR(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	UNUSED_ALWAYS(pPacket);

	if ( Security.IsDenied( &pHost->sin_addr ) || Network.IsFirewalledAddress( (LPVOID*)&pHost->sin_addr, TRUE ) ||
		Network.IsReserved( &pHost->sin_addr ) )
	{
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	CG2Packet* pKHLA = CG2Packet::New( G2_PACKET_KHL_ANS, TRUE );

	//	DWORD nBase = pPacket->m_nPosition;

	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		CG2Neighbour* pNeighbour = (CG2Neighbour*)Neighbours.GetNext( pos );

		if (pNeighbour->m_nProtocol == PROTOCOL_G2 &&
			pNeighbour->m_nState == nrsConnected &&
			pNeighbour->m_nNodeType != ntLeaf &&
			! Network.IsSelfIP( pNeighbour->m_pHost.sin_addr ) )
		{
			if ( pNeighbour->m_pVendor && pNeighbour->m_pVendor->m_sCode.GetLength() == 4 )
			{
				pKHLA->WritePacket( G2_PACKET_NEIGHBOUR_HUB, 16 + 6, TRUE );// 4
				pKHLA->WritePacket( G2_PACKET_HUB_STATUS, 4 );				// 4
				pKHLA->WriteShortBE( (WORD)pNeighbour->m_nLeafCount );		// 2
				pKHLA->WriteShortBE( (WORD)pNeighbour->m_nLeafLimit );		// 2
				pKHLA->WritePacket( G2_PACKET_VENDOR, 4 );					// 3
				pKHLA->WriteString( pNeighbour->m_pVendor->m_sCode );		// 5
			}
			else
			{
				pKHLA->WritePacket( G2_PACKET_NEIGHBOUR_HUB, 9 + 6, TRUE );	// 4
				pKHLA->WritePacket( G2_PACKET_HUB_STATUS, 4 );				// 4
				pKHLA->WriteShortBE( (WORD)pNeighbour->m_nLeafCount );		// 2
				pKHLA->WriteShortBE( (WORD)pNeighbour->m_nLeafLimit );		// 2
				pKHLA->WriteByte( 0 );										// 1
			}

			pKHLA->WriteLongLE( pNeighbour->m_pHost.sin_addr.S_un.S_addr );	// 4
			pKHLA->WriteShortBE( htons( pNeighbour->m_pHost.sin_port ) );	// 2
		}
	}

	int nCount = Settings.Gnutella2.KHLHubCount;
	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	pKHLA->WritePacket( G2_PACKET_TIMESTAMP, 4 );
	pKHLA->WriteLongBE( tNow );

	{
		CQuickLock oLock( HostCache.Gnutella2.m_pSection );

		for ( CHostCacheIterator i = HostCache.Gnutella2.Begin() ;
			i != HostCache.Gnutella2.End() && nCount > 0; ++i )
		{
			CHostCacheHost* pCachedHost = (*i);

			if ( pCachedHost->CanQuote( tNow ) &&
				Neighbours.Get( &pCachedHost->m_pAddress ) == NULL &&
				! Network.IsSelfIP( pCachedHost->m_pAddress ) )
			{

				BOOL bCompound = ( pCachedHost->m_pVendor && pCachedHost->m_pVendor->m_sCode.GetLength() > 0 );
				CG2Packet* pCHPacket = CG2Packet::New( G2_PACKET_CACHED_HUB, bCompound );

				if ( bCompound )
				{
					pCHPacket->WritePacket( G2_PACKET_VENDOR, pCachedHost->m_pVendor->m_sCode.GetLength() );
					pCHPacket->WriteString( pCachedHost->m_pVendor->m_sCode );
				}

				pCHPacket->WriteLongLE( pCachedHost->m_pAddress.S_un.S_addr );					// 4
				pCHPacket->WriteShortBE( pCachedHost->m_nPort );								// 2
				pCHPacket->WriteLongBE( pCachedHost->Seen() );									// 4
				pKHLA->WritePacket( pCHPacket );
				pCHPacket->Release();


				nCount--;
			}
		}
	}
	Send( pHost, pKHLA );

	return TRUE;
}
