//
// Hashes/Hash.hpp
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

//! \file       Hashes/Hash.hpp
//! \brief      Defines Hash template.

#pragma once

#include "HashStringConversion.hpp"


namespace Hashes
{
	//! \brief  The primary class template to define hash types.
	//!
	//! This class provides a policy based design to capture and define
	//! behavioural and structural properties of hashes. Unlike the reference
	//! example these policies are organized in a single inheritance tree. This
	//! avoids codebloat commonly found with multiple inheritance and it avoids
	//! some cumbersome workarounds, because these policies are not strictly
	//! orthogonal. While they can combined freely, policies may (and often do)
	//! depend on other policies they inherit from.
	//! \param DescriptorT       A model of \ref hashdescriptorpage
	//!                          "Hash Descriptor". Different instances that
	//!                          share the same descriptor are considered
	//!                          related and automatic conversion is possible.
	//! \param StoragePolicyT    A model of \ref hashstoragepoliciespage
	//!                          "Storage Policy".
	//! \param CheckingPolicyT   A model of \ref hashcheckingpoliciespage
	//!                          "Checking Policy".
	//! \param ValidationPolicyT A model of \ref hashvalidationpoliciespage
	//!                          "Validation Policy".
	template
	<
		typename DescriptorT,
		template<typename> class StoragePolicyT,
		template<typename> class CheckingPolicyT,
		template<typename> class ValidationPolicyT
	>
	class Hash : public ValidationPolicyT< CheckingPolicyT< StoragePolicyT< DescriptorT > > >
	{
	public:
		typedef DescriptorT Descriptor;
		typedef StoragePolicyT< Descriptor > StoragePolicy;
		typedef CheckingPolicyT< StoragePolicy > CheckingPolicy;
		typedef ValidationPolicyT< CheckingPolicy > ValidationPolicy;
		typedef typename Descriptor::WordType WordType;
		static const size_t wordCount = Descriptor::wordCount;
		static const size_t byteCount = Descriptor::byteCount;
		static const size_t numUrns = Descriptor::numUrns;
		typedef typename StoragePolicy::iterator iterator;
		typedef typename StoragePolicy::const_iterator const_iterator;
		typedef typename Descriptor::RawStorage RawStorage;
	public:
		//! Constructs an empty hash object. The exact semantics depends on
		//! policy parameters.
		Hash() : ValidationPolicy() {}

		//! Constructs a new hash object using a given byte stream.
		//! Tries to validate the input afterwards.
		Hash(const RawStorage& rhs) : ValidationPolicy( rhs ) {}

		//! Converts a related hash object. We do so by forwarding the parameter
		//! to each policy in turn.
		//! \note the implicit copy constructor is still generated and used
		//!       if the argument type matches exactly.
		template<
			template<typename> class OSP,
			template<typename> class OCP,
			template<typename> class OVP>
		Hash(const Hash< Descriptor, OSP, OCP, OVP >& rhs)
			: ValidationPolicy(
					static_cast< const OVP< OCP< OSP < Descriptor > > >& >(
					rhs ) )
		{}

		//! Assigns a related hash object. We do so by forwarding the parameter
		//! to each policy in turn.
		//! \note the implicit copy assignment operator is still generated and
		//!       used if the argument type matches exactly.
		template<
			template<typename> class OSP,
			template<typename> class OCP,
			template<typename> class OVP>
		Hash& operator=(const Hash< Descriptor, OSP, OCP, OVP >& rhs)
		{
			static_cast< ValidationPolicy& >( *this )
					= static_cast< const OVP< OCP< OSP < Descriptor > > >& >(
					rhs );
			return *this;
		}

