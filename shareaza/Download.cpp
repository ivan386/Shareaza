//
// Download.cpp
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
#include "Download.h"
#include "Downloads.h"
#include "DownloadTask.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadGroups.h"
#include "Uploads.h"

#include "Library.h"
#include "LibraryBuilder.h"
#include "LibraryHistory.h"
#include "FragmentedFile.h"
#include "BTTrackerRequest.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownload construction

CDownload::CDownload()
{
	m_nSerID		= Downloads.GetFreeSID();
	m_bExpanded		= Settings.Downloads.AutoExpand;
	m_bSelected		= FALSE;
	m_bVerify		= TS_UNKNOWN;
	
	m_nRunCookie	= 0;
	m_nSaveCookie	= 0;
	m_nGroupCookie	= 0;
	
	m_bPaused		= FALSE;
	m_bBoosted		= FALSE;
	m_bShared		= Settings.Uploads.SharePartials;
	m_bComplete		= FALSE;
	m_tCompleted	= 0;
	m_tSaved		= 0;
	m_tBegan		= 0;
	
	DownloadGroups.Link( this );
}

CDownload::~CDownload()
{
	if ( m_pTask != NULL ) m_pTask->Abort();
	DownloadGroups.Unlink( this );
	
	if ( m_pTorrent.m_nFiles > 1 && m_bComplete )
	{
		CloseTransfers();
		CloseTorrentUploads();
		Uploads.OnRename( m_sLocalName, NULL );
		if ( ! ::DeleteFile( m_sLocalName ) )
			theApp.WriteProfileString( _T("Delete"), m_sLocalName, _T("") );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownload control : pause

void CDownload::Pause()
{
	if ( m_bComplete || m_bPaused ) return;
	m_bPaused = TRUE;
	
	theApp.Message( MSG_DOWNLOAD, IDS_DOWNLOAD_PAUSED, (LPCTSTR)GetDisplayName() );
	
	m_tBegan	= 0;
	CloseTransfers();
	CloseFile();
	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownload control : resume

void CDownload::Resume()
{
	if ( m_bComplete || ! m_bPaused ) return;
	
	theApp.Message( MSG_DOWNLOAD, IDS_DOWNLOAD_RESUMED, (LPCTSTR)GetDisplayName() );
	
	if ( m_pFile != NULL )
	{
		for ( CDownloadSource* pSource = GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
		{
			pSource->OnResume();
		}
	}
	
	m_bPaused	= FALSE;
	m_bDiskFull	= FALSE;
	m_tReceived	= GetTickCount();
	m_tBegan	= GetTickCount();
	m_bTorrentTrackerError = FALSE;
	
	SetModified();
	CloseTorrentUploads();
}

//////////////////////////////////////////////////////////////////////
// CDownload control : remove

void CDownload::Remove(BOOL bDelete)
{
	CloseTorrentUploads();
	CloseTransfers();
	CloseFile();
	
	if ( m_pTask != NULL )
	{
		m_pTask->Abort();
		ASSERT( m_pTask == NULL );
	}
	
	if ( bDelete || ! IsCompleted() )
	{
		theApp.Message( MSG_DOWNLOAD, IDS_DOWNLOAD_REMOVE, (LPCTSTR)GetDisplayName() );
	}
	
	DeleteFile( bDelete );
	DeletePreviews();
	
	::DeleteFile( m_sLocalName + _T(".sd") );
	
	Downloads.Remove( this );
}

//////////////////////////////////////////////////////////////////////
// CDownload control : boost

void CDownload::Boost()
{
	if ( m_pFile == NULL || m_bBoosted ) return;
	
	theApp.Message( MSG_SYSTEM, IDS_DOWNLOAD_BOOST, (LPCTSTR)GetDisplayName() );
	
	for ( CDownloadTransfer* pTransfer = GetFirstTransfer() ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
	{
		pTransfer->Boost();
	}
	
	m_bBoosted = TRUE;
	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownload control : sharing

void CDownload::Share(BOOL bShared)
{
	m_bShared = bShared;
	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownload control : rename

BOOL CDownload::Rename(LPCTSTR pszName)
{
	if ( m_sRemoteName == pszName ) return FALSE;
	m_sRemoteName = pszName;
	SetModified();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownload control : SetStartTimer

void CDownload::SetStartTimer()
{
	m_tBegan = GetTickCount();
	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownload control : GetStartTimer

DWORD CDownload::GetStartTimer() const
{
	return( m_tBegan );
}

//////////////////////////////////////////////////////////////////////
// CDownload state checks

BOOL CDownload::IsStarted() const
{
	return ( GetVolumeComplete() > 0 );
}

BOOL CDownload::IsPaused() const
{
	return m_bPaused;
}

BOOL CDownload::IsDownloading() const
{
	return ( GetTransferCount() > 0 );
}

BOOL CDownload::IsMoving() const
{
	return ( m_pFile == NULL );
}

BOOL CDownload::IsCompleted() const
{
	return m_bComplete;
}

BOOL CDownload::IsBoosted() const
{
	return m_bBoosted;
}

BOOL CDownload::IsTrying() const
{
	return ( m_tBegan != 0 );
}

BOOL CDownload::IsShared() const
{
#ifdef _DEBUG
	return m_bShared;
#else
	return m_bShared || m_bBTH || Settings.eDonkey.EnableToday;
#endif
}

//////////////////////////////////////////////////////////////////////
// CDownload run handler

void CDownload::OnRun()
{
	DWORD tNow = GetTickCount();
	
	if ( m_bDiskFull && ! m_bPaused ) Pause();

	if( IsTrying() )
	{	//This download is trying to download

		//** 'Dead download' check- if it appears dead, give up and allow another to start.
		if ( (!IsCompleted()) && ( ( GetTickCount() - GetStartTimer() ) > ( 8 * 60 * 60 * 1000 ) ) )//If we've been searching for 6 hours
		{												
			if ( ( GetTickCount() - m_tReceived ) > ( 5 * 60 * 60 * 1000 ) )	//And had no new data for 5
			{											
				if( m_bBTH )	//If it's a torrent
				{
					if( Downloads.GetTryingCount( TRUE ) >= Settings.BitTorrent.DownloadTorrents )	//If we are at max torrents
					{
						m_tBegan = 0; //Give up on this one for now, try again later
						if ( m_bTorrentRequested ) CBTTrackerRequest::SendStopped( this );
						CloseTorrentUploads();
					}
				}
				else			//Regular download
				{
					m_tBegan = 0;	//Give up for now, try again later
				}
			}		
		} 
		//** End of 'dead download' check

		if ( RunTorrent( tNow ) )
		{
			RunSearch( tNow );
			
			if ( m_bPaused == FALSE )
			{
				if ( m_bSeeding )
				{
					RunValidation( TRUE );
				}
				else if ( m_pFile != NULL )
				{
					RunValidation( FALSE );
					
					if ( RunFile( tNow ) )
					{
						if ( ValidationCanFinish() ) OnDownloaded();
					}
					else
					{
						StartTransfersIfNeeded( tNow );
					}
				}
				else if ( m_pFile == NULL && ! m_bComplete && m_pTask == NULL )
				{
					OnDownloaded();
				}
			}
		}
	}
	else	
	{	//If this download isn't trying to download, see if it can try
		if(m_bBTH)
		{	//Torrents only try when 'ready to go'. (Reduce tracker load)
			if( Downloads.GetTryingCount( TRUE ) < Settings.BitTorrent.DownloadTorrents )
				SetStartTimer();
		}
		else
		{	//We have extra regular downloads 'trying' so when a new slot is ready, a download
			//has sources and is ready to go.	
			if( Downloads.GetTryingCount( FALSE ) < Settings.Downloads.MaxFiles + Settings.Downloads.MaxFileSearches )
				SetStartTimer();
		}
	}
	
	if ( tNow > m_tSaved && tNow - m_tSaved >= Settings.Downloads.SaveInterval )
	{
		if ( m_pFile != NULL && m_pFile->Flush() )
		{
			m_tSaved = tNow;
		}
		
		if ( m_nCookie != m_nSaveCookie )
		{
			Save();
			m_tSaved = tNow;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownload download complete handler

void CDownload::OnDownloaded()
{
	ASSERT( m_bComplete == FALSE );
	
	theApp.Message( MSG_DOWNLOAD, IDS_DOWNLOAD_COMPLETED, (LPCTSTR)GetDisplayName() );
	m_tCompleted = GetTickCount();
	
	CloseTransfers();
	
	if ( m_pFile != NULL )
	{
		m_pFile->Close();
		delete m_pFile;
		m_pFile = NULL;
		AppendMetadata();
	}
	
	ASSERT( m_pTask == NULL );
	m_pTask = new CDownloadTask( this, CDownloadTask::dtaskCopySimple );
	
	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownload task completion

void CDownload::OnTaskComplete(CDownloadTask* pTask)
{
	ASSERT( m_pTask == pTask );
	m_pTask = NULL;
	
	if ( pTask->WasAborted() ) return;
	
	if ( pTask->m_nTask == CDownloadTask::dtaskAllocate )
	{
		// allocate complete
	}
	else
	{
		OnMoved( pTask );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownload moved handler

void CDownload::OnMoved(CDownloadTask* pTask)
{
	CString strLocalName = m_sLocalName;
	ASSERT( m_pFile == NULL );
	
	if ( pTask->m_bSuccess )
	{
		m_sLocalName = pTask->m_sFilename;
		
		theApp.Message( MSG_DOWNLOAD, IDS_DOWNLOAD_MOVED,
			(LPCTSTR)GetDisplayName(), (LPCTSTR)m_sLocalName );
		
		if ( m_pXML != NULL && Settings.Downloads.Metadata )
			WriteMetadata( pTask->m_sPath );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CANT_MOVE,
			(LPCTSTR)GetDisplayName(), (LPCTSTR)pTask->m_sPath );
		
		if ( m_pTorrent.IsAvailable() && m_pTorrent.m_nFiles > 1 )
		{
			m_bDiskFull = TRUE;
			return;
		}
	}
	
	m_bComplete		= TRUE;
	m_tCompleted	= GetTickCount();
	
	::DeleteFile( strLocalName + _T(".sd") );
	
	if ( m_nTorrentBlock > 0 && m_nTorrentSuccess >= m_nTorrentBlock )
	{
		CBTTrackerRequest::SendCompleted( this );
	}
	
	LibraryBuilder.RequestPriority( m_sLocalName );
	
	if ( m_bSHA1 || m_bED2K )
	{
		LibraryHistory.Add( m_sLocalName, m_bSHA1 ? &m_pSHA1 : NULL,
			m_bED2K ? &m_pED2K : NULL, GetSourceURLs( NULL, 0, FALSE, NULL ) );
	}
	else
	{
		LibraryHistory.Add( m_sLocalName, NULL, NULL, NULL );
	}
	
	ClearSources();
	SetModified();
	
	if ( IsFullyVerified() ) OnVerify( m_sLocalName, TRUE );
}

//////////////////////////////////////////////////////////////////////
// CDownload verification handler

BOOL CDownload::OnVerify(LPCTSTR pszPath, BOOL bVerified)
{
	if ( m_bVerify != TS_UNKNOWN ) return FALSE;
	if ( m_pFile != NULL ) return FALSE;
	
	if ( pszPath != (LPCTSTR)m_sLocalName &&
		 m_sLocalName.CompareNoCase( pszPath ) != 0 ) return FALSE;
	
	m_bVerify = bVerified ? TS_TRUE : TS_FALSE;
	SetModified();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownload load and save

BOOL CDownload::Load(LPCTSTR pszName)
{
	BOOL bSuccess = FALSE;
	CFile pFile;
	
	m_sLocalName = pszName;
	m_sLocalName = m_sLocalName.Left( m_sLocalName.GetLength() - 3 );
	
	if ( pFile.Open( m_sLocalName + _T(".sd"), CFile::modeRead ) )
	{
		try
		{
			CArchive ar( &pFile, CArchive::load );
			Serialize( ar, 0 );
			bSuccess = TRUE;
		}
		catch ( CException* pException )
		{
			pException->Delete();
		}
		
		pFile.Close();
	}
	
	if ( ! bSuccess && pFile.Open( m_sLocalName + _T(".sd.sav"), CFile::modeRead ) )
	{
		try
		{
			CArchive ar( &pFile, CArchive::load );
			Serialize( ar, 0 );
			bSuccess = TRUE;
		}
		catch ( CException* pException )
		{
			pException->Delete();
		}
		
		pFile.Close();
		if ( bSuccess ) Save();
	}
	
	m_nSaveCookie = m_nCookie;
	
	return bSuccess;
}

BOOL CDownload::Save(BOOL bFlush)
{
	CFile pFile;
	
	m_nSaveCookie = m_nCookie;
	m_tSaved = GetTickCount();
	
	if ( m_bComplete ) return TRUE;
	
	GenerateLocalName();
	::DeleteFile( m_sLocalName + _T(".sd.sav") );
	
	if ( ! pFile.Open( m_sLocalName + _T(".sd.sav"),
		CFile::modeReadWrite|CFile::modeCreate|CFile::osWriteThrough ) ) return FALSE;
	
	BYTE* pBuffer = new BYTE[ 4096 ];
	{
		CArchive ar( &pFile, CArchive::store, 4096, pBuffer );
		Serialize( ar, 0 );
		ar.Close();
	}
	delete [] pBuffer;
	
	if ( Settings.Downloads.FlushSD || bFlush ) pFile.Flush();
	pFile.SeekToBegin();
	CHAR szID[3] = { 0, 0, 0 };
	pFile.Read( szID, 3 );
	pFile.Close();
	
	if ( szID[0] == 'S' && szID[1] == 'D' && szID[2] == 'L' )
	{
		::DeleteFile( m_sLocalName + _T(".sd") );
		MoveFile( m_sLocalName + _T(".sd.sav"), m_sLocalName + _T(".sd") );
		return TRUE;
	}
	else
	{
		::DeleteFile( m_sLocalName + _T(".sd.sav") );
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownload serialize

#define DOWNLOAD_SER_VERSION	30

void CDownload::Serialize(CArchive& ar, int nVersion)
{
	ASSERT( ! m_bComplete );
	
	if ( nVersion == 0 )
	{
		nVersion = DOWNLOAD_SER_VERSION;
		
		if ( ar.IsStoring() )
		{
			ar.Write( "SDL", 3 );
			ar << nVersion;
		}
		else
		{
			CHAR szID[3];
			ar.Read( szID, 3 );
			if ( strncmp( szID, "SDL", 3 ) ) AfxThrowUserException();
			ar >> nVersion;
			if ( nVersion <= 0 || nVersion > DOWNLOAD_SER_VERSION ) AfxThrowUserException();
		}
	}
	else if ( nVersion < 11 && ar.IsLoading() )
	{
		SerializeOld( ar, nVersion );
		return;
	}
	
	CDownloadWithExtras::Serialize( ar, nVersion );
	
	if ( ar.IsStoring() )
	{
		ar << m_bExpanded;
		ar << m_bPaused;
		ar << m_bBoosted;
		ar << m_bShared;
		
		ar << m_nSerID;
	}
	else
	{
		ar >> m_bExpanded;
		ar >> m_bPaused;
		ar >> m_bBoosted;
		if ( nVersion >= 14 ) ar >> m_bShared;
		if ( nVersion >= 26 ) ar >> m_nSerID;
		
		DownloadGroups.Link( this );
	}
}

void CDownload::SerializeOld(CArchive& ar, int nVersion)
{
	ASSERT( ar.IsLoading() );
	
	ar >> m_sLocalName;
	ar >> m_sRemoteName;
	
	DWORD nSize;
	ar >> nSize;
	m_nSize = nSize;
	
	ar >> m_bSHA1;
	if ( m_bSHA1 ) ar.Read( &m_pSHA1, sizeof(SHA1) );
	
	ar >> m_bPaused;
	ar >> m_bExpanded;
	if ( nVersion >= 6 ) ar >> m_bBoosted;
	
	m_pFile->Serialize( ar, nVersion );
	GenerateLocalName();
	
	for ( int nSources = ar.ReadCount() ; nSources ; nSources-- )
	{
		CDownloadSource* pSource = new CDownloadSource( this );
		pSource->Serialize( ar, nVersion );
		AddSourceInternal( pSource );
	}
	
	if ( nVersion >= 3 && ar.ReadCount() )
	{
		m_pXML = new CXMLElement();
		m_pXML->Serialize( ar );
	}
}
