//
// QueryHashTable.cpp
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
#include "QueryHashTable.h"
#include "QueryHashGroup.h"
#include "QueryHashMaster.h"

#include "QuerySearch.h"
#include "Neighbour.h"
#include "Buffer.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "XML.h"
#include "ZLib.h"
#include "SHA.h"
#include "ED2K.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CQueryHashTable construction

CQueryHashTable::CQueryHashTable()
{
	m_bLive		= FALSE;
	m_nCookie	= 0;
	m_pHash		= NULL;
	m_nHash		= 0;
	m_nBits		= 0;
	m_nInfinity	= 1;
	m_nCount	= 0;
	m_pBuffer	= new CBuffer();
	m_pGroup	= NULL;
}

CQueryHashTable::~CQueryHashTable()
{
	if ( m_pGroup ) QueryHashMaster.Remove( this );
	if ( m_pHash ) delete [] m_pHash;
	delete m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable create

void CQueryHashTable::Create()
{
	BOOL bGrouped = m_pGroup != NULL;
	if ( bGrouped ) QueryHashMaster.Remove( this );
	
	if ( m_pHash ) delete [] m_pHash;
	
	m_bLive		= TRUE;
	m_nCookie	= GetTickCount();
	m_nBits		= Settings.Library.QueryRouteSize;
	m_nHash		= (int)pow( 2, m_nBits );
	m_pHash		= new BYTE[ ( m_nHash >> 3 ) + 1 ];
	m_nCount	= 0;
	
	FillMemory( m_pHash, ( m_nHash >> 3 ) + 1, 0xFF );
	
	if ( bGrouped ) QueryHashMaster.Add( this );
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable clear

void CQueryHashTable::Clear()
{
	if ( ! m_pHash ) return;
	
	BOOL bGrouped = m_pGroup != NULL;
	if ( bGrouped ) QueryHashMaster.Remove( this );
	
	m_nCookie	= GetTickCount();
	m_nCount	= 0;
	
	FillMemory( m_pHash, ( m_nHash >> 3 ) + 1, 0xFF );
	
	if ( bGrouped ) QueryHashMaster.Add( this );
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable merge tables

BOOL CQueryHashTable::Merge(CQueryHashTable* pSource)
{
	if ( m_pHash == NULL || pSource->m_pHash == NULL ) return FALSE;
	
	if ( m_nHash == pSource->m_nHash )
	{
		LPBYTE pSourcePtr	= pSource->m_pHash;
		LPBYTE pDestPtr		= m_pHash;
		
		for ( DWORD nPosition = m_nHash >> 3 ; nPosition ; nPosition-- )
		{
			register BYTE nSourceByte = *pSourcePtr;
			register BYTE nDestByte = *pDestPtr;
			
#define DO_MERGE(MASKVAL) \
			if ( ! ( nSourceByte & MASKVAL ) && ( nDestByte & MASKVAL ) ) \
			{ \
				*pDestPtr &= ~ MASKVAL; \
				m_nCount++; \
			}
			
			DO_MERGE(0x01); DO_MERGE(0x02); DO_MERGE(0x04); DO_MERGE(0x08);
			DO_MERGE(0x10); DO_MERGE(0x20); DO_MERGE(0x40); DO_MERGE(0x80);
#undef DO_MERGE
			
			pSourcePtr ++;
			pDestPtr ++;
		}
	}
	else
	{
		int nDestScale		= 1;
		int nSourceScale	= 1;
		
		if ( m_nHash > pSource->m_nHash )
		{
			for ( DWORD nIterate = pSource->m_nHash ; nIterate < m_nHash ; nIterate *= 2 ) nDestScale++;
			if ( nIterate != m_nHash ) return FALSE;
		}
		else if ( m_nHash < pSource->m_nHash )
		{
			for ( DWORD nIterate = m_nHash ; nIterate < pSource->m_nHash ; nIterate *= 2 ) nSourceScale++;
			if ( nIterate != pSource->m_nHash ) return FALSE;
		}
		
		LPBYTE pSourcePtr	= pSource->m_pHash;
		LPBYTE pDestPtr		= m_pHash;
		BYTE nSourceMask	= 0x01;
		BYTE nDestMask		= 0x01;
		
		for ( DWORD nDest = 0, nSource = 0 ; nDest < m_nHash && nSource < pSource->m_nHash ; )
		{
			BOOL bValue = TRUE;
			
			for ( int nSample = 0 ; nSample < nSourceScale ; nSample++, nSource++ )
			{
				if ( ( *pSourcePtr & nSourceMask ) == 0 ) bValue = FALSE;
				
				if ( nSourceMask == 0x80 )
				{
					nSourceMask = 0x01;
					pSourcePtr ++;
				}
				else
				{
					nSourceMask <<= 1;
				}
			}
			
			for ( nSample = 0 ; nSample < nDestScale ; nSample++, nDest++ )
			{
				if ( ! bValue && ( *pDestPtr & nDestMask ) )
				{
					*pDestPtr &= ~nDestMask;
					m_nCount++;
				}
				
				if ( nDestMask == 0x80 )
				{
					nDestMask = 0x01;
					pDestPtr ++;
				}
				else
				{
					nDestMask <<= 1;
				}
			}
		}
	}
	
	m_nCookie = GetTickCount();
	
	return TRUE;
}

BOOL CQueryHashTable::Merge(CQueryHashGroup* pSource)
{
	if ( m_pHash == NULL || pSource->m_pHash == NULL ) return FALSE;
	
	if ( m_nHash == pSource->m_nHash )
	{
		LPBYTE pSourcePtr	= pSource->m_pHash;
		LPBYTE pDestPtr		= m_pHash;
		
		for ( DWORD nPosition = m_nHash >> 3 ; nPosition ; nPosition-- )
		{
			register BYTE nDestByte = *pDestPtr;
			
#define DO_MERGE(MASKVAL) \
			if ( *pSourcePtr++ && ( nDestByte & MASKVAL ) ) \
			{ \
				*pDestPtr &= ~ MASKVAL; \
				m_nCount++; \
			}
			
			DO_MERGE(0x01); DO_MERGE(0x02); DO_MERGE(0x04); DO_MERGE(0x08);
			DO_MERGE(0x10); DO_MERGE(0x20); DO_MERGE(0x40); DO_MERGE(0x80);
#undef DO_MERGE
			
			pDestPtr++;
		}
	}
	else
	{
		int nDestScale		= 1;
		int nSourceScale	= 1;
		
		if ( m_nHash > pSource->m_nHash )
		{
			for ( DWORD nIterate = pSource->m_nHash ; nIterate < m_nHash ; nIterate *= 2 ) nDestScale++;
			if ( nIterate != m_nHash ) return FALSE;
		}
		else if ( m_nHash < pSource->m_nHash )
		{
			for ( DWORD nIterate = m_nHash ; nIterate < pSource->m_nHash ; nIterate *= 2 ) nSourceScale++;
			if ( nIterate != pSource->m_nHash ) return FALSE;
		}
		
		LPBYTE pSourcePtr	= pSource->m_pHash;
		LPBYTE pDestPtr		= m_pHash;
		BYTE nDestMask		= 0x01;
		
		for ( DWORD nDest = 0, nSource = 0 ; nDest < m_nHash && nSource < pSource->m_nHash ; )
		{
			BOOL bValue = TRUE;
			
			for ( int nSample = 0 ; nSample < nSourceScale ; nSample++, nSource++ )
			{
				if ( *pSourcePtr++ ) bValue = FALSE;
			}
			
			for ( nSample = 0 ; nSample < nDestScale ; nSample++, nDest++ )
			{
				if ( ! bValue && ( *pDestPtr & nDestMask ) )
				{
					*pDestPtr &= ~nDestMask;
					m_nCount++;
				}
				
				if ( nDestMask == 0x80 )
				{
					nDestMask = 0x01;
					pDestPtr ++;
				}
				else
				{
					nDestMask <<= 1;
				}
			}
		}
	}
	
	m_nCookie = GetTickCount();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable packet patch dispatch

BOOL CQueryHashTable::PatchTo(CQueryHashTable* pTarget, CNeighbour* pNeighbour)
{
	if ( pTarget->m_pHash == NULL ) return FALSE;
	if ( m_nCookie == pTarget->m_nCookie ) return FALSE;
	
	m_nCookie	= pTarget->m_nCookie;
	m_nCount	= pTarget->m_nCount;
	
	BOOL bChanged = FALSE;
	
	if ( m_pHash == NULL || m_nHash != pTarget->m_nHash )
	{
		if ( m_pHash ) delete [] m_pHash;
		m_pHash = NULL;
		
		m_nBits		= pTarget->m_nBits;
		m_nHash		= pTarget->m_nHash;
		m_pHash		= new BYTE[ ( m_nHash >> 3 ) + 1 ];
		
		FillMemory( m_pHash, ( m_nHash >> 3 ) + 1, 0xFF );
		
		if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
		{
			CG1Packet* pReset = CG1Packet::New( G1_PACKET_QUERY_ROUTE, 1 );
			pReset->WriteByte( 0 );
			pReset->WriteLongLE( m_nHash );
			pReset->WriteByte( 2 );
			pNeighbour->Send( pReset );
		}
		else
		{
			CG2Packet* pReset = CG2Packet::New( G2_PACKET_QHT );
			pReset->WriteByte( 0 );
			pReset->WriteLongBE( m_nHash );
			pReset->WriteByte( 1 );
			pNeighbour->Send( pReset );
		}
		
		bChanged = TRUE;
	}
	
	BYTE nBits = 4;
	
	if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
	{
		nBits = 1;
	}
	else if ( pNeighbour->m_sUserAgent.Find( _T("Shareaza") ) == 0 )
	{
		LPCTSTR pszAgent = pNeighbour->m_sUserAgent;
		
		if (	_tcsstr( pszAgent, _T(" 1.3") ) ||
				_tcsstr( pszAgent, _T(" 1.2") ) ||
				_tcsstr( pszAgent, _T(" 1.1") ) ||
				_tcsstr( pszAgent, _T(" 1.0") ) )
		{
			return PatchToOldShareaza( pTarget, pNeighbour );
		}
		
		if (	_tcsstr( pszAgent, _T(" 1.4") ) == NULL &&
				_tcsstr( pszAgent, _T(" 1.5") ) == NULL &&
				_tcsstr( pszAgent, _T(" 1.6.0") ) == NULL )
		{
			nBits = 1;
		}
	}
	
	BYTE* pBuffer	= new BYTE[ m_nHash / ( 8 / nBits ) ];
	BYTE* pHashT	= pTarget->m_pHash;
	BYTE* pHashS	= m_pHash;
	BYTE  nMask		= 1;
	
	if ( nBits == 4 )
	{
		for ( DWORD nPosition = 0 ; nPosition < m_nHash ; nPosition++ )
		{
			BYTE nPatch = 0;
			
			if ( ( *pHashT & nMask ) == ( *pHashS & nMask ) )
			{
			}
			else if ( ( *pHashT & nMask ) )
			{
				nPatch = 1;
				*pHashS = ( *pHashS & ~nMask ) | ( *pHashT & nMask );
			}
			else
			{
				nPatch = 15;
				*pHashS = ( *pHashS & ~nMask ) | ( *pHashT & nMask );
			}
			
			if ( nPosition & 1 )
				pBuffer[ nPosition >> 1 ] |= nPatch;
			else
				pBuffer[ nPosition >> 1 ] = nPatch << 4;
			
			if ( nMask == 0x80 )
			{
				pHashS++;
				pHashT++;
				nMask = 1;
			}
			else
			{
				nMask <<= 1;
			}
			
			if ( nPatch ) bChanged = TRUE;
		}
	}
	else
	{
		ZeroMemory( pBuffer, m_nHash >> 3 );
		BYTE* pOutput = pBuffer;
		
		for ( DWORD nPosition = m_nHash ; nPosition ; nPosition-- )
		{
			if ( ( *pHashT & nMask ) != ( *pHashS & nMask ) )
			{
				*pOutput |= nMask;
				bChanged = TRUE;
			}
			
			*pHashS = ( *pHashS & ~nMask ) | ( *pHashT & nMask );
			
			if ( nMask == 0x80 )
			{
				pOutput++;
				pHashS++;
				pHashT++;
				nMask = 1;
			}
			else
			{
				nMask <<= 1;
			}
		}
	}
	
	if ( bChanged == FALSE && m_bLive )
	{
		delete [] pBuffer;
		return FALSE;
	}
	
	DWORD nCompress = 0;
	BYTE* pCompress = CZLib::Compress( pBuffer, m_nHash / ( 8 / nBits ), &nCompress );
	
	delete [] pBuffer;
	
	if ( pCompress == NULL ) return FALSE;
	
	DWORD nPacketSize	= 1024;
	BYTE nSequenceMax	= (BYTE)( nCompress / nPacketSize );
	if ( nCompress % nPacketSize ) nSequenceMax++;

	pBuffer = pCompress;

	for ( BYTE nSequence = 1 ; nSequence <= nSequenceMax ; nSequence++ )
	{
		CPacket* pPatch = NULL;

		if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
			pPatch = CG1Packet::New( G1_PACKET_QUERY_ROUTE, 1 );
		else
			pPatch = CG2Packet::New( G2_PACKET_QHT );

		pPatch->WriteByte( 1 );
		pPatch->WriteByte( nSequence );
		pPatch->WriteByte( nSequenceMax );
		pPatch->WriteByte( 1 );
		pPatch->WriteByte( nBits );

		DWORD nPacket = min( nCompress, nPacketSize );

		pPatch->Write( pBuffer, nPacket );

		pBuffer += nPacket;
		nCompress -= nPacket;

		pNeighbour->Send( pPatch );
	}
	
	delete [] pCompress;
	m_bLive = TRUE;
	
	return TRUE;
}

BOOL CQueryHashTable::PatchToOldShareaza(CQueryHashTable* pTarget, CNeighbour* pNeighbour)
{ 
	DWORD nPacketSize = 4096;

	BYTE* pBuffer	= new BYTE[ nPacketSize ];
	BYTE* pHashT	= pTarget->m_pHash;
	BYTE* pHashS	= m_pHash;
	DWORD nPosition	= 0;

	for ( BYTE nSequence = 1 ; nPosition < m_nHash ; nSequence++ )
	{
		CG1Packet* pPatch = CG1Packet::New( G1_PACKET_QUERY_ROUTE, 1 );

		pPatch->WriteByte( 1 );
		pPatch->WriteByte( nSequence );
		pPatch->WriteByte( (BYTE)( m_nHash / nPacketSize ) );
		pPatch->WriteByte( 1 );
		pPatch->WriteByte( 4 );

		BYTE nMask = 1;

		for ( DWORD nCount = 0 ; nCount < nPacketSize ; nCount++, nPosition++ )
		{
			BYTE nPatch = ( *pHashT & nMask ) != ( *pHashS & nMask ) ? 7 : 0;

			*pHashS = ( *pHashS & ~nMask ) | ( *pHashT & nMask );

			if ( nCount & 1 )
				pBuffer[ nCount >> 1 ] |= nPatch;
			else
				pBuffer[ nCount >> 1 ] = nPatch << 4;

			if ( nMask == 0x80 )
			{
				nMask = 1;
				pHashT ++;
				pHashS ++;
			}
			else
			{
				nMask <<= 1;
			}
		}

		pPatch->WriteZLib( pBuffer, nCount / 2 );
		pNeighbour->Send( pPatch );
	}
	
	delete [] pBuffer;
	m_bLive = TRUE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable packet handler

BOOL CQueryHashTable::OnPacket(CPacket* pPacket)
{
	if ( pPacket->m_nLength < 1 ) return FALSE;
	
	if ( pPacket->m_nProtocol == PROTOCOL_G1 )
	{
		CG1Packet* pG1 = (CG1Packet*)pPacket;
		if ( pG1->m_nTTL != 1 )   return FALSE;
		if ( pG1->m_nHops != 0 )  return FALSE;
	}
	else
	{
		CG2Packet* pG2 = (CG2Packet*)pPacket;
		DWORD nLength = pG2->m_nLength;
		if ( pG2->m_bCompound ) pG2->SkipCompound( nLength );
	}
	
	BYTE nVariant = pPacket->ReadByte();
	
	if ( nVariant == 0 )
	{
		return OnReset( pPacket );
	}
	else if ( nVariant == 1 )
	{
		return OnPatch( pPacket );
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable reset handler

BOOL CQueryHashTable::OnReset(CPacket* pPacket)
{
	if ( pPacket->m_nLength != 6 ) return FALSE;
	
	DWORD nHashSize	= 0;
	
	BOOL bGrouped = m_pGroup != NULL;
	if ( bGrouped ) QueryHashMaster.Remove( this );
	
	if ( pPacket->m_nProtocol == PROTOCOL_G1 )
	{
		nHashSize	= pPacket->ReadLongLE();
		m_nInfinity	= pPacket->ReadByte();
	}
	else
	{
		nHashSize	= pPacket->ReadLongBE();
		m_nInfinity	= pPacket->ReadByte();
	}
	
	if ( nHashSize < 64 ) return FALSE;	// Minimum size
	
	if ( nHashSize != m_nHash || m_pHash == NULL )
	{
		if ( m_pHash ) delete [] m_pHash;
		m_pHash = NULL;
		
		for ( m_nHash = 1, m_nBits = 0 ; m_nHash < nHashSize ; m_nBits++ ) m_nHash *= 2;
		if ( m_nHash != nHashSize ) return FALSE;
		if ( m_nBits > 24 ) return FALSE;
		
		m_pHash	= new BYTE[ ( m_nHash >> 3 ) + 1 ];
	}
	
	FillMemory( m_pHash, ( m_nHash >> 3 ) + 1, 0xFF );
	
	if ( bGrouped ) QueryHashMaster.Add( this );
	
	m_bLive		= FALSE;
	m_nCookie	= GetTickCount();
	m_nCount	= 0;
	
	m_pBuffer->Clear();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable patch handler

BOOL CQueryHashTable::OnPatch(CPacket* pPacket)
{
	if ( pPacket->m_nLength < 5 ) return FALSE;
	if ( m_pHash == NULL ) return FALSE;
	if ( m_pBuffer == NULL ) return FALSE;
	
	BYTE nSequence		= pPacket->ReadByte();
	BYTE nMaximum		= pPacket->ReadByte();
	BYTE nCompression	= pPacket->ReadByte();
	BYTE nBits			= pPacket->ReadByte();
	
	if ( nBits != 1 && nBits != 4 && nBits != 8 ) return FALSE;
	if ( nSequence < 1 || nSequence > nMaximum ) return FALSE;
	if ( nCompression > 1 ) return FALSE;
	
	if ( nSequence == 1 ) m_pBuffer->Clear();
	
	m_pBuffer->Add(	pPacket->m_pBuffer + pPacket->m_nPosition,
					pPacket->m_nLength - pPacket->m_nPosition );
	
	if ( nSequence < nMaximum ) return TRUE;
	
	if ( nCompression == 1 )
	{
		if ( ! m_pBuffer->Inflate( m_nHash + 4 ) )
		{
			m_pBuffer->Clear();
			return FALSE;
		}
	}
	
	if ( m_pBuffer->m_nLength != m_nHash / ( 8 / nBits ) )
	{
		m_pBuffer->Clear();
		return FALSE;
	}
	
	BYTE* pData		= m_pBuffer->m_pBuffer;
	BYTE* pHash		= m_pHash;
	
	BOOL bGroup		= ( m_pGroup != NULL && m_pGroup->m_nHash == m_nHash );
	BYTE* pGroup	= bGroup ? m_pGroup->m_pHash : NULL;
	
	if ( nBits == 1 )
	{
		for ( DWORD nPosition = ( m_nHash >> 3 ) ; nPosition ; nPosition--, pHash++, pData++ )
		{
			for ( BYTE nMask = 1 ; ; nMask <<= 1 )
			{
				if ( *pData & nMask )
				{
					if ( *pHash & nMask )
					{
						m_nCount ++;
						*pHash &= ~nMask;
						if ( bGroup )
						{
#ifdef _DEBUG
							ASSERT( *pGroup < 255 );
							if ( *pGroup == 0 ) m_pGroup->m_nCount++;
#endif
							(*pGroup) ++;
						}
					}
					else
					{
						m_nCount --;
						*pHash |= nMask;
						
						if ( bGroup )
						{
#ifdef _DEBUG
							ASSERT( *pGroup );
							if ( *pGroup == 1 ) m_pGroup->m_nCount--;
#endif
							(*pGroup) --;
						}
					}
				}
				
				pGroup++;
				if ( nMask == 0x80 ) break;
			}
		}
	}
	else
	{
		BYTE nMask = 1;
		
		for ( DWORD nPosition = 0 ; nPosition < m_nHash ; nPosition++, pData++, pGroup++ )
		{
			if ( nBits == 8 )
			{
				if ( *pData )
				{
					if ( *pHash & nMask )
					{
						m_nCount++;
						*pHash &= ~nMask;
						if ( bGroup ) (*pGroup) ++;
					}
					else
					{
						m_nCount--;
						*pHash |= nMask;
						if ( bGroup ) (*pGroup) --;
					}
				}
			}
			else
			{
				if ( *pData & 0xF0 )
				{
					if ( *pHash & nMask )
					{
						m_nCount++;
						*pHash &= ~nMask;
						if ( bGroup ) (*pGroup) ++;
					}
					else
					{
						m_nCount--;
						*pHash |= nMask;
						if ( bGroup ) (*pGroup) --;
					}
				}
				
				if ( ++nPosition >= m_nHash ) return FALSE;
				pGroup ++;
				nMask <<= 1;
				
				if ( *pData & 0x0F )
				{
					if ( *pHash & nMask )
					{
						m_nCount++;
						*pHash &= ~nMask;
						if ( bGroup ) *pGroup ++;
					}
					else
					{
						m_nCount--;
						*pHash |= nMask;
						if ( bGroup ) *pGroup --;
					}
				}
			}
			
			if ( nMask == 0x80 )
			{
				pHash++;
				nMask = 1;
			}
			else
			{
				nMask <<= 1;
			}
		}
	}
	
	m_bLive		= TRUE;
	m_nCookie	= GetTickCount();
	
	if ( bGroup ) QueryHashMaster.Invalidate();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable add phrases and words

int CQueryHashTable::AddPhrase(LPCTSTR pszPhrase)
{
	if ( m_pHash == NULL ) return 0;
	
	LPCTSTR pszPtr = pszPhrase;
	int nCount = 0;
	
	for ( int nStart = 0, nPos = 0 ; *pszPtr ; nPos++, pszPtr++ )
	{
		if ( ! IsCharacter( *pszPtr ) )
		{
			if ( nStart < nPos )
			{
				nCount += Add( pszPhrase, nStart, nPos - nStart );
			}
			
			nStart = nPos + 1;
		}
	}
	
	if ( nStart < nPos )
	{
		nCount += Add( pszPhrase, nStart, nPos - nStart );
	}
	
	return nCount;
}

int CQueryHashTable::AddString(LPCTSTR pszString)
{
	if ( m_pHash == NULL ) return 0;
	return Add( pszString, 0, _tcslen( pszString ) );
}

int CQueryHashTable::Add(LPCTSTR pszString, int nStart, int nLength)
{
	if ( ! nLength || ! IsWord( pszString, nStart, nLength ) ) return 0;
	
	m_nCookie = GetTickCount();
	
	DWORD nHash	= HashWord( pszString, nStart, nLength, m_nBits );
	BYTE* pHash	= m_pHash + ( nHash >> 3 );
	BYTE nMask	= 1 << ( nHash & 7 );
	
	if ( *pHash & nMask )
	{
		m_nCount++;
		*pHash &= ~nMask;
	}
	
	if ( nLength >= 5 )
	{
		nHash	= HashWord( pszString, nStart, nLength - 1, m_nBits );
		pHash	= m_pHash + ( nHash >> 3 );
		nMask	= 1 << ( nHash & 7 );
		
		if ( *pHash & nMask )
		{
			m_nCount++;
			*pHash &= ~nMask;
		}
		
		nHash	= HashWord( pszString, nStart, nLength - 2, m_nBits );
		pHash	= m_pHash + ( nHash >> 3 );
		nMask	= 1 << ( nHash & 7 );
		
		if ( *pHash & nMask )
		{
			m_nCount++;
			*pHash &= ~nMask;
		}
		
		return 3;
	}
	
	return 1;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable check phrases and words

BOOL CQueryHashTable::CheckPhrase(LPCTSTR pszPhrase)
{
	if ( ! m_bLive || m_pHash == NULL || ! *pszPhrase ) return TRUE;

	int nWordCount	= 0;
	int nWordHits	= 0;

	LPCTSTR pszPtr	= pszPhrase;
	BOOL bNegate	= FALSE;
	BOOL bSpace		= TRUE;

	for ( int nStart = 0, nPos = 0 ; *pszPtr ; nPos++, pszPtr++ )
	{
		if ( IsCharacter( *pszPtr ) )
		{
			bSpace = FALSE;
		}
		else
		{
			if ( nStart < nPos && ! bNegate && IsWord( pszPhrase, nStart, nPos - nStart ) )
			{
				nWordCount++;
				DWORD nHash	= HashWord( pszPhrase, nStart, nPos - nStart, m_nBits );
				BYTE* pHash	= m_pHash + ( nHash >> 3 );
				BYTE nMask	= 1 << ( nHash & 7 );
				if ( ! ( *pHash & nMask ) ) nWordHits++;
			}
			
			nStart	= nPos + 1;
			bNegate = bSpace && ( *pszPtr == '-' );
			bSpace	= ( *pszPtr == ' ' );
		}
	}
	
	if ( nStart < nPos && ! bNegate && IsWord( pszPhrase, nStart, nPos - nStart ) )
	{
		nWordCount++;
		DWORD nHash	= HashWord( pszPhrase, nStart, nPos - nStart, m_nBits );
		BYTE* pHash	= m_pHash + ( nHash >> 3 );
		BYTE nMask	= 1 << ( nHash & 7 );
		if ( ! ( *pHash & nMask ) ) nWordHits++;
	}
	
	return ( nWordCount >= 3 ) ? ( nWordHits * 3 / nWordCount >= 2 ) : ( nWordHits == nWordCount );
}

BOOL CQueryHashTable::CheckString(LPCTSTR pszString)
{
	if ( ! m_bLive || m_pHash == NULL || ! *pszString ) return TRUE;

	DWORD nHash	= HashWord( pszString, 0, _tcslen( pszString ), m_nBits );
	BYTE* pHash	= m_pHash + ( nHash >> 3 );
	BYTE nMask	= 1 << ( nHash & 7 );

	return ! ( *pHash & nMask );
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable check query object

BOOL CQueryHashTable::Check(CQuerySearch* pSearch)
{
	if ( ! m_bLive || m_pHash == NULL ) return TRUE;
	
	if ( pSearch->m_bSHA1 || pSearch->m_bED2K || pSearch->m_bBTH )
	{
		if ( pSearch->m_bSHA1 )
		{
			if ( CheckString( CSHA::HashToString( &pSearch->m_pSHA1, TRUE ) ) ) return TRUE;
		}
		
		if ( pSearch->m_bED2K )
		{
			if ( CheckString( CED2K::HashToString( &pSearch->m_pED2K, TRUE ) ) ) return TRUE;
		}
		
		if ( pSearch->m_bBTH )
		{
			if ( CheckString( _T("urn:btih:") + CSHA::HashToString( &pSearch->m_pBTH, FALSE ) ) ) return TRUE;
		}
		
		return FALSE;
	}
	
	LPCTSTR* pWordPtr	= pSearch->m_pWordPtr;
	DWORD* pWordLen		= pSearch->m_pWordLen;
	DWORD nWordHits		= 0;
	
	for ( int nWord = pSearch->m_nWords ; nWord > 0 ; nWord--, pWordPtr++, pWordLen++ )
	{
		if ( **pWordPtr == '-' ) continue;
		
		DWORD nHash	= HashWord( *pWordPtr, 0, *pWordLen, m_nBits );
		BYTE* pHash	= m_pHash + ( nHash >> 3 );
		BYTE nMask	= 1 << ( nHash & 7 );
		if ( ! ( *pHash & nMask ) ) nWordHits++;
	}
	
	return ( pSearch->m_nWords >= 3 ) ? ( nWordHits * 3 / pSearch->m_nWords >= 2 ) : ( nWordHits == pSearch->m_nWords );
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable hash functions

DWORD CQueryHashTable::HashWord(LPCTSTR pszString, int nStart, int nLength, int nBits)
{
	DWORD nNumber	= 0;
	int nByte		= 0;

	pszString += nStart;

	for ( int nChar = 0 ; nChar < nLength ; nChar++, pszString++ )
	{
		int nValue = tolower( *pszString ) & 0xFF; 

		nValue = nValue << ( nByte * 8 );
		nByte = ( nByte + 1 ) & 3;

		nNumber = nNumber ^ nValue;
	}

	return HashNumber( nNumber, nBits );
}

DWORD CQueryHashTable::HashNumber(DWORD nNumber, int nBits)
{
	unsigned __int64 nProduct	= (unsigned __int64)nNumber * (unsigned __int64)0x4F1BBCDC;
	unsigned __int64 nHash		= nProduct << 32;
	nHash = nHash >> ( 32 + ( 32 - nBits ) );
	return (DWORD)nHash;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable calculate percent full

int CQueryHashTable::GetPercent()
{
	if ( ! m_pHash || ! m_nHash ) return 0;
	return m_nCount * 100 / m_nHash;
}

