//
// Filefragments/List.hpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#ifndef FILEFRAGMENTS_LIST_HPP_INCLUDED
#define FILEFRAGMENTS_LIST_HPP_INCLUDED

namespace Ranges
{
template< class RangeT, template< class, class > class TraitsT, class ContainerT = std::set
	<
		RangeT,
		RangeCompare< RangeT::size_type, RangeT::payload_type >
	> >
class List : public TraitsT< RangeT, ContainerT >
{
// Interface
public:
	// Typedefs
	typedef RangeT range_type;
	typedef ContainerT container_type;
	typedef TraitsT< RangeT, ContainerT > Traits;
	typedef typename range_type::size_type range_size_type;
	typedef typename range_type::payload_type payload_type;
	typedef RangeCompare< payload_type, range_size_type > compare_type;
	typedef ListError< range_type > ListException;
	typedef typename container_type::value_type value_Type;
	typedef typename container_type::pointer pointer;
	typedef typename container_type::const_pointer const_pointer;
	typedef typename container_type::reference reference;
	typedef typename container_type::const_reference const_reference;
	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;
	typedef typename container_type::reverse_iterator reverse_iterator;
	typedef typename container_type::const_reverse_iterator const_reverse_iterator;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::difference_type difference_type;
	typedef std::pair< iterator, iterator > iterator_pair;
	typedef std::pair< const_iterator, const_iterator > const_iterator_pair;
	// Constructor
	explicit List() : Traits(), m_set() { }
	explicit List(typename Traits::ctor_arg_type arg) : Traits( arg ), m_set() { }

	// Iterators
	iterator               begin()        { return m_set.begin(); }
	const_iterator         begin()  const { return m_set.begin(); }
	iterator               end()          { return m_set.end(); }
	const_iterator         end()    const { return m_set.end(); }
	reverse_iterator       rbegin()       { return m_set.rbegin(); }
	const_reverse_iterator rbegin() const { return m_set.rbegin(); }
	reverse_iterator       rend()         { return m_set.rend(); }
	const_reverse_iterator rend()   const { return m_set.rend(); }

	// Accessors
	bool empty() const { return m_set.empty(); }
	size_type size()  const { return m_set.size(); }
	// Operations
	void clear()
	{
		m_set.clear();
		Traits::clear();
	}
	// @insert  Inserts a fragment into the container. Because of the automatic
	//          sorting and merging guarantied by the container, this might
	//          not insert the full range indicated by the fragment in cases
	//          when parts of the range of the fragment are already present.
	//          Attempting to insert an empty fragment ( Length() == 0 ) is
	//          possible ( if the fragment is constructible in the first place )
	//          but does nothing.
	// @return  Returns the length of the range that has been inserted.
	//          Effectively it reflects the change of sumLength().
	// @complexity   ~O( log( n ) )
	range_size_type insert(const range_type& value);
	// @insert  Inserts a sequence of fragments. An optimized version should be
	//          written if 2 large containers have to be merged often.
	// @complexity   ~O( n_insert * log( n ) )
	template< typename input_iterator >            
	range_size_type insert(input_iterator first, input_iterator last)
	{
		range_size_type sum = 0;
		for ( ; first != last; ) sum += insert( *first++ );
		return sum;
	}
	// @insert  Inserts a fragment using an iterator as hint. Insertion is done
	//          in constant time, if no merging occurs and the element can be
	//          inserted before the hint. Otherwise normal insertion occurs.
	// @complexity   ~O( 1 ) or ~O( log( n ) )
	range_size_type insert(const iterator where, const range_type& value);
	// @erase   Deletes a fragment from the container. Because of the automatic
	//          sorting and merging guarantied by the container, this might
	//          not delete the full range indicated by the fragment, in cases
	//          when parts of the range of the fragment are not present.
	//          Attempting to delete an empty fragment ( Length() == 0 ) is
	//          possible but does nothing.
	// @return  Returns the length of the range that has been deleted.
	//          Effectively it reflects the change of sumLength().
	//          Note that this differs from the standard containers interface
	//          which usually returns a size_type for this kind of funtion,
	//          which indictaes the number of elements being erased.
	range_size_type erase(const range_type& value);
	// @erase   Deletes a sequence of fragments from the container. That
	//          sequence need not be part of the list. If it is, use a loop
	//          over the next function instead, if speed is important.
	template< typename input_iterator >
	range_size_type erase(input_iterator first, input_iterator last)
	{
		range_size_type sum = 0;
		for ( ; first != last; ) sum += erase( *first++ );
		return sum;
	}
	// @erase   This deletes the fragment the argument points to.
	//          Iterators that point to other fragments remain valid.
	// @return  Returns iterator that points to the next fragment after the one
	//          pointed to by the argument.
	// @complexity   ~O( log( n ) )
	range_size_type erase(const iterator where)
	{
		range_size_type result = Traits::erase( where );
		m_set.erase( where );
		return result;
	}
	// @swap    Swaps two lists.
	// @complexity   ~O( 1 )
	void swap(List& rhs)                    // throw ()
	{
		Traits::swap( rhs );
		m_set.swap( rhs.m_set );
	}

