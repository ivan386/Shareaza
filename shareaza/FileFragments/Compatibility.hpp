//
// Fragments/Compatibility.hpp
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


namespace Fragments
{

inline void SerializeOut(CArchive& ar, const Ranges::Range< uint64 >& out)
{
	ar << out.begin() << out.size();
}

inline Ranges::Range< uint64 > SerializeIn(CArchive& ar, int version)
{
	try
	{
		if ( version >= 29 )
		{
			uint64 begin, length;
			ar >> begin >> length;
			if ( begin + length < begin )
				AfxThrowArchiveException( CArchiveException::genericException );

			return Ranges::Range< uint64 >( begin, begin + length );
		}
		else
		{
			uint32 begin, length;
			ar >> begin >> length;
			if ( begin + length < begin )
				AfxThrowArchiveException( CArchiveException::genericException );

			return Ranges::Range< uint64 >( begin, begin + length );
		}
	}
	catch ( Exception& )
	{
		AfxThrowArchiveException( CArchiveException::genericException );
	}
}

// used in FragmentedFile.cpp
inline void SerializeOut1(CArchive& ar,
	const Ranges::List< Ranges::Range< uint64 >, ListTraits >& out)
{
	uint64 nTotal = out.limit();
	uint64 nRemaining = out.length_sum();
	uint32 nFragments = static_cast< uint32 >( out.size() );
	ar << nTotal << nRemaining << nFragments;

	for ( Ranges::List< Ranges::Range< uint64 >, ListTraits >::const_iterator i = out.begin();
		i != out.end(); ++i )
	{
		SerializeOut( ar, *i );
	}
}

inline void SerializeIn1(CArchive& ar,
	Ranges::List< Ranges::Range< uint64 >, ListTraits >& in, int version)
{
	if ( version >= 29 )
	{
		try
		{
			uint64 nTotal, nRemaining;
			uint32 nFragments;
			ar >> nTotal >> nRemaining >> nFragments;
			{
				Ranges::List< Ranges::Range< uint64 >, ListTraits > oNewRange( nTotal );
				in.swap( oNewRange );
			}
			for ( ; nFragments--; )
			{
				const Ranges::Range< uint64 >& fragment = SerializeIn( ar, version );
				if ( fragment.end() > nTotal )
					AfxThrowArchiveException( CArchiveException::genericException );

				in.insert( in.end(), fragment );
			}
			// Sanity check
			if ( in.length_sum() != nRemaining )
				AfxThrowArchiveException( CArchiveException::genericException );
		}
		catch ( Exception& )
		{
			AfxThrowArchiveException( CArchiveException::genericException );
		}
	}
	else
	{
		try
		{
			uint32 nTotal, nRemaining;
			uint32 nFragments;
			ar >> nTotal >> nRemaining >> nFragments;
			{
				Ranges::List< Ranges::Range< uint64 >, ListTraits > oNewRange( nTotal );
				in.swap( oNewRange );
			}
			for ( ; nFragments--; )
			{
				const Ranges::Range< uint64 >& fragment = SerializeIn( ar, version );
				if ( fragment.end() > nTotal )
					AfxThrowArchiveException( CArchiveException::genericException );

				in.insert( in.end(), fragment );
			}
			// Sanity check
			if ( in.length_sum() != nRemaining )
				AfxThrowArchiveException( CArchiveException::genericException );
		}
		catch ( Exception& )
		{
			AfxThrowArchiveException( CArchiveException::genericException );
		}
	}
}

// used in DownloadSource.cpp
inline void SerializeOut2(CArchive& ar,
	const Ranges::List< Ranges::Range< uint64 >, ListTraits >& out)
{
	ar.WriteCount( out.size() );

	for ( Ranges::List< Ranges::Range< uint64 >, ListTraits >::const_iterator i	= out.begin();
		i != out.end(); ++i )
	{
		SerializeOut( ar, *i );
	}
}

inline void SerializeIn2(CArchive& ar,
	Ranges::List< Ranges::Range< uint64 >, ListTraits >& in, int version)
{
	try
	{
		if ( version >= 20 )
		{
			for ( DWORD_PTR count = ar.ReadCount(); count--; )
			{
				const Ranges::Range< uint64 >& fragment = SerializeIn( ar, version );
				if ( fragment.end() > in.limit() )
					AfxThrowArchiveException( CArchiveException::genericException );
				in.insert( in.end(), fragment );
			}
		}
		else if ( version >= 5 )
		{
			while ( ar.ReadCount() )
			{
				const Ranges::Range< uint64 >& fragment = SerializeIn( ar, version );
				if ( fragment.end() > in.limit() )
					AfxThrowArchiveException( CArchiveException::genericException );
				in.insert( in.end(), fragment );
			}
		}
	}
	catch ( Exception& )
	{
		AfxThrowArchiveException( CArchiveException::genericException );
	}
}

} // namespace Fragments
