//
// Datagrams.cpp
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

#include "G2Neighbour.h"
#include "G2Packet.h"
#include "EDClients.h"
#include "EDPacket.h"
#include "Security.h"
#include "HostCache.h"
#include "QueryKeys.h"
#include "LibraryMaps.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define HASH_SIZE		32
#define HASH_MASK		31

#define TEMP_BUFFER		4096
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
	
	SOCKADDR_IN saHost;
	
	if ( Network.Resolve( Settings.Connection.InHost, Settings.Connection.InPort, &saHost ) )
	{
		// Inbound resolved
		if ( ! Settings.Connection.InBind ) saHost.sin_addr.S_un.S_addr = 0;
	}
	else if ( Network.Resolve( Settings.Connection.OutHost, Settings.Connection.InPort, &saHost ) )
	{
		// Outbound resolved
	}
	else
	{
		saHost = Network.m_pHost;
		if ( ! Settings.Connection.InBind ) saHost.sin_addr.S_un.S_addr = 0;
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
	
	for ( nPos = m_nInputBuffer ; nPos ; nPos--, pDGI++ )
	{
		pDGI->m_pNextHash = ( nPos == 1 ) ? NULL : ( pDGI + 1 );
	}
	
	m_nOutputBuffer	= Settings.Gnutella2.UdpOutFrames; // 128;
	m_pOutputBuffer	= new CDatagramOut[ m_nOutputBuffer ];
	m_pOutputFree	= m_pOutputBuffer;
	
	CDatagramOut* pDGO = m_pOutputBuffer;
	
	for ( nPos = m_nOutputBuffer ; nPos ; nPos--, pDGO++ )
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
}

//////////////////////////////////////////////////////////////////////
// CDatagrams stable test