	iterator            lower_bound(const range_type& key)       { return m_set.lower_bound( key ); }
	const_iterator      lower_bound(const range_type& key) const { return m_set.lower_bound( key ); }
	iterator            upper_bound(const range_type& key)       { return m_set.upper_bound( key ); }
	const_iterator      upper_bound(const range_type& key) const { return m_set.upper_bound( key ); }
	iterator_pair       equal_range(const range_type& key)       { return m_set.equal_range( key ); }
	const_iterator_pair equal_range(const range_type& key) const { return m_set.equal_range( key ); }
	iterator_pair       merge_range(const range_type& key)
	{
		iterator_pair sequence( equal_range( key ) );
		if ( sequence.first != m_set.begin() && ( --sequence.first )->end() < key.begin() )
			++sequence.first;
		if ( sequence.second != m_set.end() && sequence.second->begin() == key.end() )
			++sequence.second;
		return sequence;
	}
	const_iterator_pair merge_range(const range_type& key) const
	{
		const_iterator_pair sequence( equal_range( key ) );
		if ( sequence.first != m_set.begin() && ( --sequence.first )->end() < key.begin() )
			++sequence.first;
		if ( sequence.second != m_set.end() && sequence.second->begin() == key.end() )
			++sequence.second;
		return sequence;
	}

	iterator largest_range() { return std::max_element( begin(), end(), cmp_size() ); }
	const_iterator largest_range() const { return std::max_element( begin(), end(), cmp_size() ); }

	iterator random_range()
	{
		iterator result = begin();
		if ( !empty() ) std::advance( result, GetRandomNum( 0ui64, (uint64)size() - 1 ) );
		return result;
	}
	const_iterator random_range() const
	{
		const_iterator result = begin();
		if ( !empty() ) std::advance( result, GetRandomNum( 0ui64, (uint64)size() - 1 ) );
		return result;
	}

	bool overlaps(const range_type& key) const { return m_set.find( key ) != end(); }
	bool overlaps(const List& rhs) const
	{
		return size() < rhs.size()
			? std::find_if( begin(), end(), overlaps_helper( rhs ) ) != end()
			: std::find_if( rhs.begin(), rhs.end(), overlaps_helper( *this ) ) != rhs.end();
	}
	range_size_type overlapping_sum(const range_type& key) const
	{
		const_iterator_pair sequence( equal_range( key ) );
		range_size_type sum = 0;
		for ( ; sequence.first != sequence.second; ++sequence.first )
		{
			sum += min( sequence.first->end(), key.end() )
				- max( sequence.first->begin(), key.begin() );
		}
		return sum;
	}
	bool has_position(range_size_type where) const { return overlaps( range_type( where, where + 1 ) ); }

// Implementation
private:
	container_type m_set;
	struct cmp_size : public std::binary_function< RangeT, RangeT, bool >
	{
		result_type operator()(first_argument_type lhs, second_argument_type rhs) const
		{
			return lhs.size() < rhs.size();
		}
	};
	struct overlaps_helper : public std::unary_function< RangeT, bool >
	{
		overlaps_helper(const List& list) : m_list( list ) { }
		result_type operator()(argument_type arg) const
		{
			return m_list.overlaps( arg );
		}
		const List& m_list;
	};
};

// @inverse returns a list containing each range out of the base range 0..limit
//          that is not part of the sourcelist
// @complexity   ~O( n * log( n ) )
template< class list_type >
list_type inverse(const list_type& src);

} // namespace Ranges

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

