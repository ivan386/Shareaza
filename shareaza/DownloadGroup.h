//
// DownloadGroup.h
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

#pragma once

class CDownload;


class CDownloadGroup  
{
// Construction
public:
	CDownloadGroup();
	virtual ~CDownloadGroup();
	
// Attributes
protected:
	CPtrList	m_pDownloads;
public:
	CString		m_sName;
	CString		m_sSchemaURI;
	CString		m_sFolder;
	CStringList	m_pFilters;
public:
	int			m_nImage;
	BOOL		m_bRemoteSelected;
	
// Operations
public:
	void		Add(CDownload* pDownload);
	void		Remove(CDownload* pDownload);
	void		SetCookie(int nCookie);
	void		CopyList(CPtrList* pList);
	BOOL		Link(CDownload* pDownload);
	int			LinkAll();
	void		AddFilter(LPCTSTR pszFilter);
	void		SetSchema(LPCTSTR pszURI);
	void		Serialize(CArchive& ar, int nVersion);
	
// Inlines
public:
	inline POSITION GetIterator() const
	{
		return m_pDownloads.GetHeadPosition();
	}
	
	inline CDownload* GetNext(POSITION& pos) const
	{
		return (CDownload*)m_pDownloads.GetNext( pos );
	}
	
	inline BOOL Contains(CDownload* pDownload) const
	{
		return m_pDownloads.Find( pDownload ) != NULL;
	}
	
	inline int GetCount() const
	{
		return m_pDownloads.GetCount();
	}
	
	
};
