//
// G2Packet.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

CG2Packet::CG2Packet() : CPacket( PROTOCOL_G2 )
{
	m_sType[0]		= 0;
	m_bCompound		= FALSE;
	m_bBigEndian	= FALSE;
}

CG2Packet::~CG2Packet()
{
}

//////////////////////////////////////////////////////////////////////
// CG2Packet reset

void CG2Packet::Reset()
{
	CPacket::Reset();

	m_sType[0]		= 0;
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
    LPSTR pszType = pPacket->m_sType;
	for ( ; nTypeLen-- ;  ) *pszType++ = (CHAR)*pSource++;
	*pszType++ = 0;

	pPacket->Write( pSource, nLength );

	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CG2Packet construct wrapping G1 packet

CG2Packet* CG2Packet::New(LPCSTR pszType, CG1Packet* pWrap, int nMinTTL)
{
	CG2Packet* pPacket = New( pszType, FALSE );

	GNUTELLAPACKET pHeader;

	pHeader.m_pGUID		= pWrap->m_pGUID;
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
	CG2Packet* pPacket = CG2Packet::New( m_sType, m_bCompound );
	pPacket->Write( m_pBuffer, m_nLength );
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CG2Packet sub-packet write

void CG2Packet::WritePacket(CG2Packet* pPacket)
{
	if ( pPacket == NULL ) return;
	WritePacket( pPacket->m_sType, pPacket->m_nLength, pPacket->m_bCompound );
	Write( pPacket->m_pBuffer, pPacket->m_nLength );
}

//////////////////////////////////////////////////////////////////////
// CG2Packet sub-packet write

void CG2Packet::WritePacket(LPCSTR pszType, DWORD nLength, BOOL bCompound)
{
	ASSERT( strlen( pszType ) > 0 );
	ASSERT( nLength <= 0xFFFFFF );

	BYTE nTypeLen	= (BYTE)( strlen( pszType ) - 1 ) & 0x07;
	BYTE nLenLen	= 1;

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
		if ( nLenLen >= 3 ) WriteByte( (BYTE)( ( nLength >> 16 ) & 0xFF ) );
		if ( nLenLen >= 2 ) WriteByte( (BYTE)( ( nLength >> 8 ) & 0xFF ) );
		WriteByte( (BYTE)( nLength & 0xFF ) );
	}
	else
	{
		Write( &nLength, nLenLen );
	}

	Write( pszType, nTypeLen + 1 );

	m_bCompound = TRUE;	// This must be compound now
}

//////////////////////////////////////////////////////////////////////
// CG2Packet sub-packet read

BOOL CG2Packet::ReadPacket(LPSTR pszType, DWORD& nLength, BOOL* pbCompound)
{
	if ( GetRemaining() == 0 ) return FALSE;

	BYTE nInput = ReadByte();
	if ( nInput == 0 ) return FALSE;

	BYTE nLenLen	= ( nInput & 0xC0 ) >> 6;
	BYTE nTypeLen	= ( nInput & 0x38 ) >> 3;
	BYTE nFlags		= ( nInput & 0x07 );

	if ( GetRemaining() < nTypeLen + nLenLen + 1 ) AfxThrowUserException();

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

	if ( GetRemaining() < (int)nLength + nTypeLen + 1 ) AfxThrowUserException();

	Read( pszType, nTypeLen + 1 );
	pszType[ nTypeLen + 1 ] = 0;

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
		BYTE nFlags		= ( nInput & 0x07 );

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

BOOL CG2Packet::GetTo(GGUID* pGUID)
{
	if ( m_bCompound == FALSE ) return FALSE;
	if ( GetRemaining() < 4 + 16 ) return FALSE;

	BYTE* pTest = m_pBuffer + m_nPosition;

	if ( pTest[0] != 0x48 ) return FALSE;
	if ( pTest[1] != 0x10 ) return FALSE;
	if ( pTest[2] != 'T' ) return FALSE;
	if ( pTest[3] != 'O' ) return FALSE;

	CopyMemory( pGUID, pTest + 4, 16 );

	return TRUE;
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
	BOOL bEncoded	= FALSE;

    DWORD nLength = 0;
	for ( ; nLength < nMaximum ; nLength++ )
	{
		m_nPosition++;
		if ( ! *pszScan ) break;
		pszScan ++;
	}

	int nWide = MultiByteToWideChar( CP_UTF8, 0, pszInput, nLength, NULL, 0 );
	MultiByteToWideChar( CP_UTF8, 0, pszInput, nLength, strString.GetBuffer( nWide ), nWide );
	strString.ReleaseBuffer( nWide );

	return strString;
}

void CG2Packet::WriteString(LPCTSTR pszString, BOOL bNull)
{
	if ( *pszString == NULL )
	{
		if ( bNull ) WriteByte( 0 );
		return;
	}

	int nWide		= _tcslen(pszString);
	int nByte		= WideCharToMultiByte( CP_UTF8, 0, pszString, nWide, NULL, 0, NULL, NULL );
	LPSTR pszByte	= ( nByte <= PACKET_BUF_SCHAR ) ? m_szSCHAR : new CHAR[ nByte + 1 ];

	WideCharToMultiByte( CP_UTF8, 0, pszString, nWide, pszByte, nByte, NULL, NULL );

	if ( bNull )
	{
		pszByte[ nByte ] = 0;
		Write( pszByte, nByte + 1 );
	}
	else
		Write( pszByte, nByte );

	if ( pszByte != m_szSCHAR ) delete [] pszByte;
}

void CG2Packet::WriteString(LPCSTR pszString, BOOL bNull)
{
	if ( *pszString == NULL )
	{
		if ( bNull ) WriteByte( 0 );
		return;
	}

	Write( pszString, strlen(pszString) + ( bNull ? 1 : 0 ) );
}

int CG2Packet::GetStringLen(LPCTSTR pszString) const
{
	if ( *pszString == 0 ) return 0;

	LPCTSTR pszScan = pszString;
	BOOL bPlain = TRUE;

    int nLength = 0;
	for ( ; *pszScan ; nLength++ )
	{
		if ( ( *pszScan++ ) & 0x80 ) bPlain = FALSE;
	}

	nLength = WideCharToMultiByte( CP_UTF8, 0, pszString, nLength, NULL, 0, NULL, NULL );

	return nLength;
}

//////////////////////////////////////////////////////////////////////
// CG2Packet to buffer

void CG2Packet::ToBuffer(CBuffer* pBuffer) const
{
	ASSERT( strlen( m_sType ) > 0 );

	BYTE nLenLen	= 1;
	BYTE nTypeLen	= (BYTE)( strlen( m_sType ) - 1 ) & 0x07;

	if ( m_nLength > 0xFF )
	{
		nLenLen++;
		if ( m_nLength > 0xFFFF ) nLenLen++;
	}

	BYTE nFlags = ( nLenLen << 6 ) + ( nTypeLen << 3 );

	if ( m_bCompound ) nFlags |= G2_FLAG_COMPOUND;
	if ( m_bBigEndian ) nFlags |= G2_FLAG_BIG_ENDIAN;

	pBuffer->Add( &nFlags, 1 );

	if ( m_bBigEndian )
	{
		pBuffer->EnsureBuffer( nLenLen );
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

	pBuffer->Add( m_sType, nTypeLen + 1 );

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

void CG2Packet::Debug(LPCTSTR pszReason) const
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
		if ( ! pFile.Open( _T("\\Shareaza.log"), CFile::modeWrite|CFile::modeCreate ) ) return;
	}

	strOutput.Format( _T("[G2]: '%s' %s %s\r\n\r\n"), pszReason,
		GetType(), (LPCTSTR)ToASCII() );

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

