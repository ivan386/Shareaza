//
// DownloadWithSearch.cpp
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
{
	m_pSearch		= NULL;
	m_tSearchTime	= 0;
	m_tSearchCheck	= 0;
}

CDownloadWithSearch::~CDownloadWithSearch()
{
	if ( m_pSearch ) delete m_pSearch;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch find additional sources

BOOL CDownloadWithSearch::FindMoreSources()
{
	BOOL bSuccess = CDownloadWithTiger::FindMoreSources();
	
	if ( CanSearch() )
	{
		m_tSearchTime = GetTickCount();
		if ( m_pSearch != NULL ) m_pSearch->Stop();
		bSuccess = TRUE;
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
	
	if ( tNow > m_tSearchTime && tNow - m_tSearchTime < Settings.Downloads.SearchPeriod )
	{
		StartManualSearch();
	}
	else if ( tNow > m_tSearchCheck && tNow - m_tSearchCheck >= 1000 )
	{
		BOOL bFewSources = GetSourceCount( FALSE, TRUE ) < Settings.Downloads.MinSources;
		BOOL bDataStarve = ( tNow > m_tReceived ? tNow - m_tReceived : 0 ) > Settings.Downloads.StarveTimeout * 1000;
		
		m_tSearchCheck = tNow;
		
		if ( IsPaused() == FALSE && ( bFewSources || bDataStarve ) )
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
	CSingleLock pLock( &SearchManager.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;
	
	PrepareSearch();
	
	m_pSearch->m_nPriority = CManagedSearch::spHighest;
	m_pSearch->Start();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch start (or continue) an autoamtic search

void CDownloadWithSearch::StartAutomaticSearch()
{
	CSingleLock pLock( &SearchManager.m_pSection );
	if ( ! pLock.Lock( 10 ) ) return;
	
	PrepareSearch();
	
	m_pSearch->m_nPriority = CManagedSearch::spLowest;
	m_pSearch->Start();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch check if we can actually search

BOOL CDownloadWithSearch::CanSearch() const
{
	return m_pFile != NULL &&
		( m_bSHA1 || m_bTiger || m_bED2K || m_bBTH );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch prepare a managed search object

void CDownloadWithSearch::PrepareSearch()
{
	if ( m_pSearch == NULL ) m_pSearch = new CManagedSearch();
	CQuerySearch* pSearch = m_pSearch->m_pSearch;
	
	if ( pSearch->m_bAndG1 )
	{
		pSearch->m_sSearch = m_sRemoteName;
		pSearch->BuildWordList();
	}
	
	if ( m_bSHA1 )
	{
		pSearch->m_bSHA1 = TRUE;
		pSearch->m_pSHA1 = m_pSHA1;
	}
	if ( m_bTiger )
	{
		pSearch->m_bTiger = TRUE;
		pSearch->m_pTiger = m_pTiger;
	}
	if ( m_bED2K )
	{
		pSearch->m_bED2K = TRUE;
		pSearch->m_pED2K = m_pED2K;
	}
	if ( m_bBTH )
	{
		pSearch->m_bBTH = TRUE;
		pSearch->m_pBTH = m_pBTH;
		
		// TODO: This is a very dirty hack to pass BT searches, remove it
		
		if ( pSearch->m_sSearch.IsEmpty() )
		{
			pSearch->m_sSearch = _T("BTIH");
			pSearch->BuildWordList();
		}
	}
	
	pSearch->m_bWantURL	= TRUE;
	pSearch->m_bWantDN	= ( m_sRemoteName.GetLength() == 0 || m_nSize == SIZE_UNKNOWN );
	pSearch->m_bWantXML	= FALSE;
	pSearch->m_bWantPFS	= TRUE;
	pSearch->m_bWantCOM = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSearch stop searching

void CDownloadWithSearch::StopSearch()
{
	if ( m_pSearch != NULL ) m_pSearch->Stop();
}
