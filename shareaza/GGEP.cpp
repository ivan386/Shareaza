//
// GGEP.cpp
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
#include "GGEP.h"
#include "G1Packet.h"
#include "ZLib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CGGEPBlock construction

CGGEPBlock::CGGEPBlock()
{
	m_pFirst = m_pLast = NULL;

	m_pInput = NULL;
	m_nInput = 0;
}

CGGEPBlock::~CGGEPBlock()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock clear

void CGGEPBlock::Clear()
{
	while ( m_pFirst )
	{
		CGGEPItem* pNext = m_pFirst->m_pNext;
		delete m_pFirst;
		m_pFirst = pNext;
	}

	m_pLast = NULL;
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock add

CGGEPItem* CGGEPBlock::Add(LPCTSTR pszID)
{
	if ( ! pszID || ! *pszID ) return NULL;
	
	CGGEPItem* pItem = new CGGEPItem( pszID );
	
	if ( ! m_pFirst ) m_pFirst = pItem;
	if ( m_pLast ) m_pLast->m_pNext = pItem;
	m_pLast = pItem;
	
	return pItem;
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock find

CGGEPItem* CGGEPBlock::Find(LPCTSTR pszID, DWORD nMinLength)
{
	if ( ! pszID || ! *pszID ) return NULL;

	for ( CGGEPItem* pItem = m_pFirst ; pItem ; pItem = pItem->m_pNext )
	{
		if ( pItem->m_sID.CompareNoCase( pszID ) == 0 && pItem->m_nLength >= nMinLength )
			return pItem;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock from packet

CGGEPBlock* CGGEPBlock::FromPacket(CPacket* pPacket)
{
	CGGEPBlock* pBlock = new CGGEPBlock();
	if ( pBlock->ReadFromPacket( pPacket ) ) return pBlock;
	delete pBlock;
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock read helpers

BOOL CGGEPBlock::ReadFromPacket(CPacket* pPacket)
{
	m_pInput = pPacket->m_pBuffer + pPacket->m_nPosition;
	m_nInput = pPacket->m_nLength - pPacket->m_nPosition;
	
	BOOL bSuccess = ReadInternal();
	
	pPacket->m_nPosition = pPacket->m_nLength - m_nInput;
	
	return bSuccess;
}

BOOL CGGEPBlock::ReadFromString(LPCTSTR pszData)
{
	m_pInput = (BYTE*)pszData;
	m_nInput = _tcslen( pszData );
	
	return ReadInternal();
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock read internal

BOOL CGGEPBlock::ReadInternal()
{
	while ( m_nInput )
	{
		if ( ReadByte() == GGEP_MAGIC ) break;
	}
	
	if ( ! m_nInput ) return FALSE;
	
	while ( m_nInput >= 3 )
	{
		BYTE nFlags = ReadByte();
		if ( ! ( nFlags & GGEP_HDR_IDLEN ) ) return FALSE;
		
		CGGEPItem* pItem = new CGGEPItem();
		
		if ( pItem->ReadFrom( this, nFlags ) )
		{
			if ( ! m_pFirst ) m_pFirst = pItem;
			if ( m_pLast ) m_pLast->m_pNext = pItem;
			m_pLast = pItem;
		}
		else
		{
			delete pItem;
			return FALSE;
		}
		
		if ( nFlags & GGEP_HDR_LAST ) return TRUE;
	}
	
	return FALSE;
}

BYTE CGGEPBlock::ReadByte()
{
	if ( m_nInput < 1 ) AfxThrowUserException();
	m_nInput--;
	return *m_pInput++;
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock write helpers

void CGGEPBlock::Write(CPacket* pPacket)
{
	if ( ! m_pFirst ) return;
	
	pPacket->WriteByte( GGEP_MAGIC );
	
	for ( CGGEPItem* pItem = m_pFirst ; pItem ; pItem = pItem->m_pNext )
	{
		pItem->WriteTo( pPacket );
	}
	
	Clear();
}

void CGGEPBlock::Write(CString& str)
{
	if ( ! m_pFirst ) return;
	
	str += (TCHAR)GGEP_MAGIC;
	
	for ( CGGEPItem* pItem = m_pFirst ; pItem ; pItem = pItem->m_pNext )
	{
		pItem->WriteTo( str );
	}
	
	Clear();
}


//////////////////////////////////////////////////////////////////////
// CGGEPItem construction

CGGEPItem::CGGEPItem(LPCTSTR pszID)
{
	m_pNext		= NULL;
	m_pBuffer	= NULL;
	m_nLength	= 0;
	m_nPosition	= 0;
	
	if ( pszID ) m_sID = pszID;
}

CGGEPItem::~CGGEPItem()
{
	if ( m_pBuffer ) delete [] m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem ID check

BOOL CGGEPItem::IsNamed(LPCTSTR pszID)
{
	return m_sID == pszID;
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem reading and writing

void CGGEPItem::Read(LPVOID pData, int nLength)
{
	if ( m_nPosition + (DWORD)nLength >= m_nLength ) AfxThrowUserException();
	CopyMemory( pData, m_pBuffer + m_nPosition, nLength );
	m_nPosition += nLength;
}

BYTE CGGEPItem::ReadByte()
{
	BYTE nByte;
	Read( &nByte, 1 );
	return nByte;
}

void CGGEPItem::Write(LPCVOID pData, int nLength)
{
	BYTE* pNew = new BYTE[ m_nLength + (DWORD)nLength ];
	
	if ( m_pBuffer )
	{
		CopyMemory( pNew, m_pBuffer, m_nLength );
		delete [] m_pBuffer;
	}
	
	m_pBuffer = pNew;
	CopyMemory( m_pBuffer + m_nLength, pData, nLength );
	m_nLength += nLength;
}

void CGGEPItem::WriteByte(BYTE nValue)
{
	Write( &nValue, 1 );
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem string conversion

CString CGGEPItem::ToString()
{
	CString strValue;
	
	LPTSTR pszOut = strValue.GetBuffer( m_nLength );
	LPCSTR pszIn  = (LPCSTR)m_pBuffer;
	
	for ( DWORD nChar = 0 ; nChar < m_nLength ; nChar++ )
		*pszOut++ = (TCHAR)*pszIn++;
	
	strValue.ReleaseBuffer( m_nLength );
	return strValue;
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem read

BOOL CGGEPItem::ReadFrom(CGGEPBlock* pBlock, BYTE nFlags)
{
	BYTE nLen = ( nFlags & GGEP_HDR_IDLEN );
	
	if ( pBlock->m_nInput <= nLen ) return FALSE;
	
	LPTSTR pszID = m_sID.GetBuffer( nLen );
	for ( BYTE i = nLen ; i && pBlock->m_nInput ; i-- ) *pszID++ = pBlock->ReadByte();
	m_sID.ReleaseBuffer( nLen );
	
	m_nLength = 0;
	
	for ( i = 0 ; i < 3 ; i++ )
	{
		if ( ! pBlock->m_nInput ) return FALSE;
		nLen = pBlock->ReadByte();
		
		m_nLength = ( m_nLength << 6 ) | ( nLen & GGEP_LEN_MASK );
		
		if ( nLen & GGEP_LEN_LAST ) break;
		if ( ! pBlock->m_nInput || ! ( nLen & GGEP_LEN_MORE ) ) return FALSE;
	}
	
	if ( nLen & GGEP_LEN_MORE ) return FALSE;
	if ( ! ( nLen & GGEP_LEN_LAST ) ) return FALSE;
	
	if ( ! m_nLength ) return TRUE;
	
	if ( pBlock->m_nInput < m_nLength ) return FALSE;

	m_pBuffer = new BYTE[ m_nLength ];
	
	CopyMemory( m_pBuffer, pBlock->m_pInput, m_nLength );
	pBlock->m_pInput += m_nLength;
	pBlock->m_nInput -= m_nLength;

	if ( ( nFlags & GGEP_HDR_COBS ) && ! Decode() )
	{
		delete [] m_pBuffer;
		m_pBuffer = NULL;
		return FALSE;
	}
	
	if ( ( nFlags & GGEP_HDR_DEFLATE ) && ! Inflate() )
	{
		delete [] m_pBuffer;
		m_pBuffer = NULL;
		return FALSE;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem write to packet

void CGGEPItem::WriteTo(CPacket* pPacket)
{
	BYTE nFlags = ( m_sID.GetLength() & GGEP_HDR_IDLEN );
	
	if ( Deflate( TRUE ) ) nFlags |= GGEP_HDR_DEFLATE;
	if ( Encode( TRUE ) ) nFlags |= GGEP_HDR_COBS;
	
	if ( m_pNext == NULL ) nFlags |= GGEP_HDR_LAST;
	
	pPacket->WriteByte( nFlags );
	
	for ( BYTE i = 0 ; i < m_sID.GetLength() ; i++ )
		pPacket->WriteByte( (BYTE)m_sID.GetAt( i ) );
	
	if ( m_nLength & 0x3F000 )
		pPacket->WriteByte( (BYTE)( ( ( m_nLength >> 12 ) & GGEP_LEN_MASK ) | GGEP_LEN_MORE ) );
	
	if ( m_nLength & 0xFC0 )
		pPacket->WriteByte( (BYTE)( ( ( m_nLength >> 6 ) & GGEP_LEN_MASK ) | GGEP_LEN_MORE ) );
	
	if ( m_nLength & 0x3F )
		pPacket->WriteByte( (BYTE)( ( m_nLength & GGEP_LEN_MASK ) | GGEP_LEN_LAST ) );
	
	if ( m_pBuffer && m_nLength )
		pPacket->Write( m_pBuffer, m_nLength );
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem write to string

void CGGEPItem::WriteTo(CString& str)
{
	BYTE nFlags = ( m_sID.GetLength() & GGEP_HDR_IDLEN );
	
	if ( Deflate( TRUE ) ) nFlags |= GGEP_HDR_DEFLATE;
	if ( Encode() ) nFlags |= GGEP_HDR_COBS;
	
	if ( m_pNext == NULL ) nFlags |= GGEP_HDR_LAST;
	
	str += (TCHAR)nFlags;
	str += m_sID;
	
	if ( m_nLength & 0x3F000 )
		str += (TCHAR)( ( ( m_nLength >> 12 ) & GGEP_LEN_MASK ) | GGEP_LEN_MORE );
	
	if ( m_nLength & 0xFC0 )
		str += (TCHAR)( ( ( m_nLength >> 6 ) & GGEP_LEN_MASK ) | GGEP_LEN_MORE );
	
	if ( m_nLength & 0x3F )
		str += (TCHAR)( ( m_nLength & GGEP_LEN_MASK ) | GGEP_LEN_LAST );
	
	if ( m_pBuffer && m_nLength )
	{
		for ( DWORD nLen = 0 ; nLen < m_nLength ; nLen++ )
			str += (TCHAR)m_pBuffer[ nLen ];
	}
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem COBS encoding

BOOL CGGEPItem::Encode(BOOL bIfZeros)
{
	if ( ! m_pBuffer ) return FALSE;
	
	DWORD nLength;
	BYTE* pIn;
	
	if ( bIfZeros )
	{
		for ( pIn = m_pBuffer, nLength = m_nLength ; nLength > 0 ; nLength--, pIn++ )
		{
			if ( *pIn == 0 ) break;
		}
		if ( ! nLength ) return FALSE;
	}
	
	BYTE* pOutput	= new BYTE[ m_nLength * 2 ];
	BYTE* pOut		= pOutput;
	BYTE* pRange	= NULL;
	DWORD nRange	= 0;
	
	for ( pIn = m_pBuffer, nLength = m_nLength ; nLength > 0 ; nLength--, pIn++ )
	{
		if ( *pIn == 0 )
		{
			if ( pRange )
			{
				ASSERT( nRange > 0 && nRange < 254 );
				*pOut++ = (BYTE)( nRange + 1 );
				CopyMemory( pOut, pRange, nRange );
				pOut += nRange;
				pRange = NULL;
				nRange = 0;
			}
			else
			{
				*pOut++ = 1;
			}
		}
		else if ( pRange )
		{
			if ( ++nRange == 254 )
			{
				*pOut++ = 255;
				CopyMemory( pOut, pRange, 254 );
				pOut += 254;
				pRange = NULL;
				nRange = 0;
			}
		}
		else
		{
			pRange = pIn;
			nRange = 1;
		}
	}
	
	if ( pRange )
	{
		ASSERT( nRange > 0 && nRange < 254 );
		*pOut++ = (BYTE)( nRange + 1 );
		CopyMemory( pOut, pRange, nRange );
		pOut += nRange;
		pRange = NULL;
		nRange = 0;
	}
	
	delete [] m_pBuffer;
	
	m_pBuffer = pOutput;
	m_nLength = pOut - pOutput;
	
	return TRUE;
}

BOOL CGGEPItem::Decode()
{
	if ( ! m_pBuffer ) return FALSE;
	
	BYTE* pOutput	= new BYTE[ m_nLength * 2 ];
	BYTE* pOut		= pOutput;
	BYTE* pIn		= m_pBuffer;
	
	for ( DWORD nLength = m_nLength ; nLength > 0 ; nLength--, pIn++ )
	{
		if ( *pIn == 0 )
		{
			break;
		}
		else if ( *pIn == 1 )
		{
			*pOut++ = 0;
			continue;
		}
		
		BOOL bZero = ( *pIn != 255 );
		BYTE nSize = *pIn++ - 1;
		nLength--;
		
		nSize = (BYTE)min( (DWORD)nSize, nLength );
		
		while ( nSize-- )
		{
			*pOut++ = *pIn++;
			nLength--;
		}
		
		if ( bZero && nLength ) *pOut++ = 0;
		
		pIn--;
		nLength++;
	}
	
	delete [] m_pBuffer;
	
	m_pBuffer = pOutput;
	m_nLength = pOut - pOutput;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem compression

BOOL CGGEPItem::Deflate(BOOL bIfSmaller)
{
	if ( ! m_pBuffer ) return FALSE;
	if ( bIfSmaller && m_nLength < 45 ) return FALSE;

	DWORD nCompressed = 0;
	BYTE* pCompressed = CZLib::Compress( m_pBuffer, m_nLength, &nCompressed );

	if ( ! pCompressed ) return FALSE;

	if ( bIfSmaller && nCompressed >= m_nLength )
	{
		delete [] pCompressed;
		return FALSE;
	}

	delete [] m_pBuffer;
	m_pBuffer = pCompressed;
	m_nLength = nCompressed;

	return TRUE;
}

BOOL CGGEPItem::Inflate()
{
	if ( ! m_pBuffer ) return FALSE;
	
	DWORD nCompressed = 0;
	BYTE* pCompressed = CZLib::Decompress( m_pBuffer, m_nLength, &nCompressed );
	
	if ( ! pCompressed ) return FALSE;
	
	delete [] m_pBuffer;
	m_pBuffer = pCompressed;
	m_nLength = nCompressed;
	
	return TRUE;
}
