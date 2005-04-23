//
// FileFragments/Compatibility.hpp
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

#ifndef FILEFRAGMENTS_COMPATIBILITY_HPP_INCLUDED
#define FILEFRAGMENTS_COMPATIBILITY_HPP_INCLUDED

namespace detail
{

// move to a common (private) base/mixin for block transfer (BT/ED2K)
// selects an available block, either unaligned blocks or if none is available
// a random aligned block
template< class ListType, class AvailableType >
typename ListType::FragmentType
selectBlock(const ListType& src, typename ListType::FSizeType blockSize,
    const AvailableType* available )
{
    typedef typename ListType::FragmentType FragmentType;
    typedef typename ListType::FSizeType FSizeType;
    typedef typename ListType::ConstIterator ConstIterator;
    
    if( src.empty() ) return FragmentType( 0,
        ::std::numeric_limits< FSizeType >::max() );

    ::std::deque< FSizeType > blocks;

    for( ConstIterator selectIterator = src.begin();
        selectIterator != src.end(); ++selectIterator )
    {
        FSizeType blockBegin = selectIterator->begin() / blockSize;
        FSizeType blockEnd = ( selectIterator->end() - 1 ) / blockSize;
		if ( selectIterator->begin() % blockSize )
		{
			// the start of a block is complete, but part is missing
			
			if ( !available || available[ blockBegin ] )
			{
                return FragmentType( selectIterator->begin(),
                    ::std::min( selectIterator->end(),
                    blockSize * ( blockBegin + 1 ) ) );
			}
            ++blockBegin;
		}
		if ( blockBegin <= blockEnd && selectIterator->end() % blockSize
            && selectIterator->end() < src.limit() )
		{
			// the end of a block is complete, but part is missing
			
			if ( !available || available[ blockEnd ] )
			{
                return FragmentType( blockEnd * blockSize,
                    selectIterator->end() );
			}
            --blockEnd;
		}
		// this fragment contains one or more aligned empty blocks
        if( blockEnd != ~0ULL ) for( ; blockBegin <= blockEnd; ++blockBegin )
        {
            if( !available || available[ blockBegin ] )
            {
                blocks.push_back( blockBegin );
            }
        }
	}
	
    if( blocks.empty() )  return FragmentType( 0,
        ::std::numeric_limits< FSizeType >::max() );

    FSizeType blockBegin = blocks[ ::std::rand() % blocks.size() ] * blockSize;

    return FragmentType( blockBegin, ::std::min( blockBegin + blockSize, src.limit() ) );
}

inline void SerializeOut(CArchive& ar, const SimpleFragment& out)
{
    ar << out.begin();
    ar << out.length();
}

inline SimpleFragment SerializeIn(CArchive& ar, int version)
{
    try
    {
        if ( version >= 29 )
        {
            u64 begin, length;
            ar >> begin >> length;
            return SimpleFragment( begin, begin + length );
        }
        else
        {
            u32 begin, length;
            ar >> begin >> length;
            return SimpleFragment( begin, begin + length );
        }
    }
    catch( Exception& )
    {
         AfxThrowArchiveException( CArchiveException::generic );
    }
}

// used in FragmentedFile.cpp
inline void SerializeOut1(CArchive& ar, const SimpleFragmentList& out)
{
    QWORD nTotal = out.limit();
    QWORD nRemaining = out.sumLength();
    DWORD nFragments = out.size();
    ar << nTotal << nRemaining << nFragments;

    for ( SimpleFragmentList::ConstIterator it = out.begin(); it != out.end();
        ++it )
    {
        SerializeOut( ar, *it );
    }
}

inline void SerializeIn1(CArchive& ar, SimpleFragmentList& in, int version)
{
    if( version >= 29 )
    {
        try
        {
            QWORD nTotal, nRemaining;
            DWORD nFragments;
            ar >> nTotal >> nRemaining >> nFragments;
            in.swap( SimpleFragmentList( nTotal ) );

            for ( ; nFragments--; ) in.insert( in.end(),
                SerializeIn( ar, version ) );

            // Sanity check
            if( in.sumLength() != nRemaining ) AfxThrowArchiveException( CArchiveException::generic );
        }
        catch( Exception& )
        {
            AfxThrowArchiveException( CArchiveException::generic );
        }

    }
    else
    {
        try
        {
            DWORD nTotal, nRemaining;
            DWORD nFragments;
            ar >> nTotal >> nRemaining >> nFragments;
            in.swap( SimpleFragmentList( nTotal ) );

            for ( ; nFragments--; ) in.insert( in.end(),
                SerializeIn( ar, version ) );

            // Sanity check
            if( in.sumLength() != nRemaining ) AfxThrowArchiveException( CArchiveException::generic );
        }
        catch( Exception& ) // translate exception because Shareaza
        {                   // doesn't know about this one yet
            AfxThrowArchiveException( CArchiveException::generic );
        }
    }

}

// used in DownloadSource.cpp
inline void SerializeOut2(CArchive& ar, const SimpleFragmentList& out)
{
    ar.WriteCount( out.size() );

    for ( SimpleFragmentList::ConstIterator it = out.begin(); it != out.end();
        ++it )
    {
        SerializeOut( ar, *it );
    }
}

inline void SerializeIn2(CArchive& ar, SimpleFragmentList& in, int version)
{
    try
    {
        if( version >= 20 )
        {
            for( int count = ar.ReadCount(); count--; )
            {
                if ( in.insert( SerializeIn( ar, version ) ) == 0 )
					AfxThrowArchiveException( CArchiveException::generic );
            }
        }
        else if( version >= 5 )
        {
            while( ar.ReadCount() )
            {
                in.insert( SerializeIn( ar, version ) );
            }
        }
    }
    catch( Exception& )
    {
        AfxThrowArchiveException( CArchiveException::generic );
    }
}

} // namespace detail

#endif // #ifndef FILEFRAGMENTS_COMPATIBILITY_HPP_INCLUDED
