//
// DownloadGroups.cpp
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
#include "DownloadGroup.h"
#include "DownloadGroups.h"
#include "Downloads.h"
#include "Download.h"
#include "Schema.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDownloadGroups DownloadGroups;


//////////////////////////////////////////////////////////////////////
// CDownloadGroups construction

CDownloadGroups::CDownloadGroups()
{
	m_pSuper		= NULL;
	m_nBaseCookie	= 1;
	m_nSaveCookie	= 0;
	m_nGroupCookie	= 0;
}

CDownloadGroups::~CDownloadGroups()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroups supergroup

CDownloadGroup* CDownloadGroups::GetSuperGroup()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	if ( m_pSuper != NULL ) return m_pSuper;
	
	return m_pSuper = Add( _T("All") );
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroups add group

CDownloadGroup* CDownloadGroups::Add(LPCTSTR pszName)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	CDownloadGroup* pGroup = new CDownloadGroup();
	if ( pszName != NULL ) pGroup->m_sName = pszName;
	m_pList.AddTail( pGroup );
	
	m_nBaseCookie ++;
	m_nGroupCookie ++;
	
	return pGroup;
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroups remove group

void CDownloadGroups::Remove(CDownloadGroup* pGroup)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	if ( POSITION pos = m_pList.Find( pGroup ) )
	{
		if ( pGroup == m_pSuper ) return;
		m_pList.RemoveAt( pos );
		delete pGroup;
		
		m_nBaseCookie ++;
		m_nGroupCookie ++;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroups link a download to the appropriate groups

void CDownloadGroups::Link(CDownload* pDownload)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	GetSuperGroup()->Add( pDownload );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownloadGroup* pGroup = GetNext( pos );
		pGroup->Link( pDownload );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroups unlink a download from all groups

void CDownloadGroups::Unlink(CDownload* pDownload, BOOL bAndSuper)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownloadGroup* pGroup = GetNext( pos );
		if ( bAndSuper || pGroup != m_pSuper ) pGroup->Remove( pDownload );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroups default

void CDownloadGroups::CreateDefault()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	CDownloadGroup* pGroup	= GetSuperGroup();
	
	pGroup = Add( _T("Audio") );
	pGroup->AddFilter( _T(".mp3") );
	pGroup->AddFilter( _T(".ogg") );
	pGroup->AddFilter( _T(".wav") );
	pGroup->AddFilter( _T(".wma") );
	pGroup->SetSchema( CSchema::uriMusicAlbum );
	
	pGroup = Add( _T("Video") );
	pGroup->AddFilter( _T(".asf") );
	pGroup->AddFilter( _T(".avi") );
	pGroup->AddFilter( _T(".mov") );
	pGroup->AddFilter( _T(".mpg") );
	pGroup->AddFilter( _T(".mpeg") );
	pGroup->AddFilter( _T(".ogm") );
	pGroup->AddFilter( _T(".wmv") );
	pGroup->SetSchema( CSchema::uriVideo );
	
	pGroup = Add( _T("BitTorrent") );
	pGroup->AddFilter( _T("torrent") );
	pGroup->SetSchema( CSchema::uriROM );
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroups completed path

CString CDownloadGroups::GetCompletedPath(CDownload* pDownload)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownloadGroup* pGroup = GetNext( pos );
		
		if ( pGroup != m_pSuper && pGroup->Contains( pDownload ) )
		{
			if ( pGroup->m_sFolder.GetLength() ) return pGroup->m_sFolder;
		}
	}
	
	return Settings.Downloads.CompletePath;
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroups clear

void CDownloadGroups::Clear()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	for ( POSITION pos = GetIterator() ; pos ; ) delete GetNext( pos );
	m_pList.RemoveAll();
	
	m_pSuper = NULL;
	m_nBaseCookie ++;
	m_nGroupCookie ++;
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroups load and save

BOOL CDownloadGroups::Load()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	CFile pFile;
	CString strPath = Settings.General.Path + _T("\\Data\\DownloadGroups.dat");
	if ( ! pFile.Open( strPath, CFile::modeRead ) ) return FALSE;
	
	try
	{
		CArchive ar( &pFile, CArchive::load );
		Serialize( ar );
	}
	catch ( CException* pException )
	{
		pException->Delete();
		return FALSE;
	}
	
	m_nSaveCookie = m_nBaseCookie;
	
	return TRUE;
}

BOOL CDownloadGroups::Save(BOOL bForce)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	if ( ! bForce && m_nBaseCookie == m_nSaveCookie ) return FALSE;
	m_nSaveCookie = m_nBaseCookie;
	
	CString strPath = Settings.General.Path + _T("\\Data\\DownloadGroups.dat");
	DeleteFile( strPath + _T(".tmp") );
	
	CFile pFile;
	if ( ! pFile.Open( strPath + _T(".tmp"), CFile::modeWrite | CFile::modeCreate ) ) return FALSE;
	
	BYTE* pBuffer = new BYTE[ 4096 ];
	
	try
	{
		CArchive ar( &pFile, CArchive::store, 4096, pBuffer );
		Serialize( ar );
	}
	catch ( CException* pException )
	{
		delete [] pBuffer;
		pException->Delete();
		return FALSE;
	}
	
	delete [] pBuffer;
	
	pFile.Close();
	
	DeleteFile( strPath );
	MoveFile( strPath + _T(".tmp"), strPath );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadGroups serialize

#define GROUPS_SER_VERSION	3

void CDownloadGroups::Serialize(CArchive& ar)
{
	int nVersion = GROUPS_SER_VERSION;
	BYTE nState;
	
	if ( ar.IsStoring() )
	{
		ar << nVersion;
		
		ar.WriteCount( Downloads.GetCount() );
		
		for ( POSITION pos = Downloads.GetIterator() ; pos ; )
		{
			ar << Downloads.GetNext( pos )->m_nSerID;
		}
		
		ar.WriteCount( GetCount() );
		
		for ( pos = GetIterator() ; pos ; )
		{
			CDownloadGroup* pGroup = GetNext( pos );
			
			nState = ( pGroup == m_pSuper ) ? 1 : 0;
			ar << nState;
			
			pGroup->Serialize( ar, nVersion );
		}
	}
	else
	{
		ar >> nVersion;
		if ( nVersion <= 1 || nVersion > GROUPS_SER_VERSION ) AfxThrowUserException();
		
		int nCount = ar.ReadCount();
		
		for ( ; nCount > 0 ; nCount-- )
		{
			DWORD nDownload;
			ar >> nDownload;
			if ( CDownload* pDownload = Downloads.FindBySID( nDownload ) )
				Downloads.Reorder( pDownload, NULL );
		}
		
		if ( nCount = ar.ReadCount() ) Clear();
		
		for ( ; nCount > 0 ; nCount-- )
		{
			CDownloadGroup* pGroup = Add();
			
			ar >> nState;
			if ( nState == 1 ) m_pSuper = pGroup;
			
			pGroup->Serialize( ar, nVersion );
		}
		
		GetSuperGroup();
		
		for ( POSITION pos = Downloads.GetIterator() ; pos ; )
		{
			m_pSuper->Add( Downloads.GetNext( pos ) );
		}
	}
}
