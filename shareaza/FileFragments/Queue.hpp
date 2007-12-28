//
// Filefragments/Queue.hpp
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

#ifndef FILEFRAGMENTS_QUEUE_HPP_INCLUDED
#define FILEFRAGMENTS_QUEUE_HPP_INCLUDED

namespace Ranges
{

	template< class RangeT, class ContainerT = std::list< RangeT > >
class Queue
{
// Interface
public:
	// Typedefs
	typedef RangeT range_type;
	typedef ContainerT container_type;
	typedef typename range_type::size_type range_size_type;
	typedef typename range_type::payload_type payload_type;
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
	bool      empty() const { return m_set.empty(); }
	size_type size()  const { return m_set.size(); }

	void clear() { m_set.clear(); }
	void push_back(const range_type& value) { m_set.push_back( value ); }

	template< typename input_iterator >            
	void insert(input_iterator first, input_iterator last)
	{
		for ( ; first != last; ) insert( *first++ );
	}
	void erase(const range_type& value)
	{
		for ( iterator i = begin(); i != end(); )
		{
			if ( value.begin() < i->end() && value.end() > i->begin() )
			{
				if ( value.begin() <= i->begin() )
				{
					if ( value.end() >= i->end() )
					{
						i = m_set.erase( i );
					}
					else
					{
						*i = range_type( value.end(), i->end(), i->value() );
						++i;
					}
				}
				else
				{
					if ( value.end() >= i->end() )
					{
						*i = range_type( i->begin(), value.begin(), i->value() );
						++i;
					}
					else
					{
						m_set.push_back( range_type( value.end(), i->end(), i->value() ) );
						*i = range_type( i->begin(), value.begin(), i->value() );
						++i;
					}
				}
			}
			else
			{
				++i;
			}
		}
	}
	template< typename input_iterator >
	void erase(input_iterator first, input_iterator last)
	{
		for ( ; first != last; ) erase( *first++ );
	}
	iterator erase(iterator where) { return m_set.erase( where ); }

	void pop_front() { m_set.pop_front(); }
	// @swap    Swaps two lists.
	// @complexity   ~O( 1 )
	void swap(Queue& rhs)                    // throw ()
	{
		m_set.swap( rhs.m_set );
	}
// Implementation
private:
	container_type m_set;
};

template< class RangeT, class ContainerT  >
Queue< RangeT, ContainerT > extract_range(Queue< RangeT, ContainerT >& src, const RangeT& key)
{
	typedef Queue< RangeT, ContainerT > queue_type;
	typedef typename queue_type::iterator iterator;
	queue_type result;
	for ( iterator i = src.begin(); i != src.end(); )
	{
		if ( i->begin() < key.end() && i->end() > key.begin() )
		{
			result.push_back( *i );
			i = src.erase( i );
		}
		else
		{
			++i;
		}
	}
	return result;
}

} // namespace types

#endif // #ifndef FILEFRAGMENTS_QUEUE_HPP_INCLUDED
