//
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently
//
// Copyright (c) Shareaza Development Team, 2007-2014.
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

#ifndef _SECURE_ATL
	#define _SECURE_ATL	1
#endif

#define _ATL_FREE_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_NO_CRT
#define _ATL_ALL_WARNINGS

#define ISOLATION_AWARE_ENABLED 1

#define _WIN32_WINNT 0x0501
#include <SDKDDKVer.h>

#include "resource.h"

#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <atlcoll.h>

#pragma pack(push)

#include "unrar.h"

#pragma pack(pop)

// RAROpenArchiveDataEx::Flags

#define RAR_HEAD_VOLUME_ATTR	0x0001	// Volume attribute (archive volume)
#define RAR_HEAD_COMMENT		0x0002	// Archive comment present
#define RAR_HEAD_LOCKED			0x0004	// Archive lock attribute
#define RAR_HEAD_SOLID			0x0008	// Solid attribute (solid archive)
#define RAR_HEAD_NEW_NAMING		0x0010	// New volume naming scheme (‘volname.partN.rar’)
#define RAR_HEAD_AUTH			0x0020	// Authenticity information present
#define RAR_HEAD_RECOVERY		0x0040	// Recovery record present
#define RAR_HEAD_ENCRYPTED		0x0080	// Block headers are encrypted
#define RAR_HEAD_FIRST_VOLUME	0x0100	// First volume (set only by RAR 3.0 and later)

// RARHeaderDataEx::Flags

#define RAR_FILE_PREVIOUS		0x01	// file continued from previous volume
#define RAR_FILE_NEXT			0x02	// file continued on next volume
#define RAR_FILE_ENCRYPTED		0x04	// file encrypted with password
#define RAR_FILE_COMMENT		0x08	// file comment present
#define RAR_FILE_SOLID			0x10	// compression of previous files is used (solid flag)
#define RAR_FILE_DIRECTORY		0xe0	// file is directory

typedef HANDLE (PASCAL *tRAROpenArchiveEx)(struct RAROpenArchiveDataEx *ArchiveData);
typedef int    (PASCAL *tRARCloseArchive)(HANDLE hArcData);
typedef int    (PASCAL *tRARReadHeaderEx)(HANDLE hArcData,struct RARHeaderDataEx *HeaderData);
typedef int    (PASCAL *tRARProcessFileW)(HANDLE hArcData,int Operation,wchar_t *DestPath,wchar_t *DestName);

extern tRAROpenArchiveEx fnRAROpenArchiveEx;
extern tRARCloseArchive fnRARCloseArchive;
extern tRARReadHeaderEx fnRARReadHeaderEx;
extern tRARProcessFileW fnRARProcessFileW;

using namespace ATL;

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
