////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Hashes.hpp                                                                 //
//                                                                            //
// Copyright (C) 2005 Shareaza Development Team.                              //
// This file is part of SHAREAZA (shareaza.sourceforge.net).                          //
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

//! \file       Hashes.hpp
//! \brief      General header files for Hashes.
//!
//! Includes headers for Hash class template. Defines all hash types as typedefs
//! to a Hash template class. Contains some utility functions and hash ids.

#ifndef HASHES_HPP_INCLUDED
#define HASHES_HPP_INCLUDED

#include "Hashes/Hash.hpp"
#include "Hashes/HashDescriptors.hpp"
#include "Hashes/StoragePolicies.hpp"
#include "Hashes/CheckingPolicies.hpp"
#include "Hashes/ValidationPolicies.hpp"
#include "Hashes/Compatibility.hpp"

//! \brief Contains all definitions related to the Hash class template.
//!
//! Definition of the Hash class template and related functions, types and
//! constants. Use ADL to find functions here.
namespace Hashes
{
	//! \brief The default SHA1 hash type.
	typedef Hash< Policies::Sha1Descriptor, Policies::ZeroInit,
			Policies::NoCheck, Policies::BasicValidation > Sha1Hash;
	//! \brief The SHA1 hash type suitable to represent conditions in
	//!        (partial) file objects.
	typedef Hash< Policies::Sha1Descriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::ExtendedValidation > Sha1ManagedHash;

	//! \brief The default Tiger hash type.
	typedef Hash< Policies::TigerDescriptor, Policies::ZeroInit,
			Policies::NoCheck, Policies::BasicValidation > TigerHash;
	//! \brief The Tiger hash type suitable to represent conditions in
	//!        (partial) file objects.
	typedef Hash< Policies::TigerDescriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::ExtendedValidation >
			TigerManagedHash;

	//! \brief The default ED2K hash type.
	typedef Hash< Policies::Ed2kDescriptor, Policies::ZeroInit,
			Policies::NoCheck, Policies::BasicValidation > Ed2kHash;
	//! \brief The ED2K hash type suitable to represent conditions in
	//!        (partial) file objects.
	typedef Hash< Policies::Ed2kDescriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::ExtendedValidation > Ed2kManagedHash;

	//! \brief The default MD5 hash type.
	typedef Hash< Policies::Md5Descriptor, Policies::ZeroInit,
			Policies::NoCheck, Policies::BasicValidation > Md5Hash;
	//! \brief The MD5 hash type suitable to represent conditions in
	//!        (partial) file objects.
	typedef Hash< Policies::Md5Descriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::ExtendedValidation > Md5ManagedHash;

	//! \brief The default Bittorrent info hash type.
	typedef Hash< Policies::BthDescriptor, Policies::ZeroInit,
			Policies::NoCheck, Policies::BasicValidation > BtHash;
	//! \brief The Bittorent info hash type suitable to represent conditions in
	//!        (partial) file objects.
	typedef Hash< Policies::BthDescriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::ExtendedValidation > BtManagedHash;
	//! \brief This Bittorrent hash type is useful to represent the hash of a
	//!        verification block (technically it's SHA1), no overhead
	typedef Hash< Policies::BthDescriptor, Policies::ZeroInit,
			Policies::NoCheck, Policies::NoValidation > BtPureHash;

	//! \brief The Guid hash type.
	typedef Hash< Policies::GuidDescriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::BasicValidation > Guid;

	//! \brief The Bittorrent Guid hash type.
	typedef Hash< Policies::BtGuidDescriptor, Policies::ZeroInit,
			Policies::ZeroCheck, Policies::BasicValidation > BtGuid;

	//! \brief Generates Guid from extended Bittorrent Guid
	inline Guid transformGuid(const BtGuid& other)
	{
		Guid result;
		memcpy( &result[ 0 ], &other[ 0 ], result.byteCount );
		result.validate();
		return result;
	}

	//! \brief Generates Bittorrent Guid from Guid, using Shareaza's special
	//! signature.
	inline BtGuid transformGuid(const Guid& other)
	{
		BtGuid result;
		memcpy( &result[ 0 ], &other[ 0 ], result.byteCount );
		*( result.end() - 1 )
				= *other.begin() ^ swapEndianess( *( other.end() - 1 ) );
		result.validate();
		return result;
	}

	//! \brief Predicate checking if a given Bittorrent Guid uses special
	//!        signature to signal it belong to Shareaza client.
	inline bool isExtendedBtGuid(const BtGuid& hash)
	{
		return hash.isValid() && *( hash.end() - 1 )
				== ( *hash.begin() ^ swapEndianess( *( hash.end() - 2 ) ) );
	}

} // namespace Hashes

//! Enumeration to select a specific hash type
enum
{
	HASH_NULL = 0,      //!< Unknown (any) hash type
	HASH_SHA1 = 1,      //!< use SHA1
	HASH_MD5 = 2,       //!< use MD5
	HASH_TIGERTREE = 3, //!< use Tiger tree hash
	HASH_ED2K = 4,      //!< use ED2K
	HASH_TORRENT = 5    //!< use Bittorrent info hash
};

#endif // #ifndef HASHES_HPP_INCLUDED
