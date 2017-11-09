//
// stdafx.h
//
// Copyright (c) Shareaza Development Team, 2009-2011.
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

#define WINVER 0x0600
#define _WIN32_WINNT 0x0600
#define _WIN32_WINDOWS 0x0600
#define _WIN32_IE 0x0600

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1		// Enable secure template overloads
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1	// Enable secure template overloads

#include <afxwin.h>
#include <tchar.h>
#include <stdio.h>
