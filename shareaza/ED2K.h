//
// ED2K.h
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

#if !defined(AFX_ED2K_H__0ED688AE_E4F5_49C6_8EC8_5C80EFA6EF6C__INCLUDED_)
#define AFX_ED2K_H__0ED688AE_E4F5_49C6_8EC8_5C80EFA6EF6C__INCLUDED_

#pragma once

#include "Hashes.h"
#include "MD4.h"
#include "asm/common.inc"

class CED2K : public CHashED2K
{
private:
	CHashMD4	*m_pList;
	DWORD		m_nList;
	CMD4		m_oSegment;
	DWORD		m_nCurHash;
	DWORD		m_nCurByte;
public:
	inline	CED2K();
	inline	~CED2K();
	inline	void	Clear();
	inline	void	SerializeStore(CArchive& ar, const DWORD nVersion);
	inline	void	SerializeLoad(CArchive& ar, const DWORD nVersion);
	inline	void	BeginFile(const QWORD nLength);
	inline	void	AddToFile(LPCVOID pInput, DWORD nLength);
	inline	void	FinishFile();
	inline	void	Add1(LPCVOID pData);				// add one full Block
	inline	void	Add2(LPCVOID pData1, CED2K*, LPCVOID pData2);
	inline	void	Add3(LPCVOID pData1, CED2K*, LPCVOID pData2, CED2K*, LPCVOID pData3);
	inline	void	Add4(LPCVOID pData1, CED2K*, LPCVOID pData2, CED2K*, LPCVOID pData3, CED2K*, LPCVOID pData4);
	inline	void	Add5(LPCVOID pData1, CED2K*, LPCVOID pData2, CED2K*, LPCVOID pData3, CED2K*, LPCVOID pData4, CED2K*, LPCVOID pData5);
	inline	void	Add6(LPCVOID pData1, CED2K*, LPCVOID pData2, CED2K*, LPCVOID pData3, CED2K*, LPCVOID pData4, CED2K*, LPCVOID pData5, CED2K*, LPCVOID pData6);
	inline	void	BeginBlockTest();
	inline	void	AddToTest(LPCVOID pInput, DWORD nLength);
	inline	BOOL	FinishBlockTest(DWORD nBlock);
	inline	void	ToBytes(LPBYTE &pOutput, DWORD &nOutput) const;
	inline	BOOL	FromBytes(LPBYTE pOutput, DWORD nOutput, QWORD nSize = 0);
	inline	void	FromRoot(const CHashED2K &oHash);
	inline	BOOL	CheckIntegrity();
	inline	BOOL	IsAvailable() const;
	inline	DWORD	GetBlockCount() const;
	inline	DWORD	GetSerialSize() const;
	inline	DWORD	GetHashsetSize() const;
private:
	inline	void	FinishBlock();
	inline	void	LibraryFinishBlock();
};

#define ED2K_PART_SIZE	9728000	// 1024*9500

inline CED2K::CED2K()
{
	m_pList	= NULL;
	m_nList	= 0;
}

inline CED2K::~CED2K()
{
	Clear();
}

inline void CED2K::Clear()
{
	if ( m_nList )
	{
		ASSERT( m_pList );
		delete [] m_pList;
		m_pList = NULL;
		m_nList = 0;
	}
	else ASSERT( m_pList == NULL );
}

inline BOOL CED2K::IsAvailable() const
{
	return m_pList != NULL;
}

inline DWORD CED2K::GetBlockCount() const
{
	return m_nList;
}

inline DWORD CED2K::GetSerialSize() const
{
	return m_nList > 1 ? 4 + ED2K_HASH_SIZE + m_nList * ED2K_HASH_SIZE : 4 + m_nList * ED2K_HASH_SIZE;
}

inline DWORD CED2K::GetHashsetSize() const
{
	return m_nList * ED2K_HASH_SIZE;
}

inline void CED2K::SerializeStore(CArchive& ar, const DWORD nVersion)
{
	ASSERT( ar.IsStoring() );
	DWORD nCount = 0;
	ar << m_nList;
	if ( m_nList )
	{
		CHashED2K::SerializeStore( ar, nVersion );
		if ( m_nList > 1 ) do
		{
			m_pList[ nCount ].SerializeStore( ar, nVersion );
		}
		while ( ++nCount < m_nList );
	}
}

inline void CED2K::SerializeLoad(CArchive& ar, const DWORD nVersion)
{
	ASSERT( ar.IsLoading() );
	DWORD nCount = 0;
	Clear();
	ar >> m_nList;
	if ( m_nList )
	{
		CHashED2K::SerializeLoad( ar, nVersion );
		m_pList = new CHashMD4[ m_nList ];
		if ( m_nList == 1 ) m_pList[ 0 ] = *this; else do
		{
			m_pList[ nCount ].SerializeLoad( ar, nVersion );
		}
		while ( ++nCount < m_nList );
	/*	if ( ! CheckIntegrity() )
		{
			Clear();
			AfxThrowArchiveException( CArchiveException::generic, ar.m_strFileName );
		}*/
	}
}

