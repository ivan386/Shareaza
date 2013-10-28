////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// augment/auto_array.hpp                                                     //
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

#ifndef AUGMENT_AUTO_ARRAY_HPP_INCLUDED
#define AUGMENT_AUTO_ARRAY_HPP_INCLUDED

namespace augment
{

template<typename T>
class auto_array
{
public:
	typedef T element_type;
private:
	struct auto_array_ref
	{
		explicit auto_array_ref(element_type** ref) throw()
			: ref_( ref )
		{}
		element_type* release() throw()
		{
			element_type* ptr = *ref_;
			*ref_ = NULL;
			return ptr;
		}
		element_type** ref_;
	};
public:
	explicit auto_array(element_type* ptr = NULL) throw()
		: ptr_( ptr )
	{}
	auto_array(auto_array& other) throw()
		: ptr_( other.release() )
	{}
	auto_array(auto_array_ref other) throw()
		: ptr_( other.release() )
	{}
	~auto_array() throw()
	{
		if ( get() != NULL )
			boost::checked_array_delete( get() );
	};

	auto_array& operator=(auto_array& other) throw()
	{
		reset( other.release() );
		return *this;
	}
	auto_array& operator=(auto_array_ref other) throw()
	{
		reset( other.release() );
		return *this;
	}

	element_type* get() const  throw()
	{
		return ptr_;
	}
	element_type& operator[](std::size_t index) const  throw()
	{
		return get()[ index ];
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
			boost::checked_array_delete( get() );
		ptr_ = ptr;
	}

	operator auto_array_ref() throw()
	{
		return auto_array_ref( &ptr_ );
	}
private:
	element_type* ptr_;
};

} // namespace augment

#endif // #ifndef AUGMENT_AUTO_ARRAY_HPP_INCLUDED
