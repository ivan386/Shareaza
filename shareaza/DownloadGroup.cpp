//
// DownloadGroup.cpp
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
#include "DownloadGroup.h"
#include "DownloadGroups.h"
#include "Downloads.h"
#include "Download.h"

#include "Schema.h"
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

CDownloadGroup::CDownloadGroup()
{
	m_nImage			= SHI_FOLDER_OPEN;
	m_bRemoteSelected	= TRUE;
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
		DownloadGroups.m_nBaseCookie ++;
	}
}

void CDownloadGroup::Remove(CDownload* pDownload)
{
	if ( POSITION pos = m_pDownloads.Find( pDownload ) )
	{
		m_pDownloads.RemoveAt( pos );
		DownloadGroups.m_nBaseCookie ++;
	}
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

void CDownloadGroup::CopyList(CPtrList* pList)
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		pList->AddTail( GetNext( pos ) );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup conditional add

BOOL CDownloadGroup::Link(CDownload* pDownload)
{
	if ( m_pFilters.IsEmpty() ) return FALSE;
	
	for ( POSITION pos = m_pFilters.GetHeadPosition() ; pos ; )
	{
		CString strFilter = m_pFilters.GetNext( pos );
		
		if ( ( pDownload->m_bBTH && strFilter.CompareNoCase( _T("torrent") ) == 0 ) ||
				CQuerySearch::WordMatch( pDownload->m_sRemoteName, strFilter ) )
		{
			Add( pDownload );
			return TRUE;
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
// CDownloadGroup add a filter

void CDownloadGroup::AddFilter(LPCTSTR pszFilter)
{
	m_pFilters.AddTail( pszFilter );
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup schema

void CDownloadGroup::SetSchema(LPCTSTR pszURI)
{
	if ( m_sSchemaURI != pszURI ) m_sSchemaURI = pszURI;
	
	if ( CSchema* pSchema = SchemaCache.Get( pszURI ) )
	{
		m_nImage = pSchema->m_nIcon16;
	}
	else
	{
		m_nImage = SHI_FOLDER_OPEN;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroup serialise

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
	}
	else
	{
		ar >> m_sName;
		ar >> m_sSchemaURI;
		ar >> m_sFolder;
		
		if ( nVersion >= 3 )
		{
			for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
			{
				CString strFilter;
				ar >> strFilter;
				m_pFilters.AddTail( strFilter );
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
				if ( strFilter.GetLength() ) m_pFilters.AddTail( strFilter );
			}
		}
		
		for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			DWORD nDownload;
			ar >> nDownload;
			if ( CDownload* pDownload = Downloads.FindBySID( nDownload ) )
				Add( pDownload );
		}
		
		SetSchema( m_sSchemaURI );
	}
}

