//
// augment/auto_ptr.hpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

#pragma once


namespace augment
{

	template<typename element_type, bool opt = false>
	class auto_ptr_ref
	{
		template<typename> friend class auto_ptr;
		explicit auto_ptr_ref(const void** ref, element_type* ptr) throw()
			: ref_( ref ), ptr_( ptr )
		{}
		element_type* release() throw()
		{
			*ref_ = NULL;
			return ptr_;
		}
		const void** ref_;
		element_type* ptr_;
	};

	template<typename element_type>
	class auto_ptr_ref< element_type, true >
	{
		template<typename> friend class auto_ptr;
		explicit auto_ptr_ref(const void** ref, element_type*) throw()
			: ref_( ref )
		{}
		element_type* release() throw()
		{
			element_type* ptr = const_cast< element_type* >(
				static_cast< const element_type* >( *ref_ ) );
			*ref_ = NULL;
			return ptr;
		}
		const void** ref_;
	};

	// replacement for std::auto_ptr - this avoids certain bugs in the standard library of
	// vc++7.1 and vc++8.0, namely:
	// - undefined behaviour if used for upcasts in the presence of multiple or virtual inheritance
	// - vc++7.1 does not allow upcasts from rvalues
	// - vc++8.0 allows implicit creation of auto_ptr_ref from arbitray pointers in certain situations,
	//   but those auto_ptr_ref do not refer to auto_ptr
	template<typename T>
	class auto_ptr
	{
	public:
		typedef T element_type;
		explicit auto_ptr(element_type* ptr = NULL) throw()
			: ptr_( ptr )
		{}
		auto_ptr(auto_ptr& other) throw()
			: ptr_( other.release() )
		{}
		template<typename source_element_type>
		auto_ptr(auto_ptr< source_element_type >& other) throw()
			: ptr_( implicit_cast< element_type* >( other.release() ) )
		{}
		auto_ptr(auto_ptr_ref< element_type, false > other) throw()
			: ptr_( other.release() )
		{}
		auto_ptr(auto_ptr_ref< element_type, true > other) throw()
			: ptr_( other.release() )
		{}
		~auto_ptr() throw()
		{
			if ( get() != NULL )
				boost::checked_delete( get() );
		};

		auto_ptr& operator=(auto_ptr& other) throw()
		{
			reset( other.release() );
			return *this;
		}
		template<typename source_element_type>
		auto_ptr& operator=(auto_ptr< source_element_type >& other) throw()
		{
			reset( other.release() );
			return *this;
		}
		auto_ptr& operator=(auto_ptr_ref< element_type, false > other) throw()
		{
			reset( other.release() );
			return *this;
		}
		auto_ptr& operator=(auto_ptr_ref< element_type, true > other) throw()
		{
			reset( other.release() );
			return *this;
		}

		element_type* get() const throw()
		{
			return const_cast< element_type* >( static_cast< const element_type* >( ptr_ ) );
		}
		element_type& operator*() const throw()
		{
			return *get();
		}
		element_type* operator->() const throw()
		{
			return get();
		}
		element_type* release() throw()
		{
			element_type* ptr = get();
			ptr_ = NULL;
			return ptr;
		}
		void reset(element_type* ptr = NULL) throw()
		{
			if ( ptr != get() && get() != NULL )
				boost::checked_delete( get() );
			ptr_ = ptr;
		}

		template<typename target_element_type>
		operator auto_ptr_ref< target_element_type,
				boost::is_same< target_element_type, element_type >::value >() throw()
		{
			return auto_ptr_ref< target_element_type,
				boost::is_same< target_element_type, element_type >::value >( &ptr_, get() );
		}
	private:
		const void* ptr_;
	};

} // namespace augment
