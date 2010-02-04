//
// Hashes/HashStringConversion.hpp
//
// Copyright (c) Shareaza Development Team, 2005-2010.
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

//! \file       Hashes/HashStringConversion.hpp
//! \brief      Declares functions for conversion between hashes and strings.

#pragma once


namespace Hashes
{
	//! This defines the maximum length of a uchar array supported by
	//! hash-string conversion functions.
	//! \warning Using these conversion functions with uchar arrays
	//!          larger than Hashes::maxByteCount results in buffer overflow.
	//! \note When calling these functions from within the Hash template
	//!       this limit is already being taken care of by means of a
	//!       static (=compile time) assertion.
	const size_t maxByteCount = 24;

	//! \brief Specifies the encoding used for hash strings and urns.
	//!
	//! This enumeration specifies the encoding used to generate hash strings
	//! or urns or when reading from such a string.
	enum Encoding
	{
		//! This encoding is specific to Guids.
		//! <b>It cannot be used to read from a string.</b>
		guidEncoding = 0,
		base16Encoding = 4, //!< Encodes the hash as hexadezimal string.
		base32Encoding = 5  //!< Uses Base32 encoding.
	};

	//! \brief Encodes a uchar array as guid string without "{}".
	StringType toGuid(const uchar* hash);
	//! \brief Encodes a CLSID as guid string with or without "{}".
	StringType toGuid(REFCLSID hash, bool enclosed = true);
	//! \brief Encodes a uchar array of given range as hex string.
	StringType toBase16(const uchar* hash, size_t byteCount);
	//! \brief Encodes a uchar array of given range as Base32 string.
	StringType toBase32(const uchar* hash, size_t byteCount);
	//! \brief Reads from a guid encoded string with or without "{}" into a uchar array.
	bool fromGuid(uchar* hash, const wchar* input);
	//! \brief Reads from a guid encoded string with or without "{}" into a CLSID.
	bool fromGuid(LPCTSTR input, LPVOID hash);
	//! \brief Reads from a hex encoded string into a uchar array.
	bool fromBase16(uchar* hash, const wchar* input, size_t byteCount);
	//! \brief Reads from a Base32 encoded string into a uchar array.
	bool fromBase32(uchar* hash, const wchar* input, size_t byteCount);

	template<Encoding encoding, size_t byteCount> struct HashToString;
	template<Encoding encoding, size_t byteCount> struct HashFromString;

	//! \brief Helper template to forward calls to the guid encoding
	//!        function.
	template<size_t byteCount>
	struct HashToString< guidEncoding, byteCount >
	{
		BOOST_STATIC_ASSERT( byteCount <= maxByteCount );
		inline StringType operator()(const uchar* hash) const
		{
			return toGuid( hash );
		}
	};

	//! \brief Helper template to forward calls to the base16 encoding
	//!        function.
	template<size_t byteCount>
	struct HashToString< base16Encoding, byteCount >
	{
		BOOST_STATIC_ASSERT( byteCount <= maxByteCount );
		inline StringType operator()(const uchar* hash) const
		{
			return toBase16( hash, byteCount );
		}
	};

	//! \brief Helper template to forward calls to the base32 encoding
	//!        function.
	template<size_t byteCount>
	struct HashToString< base32Encoding, byteCount >
	{
		BOOST_STATIC_ASSERT( byteCount <= maxByteCount );
		inline StringType operator()(const uchar* hash) const
		{
			return toBase32( hash, byteCount );
		}
	};

	//! \brief Helper template to forward calls to the guid encoding
	//!        function.
	template<size_t byteCount>
	struct HashFromString< guidEncoding, byteCount >
	{
		inline bool operator()(uchar* hash, const wchar* input) const
		{
			return fromGuid( hash, input );
		}
	};

	//! \brief Helper template to forward calls to the base16 encoding
	//!        function.
	template<size_t byteCount>
	struct HashFromString< base16Encoding, byteCount >
	{
		inline bool operator()(uchar* hash, const wchar* input) const
		{
			return fromBase16( hash, input, byteCount );
		}
	};

	//! \brief Helper template to forward calls to the base32 encoding
	//!        function.
	template<size_t byteCount>
	struct HashFromString< base32Encoding, byteCount >
	{
		inline bool operator()(uchar* hash, const wchar* input) const
		{
			return fromBase32( hash, input, byteCount );
		}
	};

} // namespace Hashes
