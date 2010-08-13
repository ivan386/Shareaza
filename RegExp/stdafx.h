//
// stdafx.h
//
// Copyright (c) Shareaza Development Team, 2010.
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

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#define _SCL_SECURE_NO_WARNINGS

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#pragma warning(disable:4820) // X bytes padding added after data member 'Y'
#pragma warning(disable:4365) // conversion from '' to '', signed/unsigned mismatch
#pragma warning(disable:4548) // expression before comma has no effect; expected expression with side-effect
#pragma warning(disable:4555) // expression has no effect; expected expression with side-effect
#pragma warning(disable:4571) // catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
#pragma warning(disable:4619) // #pragma warning : there is no warning number
#pragma warning(disable:4625) // copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning(disable:4668) // is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning(disable:4686) // possible change in behavior, change in UDT return calling convention

#include <atlconv.h>
#include <tchar.h>

#include "regexpr2.h"

using namespace ATL;
