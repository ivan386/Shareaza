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
// #include "Shareaza.h"
#include "TigerTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define BLOCK_SIZE		1024
#define STACK_SIZE		64
#define TIGER_SIZE		24

#define NEW_TIGER_ALG	// New "more secure" padded node combination


//////////////////////////////////////////////////////////////////////
// CTigerTree construction
extern "C" void TigerTree_Tiger_p5(WORD64* str, WORD64* state);
extern "C" void TigerTree_Tiger_MMX(WORD64* str, WORD64* state);
extern "C" void TigerTree_Tiger_SSE2(WORD64* str, WORD64* state);
void (*pTiger)(WORD64*, WORD64*);

inline int GetCPUIDFlags()
{
	_asm{
		mov	eax, 1
		cpuid
		mov eax, edx
	}
}

CTigerTree::CTigerTree()
{
	m_nHeight		= 0;
	m_pNode			= NULL;
	m_nNodeCount	= 0;

	m_pStackBase	= NULL;
	m_pStackTop		= NULL;

	pTiger = &TigerTree_Tiger_p5;
//	if (GetCPUIDFlags() & 0x00800000) pTiger = &TigerTree_Tiger_MMX;
	if (GetCPUIDFlags() & 0x04000000) pTiger = &TigerTree_Tiger_SSE2;
}

