////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Hashes/Compatibility.hpp                                                   //
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

//! \file       Hashes/Compatibility.hpp
//! \brief      Defines functions to interface with legacy and MFC code.
//!
//! This file contains function definitions to emulate the old serialization
//! functions. This should be replaced once a new serialisation method has been
//! adopted.

#ifndef HASHES_COMPATIBILITY_HPP_INCLUDED
#define HASHES_COMPATIBILITY_HPP_INCLUDED

namespace Hashes
{

	template
	<
		typename Descriptor,
		template<typename> class StoragePolicy,
		template<typename> class CheckingPolicy,
		template<typename> class ValidationPolicy
	>
	inline void SerializeOut(CArchive& ar, const Hash< Descriptor, StoragePolicy,
			CheckingPolicy, ValidationPolicy >& out);

	template
	<
		typename Descriptor,
		template<typename> class StoragePolicy,
		template<typename> class CheckingPolicy
	>
	inline void SerializeOut(CArchive& ar, const Hash< Descriptor, StoragePolicy,
			CheckingPolicy, Policies::NoValidation >& out)
	{
		ASSERT( ar.IsStoring() );
		ar.Write( out.begin(), out.byteCount );
	}

	template
	<
		typename Descriptor,
		template<typename> class StoragePolicy,
		template<typename> class CheckingPolicy
	>
	inline void SerializeOut(CArchive& ar, const Hash< Descriptor, StoragePolicy,
			CheckingPolicy, Policies::BasicValidation >& out)
	{
		ASSERT( ar.IsStoring() );
		uint32 bValid = bool( out );
		ar << bValid;
		if ( bValid ) ar.Write( out.begin(), out.byteCount );
	}

	template
	<
		typename Descriptor,
		template<typename> class StoragePolicy,
		template<typename> class CheckingPolicy
	>
	inline void SerializeOut(CArchive& ar, const Hash< Descriptor, StoragePolicy,
			CheckingPolicy, Policies::ExtendedValidation >& out)
	{
		ASSERT( ar.IsStoring() );
		uint32 bValid = bool( out );
		ar << bValid;
		if ( bValid ) ar.Write( out.begin(), out.byteCount );
		uint32 bTrusted = out.isTrusted();
		ar << bTrusted;
	}

	template
	<
		typename Descriptor,
		template<typename> class StoragePolicy,
		template<typename> class CheckingPolicy,
		template<typename> class ValidationPolicy
	>
	inline void SerializeIn(CArchive& ar, Hash< Descriptor, StoragePolicy,
			CheckingPolicy, ValidationPolicy >& in, int version);

	template
	<
		typename Descriptor,
		template<typename> class StoragePolicy,
		template<typename> class CheckingPolicy
	>
	inline void SerializeIn(CArchive& ar, Hash< Descriptor, StoragePolicy,
			CheckingPolicy, Policies::NoValidation >& in, int /*version*/)
	{
		ASSERT( ar.IsLoading() );
		ReadArchive( ar, in.begin(), in.byteCount );
	}

	template
	<
		typename Descriptor,
		template<typename> class StoragePolicy,
		template<typename> class CheckingPolicy
	>
	inline void SerializeIn(CArchive& ar, Hash< Descriptor, StoragePolicy,
			CheckingPolicy, Policies::BasicValidation >& in, int /*version*/)
	{
		ASSERT( ar.IsLoading() );
		uint32 bValid;
		ar >> bValid;
		if ( bValid )
		{
			ReadArchive( ar, in.begin(), in.byteCount );
			in.validate();
		}
		else
		{
			in.clear();
		}
	}

	template
	<
		typename Descriptor,
		template<typename> class StoragePolicy,
		template<typename> class CheckingPolicy
	>
	inline void SerializeIn(CArchive& ar, Hash< Descriptor, StoragePolicy,
			CheckingPolicy, Policies::ExtendedValidation >& in, int version)
	{
		ASSERT( ar.IsLoading() );
		uint32 bValid;
		ar >> bValid;
		if ( bValid )
		{
			ReadArchive( ar, in.begin(), in.byteCount );
			in.validate();
		}
		else
		{
			in.clear();
		}
		uint32 bTrusted = bValid;
		if ( version >= 31 ) ar >> bTrusted;
		if ( bTrusted ) in.signalTrusted();
	}

} // namespace Hashes

#endif // #ifndef HASHES_COMPATIBILITY_HPP_INCLUDED
