//
// DownloadGroup.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
#include "DownloadGroup.h"
#include "DownloadGroups.h"
#include "Downloads.h"
#include "Download.h"

#include "Settings.h"
#include "SchemaCache.h"
#include "ShellIcons.h"
#include "QuerySearch.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadGroup construction

CDownloadGroup::CDownloadGroup(const LPCTSTR szName, const BOOL bTemporary) :
	m_sName				( szName ? szName : _T("") ),
	m_nImage			( SHI_FOLDER_OPEN ),
	m_bRemoteSelected	( TRUE ),
	m_bTemporary		( bTemporary ? TRI_FALSE : TRI_UNKNOWN ),
	m_bTorrent			( FALSE )
{
}

CDownloadGroup::~CDownloadGroup()
{
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup add and remove

void CDownloadGroup::Add(CDownload* pDownload)
{
	if ( m_pDownloads.Find( pDownload ) == NULL )
	{
		m_pDownloads.AddTail( pDownload );
		DownloadGroups.IncBaseCookie();
	}
}

void CDownloadGroup::Remove(CDownload* pDownload)
{
	if ( POSITION pos = m_pDownloads.Find( pDownload ) )
	{
		m_pDownloads.RemoveAt( pos );
		DownloadGroups.IncBaseCookie();
	}
}

void CDownloadGroup::Clear()
{
	m_pDownloads.RemoveAll();
	DownloadGroups.IncBaseCookie();
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup set selection cookie

void CDownloadGroup::SetCookie(int nCookie)
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->m_nGroupCookie = nCookie;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup list copy

void CDownloadGroup::CopyList(CList< CDownload* >& pList)
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		pList.AddTail( GetNext( pos ) );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup conditional add

BOOL CDownloadGroup::Link(CDownload* pDownload)
{
	// Filter by BitTorrent flag
	if ( m_bTorrent && pDownload->IsTorrent() )
	{
		Add( pDownload );
		return TRUE;
	}

	if ( m_pFilters.IsEmpty() )
		return FALSE;

	for ( POSITION pos = m_pFilters.GetHeadPosition() ; pos ; )
	{
		CString strFilter = m_pFilters.GetNext( pos );
		
		if ( strFilter.GetAt( 0 ) == _T('.') )
		{
			// Filter by extension
			int nPos( pDownload->m_sName.ReverseFind( _T('.') ) );
			if ( nPos != -1 && ! strFilter.CompareNoCase( pDownload->m_sName.Mid( nPos ) ) )
			{
				Add( pDownload );
				return TRUE;
			}
		}
		else
		{
			// Filter by keywords
			if ( CQuerySearch::WordMatch( pDownload->m_sName, strFilter ) )
			{
				Add( pDownload );
				return TRUE;
			}
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup conditional add all downloads

int CDownloadGroup::LinkAll()
{
	if ( m_pFilters.IsEmpty() ) return 0;

	int nCount = 0;

	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		nCount += Link( Downloads.GetNext( pos ) );
	}

	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup add/remove a filter

void CDownloadGroup::AddFilter(const CString& strFilter)
{
	if ( ! strFilter.IsEmpty () )
	{
		if ( m_pFilters.Find( strFilter ) == NULL )
			m_pFilters.AddTail( strFilter );
	}
}

void CDownloadGroup::RemoveFilter(const CString& strFilter)
{
	if ( ! strFilter.IsEmpty () )
	{
		while ( POSITION pos = m_pFilters.Find( strFilter ) )
			m_pFilters.RemoveAt( pos );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup schema

void CDownloadGroup::SetSchema(LPCTSTR pszURI, BOOL bRemoveOldFilters)
{
	if ( m_sSchemaURI != pszURI )
	{
		// Remove auto filters only
		if ( bRemoveOldFilters && ! m_pFilters.IsEmpty() )
		{
			if ( CSchema* pOldSchema = SchemaCache.Get( m_sSchemaURI ) )
			{
				for ( LPCTSTR start = pOldSchema->m_sTypeFilter; *start; start++ )
				{
					LPCTSTR c = _tcschr( start, _T('|') );
					int len = c ? (int) ( c - start ) : (int) _tcslen( start );
					if ( len > 0 )
					{
						RemoveFilter( CString( start, len ) );
					}
					if ( ! c )
						break;
					start = c;
				}
			}
		}

		m_sSchemaURI = pszURI;
	}

	if ( CSchema* pSchema = SchemaCache.Get( m_sSchemaURI ) )
	{
		m_nImage = pSchema->m_nIcon16;
		if ( pSchema->m_sHeaderTitle.IsEmpty() )
			m_sName = pSchema->m_sTitle;
		else
			m_sName = pSchema->m_sHeaderTitle;
	}
	else
	{
		m_nImage = SHI_FOLDER_OPEN;
	}
}

void CDownloadGroup::SetFolder(LPCTSTR pszFolder)
{
	m_sFolder = pszFolder;
}

void CDownloadGroup::SetDefaultFilters()
{
	if ( CSchema* pSchema = SchemaCache.Get( m_sSchemaURI ) )
	{
		for ( LPCTSTR start = pSchema->m_sTypeFilter; *start; start++ )
		{
			LPCTSTR c = _tcschr( start, _T('|') );
			int len = c ? (int) ( c - start ) : (int) _tcslen( start );
			if ( len > 0 )
			{
				AddFilter( CString( start, len ) );
			}
			if ( ! c )
				break;
			start = c;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup serialize

void CDownloadGroup::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_sName;
		ar << m_sSchemaURI;
		ar << m_sFolder;

		ar.WriteCount( m_pFilters.GetCount() );

		for ( POSITION pos = m_pFilters.GetHeadPosition() ; pos ; )
		{
			ar << m_pFilters.GetNext( pos );
		}

		ar.WriteCount( GetCount() );

		for ( POSITION pos = GetIterator() ; pos ; )
		{
			DWORD nDownload = GetNext( pos )->m_nSerID;
			ar << nDownload;
		}

		ASSERT( m_bTemporary == TRI_UNKNOWN || m_bTemporary == TRI_FALSE );
		ar << m_bTemporary;

		ar << m_bTorrent;
	}
	else
	{
		ar >> m_sName;
		ar >> m_sSchemaURI;
		ar >> m_sFolder;

		if ( nVersion >= 3 )
		{
			for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
			{
				CString strFilter;
				ar >> strFilter;
				AddFilter( strFilter );
			}
		}
		else
		{
			CString strFilters;
			ar >> strFilters;

			for ( strFilters += '|' ; strFilters.GetLength() ; )
			{
				CString strFilter = strFilters.SpanExcluding( _T(" |") );
				strFilters = strFilters.Mid( strFilter.GetLength() + 1 );
				strFilter.TrimLeft(); strFilter.TrimRight();
				if ( strFilter.GetLength() ) AddFilter( strFilter );
			}
		}

		for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			DWORD nDownload;
			ar >> nDownload;
			if ( CDownload* pDownload = Downloads.FindBySID( nDownload ) )
				Add( pDownload );
		}

		if ( nVersion >= 4 )
		{
			ar >> m_bTemporary;
			ASSERT( m_bTemporary == TRI_UNKNOWN || m_bTemporary == TRI_FALSE );
		}

		if ( nVersion >= 7 )
		{
			ar >> m_bTorrent;
		}

		// Fix collection schema (nVersion < 7)
		if ( CheckURI( m_sSchemaURI, CSchema::uriCollectionsFolder ) )
		{
			m_sSchemaURI = CSchema::uriCollection;
			SetDefaultFilters();
		}

		// Restore default folder for Collections
		if ( CheckURI( m_sSchemaURI, CSchema::uriCollection ) )
		{
			if ( m_sFolder.IsEmpty() || ! PathIsDirectory( m_sFolder ) )
			{
				m_sFolder = Settings.Downloads.CollectionPath;
			}
		}

		// Restore default folder for Torrents
		if ( CheckURI( m_sSchemaURI, CSchema::uriBitTorrent ) )
		{
			if ( m_sFolder.IsEmpty() || ! PathIsDirectory( m_sFolder ) )
			{
				m_sFolder = Settings.Downloads.TorrentPath;
			}
		}

		SetSchema( m_sSchemaURI );
	}
}

BOOL CDownloadGroup::IsTemporary()
{
	if ( m_bTemporary == TRI_FALSE )
	{
		BOOL bAllCompleted = TRUE;
		for ( POSITION pos = GetIterator() ; bAllCompleted && pos ; )
		{
			CDownload* pDownload = GetNext( pos );
			if ( Downloads.Check( pDownload ) && ! pDownload->IsCompleted() )
				bAllCompleted = FALSE;
		}
		if ( bAllCompleted )
		{
			m_bTemporary = TRI_TRUE;
		}
	}
	return ( m_bTemporary == TRI_TRUE );
}
