//
// ED2K.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
#include "HashLib.h"

//////////////////////////////////////////////////////////////////////
// CED2K construction

CED2K::CED2K() :
	m_pList			( NULL )
,	m_nList			( 0 )
,	m_nCurHash		( 0 )
,	m_nCurByte		( 0 )
,	m_bNullBlock	( FALSE )
{
    std::fill_n( &m_pRoot[ 0 ], 4, 0 );
}

CED2K::~CED2K()
{
	delete [] m_pList;
}

//////////////////////////////////////////////////////////////////////
// CED2K clear

void CED2K::Clear()
{
    std::fill_n( &m_pRoot[ 0 ], 4, 0 );
	delete [] m_pList;
	m_pList = NULL;
	m_nList = 0;
	m_pSegment.Reset();
	m_nCurHash = 0;
	m_nCurByte = 0;
}

void CED2K::SetSize(uint32 nSize)
{
	Clear();

	m_nList = nSize;
}

uint32 CED2K::GetSize() const
{
	return m_nList;
}

void CED2K::Save(uchar* pBuf) const
{
	if ( m_nList )
	{
		CopyMemory( pBuf, &m_pRoot[ 0 ], sizeof( CMD4::Digest ) );
		pBuf += sizeof( CMD4::Digest );
		if ( m_nList > 1 )
			CopyMemory( pBuf, m_pList, sizeof( CMD4::Digest ) * m_nList );
	}
}

void CED2K::Load(const uchar* pBuf)
{
	if ( m_nList )
	{
		CopyMemory( &m_pRoot[ 0 ], pBuf, sizeof( CMD4::Digest ) );
		pBuf += sizeof( CMD4::Digest );
		m_pList = new (std::nothrow) CMD4::Digest[ m_nList ];
		if ( m_pList )
		{
			if ( m_nList > 1 )
				CopyMemory( m_pList, pBuf, sizeof( CMD4::Digest ) * m_nList );
			else if ( m_nList == 1 )
				std::copy( &m_pRoot[ 0 ], &m_pRoot[ 4 ], &m_pList[ 0 ][ 0 ] );
		}
	}
}

uint32 CED2K::GetSerialSize() const
{
	uint32 nSize = 0;
    if ( m_nList > 0 ) nSize += sizeof( CMD4::Digest );
    if ( m_nList > 1 ) nSize += sizeof( CMD4::Digest ) * m_nList;
	return nSize;
}

//////////////////////////////////////////////////////////////////////
// CED2K file hashing - begin

void CED2K::BeginFile(uint64 nLength)
{
	Clear();

	m_nList = nLength ? (uint32)( ( nLength + ED2K_PART_SIZE ) / ED2K_PART_SIZE ) : 0;
	if ( nLength % ED2K_PART_SIZE == 0 && nLength ) 
		m_bNullBlock = true;
    m_pList	= new (std::nothrow) CMD4::Digest[ m_nList ];
}

//////////////////////////////////////////////////////////////////////
// CED2K file hashing - add data

