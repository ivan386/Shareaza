//
// ED2K.cpp
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
#include "ED2K.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CED2K construction

CED2K::CED2K()
{
	m_pList	= NULL;
	m_nList	= 0;
}

CED2K::~CED2K()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CED2K clear

void CED2K::Clear()
{
	ZeroMemory( &m_pRoot, sizeof(MD4) );
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
		
		ar.Write( &m_pRoot, sizeof(MD4) );
		if ( m_nList > 1 ) ar.Write( m_pList, sizeof(MD4) * m_nList );
	}
	else
	{
		Clear();
		
		ar >> m_nList;
		if ( m_nList == 0 ) return;
		
		ar.Read( &m_pRoot, sizeof(MD4) );
		
		m_pList = new MD4[ m_nList ];
		
		if ( m_nList > 1 )
		{
			ar.Read( m_pList, sizeof(MD4) * m_nList );
		}
		else if ( m_nList == 1 )
		{
			*m_pList = m_pRoot;
		}
	}
}

DWORD CED2K::GetSerialSize() const
{
	DWORD nSize = 4;
	if ( m_nList > 0 ) nSize += sizeof(MD4);
	if ( m_nList > 1 ) nSize += sizeof(MD4) * m_nList;
	return nSize;
}

//////////////////////////////////////////////////////////////////////
// CED2K root value

BOOL CED2K::GetRoot(MD4* pHash) const
{
	// if ( m_nList == 0 ) return FALSE;
	*pHash = m_pRoot;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CED2K file hashing - begin

void CED2K::BeginFile(QWORD nLength)
{
	ASSERT( ! IsAvailable() );
	
	m_nList	= (DWORD)( ( nLength + ED2K_PART_SIZE - 1 ) / ED2K_PART_SIZE );
	m_pList	= new MD4[ m_nList ];
	
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
			m_pSegment.GetHash( &m_pList[ m_nCurHash ] );
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
	
	if ( m_nCurHash < m_nList )
	{
		m_pSegment.Finish();
		m_pSegment.GetHash( &m_pList[ m_nCurHash++ ] );
		m_pSegment.Reset();
	}
	
	ASSERT( m_nCurHash == m_nList );
	
	if ( m_nList == 1 )
	{
		m_pRoot = *m_pList;
	}
	else if ( m_nList == 0)
	{
		m_pSegment.Finish();
		m_pSegment.GetHash( &m_pRoot );
	}
	else
	{
		CMD4 pOverall;
		pOverall.Add( m_pList, sizeof(MD4) * m_nList );
		pOverall.Finish();
		pOverall.GetHash( &m_pRoot );
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
	
	MD4 pMD4;
	m_pSegment.Finish();
	m_pSegment.GetHash( &pMD4 );
	
	return pMD4 == m_pList[ nBlock ];
}

//////////////////////////////////////////////////////////////////////
// CED2K encode to bytes

BOOL CED2K::ToBytes(BYTE** ppOutput, DWORD* pnOutput)
{
	if ( m_nList == 0 ) return FALSE;
	
	*pnOutput = sizeof(MD4) * m_nList;
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
	
	if ( pOutput == NULL || nOutput == 0 || ( nOutput % sizeof(MD4) ) != 0 ) return FALSE;
	
	if ( nSize > 0 )
	{
		if ( nOutput / sizeof(MD4) != ( nSize + ED2K_PART_SIZE - 1 ) / ED2K_PART_SIZE )
			return FALSE;
	}
	
	m_nList	= nOutput / sizeof(MD4);
	m_pList = new MD4[ m_nList ];
	
	CopyMemory( m_pList, pOutput, nOutput );
	
	if ( m_nList == 1 )
	{
		m_pRoot = *m_pList;
	}
	else
	{
		CMD4 pOverall;
		pOverall.Add( m_pList, sizeof(MD4) * m_nList );
		pOverall.Finish();
		pOverall.GetHash( &m_pRoot );
	}
	
	return TRUE;
}

BOOL CED2K::FromRoot(MD4* pHash)
{
	Clear();
	
	m_nList = 1;
	m_pList = new MD4[ m_nList ];
	
	m_pRoot = *m_pList = *pHash;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CED2K integrity checking

BOOL CED2K::CheckIntegrity()
{
	ASSERT( IsAvailable() );
	
	if ( m_nList == 1 )
	{
		return *m_pList == m_pRoot;
	}
	else
	{
		CMD4 pOverall;
		MD4 pRoot;
		
		pOverall.Add( m_pList, sizeof(MD4) * m_nList );
		pOverall.Finish();
		pOverall.GetHash( &pRoot );
		
		return pRoot == m_pRoot;
	}
}


//////////////////////////////////////////////////////////////////////
// CED2K convert to string

CString CED2K::HashToString(const MD4* pHash, BOOL bURN)
{
	CString str;

	str.Format( bURN ?
		_T("urn:ed2khash:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x") :
		_T("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"),
		pHash->n[0], pHash->n[1], pHash->n[2], pHash->n[3],
		pHash->n[4], pHash->n[5], pHash->n[6], pHash->n[7],
		pHash->n[8], pHash->n[9], pHash->n[10], pHash->n[11],
		pHash->n[12], pHash->n[13], pHash->n[14], pHash->n[15] );

	return str;
}

//////////////////////////////////////////////////////////////////////
// CED2K parse from a string

BOOL CED2K::HashFromString(LPCTSTR pszHash, MD4* pHash)
{
	if ( _tcslen( pszHash ) < 32 ) return FALSE;

	BYTE* pOut = (BYTE*)pHash;

	for ( int nPos = 16 ; nPos ; nPos--, pOut++ )
	{
		if ( *pszHash >= '0' && *pszHash <= '9' )
			*pOut = ( *pszHash - '0' ) << 4;
		else if ( *pszHash >= 'A' && *pszHash <= 'F' )
			*pOut = ( *pszHash - 'A' + 10 ) << 4;
		else if ( *pszHash >= 'a' && *pszHash <= 'f' )
			*pOut = ( *pszHash - 'a' + 10 ) << 4;
		pszHash++;
		if ( *pszHash >= '0' && *pszHash <= '9' )
			*pOut |= ( *pszHash - '0' );
		else if ( *pszHash >= 'A' && *pszHash <= 'F' )
			*pOut |= ( *pszHash - 'A' + 10 );
		else if ( *pszHash >= 'a' && *pszHash <= 'f' )
			*pOut |= ( *pszHash - 'a' + 10 );
		pszHash++;
	}

	return TRUE;
}

BOOL CED2K::HashFromURN(LPCTSTR pszHash, MD4* pHash)
{
	if ( pszHash == NULL ) return FALSE;

	int nLen = _tcslen( pszHash );

	if ( nLen >= 9 + 32 && _tcsnicmp( pszHash, _T("urn:ed2k:"), 9 ) == 0 )
	{
		return HashFromString( pszHash + 9, pHash );
	}
	else if ( nLen >= 5 + 32 && _tcsnicmp( pszHash, _T("ed2k:"), 5 ) == 0 )
	{
		return HashFromString( pszHash + 5, pHash );
	}
	else if ( nLen >= 13 + 32 && _tcsnicmp( pszHash, _T("urn:ed2khash:"), 13 ) == 0 )
	{
		return HashFromString( pszHash + 13, pHash );
	}
	else if ( nLen >= 9 + 32 && _tcsnicmp( pszHash, _T("ed2khash:"), 9 ) == 0 )
	{
		return HashFromString( pszHash + 9, pHash );
	}
	
	return FALSE;
}

