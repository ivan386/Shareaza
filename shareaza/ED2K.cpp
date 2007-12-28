//
// ED2K.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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
#include "ED2K.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CED2K construction

CED2K::CED2K() :
	m_pList			( NULL )
,	m_nList			( 0 )
,	m_bNullBlock	( FALSE )
{
}

CED2K::~CED2K()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CED2K clear

void CED2K::Clear()
{
    std::fill_n( &m_pRoot[ 0 ], 4, 0 );
	if ( m_pList != NULL ) delete [] m_pList;
	m_pList = NULL;
	m_nList = 0;
}

//////////////////////////////////////////////////////////////////////
// CED2K serialization

void CED2K::Serialize(CArchive& ar)
{
	if ( ar.IsStoring() )
	{
		ar << m_nList;
		if ( m_nList == 0 ) return;
		
		ar.Write( &m_pRoot[ 0 ], sizeof( m_pRoot ) );
        if ( m_nList > 1 ) ar.Write( m_pList, sizeof( CMD4::MD4Digest ) * m_nList );
	}
	else
	{
		Clear();

		ar >> m_nList;
		if ( m_nList == 0 ) return;

		ReadArchive( ar, &m_pRoot[ 0 ], sizeof( m_pRoot ) );
		
		m_pList = new CMD4::MD4Digest[ m_nList ];

		if ( m_nList > 1 )
		{
			ReadArchive( ar, m_pList, sizeof( CMD4::MD4Digest ) * m_nList );
		}
		else if ( m_nList == 1 )
		{
			std::copy( &m_pRoot[ 0 ], &m_pRoot[ 4 ], &m_pList[ 0 ][ 0 ] );
		}
	}
}

DWORD CED2K::GetSerialSize() const
{
	DWORD nSize = 4;
    if ( m_nList > 0 ) nSize += sizeof( CMD4::MD4Digest );
    if ( m_nList > 1 ) nSize += sizeof( CMD4::MD4Digest ) * m_nList;
	return nSize;
}

//////////////////////////////////////////////////////////////////////
// CED2K root value

