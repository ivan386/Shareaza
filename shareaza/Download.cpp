//
// Download.cpp
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
#include "Settings.h"
#include "Download.h"
#include "Downloads.h"
#include "DownloadTask.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadGroups.h"
#include "Uploads.h"
#include "SharedFile.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "LibraryHistory.h"
#include "FragmentedFile.h"
#include "BTTrackerRequest.h"
#include "XML.h"
#include "Network.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownload construction

CDownload::CDownload() :
	m_nSerID		( Downloads.GetFreeSID() )
,	m_bExpanded		( Settings.Downloads.AutoExpand )
,	m_bSelected		( FALSE )
,	m_bVerify		( TRI_UNKNOWN )
,	m_nRunCookie	( 0 )
,	m_nSaveCookie	( 0 )
,	m_nGroupCookie	( 0 )

,	m_bPaused		( FALSE )
,	m_bBoosted		( FALSE )
,	m_bShared		( Settings.Uploads.SharePartials )
,	m_bComplete		( FALSE )
,	m_tCompleted	( 0 )
,	m_tSaved		( 0 )
,	m_tBegan		( 0 )
,	m_bDownloading	( FALSE )
,	m_bTempPaused	( FALSE )
{
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
		Uploads.OnRename( m_sPath, NULL );
		if ( m_bSeeding )
		{
			// Auto-clear activated or we don't want to seed
			if ( Settings.BitTorrent.AutoClear && 
				 Settings.BitTorrent.ClearRatio <= GetRatio() ||
				 !Settings.BitTorrent.AutoClear && 
				 !Settings.BitTorrent.AutoSeed )
			{
				if ( ! ::DeleteFile( m_sPath ) )
					theApp.WriteProfileString( L"Delete", m_sPath, L"" );
				if ( ! ::DeleteFile( m_sPath + ".sd" ) )
					theApp.WriteProfileString( L"Delete", m_sPath + L".sd", L"" );
			}
		}
		else if ( ! ::DeleteFile( m_sPath ) )
			theApp.WriteProfileString( L"Delete", m_sPath, L"" );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownload control : pause

void CDownload::Pause( BOOL bRealPause )
{
	if ( m_bComplete || m_bPaused ) return;

	theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_PAUSED, (LPCTSTR)GetDisplayName() );
	if ( !bRealPause ) 
	{
		StopTrying();
		m_bTempPaused = TRUE;
		return;
	}
	else
	{
		StopTrying();
		m_bTempPaused = TRUE;
		m_bPaused = TRUE;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownload control : resume

void CDownload::Resume()
{
	if ( m_bComplete ) return;
	if ( !Network.IsConnected() && !Network.Connect( TRUE ) ) return;
	if ( ! m_bTempPaused ) 
	{
		if ( ( m_tBegan == 0 ) && ( GetEffectiveSourceCount() < Settings.Downloads.MinSources ) ) 
			FindMoreSources();
		SetStartTimer();
		return;
	}
	
	theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_RESUMED, (LPCTSTR)GetDisplayName() );
	
	if ( m_pFile != NULL )
	{
		for ( CDownloadSource* pSource = GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
		{
			pSource->OnResume();
		}
	}
	
	m_bPaused				= FALSE;
	m_bTempPaused			= FALSE;
	m_bDiskFull				= FALSE;
	m_tReceived				= GetTickCount();
	m_bTorrentTrackerError	= FALSE;

	if ( IsTorrent() )
	{
		if ( Downloads.GetTryingCount( TRUE ) < Settings.BitTorrent.DownloadTorrents ) 
			SetStartTimer();
	}
	else
	{
		if ( Downloads.GetTryingCount( FALSE ) < ( Settings.Downloads.MaxFiles + Settings.Downloads.MaxFileSearches ) )
			SetStartTimer();
	}

	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownload control : remove

void CDownload::Remove(BOOL bDelete)
{
	CloseTorrent();
	CloseTransfers();
	CloseFile();

	if ( m_pTask != NULL )
	{
		m_pTask->Abort();
		ASSERT( m_pTask == NULL );
	}
	
	if ( bDelete || ! IsCompleted() )
	{
		theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_REMOVE, (LPCTSTR)GetDisplayName() );
	}
	
	DeleteFile( bDelete );
	DeletePreviews();
	
	if ( m_bSeeding )
	{
		::DeleteFile( Settings.Downloads.IncompletePath + L"\\" + m_sSafeName + L".sd" );
		int nBackSlash = m_sPath.ReverseFind( '\\' );
		CString strTempFileName = m_sPath.Mid( nBackSlash + 1 );
		if ( m_oBTH.toString< Hashes::base16Encoding >() == strTempFileName )
			::DeleteFile( m_sPath );
	}
	else
		::DeleteFile( m_sPath + _T(".sd") );
	::DeleteFile( m_sPath + _T(".png") );
	
	Downloads.Remove( this );
}

//////////////////////////////////////////////////////////////////////
// CDownload control : boost

void CDownload::Boost()
{
	if ( m_pFile == NULL || m_bBoosted ) return;
	
	theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_BOOST, (LPCTSTR)GetDisplayName() );
	
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
	// Don't bother if renaming to same name.
	if ( m_sName == pszName ) return FALSE;

	// Set new name
	m_sName = pszName;

	// Set the new safe name. (Can be used for previews, etc)
	m_sSafeName = CDownloadTask::SafeFilename( m_sName.Right( 64 ) );

	SetModified();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownload control : Stop trying

void CDownload::StopTrying()
{
	if ( m_bComplete ) return;
	m_tBegan = 0;

	m_bDownloading	= FALSE;

	// if m_bTorrentRequested = TRUE, raza sends Stop
	// CloseTorrent() additionally closes uploads
	if ( IsTorrent() ) CloseTorrent();

	CloseTransfers();
	CloseFile();
	StopSearch();
	SetModified();
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

BOOL CDownload::IsPaused( BOOL bRealState ) const
{
	return ( bRealState ? m_bPaused : m_bTempPaused );
}

BOOL CDownload::IsDownloading() const
{
	return m_bDownloading;
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
	return !IsPaused(TRUE) ? m_bShared || ( IsTorrent() && ( IsSeeding() || IsStarted() ) ) || ( Settings.eDonkey.EnableToday && m_oED2K ) : m_bShared;
}

//////////////////////////////////////////////////////////////////////
// CDownload run handler

void CDownload::OnRun()
{
	DWORD tNow = GetTickCount();
	BOOL bDownloading = FALSE;

	if ( ! m_bTempPaused )
	{
		if ( m_bDiskFull  ) Pause( FALSE );
		else if ( IsTrying() || IsSeeding() )
		{	//This download is trying to download

			//'Dead download' check- if download appears dead, give up and allow another to start.
			if ( ( ! m_bComplete ) && ( tNow - GetStartTimer() ) > ( 3 * 60 * 60 * 1000 )  )	
			{	//If it's not complete, and we've been trying for at least 3 hours

				DWORD tHoursToTry = min( ( GetEffectiveSourceCount() + 49u ) / 50u, 9lu ) + Settings.Downloads.StarveGiveUp;

				if (  ( tNow - m_tReceived ) > ( tHoursToTry * 60 * 60 * 1000 ) )
				{	//And have had no new data for 5-14 hours	

					if ( IsTorrent() )	//If it's a torrent
					{
						if ( Downloads.GetTryingCount( TRUE ) >= Settings.BitTorrent.DownloadTorrents )
						{	//If there are other torrents that could start
							StopTrying();		//Give up for now, try again later
							return;
						}
					}
					else			//It's a regular download
					{
						if ( Downloads.GetTryingCount( FALSE ) >= ( Settings.Downloads.MaxFiles + Settings.Downloads.MaxFileSearches ) )
						{	//If there are other downloads that could try
							StopTrying();		//Give up for now, try again later
							return;
						}
					}
				}
			}	//End of 'dead download' check

			// Run the download
			if ( RunTorrent( tNow ) )
			{
				RunSearch( tNow );
				
				if ( m_bSeeding )
				{
					// Mark as collapsed to get correct heights when dragging files
					if ( !Settings.General.DebugBTSources && m_bExpanded )
						m_bExpanded = FALSE;

					RunValidation( TRUE );
					if ( Settings.BitTorrent.AutoSeed )
					{
						if ( m_tBegan == 0 )
						{
							if ( !Network.IsConnected() )
								Network.Connect( TRUE );

							m_tBegan = GetTickCount();
						}
					}
					SetModified();
				}
				else if ( m_pFile != NULL )
				{
					RunValidation( FALSE );
					
					if ( RunFile( tNow ) )
					{
						if ( ValidationCanFinish() ) OnDownloaded();
					}
					else if ( CheckTorrentRatio() )
					{
						if ( Network.IsConnected() )
							StartTransfersIfNeeded( tNow );
						else
							m_tBegan = 0;
					}
				}
				else if ( m_pFile == NULL && ! m_bComplete && m_pTask == NULL )
				{
					OnDownloaded();
				}
			}

			// Calculate the currently downloading state
			if( HasActiveTransfers() ) 
				bDownloading = TRUE;
		}
		else if ( ! m_bComplete && m_bVerify != TRI_TRUE )
		{	//If this download isn't trying to download, see if it can try
			if ( IsDownloading() )
			{	// This download was probably started by a push/etc
				SetStartTimer();
			}
			if ( Network.IsConnected() )
			{
				if ( IsTorrent() )
				{	//Torrents only try when 'ready to go'. (Reduce tracker load)
					if ( Downloads.GetTryingCount( TRUE ) < Settings.BitTorrent.DownloadTorrents )
						SetStartTimer();
				}
				else
				{	//We have extra regular downloads 'trying' so when a new slot is ready, a download
					//has sources and is ready to go.	
					if ( Downloads.GetTryingCount( FALSE ) < ( Settings.Downloads.MaxFiles + Settings.Downloads.MaxFileSearches ) )
						SetStartTimer();
				}
			}
			else
				m_tBegan = 0;
		}
	}
	
	// Set the currently downloading state (Used to optimize display in Ctrl/Wnd functions)
	m_bDownloading = bDownloading;
	
	// Don't save Downloads with many sources too often, since it's slow
	if ( tNow - m_tSaved >=
		( GetCount() > 20 ? 5 * Settings.Downloads.SaveInterval : Settings.Downloads.SaveInterval ) )
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
	
	theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_COMPLETED, (LPCTSTR)GetDisplayName() );
	m_tCompleted = GetTickCount();
	m_bDownloading = FALSE;
	
	CloseTransfers();
	
	if ( m_pFile != NULL )
	{
		m_pFile->Close();
		delete m_pFile;
		m_pFile = NULL;
		AppendMetadata();
	}
	
	if ( m_pTask && ( m_pTask->m_nTask == CDownloadTask::dtaskPreviewRequest ||
		m_pTask->m_nTask == CDownloadTask::dtaskMergeFile ) )
	{
		m_pTask->Abort();
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
	else if ( pTask->m_nTask == CDownloadTask::dtaskPreviewRequest )
	{
		OnPreviewRequestComplete( pTask );
	}
	else if ( pTask->m_nTask == CDownloadTask::dtaskMergeFile )
	{
		// Merge Complete.
	}
	else if ( pTask->m_nTask == CDownloadTask::dtaskCreateBatch )
	{
		MakeComplete();
		SetModified();
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
	CString strDiskFileName = m_sPath;
	// File is moved
	ASSERT( m_pFile == NULL );
	
	if ( pTask->m_bSuccess )
	{
		m_sPath = pTask->m_sFilename;
		
		theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_MOVED,
			(LPCTSTR)GetDisplayName(), (LPCTSTR)m_sPath );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CANT_MOVE,
			(LPCTSTR)GetDisplayName(), (LPCTSTR)pTask->m_sPath );
		
		if ( IsTorrent() )
		{
			m_bDiskFull = TRUE;
			return;
		}
	}
	
	// We just completed torrent
	if ( m_nTorrentBlock > 0 && m_nTorrentSuccess >= m_nTorrentBlock )
	{
		CloseTorrentUploads();
		SendCompleted();
		m_bSeeding = TRUE;
		m_tBegan = 0;
		m_bDownloading	= FALSE;
		m_bTorrentStarted = TRUE;
		m_bTorrentRequested = TRUE;
		CloseTransfers();
		StopSearch();
	}
	else if ( IsTorrent() ) // Something wrong (?), since we moved the torrent
	{
		// Explicitly set the flag to send stop
		m_bTorrentRequested = TRUE;
		StopTrying();
	}
	else
		StopTrying();

	// Download finalized, tracker notified, set flags that we completed
	m_bComplete		= TRUE;
	m_tCompleted	= GetTickCount();
	

	// Delete the SD file
	::DeleteFile( strDiskFileName + _T(".sd") );

	{
		CQuickLock oLibraryLock( Library.m_pSection );

		LibraryBuilder.RequestPriority( m_sPath );

		VERIFY( LibraryHistory.Add( m_sPath, m_oSHA1, m_oED2K, m_oBTH, m_oMD5,
			GetSourceURLs( NULL, 0, PROTOCOL_NULL, NULL ) ) );

		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( m_sPath ) )
			pFile->UpdateMetadata( this );
	}

	ClearSources();

	if ( IsFullyVerified() ) OnVerify( m_sPath, TRUE );
}

//////////////////////////////////////////////////////////////////////
// CDownload verification handler

BOOL CDownload::OnVerify(LPCTSTR pszPath, BOOL bVerified)
{
	if ( m_bVerify != TRI_UNKNOWN ) return FALSE;
	if ( m_pFile != NULL ) return FALSE;
	
	if ( pszPath != (LPCTSTR)m_sPath &&
		 m_sPath.CompareNoCase( pszPath ) != 0 ) return FALSE;
	
	m_bVerify = bVerified ? TRI_TRUE : TRI_FALSE;
	SetModified();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownload load and save

BOOL CDownload::Load(LPCTSTR pszName)
{
	BOOL bSuccess = FALSE;
	CFile pFile;
	
	m_sPath = pszName;
	m_sPath = m_sPath.Left( m_sPath.GetLength() - 3 );
	
	if ( pFile.Open( m_sPath + _T(".sd"), CFile::modeRead ) )
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
	
	if ( ! bSuccess && pFile.Open( m_sPath + _T(".sd.sav"), CFile::modeRead ) )
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
	
	if ( m_bSeeding )
		m_sPath = m_sServingFileName;

	m_bGotPreview = GetFileAttributes( m_sPath + L".png" ) != INVALID_FILE_ATTRIBUTES;
	m_nSaveCookie = m_nCookie;
	
	return bSuccess;
}

BOOL CDownload::Save(BOOL bFlush)
{
	CFile pFile;
	
	m_nSaveCookie = m_nCookie;
	m_tSaved = GetTickCount();
	
	if ( m_bComplete && !m_bSeeding ) return TRUE;
	if ( m_bSeeding && !Settings.BitTorrent.AutoSeed ) return TRUE;
	
	if ( m_bSeeding )
	{
		m_sSafeName.Empty();
		GenerateDiskName( true );
		// Swap disk name with the safe name, since the complete file may be located elsewhere
		// while .sd file remains in the incomplete folder for the single-file torrents.
		m_sServingFileName = m_sPath;
		m_sPath = Settings.Downloads.IncompletePath + _T("\\") + m_sSafeName;
	}
	else
	{
		if ( m_sPath.IsEmpty() )
			GenerateDiskName();
		if ( m_sSafeName.IsEmpty() )
			m_sSafeName = CDownloadTask::SafeFilename( m_sName.Right( 64 ) );
	}
	
	::DeleteFile( m_sPath + _T(".sd.sav") );
	
	if ( ! pFile.Open( m_sPath + _T(".sd.sav"),
		CFile::modeReadWrite|CFile::modeCreate|CFile::osWriteThrough ) ) return FALSE;
	
	{
		const int nBufferLength = 65536;

		auto_array< BYTE > pBuffer( new BYTE[ nBufferLength ] );
		CArchive ar( &pFile, CArchive::store, nBufferLength, pBuffer.get() );
		try
		{
			Serialize( ar, 0 );
			ar.Close();
		}
		catch ( CFileException* pException )
		{
			ar.Abort();
			pFile.Abort();
			theApp.Message( MSG_ERROR, _T("Serialize Error: %s"), pException->m_strFileName );
//			pException->ReportError(); // Currently causes deadlock in GUI with Transfers mutex
			pException->Delete();
			return FALSE;
		}
	}
	
	if ( Settings.Downloads.FlushSD || bFlush ) pFile.Flush();
	pFile.SeekToBegin();
	CHAR szID[3] = { 0, 0, 0 };
	pFile.Read( szID, 3 );
	pFile.Close();
	
	BOOL bResult = TRUE;
	if ( szID[0] == 'S' && szID[1] == 'D' && szID[2] == 'L' )
	{
		::DeleteFile( m_sPath + _T(".sd") );
		MoveFile( m_sPath + _T(".sd.sav"), m_sPath + _T(".sd") );
	}
	else
	{
		::DeleteFile( m_sPath + _T(".sd.sav") );
		bResult = FALSE;
	}

	if ( m_bSeeding )
	{
		m_sPath = m_sServingFileName;
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CDownload serialize

void CDownload::Serialize(CArchive& ar, int nVersion)
{
	ASSERT( ! m_bComplete || m_bSeeding );
	
	if ( !Settings.BitTorrent.AutoSeed && m_bSeeding )
		return;

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
			ReadArchive( ar, szID, 3 );
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
		m_bTempPaused = m_bPaused;
		ar >> m_bBoosted;
		if ( nVersion >= 14 ) ar >> m_bShared;
		if ( nVersion >= 26 ) ar >> m_nSerID;
		
		DownloadGroups.Link( this );

		if ( nVersion == 32 )
		{ // Compatibility for CB Branch.
			if ( ! ar.IsBufferEmpty() )
			{
				ar >> m_sSearchKeyword;
			}
		}
	}
}

void CDownload::SerializeOld(CArchive& ar, int nVersion)
{
	ASSERT( ar.IsLoading() );
	
	ar >> m_sPath;
	ar >> m_sName;
	
	DWORD nSize;
	ar >> nSize;
	m_nSize = nSize;
	
    Hashes::Sha1Hash oSHA1;
    SerializeIn( ar, oSHA1, nVersion );
    m_oSHA1 = oSHA1;
    m_bSHA1Trusted = true;
	
	ar >> m_bPaused;
	ar >> m_bExpanded;
	if ( nVersion >= 6 ) ar >> m_bBoosted;
	
	m_pFile->Serialize( ar, nVersion );
	GenerateDiskName();
	
	for ( DWORD_PTR nSources = ar.ReadCount() ; nSources ; nSources-- )
	{
		CDownloadSource* pSource = new CDownloadSource( this );
		pSource->Serialize( ar, nVersion );
		AddSourceInternal( pSource );
	}
	
	if ( nVersion >= 3 && ar.ReadCount() )
	{
		auto_ptr< CXMLElement > pXML( new CXMLElement() );
		pXML->Serialize( ar );
		MergeMetadata( pXML.get() );
	}
}
