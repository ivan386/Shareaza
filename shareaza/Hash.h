//
// Hash.h
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

#if !defined(AFX_HASH_H__FC7BC368_5878_4BCF_2ADA_055B0355AC3A__INCLUDED_)
#define AFX_HASH_H__FC7BC368_5878_4BCF_2ADA_055B0355AC3A__INCLUDED_

#pragma once

#include "StdAfx.h"

const LPCTSTR pszBase16 = _T("0123456789abcdef");
const LPCTSTR pszBase32 = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ234567");

#define EXTENDED_HASH_TRUSTWORTHY_THRESHOLD	10

template < int nHashSize > class CHash;
template < class Hash > class CManagedHash;
template < class Hash > class CExtendedHash;

template < int nHashSize > class CHash
{
public:
	union
	{
		BYTE	m_b[ nHashSize /  8 ];
		WORD	m_w[ nHashSize / 16 ];
		DWORD	m_d[ nHashSize / 32 ];
		QWORD	m_q[ nHashSize / 64 ];
	};
public:
	inline	CHash();
	inline	CHash(const CHash < nHashSize > &oHash);
	inline	~CHash();
	inline	void		SerializeStore(CArchive &ar, const DWORD nVersion);
	inline	void		SerializeLoad(CArchive &ar, const DWORD nVersion);
	inline	void		operator = (const CHash < nHashSize > &oHash);

	inline	int			operator -	(const CHash < nHashSize > &oHash) const;

	inline	BOOL		operator ==	(const CHash < nHashSize > &oHash) const;
	inline	BOOL		operator == (const CManagedHash < CHash < nHashSize > > &oHash) const;
	inline	BOOL		operator !=	(const CHash < nHashSize > &oHash) const;
	inline	BOOL		operator != (const CManagedHash < CHash < nHashSize > > &oHash) const;

	inline	BOOL		operator <	(const CHash < nHashSize > &oHash) const;
	inline	BOOL		operator <=	(const CHash < nHashSize > &oHash) const;
	inline	BOOL		operator >	(const CHash < nHashSize > &oHash) const;
	inline	BOOL		operator >=	(const CHash < nHashSize > &oHash) const;

	inline	CString		ToHexString() const;
	inline	BOOL		FromHexString(const LPCTSTR pszHash);
	inline	CString		ToBase32String() const;
	inline	BOOL		FromBase32String(const LPCTSTR pszHash);
};

template < class Hash > class CManagedHash : public Hash
{
protected:
	DWORD	bValid;
public:
	inline	CManagedHash();
	inline	CManagedHash(const Hash &oHash);
	inline	CManagedHash(const CManagedHash < Hash > &oHash);
	inline	~CManagedHash();
	inline	void	SerializeStore(CArchive &ar, const DWORD nVersion);
	inline	void	SerializeLoad(CArchive &ar, const DWORD nVersion);
	inline	void	operator = (const Hash &oHash);
	inline	void	operator = (const CManagedHash < Hash > &oHash);

	inline	BOOL	operator == (const Hash &oHash) const;
	inline	BOOL	operator == (const CManagedHash < Hash > &oHash) const;
	inline	BOOL	operator != (const Hash &oHash) const;
	inline	BOOL	operator != (const CManagedHash < Hash > &oHash) const;

	inline	void	SetValid();
	inline	void	Clear();
	inline	BOOL	IsValid() const;
	inline	CString	ToString() const;
	inline	CString	ToURN() const;
	inline	BOOL	FromString(const LPCTSTR pszHash);
	inline	BOOL	FromURN(const LPCTSTR pszURN);
};

template < class Hash > class CExtendedHash : public CManagedHash < Hash >
{
protected:
	DWORD	bTrusted;
	DWORD	nSucceeded;
	DWORD	nFailed;
public:
	inline	CExtendedHash();
	inline	CExtendedHash(const Hash &oHash);
	inline	CExtendedHash(const CManagedHash < Hash > &oHash);
	inline	CExtendedHash(const CExtendedHash < Hash > &oHash);
	inline	~CExtendedHash();
	inline	void	SerializeStore(CArchive &ar, const DWORD nVersion);
	inline	void	SerializeLoad(CArchive &ar, const DWORD nVersion);
	inline	void	operator = (const Hash &oHash);
	inline	void	operator = (const CManagedHash < Hash > &oHash);
	inline	void	operator = (const CExtendedHash < Hash > &oHash);

	inline	void	SetValid();
	inline	void	Clear();
	inline	void	SetTrusted();
	inline	void	ClearTrusted();
	inline	void	SetSuccess();
	inline	void	SetFailure();
	inline	BOOL	IsTrusted() const;
	inline	BOOL	IsTrustworthy() const;
	inline	DWORD	Succeeded() const;
	inline	DWORD	Failed() const;
	inline	DWORD	Validated() const;
	inline	BOOL	FromString(const LPCTSTR pszHash);
	inline	BOOL	FromURN(const LPCTSTR pszURN);
};

