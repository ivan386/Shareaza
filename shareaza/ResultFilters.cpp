//
// ResultFilters.cpp
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
//
// Author : roo_koo_too@yahoo.com
//
///////////////////////////////////////////////////
// ResultFilters
// Save the filters used for results
#include "stdafx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "ResultFilters.h"

CResultFilters::CResultFilters(void)
{
	m_pFilters = NULL;
	m_nFilters = 0;
	m_nDefault = NONE;
}

CResultFilters::~CResultFilters(void)
{
	if (m_pFilters)
	{
		for (DWORD i = 0; i < m_nFilters; i++)
		{
			delete m_pFilters[i];
		}
	}

	delete [] (m_pFilters);
}

void CResultFilters::Serialize(CArchive & ar)
{
	int nVersion = 1;
	
	if (ar.IsStoring())
	{
		ar << nVersion;

		ar.WriteCount(m_nFilters);

		for (DWORD i = 0; i < m_nFilters; i++)
		{
			CFilterOptions* pFilter = m_pFilters[ i ];
			pFilter->Serialize( ar );
		}
		
		ar << m_nDefault;
	}
	else
	{
		ar >> nVersion;

		m_nFilters = ar.ReadCount();

		m_pFilters = new CFilterOptions *[ m_nFilters ];
		ZeroMemory( m_pFilters, sizeof(CFilterOptions*) * m_nFilters );

		for (DWORD i = 0; i < m_nFilters; i++)
		{
			CFilterOptions* pFilter = new CFilterOptions();
			m_pFilters[ i ] = pFilter;
			pFilter->Serialize( ar );
		}

		ar >> m_nDefault;
	}
}

void CResultFilters::Add(CFilterOptions *pOptions)
{
	CFilterOptions **pFilters = new CFilterOptions * [m_nFilters + 1];

	CopyMemory(pFilters, m_pFilters, sizeof(CFilterOptions *) * m_nFilters);

	pFilters[m_nFilters++] = pOptions;

	delete [] m_pFilters;

	m_pFilters = pFilters;
}

void CResultFilters::Remove(DWORD index)
{
	if ((index >= 0) && (index < m_nFilters))
	{
		delete m_pFilters[index];
		CopyMemory(&m_pFilters[index], &m_pFilters[index + 1], sizeof(CFilterOptions *) * (m_nFilters - index));
		m_nFilters--;
	}
}

void CResultFilters::Load()
{
	CString strFile;
	CFile f;

	strFile.Format( _T("%s\\Data\\Filters.dat"), (LPCTSTR)Settings.General.Path );

	if (f.Open(strFile, CFile::modeRead))
	{
		CArchive ar(&f, CArchive::load);
		Serialize(ar);
		ar.Close();
		f.Close();
	}
}

void CResultFilters::Save()
{
	CString strFile;
	CFile f;

	strFile.Format( _T("%s\\Data\\Filters.dat"), (LPCTSTR)Settings.General.Path );

	if (f.Open(strFile, CFile::modeCreate | CFile::modeWrite))
	{
		CArchive ar(&f, CArchive::store);
		Serialize(ar);
		ar.Close();
		f.Close();
	}
}
////////////////////////////////////////////////////
// FilterOptions
// The filter settings
CFilterOptions::CFilterOptions()
{
	m_bFilterBusy		= ( Settings.Search.FilterMask & ( 1 << 0 ) ) > 0;
	m_bFilterPush		= ( Settings.Search.FilterMask & ( 1 << 1 ) ) > 0;
	m_bFilterUnstable	= ( Settings.Search.FilterMask & ( 1 << 2 ) ) > 0;
	m_bFilterReject		= ( Settings.Search.FilterMask & ( 1 << 3 ) ) > 0;
	m_bFilterLocal		= ( Settings.Search.FilterMask & ( 1 << 4 ) ) > 0;
	m_bFilterBogus		= ( Settings.Search.FilterMask & ( 1 << 5 ) ) > 0;
	m_nFilterMinSize	= 1;
	m_nFilterMaxSize	= 0;
	m_nFilterSources	= 1;
}

void CFilterOptions::Serialize(CArchive & ar)
{
	if ( ar.IsStoring() ) //saving
	{
		ar << m_sName;
		ar << m_sFilter;
		ar << m_bFilterBusy;
		ar << m_bFilterPush;
		ar << m_bFilterUnstable;
		ar << m_bFilterReject;
		ar << m_bFilterLocal;
		ar << m_bFilterBogus;
		ar << m_nFilterMinSize;
		ar << m_nFilterMaxSize;
		ar << m_nFilterSources;
	}
	else //loading
	{
		ar >> m_sName;
		ar >> m_sFilter;
		ar >> m_bFilterBusy;
		ar >> m_bFilterPush;
		ar >> m_bFilterUnstable;
		ar >> m_bFilterReject;
		ar >> m_bFilterLocal;
		ar >> m_bFilterBogus;
		ar >> m_nFilterMinSize;
		ar >> m_nFilterMaxSize;
		ar >> m_nFilterSources;
	}
}