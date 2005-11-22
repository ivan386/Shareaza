////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// jconfig.h                                                                  //
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

#ifndef JCONFIG_H_INCLUDED
#define JCONFIG_H_INCLUDED


#define HAVE_PROTOTYPES
#define HAVE_UNSIGNED_CHAR
#define HAVE_UNSIGNED_SHORT
#undef CHAR_IS_UNSIGNED
#define HAVE_STDDEF_H
#define HAVE_STDLIB_H
#undef NEED_BSD_STRINGS
#undef NEED_SYS_TYPES_H
#undef NEED_FAR_POINTERS
#undef NEED_SHORT_EXTERNAL_NAMES
#undef INCOMPLETE_TYPES_BROKEN
#define HAVE_BOOLEAN
typedef unsigned char boolean;

#ifdef JPEG_INTERNALS

#undef RIGHT_SHIFT_IS_UNSIGNED

#endif

#endif // #ifndef JCONFIG_H_INCLUDED
