//
// TigerTree.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "TigerTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const unsigned BLOCK_SIZE = 1024u;
const unsigned STACK_SIZE = 64u;
const unsigned TIGER_SIZE = 24u;

typedef union
{
	BYTE	n[24];
	BYTE	b[24];
	uint64	w[3];
} TIGEROOT;


//////////////////////////////////////////////////////////////////////
// CTigerTree construction

#ifdef SHAREAZA_USE_ASM

namespace
{

	extern "C" void __stdcall TigerTree_Tiger_p5(const void* str, uint64* state);
	extern "C" void __stdcall TigerTree_Tiger_SSE2(const void* str, uint64* state);
	void (__stdcall* const Tiger)(const void*, uint64*) = Machine::SupportsSSE2()
		? &TigerTree_Tiger_SSE2
		: &TigerTree_Tiger_p5;

} // namespace

#else

#include "TigerBoxes.h"

//namespace
//{

	template< int round > inline void tigerRound(uint64& a, uint64& b, uint64& c, const uint64* x)
	{
		static const uint32 mul = ( round / 8 ) * 2 + 5;
		c ^= x[ round % 8 ];
		a -=  CTigerTree_m_pTable[ 0 ][ static_cast< std::size_t >( c >>  0 ) & 0xff ]
			^ CTigerTree_m_pTable[ 1 ][ static_cast< std::size_t >( c >> 16 ) & 0xff ]
			^ CTigerTree_m_pTable[ 2 ][ static_cast< std::size_t >( c >> 32 ) & 0xff ]
			^ CTigerTree_m_pTable[ 3 ][ static_cast< std::size_t >( c >> 48 ) & 0xff ];
		b +=  CTigerTree_m_pTable[ 3 ][ static_cast< std::size_t >( c >>  8 ) & 0xff ]
			^ CTigerTree_m_pTable[ 2 ][ static_cast< std::size_t >( c >> 24 ) & 0xff ]
			^ CTigerTree_m_pTable[ 1 ][ static_cast< std::size_t >( c >> 40 ) & 0xff ]
			^ CTigerTree_m_pTable[ 0 ][ static_cast< std::size_t >( c >> 56 ) & 0xff ];
		b *= mul;
	}

	inline void keySchedule(uint64* x)
	{
		x[ 0 ] -= x[ 7 ] ^ 0xA5A5A5A5A5A5A5A5;
		x[ 1 ] ^= x[ 0 ];
		x[ 2 ] += x[ 1 ];
		x[ 3 ] -= x[ 2 ] ^ ( ~x[ 1 ] << 19 );
		x[ 4 ] ^= x[ 3 ];
		x[ 5 ] += x[ 4 ];
		x[ 6 ] -= x[ 5 ] ^ ( ~x[ 4 ] >> 23 );
		x[ 7 ] ^= x[ 6 ];
		x[ 0 ] += x[ 7 ];
		x[ 1 ] -= x[ 0 ] ^ ( ~x[ 7 ] << 19 );
		x[ 2 ] ^= x[ 1 ];
		x[ 3 ] += x[ 2 ];
		x[ 4 ] -= x[ 3 ] ^ ( ~x[ 2 ] >> 23 );
		x[ 5 ] ^= x[ 4 ];
		x[ 6 ] += x[ 5 ];
		x[ 7 ] -= x[ 6 ] ^ 0x0123456789ABCDEF;
	}

	void Tiger(const uint64* str, uint64* state)
	{
		uint64 x[ 8 ] =
		{
			str[ 0 ], str[ 1 ], str[ 2 ], str[ 3 ], str[ 4 ], str[ 5 ], str[ 6 ], str[ 7 ]
		};

		uint64 a = state[ 0 ];
		uint64 b = state[ 1 ];
		uint64 c = state[ 2 ];

		tigerRound<  0 >( a, b, c, x );
		tigerRound<  1 >( b, c, a, x );
		tigerRound<  2 >( c, a, b, x );
		tigerRound<  3 >( a, b, c, x );
		tigerRound<  4 >( b, c, a, x );
		tigerRound<  5 >( c, a, b, x );
		tigerRound<  6 >( a, b, c, x );
		tigerRound<  7 >( b, c, a, x );
		keySchedule( x );
		tigerRound<  8 >( c, a, b, x );
		tigerRound<  9 >( a, b, c, x );
		tigerRound< 10 >( b, c, a, x );
		tigerRound< 11 >( c, a, b, x );
		tigerRound< 12 >( a, b, c, x );
		tigerRound< 13 >( b, c, a, x );
		tigerRound< 14 >( c, a, b, x );
		tigerRound< 15 >( a, b, c, x );
		keySchedule( x );
		tigerRound< 16 >( b, c, a, x );
		tigerRound< 17 >( c, a, b, x );
		tigerRound< 18 >( a, b, c, x );
		tigerRound< 19 >( b, c, a, x );
		tigerRound< 20 >( c, a, b, x );
		tigerRound< 21 >( a, b, c, x );
		tigerRound< 22 >( b, c, a, x );
		tigerRound< 23 >( c, a, b, x );

		state[ 0 ] ^= a;
		state[ 1 ]  = b - state[ 1 ];
		state[ 2 ] += c;
	}

