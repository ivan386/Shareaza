//
// LibraryHistory.h
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

#if !defined(AFX_LIBRARYHISTORY_H__71D206EC_CDA8_46AD_9800_4A11C9EBAEA5__INCLUDED_)
#define AFX_LIBRARYHISTORY_H__71D206EC_CDA8_46AD_9800_4A11C9EBAEA5__INCLUDED_

#pragma once

#include "Hashes.h"

class CLibraryRecent;
class CLibraryFile;


class CLibraryHistory
{
// Construction
public:
	CLibraryHistory();
	virtual ~CLibraryHistory();
	
// Attributes
protected:
	CPtrList	m_pList;
	
// Operations
public:
	POSITION		GetIterator() const;
	CLibraryRecent*	GetNext(POSITION& pos) const;
	int				GetCount() const;
	void			Clear();
public:
	BOOL			Check(CLibraryRecent* pRecent, int nScope = 0) const;
	CLibraryRecent*	GetByPath(LPCTSTR pszPath) const;
	CLibraryRecent*	Add(LPCTSTR pszPath, const CManagedSHA1 &oSHA1, const CManagedED2K &oED2K, LPCTSTR pszSources);
	BOOL			Submit(CLibraryFile* pFile);
	void			OnFileDelete(CLibraryFile* pFile);
	void			ClearTodays();
	int				Prune();
	void			Serialize(CArchive& ar, int nVersion);

};


class CLibraryRecent
{
// Construction
public:
	CLibraryRecent();
	CLibraryRecent(LPCTSTR pszPath, const CManagedSHA1 &oSHA1, const CManagedED2K &oED2K, LPCTSTR pszSources);
	virtual ~CLibraryRecent();

// Attributes
public:
	FILETIME		m_tAdded;
	BOOL			m_bToday;
public:
	CLibraryFile*	m_pFile;
public:
	CManagedSHA1	m_oSHA1;
	CManagedED2K	m_oED2K;
	CString			m_sPath;
	CString			m_sSources;

// Operations
public:
	void	RunVerify(CLibraryFile* pFile);
	void	Serialize(CArchive& ar, int nVersion);

};

extern CLibraryHistory LibraryHistory;

#endif // !defined(AFX_LIBRARYHISTORY_H__71D206EC_CDA8_46AD_9800_4A11C9EBAEA5__INCLUDED_)
