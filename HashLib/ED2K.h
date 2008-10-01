//
// ED2K.h
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

#include "MD4.h"

class HASHLIB_API CED2K
{
public:
	CED2K();
	virtual ~CED2K();

	void	Clear();
	uint32	GetSerialSize() const;
    void	GetRoot(CMD4::MD4Digest& oHash) const;
	void	BeginFile(uint64 nLength);
	void	AddToFile(LPCVOID pInput, uint32 nLength);
	BOOL	FinishFile();
	void	BeginBlockTest();
	void	AddToTest(LPCVOID pInput, uint32 nLength);
	BOOL	FinishBlockTest(uint32 nBlock);
	LPCVOID	GetRawPtr() const;
	BOOL	ToBytes(BYTE** pOutput, uint32* pnOutput);
	BOOL	FromBytes(BYTE* pOutput, uint32 nOutput, uint64 nSize = 0);
    void	FromRoot(const CMD4::MD4Digest& oHash);
	BOOL	CheckIntegrity();
	BOOL	IsAvailable() const;
	uint32	GetBlockCount() const;

private:
    CMD4::MD4Digest m_pRoot;
    CMD4::MD4Digest* m_pList;
	uint32	m_nList;
	CMD4	m_pSegment;
	uint32	m_nCurHash;
	uint32	m_nCurByte;
	bool	m_bNullBlock;
};

const size_t ED2K_PART_SIZE	= 9500 * 1024u;
