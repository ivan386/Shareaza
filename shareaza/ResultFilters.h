//
// ResultFilters.h
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


class CFilterOptions
{
public:
	CFilterOptions();

	void Serialize(CArchive& ar, int nVersion);

	CString m_sName; // the options set name
	CString	m_sFilter;
	BOOL	m_bFilterBusy;
	BOOL	m_bFilterPush;
	BOOL	m_bFilterUnstable;
	BOOL	m_bFilterLocal;
	BOOL	m_bFilterReject;
	BOOL	m_bFilterBogus;
	BOOL	m_bFilterDRM;
	BOOL	m_bFilterAdult;
	BOOL	m_bFilterSuspicious;
	BOOL	m_bRegExp;
	DWORD	m_nFilterSources;
	QWORD	m_nFilterMinSize;
	QWORD	m_nFilterMaxSize;
};


class CResultFilters
{
public:
	CResultFilters();
	~CResultFilters();

	void Add(CFilterOptions *pOptions);
	void Remove(DWORD index);
	BOOL Load();
	BOOL Save();
	int Search(const CString& strName) const;

	DWORD				m_nFilters; // count of filter options
	CFilterOptions **	m_pFilters; // the array of filter options
	DWORD				m_nDefault; // the index of the default filter options

private:
	mutable CCriticalSection m_pSection;

	void Clear();
	void Serialize(CArchive& ar);
};

const DWORD NONE = ~0u;
