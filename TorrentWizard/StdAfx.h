//
// StdAfx.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

#define NTDDI_VERSION	NTDDI_LONGHORN	// Minimum build target
#define _WIN32_WINNT	0x0600			// Vista, 2008
#include <sdkddkver.h>					// Setup versioning for windows SDK/DDK
#define VC_EXTRALEAN					// Exclude rarely-used stuff from Windows headers
#define BOOST_USE_WINDOWS_H

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxmt.h>			// MFC multithreading
#include <shlobj.h>			// Shell objects
#include <shlwapi.h>

#include <boost\type_traits\is_same.hpp>
#include <boost\checked_delete.hpp>
#include "..\shareaza\augment\auto_ptr.hpp"
#include "..\shareaza\augment\auto_array.hpp"

#include "..\HashLib\HashLib.h"
#include "..\Shareaza\Strings.h"
#include "..\Shareaza\Buffer.h"
#include "..\Shareaza\BENode.h"

typedef unsigned __int64 QWORD;

#ifndef BIF_NEWDIALOGSTYLE
	#define BIF_NEWDIALOGSTYLE	0x0040
#endif
#ifndef OFN_ENABLESIZING
	#define OFN_ENABLESIZING	0x00800000
#endif

using namespace augment;
