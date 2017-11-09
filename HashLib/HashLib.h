//
// HashLib.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

#ifdef HASHLIB_EXPORTS
#define HASHLIB_API __declspec(dllexport)
#else
#define HASHLIB_API __declspec(dllimport)
#endif

#pragma warning( push )
#pragma warning( disable: 4985 ) // 'ceil': attributes not present on previous declaration.

#include "Utility.hpp"

#include "SHA.h"
#include "MD4.h"
#include "MD5.h"
#include "TigerTree.h"
#include "ED2K.h"

#pragma warning( pop )
