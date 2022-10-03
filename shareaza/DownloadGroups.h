//
// DownloadGroups.h
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

class CDownload;
class CDownloadGroup;


class CDownloadGroups
{
// Construction
public:
	CDownloadGroups();
	virtual ~CDownloadGroups();

// Attributes
public:
	mutable CCriticalSection	m_pSection;

protected:
	CList< CDownloadGroup* > m_pList;
	CDownloadGroup*		m_pSuper;
	int					m_nBaseCookie;
	int					m_nSaveCookie;
	int					m_nGroupCookie;

// Operations
public:
	// Get all download group target folders (unique)
	void				GetFolders(CStringIList& oFolders) const;
	CDownloadGroup*		GetSuperGroup();
	CDownloadGroup*		Add(LPCTSTR pszName = NULL, BOOL bTemporary = FALSE, BOOL bUseExisting = FALSE);
	void				Remove(CDownloadGroup* pGroup);
	void				Link(CDownload* pDownload);
	void				Unlink(CDownload* pDownload, BOOL bAndSuper = TRUE);
	void				CreateDefault();
	CString				GetCompletedPath(CDownload* pDownload) const;

	void				Clear();
	BOOL				Load();
	BOOL				Save(BOOL bForce = TRUE);

protected:
	void				Serialize(CArchive& ar);
	void				CleanTemporary();

// Inlines
public:
	inline POSITION GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}

	inline CDownloadGroup* GetNext(POSITION& pos) const
	{
		return m_pList.GetNext( pos );
	}

	inline INT_PTR GetCount() const
	{
		return m_pList.GetCount();
	}

	inline BOOL Check(CDownloadGroup* pGroup) const
	{
		return m_pList.Find( pGroup ) != NULL;
	}

	inline int GetGroupCookie() const
	{
		return m_nGroupCookie;
	}

	inline void IncBaseCookie()
	{
		m_nBaseCookie ++;
	}

private:
	CDownloadGroups(const CDownloadGroups&);
	CDownloadGroups& operator=(const CDownloadGroups&);
};

extern CDownloadGroups DownloadGroups;
