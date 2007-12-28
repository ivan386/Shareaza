////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// augment/com_ptr.hpp                                                       //
//                                                                            //
// Copyright (C) 2002-2007 Shareaza Development Team.                         //
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

#ifndef AUGMENT_COM_PTR_HPP_INCLUDED
#define AUGMENT_COM_PTR_HPP_INCLUDED

inline void intrusive_ptr_add_ref(IUnknown* p)
{
	p->AddRef();
}
inline void intrusive_ptr_release(IUnknown* p)
{
	p->Release();
}

namespace augment
{

	template<typename T>
	class com_ptr
	{
		struct SafeBoolHelper
		{
			void trueValue() {}
		};
		typedef void (SafeBoolHelper::*SafeBool)();
	public:
		typedef T element_type;
		typedef element_type interface_type;
		BOOST_STATIC_ASSERT(( boost::is_convertible< element_type*, IUnknown* >::value ));
		com_ptr()
			: p_()
		{}
		template<typename source_interface_type>
		com_ptr(source_interface_type* p, bool add_ref = true, typename boost::enable_if_c<
				boost::is_convertible< source_interface_type*, interface_type* >::value >::type* = NULL)
			: p_( p, add_ref )
		{}
		template<typename source_interface_type>
		com_ptr(source_interface_type* p, typename boost::enable_if_c< !boost::is_convertible<
				source_interface_type*, interface_type* >::value >::type* = NULL)
			: p_( queryInterface( p ), false )
		{}
		template<typename source_interface_type>
		com_ptr(const com_ptr< source_interface_type >& p, typename boost::enable_if_c<
				boost::is_convertible< source_interface_type*, interface_type* >::value >::type* = NULL)
			: p_( p )
		{}
		template<typename source_interface_type>
		com_ptr(const com_ptr< source_interface_type >& p, typename boost::enable_if_c<
				!boost::is_convertible< source_interface_type*, interface_type* >::value >::type* = NULL)
			: p_( queryInterface( p.get() ), false )
		{}
		// implicit copy-constructor here

		template<typename source_interface_type>
		com_ptr& operator=(source_interface_type* p)
		{
			reset( p );
			return *this;
		}
		template<typename source_interface_type>
		com_ptr& operator=(const com_ptr< source_interface_type >& p)
		{
			reset( p.get() );
			return *this;
		}
		// implicit copy-assignment operator

		// implicit destructor

		interface_type* get() const
		{
			return p_.get();
		}
		interface_type& operator*() const
		{
			return *get();
		}
		interface_type* operator->() const
		{
			return get();
		}
		template<typename source_interface_type>
		void reset(source_interface_type* p)
		{
			p_ = p;
		}
		template<typename source_interface_type>
		void reset(source_interface_type* p, bool add_ref,
				typename boost::enable_if_c< boost::is_convertible<
						source_interface_type*, interface_type* >::value >::type* = NULL)
		{
			if ( add_ref )
				p_ = p;
			else
				*this = com_ptr( p, false );
		}
		void swap(com_ptr& other)
		{
			p_.swap( other.p_ );
		}
		operator SafeBool() const
		{
			return get() ? &SafeBoolHelper::trueValue : NULL;
		}
		bool operator!() const
		{
			return !get();
		}
	private:
		template<typename source_interface_type>
		static interface_type* queryInterface(source_interface_type* p,
				typename boost::enable_if_c<
					!boost::is_convertible< source_interface_type*, interface_type* >::value
					&& boost::is_convertible< source_interface_type*, IUnknown* >::value >::type* = NULL)
		{
			void* result = NULL;
			return p->QueryInterface( __uuidof( interface_type ), &result ) == S_OK
				? static_cast< interface_type* >( result )
				: NULL;
		}
		template<typename source_interface_type>
		static interface_type* queryInterface(source_interface_type* p,
				typename boost::enable_if_c<
					boost::is_convertible< source_interface_type*, interface_type* >::value
					|| !boost::is_convertible< source_interface_type*, IUnknown* >::value >::type* = NULL)
		{
			BOOST_STATIC_ASSERT( false );
		}

		boost::intrusive_ptr< interface_type > p_;
	};

	template<typename Lhs, typename Rhs>
    bool operator<(const com_ptr< Lhs >& lhs, const com_ptr< Rhs >& rhs)
	{
		return std::less< IUnknown* >()( lhs.get(), rhs.get() );
	}
	template<typename Lhs, typename Rhs>
    bool operator>(const com_ptr< Lhs >& lhs, const com_ptr< Rhs >& rhs)
	{
		return rhs < lhs;
	}
	template<typename Lhs, typename Rhs>
    bool operator<=(const com_ptr< Lhs >& lhs, const com_ptr< Rhs >& rhs)
	{
		return !( rhs < lhs );
	}
	template<typename Lhs, typename Rhs>
    bool operator>=(const com_ptr< Lhs >& lhs, const com_ptr< Rhs >& rhs)
	{
		return !( lhs < ths );
	}
	template<typename Lhs, typename Rhs>
    bool operator==(const com_ptr< Lhs >& lhs, const com_ptr< Rhs >& rhs)
	{
		return std::equal_to< IUnknown* >()( lhs.get(), rhs.get() );
	}
	template<typename Lhs, typename Rhs>
    bool operator!=(const com_ptr< Lhs >& lhs, const com_ptr< Rhs >& rhs)
	{
		return !( lhs == rhs );
	}

} // namespace augment

#endif // #ifndef AUGMENT_COM_PTR_HPP_INCLUDED
