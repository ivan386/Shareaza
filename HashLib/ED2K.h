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
	LPCVOID	GetRawPtr() const;

	template< typename T >
	inline void GetRoot(T& oHash) const
	{
		std::copy( &m_pRoot[ 0 ], &m_pRoot[ 4 ], &oHash[ 0 ] );
	}

	template< typename T >
	inline void FromRoot(const T& oHash)
	{
		Clear();
		m_nList = 1;
		m_pList = new CMD4::Digest[ m_nList ];
		std::copy( &oHash[ 0 ], &oHash[ 4 ], &m_pRoot[ 0 ] );
		std::copy( &oHash[ 0 ], &oHash[ 4 ], &m_pList[ 0 ][ 0 ] );
	}

	void	BeginFile(uint64 nLength);
	void	AddToFile(LPCVOID pInput, uint32 nLength);
	BOOL	FinishFile();

	void	BeginBlockTest();
	void	AddToTest(LPCVOID pInput, uint32 nLength);
	BOOL	FinishBlockTest(uint32 nBlock);
	
	BOOL	ToBytes(BYTE** pOutput, uint32* pnOutput);
	BOOL	FromBytes(BYTE* pOutput, uint32 nOutput, uint64 nSize = 0);
	BOOL	CheckIntegrity();
	
	BOOL	IsAvailable() const;
	uint32	GetBlockCount() const;

private:
    CMD4::Digest m_pRoot;
    CMD4::Digest* m_pList;
	uint32	m_nList;
	CMD4	m_pSegment;
	uint32	m_nCurHash;
	uint32	m_nCurByte;
	bool	m_bNullBlock;
};

const size_t ED2K_PART_SIZE	= 9500 * 1024u;
