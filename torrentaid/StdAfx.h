//
// StdAfx.h
//
// Copyright (c) Shareaza Pty. Ltd., 2003.
// This file is part of TorrentAid Torrent Wizard (www.torrentaid.com).
//
// TorrentAid Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// TorrentAid is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TorrentAid; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#if !defined(AFX_STDAFX_H__B26FAC77_146E_41B4_A688_35985BA9F7B8__INCLUDED_)
#define AFX_STDAFX_H__B26FAC77_146E_41B4_A688_35985BA9F7B8__INCLUDED_

#pragma once

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define WINVER 0x0400		// Windows Version

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxmt.h>			// MFC multithreading

#include <shlobj.h>			// Shell objects

//{{AFX_INSERT_LOCATION}}

typedef unsigned __int64 QWORD;

#define BIF_NEWDIALOGSTYLE	0x0040
#define OFN_ENABLESIZING	0x00800000

#endif // !defined(AFX_STDAFX_H__B26FAC77_146E_41B4_A688_35985BA9F7B8__INCLUDED_)
