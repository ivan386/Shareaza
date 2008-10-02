//
// TigerTree.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

#pragma once

class HASHLIB_API CTigerNode
{
public:
	CTigerNode();
	uint64	value[3];
	bool bValid;
};

class HASHLIB_API CTigerTree
{
private:
	class HASHLIB_API __declspec(novtable) CSectionLock
	{
	public:
		CSectionLock( CRITICAL_SECTION* pSection ) :
			m_pSection( pSection )
		{
			EnterCriticalSection( m_pSection );
		}
		~CSectionLock()
		{
			LeaveCriticalSection( m_pSection );
		}
		CRITICAL_SECTION* m_pSection;
	};

public:
	CTigerTree();
	virtual ~CTigerTree();

	void	SetupAndAllocate(uint32 nHeight, uint64 nLength);
	void	SetupParameters(uint64 nLength);
	void	Clear();
	void	Serialize(BOOL bStoring, uchar* pBuf);
	uint32	GetHeight() const;
	void	SetHeight(uint32 nHeight);
	uint32	GetSerialSize() const;

	struct HASHLIB_API TigerTreeDigest // 192 bit
	{
		uint64& operator[](size_t i) { return data[ i ]; }
		const uint64& operator[](size_t i) const { return data[ i ]; }
		uint64 data[ 3 ];
	};

	template< typename T >
	inline BOOL GetRoot(T& oTiger) const
	{
		CSectionLock oLock( &m_pSection );
		if ( m_pNode == NULL )
			return FALSE;
		std::copy( &m_pNode->value[ 0 ], &m_pNode->value[ 3 ], &oTiger[ 0 ] );
		return TRUE;
	}
	void	Assume(CTigerTree* pSource);

	void	BeginFile(uint32 nHeight, uint64 nLength);
	void	AddToFile(const void* pInput, uint32 nLength);
	BOOL	FinishFile();

	void	BeginBlockTest();
	void	AddToTest(const void* pInput, uint32 nLength);
	BOOL	FinishBlockTest(uint32 nBlock);

	BOOL	ToBytes(uint8** pOutput, uint32* pnOutput, uint32 nHeight = 0);
	BOOL	FromBytes(const uint8* pOutput, uint32 nOutput, uint32 nHeight, uint64 nLength);
	BOOL	CheckIntegrity();

	BOOL	IsAvailable() const;
	uint32	GetHeight() const;
	uint32	GetBlockLength() const;
	uint32	GetBlockCount() const;

private:
	uint32		m_nHeight;
	CTigerNode*	m_pNode;
	uint32		m_nNodeCount;

// Processing Data
	uint32		m_nNodeBase;
	uint32		m_nNodePos;
	uint32		m_nBaseUsed;
	uint32		m_nBlockCount;
	uint32		m_nBlockPos;
	CTigerNode*	m_pStackBase;
	CTigerNode*	m_pStackTop;

	mutable CRITICAL_SECTION	m_pSection;

	void	Collapse();
	void	BlocksToNode();
	static void	Tiger(LPCVOID pInput, uint64 nInput, uint64* pOutput, uint64* pInput1 = NULL, uint64* pInput2 = NULL);
};