template < int nHashSize > inline CHash < nHashSize >::CHash()
{
}

template < int nHashSize > inline CHash < nHashSize >::CHash(const CHash < nHashSize > &oHash)
{
	CopyMemory( &m_b, &oHash.m_b, sizeof m_b );
}

template < int nHashSize > inline CHash < nHashSize >::~CHash()
{
}

template < int nHashSize > inline void CHash < nHashSize >::SerializeStore(CArchive &ar, const DWORD nVersion)
{
	ASSERT( ar.IsStoring() );
	ar.Write( &m_b, sizeof m_b );
}

template < int nHashSize > inline void CHash < nHashSize >::SerializeLoad(CArchive &ar, const DWORD nVersion)
{
	ASSERT( ar.IsLoading() );
	ar.Read( &m_b, sizeof m_b );
}

template < int nHashSize > inline void CHash < nHashSize >::operator = (const CHash < nHashSize > &oHash)
{
	CopyMemory( &m_b, &oHash.m_b, sizeof m_b );
}

// returns 'difference' of two hashes - useful if you have to do several comparisions on the same hashes
// for example distinct between <  ,  ==  and >
template < int nHashSize > inline int CHash < nHashSize >::operator - (const CHash < nHashSize > &oHash) const
{
	return memcmp( &m_b, &oHash.m_b, sizeof m_b );
}

// simple comparision ==
template < int nHashSize > inline BOOL CHash < nHashSize >::operator == (const CHash < nHashSize > &oHash) const
{
	return operator - ( oHash ) == 0;
}

// hashes can only be equal if valid ( simple hashes are assumed to be valid when used )
template < int nHashSize > inline BOOL CHash < nHashSize >::operator == (const CManagedHash < CHash < nHashSize > > &oHash) const
{
	return oHash.bValid && operator - ( oHash ) == 0;
}

// simple comparision !=
template < int nHashSize > inline BOOL CHash < nHashSize >::operator != (const CHash < nHashSize > &oHash) const
{
	return operator - ( oHash ) != 0;
}

// ask for definitive != between hashes; thats only possible if they are valid
// this means that == and != are not exact opposites; both will always return false when used on invalid hashes
// if you need the traditidional a != b    use ! ( a == b ) instead
template < int nHashSize > inline BOOL CHash < nHashSize >::operator != (const CManagedHash < CHash < nHashSize > > &oHash) const
{
	return oHash.bValid && operator - ( oHash ) != 0;
}

// the following functions don't check for validity, afaik they are not used yet
// all those functions are guarantied to be transitive, thus suitable for sorting
// they might become useful for fast sorting (hashtable lookups), if check for validity is needed; you have to
// these functions in CManagedHash (camper)
// simple comparision <
template < int nHashSize > inline BOOL CHash < nHashSize >::operator < (const CHash < nHashSize > &oHash) const
{
	return operator - ( oHash ) < 0;
}

// simple comparision <=
template < int nHashSize > inline BOOL CHash < nHashSize >::operator <= (const CHash < nHashSize > &oHash) const
{
	return operator - ( oHash ) <= 0;
}

// simple comparision >
template < int nHashSize > inline BOOL CHash < nHashSize >::operator > (const CHash < nHashSize > &oHash) const
{
	return operator - ( oHash ) > 0;
}

// simple comparision >=
template < int nHashSize > inline BOOL CHash < nHashSize >::operator >= (const CHash < nHashSize > &oHash) const
{
	return operator - ( oHash ) >= 0;
}

