//
// MD4.h
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

#if !defined(AFX_MD4_H__B0429238_3786_452C_B43D_3311AE91B5DA__INCLUDED_)
#define AFX_MD4_H__B0429238_3786_452C_B43D_3311AE91B5DA__INCLUDED_

#pragma once

#include "Hashes.h"

class CMD4 : public CHashMD4
{
private:
	QWORD	m_nCount;
	BYTE	m_nBuffer[64];
public:
	inline	CMD4();
	inline	~CMD4();
	inline	void	Reset();
	inline	void	Add(LPCVOID pData, DWORD nLength);
	inline	void	Finish();
private:
	typedef	void	( __stdcall *tpAdd1)(CMD4*, LPCVOID pData);				// add one full Block
	typedef	void	( __stdcall *tpAdd2)(CMD4*, LPCVOID pData1, CMD4*, LPCVOID pData2);
	typedef	void	( __stdcall *tpAdd3)(CMD4*, LPCVOID pData1, CMD4*, LPCVOID pData2, CMD4*, LPCVOID pData3);
	typedef	void	( __stdcall *tpAdd4)(CMD4*, LPCVOID pData1, CMD4*, LPCVOID pData2, CMD4*, LPCVOID pData3, CMD4*, LPCVOID pData4);
	typedef	void	( __stdcall *tpAdd5)(CMD4*, LPCVOID pData1, CMD4*, LPCVOID pData2, CMD4*, LPCVOID pData3, CMD4*, LPCVOID pData4, CMD4*, LPCVOID pData5);
	typedef	void	( __stdcall *tpAdd6)(CMD4*, LPCVOID pData1, CMD4*, LPCVOID pData2, CMD4*, LPCVOID pData3, CMD4*, LPCVOID pData4, CMD4*, LPCVOID pData5, CMD4*, LPCVOID pData6);
	static	BYTE	MD4_PADDING[ 64 ];
public:
	static	tpAdd1	pAdd1;
	static	tpAdd2	pAdd2;
	static	tpAdd3	pAdd3;
	static	tpAdd4	pAdd4;
	static	tpAdd5	pAdd5;
	static	tpAdd6	pAdd6;
	static	void	Init();
};

inline CMD4::CMD4()
{
	Reset();
}

inline CMD4::~CMD4()
{
}

inline void CMD4::Reset()
{
	// Load magic initialization constants
	m_d[ 0 ] = 0x67452301;
	m_d[ 1 ] = 0xefcdab89;
	m_d[ 2 ] = 0x98badcfe;
	m_d[ 3 ] = 0x10325476;
	m_nCount = 0;
}

extern "C" void __stdcall MD4_Add_p5(CMD4*, LPCVOID pData, DWORD nLength);

inline void CMD4::Add(LPCVOID pData, DWORD nLength)
{
	MD4_Add_p5( this, pData, nLength );
}

inline void CMD4::Finish()
{
	QWORD nBits = m_nCount << 3;
	DWORD index = (DWORD)m_nCount & 0x3f;
	MD4_Add_p5( this, MD4_PADDING, (index < 56) ? (56 - index) : (120 - index) );
	MD4_Add_p5( this, &nBits, 8 );
}

#endif // !defined(AFX_MD4_H__B0429238_3786_452C_B43D_3311AE91B5DA__INCLUDED_)
