//
// ResultFilters.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
//
// Author : roo_koo_too@yahoo.com
//
#pragma once

//create a class with some of the CMatchObjects members

class CFilterOptions
{
public:
	CFilterOptions();
	void Serialize(CArchive& ar, int nVersion);
public:
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
	DWORD	m_nFilterSources;
	QWORD	m_nFilterMinSize;
	QWORD	m_nFilterMaxSize;
};

const DWORD NONE = ~0u;

class CResultFilters
{
public:
	CResultFilters(void);
	~CResultFilters(void);
	void Serialize(CArchive& ar);
	void Add(CFilterOptions *pOptions);
	void Remove(DWORD index);
	void Load();
	void Save();
	int Search(const CString& strName);
public:
	DWORD				m_nFilters; // count of filter options
	CFilterOptions **	m_pFilters; // the array of filter options
	DWORD				m_nDefault; // the index of the default filter options
};