//} // namespace

#endif

CTigerTree::CTigerTree() :
	m_nHeight		( 0 )
,	m_pNode			( NULL )
,	m_nNodeCount	( 0 )

,	m_pStackBase	( NULL )
,	m_pStackTop		( NULL )
{
}

CTigerTree::~CTigerTree()
{
	Clear();

	if ( m_pStackBase != NULL ) delete [] m_pStackBase;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree setup tree

void CTigerTree::SetupAndAllocate(DWORD nHeight, uint64 nLength)
{
	CQuickLock oLock( m_pSection );

	Clear();

	uint64 nCount = (DWORD)( nLength / BLOCK_SIZE );
	if ( nLength % BLOCK_SIZE ) nCount++;

	DWORD nActualHeight = 1;

	for ( DWORD nStep = 1 ; nStep < nCount ; nStep *= 2 ) nActualHeight++;

	m_nHeight = min( nActualHeight, nHeight );

	m_nBlockCount	= 1;
	m_nBlockPos		= 0;

	if ( nActualHeight > nHeight )
	{
		for ( DWORD nStep = nActualHeight - nHeight ; nStep ; nStep-- ) m_nBlockCount *= 2;
	}

	m_nNodeCount = 1;

	for ( DWORD nStep = m_nHeight ; nStep ; nStep-- ) m_nNodeCount *= 2;

	m_nNodeBase	= ( m_nNodeCount / 2 );
	m_nBaseUsed	= (DWORD)( nCount / m_nBlockCount );
	if ( nCount % m_nBlockCount ) m_nBaseUsed++;

	m_pNode		= new CTigerNode[ --m_nNodeCount ];
	m_nNodePos	= 0;
}

void CTigerTree::SetupParameters(uint64 nLength)
{
	CQuickLock oLock( m_pSection );

	uint64 nCount = nLength / BLOCK_SIZE;
	if ( nLength % BLOCK_SIZE ) nCount++;

	DWORD nActualHeight = 1;
	for ( DWORD nStep = 1 ; nStep < nCount ; nStep *= 2 ) nActualHeight++;

	m_nBlockCount	= 1;
	m_nBlockPos		= 0;

	if ( nActualHeight > m_nHeight )
	{
		for ( DWORD nStep = nActualHeight - m_nHeight ; nStep ; nStep-- ) m_nBlockCount *= 2;
	}

	m_nNodeCount = 1;
	for ( DWORD nStep = m_nHeight ; nStep ; nStep-- ) m_nNodeCount *= 2;

	m_nNodeBase = ( m_nNodeCount-- / 2 );

	m_nBaseUsed	= (DWORD)( nCount / m_nBlockCount );
	if ( nCount % m_nBlockCount ) m_nBaseUsed++;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree clear

void CTigerTree::Clear()
{
	CQuickLock oLock( m_pSection );

	if ( m_pNode != NULL ) delete [] m_pNode;

	m_nHeight		= 0;
	m_pNode			= NULL;
	m_nNodeCount	= 0;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree serialize

void CTigerTree::Serialize(CArchive& ar)
{
	CQuickLock oLock( m_pSection );

	CTigerNode* pNode;
	DWORD nStep;

	if ( ar.IsStoring() )
	{
		ar << m_nHeight;

		if ( m_nHeight == 0 ) return;
		ASSERT( m_pNode != NULL );

		pNode = m_pNode;

		for ( nStep = m_nNodeCount ; nStep ; nStep--, pNode++ )
		{
			ar.Write( pNode->value, TIGER_SIZE );
			ar << pNode->bValid;
		}
	}
	else
	{
		Clear();

		ar >> m_nHeight;
		if ( m_nHeight == 0 ) return;

		m_nNodeCount = 1;
		for ( nStep = m_nHeight ; nStep ; nStep-- )
		{
			m_nNodeCount *= 2;
			if ( m_nNodeCount > 0xFFFFFFFF / 2 )
				return;
		}
		m_nNodeCount --;

		m_pNode = pNode = new CTigerNode[ m_nNodeCount ];

		for ( nStep = m_nNodeCount ; nStep ; nStep--, pNode++ )
		{
			ReadArchive( ar, pNode->value, TIGER_SIZE );
			ar >> pNode->bValid;
		}
	}
}

DWORD CTigerTree::GetSerialSize() const
{
	CQuickLock oLock( m_pSection );

	return 4 + m_nNodeCount * ( TIGER_SIZE + 1 );
}

//////////////////////////////////////////////////////////////////////
// CTigerTree root output

BOOL CTigerTree::GetRoot(Hashes::TigerHash& oTiger) const
{
	CQuickLock oLock( m_pSection );

	if ( m_pNode == NULL ) return FALSE;
	std::copy( &m_pNode->value[ 0 ], &m_pNode->value[ 3 ], oTiger.begin() );
	oTiger.validate();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree assume

void CTigerTree::Assume(CTigerTree* pSource)
{
	CQuickLock oLock( m_pSection );

	Clear();
	if ( pSource->m_pNode == NULL ) return;

	m_nHeight		= pSource->m_nHeight;
	m_nNodeCount	= pSource->m_nNodeCount;
	m_pNode			= pSource->m_pNode;

	pSource->m_nHeight		= 0;
	pSource->m_nNodeCount	= 0;
	pSource->m_pNode		= NULL;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree create from file

void CTigerTree::BeginFile(DWORD nHeight, uint64 nLength)
{
	CQuickLock oLock( m_pSection );

	ASSERT( ! IsAvailable() );

	SetupAndAllocate( nHeight, nLength );

	if ( m_pStackBase == NULL ) m_pStackBase = new CTigerNode[ STACK_SIZE ];
	m_pStackTop	= m_pStackBase;
	m_nBlockPos = 0;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree add data to the file

void CTigerTree::AddToFile(LPCVOID pInput, DWORD nLength)
{
	CQuickLock oLock( m_pSection );

	ASSERT( m_pNode != NULL );

	LPBYTE pBlock = (LPBYTE)pInput;

	while ( nLength > 0 )
	{
		DWORD nBlock = min( nLength, BLOCK_SIZE );

		Tiger( pBlock, (uint64)nBlock, m_pStackTop->value );
		m_pStackTop ++;

		DWORD nCollapse = ++m_nBlockPos;

		while ( ! ( nCollapse & 1 ) )
		{
			Collapse();
			nCollapse >>= 1;
		}

		if ( m_nBlockPos >= m_nBlockCount )
		{
			BlocksToNode();
		}

		pBlock += nBlock;
		nLength -= nBlock;
	}
}

//////////////////////////////////////////////////////////////////////
// CTigerTree finish file

BOOL CTigerTree::FinishFile()
{
	CQuickLock oLock( m_pSection );

	if ( m_pStackTop == NULL ) return FALSE;
	if ( m_nBaseUsed == 0 )
	{
		Tiger( this, 0, (m_pStackTop++)->value );
		m_nBlockPos++;
	}

	BlocksToNode();

	if ( m_nNodePos > m_nNodeBase ) return FALSE;

	if ( m_pStackBase != NULL ) delete [] m_pStackBase;

	m_pStackBase	= NULL;
	m_pStackTop		= NULL;

	CTigerNode* pBase = m_pNode + m_nNodeCount - m_nNodeBase;

	for ( DWORD nCombine = m_nNodeBase ; nCombine > 1 ; nCombine /= 2 )
	{
		CTigerNode* pIn		= pBase;
		CTigerNode* pOut	= pBase - nCombine / 2;

		for ( DWORD nIterate = nCombine / 2 ; nIterate ; nIterate--, pIn += 2, pOut++ )
		{
			if ( pIn[0].bValid && pIn[1].bValid )
			{
				Tiger( NULL, TIGER_SIZE * 2, pOut->value, pIn[0].value, pIn[1].value );
				pOut->bValid = TRUE;
			}
			else if ( pIn[0].bValid )
			{
				*pOut = *pIn;
			}
		}

		pBase -= nCombine / 2;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree begin a block test

void CTigerTree::BeginBlockTest()
{
	CQuickLock oLock( m_pSection );

	ASSERT( m_pNode != NULL );

	if ( m_pStackBase == NULL ) m_pStackBase = new CTigerNode[ STACK_SIZE ];
	m_pStackTop	= m_pStackBase;
	m_nBlockPos = 0;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree add data to a block test

void CTigerTree::AddToTest(LPCVOID pInput, DWORD nLength)
{
	CQuickLock oLock( m_pSection );

	ASSERT( m_pNode != NULL );

	LPBYTE pBlock = (LPBYTE)pInput;

	while ( nLength > 0 )
	{
		DWORD nBlock = min( nLength, BLOCK_SIZE );

		Tiger( pBlock, (uint64)nBlock, m_pStackTop->value );
		m_pStackTop ++;

		ASSERT( m_nBlockPos < m_nBlockCount );

		DWORD nCollapse = ++m_nBlockPos;

		while ( ! ( nCollapse & 1 ) )
		{
			Collapse();
			nCollapse >>= 1;
		}

		pBlock += nBlock;
		nLength -= nBlock;
	}
}

//////////////////////////////////////////////////////////////////////
// CTigerTree block test finish and compare

BOOL CTigerTree::FinishBlockTest(DWORD nBlock)
{
	CQuickLock oLock( m_pSection );

	ASSERT( nBlock < m_nBaseUsed );
	if ( nBlock >= m_nBaseUsed ) return FALSE;

	while ( m_pStackTop - 1 > m_pStackBase ) Collapse();

	CTigerNode* pNode = m_pNode + m_nNodeCount - m_nNodeBase + nBlock;

	if ( pNode->value[ 0 ] != m_pStackBase->value[ 0 ] ) return FALSE;
	if ( pNode->value[ 1 ] != m_pStackBase->value[ 1 ] ) return FALSE;
	if ( pNode->value[ 2 ] != m_pStackBase->value[ 2 ] ) return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree breadth-first serialize

BOOL CTigerTree::ToBytes(BYTE** pOutput, DWORD* pnOutput, DWORD nHeight)
{
	CQuickLock oLock( m_pSection );

	if ( m_pNode == NULL ) return FALSE;

	if ( nHeight < 1 || nHeight > m_nHeight ) nHeight = m_nHeight;

	DWORD nNodeCount = 1;
	while ( nHeight-- ) nNodeCount *= 2;
	nNodeCount --;

	nNodeCount	= min( nNodeCount, m_nNodeCount );
	*pnOutput	= nNodeCount * TIGER_SIZE;
	*pOutput	= new BYTE[ *pnOutput ];

	CTigerNode* pNode = m_pNode;
	BYTE* pOut = *pOutput;

	for ( DWORD nNode = 0 ; nNode < nNodeCount ; nNode++, pNode++ )
	{
		if ( pNode->bValid )
		{
			CopyMemory( pOut, pNode->value, TIGER_SIZE );
			pOut += TIGER_SIZE;
		}
		else
		{
			*pnOutput -= TIGER_SIZE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree breadth-first serialize

BOOL CTigerTree::FromBytes(const BYTE* pInput, DWORD nInput, DWORD nHeight, uint64 nLength)
{
	CQuickLock oLock( m_pSection );

	SetupAndAllocate( nHeight, nLength );

	CTigerNode* pBase = m_pNode + m_nNodeCount - m_nNodeBase;

	for ( DWORD nStep = m_nBaseUsed ; nStep ; nStep-- )
	{
		pBase[ nStep - 1 ].bValid = TRUE;
	}

	for ( DWORD nCombine = m_nNodeBase ; nCombine > 1 ; nCombine /= 2 )
	{
		CTigerNode* pIn		= pBase;
		CTigerNode* pOut	= pBase - nCombine / 2;

		for ( DWORD nIterate = nCombine / 2 ; nIterate ; nIterate--, pIn += 2, pOut++ )
		{
			if ( pIn[0].bValid ) pOut->bValid = TRUE;
		}

		pBase -= nCombine / 2;
	}

	TIGEROOT* pTiger = (TIGEROOT*)pInput;
	nInput /= TIGER_SIZE;

	DWORD nRowPos = 0, nRowCount = 1;
	m_nHeight = 0;

	for ( DWORD nStep = 0 ; nStep < m_nNodeCount && nInput > 0 ; nStep++ )
	{
		if ( m_pNode[ nStep ].bValid )
		{
			CopyMemory( m_pNode[ nStep ].value, pTiger->w, TIGER_SIZE );
			pTiger ++; nInput --;
		}

		if ( nRowPos == 0 ) m_nHeight ++;

		if ( ++nRowPos == nRowCount )
		{
			nRowCount *= 2;
			nRowPos = 0;
		}
	}

	if ( m_nHeight == 0 || m_nHeight > nHeight )
	{
		Clear();
		return FALSE;
	}

	SetupParameters( nLength );

	if ( ! CheckIntegrity() )
	{
		Clear();
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree integrity check

BOOL CTigerTree::CheckIntegrity()
{
	CQuickLock oLock( m_pSection );

	if ( m_pNode == NULL ) return FALSE;

	m_nNodeBase = ( m_nNodeCount + 1 ) / 2;
	CTigerNode* pBase = m_pNode + m_nNodeCount - m_nNodeBase;

	for ( DWORD nCombine = m_nNodeBase ; nCombine > 1 ; nCombine /= 2 )
	{
		CTigerNode* pIn		= pBase;
		CTigerNode* pOut	= pBase - nCombine / 2;

		if ( nCombine == 2 && pOut->bValid == FALSE ) return FALSE;

		for ( DWORD nIterate = nCombine / 2 ; nIterate ; nIterate--, pIn += 2, pOut++ )
		{
			if ( pIn[0].bValid && pIn[1].bValid )
			{
				TIGEROOT pTemp;
				Tiger( NULL, TIGER_SIZE * 2, pTemp.w, pIn[0].value, pIn[1].value );
				if ( memcmp( pTemp.w, pOut->value, TIGER_SIZE ) ) return FALSE;
			}
			else if ( pIn[0].bValid )
			{
				if ( memcmp( pIn[0].value, pOut->value, TIGER_SIZE ) ) return FALSE;
			}
			else
			{
				// pOut is bValid=FALSE
			}
		}

		pBase -= nCombine / 2;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree dump to file

void CTigerTree::Dump()
{
#ifdef _DEBUG
	CQuickLock oLock( m_pSection );

	CString strOutput, strLine;

	DWORD nRowPos = 0, nRowCount = 1;
	m_nHeight = 0;

	for ( DWORD nStep = 0 ; nStep < m_nNodeCount ; nStep++ )
	{
		if ( m_pNode[ nStep ].bValid )
		{
			Hashes::TigerHash oTiger;
			std::copy( &m_pNode[ nStep ].value[ 0 ], &m_pNode[ nStep ].value[ 3 ], oTiger.begin() );
			oTiger.validate();
			strLine += oTiger.toString();
		}
		else
			strLine += _T("_______________________________________");
		strLine += ' ';

		if ( ++nRowPos == nRowCount )
		{
			strOutput += strLine;
			strOutput += _T("\r\n");
			strLine.Empty();

			m_nHeight ++;
			nRowCount *= 2;
			nRowPos = 0;
		}
	}

	theApp.LogMessage( strOutput );
#endif
}

//////////////////////////////////////////////////////////////////////
// CTigerTree tiger hash implementation
//
// Portions created and released into the public domain by Eli Biham
//

void CTigerTree::Tiger(LPCVOID pInput, uint64 nInput, uint64* pOutput, uint64* pInput1, uint64* pInput2)
{
	register uint64 i, j = 0;
	BYTE pTemp[64];

	pOutput[0] = 0x0123456789ABCDEF;
	pOutput[1] = 0xFEDCBA9876543210;
	pOutput[2] = 0xF096A5B4C3B2E187;

	if ( pInput != NULL )
	{
		if ( nInput < 63 )
		{
			pTemp[0] = 0x00;
			const BYTE* pBytes = (const BYTE*)pInput;
			for ( j = 1 ; j < nInput + 1 ; j++ ) pTemp[j] = *pBytes++;
		}
		else
		{
			pTemp[0] = 0x00;
			CopyMemory( pTemp + 1, pInput, 63 );
			::Tiger( (uint64*)pTemp, pOutput );

			const uint64* pWords = (const uint64*)( (const BYTE*)pInput + 63 );

			for ( i = nInput - 63 ; i >= 64 ; i -= 64 )
			{
				::Tiger( (uint64*) pWords, pOutput );
				pWords += 8;
			}

			for ( j = 0 ; j < i ; j++ ) pTemp[j] = ((BYTE*)pWords)[j];
		}
		nInput++;
	}
	else if ( pInput1 != NULL && pInput2 != NULL )
	{
		pTemp[0] = 0x01;
		CopyMemory( pTemp + 1, pInput1, TIGER_SIZE );
		CopyMemory( pTemp + TIGER_SIZE + 1, pInput2, TIGER_SIZE );
		j = TIGER_SIZE * 2 + 1;
		nInput = j;
	}
	else
	{
		ASSERT( FALSE );
	}

	pTemp[j++] = 0x01;

	for ( ; j & 7 ; j++ ) pTemp[j] = 0;

	if ( j > 56 )
	{
		for ( ; j < 64 ; j++ ) pTemp[j] = 0;
		::Tiger( ((uint64*)pTemp), pOutput );
		j = 0;
	}

	for ( ; j < 56 ; j++ ) pTemp[j] = 0;

	((uint64*)(&(pTemp[56])))[0] = ((uint64)nInput) << 3;

	::Tiger( ((uint64*)pTemp), pOutput );
}

//////////////////////////////////////////////////////////////////////
// CTigerTree collapse stack

void CTigerTree::Collapse()
{
	ASSERT( m_pStackTop - m_pStackBase >= 2 );

	Tiger( NULL, TIGER_SIZE * 2, m_pStackTop->value, m_pStackTop[-2].value, m_pStackTop[-1].value );

	m_pStackTop -= 2;
	m_pStackTop[0] = m_pStackTop[2];
	m_pStackTop ++;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree convert a block sequence to a node

void CTigerTree::BlocksToNode()
{
	if ( m_pStackTop == m_pStackBase ) return;

	while ( m_pStackTop - 1 > m_pStackBase ) Collapse();

	CTigerNode* pNode = m_pNode + m_nNodeCount - m_nNodeBase + m_nNodePos++;

	pNode->value[ 0 ]		= m_pStackBase->value[ 0 ];
	pNode->value[ 1 ]		= m_pStackBase->value[ 1 ];
	pNode->value[ 2 ]		= m_pStackBase->value[ 2 ];
	pNode->bValid	= TRUE;

	m_nBlockPos		= 0;
	m_pStackTop		= m_pStackBase;
}