void CED2K::AddToFile(LPCVOID pInput, uint32 nLength)
{
	if ( nLength == 0 ) return;

	const BYTE* pBytes = (const BYTE*)pInput;

	while ( nLength > 0 )
	{
		uint32 nInThisHash	= ( ED2K_PART_SIZE - m_nCurByte );
		uint32 nToProcess	= min( nInThisHash, nLength );

		m_pSegment.Add( pBytes, nToProcess );

		m_nCurByte += nToProcess;
		nLength -= nToProcess;
		pBytes += nToProcess;

		if ( m_nCurByte >= ED2K_PART_SIZE )
		{
			m_pSegment.Finish();
			m_pSegment.GetHash( (uchar*)&m_pList[ m_nCurHash ][ 0 ] );
			m_pSegment.Reset();
			m_nCurHash++;
			m_nCurByte = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CED2K file hashing - finish

BOOL CED2K::FinishFile()
{
	if ( !m_bNullBlock && m_nCurHash < m_nList )
	{
		m_pSegment.Finish();
		m_pSegment.GetHash( (uchar*)&m_pList[ m_nCurHash++ ][ 0 ] );
	}

	if ( m_bNullBlock && m_nCurHash <= m_nList )
	{
		m_pSegment.Finish();
		m_pSegment.GetHash( (uchar*)&m_pList[ m_nCurHash ][ 0 ] );
	}

	if ( m_nList == 1 )
	{
        std::copy( &m_pList[ 0 ][ 0 ], &m_pList[ 0 ][ 4 ], &m_pRoot[ 0 ] );
	}
	else if ( m_nList == 0 )
	{
		m_pSegment.Finish();
		m_pSegment.GetHash( (uchar*)&m_pRoot[ 0 ] );
	}
	else
	{
		CMD4 pOverall;
        pOverall.Add( m_pList, sizeof( CMD4::Digest ) * m_nList );
		pOverall.Finish();
		pOverall.GetHash( (uchar*)&m_pRoot[ 0 ] );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CED2K block testing - begin

void CED2K::BeginBlockTest()
{
	m_pSegment.Reset();
	m_nCurByte = 0;
}

//////////////////////////////////////////////////////////////////////
// CED2K block testing - add data

void CED2K::AddToTest(LPCVOID pInput, uint32 nLength)
{
	if ( nLength == 0 ) return;

	m_pSegment.Add( pInput, nLength );
	m_nCurByte += nLength;
}

//////////////////////////////////////////////////////////////////////
// CED2K block testing - finish

BOOL CED2K::FinishBlockTest(uint32 nBlock)
{
	if ( nBlock >= m_nList ) return FALSE;
	
    CMD4::Digest pMD4;

	m_pSegment.Finish();
	m_pSegment.GetHash( (uchar*)&pMD4[ 0 ] );
	
    return std::equal( &pMD4[ 0 ], &pMD4[ 4 ], &m_pList[ nBlock ][ 0 ] );
}

//////////////////////////////////////////////////////////////////////
// CED2K encode to bytes

BOOL CED2K::ToBytes(BYTE** ppOutput, uint32* pnOutput) const
{
	if ( m_nList == 0 )
		return FALSE;

    *pnOutput = sizeof( CMD4::Digest ) * m_nList;

	*ppOutput = (uint8*)GlobalAlloc( GPTR, *pnOutput );
	if ( ! *ppOutput )
		return FALSE;

	CopyMemory( *ppOutput, m_pList, *pnOutput );

	return TRUE;
}

LPCVOID CED2K::GetRawPtr() const
{
	return (LPCVOID)m_pList;
}

void CED2K::GetRoot(__in_bcount(16) uchar* pHash) const
{
	std::copy( &m_pRoot[ 0 ], &m_pRoot[ 4 ], (uint32*)pHash );
}

void CED2K::FromRoot(__in_bcount(16) const uchar* pHash)
{
	Clear();

	m_nList = 1;
	m_pList = new (std::nothrow) CMD4::Digest[ m_nList ];
	if ( m_pList )
	{
		std::copy( (uint32*)pHash, ( (uint32*)pHash ) + 4, &m_pRoot[ 0 ] );
		std::copy( (uint32*)pHash, ( (uint32*)pHash ) + 4, &m_pList[ 0 ][ 0 ] );
	}
}

//////////////////////////////////////////////////////////////////////
// CED2K decode from bytes

BOOL CED2K::FromBytes(BYTE* pOutput, uint32 nOutput, uint64 nSize)
{
	Clear();
	
    if ( pOutput == NULL || nOutput == 0 || ( nOutput % sizeof( CMD4::Digest ) ) != 0 ) return FALSE;
	
	if ( nSize > 0 )
	{
        if ( nSize % ED2K_PART_SIZE == 0 && nSize ) 
			m_bNullBlock = true;

		uint64 nValidBlocks = ( nSize + ED2K_PART_SIZE - 1 ) / ED2K_PART_SIZE;
		if ( m_bNullBlock )
			nValidBlocks++;

		if ( nOutput / sizeof( CMD4::Digest ) != nValidBlocks )
			return FALSE;
	}
	
    m_nList	= nOutput / sizeof( CMD4::Digest );
    m_pList = new (std::nothrow) CMD4::Digest[ m_nList ];
	if ( ! m_pList )
		return FALSE;
	
	CopyMemory( m_pList, pOutput, nOutput );

	if ( m_nList == 1 )
	{
		m_pRoot = *m_pList;
	}
	else
	{
		CMD4 pOverall;
        pOverall.Add( m_pList, sizeof( CMD4::Digest ) * m_nList );
		pOverall.Finish();
		pOverall.GetHash( (uchar*)&m_pRoot[ 0 ] );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CED2K integrity checking

BOOL CED2K::CheckIntegrity() const
{
	if ( m_nList == 1 )
	{
        return std::equal( &m_pRoot[ 0 ], &m_pRoot[ 4 ], &m_pList[ 0 ][ 0 ] );
	}
	else
	{
		CMD4 pOverall;
        CMD4::Digest pRoot;
		
        pOverall.Add( m_pList, sizeof( CMD4::Digest ) * m_nList );
		pOverall.Finish();
		pOverall.GetHash( (uchar*)&pRoot[ 0 ] );
		
		return std::equal( &m_pRoot[ 0 ], &m_pRoot[ 4 ], &pRoot[ 0 ] );
	}
}

BOOL CED2K::IsAvailable() const
{
	return m_pList != NULL;
}

uint32 CED2K::GetBlockCount() const
{
	return m_nList;
}
