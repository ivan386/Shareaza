//
// MD5.h
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

#if !defined(AFX_MD5_H__0C3A876B_CD09_4415_A661_35167D882CFD__INCLUDED_)
#define AFX_MD5_H__0C3A876B_CD09_4415_A661_35167D882CFD__INCLUDED_

#pragma once


class CMD5
{
// Construction
public:
	CMD5();
	virtual ~CMD5();

// Attributes
public:
	DWORD	m_nCount[2];
	DWORD	m_nState[4];
	BYTE	m_nBuffer[64];

// Operations
public:
	void			Reset();
	void			Add(LPCVOID pData, DWORD nLength);
	void			Finish();
	void			GetHash(MD5* pHash);
public:
	static CString	HashToString(const MD5* pHash, BOOL bURN = FALSE);
	static BOOL		HashFromString(LPCTSTR pszHash, MD5* pMD5);
	static BOOL		HashFromURN(LPCTSTR pszHash, MD5* pMD5);

// Implementation
protected:
	void			Transform(BYTE* pBlock);
	void			Encode(BYTE* pOutput, DWORD* pInput, DWORD nLength);
	void			Decode(DWORD* pOutput, BYTE* pInput, DWORD nLength);
};

#endif // !defined(AFX_MD5_H__0C3A876B_CD09_4415_A661_35167D882CFD__INCLUDED_)
