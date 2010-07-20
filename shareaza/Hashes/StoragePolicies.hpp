//
// Hashes/StoragePolicies.hpp
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

//! \file       Hashes/StoragePolicies.hpp
//! \brief      Defines storage policies.

#pragma once


namespace Hashes
{
	namespace Policies
	{

		//! \brief A model of \ref hashstoragepoliciespage "Storage Policy"
		//!
		//! This policy zero-initializes the hash value upon default construction
		//! and when clearing the hash.
		template<typename DescriptorT>
		struct ZeroInit : public DescriptorT
		{
		public:
			// types
			//! \brief  Simple typedef forwarding the template parameter
			typedef DescriptorT Descriptor;
			typedef typename Descriptor::WordType WordType;
			typedef typename Descriptor::RawStorage RawStorage;
			typedef typename Descriptor::AlignedStorage AlignedStorage;

			//! \brief  Defines an STL style random access iterator to
			//!         access the hash content.
			typedef typename AlignedStorage::iterator iterator;
			//! \brief  Defines an STL style random access const iterator to
			//!         access the hash content.
			typedef typename AlignedStorage::const_iterator const_iterator;

			// Constructors
			ZeroInit() : Descriptor(), m_storage() {}
			ZeroInit(iterator input)
				: Descriptor()
			{
				CopyMemory( &*begin(), &*input, byteCount );
			}
			ZeroInit(const RawStorage& rhs)
				: Descriptor(), m_storage( rhs )
			{}
			template<template<typename> class OtherStoragePolicy>
			ZeroInit(const OtherStoragePolicy< Descriptor >& rhs)
				: Descriptor(), m_words( rhs.alignedStorage() )
			{}
			template<template<typename> class OtherStoragePolicy>
			ZeroInit& operator=(const OtherStoragePolicy< Descriptor >& rhs)
			{
				alignedStorage() = rhs.alignedStorage();
				return *this;
			}

			void clear()
			{
				ZeroMemory( &*begin(), byteCount );
			}

			uchar& operator[](size_t index) { return m_storage[ index ]; }
			const uchar& operator[](size_t index) const
			{
				return m_storage[ index ];
			}

			RawStorage& storage() { return m_storage; }
			const RawStorage& storage() const { return m_storage; }

			AlignedStorage& alignedStorage() { return m_words; }
			const AlignedStorage& alignedStorage() const { return m_words; }

			iterator       begin()       { return m_words.begin(); }
			const_iterator begin() const { return m_words.begin(); }
			iterator       end()         { return m_words.end(); }
			const_iterator end()   const { return m_words.end(); }

		private:
			union
			{
				RawStorage m_storage;
				AlignedStorage m_words;
			};
		};

		//! \brief A model of \ref hashstoragepoliciespage "Storage Policy"
		//!
		//! This policy does not initializes the hash value upon default
		//! construction and leaves the hash unchanged when clearing.
		template<typename DescriptorT>
		struct NoInit : public DescriptorT
		{
		public:
			typedef DescriptorT Descriptor;
			typedef typename Descriptor::WordType WordType;
			static const size_t wordCount = Descriptor::wordCount;
			static const size_t byteCount = Descriptor::byteCount;
			typedef typename Descriptor::RawStorage RawStorage;
			typedef typename Descriptor::AlignedStorage AlignedStorage;

			typedef typename AlignedStorage::iterator iterator;
			typedef typename AlignedStorage::const_iterator const_iterator;

			NoInit() : Descriptor() {}
			NoInit(iterator input)
				: Descriptor()
			{
				CopyMemory( &*begin(), &*input, byteCount );
			}
			NoInit(const RawStorage& rhs)
				: Descriptor(), m_storage( rhs )
			{}
			template<template<typename> class OtherStoragePolicy>
			NoInit(const OtherStoragePolicy< Descriptor >& rhs)
				: Descriptor(), m_words( rhs.alignedStorage() )
			{}
			template<template<typename> class OtherStoragePolicy>
			NoInit& operator=(const OtherStoragePolicy< Descriptor >& rhs)
			{
				alignedStorage() = rhs.alignedStorage();
				return *this;
			}
			void clear() {}

			uchar& operator[](size_t index) { return m_storage[ index ]; }
			const uchar& operator[](size_t index) const
			{
				return m_storage[ index ];
			}

			RawStorage& storage() { return m_storage; }
			const RawStorage& storage() const { return m_storage; }

			AlignedStorage& alignedStorage() { return m_words; }
			const AlignedStorage& alignedStorage() const { return m_words; }

			iterator       begin()       { return m_words.begin(); }
			const_iterator begin() const { return m_words.begin(); }
			iterator       end()         { return m_words.end(); }
			const_iterator end()   const { return m_words.end(); }

		private:
			union
			{
				RawStorage m_storage;
				AlignedStorage m_words;
			};
		};

	} // namespace Policies

} // namespace Hashes
