//
// stdafx.h
//
// Copyright (c) Nikolay Raspopov, 2009-2010.
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

#define _WMP	// Define it to use Windows Media Player object otherwise DirectShow will be used

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#ifdef _WMP
	#define _ATL_APARTMENT_THREADED
#else
	#define _ATL_FREE_THREADED
#endif

#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_NO_CRT
#define _ATL_ALL_WARNINGS

#include "resource.h"

#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <atltypes.h>
#include <atlwin.h>

#ifdef _WMP
	#include <wmp.h>
#else
	#include <dshow.h>
#endif // _WMP

using namespace ATL;
