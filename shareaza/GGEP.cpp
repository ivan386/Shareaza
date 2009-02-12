//
// GGEP.cpp
//
// Copyright © Shareaza Development Team, 2002-2009.
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
#include "Buffer.h"
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

CGGEPBlock::CGGEPBlock() :
	m_pFirst	( NULL ),
	m_pLast		( NULL ),
	m_pInput	( NULL ),
	m_nInput	( 0 ),
	m_nItemCount( 0 )
{
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
		m_nItemCount--;
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

	if ( pItem == NULL )
	{
		theApp.Message( MSG_ERROR, _T("Memory allocation error in CGGEPBlock::Add()") );
		return NULL;
	}

	if ( ! m_pFirst ) m_pFirst = pItem;
	if ( m_pLast ) m_pLast->m_pNext = pItem;
	m_pLast = pItem;
	m_nItemCount++;
	return pItem;
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock find

CGGEPItem* CGGEPBlock::Find(LPCTSTR pszID, DWORD nMinLength) const
{
	if ( ! pszID || ! *pszID ) return NULL;

	for ( CGGEPItem* pItem = m_pFirst ; pItem ; pItem = pItem->m_pNext )
	{
		// think GGEP is Case sensitive so the original code is wrong
		//if ( pItem->m_sID.CompareNoCase( pszID ) == 0 && pItem->m_nLength >= nMinLength )
		//	return pItem;
		if ( pItem->m_sID.Compare( pszID ) == 0 && pItem->m_nLength >= nMinLength )
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
	m_nInput = static_cast< DWORD >( _tcslen( pszData ) );

	return ReadInternal();
}

BOOL CGGEPBlock::ReadFromBuffer(LPVOID pszData, DWORD nLength)
{
	m_pInput = (BYTE*)pszData;
	m_nInput = nLength;

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
			m_nItemCount++;
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
	BYTE result = *m_pInput;
	m_pInput+= 1;
	return result;
}

//////////////////////////////////////////////////////////////////////
// CGGEPBlock write helpers

void CGGEPBlock::Write(CPacket* pPacket)
{
	if ( ! m_pFirst ) return;

	pPacket->WriteByte( GGEP_MAGIC );

	for ( CGGEPItem* pItem = m_pFirst ; pItem ; pItem = pItem->m_pNext )
	{
		pItem->WriteTo( pPacket, pItem->m_bSmall, pItem->m_bCOBS );
	}

	Clear();
}

void CGGEPBlock::Write(CString& str)
{
	if ( ! m_pFirst ) return;

	str.Insert( 0, (TCHAR)GGEP_MAGIC );

	for ( CGGEPItem* pItem = m_pFirst ; pItem ; pItem = pItem->m_pNext )
	{
		pItem->WriteTo( str, pItem->m_bSmall, pItem->m_bCOBS );
	}

	Clear();
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem construction

CGGEPItem::CGGEPItem(LPCTSTR pszID) :
	m_pNext		( NULL ),
	m_sID		( pszID ? pszID : _T("") ),
	m_pBuffer	( NULL ),
	m_nLength	( 0 ),
	m_nPosition	( 0 ),
	m_bCOBS		( false ),
	m_bSmall	( false )
{
}

CGGEPItem::~CGGEPItem()
{
	if ( m_pBuffer ) delete [] m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem ID check

BOOL CGGEPItem::IsNamed(LPCTSTR pszID) const
{
	return m_sID == pszID;
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem reading and writing

void CGGEPItem::Read(LPVOID pData, int nLength)
{
	if ( m_nPosition + (DWORD)nLength > m_nLength )
		nLength = m_nLength;
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

	if ( pNew == NULL )
	{
		theApp.Message( MSG_ERROR, _T("Memory allocation error in CGGEPItem::Write()") );
		theApp.Message( MSG_DEBUG, _T("Requested length: %i"), nLength );
		return;
	}

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

void CGGEPItem::WriteUTF8(LPCWSTR pszText, size_t nLength)
{
	CBuffer pBuffer;

	pBuffer.Print( pszText, nLength, CP_UTF8 );
	Write( pBuffer.m_pBuffer, pBuffer.m_nLength );
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem string conversion

CString CGGEPItem::ToString() const
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

	if ( pBlock->m_nInput <= nLen || ( nFlags & 0x10 ) != 0 ) return FALSE;

	LPTSTR pszID = m_sID.GetBuffer( nLen );
	for ( BYTE i = nLen ; i && pBlock->m_nInput ; i-- ) *pszID++ = pBlock->ReadByte();
	m_sID.ReleaseBuffer( nLen );

	m_nLength = 0;

	for ( BYTE i = 0 ; i < 3 ; i++ )
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

	m_pBuffer = new BYTE[ m_nLength + 1 ];
	if ( m_pBuffer == NULL )
	{
		theApp.Message( MSG_ERROR, _T("Memory allocation error in CGGEPItem::ReadFrom()") );
		theApp.Message( MSG_DEBUG, _T("Requested length: %i"), m_nLength );
		return FALSE;
	}
	CopyMemory( m_pBuffer, pBlock->m_pInput, m_nLength );
	m_pBuffer[ m_nLength ] = 0;
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

void CGGEPItem::WriteTo(CPacket* pPacket, bool bSmall, bool bNeedCOBS)
{
	if (  m_sID.IsEmpty() ) return;

	// Create GGEP Extension Header
	BYTE nFlags = BYTE( m_sID.GetLength() & GGEP_HDR_IDLEN );

	if ( bSmall && Deflate( bSmall ) ) nFlags |= GGEP_HDR_DEFLATE;
	if ( bNeedCOBS && Encode( bNeedCOBS ) ) nFlags |= GGEP_HDR_COBS;

	if ( m_pNext == NULL ) nFlags |= GGEP_HDR_LAST; // last extension in the block

	// Flags -- 1 byte 
	pPacket->WriteByte( nFlags );

	// ID -- 1-15 byte
	for ( BYTE i = 0 ; i < m_sID.GetLength() ; i++ )
		pPacket->WriteByte( (BYTE)m_sID.GetAt( i ) );

	// Length of the raw extension data -- 1-3 bytes
	if ( m_nLength & 0x3F000 )
		pPacket->WriteByte( (BYTE)( ( ( m_nLength >> 12 ) & GGEP_LEN_MASK ) | GGEP_LEN_MORE ) );

	if ( m_nLength & 0xFC0 )
		pPacket->WriteByte( (BYTE)( ( ( m_nLength >> 6 ) & GGEP_LEN_MASK ) | GGEP_LEN_MORE ) );

	// shut off everything except the last 6 bits
	pPacket->WriteByte( (BYTE)( ( m_nLength & GGEP_LEN_MASK ) | GGEP_LEN_LAST ) );

	if ( m_pBuffer && m_nLength )
		pPacket->Write( m_pBuffer, m_nLength );
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem write to string

void CGGEPItem::WriteTo(CString& str, bool bSmall, bool bNeedCOBS)
{
	BYTE nFlags = BYTE( m_sID.GetLength() & GGEP_HDR_IDLEN );

	if ( bSmall && Deflate( bSmall ) ) nFlags |= GGEP_HDR_DEFLATE;
	if ( bNeedCOBS && Encode() ) nFlags |= GGEP_HDR_COBS;

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

	if ( pOutput == NULL )
	{
		theApp.Message( MSG_ERROR, _T("Memory allocation error in CGGEPItem::Encode()") );
		theApp.Message( MSG_DEBUG, _T("Requested length: %i"), m_nLength * 2 );
		return FALSE;
	}

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
	m_nLength = static_cast< DWORD >( pOut - pOutput );

	return TRUE;
}

BOOL CGGEPItem::Decode()
{
	if ( ! m_pBuffer ) return FALSE;

	BYTE* pOutput	= new BYTE[ m_nLength * 2 ];
	BYTE* pOut		= pOutput;
	BYTE* pIn		= m_pBuffer;

	if ( pOutput == NULL )
	{
		theApp.Message( MSG_ERROR, _T("Memory allocation error in CGGEPItem::Decode()") );
		theApp.Message( MSG_DEBUG, _T("Requested length: %i"), m_nLength * 2 );
		return FALSE;
	}

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

		nSize = (BYTE)min( nSize, nLength );

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
	m_nLength = static_cast< DWORD >( pOut - pOutput );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CGGEPItem compression

BOOL CGGEPItem::Deflate(BOOL bIfSmaller)
{
	if ( ! m_pBuffer ) return FALSE;
	if ( bIfSmaller && m_nLength < 45 ) return FALSE;

	DWORD nCompressed = 0;
	auto_array< BYTE > pCompressed( CZLib::Compress( m_pBuffer, m_nLength, &nCompressed ) );

	if ( !pCompressed.get() )
		return FALSE;

	if ( bIfSmaller && nCompressed >= m_nLength )
		return FALSE;

	delete [] m_pBuffer;
	m_pBuffer = pCompressed.release();
	m_nLength = nCompressed;

	return TRUE;
}

BOOL CGGEPItem::Inflate()
{
	if ( ! m_pBuffer ) return FALSE;

	DWORD nCompressed = 0;
	auto_array< BYTE > pCompressed( CZLib::Decompress( m_pBuffer, m_nLength, &nCompressed ) );

	if ( !pCompressed.get() )
		return FALSE;

	delete [] m_pBuffer;
	m_pBuffer = pCompressed.release();
	m_nLength = nCompressed;

	return TRUE;
}