		//! \brief Generates a hash string with the specified encoding.
		//!
		//! Generates a hash string with the specified encoding. The encoding
		//! cannot be deduced and must be specified explicitly. Returns an empty
		//! string if the hash is not valid.
		template<Encoding encoding>
		StringType toString() const
		{
			return isValid()
					? HashToString< encoding, byteCount >()( &( *this )[ 0 ] )
					: StringType();
		}
		//! \brief Generates a hash string using the default encoding for
		//!        this hash type.
		StringType toString() const
		{
			return toString< encoding >();
		}
		//! \brief Generates a urn string using the default encoding and the
		//!        specified urn prefix.
		//!
		//! Returns an empty string if the hash is not valid.
		//!
		//! \todo Add suitable compile time assertion to catch cases when we
		//!       try to use this function with a type that has no or not this
		//!       urn. Currently the compilation fails if the type has no urns
		//!       but the error message does not show the point of
		//!       instantiation.
		template<size_t urn>
		StringType toUrn() const
		{
			return isValid()
					? urns[ urn ].signature
							+ HashToString< encoding, byteCount >()( &( *this )[ 0 ] )
					: StringType();
		}
		//! \brief Generates a urn string using the default urn prefix.
		StringType toUrn() const { return toUrn< 0 >(); }
		//! \brief Generates a urn string using the default short urn prefix.
		StringType toShortUrn() const { return toUrn< 1 >(); }

		//! \brief Generates hash from a hash string with the specified
		//!        encoding.
		//!
		//! Generates hash from string with the specified encoding. The encoding
		//! cannot be deduced and must be specified explicitly. The string
		//! is validated afterwards. If the generation fails because either the
		//! string is not wellformed or the hash ist blacklisted, the hash will
		//! be cleared.
		template<Encoding encoding>
		bool fromString(const wchar* input)
		{
			if ( !HashFromString< encoding, byteCount >()( &( *this )[ 0 ], input ) )
				return false;
			if ( validate() ) return true;
			clear();
			return false;
		}
		//! \brief Generates hash from hash string using the default encoding
		//!        for this hash type.
		bool fromString(const wchar* input)
		{
			return fromString< encoding >( input );
		}
		//! \brief Generates hash from urn using the default encoding
		//!        for this hash type.
		//!
		//! \todo Add suitable compile time assertion to catch cases when we
		//!       try to use this function with a type that has no urns.
		//!       Currently the compilation fails but the error message does not
		//!       show the point of instantiation.
		template<Encoding encoding>
		bool fromUrn(const wchar* input)
		{
			size_t inputLen = _tcslen( input );
			for ( size_t i = 0; i < numUrns; ++i )
			{
				if ( inputLen >= urns[ i ].minimumLength
						&& _tcsnicmp( input, urns[ i ].signature,
								urns[ i ].signatureLength ) == 0 )
				{
					return fromString< encoding >( input + urns[ i ].hashOffset );
				}
			}
			return false;
		}
		//! \brief Generates hash from urn using the default encoding
		//!        for this hash type.
		bool fromUrn(const wchar* input)
		{
			return fromUrn< encoding >( input );
		}
	};

	//! \relates Hashes::Hash
	//! \brief Compares two related hashes for equality.
	//!
	//! The arguments are not tested for validity.
	template
	<
		typename Descriptor,
		template<typename> class SP, template<typename> class OSP,
		template<typename> class CP, template<typename> class OCP,
		template<typename> class VP, template<typename> class OVP
	>
	inline bool operator==(const Hash< Descriptor, SP, CP, VP >& lhs,
			const Hash< Descriptor, OSP, OCP, OVP >& rhs )
	{
		return lhs.alignedStorage() == rhs.alignedStorage();
	}

	//! \relates Hashes::Hash
	//! \brief Compares two related hashes for inequality.
	//!
	//! Equivalent to: !( lhs == rhs )
	template
	<
		typename Descriptor,
		template<typename> class SP, template<typename> class OSP,
		template<typename> class CP, template<typename> class OCP,
		template<typename> class VP, template<typename> class OVP
	>
	inline bool operator!=(const Hash< Descriptor, SP, CP, VP >& lhs,
			const Hash< Descriptor, OSP, OCP, OVP >& rhs )
	{
		return !( lhs == rhs );
	}

