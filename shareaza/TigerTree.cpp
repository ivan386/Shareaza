//
// TigerTree.cpp
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
#include "TigerTree.h"
#include "asm\common.inc"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define BLOCK_SIZE		1024
#define BLOCK_SHIFT		10				// 1024 = 2^10
#define STACK_SIZE		64

extern "C" void __stdcall TigerTree_Tiger_p5_Var(CHashTiger* state, LPVOID str, DWORD nLength);
extern "C" void __stdcall TigerTree_Tiger_p5_1(CHashTiger* state, LPVOID str);
extern "C" void __stdcall TigerTree_Tiger_p5_2(CHashTiger* state1, LPVOID str1, CHashTiger* state2, LPVOID str2);
extern "C" void __stdcall TigerTree_Tiger_p5_3(CHashTiger* state1, LPVOID str1, CHashTiger* state2, LPVOID str2, CHashTiger* state3, LPVOID str3);
extern "C" void __stdcall TigerTree_Tiger_p5_Node(CHashTiger* state, CHashTiger* hash1, CHashTiger* hash2);
extern "C" void __stdcall TigerTree_Tiger_SSE2_Var(CHashTiger* state, LPVOID str, DWORD nLength);
extern "C" void __stdcall TigerTree_Tiger_SSE2_1(CHashTiger* state, LPVOID str);
extern "C" void __stdcall TigerTree_Tiger_SSE2_2(CHashTiger* state1, LPVOID str1, CHashTiger* state2, LPVOID str2);
extern "C" void __stdcall TigerTree_Tiger_SSE2_3(CHashTiger* state1, LPVOID str1, CHashTiger* state2, LPVOID str2, CHashTiger* state3, LPVOID str3);
extern "C" void __stdcall TigerTree_Tiger_SSE2_Node(CHashTiger* state, CHashTiger* hash1, CHashTiger* hash2);

CTigerTree::tpTiger_Var		CTigerTree::pTiger_Var	= &TigerTree_Tiger_p5_Var;
CTigerTree::tpTiger_1		CTigerTree::pTiger_1	= &TigerTree_Tiger_p5_1;
CTigerTree::tpTiger_2		CTigerTree::pTiger_2	= &TigerTree_Tiger_p5_2;
CTigerTree::tpTiger_3		CTigerTree::pTiger_3	= &TigerTree_Tiger_p5_3;
CTigerTree::tpTiger_Node	CTigerTree::pTiger_Node	= &TigerTree_Tiger_p5_Node;

//////////////////////////////////////////////////////////////////////
// CTigerTree setup tree

void CTigerTree::SetupAndAllocate(const DWORD nHeight, const QWORD nLength)
{
	Clear();
	QWORD nCount = ( nLength + BLOCK_SIZE - 1 ) >> BLOCK_SHIFT;
	DWORD nActualHeight = 1;
	QWORD nStep;
	if ( ( nStep = 1 ) < nCount ) do
	{
		nActualHeight++;
	}
	while ( ( nStep <<= 1 ) < nCount );
	if ( nActualHeight > nHeight )
	{
		m_nBlockCount = 1 << ( nActualHeight - nHeight );
		m_nHeight = nHeight;
		m_nBaseUsed = (DWORD)( nCount >> ( nActualHeight - nHeight ) );
	}
	else
	{
		m_nBlockCount = 1;
		m_nHeight = nActualHeight;
		m_nBaseUsed = (DWORD)nCount;
	}
	m_nNodeCount = 1 << m_nHeight;
	m_nBlockPos = 0;
	m_nNodeBase	= m_nNodeCount-- >> 1;
	if ( nCount & ( m_nBlockCount - 1 ) ) m_nBaseUsed++;
	m_pNode = new CManagedTiger[ m_nNodeCount ];
	m_nNodePos = 0;
}