CTigerTree::~CTigerTree()
{
	Clear();

	if ( m_pStackBase != NULL ) delete [] m_pStackBase;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree setup tree

void CTigerTree::SetupAndAllocate(DWORD nHeight, QWORD nLength)
{
	Clear();
	
	QWORD nCount = (DWORD)( nLength / BLOCK_SIZE );
	if ( nLength % BLOCK_SIZE ) nCount++;
	
	DWORD nActualHeight = 1;
	
	for ( DWORD nStep = 1 ; nStep < nCount ; nStep *= 2 ) nActualHeight++;
	
	m_nHeight = min( nActualHeight, nHeight );
	
	m_nBlockCount	= 1;
	m_nBlockPos		= 0;
	
	if ( nActualHeight > nHeight )
	{
		for ( nStep = nActualHeight - nHeight ; nStep ; nStep-- ) m_nBlockCount *= 2;
	}
	
	m_nNodeCount = 1;
	
	for ( nStep = m_nHeight ; nStep ; nStep-- ) m_nNodeCount *= 2;
	
	m_nNodeBase	= ( m_nNodeCount / 2 );
	m_nBaseUsed	= (DWORD)( nCount / m_nBlockCount );
	if ( nCount % m_nBlockCount ) m_nBaseUsed++;
	
	m_pNode		= new CTigerNode[ --m_nNodeCount ];
	m_nNodePos	= 0;
}

void CTigerTree::SetupParameters(QWORD nLength)
{
	QWORD nCount = nLength / BLOCK_SIZE;
	if ( nLength % BLOCK_SIZE ) nCount++;
	
	DWORD nActualHeight = 1;
	for ( DWORD nStep = 1 ; nStep < nCount ; nStep *= 2 ) nActualHeight++;
	
	m_nBlockCount	= 1;
	m_nBlockPos		= 0;
	
	if ( nActualHeight > m_nHeight )
	{
		for ( nStep = nActualHeight - m_nHeight ; nStep ; nStep-- ) m_nBlockCount *= 2;
	}
	
	m_nNodeCount = 1;
	for ( nStep = m_nHeight ; nStep ; nStep-- ) m_nNodeCount *= 2;
	
	m_nNodeBase = ( m_nNodeCount-- / 2 );
	
	m_nBaseUsed	= (DWORD)( nCount / m_nBlockCount );
	if ( nCount % m_nBlockCount ) m_nBaseUsed++;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree clear

void CTigerTree::Clear()
{
	if ( m_pNode != NULL ) delete [] m_pNode;
	
	m_nHeight		= 0;
	m_pNode			= NULL;
	m_nNodeCount	= 0;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree serialize

void CTigerTree::Serialize(CArchive& ar)
{
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
		for ( nStep = m_nHeight ; nStep ; nStep-- ) m_nNodeCount *= 2;
		m_nNodeCount --;
		
		m_pNode = pNode = new CTigerNode[ m_nNodeCount ];
		
		for ( nStep = m_nNodeCount ; nStep ; nStep--, pNode++ )
		{
			ar.Read( pNode->value, TIGER_SIZE );
			ar >> pNode->bValid;
		}
	}
}

DWORD CTigerTree::GetSerialSize() const
{
	return 4 + m_nNodeCount * ( TIGER_SIZE + 1 );
}

//////////////////////////////////////////////////////////////////////
// CTigerTree root output

BOOL CTigerTree::GetRoot(TIGEROOT* pTiger) const
{
	if ( m_pNode == NULL ) return FALSE;
	ASSERT( sizeof(TIGEROOT) == TIGER_SIZE );
	CopyMemory( pTiger, &m_pNode->value, TIGER_SIZE );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree string output

CString CTigerTree::RootToString() const
{
	CString str;
	if ( m_pNode != NULL ) str = m_pNode->ToString();
	return str;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree assume

void CTigerTree::Assume(CTigerTree* pSource)
{
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

void CTigerTree::BeginFile(DWORD nHeight, QWORD nLength)
{
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
	ASSERT( m_pNode != NULL );
	
	LPBYTE pBlock = (LPBYTE)pInput;
	
	while ( nLength > 0 )
	{
		DWORD nBlock = min( nLength, BLOCK_SIZE );
		
		Tiger( pBlock, (WORD64)nBlock, m_pStackTop->value );
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
	ASSERT( m_pNode != NULL );
	
	if ( m_pStackBase == NULL ) m_pStackBase = new CTigerNode[ STACK_SIZE ];
	m_pStackTop	= m_pStackBase;
	m_nBlockPos = 0;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree add data to a block test

void CTigerTree::AddToTest(LPCVOID pInput, DWORD nLength)
{
	ASSERT( m_pNode != NULL );
	
	LPBYTE pBlock = (LPBYTE)pInput;
	
	while ( nLength > 0 )
	{
		DWORD nBlock = min( nLength, BLOCK_SIZE );
		
		Tiger( pBlock, (WORD64)nBlock, m_pStackTop->value );
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
	ASSERT( nBlock < m_nBaseUsed );
	if ( nBlock >= m_nBaseUsed ) return FALSE;
	
	while ( m_pStackTop - 1 > m_pStackBase ) Collapse();
	
	CTigerNode* pNode = m_pNode + m_nNodeCount - m_nNodeBase + nBlock;
	
	if ( pNode->v1 != m_pStackBase->v1 ) return FALSE;
	if ( pNode->v2 != m_pStackBase->v2 ) return FALSE;
	if ( pNode->v3 != m_pStackBase->v3 ) return FALSE;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CTigerTree breadth-first serialize

BOOL CTigerTree::ToBytes(BYTE** pOutput, DWORD* pnOutput, DWORD nHeight)
{
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

BOOL CTigerTree::FromBytes(BYTE* pInput, DWORD nInput, DWORD nHeight, QWORD nLength)
{
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
	
	for ( nStep = 0 ; nStep < m_nNodeCount && nInput > 0 ; nStep++ )
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
	CString strOutput, strLine;

	DWORD nRowPos = 0, nRowCount = 1;
	m_nHeight = 0;
	
	for ( DWORD nStep = 0 ; nStep < m_nNodeCount ; nStep++ )
	{
		if ( m_pNode[ nStep ].bValid )
			strLine += CTigerNode::HashToString( (TIGEROOT*)m_pNode[ nStep ].value );
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

	strOutput += _T("\r\n");
	
	USES_CONVERSION;
	LPCSTR pszOutput = T2A(strOutput);
	pFile.Write( pszOutput, strlen(pszOutput) );
	pFile.Close();
#endif
}


//////////////////////////////////////////////////////////////////////
// CTigerNode to string

CString CTigerNode::ToString()
{
	return HashToString( (TIGEROOT*)&value );
}

CString CTigerNode::HashToString(const TIGEROOT* pInHash, BOOL bURN)
{
	static LPCTSTR pszBase64 = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ234567");
	
	CString strHash;
	LPTSTR pszHash	= strHash.GetBuffer( bURN ? 16 + 39 : 39 );
	
	if ( bURN )
	{
		*pszHash++ = 'u'; *pszHash++ = 'r'; *pszHash++ = 'n'; *pszHash++ = ':'; 
		*pszHash++ = 't'; *pszHash++ = 'r'; *pszHash++ = 'e'; *pszHash++ = 'e'; *pszHash++ = ':'; 
		*pszHash++ = 't'; *pszHash++ = 'i'; *pszHash++ = 'g'; *pszHash++ = 'e'; *pszHash++ = 'r'; *pszHash++ = '/'; *pszHash++ = ':'; 
	}
	
	LPBYTE pHash	= (LPBYTE)pInHash;
	BYTE pZero		= 0;
	int nShift		= 7;
	int nHash		= 0;
	
	for ( int nChar = 39 ; nChar ; nChar-- )
	{
		BYTE nBits = 0;
		
		for ( int nBit = 0 ; nBit < 5 ; nBit++ )
		{
			if ( nBit ) nBits <<= 1;
			nBits |= ( *pHash >> nShift ) & 1;
			
			if ( ! nShift-- )
			{
				nShift = 7;
				if ( ++nHash < TIGER_SIZE )
					pHash++;
				else
					pHash = &pZero;
			}
		}
		
		*pszHash++ = pszBase64[ nBits ];
	}
	
	strHash.ReleaseBuffer( bURN ? 16 + 39 : 39 );
	
	return strHash;
}

//////////////////////////////////////////////////////////////////////
// CTigerNode parse from string

BOOL CTigerNode::HashFromString(LPCTSTR pszHash, TIGEROOT* pTiger)
{
	if ( ! pszHash || _tcslen( pszHash ) < 39 ) return FALSE;

	LPBYTE pHash = (LPBYTE)pTiger;
	DWORD nBits	= 0;
	int nCount	= 0;

	for ( int nChars = 39 ; nChars-- ; pszHash++ )
	{
		if ( *pszHash >= 'A' && *pszHash <= 'Z' )
			nBits |= ( *pszHash - 'A' );
		else if ( *pszHash >= 'a' && *pszHash <= 'z' )
			nBits |= ( *pszHash - 'a' );
		else if ( *pszHash >= '2' && *pszHash <= '7' )
			nBits |= ( *pszHash - '2' + 26 );
		else
			return FALSE;
		
		nCount += 5;

		if ( nCount >= 8 )
		{
			*pHash++ = (BYTE)( nBits >> ( nCount - 8 ) );
			nCount -= 8;
		}

		nBits <<= 5;
	}

	return TRUE;
}

BOOL CTigerNode::HashFromURN(LPCTSTR pszHash, TIGEROOT* pTiger)
{
	if ( pszHash == NULL ) return FALSE;

	int nLen = _tcslen( pszHash );

	if ( nLen >= 16+39 && _tcsnicmp( pszHash, _T("urn:tree:tiger/:"), 16 ) == 0 )
	{
		return HashFromString( pszHash + 16, pTiger );
	}
	else if ( nLen >= 12+39 && _tcsnicmp( pszHash, _T("tree:tiger/:"), 12 ) == 0 )
	{
		return HashFromString( pszHash + 12, pTiger );
	}
	else if ( nLen >= 15+39 && _tcsnicmp( pszHash, _T("urn:tree:tiger:"), 15 ) == 0 )
	{
		return HashFromString( pszHash + 15, pTiger );
	}
	else if ( nLen >= 11+39 && _tcsnicmp( pszHash, _T("tree:tiger:"), 11 ) == 0 )
	{
		return HashFromString( pszHash + 11, pTiger );
	}
	else if ( nLen >= 85 && _tcsnicmp( pszHash, _T("urn:bitprint:"), 13 ) == 0 )
	{
		return HashFromString( pszHash + 46, pTiger );
	}
	else if ( nLen >= 81 && _tcsnicmp( pszHash, _T("bitprint:"), 9 ) == 0 )
	{
		return HashFromString( pszHash + 42, pTiger );
	}
	
	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// CTigerTree tiger hash implementation
//
// Portions created and released into the public domain by Eli Biham
//

void CTigerTree::Tiger(LPCVOID pInput, WORD64 nInput, WORD64* pOutput, WORD64* pInput1, WORD64* pInput2)
{
	register WORD64 i, j;
	BYTE pTemp[64];
	
	pOutput[0] = 0x0123456789ABCDEF;
	pOutput[1] = 0xFEDCBA9876543210;
	pOutput[2] = 0xF096A5B4C3B2E187;
	
	if ( pInput != NULL )
	{
#ifdef NEW_TIGER_ALG
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
			pTiger( (WORD64*)pTemp, pOutput );
			
			const WORD64* pWords = (const WORD64*)( (const BYTE*)pInput + 63 );
			
			for ( i = nInput - 63 ; i >= 64 ; i -= 64 )
			{
				pTiger( (WORD64*) pWords, pOutput );
				pWords += 8;
			}
			
			for ( j = 0 ; j < i ; j++ ) pTemp[j] = ((BYTE*)pWords)[j];
		}
		nInput++;
#else
		const WORD64* pWords = (const WORD64*)pInput;
		
		for ( i = nInput ; i >= 64 ; i -= 64 )
		{
			pTiger( pWords, pOutput );
			pWords += 8;
		}
		
		for ( j = 0 ; j < i ; j++ ) pTemp[j] = ((BYTE*)pWords)[j];
#endif
	}
	else if ( pInput1 != NULL && pInput2 != NULL )
	{
#ifdef NEW_TIGER_ALG
		pTemp[0] = 0x01;
		CopyMemory( pTemp + 1, pInput1, TIGER_SIZE );
		CopyMemory( pTemp + TIGER_SIZE + 1, pInput2, TIGER_SIZE );
		j = TIGER_SIZE * 2 + 1;
		nInput = j;
#else
		CopyMemory( pTemp, pInput1, TIGER_SIZE );
		CopyMemory( pTemp + TIGER_SIZE, pInput2, TIGER_SIZE );
		nInput = j = TIGER_SIZE * 2;
#endif
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
		pTiger( ((WORD64*)pTemp), pOutput );
		j = 0;
	}
	
	for ( ; j < 56 ; j++ ) pTemp[j] = 0;
	
	((WORD64*)(&(pTemp[56])))[0] = ((WORD64)nInput) << 3;
	
	pTiger( ((WORD64*)pTemp), pOutput );
}

#define PASSES 3

#define t1 (m_pTable)
#define t2 (m_pTable+256)
#define t3 (m_pTable+256*2)
#define t4 (m_pTable+256*3)

#define save_abc \
      aa = a; \
      bb = b; \
      cc = c;

#define round(a,b,c,x,mul) \
      c ^= x; \
      a -= t1[(BYTE)(c)] ^ \
           t2[(BYTE)(((DWORD)(c))>>(2*8))] ^ \
	   t3[(BYTE)((c)>>(4*8))] ^ \
           t4[(BYTE)(((DWORD)((c)>>(4*8)))>>(2*8))] ; \
      b += t4[(BYTE)(((DWORD)(c))>>(1*8))] ^ \
           t3[(BYTE)(((DWORD)(c))>>(3*8))] ^ \
	   t2[(BYTE)(((DWORD)((c)>>(4*8)))>>(1*8))] ^ \
           t1[(BYTE)(((DWORD)((c)>>(4*8)))>>(3*8))]; \
      b *= mul;

#define pass(a,b,c,mul) \
      round(a,b,c,x0,mul) \
      round(b,c,a,x1,mul) \
      round(c,a,b,x2,mul) \
      round(a,b,c,x3,mul) \
      round(b,c,a,x4,mul) \
      round(c,a,b,x5,mul) \
      round(a,b,c,x6,mul) \
      round(b,c,a,x7,mul)

#define key_schedule \
      x0 -= x7 ^ 0xA5A5A5A5A5A5A5A5; \
      x1 ^= x0; \
      x2 += x1; \
      x3 -= x2 ^ ((~x1)<<19); \
      x4 ^= x3; \
      x5 += x4; \
      x6 -= x5 ^ ((~x4)>>23); \
      x7 ^= x6; \
      x0 += x7; \
      x1 -= x0 ^ ((~x7)<<19); \
      x2 ^= x1; \
      x3 += x2; \
      x4 -= x3 ^ ((~x2)>>23); \
      x5 ^= x4; \
      x6 += x5; \
      x7 -= x6 ^ 0x0123456789ABCDEF;

#define feedforward \
      a ^= aa; \
      b -= bb; \
      c += cc;

#define compress \
      save_abc \
      for(pass_no=0; pass_no<PASSES; pass_no++) { \
        if(pass_no != 0) {key_schedule} \
	pass(a,b,c,(pass_no==0?5:pass_no==1?7:9)); \
	tmpa=a; a=c; c=b; b=tmpa;} \
      feedforward

//void CTigerTree::Tiger(WORD64* str, WORD64* state)
//{
//	TigerTree_Tiger_p5(str,state);
/*	register WORD64 x0, x1, x2, x3, x4, x5, x6, x7;
	register WORD64 a, b, c, tmpa;
	WORD64 aa, bb, cc;
	// register DWORD i;
	int pass_no;

	a = state[0];
	b = state[1];
	c = state[2];

	x0=str[0]; x1=str[1]; x2=str[2]; x3=str[3];
	x4=str[4]; x5=str[5]; x6=str[6]; x7=str[7];

	compress;

	state[0] = a;
	state[1] = b;
	state[2] = c; */
//}

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
	
	pNode->v1		= m_pStackBase->v1;
	pNode->v2		= m_pStackBase->v2;
	pNode->v3		= m_pStackBase->v3;
	pNode->bValid	= TRUE;
	
	m_nBlockPos		= 0;
	m_pStackTop		= m_pStackBase;
}
