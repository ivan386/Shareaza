//
// G1Packet.cpp
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
#include "G1Packet.h"
#include "Network.h"
#include "Buffer.h"
#include "SHA.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CG1Packet::CG1PacketPool CG1Packet::POOL;


//////////////////////////////////////////////////////////////////////
// CG1Packet construction

CG1Packet::CG1Packet() : CPacket( PROTOCOL_G1 )
{
	m_nType = 0;
	m_nTypeIndex = 0;
	
	m_nTTL = m_nHops = 0;
	m_nHash = 0;
}

CG1Packet::~CG1Packet()
{
}

//////////////////////////////////////////////////////////////////////
// CG1Packet new

CG1Packet* CG1Packet::New(int nType, DWORD nTTL, CGUID* pGUID)
{
	CG1Packet* pPacket = (CG1Packet*)POOL.New();
	
	pPacket->m_nType		= (BYTE)nType;
	pPacket->m_nTypeIndex	= GnutellaTypeToIndex( pPacket->m_nType );
	
	pPacket->m_nTTL		= (BYTE)( nTTL > 0 ? nTTL : Settings.Gnutella1.DefaultTTL );
	pPacket->m_nHops	= 0;
	pPacket->m_nHash	= 0;
		
	if ( pGUID )
	{
		pPacket->m_pGUID = *pGUID;
	}
	else
	{
		Network.CreateID( &pPacket->m_pGUID );
	}
	
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CG1Packet type conversion

int CG1Packet::GnutellaTypeToIndex(BYTE nType)
{
	switch ( nType )
	{
	case G1_PACKET_PING:
		return G1_PACKTYPE_PING;
	case G1_PACKET_PONG:
		return G1_PACKTYPE_PONG;
	case G1_PACKET_BYE:
		return G1_PACKTYPE_BYE;
	case G1_PACKET_QUERY_ROUTE:
		return G1_PACKTYPE_QUERY_ROUTE;
	case G1_PACKET_VENDOR:
	case G1_PACKET_VENDOR_APP:
		return G1_PACKTYPE_VENDOR;
	case G1_PACKET_PUSH:
		return G1_PACKTYPE_PUSH;
	case G1_PACKET_QUERY:
		return G1_PACKTYPE_QUERY;
	case G1_PACKET_HIT:
		return G1_PACKTYPE_HIT;
	default:
		return G1_PACKTYPE_UNKNOWN;
	}
}

//////////////////////////////////////////////////////////////////////
// CG1Packet hopping

BOOL CG1Packet::Hop()
{
	if ( m_nTTL < 2 ) return FALSE;

	m_nTTL--;
	m_nHops++;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG1Packet hashing

void CG1Packet::CacheHash()
{
	BYTE* pInput = m_pBuffer;
	m_nHash = 0;

	for ( DWORD nPosition = m_nLength ; nPosition ; nPosition-- )
	{
		m_nHash = ( m_nHash << 8 ) | ( ( m_nHash >> 24 ) ^ *pInput++ );
	}

	m_nHash |= 1;
}

BOOL CG1Packet::GetRazaHash(CHashSHA1 &oHash, DWORD nLength) const
{
	if ( nLength == 0xFFFFFFFF ) nLength = m_nLength;
	if ( (DWORD)m_nLength < nLength ) return FALSE;

	CSHA1 oSHA;

	oSHA.Add( &m_pGUID.m_b, GUID_SIZE );
	oSHA.Add( &m_nType, sizeof(m_nType) );
	oSHA.Add( m_pBuffer, nLength );
	oSHA.Finish();
	oHash = oSHA;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG1Packet string conversion

LPCTSTR CG1Packet::m_pszPackets[ G1_PACKTYPE_MAX ] =
{
	_T("Unknown"), _T("Ping"), _T("Pong"), _T("Bye"), _T("QRP"), _T("Vendor"), _T("Push"), _T("Query"), _T("Hit")
};

LPCTSTR CG1Packet::GetType() const
{
	return m_pszPackets[ m_nTypeIndex ];
}

CString CG1Packet::GetGUID() const
{
	CString strOut;
	strOut.Format( _T("%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X"),
		m_pGUID.m_b[0],  m_pGUID.m_b[1],  m_pGUID.m_b[2],  m_pGUID.m_b[3],
		m_pGUID.m_b[4],  m_pGUID.m_b[5],  m_pGUID.m_b[6],  m_pGUID.m_b[7],
		m_pGUID.m_b[8],  m_pGUID.m_b[9],  m_pGUID.m_b[10], m_pGUID.m_b[11],
		m_pGUID.m_b[12], m_pGUID.m_b[13], m_pGUID.m_b[14], m_pGUID.m_b[15] );
	return strOut;
}

//////////////////////////////////////////////////////////////////////
// CG1Packet buffer output

void CG1Packet::ToBuffer(CBuffer* pBuffer) const
{
	GNUTELLAPACKET pHeader;

	pHeader.m_pGUID		= m_pGUID;
	pHeader.m_nType		= m_nType;
	pHeader.m_nTTL		= m_nTTL;
	pHeader.m_nHops		= m_nHops;
	pHeader.m_nLength	= (LONG)m_nLength;

	pBuffer->Add( &pHeader, sizeof(pHeader) );
	pBuffer->Add( m_pBuffer, m_nLength );
}

//////////////////////////////////////////////////////////////////////
// CG1Packet debug

void CG1Packet::Debug(LPCTSTR pszReason) const
{
#ifdef _DEBUG
	if ( ! Settings.General.Debug ) return;

	CString strOutput;
	CFile pFile;

	if ( pFile.Open( _T("\\Shareaza.log"), CFile::modeReadWrite ) )
	{
		pFile.Seek( 0, CFile::end );
	}
	else
	{
		if ( ! pFile.Open( _T("\\Shareaza.log"), CFile::modeWrite|CFile::modeCreate ) )
			return;
	}

	strOutput.Format( _T("%s: %s [%i/%i] %s\r\n\r\n"), pszReason,
		GetType(), m_nTTL, m_nHops, (LPCTSTR)ToASCII() );
	
	USES_CONVERSION;
	LPCSTR pszOutput = T2CA( (LPCTSTR)strOutput );
	pFile.Write( pszOutput, strlen(pszOutput) );
	
	for ( DWORD i = 0 ; i < m_nLength ; i++ )
	{
		int nChar = m_pBuffer[i];
		strOutput.Format( _T("%.2X(%c) "), nChar, ( nChar >= 32 ? nChar : '.' ) );
		pszOutput = T2CA( (LPCTSTR)strOutput );
		pFile.Write( pszOutput, strlen(pszOutput) );
	}

	pFile.Write( "\r\n\r\n", 4 );

	pFile.Close();
#endif
}

