//
// Hashes.h
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

#if !defined(AFX_HASHES_H__FC7BC368_5878_4BCF_2ADA_055B0355AC3A__INCLUDED_)
#define AFX_HASHES_H__FC7BC368_5878_4BCF_2ADA_055B0355AC3A__INCLUDED_

#pragma once

#include "StdAfx.h"
#include "Hash.h"

#define HASH_NULL		0
#define HASH_SHA1		1
#define HASH_MD5		2
#define HASH_TIGERTREE	3
#define HASH_ED2K		4
#define HASH_TORRENT	5

typedef DWORD HASHID;

#define MD4_HASH_SIZE	16

class CHashMD4 : public CHash < MD4_HASH_SIZE * 8 >
{
public:
	inline	CString		ToString() const;
	inline	BOOL		FromString(const LPCTSTR pszHash);
};

#define ED2K_HASH_SIZE	16

class CHashED2K : public CHashMD4
{
public:
	inline	CString		ToURN() const;
	inline	BOOL		FromURN(const LPCTSTR pszURN);
};

#define MD5_HASH_SIZE	16

class CHashMD5 : public CHash < MD5_HASH_SIZE * 8 >
{
public:
	inline	CString		ToString() const;
	inline	CString		ToURN() const;
	inline	BOOL		FromString(const LPCTSTR pszHash);
	inline	BOOL		FromURN(const LPCTSTR pszURN);
};

#define SHA1_HASH_SIZE	20

class CHashSHA1 : public CHash < SHA1_HASH_SIZE * 8 >
{
public:
	inline	CString		ToString() const;
	inline	CString		ToURN() const;
	inline	BOOL		FromString(const LPCTSTR pszHash);
	inline	BOOL		FromURN(const LPCTSTR pszURN);
};

#define BT_HASH_SIZE	20

typedef CHashSHA1 CHashBT;

#define TIGER_HASH_SIZE	24

class CHashTiger : public CHash < TIGER_HASH_SIZE * 8 >
{
public:
	inline	CString		ToString() const;
	inline	CString		ToURN() const;
	inline	BOOL		FromString(const LPCTSTR pszHash);
	inline	BOOL		FromURN(const LPCTSTR pszURN);
};

typedef CManagedHash < CHashMD5 >		CManagedMD5;
typedef CManagedHash < CHashSHA1 >		CManagedSHA1;
typedef CManagedHash < CHashBT >		CManagedBTH;
typedef CManagedHash < CHashTiger >		CManagedTiger;
typedef CManagedHash < CHashED2K >		CManagedED2K;

typedef CExtendedHash < CHashMD5 >		CExtendedMD5;
typedef CExtendedHash < CHashSHA1 >		CExtendedSHA1;
typedef CExtendedHash < CHashBT >		CExtendedBTH;
typedef CExtendedHash < CHashTiger >	CExtendedTiger;
typedef CExtendedHash < CHashED2K >		CExtendedED2K;

inline CString CHashMD4::ToString() const
{
	return ToHexString();
}

inline BOOL CHashMD4::FromString(const LPCTSTR pszHash)
{
	return FromHexString( pszHash );
}

inline CString CHashED2K::ToURN() const
{
	return _T("urn:ed2khash:") + ToString();
}

inline BOOL CHashED2K::FromURN(const LPCTSTR pszURN)
{
	if ( ! pszURN ) return FALSE;
	size_t nLen = _tcslen( pszURN );
	if ( nLen >= 5 + 32 && ! _tcsnicmp( pszURN, _T("ed2k:"), 5 ) ) return FromString( pszURN + 5 );
	else if ( nLen >= 9 + 32 && ! _tcsnicmp( pszURN, _T("urn:ed2k:"), 9 ) ) return FromString( pszURN + 9 );
	else if ( nLen >= 9 + 32 && ! _tcsnicmp( pszURN, _T("ed2khash:"), 9 ) ) return FromString( pszURN + 9 );
	else if ( nLen >= 13 + 32 && ! _tcsnicmp( pszURN, _T("urn:ed2khash:"), 13 ) ) return FromString( pszURN + 13 );
	return FALSE;
}

