//
// FileFragments/Fragment.hpp
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

#ifndef FILEFRAGMENTS_FRAGMENT_HPP_INCLUDED
#define FILEFRAGMENTS_FRAGMENT_HPP_INCLUDED

namespace detail
{

template< class Payload, typename OffsetType >
class Fragment : private Payload
{
public:
    typedef Payload PayloadType;
    typedef PayloadTraits< PayloadType > Traits;
    typedef OffsetType SizeType;
    typedef BadFragment< Fragment > BadFragmentException;
    Fragment(SizeType begin, SizeType end,
        const PayloadType& payload = PayloadType() )
    : Payload( payload ), range_( begin, end )
    {
        if( Traits::allowEmptyFragments
            ? end < begin
            : end <= begin )
        {
//            throw BadFragmentException( begin, end, payload );
            range_.first = ~0ULL - 1000;
            range_.second = ~0ULL;
            CString errorMsg;
            errorMsg.Format( _T( "FF::SimpleFragment - invalid args in c'tor - begin: %u64 - end: %u64" ),
                begin, end );
            theApp.Message( MSG_ERROR, errorMsg );
        }
    }   
    Fragment(::std::pair< SizeType, SizeType > range,
        const PayloadType& payload = PayloadType() )
    : Payload( payload ), range_( range )
    {
        if( Traits::allowEmptyFragments
            ? range.second < range.first
            : range.second <= range.first )
        {
//            throw BadFragmentException( range.first, range.second, payload );
            range_.first = ~0ULL - 1000;
            range_.second = ~0ULL;
            CString errorMsg;
            errorMsg.Format( _T( "FF::SimpleFragment - invalid args in c'tor (pair) - begin: %u64 - end: %u64" ),
                begin, end );
            theApp.Message( MSG_ERROR, errorMsg );
        }
    }
    
    // A resizing copy-c'tor 
    Fragment(const Fragment& other, SizeType begin, SizeType end)
    : Payload( other ), range_( begin, end )
    {
        if( Traits::allowEmptyFragments
            ? end < begin
            : end <= begin )
        {
//            throw BadFragmentException( begin, end, other );
            range_.first = ~0ULL - 1000;
            range_.second = ~0ULL;
            CString errorMsg;
            errorMsg.Format( _T( "FF::SimpleFragment - invalid args in c'tor (conversion) - begin: %u64 - end: %u64" ),
                begin, end );
            theApp.Message( MSG_ERROR, errorMsg );
        }
    }

    // Fragment conversion
    template< class OtherPayload, typename OtherOffsetType >
    Fragment(const Fragment< OtherPayload, OtherOffsetType >& other,
        const PayloadType& payload = PayloadType() )
    : Payload( payload ), range_( other.range_ )
    { }

    SizeType begin() const { return range_.first; }
    SizeType end() const { return range_.second; }
    SizeType size() const { return end() - begin(); }
    SizeType length() const { return size(); }
    
    const PayloadType& value() const { return *this; }
//Implementation
private:
    ::std::pair< SizeType, SizeType > range_;
};

template
<
    class Payload,
    typename OffsetType
>
bool operator==(
    const Fragment< Payload, OffsetType >& lhs,
    const Fragment< Payload, OffsetType >& rhs )
{
    return lhs.begin() == rhs.begin() && lhs.end() == rhs.end()
        && Fragment< Payload, OffsetType >::Traits::equals( lhs, rhs );
}

template< class FragmentT >
struct CompareFragments
    : public ::std::binary_function< FragmentT, FragmentT, bool >
{
    typedef FragmentT FragmentType;
    typedef typename FragmentType::Traits Traits;
    typedef ::std::binary_function< FragmentType, FragmentType, bool >
                                                                TemplateType;
    typedef typename TemplateType::result_type ResultType;
    typedef typename TemplateType::first_argument_type FirstArgumentType;
    typedef typename TemplateType::second_argument_type SecondArgumentType;
    ResultType operator()(FirstArgumentType lhs, SecondArgumentType rhs) const
    {
        return lhs.end() < rhs.begin()
            || lhs.end() == rhs.begin() && !Traits::equals( lhs, rhs );
    }
};

} // namespace detail

#endif // #ifndef FILEFRAGMENTS_FRAGMENT_HPP_INCLUDED
