//
// FileFragments.hpp
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

#ifndef FILEFRAGMENTS_HPP_INCLUDED
#define FILEFRAGMENTS_HPP_INCLUDED

#include <utility>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <deque>
#include <list>
#include <set>
#include <limits>

#ifdef _MSC_VER                 // ToDo: use boost/cstdint.hpp instead
typedef unsigned __int64 u64;   // http://sourceforge.net/projects/boost/
typedef unsigned __int32 u32;
typedef unsigned __int16 u16;
typedef unsigned __int8 u8;
typedef __int64 i64;
typedef __int32 i32;
typedef __int16 i16;
typedef __int8 i8;
#else
typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef long long i64;
typedef int i32;
typedef short i16;
typedef signed char i8;
#endif

#include "Shareaza.h"

namespace FF    // FileFragments
{

namespace detail
{

// Forward declarations

// @ Exception      The general exception class
//                  All exceptions thrown here will be of that type or
//                  derived from it, except for std::bad_alloc and
//                  exceptions thrown by the Payload class
class Exception;
// @BadFragment     Will be thrown when trying to create a fragment with
//                  negative size ( or zero size if that's forbidden by traits )
template< class FragmentT > class BadFragment;
// @BadRange        Thrown by List when trying to insert or erase a fragment
//                  which excceds the limits of the list
template< class FragmentT > class BadRange;

// @Payload         defines traits for any Payload type which serves as value
//                  type for a fragment, currently used:
//                      whether empty Fragments are allowed and
//                      the strategy used for merging fragments
template< class Payload > struct PayloadTraits;

// @Fragment        defines Fragments, takes the Payload type to be used and the
//                  integral type used to represent offsets
//                  Payload type must be of class type and have accessible default,
//                  and copy-constructors, assignment and destructor.
//                  Its Copy-c'tor and assignment are not allowed to throw.
//                  This allows a fragment to transport more information aside
//                  from the range it represents, for example a download source
//                  (to improve bad source detection and alike) or a counter
//                  that will be increased when merging occurs in a list.
//                  No function aside from contructors throw.
template
<
    class Payload = EmptyType,
    typename OffsetType = u64
>
class Fragment;
template< class FragmentT > struct CompareFragments;

// @List            Defines a near container that stores fragments and provides
//                  automatic sorting and merging (according to traits).
//                  Searching, insertion and deletion are generally done in
//                  logarithmic time.
//                  Iterators may not be used to change a fragment, but are needed
//                  in conjunction with insert and erase operations.
//                  Insert and erase always invalidate all iterators into the
//                  container except for the erase that takes a single iterators;
//                  that method only invalidates the argument.
//                  All methods are safe and transparant in the presence of
//                  exceptions, mutating operations will leave the list in a
//                  valid if unpredictable state. Swap never throws. Non-mutating
//                  standard-algorithms can be used.
template
<
    class FragmentT,
    class ContainerT = ::std::set< FragmentT, CompareFragments< FragmentT > >
>
class List;

// @Queue           A queue for simple fragments (without Payload), but
//                  without access restrictions, provides methods to erase
//                  fragments directly.
class Queue;

} // namespace detail

class EmptyType { };

#include "FileFragments/Exception.hpp"
#include "FileFragments/PayloadTraits.hpp"
#include "FileFragments/Fragment.hpp"
#include "FileFragments/List.hpp"
#include "FileFragments/Queue.hpp"

using detail::Exception;

typedef detail::Fragment< EmptyType, u64 > SimpleFragment;
typedef detail::BadFragment< SimpleFragment > SimpleBadFragment;
typedef detail::BadRange< SimpleFragment > SimpleBadRange;
typedef detail::List< SimpleFragment > SimpleFragmentList;
typedef detail::Queue SimpleFragmentQueue;

// explicit instances to force errors and improve compilation speed
template SimpleFragment;
template SimpleBadFragment;
template SimpleBadRange;
template SimpleFragmentList;

#include "FileFragments/Compatibility.hpp"

} // namespace FF

#endif // #ifndef FILEFRAGMENTS_HPP_INCLUDED
