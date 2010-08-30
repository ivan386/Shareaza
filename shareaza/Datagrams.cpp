//
// Datagrams.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
#include "Statistics.h"
#include "Network.h"
#include "Datagrams.h"
#include "Datagram.h"
#include "DatagramPart.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "EDPacket.h"
#include "DCPacket.h"
#include "BENode.h"
#include "Security.h"
#include "DHT.h"

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
	if ( IsValid() ) return FALSE;

	m_hSocket = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( ! IsValid() ) return FALSE;

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
		theApp.Message( MSG_INFO, IDS_NETWORK_LISTENING_UDP,
			(LPCTSTR)CString( inet_ntoa( saHost.sin_addr ) ), htons( saHost.sin_port ) );
	}

	WSAEventSelect( m_hSocket, Network.GetWakeupEvent(), FD_READ );

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
	if ( ! IsValid() ) return;

	CNetwork::CloseSocket( m_hSocket, false );

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

BOOL CDatagrams::Send(const IN_ADDR* pAddress, WORD nPort, CPacket* pPacket, BOOL bRelease, LPVOID pToken, BOOL bAck)
{
	SOCKADDR_IN pHost = {};
	pHost.sin_family = PF_INET;
	pHost.sin_addr = *pAddress;
	pHost.sin_port = htons( nPort );

	return Send( &pHost, pPacket, bRelease, pToken, bAck );
}

BOOL CDatagrams::Send(const SOCKADDR_IN* pHost, const CBuffer& pOutput)
{
	ASSERT( pHost != NULL && pOutput.m_pBuffer != NULL );

	if ( ! IsValid() || Security.IsDenied( &pHost->sin_addr ) )
	{
		return FALSE;
	}

	CNetwork::SendTo( m_hSocket, (const char*)pOutput.m_pBuffer, pOutput.m_nLength, pHost );

	m_nOutPackets++;

	return TRUE;
}

BOOL CDatagrams::Send(const SOCKADDR_IN* pHost, CPacket* pPacket, BOOL bRelease, LPVOID pToken, BOOL bAck)
{
	ASSERT( pHost != NULL && pPacket != NULL );

	if ( ! IsValid() || Security.IsDenied( &pHost->sin_addr ) )
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
			CNetwork::SendTo( m_hSocket, (LPSTR)pBuffer.m_pBuffer, pBuffer.m_nLength, pHost );

			m_nOutPackets++;
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

		CNetwork::SendTo( m_hSocket, (LPSTR)pBuffer.m_pBuffer, pBuffer.m_nLength, pHost );

		m_nOutPackets++;

		return TRUE;
	}
	else if ( pPacket->m_nProtocol == PROTOCOL_DC )
	{
		// Quick hack
		CBuffer pBuffer;

		((CDCPacket*)pPacket)->ToBuffer( &pBuffer );
		pPacket->SmartDump( pHost, TRUE, TRUE );
		if ( bRelease ) pPacket->Release();

		CNetwork::SendTo( m_hSocket, (LPSTR)pBuffer.m_pBuffer, pBuffer.m_nLength, pHost );

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
	if ( ! IsValid() ) return;

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
				CNetwork::SendTo( m_hSocket, (LPCSTR)pPacket, nPacket, &pDG->m_pHost );

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
	if ( ! IsValid() )
		return FALSE;

	SOCKADDR_IN pFrom = {};
	int nLength	= CNetwork::RecvFrom( m_hSocket, (char*)m_pReadBuffer, sizeof( m_pReadBuffer ), &pFrom );

	if ( nLength < 1 )
		return FALSE;

	// Clear rest of buffer for security reasons
	ZeroMemory( m_pReadBuffer + nLength, sizeof( m_pReadBuffer ) - nLength );

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

	if ( Security.IsDenied( &pFrom.sin_addr ) )
		return TRUE;

	if ( Network.IsFirewalledAddress( &pFrom.sin_addr, Settings.Connection.IgnoreOwnUDP ) )
	{
		theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING,
			_T("UDP: Dropped datagram (%i bytes) from %s."),
			nLength, (LPCTSTR)CString( inet_ntoa( pFrom.sin_addr ) ) );
		return TRUE;
	}

	if ( ! OnDatagram( &pFrom, m_pReadBuffer, nLength ) )
	{
		// Report unknown packets
		CString strText;
		for ( int i = 0; i < nLength && i < 80; i++ )
		{
			strText += ( ( m_pReadBuffer[ i ] < ' ' ) ? '.' : (char)m_pReadBuffer[ i ] );
		}
		theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING,
			_T("UDP: Recieved unknown packet (%i bytes) from %s: "),
			nLength, (LPCTSTR)CString( inet_ntoa( pFrom.sin_addr ) ), strText );
		return TRUE;
	}

	// Packet fully handled

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams datagram handler