void CTigerTree::SetupParameters(const QWORD nLength)
{
	QWORD nCount = ( nLength + BLOCK_SIZE - 1 ) >> BLOCK_SHIFT;
	DWORD nActualHeight = 1;
	QWORD nStep;
	if ( ( nStep = 1 ) < nCount ) do
	{
		nActualHeight++;
	}
	while ( ( nStep <<= 1 ) < nCount );
	m_nBlockPos		= 0;
	if ( nActualHeight > m_nHeight )
	{
		m_nBlockCount = 1 << ( nActualHeight - m_nHeight );
		m_nBaseUsed = (DWORD)( nCount >> ( nActualHeight - m_nHeight ) );
	}
	else
	{
		m_nBlockCount = 1;
		m_nBaseUsed = (DWORD)nCount;
	}
	m_nNodeCount = 1 << m_nHeight;
	m_nNodeBase = m_nNodeCount-- >> 1;
	if ( nCount & ( m_nBlockCount - 1 ) ) m_nBaseUsed++;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree create from file

void CTigerTree::BeginFile(const DWORD nHeight, const QWORD nLength)
{
	ASSERT( ! IsAvailable() );
	SetupAndAllocate( nHeight, nLength );
	if ( m_pStackBase == NULL ) m_pStackBase = new CHashTiger[ STACK_SIZE ];
	m_pStackTop	= m_pStackBase;
	m_nBlockPos = 0;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree convert a block sequence to a node

inline void CTigerTree::BlocksToNode()
{
	if ( m_pStackTop != m_pStackBase )
	{
		while ( --m_pStackTop > m_pStackBase ) pTiger_Node( m_pStackTop - 1, m_pStackTop - 1, m_pStackTop );
		m_pNode[ m_nNodeBase - 1 + m_nNodePos++ ] = *( m_pStackTop = m_pStackBase );
		m_nBlockPos = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// CTigerTree collapse stack

inline void CTigerTree::Collapse()
{
	DWORD nCollapse;
	if ( ! ( ( nCollapse = ++m_nBlockPos ) & 1 ) ) do
	{
		m_pStackTop --;
		pTiger_Node( m_pStackTop - 1, m_pStackTop - 1, m_pStackTop );
	}
	while ( ! ( ( nCollapse >>= 1 ) & 1 ) );
}

//////////////////////////////////////////////////////////////////////
// CTigerTree add data to the file

void CTigerTree::AddToFile(LPCVOID pInput, DWORD nLength)
{
	ASSERT( m_pNode != NULL );
	
	LPBYTE pBlock = (LPBYTE)pInput;
	CHashTiger Node2, Node3;
	while ( nLength > 0 )
	{
		if ( nLength >= 3*BLOCK_SIZE )
		{
			pTiger_3( m_pStackTop++, (QWORD*)pBlock, &Node2, (QWORD*)(pBlock+BLOCK_SIZE), &Node3, (QWORD*)(pBlock+2*BLOCK_SIZE) );
			Collapse();
			if ( m_nBlockPos >= m_nBlockCount ) BlocksToNode();
			pBlock += BLOCK_SIZE;
			nLength -= BLOCK_SIZE;
			*m_pStackTop++ = Node2;
			Collapse();
			if ( m_nBlockPos >= m_nBlockCount ) BlocksToNode();
			pBlock += BLOCK_SIZE;
			nLength -= BLOCK_SIZE;
			*m_pStackTop++ = Node3;
			Collapse();
			if ( m_nBlockPos >= m_nBlockCount ) BlocksToNode();
			pBlock += BLOCK_SIZE;
			nLength -= BLOCK_SIZE;
		}
		else if ( nLength >= 2*BLOCK_SIZE )
		{
			pTiger_2( m_pStackTop++, (QWORD*)pBlock, &Node2, (QWORD*)(pBlock+BLOCK_SIZE) );
			Collapse();
			if ( m_nBlockPos >= m_nBlockCount ) BlocksToNode();
			pBlock += BLOCK_SIZE;
			nLength -= BLOCK_SIZE;
			*m_pStackTop++ = Node2;
			Collapse();
			if ( m_nBlockPos >= m_nBlockCount ) BlocksToNode();
			pBlock += BLOCK_SIZE;
			nLength -= BLOCK_SIZE;
		}
		else if ( nLength >= BLOCK_SIZE )
		{
			pTiger_1( m_pStackTop++, (QWORD*)pBlock );
			Collapse();
			if ( m_nBlockPos >= m_nBlockCount ) BlocksToNode();
			pBlock += BLOCK_SIZE;
			nLength -= BLOCK_SIZE;
		}
		else
		{
			pTiger_Var( m_pStackTop++, (QWORD*)pBlock, nLength );
			Collapse();
			if ( m_nBlockPos >= m_nBlockCount ) BlocksToNode();
			pBlock += nLength;
			nLength = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CTigerTree finish file

BOOL CTigerTree::FinishFile()
{
	DWORD nIn, nOut, nCombine;
	if ( m_pStackTop == NULL ) return FALSE;
	if ( m_nBaseUsed == 0 )
	{
		pTiger_Var( m_pStackTop++, NULL, 0 );
		m_nBlockPos++;
	}
	BlocksToNode();
	if ( m_nNodePos > m_nNodeBase ) return FALSE;
	delete [] m_pStackBase;
	m_pStackTop = m_pStackBase = NULL;
	if ( nCombine = m_nNodeBase - 1 ) do
	{
		nIn = nCombine;
		nOut = nCombine >> 1;
		do
		{
			if ( m_pNode[ nIn ].IsValid() )
			{
				if ( m_pNode[ nIn + 1 ].IsValid() )
				{
					pTiger_Node( &m_pNode[ nOut ], &m_pNode[ nIn ], &m_pNode[ nIn + 1 ] );
					m_pNode[ nOut ].SetValid();
				}
				else
				{
					m_pNode[ nOut ] = m_pNode[ nIn ];
				}
			}
			nIn += 2;
		}
		while ( ++nOut < nCombine );
	}
	while ( nCombine >>= 1 );
	CHashTiger::operator = ( m_pNode[ 0 ] );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree begin a block test

void CTigerTree::BeginBlockTest()
{
	ASSERT( m_pNode != NULL );
	if ( m_pStackBase == NULL ) m_pStackBase = new CHashTiger[ STACK_SIZE ];
	m_pStackTop	= m_pStackBase;
	m_nBlockPos = 0;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree add data to a block test

void CTigerTree::AddToTest(LPCVOID pInput, DWORD nLength)
{
	ASSERT( m_pNode != NULL );
	LPBYTE pBlock = (LPBYTE)pInput;
	CHashTiger Node2, Node3;
	while ( nLength > 0 )
	{
		if ( nLength >= 3 * BLOCK_SIZE )
		{
			pTiger_3( m_pStackTop++, pBlock, &Node2, &pBlock[ BLOCK_SIZE ], &Node3, &pBlock[ 2 * BLOCK_SIZE ] );
			Collapse();
			*m_pStackTop++ = Node2;
			Collapse();
			*m_pStackTop++ = Node3;
			Collapse();
			pBlock += 3 * BLOCK_SIZE;
			nLength -= 3 * BLOCK_SIZE;
		}
		else if ( nLength >= 2 * BLOCK_SIZE )
		{
			pTiger_2( m_pStackTop++, pBlock, &Node2, &pBlock[ BLOCK_SIZE ] );
			Collapse();
			*m_pStackTop++ = Node2;
			Collapse();
			pBlock += 2 * BLOCK_SIZE;
			nLength -= 2 * BLOCK_SIZE;
		}
		else if ( nLength >= BLOCK_SIZE )
		{
			pTiger_1( m_pStackTop++, pBlock );
			Collapse();
			pBlock += BLOCK_SIZE;
			nLength -= BLOCK_SIZE;
		}
		else
		{
			pTiger_Var( m_pStackTop++, pBlock, nLength );
			Collapse();
			nLength = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CTigerTree block test finish and compare

BOOL CTigerTree::FinishBlockTest(DWORD nBlock)
{
	ASSERT( nBlock < m_nBaseUsed );
	if ( nBlock >= m_nBaseUsed ) return FALSE;
	while ( --m_pStackTop > m_pStackBase ) pTiger_Node( m_pStackTop - 1, m_pStackTop - 1, m_pStackTop );
	return m_pNode[ m_nNodeBase - 1 + nBlock ] == *m_pStackBase;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree breadth-first serialize

BOOL CTigerTree::ToBytes(LPBYTE &pOutput, DWORD &nOutput, DWORD nHeight)
{
	if ( m_pNode == NULL ) return FALSE;
	if ( nHeight < 1 || nHeight > m_nHeight ) nHeight = m_nHeight;
	DWORD nNode = 0, nOut = 0, nNodeCount = ( 1 << nHeight ) - 1;
	if ( m_nNodeCount < nNodeCount ) nNodeCount = m_nNodeCount;
	pOutput = new BYTE[ nOutput = nNodeCount * TIGER_HASH_SIZE ];
	if ( nNodeCount ) do
	{
		if ( m_pNode[ nNode ].IsValid() )
		{
			CopyMemory( &pOutput[ nOut ], &m_pNode[ nNode ].m_b, TIGER_HASH_SIZE );
			nOut += TIGER_HASH_SIZE;
		}
		else
		{
			nOutput -= TIGER_HASH_SIZE;
		}
	}
	while ( ++nNode < nNodeCount );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree breadth-first serialize

BOOL CTigerTree::FromBytes(LPBYTE pInput, DWORD nInput, DWORD nHeight, QWORD nLength)
{
	SetupAndAllocate( nHeight, nLength );
	DWORD nStep, nCombine, nIn, nOut;
	for ( nStep = 0 ; nStep < m_nBaseUsed ; nStep++ )
	{
		m_pNode[ m_nNodeBase - 1 + nStep ].SetValid();
	}
	if ( nCombine = m_nNodeBase - 1 ) do
	{
		nIn = nCombine;
		nOut = nCombine >> 1;
		do
		{
			if ( m_pNode[ nIn ].IsValid() ) m_pNode[ nOut ].SetValid();
			nIn += 2;
		}
		while ( ++nOut < nCombine );
	}
	while ( nCombine >>= 1 );
	nIn = 0;
	DWORD nBlocks = nInput / TIGER_HASH_SIZE;
	DWORD nRowPos = 0, nRowCount = 1;
	m_nHeight = 0;
	for ( nStep = 0 ; nStep < m_nNodeCount && nBlocks; nStep++ )
	{
		if ( m_pNode[ nStep ].IsValid() )
		{
			CopyMemory( &m_pNode[ nStep ], &pInput[ nIn ], TIGER_HASH_SIZE );
			nIn += TIGER_HASH_SIZE;
			nBlocks--;
		}
		if ( nRowPos == 0 ) m_nHeight++;
		if ( ++nRowPos == nRowCount )
		{
			nRowCount <<= 1;
			nRowPos = 0;
		}
	}
	if ( m_nHeight == 0 || m_nHeight > nHeight )
	{
		Clear();
		return FALSE;
	}
	SetupParameters( nLength );
	CHashTiger::operator = ( m_pNode[ 0 ] );
	if ( CheckIntegrity() ) return TRUE;
	Clear();
	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// CTigerTree integrity check

BOOL CTigerTree::CheckIntegrity()
{
	if ( m_pNode == NULL ) return FALSE;
	m_nNodeBase = ( m_nNodeCount + 1 ) >> 1;
	DWORD nCombine, nIn, nOut;
	CHashTiger oTemp;
	if ( nCombine = m_nNodeBase - 1 ) do
	{
		nIn = nCombine;
		if ( ! ( nOut = nCombine >> 1 ) && ! m_pNode[ nOut ].IsValid() ) return FALSE;;
		do
		{
			if ( m_pNode[ nIn ].IsValid() )
			{
				if ( m_pNode[ nIn + 1 ].IsValid() )
				{
					pTiger_Node( &oTemp, &m_pNode[ nIn ], &m_pNode[ nIn + 1 ] );
					if ( ! ( oTemp == m_pNode[ nOut ] ) ) return FALSE;
				}
				else
				{
					if ( ! ( m_pNode[ nIn ] == m_pNode[ nOut ] ) ) return FALSE;
				}
			}
			nIn += 2;
		}
		while ( ++nOut < nCombine );
	}
	while ( nCombine >>= 1 );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree dump to file

#ifdef _DEBUG
void CTigerTree::Dump()
{
	CString strOutput, strLine;
	DWORD nRowPos = 0, nRowCount = 1;
	m_nHeight = 0;
	for ( DWORD nStep = 0 ; nStep < m_nNodeCount ; nStep++ )
	{
		if ( m_pNode[ nStep ].IsValid() )
			strLine += m_pNode[ nStep ].ToString();
		else
			strLine += _T("_______________________________________");
		strLine += ' ';
		if ( ++nRowPos == nRowCount )
		{
			strOutput += strLine + _T("\r\n");
			strLine.Empty();
			m_nHeight ++;
			nRowCount <<= 1;
			nRowPos = 0;
		}
	}
	CFile pFile;
	if ( pFile.Open( _T("\\Shareaza.log"), CFile::modeReadWrite ) )
	{
		pFile.Seek( 0, CFile::end );
	}
	else
	{
		if ( ! pFile.Open( _T("\\Shareaza.log"), CFile::modeWrite|CFile::modeCreate ) ) return;
	}
	strOutput += _T("\r\n");
	USES_CONVERSION;
	LPCSTR pszOutput = T2A(strOutput);
	pFile.Write( pszOutput, strlen(pszOutput) );
	pFile.Close();
}
#endif

//////////////////////////////////////////////////////////////////////
// CTigerTree initialization

void CTigerTree::Init()
{
	if ( SupportsSSE2() )
	{
		pTiger_Var	= &TigerTree_Tiger_SSE2_Var;
		pTiger_1	= &TigerTree_Tiger_SSE2_1;
		pTiger_2	= &TigerTree_Tiger_SSE2_2;
		pTiger_3	= &TigerTree_Tiger_SSE2_3;
		pTiger_Node	= &TigerTree_Tiger_SSE2_Node;
	}
	else
	{
		pTiger_Var	= &TigerTree_Tiger_p5_Var;
		pTiger_1	= &TigerTree_Tiger_p5_1;
		pTiger_2	= &TigerTree_Tiger_p5_2;
		pTiger_3	= &TigerTree_Tiger_p5_3;
		pTiger_Node	= &TigerTree_Tiger_p5_Node;
	}
}