inline void CED2K::BeginFile(const QWORD nLength)
{
	ASSERT( ! IsAvailable() );
	m_nList	= (DWORD)( ( nLength + ED2K_PART_SIZE - 1 ) / ED2K_PART_SIZE );
	if ( m_nList == 0 ) m_nList++;
	m_pList	= new CHashMD4[ m_nList ];
	m_oSegment.Reset();
	m_nCurHash = 0;
	m_nCurByte = 0;
}

inline void CED2K::FinishBlock()
{
	ASSERT( m_nCurByte <= ED2K_PART_SIZE );
	if ( m_nCurByte >= ED2K_PART_SIZE )
	{
		ASSERT( m_nCurHash < m_nList );
		m_oSegment.Finish();
		m_pList[ m_nCurHash ] = m_oSegment;
		m_oSegment.Reset();
		m_nCurHash++;
		m_nCurByte = 0;
	}
}

inline void CED2K::LibraryFinishBlock()
{
	if ( ( m_nCurByte += LIBRARY_BUILDER_BLOCK_SIZE ) >= ED2K_PART_SIZE )
	{
		ASSERT( m_nCurByte <= ED2K_PART_SIZE );
		ASSERT( m_nCurHash < m_nList );
		m_oSegment.Finish();
		m_pList[ m_nCurHash ] = m_oSegment;
		m_oSegment.Reset();
		m_nCurHash++;
		m_nCurByte = 0;
	}
}

inline void CED2K::AddToFile(LPCVOID pInput, DWORD nLength)
{
	DWORD nToProcess;
	if ( nLength == 0 ) return;
	ASSERT( IsAvailable() );
	ASSERT( m_nCurHash < m_nList );
	ASSERT( m_nCurByte < ED2K_PART_SIZE );
	LPBYTE pBytes = (LPBYTE)pInput;
	while ( nLength > 0 )
	{
		nToProcess = min( ED2K_PART_SIZE - m_nCurByte, nLength );
		m_oSegment.Add( pBytes, nToProcess );
		m_nCurByte += nToProcess;
		nLength -= nToProcess;
		pBytes += nToProcess;
		FinishBlock();
	}
}

inline void CED2K::Add1(LPCVOID pData)
{
	CMD4::pAdd1( &m_oSegment, pData );
	LibraryFinishBlock();
}

inline void CED2K::Add2(LPCVOID pData1, CED2K* pED2K2, LPCVOID pData2)
{
	CMD4::pAdd2( &m_oSegment, pData1, &pED2K2->m_oSegment, pData2 );
	LibraryFinishBlock();
	pED2K2->LibraryFinishBlock();
}

inline void CED2K::Add3(LPCVOID pData1, CED2K* pED2K2, LPCVOID pData2, CED2K* pED2K3, LPCVOID pData3)
{
	CMD4::pAdd3( &m_oSegment, pData1, &pED2K2->m_oSegment, pData2, &pED2K3->m_oSegment, pData3 );
	LibraryFinishBlock();
	pED2K2->LibraryFinishBlock();
	pED2K3->LibraryFinishBlock();
}

inline void CED2K::Add4(LPCVOID pData1, CED2K* pED2K2, LPCVOID pData2, CED2K* pED2K3, LPCVOID pData3, CED2K* pED2K4, LPCVOID pData4)
{
	CMD4::pAdd4( &m_oSegment, pData1, &pED2K2->m_oSegment, pData2, &pED2K3->m_oSegment, pData3, &pED2K4->m_oSegment, pData4);
	LibraryFinishBlock();
	pED2K2->LibraryFinishBlock();
	pED2K3->LibraryFinishBlock();
	pED2K4->LibraryFinishBlock();
}

inline void CED2K::Add5(LPCVOID pData1, CED2K* pED2K2, LPCVOID pData2, CED2K* pED2K3, LPCVOID pData3, CED2K* pED2K4, LPCVOID pData4, CED2K* pED2K5, LPCVOID pData5)
{
	CMD4::pAdd5( &m_oSegment, pData1, &pED2K2->m_oSegment, pData2, &pED2K3->m_oSegment, pData3, &pED2K4->m_oSegment, pData4, &pED2K5->m_oSegment, pData5);
	LibraryFinishBlock();
	pED2K2->LibraryFinishBlock();
	pED2K3->LibraryFinishBlock();
	pED2K4->LibraryFinishBlock();
	pED2K5->LibraryFinishBlock();
}

