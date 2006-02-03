////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Hashes/HashStringConversion.cpp                                            //
//                                                                            //
// Copyright (C) 2005 Shareaza Development Team.                              //
// This file is part of SHAREAZA (www.shareaza.com).                          //
//                                                                            //
// Shareaza is free software; you can redistribute it                         //
// and/or modify it under the terms of the GNU General Public License         //
// as published by the Free Software Foundation; either version 2 of          //
// the License, or (at your option) any later version.                        //
//                                                                            //
// Shareaza is distributed in the hope that it will be useful,                //
// but WITHOUT ANY WARRANTY; without even the implied warranty of             //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       //
// See the GNU General Public License for more details.                       //
//                                                                            //
// You should have received a copy of the GNU General Public License          //
// along with Shareaza; if not, write to the                                  //
// Free Software Foundation, Inc,                                             //
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

//! \file       Hashes/HashStringConversion.cpp
//! \brief      Defines functions for conversion between hashes and strings.

#include "..\StdAfx.h"

namespace Hashes
{

	const wchar base16[] = L"0123456789abcdef";
	const wchar base32[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ234567=";

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
