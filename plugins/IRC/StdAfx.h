//
// StdAfx.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#pragma once

#ifndef STRICT
#define STRICT
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400	// Change this to the appropriate value to target Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off ATL's hiding of some common and often safely ignored warning messages
#define _ATL_ALL_WARNINGS

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atlcoll.h>

#ifndef ATLENSURE_THROW
#define ATLENSURE_THROW(expr, hr)          \
	do {                                       \
	int __atl_condVal=!!(expr);            \
	ATLASSERT(__atl_condVal);              \
	if(!(__atl_condVal)) AtlThrow(hr);     \
	} while (0)
#endif // ATLENSURE

#ifndef ATLENSURE
#define ATLENSURE(expr) ATLENSURE_THROW(expr, E_FAIL)
#endif // ATLENSURE

#ifndef ATLENSURE_SUCCEEDED
#define ATLENSURE_SUCCEEDED(hr) ATLENSURE_THROW(SUCCEEDED(hr), hr)
#endif // ATLENSURE

/* Used inside COM methods that do not want to throw */
#ifndef ATLENSURE_RETURN_VAL
#define ATLENSURE_RETURN_VAL(expr, val)        \
	do {                                           \
	int __atl_condVal=!!(expr);                \
	ATLASSERT(__atl_condVal);                  \
	if(!(__atl_condVal)) return val;           \
	} while (0) 
#endif 

/* Used inside COM methods that do not want to throw */
#ifndef ATLENSURE_RETURN
#define ATLENSURE_RETURN(expr) ATLENSURE_RETURN_HR(expr, E_FAIL)
#endif 

/* Naming is slightly off in these macros
ATLENSURE_RETURN is an HRESULT return of E_FAIL
ATLENSURE_RETURN_VAL is any return value (function can pick)
ATLENSURE_RETURN_HR is HRESULT-specific, though currently the same as _VAL
*/
#ifndef ATLENSURE_RETURN_HR
#define ATLENSURE_RETURN_HR(expr, hr) ATLENSURE_RETURN_VAL(expr, hr)
#endif

#define THROW_HR(x) if( FAILED(x) ) throw x

#include "AtlControls.h"

extern HINSTANCE			v_hModule;
extern HINSTANCE			v_hResources;
extern CRITICAL_SECTION		v_csSynch;
extern BOOL					v_fRunningOnNT;

using namespace ATL;