//
// LibraryDictionary.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
// This file is part of SHAREAZA (www.shareaza.com)
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

#if !defined(AFX_LIBRARYDICTIONARY_H__6C8B6129_DF70_4AE4_895B_7C7CE7CB3192__INCLUDED_)
#define AFX_LIBRARYDICTIONARY_H__6C8B6129_DF70_4AE4_895B_7C7CE7CB3192__INCLUDED_

#pragma once

class CLibraryWord;
class CLibraryFile;
class CQueryHashTable;
class CQuerySearch;


class CLibraryDictionary  
{
// Construction
public:
	CLibraryDictionary();
	virtual ~CLibraryDictionary();
	
// Attributes
public:
	CQueryHashTable*	m_pTable;
protected:
	CMapStringToPtr		m_pWords;
	BOOL				m_bTable;
	DWORD				m_nSearchCookie;

// Operations
public:
	void				Add(CLibraryFile* pFile);
	void				Remove(CLibraryFile* pFile);
	BOOL				BuildHashTable();					//Build hash table if needed
	void				RebuildHashTable();					//Force hash table to re-build
	CQueryHashTable*	GetHashTable();
	void				Clear();
	CPtrList*			Search(CQuerySearch* pSearch, int nMaximum = 0, BOOL bLocal = FALSE);
protected:
	void				ProcessFile(CLibraryFile* pFile, BOOL bAdd);
	int					ProcessPhrase(CLibraryFile* pFile, const CString& strPhrase, BOOL bAdd, BOOL bLowercase = TRUE);
	inline void			ProcessWord(CLibraryFile* pFile, const CString& strWord, BOOL bAdd);

};


class CLibraryWord
{
// Construction
public:
	CLibraryWord();
	~CLibraryWord();

// Attributes
public:
	CLibraryFile**	m_pList;
	DWORD			m_nCount;

// Operations
public:
	inline void		Add(CLibraryFile* pFile);
	inline BOOL		Remove(CLibraryFile* pFile);

};

extern CLibraryDictionary LibraryDictionary;

#endif // !defined(AFX_LIBRARYDICTIONARY_H__6C8B6129_DF70_4AE4_895B_7C7CE7CB3192__INCLUDED_)
