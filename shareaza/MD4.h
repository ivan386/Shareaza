//
// MD4.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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


class CMD4
{
// Construction
public:
	CMD4();
	virtual ~CMD4();

// Attributes
protected:
	DWORD	m_nState[4];
	DWORD	m_nCount[2];
	BYTE	m_pBuffer[64];

// Operations
public:
	void	Reset();
	void	Add(LPCVOID pData, DWORD nLength);
	void	Finish();
	void	GetHash(MD4* pHash);

};

inline bool operator==(const MD4& md4a, const MD4& md4b)
{
    return memcmp( &md4a, &md4b, 16 ) == 0;
}

inline bool operator!=(const MD4& md4a, const MD4& md4b)
{
    return memcmp( &md4a, &md4b, 16 ) != 0;
}

#endif // !defined(AFX_MD4_H__B0429238_3786_452C_B43D_3311AE91B5DA__INCLUDED_)
