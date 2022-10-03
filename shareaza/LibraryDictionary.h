//
// LibraryDictionary.h
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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

#include "SharedFile.h"

class CQueryHashTable;
class CQuerySearch;


class CLibraryDictionary
{
public:
	CLibraryDictionary();
	virtual ~CLibraryDictionary();

	void					AddFile(const CLibraryFile* pFile);
	void					RemoveFile(const CLibraryFile* pFile);
	void					BuildHashTable();					// Build hash table if needed
	void					Invalidate();						// Force dictionary and hash table to re-build
	const CQueryHashTable*	GetHashTable();
	void					Clear();
	CFileList*				Search(const CQuerySearch* pSearch, int nMaximum = 0, bool bLocal = false, bool bAvailableOnly = true);
	void					Serialize(CArchive& ar, int nVersion);

	INT_PTR					GetWordCount() const { return m_oWordMap.GetCount(); }

private:
	typedef CMap< CString, const CString&, CFileList*, CFileList*& > CWordMap;

	CWordMap			m_oWordMap;
	CQueryHashTable*	m_pTable;
	bool				m_bValid;							// Table is up to date
	DWORD				m_nSearchCookie;

	void					ProcessFile(const CLibraryFile* pFile, bool bAdd, bool bCanUpload);
	void					ProcessPhrase(const CLibraryFile* pFile, const CString& strPhrase, bool bAdd, bool bCanUpload);
	void					ProcessWord(const CLibraryFile* pFile, const CString& strWord, bool bAdd, bool bCanUpload);
};

extern CLibraryDictionary LibraryDictionary;
