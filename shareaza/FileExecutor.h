//
// FileExecutor.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

class CMediaWnd;
class CLibraryWnd;


class CFileExecutor
{
public:
	static BOOL		Execute(LPCTSTR pszFile, BOOL bSkipSecurityCheck = FALSE, BOOL bHasThumbnail = FALSE, LPCTSTR pszExt = NULL);
	static BOOL		Enqueue(LPCTSTR pszFiles, BOOL bSkipSecurityCheck = FALSE, LPCTSTR pszExt = NULL);
	static BOOL		ShowBitziTicket(DWORD nFile);
	static BOOL		DisplayURL(LPCTSTR pszURL);

protected:
	// Open Media Player window
	static CMediaWnd*	GetMediaWindow(BOOL bFocus);

	// Open Library window
	static CLibraryWnd*	GetLibraryWindow();

	// Is file a video, audio or image file?
	static void DetectFileType(LPCTSTR pszFile, bool& bVideo, bool& bAudio, bool& bImage);
};
