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

typedef unsigned __int64 WORD64;

class CTigerNode;


class CTigerTree
{
// Construction
public:
	CTigerTree();
	~CTigerTree();
	
	void (*pTiger)(WORD64*, WORD64*);

// Operations
public:
	void	SetupAndAllocate(DWORD nHeight, QWORD nLength);
	void	SetupParameters(QWORD nLength);
	void	Clear();
	void	Serialize(CArchive& ar);
	DWORD	GetSerialSize() const;
public:
	BOOL	GetRoot(TIGEROOT* pTiger) const;
	CString	RootToString() const;
	void	Assume(CTigerTree* pSource);
public:
	void	BeginFile(DWORD nHeight, QWORD nLength);
	void	AddToFile(LPCVOID pInput, DWORD nLength);
	BOOL	FinishFile();
public:
	void	BeginBlockTest();
	void	AddToTest(LPCVOID pInput, DWORD nLength);
	BOOL	FinishBlockTest(DWORD nBlock);
public:
	BOOL	ToBytes(BYTE** pOutput, DWORD* pnOutput, DWORD nHeight = 0);
	BOOL	FromBytes(BYTE* pOutput, DWORD nOutput, DWORD nHeight, QWORD nLength);
	BOOL	CheckIntegrity();
	void	Dump();

// Inlines
public:
	inline BOOL		IsAvailable() const { return m_pNode != NULL; }
	inline DWORD	GetHeight() const { return m_nHeight; }
	inline DWORD	GetBlockLength() const { return 1024 * m_nBlockCount; }
	inline DWORD	GetBlockCount() const { return m_nBaseUsed; }

// Attributes
protected:
	DWORD		m_nHeight;
	CTigerNode*	m_pNode;
	DWORD		m_nNodeCount;

// Processing Data
protected:
	DWORD		m_nNodeBase;
	DWORD		m_nNodePos;
	DWORD		m_nBaseUsed;
	DWORD		m_nBlockCount;
	DWORD		m_nBlockPos;
	CTigerNode*	m_pStackBase;
	CTigerNode*	m_pStackTop;
protected:
	static WORD64 m_pTable[4*256];

// Implementation
protected:
	inline void	Collapse();
	inline void BlocksToNode();
	inline void	Tiger(LPCVOID pInput, WORD64 nInput, WORD64* pOutput, WORD64* pInput1 = NULL, WORD64* pInput2 = NULL);
//	inline void	Tiger(WORD64* str, WORD64* state);
};


class CTigerNode
{
// Construction
public:
	inline CTigerNode()
	{
		v1 = v2 = v3 = 0;
		bValid = FALSE;
	}

// Attributes
public:
	union
	{
		WORD64	value[3];
		struct
		{
			WORD64	v1;
			WORD64	v2;
			WORD64	v3;
		};
	};

	BYTE bValid;

// Operations
public:
	CString			ToString();
	static CString	HashToString(const TIGEROOT* pTiger, BOOL bURN = FALSE);
	static BOOL		HashFromString(LPCTSTR pszHash, TIGEROOT* pTiger);
	static BOOL		HashFromURN(LPCTSTR pszHash, TIGEROOT* pTiger);

};

inline bool operator==(const TIGEROOT& tigera, const TIGEROOT& tigerb)
{
    return memcmp( &tigera, &tigerb, 24 ) == 0;
}

inline bool operator!=(const TIGEROOT& tigera, const TIGEROOT& tigerb)
{
    return memcmp( &tigera, &tigerb, 24 ) != 0;
}

#endif // !defined(AFX_TIGERTREE_H__59910156_59A2_454F_A0FE_0A66B7D37218__INCLUDED_)
