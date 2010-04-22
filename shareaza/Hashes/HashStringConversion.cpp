//
// Hashes/HashStringConversion.cpp
//
// Copyright (c) Shareaza Development Team, 2005-2008.
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

//! \file       Hashes/HashStringConversion.cpp
//! \brief      Defines functions for conversion between hashes and strings.

#include "..\StdAfx.h"

// If we are compiling in debug mode, replace the text "THIS_FILE" in the code with the name of this file
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace Hashes
{

	const wchar base16[] = L"0123456789abcdef";
	const wchar base32[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ234567=";

	bool Unhex(LPCTSTR psz, uchar* pOut)
	{
		register TCHAR c = *psz++;
		if ( c >= '0' && c <= '9' )
			*pOut = uchar( ( c - '0' ) << 4 );
		else if ( c >= 'A' && c <= 'F' )
			*pOut = uchar( ( c - 'A' + 10 ) << 4 );
		else if ( c >= 'a' && c <= 'f' )
			*pOut = uchar( ( c - 'a' + 10 ) << 4 );
		else
			return false;
		c = *psz;
		if ( c >= '0' && c <= '9' )
			*pOut |= uchar( c - '0' );
		else if ( c >= 'A' && c <= 'F' )
			*pOut |= uchar( c - 'A' + 10 );
		else if ( c >= 'a' && c <= 'f' )
			*pOut |= uchar( c - 'a' + 10 );
		else
			return false;
		return true;
	}

	StringType toGuid(const uchar* hash)
	{
		return toGuid( *reinterpret_cast< const CLSID* >( hash ), false );
	}

	StringType toGuid(REFCLSID hash, bool enclosed)
	{
		StringType str;
		if ( enclosed )
			str.Format( _T("{%.8X-%.4X-%.4X-%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X}"),
			hash.Data1, hash.Data2, hash.Data3,
			hash.Data4[0], hash.Data4[1], hash.Data4[2], hash.Data4[3],
			hash.Data4[4], hash.Data4[5], hash.Data4[6], hash.Data4[7] );
		else
			str.Format( _T("%.8X-%.4X-%.4X-%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X"),
			hash.Data1, hash.Data2, hash.Data3,
			hash.Data4[0], hash.Data4[1], hash.Data4[2], hash.Data4[3],
			hash.Data4[4], hash.Data4[5], hash.Data4[6], hash.Data4[7] );
		return str;
	}

	bool fromGuid(uchar* hash, LPCTSTR input)
	{
		if ( input == NULL ) return false;

		switch ( _tcslen( input ) )
		{
		case 38:
			// {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
			if ( input[0] != L'{' || input[37] != L'}' ) return false;
			input ++;
			break;
		case 36:
			// xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
			break;
		default:
			return false;
		}

		if ( ! Unhex( input + 0, hash + 3 ) ) return false;
		if ( ! Unhex( input + 2, hash + 2 ) ) return false;
		if ( ! Unhex( input + 4, hash + 1 ) ) return false;
		if ( ! Unhex( input + 6, hash + 0 ) ) return false;
		if ( ! Unhex( input + 9, hash + 5 ) ) return false;
		if ( ! Unhex( input + 11, hash + 4 ) ) return false;
		if ( ! Unhex( input + 14, hash + 7 ) ) return false;
		if ( ! Unhex( input + 16, hash + 6 ) ) return false;
		if ( ! Unhex( input + 19, hash + 8 ) ) return false;
		if ( ! Unhex( input + 21, hash + 9 ) ) return false;
		if ( ! Unhex( input + 24, hash + 10 ) ) return false;
		if ( ! Unhex( input + 26, hash + 11 ) ) return false;
		if ( ! Unhex( input + 28, hash + 12 ) ) return false;
		if ( ! Unhex( input + 30, hash + 13 ) ) return false;
		if ( ! Unhex( input + 32, hash + 14 ) ) return false;
		if ( ! Unhex( input + 34, hash + 15 ) ) return false;

		return true;
	}

	bool fromGuid(LPCTSTR input, LPVOID hash)
	{
		return fromGuid( reinterpret_cast< uchar* >( hash ), input );
	}

	StringType toBase16(const uchar* hash, size_t byteCount)
	{
		wchar result[ maxByteCount * 2 + 1 ];
		for ( size_t i = 0; i < byteCount; ++i )
		{
			result[ i * 2     ] = base16[ hash[ i ] / 16 ];
			result[ i * 2 + 1 ] = base16[ hash[ i ] % 16 ];
		}
		result[ byteCount * 2 ] = 0;
		return result;
	}

	bool fromBase16(uchar* hash, const wchar* input, size_t byteCount)
	{
		for ( size_t i = 0; i < byteCount; ++i )
		{
			uwchar tmp = input[ i * 2 ];
			if ( tmp - '0' < 10u )
				hash[ i ] = uchar( ( tmp - '0' ) * 16 );
			else if ( tmp - 'A' < 6u )
				hash[ i ] = uchar( ( tmp - 'A' + 10 ) * 16 );
			else if ( tmp - 'a' < 6u )
				hash[ i ] = uchar( ( tmp - 'a' + 10 ) * 16 );
			else
				return false;
			tmp = input[ i * 2 + 1 ];
			if ( tmp - '0' < 10u )
				hash[ i ] |= tmp - '0';
			else if ( tmp - 'A' < 6u )
				hash[ i ] |= tmp - 'A' + 10;
			else if ( tmp - 'a' < 6u )
				hash[ i ] |= tmp - 'a' + 10;
			else
				return false;
		}
		return true;
	}

	StringType toBase32(const uchar* hash, size_t byteCount)
	{
		const size_t base32Chars = ( byteCount * CHAR_BIT + 4 ) / 5;
		wchar result[ ( maxByteCount * CHAR_BIT + 4 ) / 5 + 1 ];

		int shift = 11;
		size_t i = 0, ch = 0;
		do
		{
			result[ ch ] = base32[ ( ( hash[ i ] * 256 + hash[ i + 1 ] )
				>> shift ) & 0x1f ];
			shift -= 5;
			if ( shift <= 0 )
			{
				shift += CHAR_BIT;
				++i;
			}
		}
		while ( ++ch < base32Chars - 1 );

		result[ base32Chars - 1 ]
				= base32[ ( hash[ byteCount - 1 ]
						<< ( base32Chars * 5 % CHAR_BIT ) ) & 0x1f ];
		result[ base32Chars ] = 0;
		return result;
	}

	bool fromBase32(uchar* hash, const wchar* input, size_t byteCount)
	{
		const size_t base32Chars = ( byteCount * CHAR_BIT + 4 ) / 5;

		int shift = 11;
		size_t ch = 0, i = 0;
		unsigned carry = 0;
		do
		{
			unsigned tmp;
			if ( ( tmp = input[ ch ] - 'A' ) < 26 )
				carry |= tmp << shift;
			else if ( ( tmp = input[ ch ] - 'a' ) < 26 )
				carry |= tmp << shift;
			else if ( ( tmp = input[ ch ] - '2' ) < 6 )
				carry |= ( tmp + 26 ) << shift;
			else
				return false;
			shift -= 5;
			if ( shift < 0 )
			{
				shift += 8;
				hash[ i++ ] = uchar( carry >> 8 & 0xff );
				carry <<= 8;
			}
		}
		while ( ++ch < base32Chars );
		hash[ i ] = uchar( carry >> 8 & 0xff );
		return true;
	}

} // namespace Hashes
