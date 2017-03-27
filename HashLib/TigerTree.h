//
// TigerTree.h
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
public:
	CTigerTree();
	~CTigerTree();

	void	SetupAndAllocate(uint32 nHeight, uint64 nLength);
	void	SetupParameters(uint64 nLength);
	void	Clear();
	void	Save(uchar* pBuf) const;
	void	Load(const uchar* pBuf);
	uint32	GetSerialSize() const;

	struct HASHLIB_API TigerTreeDigest // 192 bit
	{
		uint64& operator[](size_t i) { return data[ i ]; }
		const uint64& operator[](size_t i) const { return data[ i ]; }
		uint64 data[ 3 ];
	};

	BOOL	GetRoot(__in_bcount(24) uchar* pHash) const;
	//void	Assume(CTigerTree* pSource);

	void	BeginFile(uint32 nHeight, uint64 nLength);
	void	AddToFile(const void* pInput, uint32 nLength);
	BOOL	FinishFile();

	void	BeginBlockTest();
	void	AddToTest(const void* pInput, uint32 nLength);
	BOOL	FinishBlockTest(uint32 nBlock);

	// Extract hash tree (to free ppOutput use the GlobalFree function)
	BOOL	ToBytes(uint8** ppOutput, uint32* pnOutput, uint32 nHeight = 0) const;
	// Extract first level of hash tree (to free ppOutput use the GlobalFree function)
	BOOL	ToBytesLevel1(uint8** ppOutput, uint32* pnOutput) const;
	// Create hash tree from full tree data
	BOOL	FromBytes(const uint8* pInput, uint32 nInput, uint32 nHeight, uint64 nLength);
	// Create hash tree from first level of tree data
	BOOL	FromBytesLevel1(const uint8* pInput, uint32 nInput, uint64 nLength);

	BOOL	IsAvailable() const;
	void	SetHeight(uint32 nHeight);
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

	// Check hash tree integrity (rebuilding missed hashes if needed)
	BOOL	CheckIntegrity() const;
	void	Collapse();
	void	BlocksToNode();
	static void	Tiger(LPCVOID pInput, uint64 nInput, uint64* pOutput, uint64* pInput1 = NULL, uint64* pInput2 = NULL);
};
