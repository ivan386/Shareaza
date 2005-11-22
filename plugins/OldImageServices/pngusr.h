////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// pngusr.h                                                                   //
//                                                                            //
// Copyright (C) 2005 Shareaza Development Team.                              //
// This file is part of SHAREAZA (www.shareaza.com).                          //
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

#ifndef PNGUSR_H_INCLUDED
#define PNGUSR_H_INCLUDED

#include <exception>

#define PNG_USE_LOCAL_ARRAYS
#define PNG_ABORT png_abort

struct PngException : public std::exception {};

inline void png_abort()
{
	throw PngException();
}

#endif // #ifndef PNGUSR_H_INCLUDED
