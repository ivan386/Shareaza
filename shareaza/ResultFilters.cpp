//
// ResultFilters.cpp
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

#include "stdafx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "ResultFilters.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CResultFilters::CResultFilters()
	: m_pFilters( NULL )
	, m_nFilters( 0 )
	, m_nDefault( NONE )
{
}

CResultFilters::~CResultFilters()
{
	Clear();
}

void CResultFilters::Clear()
{
	CQuickLock oLock( m_pSection );

	if ( m_pFilters )
	{
		for ( DWORD i = 0; i < m_nFilters; i++ )
		{
			delete m_pFilters[ i ];
			m_pFilters[ i ] = NULL;
		}
	}

	delete [] ( m_pFilters );
	m_pFilters = NULL;
	m_nFilters = 0;
	m_nDefault = NONE;
}

void CResultFilters::Serialize(CArchive & ar)
{
	int nVersion = 2;

	if ( ar.IsStoring() )
	{
		ar << nVersion;

		ar.WriteCount(m_nFilters);

		for (DWORD i = 0; i < m_nFilters; i++)
		{
			CFilterOptions* pFilter = m_pFilters[ i ];
			pFilter->Serialize( ar, nVersion );
		}

		ar << m_nDefault;
	}
	else
	{
		ar >> nVersion;

		m_nFilters = static_cast< DWORD >( ar.ReadCount() );

		m_pFilters = new CFilterOptions *[ m_nFilters ];
		ZeroMemory( m_pFilters, sizeof(CFilterOptions*) * m_nFilters );

		for (DWORD i = 0; i < m_nFilters; i++)
		{
			CAutoPtr< CFilterOptions > pFilter( new CFilterOptions() );
			pFilter->Serialize( ar, nVersion);
			m_pFilters[ i ] = pFilter.Detach();
		}

		ar >> m_nDefault;
	}
}

void CResultFilters::Add(CFilterOptions *pOptions)
{
	CQuickLock oLock( m_pSection );

	CFilterOptions **pFilters = new CFilterOptions * [m_nFilters + 1];

	CopyMemory(pFilters, m_pFilters, sizeof(CFilterOptions *) * m_nFilters);

	pFilters[m_nFilters++] = pOptions;

	delete [] m_pFilters;

	m_pFilters = pFilters;
}

// Search for (first) filter with name strName, return index if found, -1 (NONE) otherwise
int CResultFilters::Search(const CString& strName) const
{
	CQuickLock oLock( m_pSection );

	for ( DWORD index = 0; index < m_nFilters; index++ )
	{
		if ( strName.Compare( m_pFilters[index]->m_sName ) == 0 )
		{
			return index;
		}
	}
	return NONE;
 }

void CResultFilters::Remove(DWORD index)
{
	CQuickLock oLock( m_pSection );

	if ( index < m_nFilters )
	{
		delete m_pFilters[index];
		CopyMemory(&m_pFilters[index], &m_pFilters[index + 1], sizeof(CFilterOptions *) * (m_nFilters - index));
		m_nFilters--;

		if ( index == m_nDefault ) m_nDefault = NONE;
		else if ( ( m_nDefault != NONE ) && ( index < m_nDefault ) ) m_nDefault--;

		if ( m_nFilters == 0 ) m_nDefault = NONE;
	}
}

BOOL CResultFilters::Load()
{
	CQuickLock oLock( m_pSection );

	// Delete old content first
	Clear();

	CString strFile = Settings.General.UserPath + _T("\\Data\\Filters.dat");

	CFile pFile;
	if ( ! pFile.Open( strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
		return FALSE;

	try
	{
		CArchive ar( &pFile, CArchive::load );	// 4 KB buffer
		try
		{
			Serialize( ar );

			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			theApp.Message( MSG_ERROR, _T("Failed to load result filters: %s"), (LPCTSTR)strFile );
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		theApp.Message( MSG_ERROR, _T("Failed to load result filters: %s"), (LPCTSTR)strFile );
		return FALSE;
	}

	return TRUE;
}

BOOL CResultFilters::Save()
{
	CQuickLock oLock( m_pSection );

	CString strTemp = Settings.General.UserPath + _T("\\Data\\Filters.tmp");
	CString strFile = Settings.General.UserPath + _T("\\Data\\Filters.dat");

	if ( m_nFilters == 0 )
	{
		DeleteFile( strFile );
		return TRUE;
	}

	CFile pFile;
	if ( ! pFile.Open( strTemp, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive | CFile::osSequentialScan ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, _T("Failed to save result filters: %s"), (LPCTSTR)strTemp );
		return FALSE;
	}

	try
	{
		CArchive ar( &pFile, CArchive::store );	// 4 KB buffer
		try
		{
			Serialize( ar );

			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			theApp.Message( MSG_ERROR, _T("Failed to save result filters: %s"), (LPCTSTR)strTemp );
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		theApp.Message( MSG_ERROR, _T("Failed to save result filters: %s"), (LPCTSTR)strTemp );
		return FALSE;
	}

	if ( ! MoveFileEx( strTemp, strFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, _T("Failed to save result filters: %s"), (LPCTSTR)strFile );
		return FALSE;
	}

	return TRUE;
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
	m_bFilterDRM		= ( Settings.Search.FilterMask & ( 1 << 6 ) ) > 0;
	m_bFilterAdult		= ( Settings.Search.FilterMask & ( 1 << 7 ) ) > 0;
	m_bFilterSuspicious = ( Settings.Search.FilterMask & ( 1 << 8 ) ) > 0;
	m_bRegExp			= ( Settings.Search.FilterMask & ( 1 << 9 ) ) > 0;
	m_nFilterMinSize	= 1;
	m_nFilterMaxSize	= 0;
	m_nFilterSources	= 1;
}

void CFilterOptions::Serialize(CArchive & ar, int nVersion)
{
	if ( ar.IsStoring() ) // saving
	{
		ar << m_sName;
		ar << m_sFilter;
		ar << m_bFilterBusy;
		ar << m_bFilterPush;
		ar << m_bFilterUnstable;
		ar << m_bFilterReject;
		ar << m_bFilterLocal;
		ar << m_bFilterBogus;
		ar << m_bFilterDRM;
		ar << m_bFilterAdult;
		ar << m_bFilterSuspicious;
		ar << m_bRegExp;
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

		if ( nVersion >= 2 )
		{
			ar >> m_bFilterDRM;
			ar >> m_bFilterAdult;
			ar >> m_bFilterSuspicious;
			ar >> m_bRegExp;
			if ( m_sFilter.IsEmpty() )
				m_bRegExp = FALSE;
		}

		ar >> m_nFilterMinSize;
		ar >> m_nFilterMaxSize;
		ar >> m_nFilterSources;
	}
}
