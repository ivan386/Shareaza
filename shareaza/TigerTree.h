//
// TigerTree.h
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

#if !defined(AFX_TIGERTREE_H__59910156_59A2_454F_A0FE_0A66B7D37218__INCLUDED_)
#define AFX_TIGERTREE_H__59910156_59A2_454F_A0FE_0A66B7D37218__INCLUDED_

#pragma once

#include "Hashes.h"

class CTigerTree : public CHashTiger
{
// Attributes
private:
	DWORD		m_nHeight;
	DWORD		m_nNodeCount;
	CManagedTiger* m_pNode;
	CHashTiger*	m_pStackBase;
	CHashTiger*	m_pStackTop;
	DWORD		m_nNodeBase;
	DWORD		m_nNodePos;
	DWORD		m_nBaseUsed;
	DWORD		m_nBlockCount;
	DWORD		m_nBlockPos;
public:
	inline	CTigerTree();
	inline	~CTigerTree();
	inline	void	Clear();
			void	SetupAndAllocate(const DWORD nHeight, const QWORD nLength);
			void	SetupParameters(const QWORD nLength);
	inline	void	SerializeStore(CArchive& ar, const DWORD nVersion);
	inline	void	SerializeLoad(CArchive& ar, const DWORD nVersion);
	inline	void	Assume(CTigerTree* pSource);
			void	BeginFile(const DWORD nHeight, const QWORD nLength);
			void	AddToFile(LPCVOID pInput, DWORD nLength);
			BOOL	FinishFile();
			void	BeginBlockTest();
			void	AddToTest(LPCVOID pInput, DWORD nLength);
			BOOL	FinishBlockTest(DWORD nBlock);
			BOOL	ToBytes(LPBYTE &pOutput, DWORD &nOutput, DWORD nHeight = 0);
			BOOL	FromBytes(LPBYTE pOutput, DWORD nOutput, DWORD nHeight, QWORD nLength);
			BOOL	CheckIntegrity();
#ifdef _DEBUG
	void	Dump();
#endif
	inline	BOOL	IsAvailable() const { return m_pNode != NULL; }
	inline	DWORD	GetHeight() const { return m_nHeight; }
	inline	DWORD	GetBlockLength() const { return m_nBlockCount << 10; }
	inline	DWORD	GetBlockCount() const { return m_nBaseUsed; }
	inline	DWORD	GetSerialSize() const { return 4 + m_nNodeCount * ( TIGER_HASH_SIZE + 1 ); }
// Implementation
private:
	inline	void	BlocksToNode();
	inline	void	Collapse();
	typedef	void	(__stdcall *tpTiger_Var)	(CHashTiger* state, LPVOID str, DWORD nLength);
	typedef	void	(__stdcall *tpTiger_1)		(CHashTiger* state, LPVOID str);
	typedef	void	(__stdcall *tpTiger_2)		(CHashTiger* state1, LPVOID str1, CHashTiger* state2, LPVOID str2);
	typedef	void	(__stdcall *tpTiger_3)		(CHashTiger* state1, LPVOID str1, CHashTiger* state2, LPVOID str2, CHashTiger* state3, LPVOID str3);
	typedef	void	(__stdcall *tpTiger_Node)	(CHashTiger* state, CHashTiger* hash1, CHashTiger* hash2);
	static	tpTiger_Var		pTiger_Var;
	static	tpTiger_1		pTiger_1;
	static	tpTiger_2		pTiger_2;
	static	tpTiger_3		pTiger_3;
	static	tpTiger_Node	pTiger_Node;
public:
	static	void	Init();
};

inline CTigerTree::CTigerTree()
{
	m_nNodeCount = m_nHeight = 0;
	m_pStackTop = m_pStackBase = m_pNode = NULL;
}

inline CTigerTree::~CTigerTree()
{
	Clear();
	if ( m_pStackBase ) delete [] m_pStackBase;
}

inline void CTigerTree::Clear()
{
	if ( m_pNode )
	{
		delete [] m_pNode;
		m_pNode = NULL;
	}
	m_nNodeCount = m_nHeight = 0;
}

inline void CTigerTree::SerializeStore(CArchive& ar, const DWORD nVersion)
{
	ASSERT( ar.IsStoring() );
	DWORD nNode = 0;
	BYTE bValid;
	ar << m_nHeight;
	ASSERT( ( m_pNode != NULL ) || ( m_nHeight == 0 ) );
	ASSERT( ( m_nHeight == 0 ) == ( m_nNodeCount == 0 ) );
	if ( m_nNodeCount ) do
	{
		( (CHashTiger)m_pNode[ nNode ] ).SerializeStore( ar, nVersion );
		bValid = m_pNode[ nNode ].IsValid();			// always storing entire tree - is that necessary?
		ar << bValid;
	}
	while ( ++nNode < m_nNodeCount );
}

inline void CTigerTree::SerializeLoad(CArchive& ar, const DWORD nVersion)
{
	ASSERT( ar.IsLoading() );
	DWORD nNode = 0;
	BYTE bValid;
	Clear();
	ar >> m_nHeight;
	if ( m_nHeight != 0 )
	{
		m_nNodeCount = ( 1 << m_nHeight ) - 1;
		m_pNode = new CManagedTiger[ m_nNodeCount ];
		do
		{
			ASSERT( ! m_pNode[ nNode ].IsValid() );
		//	( (CHashTiger)m_pNode[ nNode ] ).SerializeLoad( ar, nVersion );
			ar.Read( &m_pNode[ nNode ].m_b, 24 );
			ar >> bValid;
			if ( bValid ) m_pNode[ nNode ].SetValid();
		}
		while ( ++nNode < m_nNodeCount );
		if ( ! CheckIntegrity() )
		{
			Clear();
			AfxThrowArchiveException( CArchiveException::generic, ar.m_strFileName );
		}
	}
}

inline void CTigerTree::Assume(CTigerTree* pSource)
{
	Clear();
	if ( pSource->m_pNode == NULL ) return;
	m_nHeight = pSource->m_nHeight;
	m_nNodeCount = pSource->m_nNodeCount;
	m_pNode = pSource->m_pNode;
	pSource->m_nNodeCount = pSource->m_nHeight = 0;
	pSource->m_pNode = NULL;
}

#endif // !defined(AFX_TIGERTREE_H__59910156_59A2_454F_A0FE_0A66B7D37218__INCLUDED_)
