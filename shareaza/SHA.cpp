/*
 ---------------------------------------------------------------------------
 Copyright (c) 2002, Dr Brian Gladman <brg@gladman.me.uk>, Worcester, UK.
 All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary 
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright 
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products 
      built using this software without specific written permission. 

 ALTERNATIVELY, provided that this notice is retained in full, this product
 may be distributed under the terms of the GNU General Public License (GPL),
 in which case the provisions of the GPL apply INSTEAD OF those given above.
 
 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness 
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue Date: 30/11/2002

 This is a byte oriented version of SHA1 that operates on arrays of bytes
 stored in memory. It runs at 22 cycles per byte on a Pentium P4 processor
*/

/* Modified by Camper using extern methods     6.7.2004 */

#include "StdAfx.h"
#include "SHA.h"

// This detects ICL and makes necessary changes for proper compilation
#if __INTEL_COMPILER > 0
#define asm_m_nCount CSHA.m_nCount
#else
#define asm_m_nCount m_nCount
#endif

extern "C" void SHA_Add_p5(CSHA *, LPCVOID pData, DWORD nLength);
static unsigned char SHA_PADDING[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

CSHA::CSHA()
{
	Reset();
}

CSHA::~CSHA()
{
}

void CSHA::Reset()
{
    m_nCount[0] = m_nCount[1] = 0;
    m_nHash[0] = 0x67452301;
    m_nHash[1] = 0xefcdab89;
    m_nHash[2] = 0x98badcfe;
    m_nHash[3] = 0x10325476;
    m_nHash[4] = 0xc3d2e1f0;
}

void CSHA::GetHash(SHA1* pHash)
{
    /* extract the hash value as bytes in case the hash buffer is   */
    /* misaligned for 32-bit words                                  */
    for(int i = 0; i < SHA1_DIGEST_SIZE; ++i)
        pHash->b[i] = (unsigned char)(m_nHash[i >> 2] >> 8 * (~i & 3));
}

void CSHA::Add(LPCVOID pData, DWORD nLength)
{
	SHA_Add_p5(this, pData, nLength);
}


void CSHA::Finish()
{
	unsigned int bits[2], index = 0;
	byte bit[8];
	// Save number of bits
	_asm
	{
		mov		ecx, this
		mov		eax, [ecx+asm_m_nCount]
		mov		edx, [ecx+asm_m_nCount+4]
		shld	edx, eax, 3
		shl		eax, 3
		bswap	edx
		bswap	eax
		mov		bits, edx
		mov		bits+4, eax
	}
	// Pad out to 56 mod 64.
	index = (unsigned int)(m_nCount[0] & 0x3f);
	SHA_Add_p5(this, SHA_PADDING, (index < 56) ? (56 - index) : (120 - index) );
	// Append length (before padding)
	SHA_Add_p5(this, bits, 8 );
}

//////////////////////////////////////////////////////////////////////
// CSHA get hash string (Base64)

CString CSHA::GetHashString(BOOL bURN)
{
	SHA1 pHash;
	GetHash( &pHash );
	return HashToString( &pHash, bURN );
}

//////////////////////////////////////////////////////////////////////
// CSHA convert hash to string (Base64)

CString CSHA::HashToString(const SHA1* pHashIn, BOOL bURN)
{
	static LPCTSTR pszBase64 = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ234567");

	CString strHash;
	LPTSTR pszHash = strHash.GetBuffer( bURN ? 9 + 32 : 32 );

	if ( bURN )
	{
		*pszHash++ = 'u'; *pszHash++ = 'r'; *pszHash++ = 'n'; *pszHash++ = ':';
		*pszHash++ = 's'; *pszHash++ = 'h'; *pszHash++ = 'a'; *pszHash++ = '1'; *pszHash++ = ':';
	}

	LPBYTE pHash = (LPBYTE)pHashIn;
	int nShift = 7;

	for ( int nChar = 32 ; nChar ; nChar-- )
	{
		BYTE nBits = 0;

		for ( int nBit = 0 ; nBit < 5 ; nBit++ )
		{
			if ( nBit ) nBits <<= 1;
			nBits |= ( *pHash >> nShift ) & 1;

			if ( ! nShift-- )
			{
				nShift = 7;
				pHash++;
			}
		}

		*pszHash++ = pszBase64[ nBits ];
	}

	strHash.ReleaseBuffer( bURN ? 9 + 32 : 32 );

	return strHash;
}

//////////////////////////////////////////////////////////////////////
// CSHA convert hash to string (hex)

CString CSHA::HashToHexString(const SHA1* pHashIn, BOOL bURN)
{
	static LPCTSTR pszHex = _T("0123456789ABCDEF");

	LPBYTE pHash = (LPBYTE)pHashIn;
	CString strHash;
	LPTSTR pszHash = strHash.GetBuffer( 40 );

	for ( int nByte = 0 ; nByte < 20 ; nByte++, pHash++ )
	{
		*pszHash++ = pszHex[ *pHash >> 4 ];
		*pszHash++ = pszHex[ *pHash & 15 ];
	}

	strHash.ReleaseBuffer( 40 );

	if ( bURN ) strHash = _T("urn:sha1:") + strHash;

	return strHash;
}

//////////////////////////////////////////////////////////////////////
// CSHA parse hash from string (Base64)

BOOL CSHA::HashFromString(LPCTSTR pszHash, SHA1* pHashIn)
{
	if ( ! pszHash || _tcslen( pszHash ) < 32 ) return FALSE;

	LPBYTE pHash = (LPBYTE)pHashIn;
	DWORD nBits	= 0;
	int nCount	= 0;

	for ( int nChars = 32 ; nChars-- ; pszHash++ )
	{
		if ( *pszHash >= 'A' && *pszHash <= 'Z' )
			nBits |= ( *pszHash - 'A' );
		else if ( *pszHash >= 'a' && *pszHash <= 'z' )
			nBits |= ( *pszHash - 'a' );
		else if ( *pszHash >= '2' && *pszHash <= '7' )
			nBits |= ( *pszHash - '2' + 26 );
		else
			return FALSE;
		
		nCount += 5;

		if ( nCount >= 8 )
		{
			*pHash++ = (BYTE)( nBits >> ( nCount - 8 ) );
			nCount -= 8;
		}

		nBits <<= 5;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSHA parse hash from URN

BOOL CSHA::HashFromURN(LPCTSTR pszHash, SHA1* pHashIn)
{
	if ( pszHash == NULL ) return FALSE;
	int nLen = _tcslen( pszHash );

	if ( nLen >= 41 && _tcsnicmp( pszHash, _T("urn:sha1:"), 9 ) == 0 )
	{
		return HashFromString( pszHash + 9, pHashIn );
	}
	else if ( nLen >= 37 && _tcsnicmp( pszHash, _T("sha1:"), 5 ) == 0 )
	{
		return HashFromString( pszHash + 5, pHashIn );
	}
	else if ( nLen >= 85 && _tcsnicmp( pszHash, _T("urn:bitprint:"), 13 ) == 0 )
	{
		// 13 + 32 + 1 + 39
		return HashFromString( pszHash + 13, pHashIn );
	}
	else if ( nLen >= 81 && _tcsnicmp( pszHash, _T("bitprint:"), 9 ) == 0 )
	{
		return HashFromString( pszHash + 9, pHashIn );
	}

	return FALSE;
}

