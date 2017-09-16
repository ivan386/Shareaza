//
// G2Packet.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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
#include "CrawlSession.h"
#include "Datagrams.h"
#include "DiscoveryServices.h"
#include "RouteCache.h"
#include "G1Neighbour.h"
#include "G2Neighbour.h"
#include "G2Packet.h"
#include "G1Packet.h"
#include "GProfile.h"
#include "Handshakes.h"
#include "HostCache.h"
#include "LocalSearch.h"
#include "Neighbours.h"
#include "Network.h"
#include "QueryHit.h"
#include "QueryKeys.h"
#include "QuerySearch.h"
#include "SearchManager.h"
#include "Security.h"
#include "Statistics.h"
#include "VendorCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CG2Packet::CG2PacketPool CG2Packet::POOL;


//////////////////////////////////////////////////////////////////////
// CG2Packet construction

CG2Packet::CG2Packet()
	: CPacket		( PROTOCOL_G2 )
	, m_nType		( G2_PACKET_NULL )
	, m_bCompound	( FALSE )
{
	m_bBigEndian = FALSE;
}

CG2Packet::~CG2Packet()
{
}

//////////////////////////////////////////////////////////////////////
// CG2Packet reset

void CG2Packet::Reset()
{
	CPacket::Reset();

	m_nType			= G2_PACKET_NULL;
	m_bCompound		= FALSE;
	m_bBigEndian	= FALSE;
}

//////////////////////////////////////////////////////////////////////
// CG2Packet construct from byte source