// returns a String that contains a hexadezimal representation of the hash (lower case)
template < int nHashSize > inline CString CHash < nHashSize >::ToHexString() const
{
	CString str;
	LPTSTR pszHash = str.GetBuffer( 2 * nHashSize / 8 );
	int nByte = -1, nPos = 0;
	do
	{
		pszHash[ nPos ] = pszBase16[ m_b[ ++nByte ] >> 4 ];
		pszHash[ nPos + 1 ] = pszBase16[ m_b[ nByte ] & 0xf ];
	}
	while ( ( nPos += 2 ) < 2 * nHashSize / 8 );
	str.ReleaseBuffer( 2 * nHashSize / 8 );
	return str;
}

// reads a hash from string that is formatted in hex - returns true on success
// returns false if it fails - in that case part of the original hash might be overwritten
template < int nHashSize > inline BOOL CHash < nHashSize >::FromHexString(const LPCTSTR pszHash)
{
	if ( ! pszHash || _tcslen( pszHash ) < 2 * nHashSize / 8 ) return FALSE;
	int nByte = -1, nPos = 0;
	WORD nChar;
	do
	{
		if ( ( nChar = pszHash[ nPos ] - '0' ) >= 0 && nChar <= 9 ) m_b[ ++nByte ] = nChar << 4;
		else if ( ( nChar = pszHash[ nPos ] - 'A' + 10 ) >= 10 && nChar <= 15 ) m_b[ ++nByte ] = nChar << 4;
		else if ( ( nChar = pszHash[ nPos ] - 'a' + 10 ) >= 10 && nChar <= 15 ) m_b[ ++nByte ] = nChar << 4;
		else return FALSE;
		if ( ( nChar = pszHash[ nPos + 1 ] - '0' ) >= 0 && nChar <= 9 ) m_b[ nByte ] |= nChar;
		else if ( ( nChar = pszHash[ nPos + 1 ] - 'A' + 10 ) >= 10 && nChar <= 15 ) m_b[ nByte ] |= nChar;
		else if ( ( nChar = pszHash[ nPos + 1 ] - 'a' + 10 ) >= 10 && nChar <= 15 ) m_b[ nByte ] |= nChar;
		else return FALSE;
	}
	while ( ( nPos += 2 ) < 2 * nHashSize / 8 );
	return TRUE;
}

// returns a String that contains a base32 encoded representation of the hash (lower case)
template < int nHashSize > inline CString CHash < nHashSize >::ToBase32String() const
{
	CString str;
	LPTSTR pszHash = str.GetBuffer( ( nHashSize + 4 ) / 5 );
	int nShift = 11, nByte = 0, nPos = 0;
	do
	{
		pszHash[ nPos ] = pszBase32[ ( (WORD)m_b[ nByte ] << 8 | m_b[ nByte + 1 ] ) >> nShift & 0x1f ];
		if ( ( nShift -= 5 ) <= 0 )
		{
			nShift += 8;
			nByte++;
		}
	}
	while ( ++nPos < ( nHashSize - 1 ) / 5 );
// if 5 is not a divisor of nHashSize we need to be careful with the last character
// in order to avoid printing uninitialized memory
	pszHash[ nPos ] = pszBase32[ m_b[ nByte ] << ( 8 - nShift ) & 0x1f ];
	str.ReleaseBuffer( ( nHashSize + 4 ) / 5 );
	return str;
}

// reads a hash from string that is formatted in base32 encoding - returns true on success
// returns false if it fails - in that case part of the original hash might be overwritten
template < int nHashSize > inline BOOL CHash < nHashSize >::FromBase32String(const LPCTSTR pszHash)
{
	if ( ! pszHash || _tcslen( pszHash ) < ( nHashSize + 4 ) / 5 ) return FALSE;
	int nPos = 0, nShift = 11, nByte = -1;
	WORD nChar;
	union
	{
		BYTE	b[2];
		WORD	w;
	};
	w = 0;
	do
	{
		if ( ( nChar = pszHash[ nPos ] - 'A' ) >= 0 && nChar < 26 ) w |= nChar << nShift;
		else if ( ( nChar = pszHash[ nPos ] - 'a' ) >= 0 && nChar < 26 ) w |= nChar << nShift;
		else if ( ( nChar = pszHash[ nPos ] - '2' + 26 ) >= 26 && nChar < 32 ) w |= nChar << nShift;
		else return FALSE;
		if ( ( nShift -= 5 ) < 0 )
		{
			nShift += 8;
			m_b[ ++nByte ] = b[ 1 ];
			w <<= 8;
		}
	}
	while ( ++nPos < ( nHashSize + 4 ) / 5 );
	ASSERT( nByte == nHashSize / 8 - 2 );
	m_b[ nByte + 1 ] = b[ 1 ];
	return TRUE;
}

