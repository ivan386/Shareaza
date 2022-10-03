////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Hashes/CheckingPolicies.hpp                                                //
//                                                                            //
// Copyright (C) 2005-2014 Shareaza Development Team.                         //
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

//! \file       Hashes/CheckingPolicies.hpp
//! \brief      Defines checking policies.

#ifndef HASHES_CHECKINGPOLICIES_HPP_INCLUDED
#define HASHES_CHECKINGPOLICIES_HPP_INCLUDED

namespace Hashes
{

	namespace Policies
	{

		//! \brief A model of \ref hashcheckingpoliciespage "Checking Policy"
		//!
		//! This policy does not perform any checking.
		//! Thus \a check() always returns \c true.
		template<class StoragePolicyT>
		struct NoCheck : public StoragePolicyT
		{
		public:
			typedef StoragePolicyT StoragePolicy;
			typedef typename StoragePolicy::Descriptor Descriptor;
			typedef typename Descriptor::WordType WordType;
			static const size_t wordCount = Descriptor::wordCount;
			static const size_t byteCount = Descriptor::byteCount;
			typedef typename StoragePolicy::const_iterator const_iterator;
			typedef typename StoragePolicy::RawStorage RawStorage;

			NoCheck() : StoragePolicy() {}
			NoCheck(const RawStorage& rhs)
				: StoragePolicy( rhs )
			{}

			template<template<typename> class OCP, typename OSP>
			NoCheck(const OCP< OSP >& rhs)
				: StoragePolicy( static_cast< const OSP& >( rhs ) )
			{}

			template<template<typename> class OCP, typename OSP>
			NoCheck& operator=(const OCP< OSP >& rhs)
			{
				static_cast< StoragePolicy& >( *this )
						= static_cast< const OSP& >( rhs );
				return *this;
			}

			bool check() const { return true; }
		};

		//! \brief A model of \ref hashcheckingpoliciespage "Checking Policy"
		//!
		//! This policy checks whether the current value is all 0.
		//! In that case \a check() fails. This works nicely in conjunction
		//! with Hashes::Policies::ZeroInit to catch uninitialized hashes.
		template<class StoragePolicyT>
		struct ZeroCheck : public StoragePolicyT
		{
		public:
			typedef StoragePolicyT StoragePolicy;
			typedef typename StoragePolicy::Descriptor Descriptor;
			typedef typename Descriptor::WordType WordType;
			static const size_t wordCount = Descriptor::wordCount;
			static const size_t byteCount = Descriptor::byteCount;
			typedef typename StoragePolicy::const_iterator const_iterator;
			typedef typename StoragePolicy::RawStorage RawStorage;

			ZeroCheck() : StoragePolicy() {}
			ZeroCheck(const RawStorage& rhs)
				: StoragePolicy( rhs )
			{}
			template<template<typename> class OCP, typename OSP>
			ZeroCheck(const OCP< OSP >& rhs)
				: StoragePolicy( static_cast< const OSP& >( rhs ) )
			{}

			template<template<typename> class OCP, typename OSP>
			ZeroCheck& operator=(const OCP< OSP >& rhs)
			{
				static_cast< StoragePolicy& >( *this )
						= static_cast< const OSP& >( rhs );
				return *this;
			}
		private:
			template<size_t index> struct CheckHelper
			{
				bool operator()(const_iterator storage)
				{
					return *storage != 0
							|| CheckHelper< index - 1 >()( storage + 1 );
				}
			};
			template<> struct CheckHelper< 1 >
			{
				bool operator()(const_iterator storage)
				{
					return *storage != 0;
				}
			};
		public:
			bool check() const { return CheckHelper< wordCount >()( begin() ); }
		};

		//! \brief A model of \ref hashcheckingpoliciespage "Checking Policy"
		//!
		//! This policy is a refinement of Hashes::Policies::ZeroCheck.
		//! It checks the current value against the global blacklist
		//! for that hash type in addition to the check against 0.
		template<class StoragePolicyT>
		struct GlobalCheck : public ZeroCheck< StoragePolicyT >
		{
		public:
			typedef StoragePolicyT StoragePolicy;
			typedef typename StoragePolicy::Descriptor Descriptor;
			typedef typename Descriptor::WordType WordType;
			static const size_t wordCount = Descriptor::wordCount;
			static const size_t byteCount = Descriptor::byteCount;
			typedef typename StoragePolicy::const_iterator const_iterator;
			typedef typename StoragePolicy::RawStorage RawStorage;