BOOL CDatagrams::IsStable()
{
	if ( m_hSocket == INVALID_SOCKET ) return FALSE;
	if ( ! Network.IsListening() ) return FALSE;
	
	// Are we stable OR know we are not firewalled
	return m_bStable || ! Settings.Connection.Firewalled;
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
		pPacket->SmartDump( NULL, &pHost->sin_addr, TRUE );
		if ( bRelease ) pPacket->Release();
		
		if ( ntohs( pHost->sin_port ) != 4669 )	// Hack
		{
			sendto( m_hSocket, (LPSTR)pBuffer.m_pBuffer, pBuffer.m_nLength, 0,
				(SOCKADDR*)pHost, sizeof(SOCKADDR_IN) );
		}
		
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
	
	BYTE nHash	= pHost->sin_addr.S_un.S_un_b.s_b1
				+ pHost->sin_addr.S_un.S_un_b.s_b2
				+ pHost->sin_addr.S_un.S_un_b.s_b3
				+ pHost->sin_addr.S_un.S_un_b.s_b4
				+ pHost->sin_port
				+ pDG->m_nSequence;
	
	CDatagramOut** pHash = m_pOutputHash + ( nHash & HASH_MASK );
	
	if ( *pHash ) (*pHash)->m_pPrevHash = &pDG->m_pNextHash;
	pDG->m_pNextHash = *pHash;
	pDG->m_pPrevHash = pHash;
	*pHash = pDG;
	
	m_nOutPackets++;
	
	pPacket->SmartDump( NULL, &pHost->sin_addr, TRUE );
		
#ifdef DEBUG_UDP
	pPacket->Debug( _T("UDP Out") );
	theApp.Message( MSG_DEBUG, _T("UDP: Queued (#%i) x%i for %s:%lu"),
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
		for ( CDatagramOut* pDG = m_pOutputFirst ; pDG ; pDG = pDG->m_pPrevTime )
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
					(LPCTSTR)CString( inet_ntoa( pDG->m_pHost.sin_addr ) 0,
					htons( pDG->m_pHost.sin_port ) );
#endif
				
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

#define TEMP_BUFFER 4096

BOOL CDatagrams::TryRead()
{
	static BYTE pBuffer[ TEMP_BUFFER ];
	int nLength, nFromLen;
	SOCKADDR_IN pFrom;
	
	nFromLen = sizeof(pFrom);
	nLength	= recvfrom( m_hSocket, (LPSTR)pBuffer, TEMP_BUFFER, 0,
						(SOCKADDR*)&pFrom, &nFromLen );
	
	if ( nLength < 1 ) return FALSE;
	
	if ( m_mInput.pHistory && nLength > 0 )
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
	
	if ( Security.IsAccepted( &pFrom.sin_addr ) )
	{
		OnDatagram( &pFrom, pBuffer, nLength );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams datagram handler

BOOL CDatagrams::OnDatagram(SOCKADDR_IN* pHost, BYTE* pBuffer, DWORD nLength)
{
	ED2K_UDP_HEADER* pMULE = (ED2K_UDP_HEADER*)pBuffer;
	
	if ( nLength > sizeof(*pMULE) && (
		 pMULE->nProtocol == ED2K_PROTOCOL_EDONKEY ||
		 pMULE->nProtocol == ED2K_PROTOCOL_EMULE ||
		 pMULE->nProtocol == ED2K_PROTOCOL_PACKED ) )
	{
		CEDPacket* pPacket = CEDPacket::New( pMULE, nLength );
		
		if ( ! pPacket->InflateOrRelease( ED2K_PROTOCOL_EMULE ) )
		{
			pPacket->SmartDump( NULL, &pHost->sin_addr, FALSE );
			EDClients.OnUDP( pHost, pPacket );
			pPacket->Release();
		}
		
		return TRUE;
	}
	
	SGP_HEADER* pSGP = (SGP_HEADER*)pBuffer;
	
	if ( nLength >= sizeof(*pSGP) && strncmp( pSGP->szTag, SGP_TAG_2, 3 ) == 0 )
	{
		if ( pSGP->nPart == 0 ) return FALSE;
		if ( pSGP->nCount && pSGP->nPart > pSGP->nCount ) return FALSE;
		
		nLength -= sizeof(*pSGP);
		
		if ( pSGP->nCount )
		{
			OnReceiveSGP( pHost, pSGP, nLength );
		}
		else
		{
			OnAcknowledgeSGP( pHost, pSGP, nLength );
		}
		
		return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams SGP receive handler

BOOL CDatagrams::OnReceiveSGP(SOCKADDR_IN* pHost, SGP_HEADER* pHeader, DWORD nLength)
{
#ifdef DEBUG_UDP
	theApp.Message( MSG_DEBUG, _T("UDP: Received (#%i) %i of %i from %s"),
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
	
	BYTE nHash	= pHost->sin_addr.S_un.S_un_b.s_b1
				+ pHost->sin_addr.S_un.S_un_b.s_b2
				+ pHost->sin_addr.S_un.S_un_b.s_b3
				+ pHost->sin_addr.S_un.S_un_b.s_b4
				+ pHost->sin_port
				+ pHeader->nSequence;
	
	CDatagramIn** pHash = m_pInputHash + ( nHash & HASH_MASK );
	
	for ( CDatagramIn* pDG = *pHash ; pDG ; pDG = pDG->m_pNextHash )
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

BOOL CDatagrams::OnAcknowledgeSGP(SOCKADDR_IN* pHost, SGP_HEADER* pHeader, DWORD nLength)
{
#ifdef DEBUG_UDP
	theApp.Message( MSG_DEBUG, _T("UDP: Received ack (#%i) %i from %s"),
		pHeader->nSequence, pHeader->nPart, (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );
#endif
	
	BYTE nHash	= pHost->sin_addr.S_un.S_un_b.s_b1
				+ pHost->sin_addr.S_un.S_un_b.s_b2
				+ pHost->sin_addr.S_un.S_un_b.s_b3
				+ pHost->sin_addr.S_un.S_un_b.s_b4
				+ pHost->sin_port
				+ pHeader->nSequence;
	
	CDatagramOut** pHash = m_pOutputHash + ( nHash & HASH_MASK );
	
	for ( CDatagramOut* pDG = *pHash ; pDG ; pDG = pDG->m_pNextHash )
	{
		if (	pDG->m_pHost.sin_addr.S_un.S_addr == pHost->sin_addr.S_un.S_addr &&
				pDG->m_pHost.sin_port == pHost->sin_port &&
				pDG->m_nSequence == pHeader->nSequence )
		{
			if ( pDG->Acknowledge( pHeader->nPart ) ) Remove( pDG );
			return TRUE;
		}
	}
	
	return FALSE;
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
// CDatagrams packet handler

BOOL CDatagrams::OnPacket(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	pPacket->SmartDump( NULL, &pHost->sin_addr, FALSE );
	
	m_nInPackets++;
	
	if ( Network.RoutePacket( pPacket ) ) return TRUE;
	
	if ( pPacket->IsType( G2_PACKET_QUERY ) )
	{
		return OnQuery( pHost, pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_QUERY_KEY_REQ ) )
	{
		return OnQueryKeyRequest( pHost, pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_HIT ) || pPacket->IsType( G2_PACKET_HIT_WRAP ) )
	{
		return OnHit( pHost, pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_QUERY_ACK ) )
	{
		return OnQueryAck( pHost, pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_QUERY_KEY_ANS ) )
	{
		return OnQueryKeyAnswer( pHost, pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_PING ) )
	{
		return OnPing( pHost, pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_PONG ) )
	{
		return OnPong( pHost, pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_PUSH ) )
	{
		return OnPush( pHost, pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_CRAWL_REQ ) )
	{
		return OnCrawlRequest( pHost, pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_CRAWL_ANS ) )
	{
		return OnCrawlAnswer( pHost, pPacket );
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams PING packet handler

BOOL CDatagrams::OnPing(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	Send( pHost, CG2Packet::New( G2_PACKET_PONG ) );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams PONG packet handler

BOOL CDatagrams::OnPong(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;
	
	BOOL bRelayed = FALSE;
	CHAR szType[9];
	DWORD nLength;
	
	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;
		if ( strcmp( szType, "RELAY" ) == 0 ) bRelayed = TRUE;
		pPacket->m_nPosition = nOffset;
	}
	
	if ( ! bRelayed ) return TRUE;
	
	if ( ! Network.IsConnectedTo( &pHost->sin_addr ) ) m_bStable = TRUE;
	
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
	
	if ( Security.IsDenied( &pSearch->m_pEndpoint.sin_addr ) )
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
		pAnswer->WritePacket( "QK", 4 );
		pAnswer->WriteLongBE( nKey );
		
		if ( pHost->sin_addr.S_un.S_addr != pSearch->m_pEndpoint.sin_addr.S_un.S_addr )
		{
			pAnswer->WritePacket( "SNA", 4 );
			pAnswer->WriteLongLE( pHost->sin_addr.S_un.S_addr );
		}
		
		Send( &pSearch->m_pEndpoint, pAnswer, TRUE );
		
		delete pSearch;
		return TRUE;
	}
	
	if ( ! Network.QueryRoute->Add( &pSearch->m_pGUID, &pSearch->m_pEndpoint ) )
	{
		CG2Packet* pAnswer = CG2Packet::New( G2_PACKET_QUERY_ACK, TRUE );
		pAnswer->WritePacket( "D", 8 );
		pAnswer->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
		pAnswer->WriteShortBE( htons( Network.m_pHost.sin_port ) );
		pAnswer->WriteShortBE( 0 );
		pAnswer->WriteByte( 0 );
		pAnswer->Write( &pSearch->m_pGUID, GUID_SIZE );
		Send( &pSearch->m_pEndpoint, pAnswer, TRUE );
		
		delete pSearch;
		Statistics.Current.Gnutella2.Dropped++;
		return TRUE;
	}
	
	Neighbours.RouteQuery( pSearch, pPacket, NULL, TRUE );
	
	Network.OnQuerySearch( pSearch );
	
	CLocalSearch pLocal( pSearch, &pSearch->m_pEndpoint );
	pLocal.Execute();
	
	Send( &pSearch->m_pEndpoint, Neighbours.CreateQueryWeb( &pSearch->m_pGUID ), TRUE );
	
	delete pSearch;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams QUERY ACK packet handler

BOOL CDatagrams::OnQueryAck(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	CHostCacheHost* pCache = HostCache.Gnutella2.Add( &pHost->sin_addr, htons( pHost->sin_port ) );
	if ( pCache ) pCache->m_tAck = pCache->m_nFailures = 0;
	
	CGUID pGUID;
	
	if ( SearchManager.OnQueryAck( pPacket, pHost, &pGUID ) )
	{
		CNeighbour* pNeighbour = NULL;
		SOCKADDR_IN pEndpoint;

		if ( Network.QueryRoute->Lookup( &pGUID, &pNeighbour, &pEndpoint ) )
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

BOOL CDatagrams::OnHit(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	int nHops = 0;
	CQueryHit* pHits = CQueryHit::FromPacket( pPacket, &nHops );
	
	if ( pHits == NULL )
	{
		pPacket->Debug( _T("BadHit") );
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT,
			(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}
	
	if ( Security.IsDenied( &pHits->m_pAddress ) || nHops > (int)Settings.Gnutella1.MaximumTTL )
	{
		pHits->Delete();
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}
	
	Network.NodeRoute->Add( &pHits->m_pClientID, pHost );
	
	if ( SearchManager.OnQueryHits( pHits ) )
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
	if ( ! Neighbours.IsHub() ) return FALSE;
	
	DWORD nRequestedAddress = pHost->sin_addr.S_un.S_addr;
	WORD nRequestedPort = ntohs( pHost->sin_port );
	DWORD nSendingAddress = pHost->sin_addr.S_un.S_addr;
	
	if ( pPacket->m_bCompound )
	{
		CHAR szType[9];
		DWORD nLength;
		
		while ( pPacket->ReadPacket( szType, nLength ) )
		{
			DWORD nOffset = pPacket->m_nPosition + nLength;
			
			if ( strcmp( szType, "RNA" ) == 0 && nLength >= 6 )
			{
				nRequestedAddress	= pPacket->ReadLongLE();
				nRequestedPort		= pPacket->ReadShortBE();
			}
			else if ( strcmp( szType, "SNA" ) == 0 && nLength >= 4 )
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
	
	pAnswer->WritePacket( "QK", 4 );
	pAnswer->WriteLongBE( nKey );
	
	if ( nRequestedAddress != nSendingAddress )
	{
		pAnswer->WritePacket( "SNA", 4 );
		pAnswer->WriteLongLE( nSendingAddress );
	}
	
	Send( (IN_ADDR*)&nRequestedAddress, nRequestedPort, pAnswer, TRUE );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams QUERY KEY ANSWER packet handler

BOOL CDatagrams::OnQueryKeyAnswer(SOCKADDR_IN* pHost, CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return FALSE;
	
	DWORD nKey = 0, nAddress = 0;
	
	CHAR szType[9];
	DWORD nLength;
	
	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;
		
		if ( strcmp( szType, "QK" ) == 0 && nLength >= 4 )
		{
			nKey = pPacket->ReadLongBE();
		}
		else if ( strcmp( szType, "SNA" ) == 0 && nLength >= 4 )
		{
			nAddress = pPacket->ReadLongLE();
		}
		
		pPacket->m_nPosition = nOffset;
	}
	
	theApp.Message( MSG_DEBUG, _T("Got a query key for %s:%lu: 0x%x"),
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), htons( pHost->sin_port ), nKey );
	
	CHostCacheHost* pCache = HostCache.Gnutella2.Add(
		&pHost->sin_addr, htons( pHost->sin_port ) );
	
	if ( pCache != NULL ) pCache->SetKey( nKey );
	
	if ( nAddress != 0 && nAddress != Network.m_pHost.sin_addr.S_un.S_addr )
	{
		if ( CNeighbour* pNeighbour = Neighbours.Get( (IN_ADDR*)&nAddress ) )
		{
			BYTE* pOut = pPacket->WriteGetPointer( 11, 0 );
			
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
		pPacket->Debug( _T("BadPush") );
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
	if ( ! pPacket->m_bCompound ) return FALSE;
	
	BOOL bWantLeaves	= FALSE;
	BOOL bWantNames		= FALSE;
	BOOL bWantGPS		= FALSE;
	
	CHAR szType[9];
	DWORD nLength;
	
	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nNext = pPacket->m_nPosition + nLength;
		
		if ( strcmp( szType, "RLEAF" ) == 0 )
		{
			bWantLeaves = TRUE;
		}
		else if ( strcmp( szType, "RNAME" ) == 0 )
		{
			bWantNames = TRUE;
		}
		else if ( strcmp( szType, "RGPS" ) == 0 )
		{
			bWantGPS = TRUE;
		}
		
		pPacket->m_nPosition = nNext;
	}
	
	pPacket = CG2Packet::New( G2_PACKET_CRAWL_ANS, TRUE );
	
	CString strNick;
	DWORD nGPS = 0;
	
	if ( bWantNames ) strNick = MyProfile.GetNick();
	if ( bWantGPS ) nGPS = MyProfile.GetPackedGPS();
	
	pPacket->WritePacket( "SELF", 16 + ( strNick.GetLength() ? pPacket->GetStringLen( strNick ) + 6 : 0 ) + ( nGPS ? 5 + 4 : 0 ), TRUE );
	
	pPacket->WritePacket( "NA", 6 );
	pPacket->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
	pPacket->WriteShortBE( htons( Network.m_pHost.sin_port ) );
	
	pPacket->WritePacket( "HS", 2 );
	pPacket->WriteShortBE( Neighbours.GetCount( -1, -1, ntLeaf ) );
	
	if ( strNick.GetLength() )
	{
		pPacket->WritePacket( "NAME", pPacket->GetStringLen( strNick) );
		pPacket->WriteString( strNick, FALSE );
	}
	
	if ( nGPS )
	{
		pPacket->WritePacket( "GPS", 4 );
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
				if ( bWantNames ) strNick = pProfile->GetNick();
				if ( bWantGPS ) nGPS = pProfile->GetPackedGPS();
				
				if ( strNick.GetLength() ) nExtraLen += 6 + pPacket->GetStringLen( strNick );
				if ( nGPS ) nExtraLen += 9;
			}
		}
		
		if ( pNeighbour->m_nProtocol == PROTOCOL_G2 &&
			 pNeighbour->m_nNodeType != ntLeaf )
		{
			pPacket->WritePacket( "NH", 16 + nExtraLen, TRUE );
			
			pPacket->WritePacket( "NA", 6 );
			pPacket->WriteLongLE( pNeighbour->m_pHost.sin_addr.S_un.S_addr );
			pPacket->WriteShortBE( htons( pNeighbour->m_pHost.sin_port ) );
			
			pPacket->WritePacket( "HS", 2 );
			pPacket->WriteShortBE( (WORD)((CG2Neighbour*)pNeighbour)->m_nLeafCount );
		}
		else if ( pNeighbour->m_nNodeType == ntLeaf && bWantLeaves )
		{
			pPacket->WritePacket( "NL", 10 + nExtraLen, TRUE );

			pPacket->WritePacket( "NA", 6 );
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
				pPacket->WritePacket( "NAME", pPacket->GetStringLen( strNick ) );
				pPacket->WriteString( strNick, FALSE );
			}
			
			if ( nGPS )
			{
				pPacket->WritePacket( "GPS", 4 );
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
