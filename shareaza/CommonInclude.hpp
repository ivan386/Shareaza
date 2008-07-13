////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CommonInclude.hpp                                                          //
//                                                                            //
// Copyright (C) 2005 Shareaza Development Team.                              //
// This file is part of SHAREAZA (shareaza.sourceforge.net).                  //
//                                                                            //
// Shareaza is free software; you can redistribute it                         //
// and/or modify it under the terms of the GNU General Public License         //
// as published by the Free Software Foundation; either version 2 of          //
// the License, or (at your option) any later version.                        //
//                                                                            //
// Shareaza is distributed in the hope that it will be useful,                //
// but WITHOUT ANY WARRANTY; without even the implied warranty of             //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       //
// See the GNU General Public License for more details.                       //
//                                                                            //
// You should have received a copy of the GNU General Public License          //
// along with Shareaza; if not, write to the                                  //
// Free Software Foundation, Inc,                                             //
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

//! \file       CommonInclude.hpp
//! \brief      Includes all needed std/boost headers

#pragma once

#pragma warning( push )
#pragma warning( disable : 4548 )
#pragma warning( disable : 4541 ) // exception.hpp : exception::what() method uses 'typeid'

#include <vector>
#include <list>
#include <deque>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <functional>
#include <algorithm>
#include <memory>
#include <iterator>
#include <limits>
#include <new>

#ifndef _WIN64
	#define BOOST_BIND_ENABLE_STDCALL 1
	#define BOOST_MEM_FN_ENABLE_STDCALL 1
#endif

#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/type_traits.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/array.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/checked_delete.hpp>

#if _MSC_VER >= 1400 
#elif _MSC_VER > 1310
#include <intrin.h>
#endif

#include "augment/augment.hpp"
using augment::implicit_cast;
using augment::auto_ptr;
using augment::auto_array;
using augment::com_ptr;
using augment::IUnknownImplementation;
#include "Utility.hpp"
#include "MinMax.hpp"
#include "Hashes.hpp"

#pragma warning( pop )
