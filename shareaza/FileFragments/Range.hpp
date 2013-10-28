//
// Filefragments/Range.hpp
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

#ifndef FILEFRAGMENTS_RANGE_HPP_INCLUDED
#define FILEFRAGMENTS_RANGE_HPP_INCLUDED

namespace Ranges
{

// helper class that stands for ... nothing
class EmptyType
{
public:
	// Anything can be turned into nothing
	template< typename T >
	EmptyType(const T&) { }
	// From nothing we get something ;)
	EmptyType() { }
};

template< typename SizeT, class PayloadT = EmptyType >
class Range : private PayloadT
{
public:
	typedef SizeT size_type;
	typedef PayloadT payload_type;
	typedef RangeError< Range > RangeException;
	Range(size_type begin, size_type end, const payload_type& payload = payload_type())
	: PayloadT( payload ), m_range( begin, end )
	{
		if ( end < begin )
		{
//			throw RangeException( begin, end, payload );
			m_range.first = ~0ULL - 1000;
			m_range.second = ~0ULL;
			CString msg;
			msg.Format( _T( "RangeError - default - begin: %I64u - end: %I64u" ), begin, end );
			theApp.Message( MSG_ERROR, msg );
		}
	}   
	Range(std::pair< size_type, size_type > range, const payload_type& payload = payload_type())
	: PayloadT( payload ), m_range( range )
	{
		if ( range.second < range.first )
		{
//			throw RangeException( range.first, range.second, payload );
			m_range.first = ~0ULL - 1000;
			m_range.second = ~0ULL;
			CString msg;
			msg.Format( _T( "RangeError - pair - begin: %I64u - end: %I64u" ), range.first, range.second );
			theApp.Message( MSG_ERROR, msg );
		}
	}

	// conversion - will succeed if payload is convertible
	template< class OtherPayloadT >
	Range(const Range< size_type, OtherPayloadT >& other)
	: PayloadT( other.value() ), m_range( other.m_range )
	{ }

	size_type begin() const { return m_range.first; }
	size_type end() const { return m_range.second; }
	size_type size() const { return end() - begin(); }

	payload_type& value() { return *this; }
	const payload_type& value() const { return *this; }
private:
	std::pair< size_type, size_type > m_range;
};

template< typename SizeT, class PayloadT >
bool operator==(const Range< SizeT, PayloadT >& lhs, const Range< SizeT, PayloadT >& rhs)
{
	return lhs.begin() == rhs.begin() && lhs.end() == rhs.end() && lhs.value() == rhs.value();
}
template< typename SizeT >
bool operator==(const Range< SizeT >& lhs, const Range< SizeT >& rhs)
{
	return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
}

template< typename SizeT, class PayloadT >
bool operator!=(const Range< SizeT, PayloadT >& lhs, const Range< SizeT, PayloadT >& rhs)
{
	return !( lhs == rhs );
}

template< typename SizeT, class PayloadT >
struct RangeCompare
: public std::binary_function< Range< SizeT, PayloadT >, Range< SizeT, PayloadT >, bool >
{
	result_type operator()(first_argument_type lhs, second_argument_type rhs) const
	{
		return lhs.end() <= rhs.begin();
	}
};

} // namespace Ranges

#endif // #ifndef FILEFRAGMENTS_RANGE_HPP_INCLUDED
