//
// FileExecutor.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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


class CFileExecutor
{
public:
	// Is file extension safe to execute?
	// Returns: TRI_TRUE - safe, TRI_FALSE - dangerous, TRI_UNKNOWN - cancel operation
	static TRISTATE IsSafeExecute(LPCTSTR szExt, LPCTSTR szFile = NULL);
	// Is file verified?
	// Returns: TRI_TRUE - secure, TRI_FALSE - insecure, TRI_UNKNOWN - cancel operation
	static TRISTATE	IsVerified(LPCTSTR szFile);
	// Execute one file
	static BOOL		Execute(LPCTSTR pszFile, LPCTSTR pszExt = NULL);
	// Execute file list
	static BOOL		Execute(const CStringList& pList);
	// Enqueue one file
	static BOOL		Enqueue(LPCTSTR pszFile, LPCTSTR pszExt = NULL);
	// Enqueue file list
	static BOOL		Enqueue(const CStringList& pList);

	static BOOL		DisplayURL(LPCTSTR pszURL);

protected:
	// Is file a video, audio or image file?
	static void DetectFileType(LPCTSTR pszFile, LPCTSTR szType, bool& bVideo, bool& bAudio, bool& bImage);

	// Extracts player form settings
	static CString GetCustomPlayer();
};