inline CString CHashMD5::ToString() const
{
	return ToHexString();
}

inline CString CHashMD5::ToURN() const
{
	return _T("md5:") + ToString();
}

inline BOOL CHashMD5::FromString(const LPCTSTR pszHash)
{
	return FromHexString( pszHash );
}

inline BOOL CHashMD5::FromURN(const LPCTSTR pszURN)
{
	if ( ! pszURN ) return FALSE;
	size_t nLen = _tcslen( pszURN );
	if ( nLen >= 4 + 32 && ! _tcsnicmp( pszURN, _T("md5:"), 4 ) ) return FromString( pszURN + 4 );
	else if ( nLen >= 8 + 32 && ! _tcsnicmp( pszURN, _T("urn:md5:"), 8 ) ) return FromString( pszURN + 8 );
	return FALSE;
}

inline CString CHashSHA1::ToString() const
{
	return ToBase32String();
}

inline CString CHashSHA1::ToURN() const
{
	return _T("urn:sha1:") + ToString();
}

inline BOOL CHashSHA1::FromString(const LPCTSTR pszHash)
{
	return FromBase32String( pszHash );
}

inline BOOL CHashSHA1::FromURN(const LPCTSTR pszURN)
{
	if ( !pszURN ) return FALSE;
	size_t nLen = _tcslen( pszURN );
	if ( nLen >= 41 && ! _tcsnicmp( pszURN, _T("urn:sha1:"), 9 ) ) return FromString( pszURN + 9 );
	else if ( nLen >= 37 && ! _tcsnicmp( pszURN, _T("sha1:"), 5 ) ) return FromString( pszURN + 5 );
	else if ( nLen >= 85 && ! _tcsnicmp( pszURN, _T("urn:bitprint:"), 13 ) ) return FromString( pszURN + 13 );
	else if ( nLen >= 81 && ! _tcsnicmp( pszURN, _T("bitprint:"), 9 ) ) return FromString( pszURN + 9 );
	return FALSE;
}

inline CString CHashTiger::ToString() const
{
	return ToBase32String();
}

inline CString CHashTiger::ToURN() const
{
	return _T("urn:tree:tiger/:") + ToString();
}

inline BOOL CHashTiger::FromString(const LPCTSTR pszHash)
{
	return FromBase32String( pszHash );
}

inline BOOL CHashTiger::FromURN(const LPCTSTR pszURN)
{
	if ( !pszURN ) return FALSE;
	size_t nLen = _tcslen( pszURN );
	if ( nLen >= 16+39 && ! _tcsncmp( pszURN, _T("urn:tree:tiger/:"), 16 ) ) return FromString( pszURN + 16 );
	else if ( nLen >= 12+39 && ! _tcsncmp( pszURN, _T("tree:tiger/:"), 12 ) ) return FromString( pszURN + 12 );
	else if ( nLen >= 15+39 && ! _tcsncmp( pszURN, _T("urn:tree:tiger:"), 15 ) ) return FromString( pszURN + 15 );
	else if ( nLen >= 11+39 && ! _tcsncmp( pszURN, _T("tree:tiger:"), 11 ) ) return FromString( pszURN + 11 );
	else if ( nLen >= 85 && ! _tcsncmp( pszURN, _T("urn:bitprint:"), 13 ) ) return FromString( pszURN + 46 );
	else if ( nLen >= 81 && ! _tcsncmp( pszURN, _T("bitprint:"), 9 ) ) return FromString( pszURN + 42 );
	return FALSE;
}

#endif // !defined(AFX_HASHES_H__FC7BC368_5878_4BCF_2ADA_055B0355AC3A__INCLUDED_)
