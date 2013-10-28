////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Hashes/ValidationPolicies.hpp                                              //
//                                                                            //
// Copyright (C) 2005-2010 Shareaza Development Team.                         //
// This file is part of SHAREAZA (shareaza.sourceforge.net).                  //
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

//! \file       Hashes/ValidationPolicies.hpp
//! \brief      Defines validation policies.

#ifndef HASHES_VALIDATIONPOLICIES_HPP_INCLUDED
#define HASHES_VALIDATIONPOLICIES_HPP_INCLUDED

namespace Hashes
{

	namespace Policies
	{

		//! \brief A model of
		//!        \ref hashvalidationpoliciespage "Validation Policy"
		//!
		//! This policy does not store the result of \a check().
		//! Its member function \a isValid() will always return \c true in turn.
		//! It is the responsibility of the calling code, to ensure that this
		//! makes sense. Use this policy to avoid overhead were it's not needed.
		//! In conjunction with Hashes::Policies::NoCheck it results in a hash
		//! type thats as space efficient as a plain struct would be.
		template<class CheckingPolicyT>
		struct NoValidation : public CheckingPolicyT
		{
		public:
			typedef CheckingPolicyT CheckingPolicy;
			typedef typename CheckingPolicy::StoragePolicy StoragePolicy;
			typedef typename StoragePolicy::RawStorage RawStorage;

			NoValidation() : CheckingPolicy() {}
			NoValidation(const RawStorage& rhs)
				: CheckingPolicy( rhs )
			{}

			template<template<typename> class OVP, typename OCP>
			NoValidation(const OVP< OCP >& rhs)
				: CheckingPolicy( static_cast< const OCP& >( rhs ) )
			{}

			template<template<typename> class OVP, typename OCP>
			NoValidation& operator=(const OVP< OCP >& rhs)
			{
				static_cast< CheckingPolicy& >( *this )
						= static_cast< const OCP& >( rhs );
				return *this;
			}

			void clear() { CheckingPolicy::clear(); }
			bool validate() { return check(); }
			bool isValid() const { return true; }
		};

		//! \brief A model of 
		//!        \ref hashvalidationpoliciespage "Validation Policy"
		//!
		template<class CheckingPolicyT>
		struct BasicValidation : public CheckingPolicyT
		{
		private:
			struct SafeBoolHelper
			{
				void trueValue() {}
			};
			typedef void (SafeBoolHelper::*SafeBool)();
		public:
			typedef CheckingPolicyT CheckingPolicy;
			typedef typename CheckingPolicy::StoragePolicy StoragePolicy;
			typedef typename StoragePolicy::RawStorage RawStorage;

			BasicValidation()
				: CheckingPolicy(), m_valid( false )
			{}
			BasicValidation(const RawStorage& rhs)
				: CheckingPolicy( rhs )
			{ validate(); }

			template<template<typename> class OVP, typename OCP>
			BasicValidation(const OVP< OCP >& rhs)
				: CheckingPolicy( static_cast< const OCP& >( rhs ) )
				, m_valid( rhs.isValid() && check() )
			{}

			template<template<typename> class OVP, typename OCP>
			BasicValidation& operator=(const OVP< OCP >& rhs)
			{
				static_cast< CheckingPolicy& >( *this )
						= static_cast< const OCP& >( rhs );
				m_valid = rhs.isValid() && check();
				return *this;
			}

			void clear() { CheckingPolicy::clear(); m_valid = false; }
			bool validate() { return m_valid = check(); }

			bool isValid() const { return m_valid; }
			operator SafeBool() const
			{
				return m_valid ? &SafeBoolHelper::trueValue : 0;
			}

		private:
			bool m_valid;
		};

		//! \brief A model of
		//!        \ref hashvalidationpoliciespage "Validation Policy"
		//!
		//! This policy is a refinement of Hashes::Policies::BasicValidation.
		//! It associates another flag "trusted" with the hash.
		//! The exact semantics of trusted is unspecified, except that:
		//! - The flag can be set only if the hash is valid.
		//! - This flag is cleared if conversion from another Validation
		//!   Policy occurs. It is copied if copying from another hash with
		//!   this Policy provided the hash can be validated here.
		template<class CheckingPolicyT>
		struct ExtendedValidation : public BasicValidation< CheckingPolicyT >
		{
		public:
			typedef CheckingPolicyT CheckingPolicy;
			typedef typename CheckingPolicy::StoragePolicy StoragePolicy;
			typedef typename StoragePolicy::RawStorage RawStorage;

			ExtendedValidation()
				: BasicValidation< CheckingPolicy >(), m_trusted( false )
			{}
			ExtendedValidation(const RawStorage& rhs)
				: BasicValidation< CheckingPolicy >( rhs ), m_trusted( false )
			{}

			template<template<typename> class OVP, typename OCP>
			ExtendedValidation(const OVP< OCP >& rhs)
				: BasicValidation< CheckingPolicy >( rhs )
				, m_trusted( false )
			{}
			template<typename OCP>
			ExtendedValidation(const ExtendedValidation< OCP >& rhs)
				: BasicValidation< CheckingPolicy >( rhs )
				, m_trusted( isValid() && rhs.trusted() )
			{}

			template<template<typename> class OVP, typename OCP>
			ExtendedValidation& operator=(const OVP< OCP >& rhs)
			{
				static_cast< BasicValidation< CheckingPolicy >& >( *this )
						= rhs;
				m_trusted = false;
				return *this;
			}
			template<typename OCP>
			ExtendedValidation& operator=(const ExtendedValidation< OCP >& rhs)
			{
				static_cast< BasicValidation& >( *this )
					= static_cast< const BasicValidation< OCP >& >( rhs );
				m_trusted = isValid() && rhs.trusted();
				return *this;
			}

			//! \brief Overides \a clear() in order to clear the trusted flag
			//!        as well
			void clear()
			{
				BasicValidation< CheckingPolicy >::clear();
				m_trusted = false;
			}
			//! \brief Overrides \a validate() in order to clear the trusted
			//!        flag if validation fails.
			bool validate()
			{
				if ( BasicValidation< CheckingPolicy >::validate() )
					return true;
				m_trusted = false;
				return false;
			}

			//! \brief Sets trusted flag, provided the Hash is valid.
			//!        Returns the valid state of the hash.
			bool signalTrusted()
			{
				if ( ! isValid() )
					return false;
				m_trusted = true;
				return true;
			}
			//! \brief Clears the trusted flag.
			void signalUntrusted()
			{
				m_trusted = false;
			}
			//! \brief Queries the trusted flag.
			bool isTrusted() const { return m_trusted; }

		private:
			bool m_trusted;
		};

	} // Policies

} // namespace Hashes

#endif // #ifndef HASHES_VALIDATIONPOLICIES_HPP_INCLUDED