template < class Hash > inline CManagedHash < Hash >::CManagedHash()
{
	Clear();
}

template < class Hash > inline CManagedHash < Hash >::CManagedHash(const Hash &oHash)
{
	Hash::operator = ( oHash );
	bValid = TRUE;
}

template < class Hash > inline CManagedHash < Hash >::CManagedHash(const CManagedHash < Hash > &oHash)
{
	Hash::operator = ( oHash );
	bValid = oHash.bValid;
}

template < class Hash > inline CManagedHash < Hash >::~CManagedHash()
{
}

template < class Hash > inline void CManagedHash < Hash >::SerializeStore(CArchive &ar, const DWORD nVersion)
{
	ASSERT( ar.IsStoring() );
	ar << bValid;
	if ( bValid ) Hash::SerializeStore( ar, nVersion );
}

template < class Hash > inline void CManagedHash < Hash >::SerializeLoad(CArchive &ar, const DWORD nVersion)
{
	ASSERT( ar.IsLoading() );
	ar >> bValid;
	if ( bValid ) Hash::SerializeLoad( ar, nVersion );
}

template < class Hash > inline void CManagedHash < Hash >::operator = (const CManagedHash < Hash > &oHash)
{
	if ( bValid = oHash.bValid ) Hash::operator = ( oHash );
}

template < class Hash > inline void CManagedHash < Hash >::operator = (const Hash &oHash)
{
	Hash::operator = ( oHash );
	bValid = TRUE;
}

template < class Hash > inline BOOL CManagedHash < Hash >::operator == (const CManagedHash < Hash > &oHash) const
{
	return bValid && Hash::operator == ( oHash );
}

template < class Hash > inline BOOL CManagedHash < Hash >::operator == (const Hash &oHash) const
{
	return bValid && Hash::operator == ( oHash );
}

template < class Hash > inline BOOL CManagedHash < Hash >::operator != (const CManagedHash < Hash > &oHash) const
{
	return bValid && Hash::operator != ( oHash );
}

template < class Hash > inline BOOL CManagedHash < Hash >::operator != (const Hash &oHash) const
{
	return bValid && Hash::operator != ( oHash );
}

template < class Hash > inline void CManagedHash < Hash >::SetValid()
{
	bValid = TRUE;
}

template < class Hash > inline void CManagedHash < Hash >::Clear()
{
	bValid = FALSE;
}

template < class Hash > inline BOOL CManagedHash < Hash >::IsValid() const
{
	return bValid;
}

template < class Hash > inline CString CManagedHash < Hash >::ToString() const
{
	if ( IsValid() ) return Hash::ToString();
	CString str;
	return str;
}

template < class Hash > inline CString CManagedHash < Hash >::ToURN() const
{
	if ( IsValid() ) return Hash::ToURN();
	CString str;
	return str;
}

template < class Hash > inline BOOL CManagedHash < Hash >::FromString(const LPCTSTR pszHash)
{
	return bValid = Hash::FromString( pszHash );
}

template < class Hash > inline BOOL CManagedHash < Hash >::FromURN(const LPCTSTR pszURN)
{
	return bValid = Hash::FromURN( pszURN );
}

template < class Hash > inline CExtendedHash < Hash >::CExtendedHash()
{
	bTrusted = bValid = FALSE;
	nFailed = nSucceeded = 0;
}

template < class Hash > inline CExtendedHash < Hash >::CExtendedHash(const Hash &oHash)
{
	CManagedHash < Hash >::operator = ( oHash );
	bTrusted = FALSE;
	nFailed = nSucceeded = 0;
}

template < class Hash > inline CExtendedHash < Hash >::CExtendedHash(const CManagedHash < Hash > &oHash)
{
	CManagedHash < Hash >::operator = ( oHash );
	bTrusted = FALSE;
	nFailed = nSucceeded = 0;
}