			GlobalCheck() : StoragePolicy() {}
			GlobalCheck(const RawStorage& rhs)
				: ZeroCheck< StoragePolicy >( rhs )
			{}
			template<template<typename> class OCP, typename OSP>
			GlobalCheck(const OCP< OSP >& rhs)
				: ZeroCheck< StoragePolicy >( rhs )
			{}

			template<template<typename> class OCP, typename OSP>
			GlobalCheck& operator=(const OCP< OSP >& rhs)
			{
				static_cast< ZeroCheck< StoragePolicy >& >( *this ) = rhs;
				return *this;
			}
			bool check() const
			{
				return ZeroCheck< StoragePolicy >::check()
						&& !std::binary_search( blackList.begin(),
							blackList.end(), alignedStorage() );
			}
		};

		//! \brief A model of \ref hashcheckingpoliciespage "Checking Policy"
		//!
		//! This policy is a refinement of Hashes::Policies::GlobalCheck
		//! It checks the current value against the global blacklist 
		//! and a local blacklist.\n
		//! The local blacklist is local to a particular object and not copied
		//! when the object is copied.\n
		//! Rationale:\n
		//! A local blacklist is usually generated in steps, starting from
		//! an empty list, adding values as new hashes are blacklisted.
		//! This means that we want to keep the current blacklist when we
		//! try to assign a new hash. It would be surprising if that wasn't
		//! the case too if we assign a hash type that by chance happens to
		//! use this policy as well. Typically a local blacklist has only
		//! meaning local to the context in which the hash object it belongs to
		//! exists. If for any reasons a local blacklist needs to be copied,
		//! this has to be done in an additional step, by simply assigning
		//! blacklist() (because these return references).
		template<class StoragePolicyT>
		struct LocalCheck : public GlobalCheck< StoragePolicyT >
		{
		public:
			typedef StoragePolicyT StoragePolicy;
			typedef typename StoragePolicy::Descriptor Descriptor;
			typedef typename Descriptor::WordType WordType;
			static const size_t wordCount = Descriptor::wordCount;
			static const size_t byteCount = Descriptor::byteCount;
			typedef typename StoragePolicy::const_iterator const_iterator;
			typedef typename StoragePolicy::RawStorage RawStorage;
			typedef typename StoragePolicy::AlignedStorage AlignedStorage;

			LocalCheck() : StoragePolicy() {}
			LocalCheck(const RawStorage& rhs)
				: GlobalCheck< StoragePolicy >( rhs )
			{}
			template<template<typename> class OCP, typename OSP>
			LocalCheck(const OCP< OSP >& rhs)
				: GlobalCheck< StoragePolicy >( rhs )
			{}

			template<template<typename> class OCP, typename OSP>
			LocalCheck& operator=(const OCP< OSP >& rhs)
			{
				static_cast< GlobalCheck< StoragePolicy >& >( *this ) = rhs;
				return *this;
			}
			bool check() const
			{
				return GlobalCheck< StoragePolicy >::check()
						&& !std::binary_search( m_blacklist.begin(),
						m_blacklist.end(), alignedStorage() );
			}
			//! \brief Returns the local blacklist.
			std::vector< AlignedStorage >& blacklist() { return m_blacklist; }
			const std::vector< AlignedStorage >& blacklist() const
			{
				return m_blacklist;
			}
			//! \brief Add an entry to the blacklist.
			//!
			//! If no argument is given, the current value of this hash is used.
			void addBlacklisted(const AlignedStorage&
					storage = alignedStorage())
			{
				// we don't want duplicates
				if ( !std::binary_search( m_blacklist.begin(),
						m_blacklist.end(), storage ) )
				{
					m_blacklist.insert( std::lower_bound( m_blacklist.begin(),
						m_blacklist.end(), storage() ), storage() );
				}
			}
			//! \brief Remove an entry from the blacklist.
			//!
			//! If no argument is given, the current value of this hash is used.
			void removeBlackListed(const AlignedStorage&
					storage = alignedStorage())
			{
				if ( std::binary_search( m_blacklist.begin(),
						m_blacklist.end(), storage() ) )
				{
					m_blacklist.erase( std::lower_bound( m_blacklist.begin(),
						m_blacklist.end(), storage() ) );
				}
			}
		private:
			std::vector< AlignedStorage > m_blacklist;
		};

	} // namespace Policies

} // namespace Hashes

#endif // #ifndef HASHES_CHECKINGPOLICIES_HPP_INCLUDED
