//
// DownloadWithSearch.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "DownloadWithSearch.h"
#include "SearchManager.h"
#include "ManagedSearch.h"
#include "QuerySearch.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch construction

CDownloadWithSearch::CDownloadWithSearch()
	: m_bUpdateSearch	( TRUE )
	, m_tSearchTime		( 0 )
	, m_tSearchCheck	( 0 )
	, m_tLastED2KGlobal	( 0 )
	, m_tLastED2KLocal	( 0 )
{
}

CDownloadWithSearch::~CDownloadWithSearch()
{
	StopSearch();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch Can Find Sources

BOOL CDownloadWithSearch::FindSourcesAllowed(DWORD tNow) const
{
	if ( tNow > m_tSearchTime + 15*1000 )
		return TRUE;
	else
		return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch find additional sources

BOOL CDownloadWithSearch::FindMoreSources()
{
	BOOL bSuccess = CDownloadWithTiger::FindMoreSources();
	
	if ( CanSearch() )
	{
		DWORD tNow = GetTickCount();
		if ( tNow > m_tSearchTime + Settings.Downloads.SearchPeriod / 4 )
		{
			m_tSearchTime = tNow;
			if ( IsSearching() ) m_pSearch->Stop();
			bSuccess = TRUE;
		}
	}
	
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch run process

void CDownloadWithSearch::RunSearch(DWORD tNow)
{
	if ( ! CanSearch() )
	{
		StopSearch();
		return;
	}
	
	if ( tNow < m_tSearchTime + Settings.Downloads.SearchPeriod )
	{
		StartManualSearch();
	}
	else if ( tNow > m_tSearchCheck + 1000 )
	{
		BOOL bFewSources = GetEffectiveSourceCount() < Settings.Downloads.MinSources;
		BOOL bDataStarve = tNow > m_tReceived + Settings.Downloads.StarveTimeout;
		
		m_tSearchCheck = tNow;
		
		if ( bFewSources || bDataStarve )
		{
			StartAutomaticSearch();
		}
		else
		{
			StopSearch();
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch start (or continue) a manual search

void CDownloadWithSearch::StartManualSearch()
{
	PrepareSearch();

	m_pSearch->SetPriority( CManagedSearch::spHighest );
	m_pSearch->Start();
}

BOOL CDownloadWithSearch::IsSearching() const
{
	return m_pSearch && m_pSearch->IsActive();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch start (or continue) an automatic search

void CDownloadWithSearch::StartAutomaticSearch()
{
	PrepareSearch();

	m_pSearch->SetPriority( CManagedSearch::spLowest );
	m_pSearch->Start();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch check if we can actually search

BOOL CDownloadWithSearch::CanSearch() const
{
	if ( IsMoving() || IsCompleted() )
		return FALSE;

	if ( Settings.Gnutella1.EnableToday && m_oSHA1 )
		return TRUE;
	if ( Settings.Gnutella2.EnableToday && HasHash() )
		return TRUE;
	if (  Settings.eDonkey.EnableToday && m_oED2K )
		return TRUE;
	if ( Settings.DC.EnableToday && m_oTiger )
		return TRUE;

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch prepare a managed search object

void CDownloadWithSearch::PrepareSearch()
{
	if ( ! m_pSearch )
		m_pSearch = new CManagedSearch();
	else if ( ! m_bUpdateSearch )
		// Search not changed
		return;
	m_bUpdateSearch = FALSE;

	CQuerySearchPtr pSearch = m_pSearch->GetSearch();

	pSearch->m_bAndG1 = Settings.Gnutella1.EnableToday;

	if ( pSearch->m_sSearch.IsEmpty() && ! m_sName.IsEmpty() )
	{
		pSearch->m_sKeywords.Empty();
		pSearch->m_sSearch = m_sName;
		pSearch->BuildWordList( false );
	}

	if ( m_oSHA1 )
	{
		pSearch->m_oSHA1 = m_oSHA1;
	}

	if ( m_oTiger )
	{
		pSearch->m_oTiger = m_oTiger;
		m_pSearch->m_bAllowDC = TRUE;
	}
	else
	{
		m_pSearch->m_bAllowDC = FALSE;
	}

	if ( m_oED2K )
	{
		pSearch->m_oED2K = m_oED2K;
		m_pSearch->m_bAllowED2K = TRUE;
	}
	else
	{
		m_pSearch->m_bAllowED2K = FALSE;
	}

	if ( m_oBTH )
	{
		pSearch->m_oBTH = m_oBTH;
	}

	if ( m_oMD5 )
	{
		pSearch->m_oMD5 = m_oMD5;
	}
	
	pSearch->m_bWantURL	= TRUE;
	pSearch->m_bWantDN	= m_sName.IsEmpty();
	pSearch->m_bWantXML	= FALSE;
	pSearch->m_bWantPFS	= TRUE;
	pSearch->m_bWantCOM = FALSE;

	if ( m_nSize == SIZE_UNKNOWN )
	{
		pSearch->m_nMinSize = 0;
		pSearch->m_nMaxSize = SIZE_UNKNOWN;
		pSearch->m_bWantDN	= TRUE;
	}
	else
	{
		pSearch->m_nMinSize = m_nSize;
		pSearch->m_nMaxSize = m_nSize;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch stop searching

void CDownloadWithSearch::StopSearch()
{
	if ( IsSearching() )
	{
		m_pSearch->Stop();
		m_bUpdateSearch = TRUE;
	}
}