	//! \relates Hashes::Hash
	//! \brief Provides strict weak ordering for related hashes.
	//!
	//! The arguments are not tested for validity. If either argument
	//! is invalid, the result is undefined. The comparison is equavilant
	//! to a lexicographical ordering of its word array. Thus it does not
	//! provide the same ordering as comparing hash string would give.
	template
	<
		typename Descriptor,
		template<typename> class SP, template<typename> class OSP,
		template<typename> class CP, template<typename> class OCP,
		template<typename> class VP, template<typename> class OVP
	>
	inline bool operator<(const Hash< Descriptor, SP, CP, VP >& lhs,
			const Hash< Descriptor, OSP, OCP, OVP >& rhs )
	{
		return lhs.alignedStorage() < rhs.alignedStorage();
	}

	//! \relates Hashes::Hash
	//! \brief Provides strict weak ordering for related hashes.
	//!
	//! Equivalent to: rhs < rhs
	template
	<
		typename Descriptor,
		template<typename> class SP, template<typename> class OSP,
		template<typename> class CP, template<typename> class OCP,
		template<typename> class VP, template<typename> class OVP
	>
	inline bool operator>(const Hash< Descriptor, SP, CP, VP >& lhs,
			const Hash< Descriptor, OSP, OCP, OVP >& rhs )
	{
		return rhs < lhs;
	}

	//! \relates Hashes::Hash
	//! \brief Provides strict weak ordering for related hashes.
	//!
	//! Equivalent to: !( lhs < rhs )
	template
	<
		typename Descriptor,
		template<typename> class SP, template<typename> class OSP,
		template<typename> class CP, template<typename> class OCP,
		template<typename> class VP, template<typename> class OVP
	>
	inline bool operator>=(const Hash< Descriptor, SP, CP, VP >& lhs,
			const Hash< Descriptor, OSP, OCP, OVP >& rhs )
	{
		return !( lhs < rhs );
	}

	//! \relates Hashes::Hash
	//! \brief Provides strict weak ordering for related hashes.
	//!
	//! Equivalent to: !( rhs < rhs )
	template
	<
		typename Descriptor,
		template<typename> class SP, template<typename> class OSP,
		template<typename> class CP, template<typename> class OCP,
		template<typename> class VP, template<typename> class OVP
	>
	inline bool operator<=(const Hash< Descriptor, SP, CP, VP >& lhs,
			const Hash< Descriptor, OSP, OCP, OVP >& rhs )
	{
		return !( rhs < lhs );
	}

	//! \relates Hashes::Hash
	//! \brief Compares two related hashes for validaty and equality.
	//!
	//! This predicate returns true if and only if both arguments are valid
	//! and equal.
	template
	<
		typename Descriptor,
		template<typename> class SP, template<typename> class OSP,
		template<typename> class CP, template<typename> class OCP,
		template<typename> class VP, template<typename> class OVP
	>
	inline bool validAndEqual(const Hash< Descriptor, SP, CP, VP >& lhs,
			const Hash< Descriptor, OSP, OCP, OVP >& rhs )
	{
		return lhs.isValid() && rhs.isValid() && lhs == rhs;
	}

	//! \relates Hashes::Hash
	//! \brief Compares two related hashes for validaty and unequality.
	//!
	//! This predicate returns true if and only if both arguments are valid
	//! and unequal.
	template
	<
		typename Descriptor,
		template<typename> class SP, template<typename> class OSP,
		template<typename> class CP, template<typename> class OCP,
		template<typename> class VP, template<typename> class OVP
	>
	inline bool validAndUnequal(const Hash< Descriptor, SP, CP, VP >& lhs,
			const Hash< Descriptor, OSP, OCP, OVP >& rhs )
	{
		return lhs.isValid() && rhs.isValid() && lhs != rhs;
	}

	//! \namespace Hashes::Policies
	//! \brief This namespace is used to locate all possible Policies for
	//!        the Hash class template.
} // namespace Hashes