CG2Packet* CG2Packet::New(BYTE* pSource)
{
	CG2Packet* pPacket = New();

	BYTE nInput		= *pSource++;

	BYTE nLenLen	= ( nInput & 0xC0 ) >> 6;
	BYTE nTypeLen	= ( nInput & 0x38 ) >> 3;
	BYTE nFlags		= ( nInput & 0x07 );

	pPacket->m_bCompound	= ( nFlags & G2_FLAG_COMPOUND ) ? TRUE : FALSE;
	pPacket->m_bBigEndian	= ( nFlags & G2_FLAG_BIG_ENDIAN ) ? TRUE : FALSE;

	DWORD nLength = 0;

	if ( pPacket->m_bBigEndian )
	{
		for ( nLength = 0 ; nLenLen-- ; )
		{
			nLength <<= 8;
			nLength |= *pSource++;
		}
	}
	else
	{
		BYTE* pLenOut = (BYTE*)&nLength;
		while ( nLenLen-- ) *pLenOut++ = *pSource++;
	}

	nTypeLen++;
	CopyMemory( &pPacket->m_nType, pSource, nTypeLen );
	pSource += nTypeLen;

	pPacket->Write( pSource, nLength );

	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CG2Packet construct wrapping G1 packet

CG2Packet* CG2Packet::New(G2_PACKET nType, CG1Packet* pWrap, int nMinTTL)
{
	CG2Packet* pPacket = New( nType, FALSE );

	GNUTELLAPACKET pHeader;

	pHeader.m_oGUID		= pWrap->m_oGUID.storage();
	pHeader.m_nType		= pWrap->m_nType;
	pHeader.m_nTTL		= min( pWrap->m_nTTL, BYTE(nMinTTL) );
	pHeader.m_nHops		= pWrap->m_nHops;
	pHeader.m_nLength	= (LONG)pWrap->m_nLength;

	pPacket->Write( &pHeader, sizeof(pHeader) );
	pPacket->Write( pWrap->m_pBuffer, pWrap->m_nLength );

	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CG2Packet clone

CG2Packet* CG2Packet::Clone() const
{
	CG2Packet* pPacket = CG2Packet::New( m_nType, m_bCompound );
	pPacket->Write( m_pBuffer, m_nLength );
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CG2Packet sub-packet write

void CG2Packet::WritePacket(CG2Packet* pPacket)
{
	if ( pPacket == NULL ) return;
	WritePacket( pPacket->m_nType, pPacket->m_nLength, pPacket->m_bCompound );
	Write( pPacket->m_pBuffer, pPacket->m_nLength );
}

//////////////////////////////////////////////////////////////////////
// CG2Packet sub-packet write

void CG2Packet::WritePacket(G2_PACKET nType, DWORD nLength, BOOL bCompound)
{
	ASSERT( G2_TYPE_LEN( nType ) > 0 && G2_TYPE_LEN( nType ) < 9 );
	ASSERT( nLength <= 0xFFFFFF );

	BYTE nTypeLen	= (BYTE)( G2_TYPE_LEN( nType ) - 1 ) & 0x07;	// 0..7
	BYTE nLenLen	= 1;	// 1, 2, 3

	if ( nLength > 0xFF )
	{
		nLenLen++;
		if ( nLength > 0xFFFF ) nLenLen++;
	}

	BYTE nFlags = ( nLenLen << 6 ) + ( nTypeLen << 3 );

	if ( bCompound ) nFlags |= G2_FLAG_COMPOUND;
	if ( m_bBigEndian ) nFlags |= G2_FLAG_BIG_ENDIAN;

	WriteByte( nFlags );

	if ( m_bBigEndian )
	{
		switch ( nLenLen )
		{
		case 3:
			WriteByte( (BYTE)( ( nLength >> 16 ) & 0xFF ) );
		case 2:
			WriteByte( (BYTE)( ( nLength >> 8 ) & 0xFF ) );
		case 1:
			WriteByte( (BYTE)( nLength & 0xFF ) );
		}
	}
	else
	{
		Write( &nLength, nLenLen );
	}

	Write( &nType, nTypeLen + 1 );

	m_bCompound = TRUE;	// This must be compound now
}

//////////////////////////////////////////////////////////////////////
// CG2Packet sub-packet read

BOOL CG2Packet::ReadPacket(G2_PACKET& nType, DWORD& nLength, BOOL* pbCompound)
{
	if ( GetRemaining() == 0 ) return FALSE;

	BYTE nInput = ReadByte();
	if ( nInput == 0 ) return FALSE;

	BYTE nLenLen	= ( nInput & 0xC0 ) >> 6;
	BYTE nTypeLen	= ( nInput & 0x38 ) >> 3;
	BYTE nFlags		= ( nInput & 0x07 );

	if ( GetRemaining() < nTypeLen + nLenLen + 1u ) AfxThrowUserException();

	if ( m_bBigEndian )
	{
		for ( nLength = 0 ; nLenLen-- ; )
		{
			nLength <<= 8;
			nLength |= ReadByte();
		}
	}
	else
	{
		nLength = 0;
		Read( &nLength, nLenLen );
	}

	if ( GetRemaining() < nLength + nTypeLen + 1u ) AfxThrowUserException();

	nType = G2_PACKET_NULL;
	Read( &nType, nTypeLen + 1 );

	if ( pbCompound )
	{
		*pbCompound = ( nFlags & G2_FLAG_COMPOUND ) == G2_FLAG_COMPOUND;
	}
	else
	{
		if ( nFlags & G2_FLAG_COMPOUND ) SkipCompound( nLength );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Packet skip compound sub-packets

BOOL CG2Packet::SkipCompound()
{
	if ( m_bCompound )
	{
		DWORD nLength = m_nLength;
		if ( ! SkipCompound( nLength ) ) return FALSE;
	}

	return TRUE;
}

BOOL CG2Packet::SkipCompound(DWORD& nLength, DWORD nRemaining)
{
	DWORD nStart	= m_nPosition;
	DWORD nEnd		= m_nPosition + nLength;

	while ( m_nPosition < nEnd )
	{
		BYTE nInput = ReadByte();
		if ( nInput == 0 ) break;

		BYTE nLenLen	= ( nInput & 0xC0 ) >> 6;
		BYTE nTypeLen	= ( nInput & 0x38 ) >> 3;
//		BYTE nFlags		= ( nInput & 0x07 );

		if ( m_nPosition + nTypeLen + nLenLen + 1 > nEnd ) AfxThrowUserException();

		DWORD nPacket = 0;

		if ( m_bBigEndian )
		{
			for ( nPacket = 0 ; nLenLen-- ; )
			{
				nPacket <<= 8;
				nPacket |= ReadByte();
			}
		}
		else
		{
			Read( &nPacket, nLenLen );
		}

		if ( m_nPosition + nTypeLen + 1 + nPacket > nEnd ) AfxThrowUserException();

		m_nPosition += nPacket + nTypeLen + 1;
	}

	nEnd = m_nPosition - nStart;
	if ( nEnd > nLength ) AfxThrowUserException();
	nLength -= nEnd;

	return nRemaining ? nLength >= nRemaining : TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Packet read a TO block

BOOL CG2Packet::GetTo(Hashes::Guid& oGUID)
{
	if ( m_bCompound == FALSE ) return FALSE;
	if ( GetRemaining() < 4 + 16 ) return FALSE;

	BYTE* pTest = m_pBuffer + m_nPosition;

	if ( pTest[0] != 0x48 ) return FALSE;
	if ( pTest[1] != 0x10 ) return FALSE;
	if ( pTest[2] != 'T' ) return FALSE;
	if ( pTest[3] != 'O' ) return FALSE;

	CopyMemory( &oGUID[ 0 ], pTest + 4, oGUID.byteCount );

	return BOOL( oGUID.validate() );
}

//////////////////////////////////////////////////////////////////////
// CG2Packet seek to a wrapped packet (past compound)

BOOL CG2Packet::SeekToWrapped()
{
	m_nPosition = 0;

	if ( ! SkipCompound() ) return FALSE;
	if ( GetRemaining() < sizeof(GNUTELLAPACKET) ) return FALSE;

	GNUTELLAPACKET* pHead = (GNUTELLAPACKET*)( m_pBuffer + m_nPosition );
	return (DWORD)GetRemaining() >= sizeof(GNUTELLAPACKET) + pHead->m_nLength;
}

//////////////////////////////////////////////////////////////////////
// CG2Packet strings with UTF-8 encoding

CString CG2Packet::ReadString(DWORD nMaximum)
{
	CString strString;

	nMaximum = min( nMaximum, m_nLength - m_nPosition );
	if ( ! nMaximum ) return strString;

	LPCSTR pszInput	= (LPCSTR)m_pBuffer + m_nPosition;
	LPCSTR pszScan	= pszInput;
//	BOOL bEncoded	= FALSE;

    DWORD nLength = 0;
	for ( ; nLength < nMaximum ; nLength++ )
	{
		m_nPosition++;
		if ( ! *pszScan ) break;
		pszScan ++;
	}

	return UTF8Decode( pszInput, nLength );
}

void CG2Packet::WriteString(LPCTSTR pszString, BOOL bNull)
{
	if ( *pszString == NULL )
	{
		if ( bNull ) WriteByte( 0 );
		return;
	}

	CStringA strUTF8 = UTF8Encode( pszString, (int)_tcslen( pszString ) );

	if ( bNull )
	{
		Write( (LPCSTR)strUTF8, strUTF8.GetLength() + 1 );
	}
	else
		Write( (LPCSTR)strUTF8, strUTF8.GetLength() );
}

void CG2Packet::WriteString(LPCSTR pszString, BOOL bNull)
{
	if ( *pszString == NULL )
	{
		if ( bNull ) WriteByte( 0 );
		return;
	}

	Write( pszString, static_cast< int >( strlen(pszString) + ( bNull ? 1 : 0 ) ) );
}

int CG2Packet::GetStringLen(LPCTSTR pszString) const
{
	if ( *pszString == 0 )
		return 0;

	int nLength = WideCharToMultiByte( CP_UTF8, 0, pszString, -1, NULL, 0, NULL, NULL );

	return ( nLength > 0 ) ? ( nLength - 1 ) : 0;
}

//////////////////////////////////////////////////////////////////////
// CG2Packet to buffer

void CG2Packet::ToBuffer(CBuffer* pBuffer, bool /*bTCP*/)
{
	ASSERT( G2_TYPE_LEN( m_nType ) > 0 );

	BYTE nLenLen	= 1;
	BYTE nTypeLen	= (BYTE)( G2_TYPE_LEN( m_nType ) - 1 ) & 0x07;

	if ( m_nLength > 0xFF )
	{
		nLenLen++;
		if ( m_nLength > 0xFFFF ) nLenLen++;
	}

	BYTE nFlags = ( nLenLen << 6 ) + ( nTypeLen << 3 );

	if ( m_bCompound ) nFlags |= G2_FLAG_COMPOUND;
	if ( m_bBigEndian ) nFlags |= G2_FLAG_BIG_ENDIAN;

	// Abort if the buffer isn't big enough for the packet
	if ( !pBuffer->EnsureBuffer( 1 + nLenLen + nTypeLen + 1 + m_nLength ) )
		return;

	pBuffer->Add( &nFlags, 1 );

	if ( m_bBigEndian )
	{
		BYTE* pOut = pBuffer->m_pBuffer + pBuffer->m_nLength;
		pBuffer->m_nLength += nLenLen;

		if ( nLenLen >= 3 ) *pOut++ = (BYTE)( ( m_nLength >> 16 ) & 0xFF );
		if ( nLenLen >= 2 ) *pOut++ = (BYTE)( ( m_nLength >> 8 ) & 0xFF );
		*pOut++ = (BYTE)( m_nLength & 0xFF );
	}
	else
	{
		pBuffer->Add( &m_nLength, nLenLen );
	}

	pBuffer->Add( &m_nType, nTypeLen + 1 );

	pBuffer->Add( m_pBuffer, m_nLength );
}

//////////////////////////////////////////////////////////////////////
// CG2Packet buffer stream read

CG2Packet* CG2Packet::ReadBuffer(CBuffer* pBuffer)
{
	if ( pBuffer == NULL ) return NULL;

	if ( pBuffer->m_nLength == 0 ) return NULL;
	BYTE nInput = *(pBuffer->m_pBuffer);

	if ( nInput == 0 )
	{
		pBuffer->Remove( 1 );
		return NULL;
	}

	BYTE nLenLen	= ( nInput & 0xC0 ) >> 6;
	BYTE nTypeLen	= ( nInput & 0x38 ) >> 3;
	BYTE nFlags		= ( nInput & 0x07 );

	if ( (DWORD)pBuffer->m_nLength < (DWORD)nLenLen + nTypeLen + 2 ) return NULL;

	DWORD nLength = 0;

	if ( nFlags & G2_FLAG_BIG_ENDIAN )
	{
		BYTE* pLenIn = pBuffer->m_pBuffer + 1;

		for ( BYTE nIt = nLenLen ; nIt ; nIt-- )
		{
			nLength <<= 8;
			nLength |= *pLenIn++;
		}
	}
	else
	{
		BYTE* pLenIn	= pBuffer->m_pBuffer + 1;
		BYTE* pLenOut	= (BYTE*)&nLength;
		for ( BYTE nLenCnt = nLenLen ; nLenCnt-- ; ) *pLenOut++ = *pLenIn++;
	}

	if ( (DWORD)pBuffer->m_nLength < (DWORD)nLength + nLenLen + nTypeLen + 2 )
		return NULL;

	CG2Packet* pPacket = CG2Packet::New( pBuffer->m_pBuffer );
	pBuffer->Remove( nLength + nLenLen + nTypeLen + 2 );

	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CG2Packet debug

CString CG2Packet::GetType() const
{
	CStringA sType;
	sType.Append( (LPCSTR)&m_nType, G2_TYPE_LEN( m_nType ) );

	return CString(  sType );
}

#ifdef DEBUG_G2

CString CG2Packet::ToASCII() const
{
	if ( ! m_bCompound )
		return CPacket::ToASCII();

	CString sASCII;

	DWORD nOrigPosition = m_nPosition;
	const_cast< CG2Packet* >( this )->m_nPosition = 0;
	try
	{
		sASCII = const_cast< CG2Packet* >( this )->Dump( m_nLength );
	}
	catch ( CException* pException )
	{
		pException->Delete();
	}
	const_cast< CG2Packet* >( this )->m_nPosition = nOrigPosition;

	return sASCII;
}

CString CG2Packet::Dump(DWORD nTotal)
{
	CString sASCII;

	G2_PACKET nType;
	DWORD nLength;
	BOOL bCompound;
	while ( m_nPosition < nTotal && ReadPacket( nType, nLength, &bCompound ) )
	{
		DWORD nOffset = m_nPosition + nLength;

		if ( ! sASCII.IsEmpty() ) sASCII += _T(", ");

		CStringA sType;
		sType.Append( (LPCSTR)&nType, G2_TYPE_LEN( nType ) );
		sASCII += sType;

		if ( nLength )
		{
			CString sTmp;
			sTmp.Format( _T("[%u]="), nLength );
			sASCII += sTmp;

			if ( bCompound )
				sASCII += _T("{ ") + Dump( m_nPosition + nLength ) + _T(" }");
			else if ( nType == G2_PACKET_HUB_STATUS )
			{
				WORD nLeafCount = ReadShortBE();
				WORD nLeafLimit = ReadShortBE();
				sTmp.Format( _T("%u/%u"), nLeafCount, nLeafLimit );
				sASCII += sTmp;
			}
			else if ( nType == G2_PACKET_TIMESTAMP )
			{
				DWORD tSeen = ReadLongBE();
				sTmp.Format( _T("%u"), tSeen );
				sASCII += sTmp;
			}
			else if ( nType == G2_PACKET_QUERY_DONE )
			{
				DWORD nAddress = ReadLongLE();
				WORD nPort = 0;
				if ( nLength >= 6 )
					nPort = ReadShortBE();
				WORD nLeaves = 0;
				if ( nLength >= 8 )
					nLeaves = ReadShortBE();
				sTmp.Format( _T("%hs:%u/%u"), inet_ntoa( *(IN_ADDR*)&nAddress ), nPort, nLeaves );
				sASCII += sTmp;
			}
			else if ( nType == G2_PACKET_QUERY_SEARCH )
			{
				DWORD nAddress = ReadLongLE();
				WORD nPort = ReadShortBE();
				DWORD tSeen = 0;
				if ( nLength >= 10 )
					tSeen = ReadLongBE();
				sTmp.Format( _T("%hs:%u+%u"), inet_ntoa( *(IN_ADDR*)&nAddress ), nPort, tSeen );
				sASCII += sTmp;
			}
			else if ( nType == G2_PACKET_LIBRARY_STATUS )
			{
				DWORD nFileCount = ReadLongBE();
				DWORD nFileVolume = ReadLongBE();
				sTmp.Format( _T("%lu/%lu"), nFileCount, nFileVolume );
				sASCII += sTmp;
			}
			else if ( nType == G2_PACKET_NODE_ADDRESS ||
				nType == G2_PACKET_UDP )
			{
				DWORD nAddress = ReadLongLE();
				WORD nPort = ReadShortBE();
				sTmp.Format( _T("%hs:%u"), inet_ntoa( *(IN_ADDR*)&nAddress ), nPort );
				sASCII += sTmp;
			}
			else
			{
				CStringA sDump;
				char* c = sDump.GetBuffer( nLength );
				for ( DWORD i = 0; i < nLength; ++i )
				{
					c[ i ] = ( m_pBuffer[ m_nPosition + i ] < ' ' ) ? '.' : m_pBuffer[ m_nPosition + i ];
				}
				sDump.ReleaseBuffer( nLength );
				sASCII += _T("\"") + UTF8Decode( (LPCSTR)sDump, nLength ) + _T("\"");
			}
		}

		m_nPosition = nOffset;
	}

	return sASCII;
}

#endif // DEBUG_G2

#ifdef _DEBUG

void CG2Packet::Debug(LPCTSTR pszReason) const
{
	CString strOutput;
	strOutput.Format( L"[G2] %s Type: %s", pszReason, (LPCTSTR)GetType() );
	CPacket::Debug( strOutput );
}

#endif // _DEBUG

//////////////////////////////////////////////////////////////////////
// CDatagrams G2UDP packet handler

BOOL CG2Packet::OnPacket(const SOCKADDR_IN* pHost)
{
	Statistics.Current.Gnutella2.Incoming++;

	SmartDump( pHost, TRUE, FALSE );

	if ( ! Settings.Gnutella2.EnableToday ) return TRUE;

	// Is it neigbour's packet or stranger's packet?
	CNeighbour* pNeighbour = Neighbours.Get( pHost->sin_addr );
//	CG1Neighbour* pNeighbour1 = static_cast< CG1Neighbour* >
//		( ( pNeighbour && pNeighbour->m_nProtocol == PROTOCOL_G1 ) ? pNeighbour : NULL );
	CG2Neighbour* pNeighbour2 = static_cast< CG2Neighbour* >
		( ( pNeighbour && pNeighbour->m_nProtocol == PROTOCOL_G2 ) ? pNeighbour : NULL );

	if ( Network.RoutePacket( this ) ) return TRUE;

	switch( m_nType )
	{
	case G2_PACKET_QUERY:
		return OnQuery( pHost );
	case G2_PACKET_QUERY_KEY_REQ:
		return OnQueryKeyRequest( pHost );
	case G2_PACKET_HIT:
		return OnCommonHit( pHost );
	case G2_PACKET_HIT_WRAP:
		return OnCommonHit( pHost );
	case G2_PACKET_QUERY_WRAP:
		// G2_PACKET_QUERY_WRAP deprecated and ignored
		break;
	case G2_PACKET_QUERY_ACK:
		return OnQueryAck( pHost );
	case G2_PACKET_QUERY_KEY_ANS:
		return OnQueryKeyAnswer( pHost );
	case G2_PACKET_PING:
		// Pass packet handling to neighbour if any
		return pNeighbour2 ? pNeighbour2->OnPing( this, FALSE ) : OnPing( pHost );
	case G2_PACKET_PONG:
		// Pass packet handling to neighbour if any
		return pNeighbour2 ? pNeighbour2->OnPong( this, FALSE ) : OnPong( pHost );
	case G2_PACKET_PUSH:
		return OnPush( pHost );
	case G2_PACKET_CRAWL_REQ:
		return OnCrawlRequest( pHost );
	case G2_PACKET_CRAWL_ANS:
		return OnCrawlAnswer( pHost );
	case G2_PACKET_KHL_ANS:
		return OnKHLA( pHost );
	case G2_PACKET_KHL_REQ:
		return OnKHLR( pHost );
	case G2_PACKET_DISCOVERY:
		return OnDiscovery( pHost );
	case G2_PACKET_KHL:
		return OnKHL( pHost );

#ifdef _DEBUG
	default:
		CString tmp;
		tmp.Format( _T("Unknown packet from %s:%u."),
			(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ),
			htons( pHost->sin_port ) );
		Debug( tmp );
#endif // _DEBUG
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams PING packet handler

BOOL CG2Packet::OnPing(const SOCKADDR_IN* pHost)
{
	Datagrams.Send( pHost, CG2Packet::New( G2_PACKET_PONG ) );
	return TRUE;
}

BOOL CG2Packet::OnPong(const SOCKADDR_IN* pHost)
{
	if ( ! m_bCompound ) return TRUE;

	BOOL bRelayed = FALSE;
	G2_PACKET nType;
	DWORD nLength;

	while ( ReadPacket( nType, nLength ) )
	{
		DWORD nOffset = m_nPosition + nLength;
		if ( nType == G2_PACKET_RELAY ) bRelayed = TRUE;
		m_nPosition = nOffset;
	}

	if ( bRelayed && ! Network.IsConnectedTo( &pHost->sin_addr ) )
		Datagrams.SetStable();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams QUERY packet handler

BOOL CG2Packet::OnQuery(const SOCKADDR_IN* pHost)
{
	Statistics.Current.Gnutella2.Queries++;

	CQuerySearchPtr pSearch = CQuerySearch::FromPacket( this, pHost );
	if ( ! pSearch || pSearch->m_bDropMe ||	// Malformed query
		 ! pSearch->m_bUDP )	// Forbid query with firewalled return address
	{
		if ( ! pSearch || ! pSearch->m_bUDP )
		{
			DEBUG_ONLY( Debug( _T("Malformed Query.") ) );
			theApp.Message( MSG_WARNING, IDS_PROTOCOL_BAD_QUERY, (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );
		}
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	if ( Security.IsDenied( &pSearch->m_pEndpoint.sin_addr ) )
	{
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	if ( ! Network.QueryKeys->Check( pSearch->m_pEndpoint.sin_addr.S_un.S_addr, pSearch->m_nKey ) )
	{
		DWORD nKey = Network.QueryKeys->Create( pSearch->m_pEndpoint.sin_addr.S_un.S_addr );

		CString strNode( inet_ntoa( pSearch->m_pEndpoint.sin_addr ) );
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

		Datagrams.Send( &pSearch->m_pEndpoint, pAnswer, TRUE );

		return TRUE;
	}

	if ( ! Network.QueryRoute->Add( pSearch->m_oGUID, &pSearch->m_pEndpoint ) )
	{
		// Ack without hub list
		Datagrams.Send( &pSearch->m_pEndpoint, Neighbours.CreateQueryWeb( pSearch->m_oGUID, false ) );

		Statistics.Current.Gnutella2.Dropped++;
		return TRUE;
	}

	if ( ! Neighbours.CheckQuery( pSearch ) )
	{
		// Ack without hub list with retry time
		Datagrams.Send( &pSearch->m_pEndpoint, Neighbours.CreateQueryWeb( pSearch->m_oGUID, false, NULL, false ) );

		theApp.Message( MSG_WARNING, IDS_PROTOCOL_EXCESS,
			(LPCTSTR)( CString( inet_ntoa( pHost->sin_addr ) ) + _T(" [UDP]") ),
			(LPCTSTR)CString( inet_ntoa( pSearch->m_pEndpoint.sin_addr ) ) );
		Statistics.Current.Gnutella2.Dropped++;
		return TRUE;
	}

	Neighbours.RouteQuery( pSearch, this, NULL, TRUE );

	Network.OnQuerySearch( new CLocalSearch( pSearch, PROTOCOL_G2 ) );

	// Ack with hub list
	Datagrams.Send( &pSearch->m_pEndpoint, Neighbours.CreateQueryWeb( pSearch->m_oGUID, true ) );

	Statistics.Current.Gnutella2.QueriesProcessed++;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams QUERY ACK packet handler

BOOL CG2Packet::OnQueryAck(const SOCKADDR_IN* pHost)
{
	{
		CQuickLock oLock( HostCache.Gnutella2.m_pSection );

		CHostCacheHostPtr pCache = HostCache.Gnutella2.Add( &pHost->sin_addr, htons( pHost->sin_port ) );
		if ( pCache ) pCache->m_tAck = pCache->m_nFailures = 0;
	}

	Hashes::Guid oGUID;

	if ( SearchManager.OnQueryAck( this, pHost, oGUID ) )
	{
		CNeighbour* pNeighbour = NULL;
		SOCKADDR_IN pEndpoint;

		if ( Network.QueryRoute->Lookup( oGUID, &pNeighbour, &pEndpoint ) )
		{
			// TODO: Add a "FR" from tag

			if ( pNeighbour != NULL && pNeighbour->m_nNodeType == ntLeaf )
			{
				pNeighbour->Send( this, FALSE, FALSE );
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

BOOL CG2Packet::OnCommonHit(const SOCKADDR_IN* pHost)
{
	int nHops = 0;
	CQueryHit* pHits = CQueryHit::FromG2Packet( this, &nHops );

	if ( pHits == NULL )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT, (LPCTSTR)HostToString( pHost ) );
		DEBUG_ONLY( Debug( _T("Malformed Hit") ) );
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	// The sender IP of this hit should match the Node Address contained in the packet
	// If it doesn't we'll drop it.
	if ( pHits->m_pAddress.S_un.S_addr != pHost->sin_addr.S_un.S_addr )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT, (LPCTSTR)HostToString( pHost ) );
		DEBUG_ONLY( Debug( _T("Hit sender IP does not match \"Node Address\"") ) );
		Statistics.Current.Gnutella2.Dropped++;
		pHits->Delete();
		return TRUE;
	}

	if ( Security.IsDenied( &pHits->m_pAddress ) )
	{
		Statistics.Current.Gnutella2.Dropped++;
		pHits->Delete();
		return TRUE;
	}

	if ( ! pHits->m_bBogus )
	{
		Network.NodeRoute->Add( pHits->m_oClientID, pHost );

		// Don't route exceeded hits
		if ( nHops <= (int)Settings.Gnutella1.MaximumTTL &&
			SearchManager.OnQueryHits( pHits ) )
		{
			Network.RouteHits( pHits, this );
		}
	}

	Network.OnQueryHits( pHits );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams QUERY KEY REQUEST packet handler

BOOL CG2Packet::OnQueryKeyRequest(const SOCKADDR_IN* pHost)
{
	if ( ! Neighbours.IsG2Hub() )
	{
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	DWORD nRequestedAddress = pHost->sin_addr.S_un.S_addr;
	WORD nRequestedPort = ntohs( pHost->sin_port );
	DWORD nSendingAddress = pHost->sin_addr.S_un.S_addr;

	if ( m_bCompound )
	{
		G2_PACKET nType;
		DWORD nLength;

		while ( ReadPacket( nType, nLength ) )
		{
			DWORD nOffset = m_nPosition + nLength;

			if ( nType == G2_PACKET_REQUEST_ADDRESS && nLength >= 6 )
			{
				nRequestedAddress	= ReadLongLE();
				nRequestedPort		= ReadShortBE();
			}
			else if ( nType == G2_PACKET_SEND_ADDRESS && nLength >= 4 )
			{
				nSendingAddress		= ReadLongLE();
			}

			m_nPosition = nOffset;
		}
	}

	if ( Network.IsFirewalledAddress( (IN_ADDR*)&nRequestedAddress, TRUE ) ||
		! nRequestedPort )
		return TRUE;

	DWORD nKey = Network.QueryKeys->Create( nRequestedAddress );

	CG2Packet* pAnswer = CG2Packet::New( G2_PACKET_QUERY_KEY_ANS, TRUE );

	pAnswer->WritePacket( G2_PACKET_QUERY_KEY, 4 );
	pAnswer->WriteLongBE( nKey );

	if ( nRequestedAddress != nSendingAddress )
	{
		pAnswer->WritePacket( G2_PACKET_SEND_ADDRESS, 4 );
		pAnswer->WriteLongLE( nSendingAddress );
	}

	Datagrams.Send( (IN_ADDR*)&nRequestedAddress, nRequestedPort, pAnswer, TRUE );

	theApp.Message( MSG_DEBUG, _T("Node %s asked for a query key (0x%08x) for node %s:%i"),
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), nKey,
		(LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&nRequestedAddress ) ), nRequestedPort );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams QUERY KEY ANSWER packet handler

BOOL CG2Packet::OnQueryKeyAnswer(const SOCKADDR_IN* pHost)
{
	if ( ! m_bCompound )
	{
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	DWORD nKey = 0;
	IN_ADDR nAddress = {};

	G2_PACKET nType;
	DWORD nLength;

	while ( ReadPacket( nType, nLength ) )
	{
		DWORD nOffset = m_nPosition + nLength;

		if ( nType == G2_PACKET_QUERY_KEY && nLength >= 4 )
		{
			nKey = ReadLongBE();
		}
		else if ( nType == G2_PACKET_SEND_ADDRESS && nLength >= 4 )
		{
			nAddress.s_addr = ReadLongLE();
		}

		m_nPosition = nOffset;
	}

	theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("Got a query key for %s:%lu: 0x%x"),
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), htons( pHost->sin_port ), nKey );

	{
		CQuickLock oLock( HostCache.Gnutella2.m_pSection );

		CHostCacheHostPtr pCache = HostCache.Gnutella2.Add(
			&pHost->sin_addr, htons( pHost->sin_port ) );
		if ( pCache != NULL ) pCache->SetKey( nKey );
	}

	if ( nAddress.s_addr != 0 && ! Network.IsSelfIP( nAddress ) )
	{
		if ( CNeighbour* pNeighbour = Neighbours.Get( nAddress ) )
		{
			BYTE* pOut = WriteGetPointer( 11, 0 );

			if ( pOut == NULL )
			{
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

			if ( m_bBigEndian )
			{
				*pOut++ = (BYTE)( pHost->sin_port & 0xFF );
				*pOut++ = (BYTE)( pHost->sin_port >> 8 );
			}
			else
			{
				*pOut++ = (BYTE)( pHost->sin_port >> 8 );
				*pOut++ = (BYTE)( pHost->sin_port & 0xFF );
			}

			pNeighbour->Send( this, FALSE, FALSE );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams PUSH packet handler

BOOL CG2Packet::OnPush(const SOCKADDR_IN* pHost)
{
	DWORD nLength = GetRemaining();

	if ( ! SkipCompound( nLength, 6 ) )
	{
		theApp.Message( MSG_ERROR, _T("[G2] Invalid PUSH packet received from %s"),
			(LPCTSTR)inet_ntoa( pHost->sin_addr ) );
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	DWORD nAddress	= ReadLongLE();
	WORD nPort		= ReadShortBE();

	if ( Security.IsDenied( (IN_ADDR*)&nAddress ) ||
		 Network.IsFirewalledAddress( (IN_ADDR*)&nAddress ) )
	{
		Statistics.Current.Gnutella2.Dropped++;
		return TRUE;
	}

	Handshakes.PushTo( (IN_ADDR*)&nAddress, nPort );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDatagrams CRAWL packet handler

BOOL CG2Packet::OnCrawlRequest(const SOCKADDR_IN* pHost)
{
	if ( ! m_bCompound )
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

	while ( ReadPacket( nType, nLength ) )
	{
		DWORD nNext = m_nPosition + nLength;

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

		m_nPosition = nNext;
	}

	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_CRAWL_ANS, TRUE );

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

	Datagrams.Send( pHost, pPacket );

	return TRUE;
}

BOOL CG2Packet::OnCrawlAnswer(const SOCKADDR_IN* pHost)
{
	CrawlSession.OnCrawl( pHost, this );
	return TRUE;
}

BOOL CG2Packet::OnDiscovery(const SOCKADDR_IN* pHost)
{
	if ( CG2Packet* pKHL = CG2Neighbour::CreateKHLPacket() )
	{
		// Add myself
		if ( ! Neighbours.IsG2Leaf() &&
			 ( Neighbours.IsG2Hub() ||
			   Neighbours.IsG2HubCapable( HostCache.Gnutella2.IsEmpty() ) ) &&
			   Network.IsListening() )
		{
			pKHL->WritePacket( G2_PACKET_CACHED_HUB, 18, TRUE );		// 4
			pKHL->WritePacket( G2_PACKET_VENDOR, 4 );					// 3
			pKHL->WriteString( VENDOR_CODE, FALSE );					// 4
			pKHL->WriteByte( 0 );										// 1
			pKHL->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );	// 4
			pKHL->WriteShortBE( htons( Network.m_pHost.sin_port ) );	// 2
			pKHL->WriteLongBE( static_cast< DWORD >( time( NULL ) ) );	// 4
		}

		pKHL->WritePacket( G2_PACKET_YOURIP, 4 );
		pKHL->WriteLongLE( pHost->sin_addr.S_un.S_addr );				// 4

		Datagrams.Send( pHost, pKHL, TRUE, 0, FALSE );
	}

	return TRUE;
}

BOOL CG2Packet::OnKHL(const SOCKADDR_IN* pHost)
{
	return CG2Neighbour::ParseKHLPacket( this, pHost );
}

// KHLA - KHL(Known Hub List) Answer, go over G2 UDP packet more like Gnutella2 version of UDPHC
// Better put cache as security to prevent attack, such as flooding cache with invalid host addresses.
BOOL CG2Packet::OnKHLA(const SOCKADDR_IN* pHost)
{
	if ( ! m_bCompound )
	{
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE; // if it is not Compound packet, it is basically malformed packet
	}

	CDiscoveryService * pService = DiscoveryServices.GetByAddress(
		&(pHost->sin_addr) , ntohs(pHost->sin_port), CDiscoveryService::dsGnutella2UDPKHL );

	if (	pService == NULL &&
		(	Network.IsFirewalledAddress( &pHost->sin_addr, TRUE ) ||
			Network.IsReserved( &pHost->sin_addr ) ||
			Security.IsDenied( &pHost->sin_addr ) )
		)
	{
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	G2_PACKET nType, nInner;
	DWORD nLength, nInnerLength;
	LONG tAdjust = 0;
	BOOL bCompound;
	int nCount = 0;

	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	while ( ReadPacket( nType, nLength, &bCompound ) )
	{
		DWORD nNext = m_nPosition + nLength;

		if ( nType == G2_PACKET_NEIGHBOUR_HUB ||
			nType == G2_PACKET_CACHED_HUB )
		{
			DWORD nAddress = 0, tSeen = tNow;
			WORD nPort = 0;
			CString strVendor;

			if ( bCompound || G2_PACKET_NEIGHBOUR_HUB == nType )
			{
				while ( m_nPosition < nNext && ReadPacket( nInner, nInnerLength ) )
				{
					DWORD nNextX = m_nPosition + nInnerLength;

					if ( nInner == G2_PACKET_NODE_ADDRESS && nInnerLength >= 6 )
					{
						nAddress = ReadLongLE();
						nPort = ReadShortBE();
					}
					else if ( nInner == G2_PACKET_VENDOR && nInnerLength >= 4 )
					{
						strVendor = ReadString( 4 );
					}
					else if ( nInner == G2_PACKET_TIMESTAMP && nInnerLength >= 4 )
					{
						tSeen = ReadLongBE() + tAdjust;
					}

					m_nPosition = nNextX;
				}

				nLength = nNext - m_nPosition;
			}

			if ( nLength >= 6 )
			{
				nAddress = ReadLongLE();
				nPort = ReadShortBE();
				if ( nLength >= 10 )
					tSeen = ReadLongBE() + tAdjust;
			}

			CHostCacheHostPtr pCached = HostCache.Gnutella2.Add(
				(IN_ADDR*)&nAddress, nPort, tSeen, strVendor );
			if ( pCached != NULL )
			{
				nCount++;
			}

		}
		else if ( nType == G2_PACKET_TIMESTAMP && nLength >= 4 )
		{
			tAdjust = (LONG)tNow - (LONG)ReadLongBE();
		}
		else if ( nType == G2_PACKET_YOURIP && nLength >= 4 )
		{
			IN_ADDR pMyAddress;
			pMyAddress.s_addr = ReadLongLE();
			Network.AcquireLocalAddress( pMyAddress );
		}

		m_nPosition = nNext;
	}

	if ( pService != NULL )
	{
		pService->OnSuccess();
		pService->m_nHosts = nCount;
	}

	return TRUE;
}

// KHLR - KHL(Known Hub List) request, go over UDP packet more like UDPHC for G1.
BOOL CG2Packet::OnKHLR(const SOCKADDR_IN* pHost)
{
	if ( Security.IsDenied( &pHost->sin_addr ) ||
		Network.IsFirewalledAddress( &pHost->sin_addr, TRUE ) ||
		Network.IsReserved( &pHost->sin_addr ) )
	{
		Statistics.Current.Gnutella2.Dropped++;
		return FALSE;
	}

	CG2Packet* pKHLA = CG2Packet::New( G2_PACKET_KHL_ANS, TRUE );

	//	DWORD nBase = m_nPosition;

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
			CHostCacheHostPtr pCachedHost = (*i);

			if ( pCachedHost->CanQuote( tNow ) &&
				Neighbours.Get( pCachedHost->m_pAddress ) == NULL &&
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

	pKHLA->WritePacket( G2_PACKET_YOURIP, 4 );
	pKHLA->WriteLongLE( pHost->sin_addr.S_un.S_addr );	// 4

	Datagrams.Send( pHost, pKHLA );

	return TRUE;
}
