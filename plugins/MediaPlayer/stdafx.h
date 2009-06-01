// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_FREE_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_NO_CRT
#define _ATL_ALL_WARNINGS

#define _ATL_DEBUG_QI
#define _ATL_DEBUG_INTERFACES

#include "resource.h"

#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <atltypes.h>

#include <dshow.h>
#include <mmreg.h>

using namespace ATL;