BOOL CED2K::GetRoot(Hashes::Ed2kHash& oHash) const
{
	// if ( m_nList == 0 ) return FALSE;
	oHash = reinterpret_cast< const Hashes::Ed2kHash::RawStorage& >(
			m_pRoot[ 0 ] );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CED2K file hashing - begin

void CED2K::BeginFile(QWORD nLength)
{
	ASSERT( ! IsAvailable() );

	m_nList = nLength ? (DWORD)( ( nLength + ED2K_PART_SIZE ) / ED2K_PART_SIZE ) : 0;
	if ( nLength % ED2K_PART_SIZE == 0 && nLength ) 
		m_bNullBlock = true;
    m_pList	= new CMD4::MD4Digest[ m_nList ];
	
	m_pSegment.Reset();
	m_nCurHash = 0;
	m_nCurByte = 0;
}

//////////////////////////////////////////////////////////////////////
// CED2K file hashing - add data

void CED2K::AddToFile(LPCVOID pInput, DWORD nLength)
{
	if ( nLength == 0 ) return;

	ASSERT( IsAvailable() );
	ASSERT( m_nCurHash < m_nList );
	ASSERT( m_nCurByte < ED2K_PART_SIZE );

	const BYTE* pBytes = (const BYTE*)pInput;

	while ( nLength > 0 )
	{
		DWORD nInThisHash	= ( ED2K_PART_SIZE - m_nCurByte );
		DWORD nToProcess	= min( nInThisHash, nLength );

		m_pSegment.Add( pBytes, nToProcess );

		m_nCurByte += nToProcess;
		nLength -= nToProcess;
		pBytes += nToProcess;

		if ( m_nCurByte >= ED2K_PART_SIZE )
		{
			ASSERT( m_nCurHash < m_nList );
			m_pSegment.Finish();
			m_pSegment.GetHash( m_pList[ m_nCurHash ] );
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
	ASSERT( IsAvailable() );
	ASSERT( m_nCurHash <= m_nList );
	ASSERT( m_nCurByte < ED2K_PART_SIZE );

	if ( !m_bNullBlock && m_nCurHash < m_nList )
	{
		m_pSegment.Finish();
		m_pSegment.GetHash( m_pList[ m_nCurHash++ ] );
	}

	if ( m_bNullBlock && m_nCurHash <= m_nList )
	{
		ASSERT( m_nCurByte == 0 );
		m_pSegment.Finish();
		m_pSegment.GetHash( m_pList[ m_nCurHash ] );
	}

	ASSERT( m_nCurHash <= m_nList );

	if ( m_nList == 1 )
	{
        std::copy( &m_pList[ 0 ][ 0 ], &m_pList[ 0 ][ 4 ], &m_pRoot[ 0 ] );
	}
	else if ( m_nList == 0 )
	{
		m_pSegment.Finish();
		m_pSegment.GetHash( m_pRoot );
	}
	else
	{
		CMD4 pOverall;
        pOverall.Add( m_pList, sizeof( CMD4::MD4Digest ) * m_nList );
		pOverall.Finish();
		pOverall.GetHash( m_pRoot );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CED2K block testing - begin

void CED2K::BeginBlockTest()
{
	ASSERT( IsAvailable() );

	m_pSegment.Reset();
	m_nCurByte = 0;
}

//////////////////////////////////////////////////////////////////////
// CED2K block testing - add data

void CED2K::AddToTest(LPCVOID pInput, DWORD nLength)
{
	if ( nLength == 0 ) return;

	ASSERT( IsAvailable() );
	ASSERT( m_nCurByte + nLength <= ED2K_PART_SIZE );

	m_pSegment.Add( pInput, nLength );
	m_nCurByte += nLength;
}

//////////////////////////////////////////////////////////////////////
// CED2K block testing - finish

BOOL CED2K::FinishBlockTest(DWORD nBlock)
{
	ASSERT( IsAvailable() );

	if ( nBlock >= m_nList ) return FALSE;
	
    CMD4::MD4Digest pMD4;

	m_pSegment.Finish();
	m_pSegment.GetHash( pMD4 );
	
    return std::equal( &pMD4[ 0 ], &pMD4[ 4 ], &m_pList[ nBlock ][ 0 ] );
}

//////////////////////////////////////////////////////////////////////
// CED2K encode to bytes

BOOL CED2K::ToBytes(BYTE** ppOutput, DWORD* pnOutput)
{
	if ( m_nList == 0 ) return FALSE;

    *pnOutput = sizeof( CMD4::MD4Digest ) * m_nList;
	*ppOutput = new BYTE[ *pnOutput ];
	CopyMemory( *ppOutput, m_pList, *pnOutput );

	return TRUE;
}

LPCVOID CED2K::GetRawPtr() const
{
	return (LPCVOID)m_pList;
}

//////////////////////////////////////////////////////////////////////
// CED2K decode from bytes

BOOL CED2K::FromBytes(BYTE* pOutput, DWORD nOutput, QWORD nSize)
{
	Clear();
	
    if ( pOutput == NULL || nOutput == 0 || ( nOutput % sizeof( CMD4::MD4Digest ) ) != 0 ) return FALSE;
	
	if ( nSize > 0 )
	{
        if ( nSize % ED2K_PART_SIZE == 0 && nSize ) 
			m_bNullBlock = true;

		QWORD nValidBlocks = ( nSize + ED2K_PART_SIZE - 1 ) / ED2K_PART_SIZE;
		if ( m_bNullBlock )
			nValidBlocks++;

		if ( nOutput / sizeof( CMD4::MD4Digest ) != nValidBlocks )
			return FALSE;
	}
	
    m_nList	= nOutput / sizeof( CMD4::MD4Digest );
    m_pList = new CMD4::MD4Digest[ m_nList ];
	
	CopyMemory( m_pList, pOutput, nOutput );

	if ( m_nList == 1 )
	{
		m_pRoot = *m_pList;
	}
	else
	{
		CMD4 pOverall;
        pOverall.Add( m_pList, sizeof( CMD4::MD4Digest ) * m_nList );
		pOverall.Finish();
		pOverall.GetHash( m_pRoot );
	}

	return TRUE;
}

BOOL CED2K::FromRoot(const Hashes::Ed2kHash& oHash)
{
	Clear();

	m_nList = 1;
    m_pList = new CMD4::MD4Digest[ m_nList ];
	
    std::copy( oHash.begin(), oHash.end(), &m_pRoot[ 0 ] );
    std::copy( oHash.begin(), oHash.end(), &m_pList[ 0 ][ 0 ] );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CED2K integrity checking

BOOL CED2K::CheckIntegrity()
{
	ASSERT( IsAvailable() );

	if ( m_nList == 1 )
	{
        return std::equal( &m_pRoot[ 0 ], &m_pRoot[ 4 ], &m_pList[ 0 ][ 0 ] );
	}
	else
	{
		CMD4 pOverall;
        CMD4::MD4Digest pRoot;
		
        pOverall.Add( m_pList, sizeof( CMD4::MD4Digest ) * m_nList );
		pOverall.Finish();
		pOverall.GetHash( pRoot );
		
		return std::equal( &m_pRoot[ 0 ], &m_pRoot[ 4 ], &pRoot[ 0 ] );
	}
}
