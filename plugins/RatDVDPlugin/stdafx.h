//
// stdafx.h
//
//	Created by:		Rolandas Rudomanskis
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

#define STRICT
#define _CRT_SECURE_NO_DEPRECATE
#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_ALL_WARNINGS
#define _HAS_EXCEPTIONS 0

#define _WIN32_WINNT 0x0501
#include <SDKDDKVer.h>

#pragma warning( push, 0 )

#include "resource.h"

#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <string>

#pragma warning( pop )

using namespace ATL;

#include "Globals.h"
