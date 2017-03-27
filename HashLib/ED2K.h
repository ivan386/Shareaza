//
// ED2K.h
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

#include "MD4.h"

class HASHLIB_API CED2K
{
public:
	CED2K();
	~CED2K();

	void	Clear();
	void	Save(uchar* pBuf) const;
	void	Load(const uchar* pBuf);
	uint32	GetSerialSize() const;
	LPCVOID	GetRawPtr() const;

	void	GetRoot(__in_bcount(16) uchar* pHash) const;
	void	FromRoot(__in_bcount(16) const uchar* pHash);

	void	BeginFile(uint64 nLength);
	void	AddToFile(LPCVOID pInput, uint32 nLength);
	BOOL	FinishFile();

	void	BeginBlockTest();
	void	AddToTest(LPCVOID pInput, uint32 nLength);
	BOOL	FinishBlockTest(uint32 nBlock);
	
	// To free ppOutput, use the GlobalFree function
	BOOL	ToBytes(BYTE** ppOutput, uint32* pnOutput) const;
	BOOL	FromBytes(BYTE* pOutput, uint32 nOutput, uint64 nSize = 0);
	
	BOOL	IsAvailable() const;
	void	SetSize(uint32 nSize);
	uint32	GetSize() const;
	uint32	GetBlockCount() const;

private:
    CMD4::Digest m_pRoot;
    CMD4::Digest* m_pList;
	uint32	m_nList;
	CMD4	m_pSegment;
	uint32	m_nCurHash;
	uint32	m_nCurByte;
	bool	m_bNullBlock;

	// Check hash tree integrity (rebuilding missed hashes if needed)
	BOOL	CheckIntegrity() const;
};

const DWORD ED2K_PART_SIZE	= 9500 * 1024u;