inline void CED2K::Add6(LPCVOID pData1, CED2K* pED2K2, LPCVOID pData2, CED2K* pED2K3, LPCVOID pData3, CED2K* pED2K4, LPCVOID pData4, CED2K* pED2K5, LPCVOID pData5, CED2K* pED2K6, LPCVOID pData6)
{
	CMD4::pAdd6( &m_oSegment, pData1, &pED2K2->m_oSegment, pData2, &pED2K3->m_oSegment, pData3, &pED2K4->m_oSegment, pData4, &pED2K5->m_oSegment, pData5, &pED2K6->m_oSegment, pData6);
	LibraryFinishBlock();
	pED2K2->LibraryFinishBlock();
	pED2K3->LibraryFinishBlock();
	pED2K4->LibraryFinishBlock();
	pED2K5->LibraryFinishBlock();
	pED2K6->LibraryFinishBlock();
}

inline void CED2K::FinishFile()
{
	ASSERT( IsAvailable() );
	ASSERT( m_nCurHash <= m_nList );
	ASSERT( m_nCurByte < ED2K_PART_SIZE );
	if ( m_nCurHash < m_nList )
	{
		m_oSegment.Finish();
		m_pList[ m_nCurHash++ ] = m_oSegment;
		m_oSegment.Reset();
	}
	ASSERT( m_nCurHash == m_nList );
	if ( m_nList > 1 )
	{
		m_oSegment.Add( m_pList, m_nList * MD4_HASH_SIZE );
		m_oSegment.Finish();
		CHashMD4::operator = ( m_pList[ 0 ] );
	}
	else
	{
		CHashMD4::operator = ( m_pList[ 0 ] );
	}
}

inline void CED2K::BeginBlockTest()
{
	ASSERT( IsAvailable() );
	m_oSegment.Reset();
	m_nCurByte = 0;
}

inline void CED2K::AddToTest(LPCVOID pInput, DWORD nLength)
{
	if ( nLength == 0 ) return;
	ASSERT( IsAvailable() );
	ASSERT( m_nCurByte + nLength <= ED2K_PART_SIZE );
	m_oSegment.Add( pInput, nLength );
	m_nCurByte += nLength;
}

inline BOOL CED2K::FinishBlockTest(DWORD nBlock)
{
	ASSERT( IsAvailable() );
	if ( nBlock >= m_nList ) return FALSE;
	m_oSegment.Finish();
	return m_oSegment == m_pList[ nBlock ];
}

inline void CED2K::ToBytes(LPBYTE &pOutput, DWORD &nOutput) const
{
	if ( ! pOutput ) pOutput = new BYTE[ nOutput = m_nList * MD4_HASH_SIZE ];
	CopyMemory( pOutput, m_pList, nOutput );
}

inline BOOL CED2K::FromBytes(LPBYTE pOutput, DWORD nOutput, QWORD nSize)
{
	Clear();
	if ( pOutput == NULL || nOutput == 0 || ( nOutput % MD4_HASH_SIZE ) ) return FALSE;
	if ( nSize == 0 )
	{
		if ( nOutput != MD4_HASH_SIZE ) return FALSE;
		m_nList = 1;
	}
	else
	{
		if ( nOutput / MD4_HASH_SIZE != ( nSize + ED2K_PART_SIZE - 1 ) / ED2K_PART_SIZE ) return FALSE;
		m_nList = nOutput / MD4_HASH_SIZE;
	}
	m_pList = new CHashMD4[ m_nList	];
	CopyMemory( m_pList, pOutput, nOutput );
	if ( m_nList > 1 )
	{
		m_oSegment.Reset();
		m_oSegment.Add( m_pList, m_nList * MD4_HASH_SIZE );
		m_oSegment.Finish();
		CHashMD4::operator = ( m_oSegment );
	}
	else
	{
		CHashMD4::operator = ( m_pList[ 0 ] );
	}
	return TRUE;
}

inline void CED2K::FromRoot(const CHashED2K &oHash)
{
	Clear();
	m_pList = new CHashMD4[ m_nList = 1 ];
	m_pList[ 0 ] = oHash;
	CHashMD4::operator = ( oHash );
}

inline BOOL CED2K::CheckIntegrity()
{
	ASSERT( IsAvailable() );
	if ( m_nList > 1 )
	{
		m_oSegment.Reset();
		m_oSegment.Add( m_pList, m_nList * MD4_HASH_SIZE );
		m_oSegment.Finish();
		return CHashMD4::operator == ( m_oSegment );
	}
	return CHashMD4::operator == ( m_pList[ 0 ] );
}

#endif // !defined(AFX_ED2K_H__0ED688AE_E4F5_49C6_8EC8_5C80EFA6EF6C__INCLUDED_)
