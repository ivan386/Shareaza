//
// FileFragments/List.hpp
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

#ifndef FILEFRAGMENTS_LIST_HPP_INCLUDED
#define FILEFRAGMENTS_LIST_HPP_INCLUDED

namespace detail
{

template< class FragmentT, class ContainerT >
class List
{
// Interface
public:
    // Typedefs
    typedef FragmentT FragmentType;
    typedef ContainerT ContainerType;
    typedef typename FragmentType::SizeType FSizeType;
    typedef typename FragmentType::PayloadType PayloadType;
    typedef typename FragmentType::Traits Traits;
    typedef BadRange< FragmentType > BadRangeException;
    typedef typename ContainerType::value_type ValueType;
    typedef typename ContainerType::pointer Pointer;
    typedef typename ContainerType::const_pointer ConstPointer;
    typedef typename ContainerType::reference Reference;
    typedef typename ContainerType::const_reference ConstReference;
    typedef typename ContainerType::iterator Iterator;
    typedef typename ContainerType::const_iterator ConstIterator;
    typedef typename ContainerType::reverse_iterator ReverseIterator;
    typedef typename ContainerType::const_reverse_iterator ConstReverseIterator;
    typedef typename ContainerType::size_type SizeType;
    typedef typename ContainerType::difference_type DifferenceType;
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
    typedef ::std::pair< Iterator, Iterator > IteratorPair;
    typedef ::std::pair< ConstIterator, ConstIterator > ConstIteratorPair;
    // Constructor
    // Creates new list, that accepts fragments in the range 0..limit
    explicit List(FSizeType limit)
    : s_(), sumLength_( 0 ), upperLimit_( limit )
    { }
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
    FSizeType limit() const { return upperLimit_; }
    FSizeType sumLength() const { return sumLength_; }
    FSizeType missing() const { return limit() - sumLength(); }
    bool empty() const { return s_.size() == 0; }
    // Operations
    void clear()
    {
        s_.clear();
        sumLength_ = 0;
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
    FSizeType insert(const FragmentType& insertFragment);
    // @insert  Inserts a sequence of fragments. An optimized version should be
    //          written if 2 large containers have to be merged often.
    //          It is an error to insert a sequence which is part of the
    //          list. Doing so results in undefined behaviour.
    // @complexity   ~O( n_insert * log( n ) )
	template< typename InputIterator >            
    FSizeType insert(InputIterator first, InputIterator last)
    {
        FSizeType oldSum = sumLength();
        for( ; first != last; ) insert( *first++ );
        return sumLength() - oldSum;
    }
    // @insert  Inserts a fragment using an iterator as hint. Insertion is done
    //          in constant time, if no merging occurs and the element can be
    //          inserted before the hint. Otherwise normal insertion occurs.
    // @complexity   ~O( 1 ) or ~O( log( n ) )
    FSizeType insert(Iterator where, const FragmentType& insertFragment);
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
    FSizeType erase(const FragmentType& eraseFragment);
    // @erase   Deletes a sequence of fragments from the container. That
    //          sequence need not be part of the list. If it is, use a loop
    //          over the next function instead, if speed is important.
    template< typename InputIterator >
    FSizeType erase(InputIterator first, InputIterator last)
    {
        FSizeType oldSum = sumLength();
        for( ; first != last; ) erase( *first++ );
        return oldSum - sumLength();
    }
    // @erase   This deletes the fragment the argument points to.
    //          Iterators that point to other fragments remain valid.
    // @return  Returns iterator that points to the next fragment after the one
    //          pointed to by the argument.
    // @complexity   ~O( log( n ) )
    void erase(Iterator where)
    {
        sumLength_ -= where->length();
        s_.erase( where );
    }
    // @swap    Swaps two lists.
    // @complexity   ~O( 1 )
    void swap(List& rhs)                    // throw ()
    {
        s_.swap( rhs.s_ );
        ::std::swap( sumLength_, rhs.sumLength_ );
        ::std::swap( upperLimit_, rhs.upperLimit_ );
    }

    Iterator lowerBound(const FragmentType& match)
    {
        return s_.lower_bound( match );
    }
    ConstIterator lowerBound(const FragmentType& match) const
    {
        return s_.lower_bound( match );
    }
    Iterator upperBound(const FragmentType& match)
    {
        return s_.upper_bound( match );
    }
    ConstIterator upperBound(const FragmentType& match) const
    {
        return s_.upper_bound( match );
    }
    IteratorPair equalRange(const FragmentType& match)
    {
        return s_.equal_range( match );
    }
    ConstIteratorPair equalRange(const FragmentType& match) const
    {
        return s_.equal_range( match );
    }

    Iterator lower_bound(const FragmentType& match)
    {
        return lowerBound( match );
    }
    ConstIterator lower_bound(const FragmentType& match) const
    {
        return lowerBound( match );
    }
    Iterator upper_bound(const FragmentType& match)
    {
        return upperBound( match );
    }
    ConstIterator upper_bound(const FragmentType& match) const
    {
        return upperBound( match );
    }
    IteratorPair equal_range(const FragmentType& match)
    {
        return equalRange( match );
    }
    ConstIteratorPair equal_range(const FragmentType& match) const
    {
        return equalRange( match );
    }

    // same as equalRange, except that fragments that are adjacent
    // to the argument are not part of the result
    IteratorPair overlappingRange(const FragmentType& match)
    {
        IteratorPair result( equalRange( match ) );
        if( result.first != result.second )
        {
            if( result.first->end() == match.begin() ) ++result.first;
            if( ( --result.second )->begin() != match.end() ) ++result.second;
        }
        return result;
    }
    ConstIteratorPair overlappingRange(const FragmentType& match) const
    {
        ConstIteratorPair result( equalRange( match ) );
        if( result.first != result.second )
        {
            if( result.first->end() == match.begin() ) ++result.first;
            if( ( --result.second )->begin() != match.end() ) ++result.second;
        }
        return result;
    }
// Implementation
private:
    ContainerType s_;
    FSizeType sumLength_;
    FSizeType upperLimit_;
};

// @inverse returns a list containing each range out of the base range 0..limit
//          that is not part of the sourcelist
// @complexity   ~O( n * log( n ) )
template< class ListType >
ListType inverse(const ListType& src);

// @largestFragment
//          Returns iterator to the first largest fragment in a list, or
//          end() iterator if list is empty.
// @complexity   ~O( n )
template< class ListType >
typename ListType::Iterator largestFragment(ListType& src);
template< class ListType >
typename ListType::ConstIterator largestFragment(const ListType& src);

// @randomFragment
//          Returns iterator to the random fragment in a list, or
//          end() iterator if list is empty.
// @complexity   ~O( n )
template< class ListType >
typename ListType::Iterator randomFragment(ListType& src);
template< class ListType >
typename ListType::ConstIterator randomFragment(const ListType& src);

template< class ListType >
bool hasPosition(const ListType& src, typename ListType::FSizeType pos);

// returns if argument overlaps with any fragment in the list
template< class ListType >
bool overlaps(const ListType& src,
    const typename ListType::FragmentType& match);

template< class ListType >
bool overlaps(const ListType& src, const ListType& match);

template< class ListType >
typename ListType::FSizeType
overlappingSum(const ListType& src,
    const typename ListType::FragmentType& match);

} // namespace detail

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

namespace detail
{

template< class FragmentT, class ContainerT >
inline typename List< FragmentT, ContainerT >::FSizeType
List< FragmentT, ContainerT >::insert(
    const typename List< FragmentT, ContainerT >::FragmentType& insertFragment)
{
    if( insertFragment.end() > limit() )
    {
//        throw BadRangeException( insertFragment, limit() );
        CString errorMsg;
        errorMsg.Format(
            _T( "FF::SimpleFragmentList - invalid arg for insert - " )
            _T( "List - size: %u - limit: %I64u - sum: %I64u - " )
            _T( "Fragment - begin: %I64u - end: %I64u" ), size(), limit(), sumLength(),
            insertFragment.begin(), insertFragment.end() );
        theApp.Message( MSG_ERROR, errorMsg );
        return 0;
    }
    if( insertFragment.length() == 0 ) return 0;
    ::std::pair< Iterator, Iterator > insertRange =
        equalRange( insertFragment );
    if( insertRange.first != insertRange.second )
    {
        FSizeType chgSum =
            Traits::mergeAndReplace( s_, insertRange, insertFragment );
        sumLength_ += chgSum;
        return chgSum;
    }
    else
    {
        s_.insert( insertRange.second, insertFragment );
        sumLength_ += insertFragment.length();
        return insertFragment.length();
    }
}

template< class FragmentT, class ContainerT >
inline typename List< FragmentT, ContainerT >::FSizeType
List< FragmentT, ContainerT >::insert(
    typename List< FragmentT, ContainerT >::Iterator where,
    const typename List< FragmentT, ContainerT >::FragmentType& insertFragment)
{
    Iterator tmp = where;
    CompareFragments< FragmentType > cmp;
    if( ( where == begin() || cmp( *--tmp, insertFragment ) )
        && ( where == end() || cmp( insertFragment, *where ) ) )
    {
        s_.insert( where, insertFragment );
        sumLength_ += insertFragment.length();
        return insertFragment.length();
    }
    else
    {
        return insert( insertFragment );
    }
}

template< class FragmentT, class ContainerT >
inline typename List< FragmentT, ContainerT >::FSizeType
List< FragmentT, ContainerT >::erase(
    const typename List< FragmentT, ContainerT >::FragmentType& eraseFragment)
{
    if( eraseFragment.end() > limit() )
    {
//        throw BadRangeException( eraseFragment, limit() );
        CString errorMsg;
        errorMsg.Format(
            _T( "FF::SimpleFragmentList - invalid arg for erase - " )
            _T( "List - size: %u - limit: %I64u - sum: %I64u - " )
            _T( "Fragment - begin: %I64i - end: %I64u" ), size(), limit(), sumLength(),
            eraseFragment.begin(), eraseFragment.end() );
        theApp.Message( MSG_ERROR, errorMsg );
        return 0;
    }
    if( eraseFragment.length() == 0 ) return 0;
    ::std::pair< Iterator, Iterator > eraseRange =
        overlappingRange( eraseFragment );
    if( eraseRange.first == eraseRange.second ) return 0;
    FragmentType frontFragment
        = eraseRange.first->begin() < eraseFragment.begin()
            ? FragmentType( *eraseRange.first,
                 eraseRange.first->begin(), eraseFragment.begin() )
            : FragmentType( *eraseRange.first, 0,
                ::std::numeric_limits< FSizeType >::max() );
    --eraseRange.second;
    FragmentType backFragment
        = eraseRange.second->end() > eraseFragment.end()
        ? FragmentType( *eraseRange.second,
                eraseFragment.end(), eraseRange.second->end() )
        : FragmentType( *eraseRange.second, 0,
                ::std::numeric_limits< FSizeType >::max() );
    const FSizeType oldSum = sumLength();
    ++eraseRange.second;
    for( ; eraseRange.first != eraseRange.second; )
    {
        sumLength_ -= eraseRange.first->length();
        s_.erase( eraseRange.first++ );
    }
    if( frontFragment.end() < ::std::numeric_limits< FSizeType >::max() )
    {
        s_.insert( eraseRange.second, frontFragment );
        sumLength_ += frontFragment.length();
    }
    if( backFragment.end() < ::std::numeric_limits< FSizeType >::max() )
    {
        s_.insert( eraseRange.second, backFragment );
        sumLength_ += backFragment.length();
    }
    return oldSum - sumLength();
}

template< class ListType >
inline ListType inverse(const ListType& src)
{
    typedef typename ListType::FragmentType FragmentType;
    typedef typename ListType::PayloadType PayloadType;
    ListType result( src.limit() );
    typename ListType::FSizeType last = 0;
    for( typename ListType::ConstIterator i = src.begin(); i != src.end();
        ++i )
    {
        if( last < i->begin() )
        {
            result.insert( FragmentType( last, i->begin(),
                PayloadType() ) );
        }
        last = i->end();
    }
    if( last < src.limit() )
    {
        result.insert( FragmentType( last, src.limit(), PayloadType() ) );
    }
    return result;
}

template< class ListType >
inline typename ListType::Iterator largestFragment(ListType& src)
{
    typedef typename ListType::Iterator Iterator;
    Iterator result = src.begin();
    for( Iterator i = result; i != src.end(); ++i )
        if( result->length() < i->length() ) result = i;
    return result;
}
template< class ListType >
inline typename ListType::ConstIterator largestFragment(const ListType& src)
{
    typedef typename ListType::ConstIterator ConstIterator;
    ConstIterator result = src.begin();
    for( ConstIterator i = result; i != src.end(); ++i )
        if( result->length() < i->length() ) result = i;
    return result;
}

template< class ListType >
inline typename ListType::Iterator randomFragment(ListType& src)
{
    if( src.empty() ) return src.end();
    typename ListType::Iterator result = src.begin();
    ::std::advance( result, ::std::rand() % src.size() );
    return result;
}
template< class ListType >
inline typename ListType::Iterator randomFragment(const ListType& src)
{
    if( src.empty() ) return src.end();
    typename ListType::ConstIterator result = src.begin();
    ::std::advance( result, ::std::rand() % src.size() );
    return result;
}

template< class ListType >
bool hasPosition(const ListType& src, typename ListType::FSizeType pos)
{
    typename ListType::ConstIteratorPair match = src.overlappingRange(
        typename ListType::FragmentType( pos, pos + 1,
        typename ListType::PayloadType() ) );
    return match.first != match.second;
}

template< class ListType >
bool overlaps(const ListType& src,
    const typename ListType::FragmentType& match)
{
    typename ListType::ConstIteratorPair matchPair
        = src.overlappingRange( match );
    return matchPair.first != matchPair.second;
}    

// ToDo: tweak for src.size() < match.size() and src.size ~== match.size()
template< class ListType >
bool overlaps(const ListType& src, const ListType& match)
{
    for( typename ListType::ConstIterator matchIterator = match.begin();
        matchIterator != match.end(); ++matchIterator )
    {
        typename ListType::ConstIteratorPair matchPair
            = src.overlappingRange( *matchIterator );
        if( matchPair.first != matchPair.second ) return true;
    }
    return false;
}

template< class ListType >
typename ListType::FSizeType
overlappingSum(const ListType& src,
    const typename ListType::FragmentType& match)
{
    typename ListType::ConstIteratorPair matchIterators
        = src.overlappingRange( match );
    typename ListType::FSizeType result = 0;
    for( ; matchIterators.first != matchIterators.second;
        ++matchIterators.first )
    {
        result += ::std::min( matchIterators.first->end(), match.end() )
            -  ::std::max( matchIterators.first->begin(), match.begin() );
    }
    return result;
}    

} // namespace detail

#endif // #ifndef FILEFRAGMENTS_LIST_HPP_INCLUDED
