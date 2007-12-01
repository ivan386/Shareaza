//
// StdAfx.h
//
// This software is released into the public domain. You are free to
// redistribute and modify without any restrictions.
// This file is part of Shareaza (www.shareaza.com), original author Michael Stokes. 
//

#pragma once

#define STRICT
#define WINVER 0x0400
#define _WIN32_WINNT 0x0400
#define _WIN32_WINDOWS 0x0410
#define _WIN32_IE 0x0400
#define _WIN32_DCOM
#define _ATL_APARTMENT_THREADED
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_NO_CRT
#define _ATL_ALL_WARNINGS

#pragma warning( push, 0 )

#include "resource.h"

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>

using namespace ATL;

#pragma warning( pop )
