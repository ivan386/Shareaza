//
// FileFragments/PayloadTraits.hpp
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

#ifndef FILEFRAGMENTS_PAYLOADTRAITS_HPP_INCLUDED
#define FILEFRAGMENTS_PAYLOADTRAITS_HPP_INCLUDED

namespace detail
{
    
template<>
struct PayloadTraits< EmptyType >
{
    typedef EmptyType PayloadType;
    static const bool allowEmptyFragments = true;
    template< typename OffsetType >
    static bool equals(const Fragment< EmptyType, OffsetType >& lhs,
                       const Fragment< EmptyType, OffsetType >& rhs)
    {
        return true;
    }
    template< class ContainerType, typename OffsetType >
    static typename Fragment< EmptyType, OffsetType >::SizeType
    mergeAndReplace(ContainerType& s, ::std::pair
    <
        typename List
        <
            Fragment< EmptyType, OffsetType >,
            ContainerType
        >::Iterator,
        typename List
        <
            Fragment< EmptyType, OffsetType >,
            ContainerType
        >::Iterator
    >& range, const Fragment< EmptyType, OffsetType >& toMerge)
    {
        typedef Fragment< EmptyType, OffsetType > FragmentType;
        typedef typename FragmentType::SizeType FSizeType;
        if( range.first->begin() <= toMerge.begin()
            && range.first->end() >= toMerge.end() ) return 0;
        FSizeType chgSum = 0;
        FSizeType low = ::std::min( range.first->begin(), toMerge.begin() );
        FSizeType high;
        for( ; range.first != range.second; )
        {
            high = range.first->end();
            chgSum -= range.first->length();
            s.erase( range.first++ );
        }
        high = ::std::max( high, toMerge.end() );
        s.insert( range.second, FragmentType( low, high ) );
        chgSum += high - low;
        return chgSum;
    }
};

} // namespace detail

#endif // #ifndef FILEFRAGMENTS_PAYLOADTRAITS_HPP_INCLUDED
