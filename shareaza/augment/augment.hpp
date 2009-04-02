////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// augment/augment.hpp                                                        //
//                                                                            //
// Copyright (C) 2002-2007 Shareaza Development Team.                         //
// This file is part of SHAREAZA (shareaza.sourceforge.net).                          //
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

#ifndef AUGMENT_AUGMENT_HPP_INCLUDED
#define AUGMENT_AUGMENT_HPP_INCLUDED

namespace augment
{

	template<typename T, typename U>
	T implicit_cast(U u)
	{
		return u;
	}

} // namespace augment

#include "auto_ptr.hpp"
#include "auto_array.hpp"
#include "IUnknownImplementation.hpp"

#endif // #ifndef AUGMENT_AUGMENT_HPP_INCLUDED
