//
// LibraryHistory.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "Download.h"
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
	LastSeededTorrent.m_sName.Empty();
	LastSeededTorrent.m_sPath.Empty();

	LastSeededTorrent.m_oBTH.clear();
	LastSeededTorrent.m_tLastSeeded		= 0;
	LastSeededTorrent.m_nUploaded		= 0;
	LastSeededTorrent.m_nDownloaded		= 0;

	LastCompletedTorrent.m_sName.Empty();
	LastCompletedTorrent.m_sPath.Empty();
	LastCompletedTorrent.m_oBTH.clear();
	LastCompletedTorrent.m_tLastSeeded	= 0;
	LastCompletedTorrent.m_nUploaded	= 0;
	LastCompletedTorrent.m_nDownloaded	= 0;
}

CLibraryHistory::~CLibraryHistory()
{
	ASSERT( m_pList.IsEmpty() );
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory file list

POSITION CLibraryHistory::GetIterator() const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pList.GetHeadPosition();
}

CLibraryRecent* CLibraryHistory::GetNext(POSITION& pos) const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pList.GetNext( pos );
}

INT_PTR CLibraryHistory::GetCount() const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pList.GetCount();
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory clear

void CLibraryHistory::Clear()
{
	ASSUME_LOCK( Library.m_pSection );

	for ( POSITION pos = GetIterator() ; pos ; )
		delete GetNext( pos );
	m_pList.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory check

BOOL CLibraryHistory::Check(CLibraryRecent* pRecent, int nScope) const
{
	ASSUME_LOCK( Library.m_pSection );

	if ( nScope == 0 )
		return ( m_pList.Find( pRecent ) != NULL );

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos && nScope > 0 ; )
	{
		CLibraryRecent* pExisting = m_pList.GetNext( pos );
		if ( pRecent == pExisting )
			return TRUE;
		if ( pExisting->m_pFile != NULL )
			nScope--;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory lookups

CLibraryRecent* CLibraryHistory::GetByPath(LPCTSTR pszPath) const
{
	ASSUME_LOCK( Library.m_pSection );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CLibraryRecent* pRecent = GetNext( pos );
		if ( pRecent->m_sPath.CompareNoCase( pszPath ) == 0 )
			return pRecent;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory add new download

void CLibraryHistory::Add(LPCTSTR pszPath, const CDownload* pDownload)
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	CLibraryRecent* pRecent = GetByPath( pszPath );
	if ( pRecent == NULL )
	{
		pRecent = new CLibraryRecent( pszPath, pDownload );
		if ( pRecent )
		{
			m_pList.AddHead( pRecent );
			Prune();
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory submit a library file

void CLibraryHistory::Submit(CLibraryFile* pFile)
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( CLibraryRecent* pRecent = GetByPath( pFile->GetPath() ) )
	{
		if ( pRecent->m_pFile == NULL )
		{
			pRecent->m_pFile = pFile;

			pFile->OnVerifyDownload( pRecent );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory prune list to a fixed size

void CLibraryHistory::Prune()
{
	ASSUME_LOCK( Library.m_pSection );

	FILETIME tNow;
	GetSystemTimeAsFileTime( &tNow );

	for ( POSITION pos = m_pList.GetTailPosition() ; pos ; )
	{
		POSITION posCur = pos;
		CLibraryRecent* pRecent = m_pList.GetPrev( pos );

		if ( ! pRecent->m_pFile )
			// Keep not verified files
			continue;

		DWORD nDays = (DWORD)( ( MAKEQWORD( tNow.dwLowDateTime, tNow.dwHighDateTime ) -
			 MAKEQWORD( pRecent->m_tAdded.dwLowDateTime, pRecent->m_tAdded.dwHighDateTime ) ) / 
			 ( 10000000ull * 60 * 60 * 24 ) );
		if ( nDays > Settings.Library.HistoryDays ||
			GetCount() > (int)Settings.Library.HistoryTotal )
		{
			delete pRecent;
			m_pList.RemoveAt( posCur );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryHistory file delete handler

void CLibraryHistory::OnFileDelete(CLibraryFile* pFile)
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

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
	ASSUME_LOCK( Library.m_pSection );

	if ( nVersion < 7 ) return;

	DWORD_PTR nCount = 0;
	POSITION pos;

	if ( ar.IsStoring() )
	{
		for ( pos = GetIterator() ; pos ; )
		{
			CLibraryRecent* pRecent = GetNext( pos );
			if ( pRecent->m_pFile != NULL ) nCount ++;
		}

		ar.WriteCount( nCount );

		for ( pos = GetIterator() ; pos ; )
		{
			CLibraryRecent* pRecent = GetNext( pos );
			if ( pRecent->m_pFile != NULL ) pRecent->Serialize( ar, nVersion );
		}

		ar << LastSeededTorrent.m_sPath;
		if ( LastSeededTorrent.m_sPath.GetLength() )
		{
			ar << LastSeededTorrent.m_sName;
			ar << LastSeededTorrent.m_tLastSeeded;
			Hashes::BtPureHash tmp( LastSeededTorrent.m_oBTH );
			SerializeOut( ar, tmp );
		}
	}
	else
	{
		Clear();

		for ( nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CAutoPtr< CLibraryRecent > pRecent( new CLibraryRecent() );
			pRecent->Serialize( ar, nVersion );
			if ( pRecent->m_pFile )
			{
				m_pList.AddTail( pRecent.Detach() );
			}
		}

		if ( nVersion > 22 )
		{
			ar >> LastSeededTorrent.m_sPath;
			if ( LastSeededTorrent.m_sPath.GetLength() )
			{
				ar >> LastSeededTorrent.m_sName;
				ar >> LastSeededTorrent.m_tLastSeeded;
				Hashes::BtPureHash tmp;
				SerializeIn( ar, tmp, nVersion );
				LastSeededTorrent.m_oBTH = tmp;
			}
		}

		Prune();
	}
}


//////////////////////////////////////////////////////////////////////
// CLibraryRecent construction

CLibraryRecent::CLibraryRecent()
	: m_pFile		( NULL )
{
	ZeroMemory( &m_tAdded, sizeof( m_tAdded ) );
}

CLibraryRecent::CLibraryRecent(LPCTSTR pszPath, const CDownload* pDownload)
	: m_pFile		( NULL )
	, m_sPath		( pszPath )
{
	if ( pDownload )
	{
		m_oSHA1 = pDownload->m_oSHA1;
		if ( pDownload->m_bSHA1Trusted ) m_oSHA1.signalTrusted();
		m_oTiger = pDownload->m_oTiger;
		if ( pDownload->m_bTigerTrusted ) m_oTiger.signalTrusted();
		m_oED2K = pDownload->m_oED2K;
		if ( pDownload->m_bED2KTrusted ) m_oED2K.signalTrusted();
		m_oBTH = pDownload->m_oBTH;
		if ( pDownload->m_bBTHTrusted ) m_oBTH.signalTrusted();
		m_oMD5 = pDownload->m_oMD5;
		if ( pDownload->m_bMD5Trusted ) m_oMD5.signalTrusted();
		m_sSources = pDownload->GetSourceURLs();
	}
	GetSystemTimeAsFileTime( &m_tAdded );	
}

//////////////////////////////////////////////////////////////////////
// CLibraryRecent serialize

void CLibraryRecent::Serialize(CArchive& ar, int /*nVersion*/)
{
	if ( ar.IsStoring() )
	{
		ar.Write( &m_tAdded, sizeof(FILETIME) );
		ar << m_pFile->m_nIndex;
	}
	else
	{
		DWORD nIndex;

		ReadArchive( ar, &m_tAdded, sizeof(FILETIME) );
		ar >> nIndex;

		if ( ( m_pFile = Library.LookupFile( nIndex ) ) != NULL )
		{
			m_sPath = m_pFile->GetPath();
			m_oSHA1 = m_pFile->m_oSHA1;
			m_oTiger = m_pFile->m_oTiger;
			m_oED2K = m_pFile->m_oED2K;
			m_oBTH = m_pFile->m_oBTH;
			m_oMD5 = m_pFile->m_oMD5;
		}
	}
}