namespace Ranges
{

template< class RangeT, template< class, class > class TraitsT, class ContainerT >
typename RangeT::size_type List< RangeT, TraitsT, ContainerT >::insert(const RangeT& value)
{
	if ( value.end() > limit() )
	{
//		throw ListException( value, limit() );
		CString msg;
		msg.Format( _T( "ListError - insert - size: %I64u - limit: %I64u - sum: %I64u - " )
			_T( "Range - begin: %I64u - end: %I64u" ),
			size(), limit(), Traits::length_sum(), value.begin(), value.end() );
		theApp.Message( MSG_ERROR, msg );
		return 0;
	}
	if ( value.size() == 0 ) return 0;
	iterator_pair sequence( merge_range( value ) );
	return sequence.first != sequence.second
		? Traits::merge_and_replace( m_set, sequence, value )
		: Traits::simple_merge( m_set, sequence.first, value );
}

template< class RangeT, template< class, class > class TraitsT, class ContainerT >
typename RangeT::size_type List< RangeT, TraitsT, ContainerT >::insert(
	typename List< RangeT, TraitsT, ContainerT >::iterator where, const RangeT& value)
{
	if ( value.end() > limit() )
	{
//		throw ListException( value, limit() );
		CString msg;
		msg.Format( _T( "ListError - insert(h) - size: %I64u - limit: %I64u - sum: %I64u - " )
			_T( "Range - begin: %I64u - end: %I64u" ),
			size(), limit(), Traits::length_sum(), value.begin(), value.end() );
		theApp.Message( MSG_ERROR, msg );
		return 0;
	}
	if ( value.size() == 0 ) return 0;
	iterator tmp( where );
	return ( where == begin() || ( --tmp )->end() < value.begin() )
			&& ( where == end() || value.end() < where->begin() )
		? Traits::simple_merge( m_set, where, value )
		: insert( value );
}

template< class RangeT, template< class, class > class TraitsT, class ContainerT >
typename RangeT::size_type List< RangeT, TraitsT, ContainerT >::erase(const RangeT& value)
{
	if ( value.end() > limit() )
	{
//		throw ListException( value, limit() );
		CString msg;
		msg.Format( _T( "ListError - erase - size: %I64u - limit: %I64u - sum: %I64u - " )
			_T( "Range - begin: %I64u - end: %I64u" ),
			size(), limit(), Traits::length_sum(), value.begin(), value.end() );
		theApp.Message( MSG_ERROR, msg );
		return 0;
	}
	if ( value.size() == 0 ) return 0;
	iterator_pair sequence( equal_range( value ) );
	if ( sequence.first == sequence.second ) return 0;
	const range_type front( min( sequence.first->begin(), value.begin() ),
		value.begin(), value.value() );
	const range_type back( value.end(),
		max( ( --sequence.second )->end(), value.end() ), value.value() );
	range_size_type sum = 0;
	for ( ++sequence.second; sequence.first != sequence.second; ) sum += erase( sequence.first++ );
	sum -= insert( sequence.second, front );
	sum -= insert( sequence.second, back );
	return sum;
}

template< class list_type >
list_type inverse(const list_type& src)
{
	typedef typename list_type::range_type range_type;
	typedef typename list_type::payload_type payload_type;
	typedef typename list_type::range_size_type range_size_type;
	typedef typename list_type::const_iterator const_iterator;
	list_type result( src.limit() );
	range_size_type last = 0;
	for ( const_iterator i = src.begin(); i != src.end(); ++i )
	{
		result.insert( range_type( last, i->begin() ) );
		last = i->end();
	}
	result.insert( range_type( last, src.limit() ) );
	return result;
}

} // namespace Ranges

#endif // #ifndef FILEFRAGMENTS_LIST_HPP_INCLUDED