BOOL CDatagrams::OnDatagram(const SOCKADDR_IN* pHost, const BYTE* pBuffer, DWORD nLength)
{
	BOOL bHandled = FALSE;

	// Detect Gnutella 1 packets
	if ( nLength >= sizeof(GNUTELLAPACKET) )
	{
		const GNUTELLAPACKET* pG1UDP = (const GNUTELLAPACKET*)pBuffer;
		if ( ( sizeof(GNUTELLAPACKET) + pG1UDP->m_nLength ) == nLength )
		{
			if ( CG1Packet* pPacket = CG1Packet::New( pG1UDP ) )
			{
				try
				{
					m_nInPackets++;

					bHandled = pPacket->OnPacket( pHost );
				}
				catch ( CException* pException )
				{
					pException->Delete();
					pPacket->Debug( _T("Malformed packet.") );
				}
				pPacket->Release();

				if ( bHandled )
					return TRUE;
			}
		}
	}

	// Detect Gnutella 2 packets
	if ( nLength >= sizeof(SGP_HEADER) )
	{
		const SGP_HEADER* pSGP = (const SGP_HEADER*)pBuffer;
		if ( ( *(DWORD*)pSGP->szTag & 0x00ffffff ) == ( *(DWORD*)SGP_TAG_2 & 0x00ffffff ) &&
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
	}

	// Detect ED2K and KAD packets
	if ( nLength > sizeof(ED2K_UDP_HEADER) )
	{
		const ED2K_UDP_HEADER* pMULE = (const ED2K_UDP_HEADER*)pBuffer;
		switch ( pMULE->nProtocol )
		{
		case ED2K_PROTOCOL_EDONKEY:
		case ED2K_PROTOCOL_EMULE:
		case ED2K_PROTOCOL_EMULE_PACKED:
		case ED2K_PROTOCOL_KAD:
		case ED2K_PROTOCOL_KAD_PACKED:
		case ED2K_PROTOCOL_REVCONNECT:
		case ED2K_PROTOCOL_REVCONNECT_PACKED:
			if ( CEDPacket* pPacket = CEDPacket::New( pMULE, nLength ) )
			{
				try
				{
					m_nInPackets++;

					bHandled = pPacket->OnPacket( pHost );
				}
				catch ( CException* pException )
				{
					pException->Delete();
					pPacket->Debug( _T("Malformed packet.") );
				}
				pPacket->Release();

				if ( bHandled )
					return TRUE;
			}
			// TODO: Detect obfuscated eMule packets
		}
	}

	// Detect DC++ packets
	if ( nLength > 2 && pBuffer[ 0 ] == '$' && pBuffer[ nLength - 1 ] == '|' )
	{
		if ( CDCPacket* pPacket = CDCPacket::New( pBuffer, nLength ) )
		{
			m_nInPackets++;

			bHandled = pPacket->OnPacket( pHost );

			pPacket->Release();

			if ( bHandled )
				return TRUE;
		}
	}

	// Detect BitTorrent packets
	if ( nLength > 16 )
	{
		try
		{
			auto_ptr< CBENode > pNode( new CBENode() );
			if ( ! pNode.get() )
				AfxThrowMemoryException();
			const BYTE* pInput = pBuffer;
			DWORD nInput = nLength;
			pNode->Decode( pInput, nInput, nInput );

			m_nInPackets++;

			if ( DHT.OnPacket( pHost, pNode.get() ) )
				return TRUE;
		}
		catch ( CException* pException )
		{
			pException->Delete();
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams SGP receive handler

BOOL CDatagrams::OnReceiveSGP(const SOCKADDR_IN* pHost, const SGP_HEADER* pHeader, DWORD nLength)
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

		memcpy( pAck.szTag, SGP_TAG_2, 3 );
		pAck.nFlags		= 0;
		pAck.nSequence	= pHeader->nSequence;
		pAck.nPart		= pHeader->nPart;
		pAck.nCount		= 0;

		CNetwork::SendTo( m_hSocket, (LPCSTR)&pAck, sizeof(pAck), pHost );
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
						m_nInPackets++;

						pPacket->OnPacket( pHost );
					}
					catch ( CException* pException )
					{
						pException->Delete();
						pPacket->Debug( _T("Malformed packet.") );
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
				m_nInPackets++;

				pPacket->OnPacket( pHost );
			}
			catch ( CException* pException )
			{
				pException->Delete();
				pPacket->Debug( _T("Malformed packet.") );
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
// CDatagrams SGP acknowledgment handler

BOOL CDatagrams::OnAcknowledgeSGP(const SOCKADDR_IN* pHost, const SGP_HEADER* pHeader, DWORD /*nLength*/)
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
