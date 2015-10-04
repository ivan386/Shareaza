//
// StdAfx.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
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

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1		// Enable secure template overloads
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1	// Enable secure template overloads

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// Makes certain ATL CString constructors explicit, preventing any unintentional conversions
#define _ATL_CSTRING_NO_CRT
#define _ATL_NO_COM_SUPPORT					// Prevents ATL COM-related code from being compiled

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxmt.h>			// MFC multithreading
#include <atlbase.h>
#include <shlobj.h>			// Shell objects
#include <shlwapi.h>

typedef unsigned __int64 QWORD;

#ifndef BIF_NEWDIALOGSTYLE
	#define BIF_NEWDIALOGSTYLE	0x0040
#endif
#ifndef OFN_ENABLESIZING
	#define OFN_ENABLESIZING	0x00800000
#endif

#include "..\HashLib\HashLib.h"
#include "Strings.h"
#include "Buffer.h"
#include "BENode.h"

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
