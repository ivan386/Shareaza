//
// SHA.h
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

#if !defined(AFX_SHA_H__9CC84DD7_E62A_410F_BFE4_B6190C509ACE__INCLUDED_)
#define AFX_SHA_H__9CC84DD7_E62A_410F_BFE4_B6190C509ACE__INCLUDED_

#pragma once


class CSHA  
{
// Construction
public:
	CSHA();
	~CSHA();
	
// Attributes
public:
	DWORD	m_nCount[2];
	DWORD	m_nHash[5];
	DWORD	m_nBuffer[16];
	
// Operations
public:
	void			Reset();
	void			Add(LPCVOID pData, DWORD nLength);
	void			Finish();
	void			GetHash(SHA1* pHash);
	CString			GetHashString(BOOL bURN = FALSE);
public:
	static CString	HashToString(const SHA1* pHash, BOOL bURN = FALSE);
	static CString	HashToHexString(const SHA1* pHash, BOOL bURN = FALSE);
	static BOOL		HashFromString(LPCTSTR pszHash, SHA1* pHash);
	static BOOL		HashFromURN(LPCTSTR pszHash, SHA1* pHash);
protected:
	void			Compile();
};

#define SHA1_BLOCK_SIZE		64
#define SHA1_DIGEST_SIZE	20

inline bool operator==(const SHA1& sha1a, const SHA1& sha1b)
{
    return memcmp( &sha1a, &sha1b, 20 ) == 0;
}

inline bool operator!=(const SHA1& sha1a, const SHA1& sha1b)
{
    return memcmp( &sha1a, &sha1b, 20 ) != 0;
}

#endif // !defined(AFX_SHA_H__9CC84DD7_E62A_410F_BFE4_B6190C509ACE__INCLUDED_)
