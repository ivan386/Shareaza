//
// LibraryHistory.cpp
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
#include "Library.h"
#include "LibraryHistory.h"
#include "SharedFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CLibraryHistory LibraryHistory;


//////////////////////////////////////////////////////////////////////
// CLibraryHistory construction

CLibraryHistory::CLibraryHistory()
{
}

CLibraryHistory::~CLibraryHistory()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory file list

POSITION CLibraryHistory::GetIterator() const
{
	return m_pList.GetHeadPosition();
}

CLibraryRecent* CLibraryHistory::GetNext(POSITION& pos) const
{
	return (CLibraryRecent*)m_pList.GetNext( pos );
}

int CLibraryHistory::GetCount() const
{
	return m_pList.GetCount();
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory clear

void CLibraryHistory::Clear()
{
	for ( POSITION pos = GetIterator() ; pos ; ) delete GetNext( pos );
	m_pList.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory check

BOOL CLibraryHistory::Check(CLibraryRecent* pRecent, int nScope) const
{
	if ( nScope == 0 ) return m_pList.Find( pRecent ) != NULL;
	
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos && nScope > 0 ; )
	{
		CLibraryRecent* pExisting = (CLibraryRecent*)m_pList.GetNext( pos );
		if ( pRecent == pExisting ) return TRUE;
		if ( pExisting->m_pFile != NULL ) nScope--;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory lookups

CLibraryRecent* CLibraryHistory::GetByPath(LPCTSTR pszPath) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CLibraryRecent* pRecent = GetNext( pos );
		if ( pRecent->m_sPath.CompareNoCase( pszPath ) == 0 ) return pRecent;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory add new download

CLibraryRecent* CLibraryHistory::Add(LPCTSTR pszPath, const CManagedSHA1 &oSHA1, const CManagedED2K &oED2K, LPCTSTR pszSources)
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 500 ) ) return NULL;
	
	CLibraryRecent* pRecent = GetByPath( pszPath );
	if ( pRecent != NULL ) return pRecent;
	
	pRecent = new CLibraryRecent( pszPath, oSHA1, oED2K, pszSources );
	m_pList.AddHead( pRecent );
	
	Prune();
	
	return pRecent;
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory submit a library file

BOOL CLibraryHistory::Submit(CLibraryFile* pFile)
{
	CLibraryRecent* pRecent = GetByPath( pFile->GetPath() );
	if ( pRecent == NULL ) return FALSE;
	
	pRecent->RunVerify( pFile );
	
	Prune();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory clear today flags

void CLibraryHistory::ClearTodays()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->m_bToday = FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory prune list to a fixed size

int CLibraryHistory::Prune()
{
	LONGLONG tNow, tRecent;
	SYSTEMTIME pNow;
	int nCount = 0;
	
	GetSystemTime( &pNow );
	SystemTimeToFileTime( &pNow, (FILETIME*)&tNow );
	
	for ( POSITION pos = m_pList.GetTailPosition() ; pos ; )
	{
		POSITION posCur = pos;
		CLibraryRecent* pRecent = (CLibraryRecent*)m_pList.GetPrev( pos );
		
		CopyMemory( &tRecent, &pRecent->m_tAdded, sizeof(LONGLONG) );
		
		if ( tNow - tRecent > (LONGLONG)Settings.Library.HistoryDays * 0xC92A69C000 )
		{
			delete pRecent;
			m_pList.RemoveAt( posCur );
			nCount++;
		}
	}
	
	while ( GetCount() > (int)Settings.Library.HistoryTotal )
	{
		delete (CLibraryRecent*)m_pList.RemoveTail();
		nCount++;
	}
	
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory file delete handler

void CLibraryHistory::OnFileDelete(CLibraryFile* pFile)
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posCur = pos;
		CLibraryRecent* pRecent = GetNext( pos );
		
		if ( pRecent->m_pFile == pFile )
		{
			delete pRecent;
			m_pList.RemoveAt( posCur );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory serialize

void CLibraryHistory::Serialize(CArchive& ar, int nVersion)
{
	if ( nVersion < 7 ) return;
	
	int nCount = 0;
	POSITION pos;
	
	if ( ar.IsStoring() )
	{
		for ( pos = GetIterator() ; pos ; )
		{
			if ( GetNext( pos )->m_pFile != NULL ) nCount ++;
		}
		
		ar.WriteCount( nCount );
		
		for ( pos = GetIterator() ; pos ; )
		{
			CLibraryRecent* pRecent = GetNext( pos );
			if ( pRecent->m_pFile != NULL ) pRecent->Serialize( ar, nVersion );
		}
	}
	else
	{
		Clear();
		
		for ( nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CLibraryRecent* pRecent = new CLibraryRecent();
			pRecent->Serialize( ar, nVersion );
			
			if ( pRecent->m_pFile != NULL )
			{
				m_pList.AddTail( pRecent );
			}
			else
			{
				delete pRecent;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////
// CLibraryRecent construction

CLibraryRecent::CLibraryRecent()
{
	ZeroMemory( &m_tAdded, sizeof(FILETIME) );

	m_bToday	= FALSE;
	m_pFile		= NULL;
	ASSERT( ! m_oSHA1.IsValid() );
	ASSERT( ! m_oED2K.IsValid() );
}

CLibraryRecent::CLibraryRecent(LPCTSTR pszPath, const CManagedSHA1 &oSHA1, const CManagedED2K &oED2K, LPCTSTR pszSources)
{
	SYSTEMTIME pTime;
	GetSystemTime( &pTime );
	SystemTimeToFileTime( &pTime, &m_tAdded );
	
	m_pFile		= NULL;
	m_sPath		= pszPath;
	m_sSources	= pszSources;
	
	m_oSHA1 = oSHA1;
	m_oED2K = oED2K;
}

CLibraryRecent::~CLibraryRecent()
{
}

//////////////////////////////////////////////////////////////////////
// CLibraryRecent verification

void CLibraryRecent::RunVerify(CLibraryFile* pFile)
{
	if ( m_pFile == NULL )
	{
		m_pFile = pFile;
		m_pFile->OnVerifyDownload( m_oSHA1, m_oED2K, m_sSources );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryRecent serialize

void CLibraryRecent::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ASSERT( m_pFile != NULL );
		
		ar.Write( &m_tAdded, sizeof(FILETIME) );
		ar << m_pFile->m_nIndex;
	}
	else
	{
		DWORD nIndex;
		
		ar.Read( &m_tAdded, sizeof(FILETIME) );
		ar >> nIndex;
		
		if ( m_pFile = Library.LookupFile( nIndex ) )
		{
			m_sPath = m_pFile->GetPath();
			m_oSHA1 = m_pFile->m_oSHA1;
			m_oED2K = m_pFile->m_oED2K;
		}
	}
}

