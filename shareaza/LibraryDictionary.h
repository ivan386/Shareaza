//
// LibraryDictionary.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

class CLibraryFile;
class CQueryHashTable;
class CQuerySearch;

typedef CList< const CLibraryFile* > FilePtrList;


class CLibraryDictionary : private ::boost::noncopyable
{
private:
	typedef CMap< CString, const CString&, FilePtrList*, FilePtrList* > WordMap;

// Construction
public:
	CLibraryDictionary();
	virtual ~CLibraryDictionary();

// Attributes
private:
	WordMap				m_oWordMap;
	CQueryHashTable*	m_pTable;
	bool				m_bValid;							// Table is up to date
	DWORD				m_nSearchCookie;

// Operations
public:
	void					AddFile(const CLibraryFile& oFile);
	void					RemoveFile(const CLibraryFile& oFile);
	const bool				BuildHashTable();					// Build hash table if needed
	void					RebuildHashTable();					// Force hash table to re-build
	const CQueryHashTable&	GetHashTable();
	void					Clear();
	FilePtrList*			Search(const CQuerySearch& oSearch, int nMaximum = 0, bool bLocal = false, bool bAvailableOnly = true);
	void					Serialize(CArchive& ar, int nVersion);
private:
	void					ProcessFile(const CLibraryFile& oFile, bool bAdd, bool bCanUpload);
	void					ProcessPhrase(const CLibraryFile& oFile, const CString& strPhrase, bool bAdd, bool bCanUpload);
	void					MakeKeywords(const CLibraryFile& oFile, const CString& strWord, WORD nWordType, bool bAdd, bool bCanUpload);
	void					ProcessWord(const CLibraryFile& oFile, const CString& strWord, bool bAdd, bool bCanUpload);
	const bool				CanUpload(const CLibraryFile& oFile) const;
	void					AddHashes(const CLibraryFile& oFile);
};

extern CLibraryDictionary LibraryDictionary;
