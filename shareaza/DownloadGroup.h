//
// DownloadGroup.h
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

class CDownload;


class CDownloadGroup
{
// Construction
public:
	CDownloadGroup(const LPCTSTR szName = NULL, const BOOL bTemporary = FALSE);
	virtual ~CDownloadGroup();

// Attributes
protected:
	CList< CDownload* >	m_pDownloads;		// List of linked downloads

	// Temporary group: TRI_UNKNOWN	- Persistent group;
	//                  TRI_FALSE	- Temporary group, not completed yet;
	//                  TRI_TRUE	- Temporary group, feel free to delete.
	TRISTATE			m_bTemporary;

public:
	CString				m_sName;			// Group name
	CString				m_sSchemaURI;		// Default schema (used to fill filters list)
	CString				m_sFolder;			// Folder for completed downloads
	CList< CString >	m_pFilters;			// Filters list
	int					m_nImage;			// 16x16 group icon
	BOOL				m_bRemoteSelected;	// Active(selected) group for Remote Interface
	BOOL				m_bTorrent;			// Filter BitTorrent downloads

// Operations
public:
	void		Add(CDownload* pDownload);
	void		Remove(CDownload* pDownload);
	void		Clear();
	void		SetCookie(int nCookie);
	void		CopyList(CList< CDownload* >& pList);
	BOOL		Link(CDownload* pDownload);
	int			LinkAll();
	void		AddFilter(const CString& strFilter);
	void		RemoveFilter(const CString& strFilter);
	void		SetSchema(LPCTSTR pszURI, BOOL bRemoveOldFilters = FALSE);
	void		SetFolder(LPCTSTR pszFolder);
	void		Serialize(CArchive& ar, const int nVersion);
	BOOL		IsTemporary();
	void		SetDefaultFilters();	// Load file extensions from schema

// Inlines
public:
	inline POSITION GetIterator() const
	{
		return m_pDownloads.GetHeadPosition();
	}

	inline CDownload* GetNext(POSITION& pos) const
	{
		return m_pDownloads.GetNext( pos );
	}

	inline BOOL Contains(CDownload* pDownload) const
	{
		return m_pDownloads.Find( pDownload ) != NULL;
	}

	inline INT_PTR GetCount() const
	{
		return m_pDownloads.GetCount();
	}
};
