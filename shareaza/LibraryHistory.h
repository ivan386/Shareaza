//
// LibraryHistory.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

#include "ShareazaFile.h"

class CDownload;
class CLibraryRecent;
class CLibraryFile;


class CLibraryHistory
{
public:
	CLibraryHistory();
	~CLibraryHistory();

	struct sTorrentDetails
	{
		CString			m_sName;
		CString			m_sPath;
		Hashes::BtHash  m_oBTH;
		DWORD			m_tLastSeeded;
		QWORD			m_nUploaded;
		QWORD			m_nDownloaded;
	};

	sTorrentDetails	LastSeededTorrent;		// Most recently seeded torrent (for home page button)
	sTorrentDetails	LastCompletedTorrent;	// Most recently completed torrent that didn't reach 100% ratio

	POSITION		GetIterator() const;
	CLibraryRecent*	GetNext(POSITION& pos) const;
	INT_PTR			GetCount() const;
	void			Clear();

	BOOL			Check(CLibraryRecent* pRecent, int nScope = 0) const;
    void			Add(LPCTSTR pszPath, const CDownload* pDownload = NULL);
	void			Submit(CLibraryFile* pFile);
	void			OnFileDelete(CLibraryFile* pFile);
	void			Serialize(CArchive& ar, int nVersion);

protected:
	CList< CLibraryRecent* > m_pList;

	CLibraryRecent*	GetByPath(LPCTSTR pszPath) const;
	void			Prune();
};


class CLibraryRecent
{
public:
	FILETIME					m_tAdded;
	CLibraryFile*				m_pFile;
	CString						m_sSources;
	CString						m_sPath;
    Hashes::Sha1ManagedHash		m_oSHA1;
    Hashes::TigerManagedHash	m_oTiger;
    Hashes::Md5ManagedHash		m_oMD5;
    Hashes::Ed2kManagedHash		m_oED2K;
    Hashes::BtManagedHash		m_oBTH;

protected:
	CLibraryRecent();
	CLibraryRecent(LPCTSTR pszPath, const CDownload* pDownload = NULL);

	void	RunVerify(CLibraryFile* pFile);
	void	Serialize(CArchive& ar, int nVersion);

	friend class CLibraryHistory;
};

extern CLibraryHistory LibraryHistory;