template < class Hash > inline CExtendedHash < Hash >::CExtendedHash(const CExtendedHash < Hash > &oHash)
{
	CManagedHash < Hash >::operator = ( oHash );
	bTrusted = oHash.bTrusted;
	nSucceeded = oHash.nSucceeded;
	nFailed = oHash.nFailed;
}

template < class Hash > inline CExtendedHash < Hash >::~CExtendedHash()
{
}

template < class Hash > inline void CExtendedHash < Hash >::SerializeStore(CArchive &ar, const DWORD nVersion)
{
	ASSERT( ar.IsStoring() );
	CManagedHash < Hash >::SerializeStore( ar, nVersion );
	ar << bTrusted;
}

template < class Hash > inline void CExtendedHash < Hash >::SerializeLoad(CArchive &ar, const DWORD nVersion)
{
	ASSERT( ar.IsLoading() );
	CManagedHash < Hash >::SerializeLoad( ar, nVersion );
	if ( nVersion >= 31 ) ar >> bTrusted;
	if ( nVersion < 32 ) bTrusted = bValid;
	nFailed = nSucceeded = 0;
}

template < class Hash > inline void CExtendedHash < Hash >::operator = (const Hash &oHash)
{
	CManagedHash < Hash >::operator = ( oHash );
	bTrusted = FALSE;
	nFailed = nSucceeded = 0;
}

template < class Hash > inline void CExtendedHash < Hash >::operator = (const CManagedHash < Hash > &oHash)
{
	CManagedHash < Hash >::operator = ( oHash );
	bTrusted = FALSE;
	nFailed = nSucceeded = 0;
}

template < class Hash > inline void CExtendedHash < Hash >::operator = (const CExtendedHash < Hash > &oHash)
{
	CManagedHash < Hash >::operator = ( oHash );
	bTrusted = oHash.bTrusted;
	nSucceeded = oHash.nSucceeded;
	nFailed = oHash.nFailed;
}

template < class Hash > inline void CExtendedHash < Hash >::SetValid()
{
	bValid = TRUE;
	bTrusted = FALSE;
	nFailed = nSucceeded = 0;
}

template < class Hash > inline void CExtendedHash < Hash >::Clear()
{
	bTrusted = bValid = FALSE;
	nFailed = nSucceeded = 0;
}

template < class Hash > inline void CExtendedHash < Hash >::SetTrusted()
{
	if ( bTrusted != bValid )
	{
		bTrusted = bValid;
		nFailed = nSucceeded = 0;
	}
}

template < class Hash > inline void CExtendedHash < Hash >::ClearTrusted()
{
	if ( bTrusted != bValid )
	{
		ASSERT( bValid );
		bTrusted = FALSE;
		nFailed = nSucceeded = 0;
	}
}

template < class Hash > inline void CExtendedHash < Hash >::SetSuccess()
{
	ASSERT( bValid );
	nSucceeded++;
}

template < class Hash > inline void CExtendedHash < Hash >::SetFailure()
{
	ASSERT( bValid );
	nFailed++;
}

template < class Hash > inline BOOL CExtendedHash < Hash >::IsTrusted() const
{
	return bTrusted;
}

template < class Hash > inline BOOL CExtendedHash < Hash >::IsTrustworthy() const
{
	return bTrusted || ( bValid && ! nFailed && nSucceeded >= EXTENDED_HASH_TRUSTWORTHY_THRESHOLD );
}

template < class Hash > inline DWORD CExtendedHash < Hash >::Succeeded() const
{
	return nSucceeded;
}

template < class Hash > inline DWORD CExtendedHash < Hash >::Failed() const
{
	return nFailed;
}

template < class Hash > inline DWORD CExtendedHash < Hash >::Validated() const
{
	return nSucceeded + nFailed;
}

template < class Hash > inline BOOL CExtendedHash < Hash >::FromString(const LPCTSTR pszHash)
{
	bTrusted = FALSE;
	nFailed = nSucceeded = 0;
	return CManagedHash < Hash>::FromString( pszHash );
}

template < class Hash > inline BOOL CExtendedHash < Hash >::FromURN(const LPCTSTR pszURN)
{
	bTrusted = FALSE;
	nFailed = nSucceeded = 0;
	return CManagedHash < Hash >::FromURN( pszURN );
}

#endif // !defined(AFX_HASH_H__FC7BC368_5878_4BCF_2ADA_055B0355AC3A__INCLUDED_)
