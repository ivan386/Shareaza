//
// FileFragments/Queue.hpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
// This file is part of SHAREAZA (www.shareaza.com)
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

namespace detail
{

class Queue
{
// Interface
public:
    // Typedefs
    typedef Fragment< EmptyType, u64 > FragmentType;
    typedef std::list< FragmentType > ContainerType;
    typedef FragmentType::SizeType FSizeType;
    typedef FragmentType::PayloadType PayloadType;
    typedef FragmentType::Traits Traits;
    typedef ContainerType::value_type ValueType;
    typedef ContainerType::pointer Pointer;
    typedef ContainerType::const_pointer ConstPointer;
    typedef ContainerType::reference Reference;
    typedef ContainerType::const_reference ConstReference;
    typedef ContainerType::iterator Iterator;
    typedef ContainerType::const_iterator ConstIterator;
    typedef ContainerType::reverse_iterator ReverseIterator;
    typedef ContainerType::const_reverse_iterator ConstReverseIterator;
    typedef ContainerType::size_type SizeType;
    typedef ContainerType::difference_type DifferenceType;
    typedef ValueType value_type;
    typedef Pointer pointer;
    typedef ConstPointer const_pointer;
    typedef Reference reference;
    typedef ConstReference const_reference;
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;
    typedef ReverseIterator reverse_iterator;
    typedef ConstReverseIterator const_reverse_iterator;
    typedef SizeType size_type;
    typedef DifferenceType difference_type;
    // Constructor
        // Use implicit version
    // Copy constructor
        // Use implicit version
    // Destructor
        // Use implicit version        // We don't inherit from this class
    // Assignment
        // Use implicit version

    // Iterators
    Iterator begin() { return s_.begin(); }
    ConstIterator begin() const { return s_.begin(); }
    Iterator end() { return s_.end(); }
    ConstIterator end() const { return s_.end(); }
    ReverseIterator rbegin() { return s_.rbegin(); }
    ConstReverseIterator rbegin() const { return s_.rbegin(); }
    ReverseIterator rend() { return s_.rend(); }
    ConstReverseIterator rend() const { return s_.rend(); }

    // Accessors
    SizeType size() const { return s_.size(); }
    // @empty   Returns true, if list does not contain any fragments.
    bool empty() const { return s_.size() == 0; }
    // Operations
    void clear()
    {
        s_.clear();
    }
    void pushBack(const FragmentType& insertFragment)
    {
        s_.push_back( insertFragment );
    }
    void push_back(const FragmentType& insertFragment)
    {
        pushBack( insertFragment );
    }
	template< typename InputIterator >            
    void insert(InputIterator first, InputIterator last)
    {
        for( ; first != last; ) s_.insert( *first++ );
    }
    void erase(const FragmentType& eraseFragment)
    {
        for( Iterator it = begin(); it != end(); )
        {
            if( eraseFragment.begin() < it->end()
                && eraseFragment.end() > it->begin() )
            {
                if( eraseFragment.begin() <= it->begin() )
                {
                    if( eraseFragment.end() >= it->end() )
                    {
                        it = s_.erase( it );
                    }
                    else
                    {
                        *it = FragmentType( eraseFragment.end(), it->end() );
                        ++it;
                    }
                }
                else
                {
                    if( eraseFragment.end() >= it->end() )
                    {
                        *it = FragmentType( it->begin(),
                            eraseFragment.begin() );
                        ++it;
                    }
                    else
                    {
                        s_.push_back(
                            FragmentType( eraseFragment.end(), it->end() ) );
                        *it = FragmentType(
                            it->begin(), eraseFragment.begin() );
                        ++it;
                    }
                }
            }
            else
            {
                ++it;
            }
        }
    }
    template< typename InputIterator >
    void erase(InputIterator first, InputIterator last)
    {
        for( ; first != last; ) erase( *first++ );
    }
    Iterator erase(Iterator where)
    {
        return s_.erase( where );
    }
    void popFront() { s_.pop_front(); }
    void pop_front() { popFront(); }
    // @swap    Swaps two lists.
    // @complexity   ~O( 1 )
    void swap(Queue& rhs)                    // throw ()
    {
        s_.swap( rhs.s_ );
    }
// Implementation
private:
    ContainerType s_;
};

inline Queue extractRange(Queue& src, const Queue::FragmentType& match)
{
    Queue result;
    for( Queue::Iterator it = src.begin(); it != src.end(); )
    {
        if( it->begin() < match.end() && it->end() > match.begin() )
        {
            result.pushBack( *it );
            it = src.erase( it );
        }
        else
        {
            ++it;
        }
    }
    return result;
}

} // namespace types

#endif // #ifndef FILEFRAGMENTS_QUEUE_HPP_INCLUDED
