//
// SHA.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#ifndef SHA_H_INCLUDED
#define SHA_H_INCLUDED

#pragma once

struct SHA1State
{
	static const size_t blockSize = 64;
	uint64	m_nCount;
	uint32	m_nState[ 5 ];
	uchar m_oBuffer[ blockSize ];
};

#ifdef SHAREAZA_USE_ASM
extern "C" void __stdcall SHA1_Add_p5(SHA1State*, const void* pData, std::size_t nLength);
#endif

class CSHA
{
// Construction
public:
	CSHA() { Reset(); }

// Attributes
private:
	SHA1State m_State;

// Operations
public:
	void	GetHash(Hashes::Sha1Hash& oHash) const;
	void	GetHash(Hashes::BtHash& oHash) const;
	void	Reset();
	void	Finish();
#ifdef SHAREAZA_USE_ASM
	void	Add(const void* pData, std::size_t nLength)
	{
		SHA1_Add_p5( &m_State, pData, nLength );
	}
#else
	void	Add(const void* pData, std::size_t nLength);
	struct TransformArray
	{
		TransformArray(const uint32* const buffer);
		uint32 operator[](uint32 index) const { return m_buffer[ index ]; }
		uint32 m_buffer[ 80 ];
	};
private:
	void	Transform(const TransformArray w);
#endif
};

#endif // #ifndef SHA_H_INCLUDED
