//
// G2Packet.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "G2Packet.h"
#include "G1Packet.h"
#include "Buffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CG2Packet::CG2PacketPool CG2Packet::POOL;


//////////////////////////////////////////////////////////////////////
// CG2Packet construction

CG2Packet::CG2Packet() :
	CPacket( PROTOCOL_G2 ),
	m_nType( G2_PACKET_NULL ),
	m_bCompound( FALSE )
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

	return WideCharToMultiByte( CP_UTF8, 0, pszString, -1, NULL, 0, NULL, NULL );
}

//////////////////////////////////////////////////////////////////////
// CG2Packet to buffer

void CG2Packet::ToBuffer(CBuffer* pBuffer) const
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
	CStringA tmp;
	tmp.Append( (LPCSTR)&m_nType, G2_TYPE_LEN( m_nType ) );
	return CString( tmp );
}

void CG2Packet::Debug(LPCTSTR pszReason) const
{
#ifdef _DEBUG
	CString strOutput;
	strOutput.Format( L"[G2] %s Type: %s", pszReason, GetType() );
	CPacket::Debug( strOutput );
#else
	pszReason;
#endif
}
