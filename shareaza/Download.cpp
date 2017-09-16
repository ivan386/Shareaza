//
// Download.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
#include "BTTrackerRequest.h"
#include "Download.h"
#include "DownloadGroups.h"
#include "DownloadSource.h"
#include "DownloadTask.h"
#include "DownloadTransfer.h"
#include "Downloads.h"
#include "FileExecutor.h"
#include "FragmentedFile.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "LibraryHistory.h"
#include "Network.h"
#include "Settings.h"
#include "SharedFile.h"
#include "Transfers.h"
#include "Uploads.h"
#include "XML.h"

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
,	m_tCompleted	( 0 )
,	m_nRunCookie	( 0 )
,	m_nGroupCookie	( 0 )

,	m_bTempPaused	( FALSE )
,	m_bPaused		( FALSE )
,	m_bBoosted		( FALSE )
,	m_bShared		( Settings.Uploads.SharePartials )
,	m_bComplete		( false )
,	m_tSaved		( 0 )
,	m_tBegan		( 0 )
,	m_bDownloading	( false )
#pragma warning(suppress:4355) // 'this' : used in base member initializer list
,	m_pTask			( this )
,	m_bStableName	( false )
{
}

CDownload::~CDownload()
{
	AbortTask();
	DownloadGroups.Unlink( this );
}

bool CDownload::HasStableName() const
{
	return ! m_sName.IsEmpty() && ( m_bStableName || HasHash() );
}

void CDownload::SetStableName(bool bStable)
{
	m_bStableName = bStable;
}

float CDownload::GetProgress() const
{
	return IsMoving() ? m_pTask.GetProgress() : CDownloadWithExtras::GetProgress();
}

//////////////////////////////////////////////////////////////////////
// CDownload check if a task is already running

bool CDownload::IsTasking() const
{
	return ( m_pTask.GetTaskType() != dtaskNone );
}

bool CDownload::IsMoving() const
{
	return ( m_pTask.GetTaskType() == dtaskCopy );
}

dtask CDownload::GetTaskType() const
{
	return m_pTask.GetTaskType();
}

void CDownload::AbortTask()
{
	m_pTask.Abort();
}

void CDownload::Allocate()
{
	m_pTask.Allocate();
}

void CDownload::Copy()
{
	m_pTask.Copy();
}

void CDownload::PreviewRequest(LPCTSTR szURL)
{
	m_pTask.PreviewRequest( szURL );
	m_bWaitingPreview = TRUE;
}

void CDownload::MergeFile(CList< CString >* pFiles, BOOL bValidation, const Fragments::List* pGaps)
{
	m_pTask.MergeFile( pFiles, bValidation, pGaps );
}

//////////////////////////////////////////////////////////////////////
// CDownload control : pause

void CDownload::Pause(BOOL bRealPause)
{
	if ( m_bComplete || m_bPaused )
		return;

	theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_PAUSED, (LPCTSTR)GetDisplayName() );

	m_bTempPaused = TRUE;

	if ( bRealPause )
		m_bPaused = TRUE;

	StopTrying();

	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownload control : resume

void CDownload::Resume()
{
	if ( IsCompleted() )
		return;

	if ( ! IsPaused() )
	{
		StartTrying();
		return;
	}

	theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_RESUMED, (LPCTSTR)GetDisplayName() );

	if ( IsFileOpen() )
	{
		for ( POSITION posSource = GetIterator(); posSource ; )
		{
			CDownloadSource* pSource = GetNext( posSource );

			pSource->OnResume();
		}
	}

	m_bPaused				= FALSE;
	m_bTempPaused			= FALSE;
	m_tReceived				= GetTickCount();
	m_bTorrentTrackerError	= FALSE;

	// Try again
	ClearFileError();

	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownload control : remove

void CDownload::Remove()
{
	StopTrying();
	CloseTorrent();
	CloseTransfers();
	AbortTask();

	theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_REMOVE, (LPCTSTR)GetDisplayName() );

	if ( IsCompleted() )
		CloseFile();
	else
		DeleteFile();

	DeletePreviews();

	if ( ! m_sPath.IsEmpty() )
	{
		DeleteFileEx( m_sPath + _T(".png"), FALSE, FALSE, TRUE );
		DeleteFileEx( m_sPath + _T(".sav"), FALSE, FALSE, TRUE );
		DeleteFileEx( m_sPath, FALSE, FALSE, TRUE );
		m_sPath.Empty();
	}

	Downloads.Remove( this );
}

//////////////////////////////////////////////////////////////////////
// CDownload control : boost

void CDownload::Boost()
{
	if ( ! IsFileOpen() || m_bBoosted ) return;

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
// CDownload control : Stop trying

void CDownload::StopTrying()
{
	if ( !IsTrying() || ( IsCompleted() && !IsSeeding() ) )
		return;

	m_tBegan = 0;
	m_bDownloading = false;

	// if m_bTorrentRequested = TRUE, raza sends Stop
	// CloseTorrent() additionally closes uploads
	if ( IsTorrent() )
		CloseTorrent();

	CloseTransfers();
	CloseFile();
	StopSearch();
	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownload control : StartTrying

void CDownload::StartTrying()
{
	ASSERT( !IsCompleted() || IsSeeding() );
	ASSERT( !IsPaused() );
	if ( IsTrying() || IsPaused() || ( IsCompleted() && !IsSeeding() ) )
		return;

	if ( !Network.IsConnected() && !Network.Connect( TRUE ) )
		return;

	m_tBegan = GetTickCount();
}

//////////////////////////////////////////////////////////////////////
// CDownload control : GetStartTimer

DWORD CDownload::GetStartTimer() const
{
	return m_tBegan ;
}

//////////////////////////////////////////////////////////////////////
// CDownload state checks

bool CDownload::IsStarted() const
{
	return GetVolumeComplete() > 0;
}

bool CDownload::IsPaused(bool bRealState /*= false*/) const
{
	if ( bRealState )
		return m_bPaused != 0;
	else
		return m_bTempPaused != 0;
}

// Is the download receiving data?
bool CDownload::IsDownloading() const
{
	return m_bDownloading;
}

bool CDownload::IsCompleted() const
{
	return m_bComplete;
}

bool CDownload::IsBoosted() const
{
	return ( m_bBoosted != 0 );
}

// Is the download currently trying to download?
bool CDownload::IsTrying() const
{
	return ( m_tBegan != 0 );
}

bool CDownload::IsShared() const
{
	return m_bShared ||
		( Settings.Gnutella1.EnableToday && m_oSHA1 ) ||
		( Settings.Gnutella2.EnableToday && HasHash() ) ||
		( Settings.eDonkey.EnableToday && m_oED2K ) ||
		( Settings.DC.EnableToday && m_oTiger ) ||
		( Settings.BitTorrent.EnableToday && IsTorrent() && ( IsSeeding() || IsStarted() ) );
}

CString CDownload::GetDownloadStatus() const
{
	CString strText;
	int nSources = GetEffectiveSourceCount();

	if ( IsCompleted() )
	{
		if ( IsSeeding() )
		{
			if ( m_bTorrentTrackerError )
				LoadString( strText, IDS_STATUS_TRACKERDOWN );
			else
				LoadString( strText, IDS_STATUS_SEEDING );
		}
		else
			LoadString( strText, IDS_STATUS_COMPLETED );
	}
	else if ( IsPaused() )
	{
		if ( GetFileError() != ERROR_SUCCESS )
			if ( IsMoving() )
				LoadString( strText, IDS_STATUS_CANTMOVE );
			else
				LoadString( strText, IDS_STATUS_FILEERROR );
		else
			LoadString( strText, IDS_STATUS_PAUSED );
	}
	else if ( IsMoving() )
		LoadString( strText, IDS_STATUS_MOVING );
	else if ( IsStarted() && GetProgress() == 100.0f )
		LoadString( strText, IDS_STATUS_VERIFYING );
	else if ( IsDownloading() )
	{
		DWORD nTime = GetTimeRemaining();

		if ( nTime == 0xFFFFFFFF )
			LoadString( strText, IDS_STATUS_ACTIVE );
		else if ( nTime == 0 )
			LoadString( strText, IDS_STATUS_DOWNLOADING );
		else if ( nTime > 86400 )
			strText.Format( _T("%u:%.2u:%.2u:%.2u"), nTime / 86400, ( nTime / 3600 ) % 24, ( nTime / 60 ) % 60, nTime % 60 );
		else
			strText.Format( _T("%u:%.2u:%.2u"), nTime / 3600, ( nTime / 60 ) % 60, nTime % 60 );
	}
	else if ( ! IsTrying() )
		LoadString( strText, IDS_STATUS_QUEUED );
	else if ( nSources > 0 )
		LoadString( strText, IDS_STATUS_PENDING );
	else if ( IsTorrent() )
	{
		if ( GetTaskType() == dtaskAllocate )
			LoadString( strText, IDS_STATUS_CREATING );
		else if ( m_bTorrentTrackerError )
			LoadString( strText, IDS_STATUS_TRACKERDOWN );
		else
			LoadString( strText, IDS_STATUS_TORRENT );
	}
	else
		LoadString( strText, IDS_STATUS_QUEUED );

	return strText;
}

int CDownload::GetClientStatus() const
{
	return IsCompleted() ? -1 : (int)GetEffectiveSourceCount();
}

CString CDownload::GetDownloadSources() const
{
	int nTotalSources = GetSourceCount();
	int nSources = GetEffectiveSourceCount();

	CString strText;
	if ( IsCompleted() )
	{
		if ( m_bVerify == TRI_TRUE )
			LoadString( strText, IDS_STATUS_VERIFIED );
		else if ( m_bVerify == TRI_FALSE )
			LoadString( strText, IDS_STATUS_UNVERIFIED );
	}
	else if ( nTotalSources == 0 )
		LoadString( strText, IDS_STATUS_NOSOURCES );
	else if ( nSources == nTotalSources )
	{
		CString strSource;
		LoadSourcesString( strSource,  nSources );
		strText.Format( _T("(%i %s)"), nSources, (LPCTSTR)strSource );
	}
	else
	{
		CString strSource;
		LoadSourcesString( strSource,  nTotalSources, true );
		strText.Format( _T("(%i/%i %s)"), nSources, nTotalSources, (LPCTSTR)strSource );
	}
	return strText;
}

//////////////////////////////////////////////////////////////////////
// CDownload run handler

void CDownload::OnRun()
{
	// Set the currently downloading state
	// (Used to optimize display in Ctrl/Wnd functions)
	m_bDownloading = false;

	DWORD tNow = GetTickCount();

	if ( ! IsPaused() )
	{
		if ( GetFileError() != ERROR_SUCCESS  )
		{
			// File or disk errors
			Pause( FALSE );
		}
		else if ( IsMoving() )
		{
			if ( ! IsCompleted() && ! IsTasking() )
				OnDownloaded();
		}
		else if ( IsTrying() || IsSeeding() )
		{
			// This download is trying to download
			OpenDownload();

			//'Dead download' check- if download appears dead, give up and allow another to start.
			if ( ! IsCompleted() && ( tNow - GetStartTimer() ) > ( 3 * 60 * 60 * 1000 )  )
			{	//If it's not complete, and we've been trying for at least 3 hours

				DWORD tHoursToTry = min( ( GetEffectiveSourceCount() + 49u ) / 50u, 9lu ) + Settings.Downloads.StarveGiveUp;

				if (  ( tNow - m_tReceived ) > ( tHoursToTry * 60 * 60 * 1000 ) )
				{	//And have had no new data for 5-14 hours

					if ( IsTorrent() )	//If it's a torrent
					{
						if ( Downloads.GetTryingCount( true ) >= Settings.BitTorrent.DownloadTorrents )
						{	//If there are other torrents that could start
							StopTrying();		//Give up for now, try again later
							return;
						}
					}
					else			//It's a regular download
					{
						if ( Downloads.GetTryingCount() >= ( Settings.Downloads.MaxFiles + Settings.Downloads.MaxFileSearches ) )
						{	//If there are other downloads that could try
							StopTrying();		//Give up for now, try again later
							return;
						}
					}
				}
			}	//End of 'dead download' check

			// Run the download
			RunTorrent( tNow );
			RunSearch( tNow );
			RunValidation();

			if ( IsSeeding() )
			{
				// Mark as collapsed to get correct heights when dragging files
				if ( ! Settings.General.DebugBTSources && m_bExpanded )
					m_bExpanded = FALSE;
			}
			else
			{
				if ( IsComplete() && IsFileOpen() )
				{
					if ( IsFullyVerified() )
						OnDownloaded();
				}
				else if ( CheckTorrentRatio() )
				{
					if ( Network.IsConnected() )
						StartTransfersIfNeeded( tNow );
					else
					{
						StopTrying();
						return;
					}
				}
			}

			// Calculate the current downloading state
			if ( HasActiveTransfers() )
				m_bDownloading = true;

			// Mutate regular download to torrent download
			if ( Settings.BitTorrent.EnablePromote && m_oBTH && ! IsTorrent() )
			{
				m_pTorrent.Clear();
				m_pTorrent.m_oMD5	= m_oMD5;
				m_pTorrent.m_oBTH	= m_oBTH;
				m_pTorrent.m_oSHA1	= m_oSHA1;
				m_pTorrent.m_oED2K	= m_oED2K;
				m_pTorrent.m_oTiger	= m_oTiger;
				m_pTorrent.m_sName	= m_sName;
				m_pTorrent.m_nSize	= m_nSize;
				SetTorrent();
			}
		}
		else if ( ! IsCompleted() && m_bVerify != TRI_TRUE )
		{
			// This is pending download
			if ( Network.IsConnected() )
			{
				// We have extra regular downloads 'trying' so when a new slot
				// is ready, a download has sources and is ready to go.
				if ( Downloads.GetTryingCount() < ( Settings.Downloads.MaxFiles + Settings.Downloads.MaxFileSearches ) )
				{
					if ( ! IsTorrent() ||
						// Torrents only try when 'ready to go'. (Reduce tracker load)
						Downloads.GetTryingCount( true ) < Settings.BitTorrent.DownloadTorrents )
					{
						Resume();
					}
				}
			}
		}
	}

	// Don't save Downloads with many sources too often, since it's slow
	if ( tNow - m_tSaved >=
		( GetCount() > 20 ? 5 * Settings.Downloads.SaveInterval : Settings.Downloads.SaveInterval ) )
	{
		if ( IsModified() )
		{
			FlushFile();
			if ( Save() )
				m_tSaved = tNow;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownload download complete handler

void CDownload::OnDownloaded()
{
	ASSERT( m_bComplete == false );

	theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_COMPLETED, (LPCTSTR)GetDisplayName() );

	m_tCompleted = GetTickCount();
	m_bDownloading = false;

	StopSearch();

	CloseTransfers();

	// AppendMetadata();

	if ( GetTaskType() == dtaskMergeFile || GetTaskType() == dtaskPreviewRequest )
	{
		AbortTask();
	}

	m_pTask.Copy();
}

//////////////////////////////////////////////////////////////////////
// CDownload moved handler

void CDownload::OnMoved()
{
	CSingleLock pTransfersLock( &Transfers.m_pSection, TRUE );

	// We just completed torrent
	if ( IsTorrent() )
	{
		if ( IsFullyVerified() )
		{
			// Set to FALSE to prevent sending 'stop' announce to tracker
			m_bTorrentRequested = FALSE;
			StopTrying();

			// Send 'completed' announce to tracker
			SendCompleted();

			// This torrent is now seeding
			m_bSeeding = TRUE;
			m_bVerify = TRI_TRUE;
			m_bTorrentStarted = TRUE;
			m_bTorrentRequested = TRUE;
		}
		else // Something wrong (?), since we moved the torrent
		{
			// Explicitly set to TRUE to send stop announce to tracker
			m_bTorrentRequested = TRUE;
			StopTrying();
		}
	}
	else
		StopTrying();

	ASSERT( ! m_sPath.IsEmpty() );
	DeleteFileEx( m_sPath + _T(".png"), FALSE, FALSE, TRUE );
	DeleteFileEx( m_sPath + _T(".sav"), FALSE, FALSE, TRUE );
	DeleteFileEx( m_sPath, FALSE, FALSE, TRUE );
	m_sPath.Empty();

	// Download finalized, tracker notified, set flags that we completed
	m_bComplete		= true;
	m_tCompleted	= GetTickCount();
}

//////////////////////////////////////////////////////////////////////
// CDownload open the file

BOOL CDownload::OpenDownload()
{
	if ( m_sName.IsEmpty() )
		// Download has no name yet, postponing
		return TRUE;

	if ( IsFileOpen() )
		// Already opened
		return TRUE;

	SetModified();

	if ( IsTorrent() && ! ( m_oSHA1 || m_oTiger || m_oED2K || m_oMD5 ) )
	{
		if ( Open( m_pTorrent ) )
			return TRUE;
	}
	else
	{
		if ( Open( this ) )
			return TRUE;
	}

	if ( m_nSize != SIZE_UNKNOWN && ! Downloads.IsSpaceAvailable( m_nSize, Downloads.dlPathIncomplete ) )
	{
		CString sFileError;
		sFileError.Format( LoadString( IDS_DOWNLOAD_DISK_SPACE ), (LPCTSTR)m_sName, (LPCTSTR)Settings.SmartVolume( m_nSize ) );
		SetFileError( ERROR_DISK_FULL, sFileError );

		theApp.Message( MSG_ERROR, _T("%s"), (LPCTSTR)sFileError );
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownload prepare file

BOOL CDownload::PrepareFile()
{
	return OpenDownload() && IsRemaining();
}

//////////////////////////////////////////////////////////////////////
// CDownload seed

BOOL CDownload::SeedTorrent()
{
	if ( IsMoving() || IsCompleted() )
		return FALSE;

	ASSERT( IsFileOpen() == FALSE );
	if ( IsFileOpen() )
		return FALSE;

	ASSERT( m_pTorrent.GetCount() );

	auto_ptr< CFragmentedFile > pFragmentedFile( new CFragmentedFile );
	if ( ! pFragmentedFile.get() )
		// Out of memory
		return FALSE;

	if ( ! pFragmentedFile->Open( m_pTorrent, FALSE ) )
	{
		SetFileError( pFragmentedFile->GetFileError(), pFragmentedFile->GetFileErrorString() );
		return FALSE;
	}

	AttachFile( pFragmentedFile.release() );

	if ( IsSingleFileTorrent() )
	{
		// Refill missed hashes for single-file torrent
		CBTInfo::CBTFile* pBTFile = m_pTorrent.m_pFiles.GetHead();
		if ( ! m_pTorrent.m_oSHA1 && pBTFile->m_oSHA1 )
			m_pTorrent.m_oSHA1 = pBTFile->m_oSHA1;
		if ( ! m_pTorrent.m_oTiger && pBTFile->m_oTiger )
			m_pTorrent.m_oTiger = pBTFile->m_oTiger;
		if ( ! m_pTorrent.m_oED2K && pBTFile->m_oED2K )
			m_pTorrent.m_oED2K = pBTFile->m_oED2K;
		if ( ! m_pTorrent.m_oMD5 && pBTFile->m_oMD5 )
			m_pTorrent.m_oMD5 = pBTFile->m_oMD5;

		// Refill missed hash for library file
		CQuickLock oLock( Library.m_pSection );
		if ( CLibraryFile* pLibraryFile = LibraryMaps.LookupFileByPath( pBTFile->FindFile() ) )
		{
			if ( ! pLibraryFile->m_oBTH && m_oBTH )
			{
				Library.RemoveFile( pLibraryFile );

				pLibraryFile->m_oBTH = m_oBTH;

				Library.AddFile( pLibraryFile );
			}
		}
	}

	// Refill missed hashes
	if ( ! m_oSHA1 && m_pTorrent.m_oSHA1 )
		m_oSHA1 = m_pTorrent.m_oSHA1;
	if ( ! m_oTiger && m_pTorrent.m_oTiger )
		 m_oTiger = m_pTorrent.m_oTiger;
	if ( ! m_oED2K && m_pTorrent.m_oED2K )
		m_oED2K = m_pTorrent.m_oED2K;
	if ( ! m_oMD5 && m_pTorrent.m_oMD5 )
		m_oMD5 = m_pTorrent.m_oMD5;

	GenerateTorrentDownloadID();

	m_bSeeding = TRUE;
	m_bComplete = true;
	m_tCompleted = GetTickCount();
	m_bVerify = TRI_TRUE;

	memset( m_pTorrentBlock, TRI_TRUE, m_nTorrentBlock );
	m_nTorrentSuccess = m_nTorrentBlock;

	MakeComplete();
	ResetVerification();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownload load and save

BOOL CDownload::Load(LPCTSTR pszName)
{
	ASSERT( m_sPath.IsEmpty() );
	m_sPath = pszName;

	BOOL bSuccess = FALSE;
	CFile pFile;
	if ( pFile.Open( _T("\\\\?\\") + m_sPath, CFile::modeRead ) )
	{
		TRY
		{
			CArchive ar( &pFile, CArchive::load, 32768 );	// 32 KB buffer
			Serialize( ar, 0 );
			bSuccess = TRUE;
		}
		CATCH( CFileException, pException )
		{
			if ( pException->m_cause == CFileException::fileNotFound )
			{
				// Subfile missing
				return FALSE;
			}
		}
		AND_CATCH_ALL( pException )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_FILE_OPEN_ERROR, (LPCTSTR)m_sPath );
		}
		END_CATCH_ALL

		pFile.Close();
	}

	if ( ! bSuccess && pFile.Open( _T("\\\\?\\") + m_sPath + _T(".sav"), CFile::modeRead ) )
	{
		TRY
		{
			CArchive ar( &pFile, CArchive::load, 32768 );	// 32 KB buffer
			Serialize( ar, 0 );
			bSuccess = TRUE;
		}
		CATCH( CFileException, pException )
		{
			if ( pException->m_cause == CFileException::fileNotFound )
			{
				// Subfile missing
				return FALSE;
			}
		}
		AND_CATCH_ALL( pException )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_FILE_OPEN_ERROR, (LPCTSTR)( m_sPath + _T(".sav") ) );
		}
		END_CATCH_ALL

		pFile.Close();

		if ( bSuccess )
			Save();
	}

	m_nSaveCookie = m_nCookie;

	return bSuccess;
}

BOOL CDownload::Save(BOOL bFlush)
{
	CSingleLock pTransfersLock( &Transfers.m_pSection, TRUE );

	if ( m_sPath.IsEmpty() )
	{
		// From incomplete folder
		m_sPath = Settings.Downloads.IncompletePath + _T("\\") + GetFilename() + _T(".sd");
	}

	m_nSaveCookie = m_nCookie;
	m_tSaved = GetTickCount();

	if ( m_bComplete && !m_bSeeding )
		return TRUE;

	if ( m_bSeeding && !Settings.BitTorrent.AutoSeed )
		return TRUE;

	DeleteFileEx( m_sPath + _T(".sav"), FALSE, FALSE, FALSE );

	CFile pFile;
	if ( ! pFile.Open( _T("\\\\?\\") + m_sPath + _T(".sav"),
		CFile::modeReadWrite|CFile::modeCreate|CFile::osWriteThrough ) )
		return FALSE;

	CHAR szID[3] = { 0, 0, 0 };
	try
	{
		CArchive ar( &pFile, CArchive::store, 32768 );	// 32 KB buffer
		try
		{
			Serialize( ar, 0 );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			return FALSE;
		}

		if ( Settings.Downloads.FlushSD || bFlush )
			pFile.Flush();

		pFile.SeekToBegin();
		pFile.Read( szID, 3 );
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		return FALSE;
	}

	BOOL bSuccess = FALSE;
	if ( szID[0] == 'S' && szID[1] == 'D' && szID[2] == 'L' )
	{
		bSuccess = ::MoveFileEx( CString( _T("\\\\?\\") ) + m_sPath + _T(".sav"),
			CString( _T("\\\\?\\") ) + m_sPath,
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH );
	}
	else
		DeleteFileEx( m_sPath + _T(".sav"), FALSE, FALSE, FALSE );

	return bSuccess;
}

BOOL CDownload::OnVerify(const CLibraryFile* pFile, TRISTATE bVerified)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( ! CDownloadWithExtras::OnVerify( pFile, bVerified ) )
		return FALSE;

	if ( bVerified != TRI_FALSE && ( ! IsTorrent() || IsSingleFileTorrent() ) )
	{
		if ( ! m_oSHA1 && pFile->m_oSHA1 )
			m_oSHA1 = pFile->m_oSHA1;
		if ( ! m_oTiger && pFile->m_oTiger )
			m_oTiger = pFile->m_oTiger;
		if ( ! m_oED2K && pFile->m_oED2K )
			m_oED2K = pFile->m_oED2K;
		if ( ! m_oMD5 && pFile->m_oMD5 )
			m_oMD5 = pFile->m_oMD5;

		// Auto-start for certain file extensions
		const CString strExt = PathFindExtension( pFile->GetPath() );
		if ( strExt.CompareNoCase( _T(".torrent") ) == 0 )
		{
			theApp.Message( MSG_DEBUG, _T("Auto-starting torrent file: %s"), (LPCTSTR)pFile->GetPath() );
			theApp.OpenTorrent( pFile->GetPath(), TRUE );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownload serialize

void CDownload::Serialize(CArchive& ar, int nVersion /* DOWNLOAD_SER_VERSION */)
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
			if ( strncmp( szID, "SDL", 3 ) != 0 ) AfxThrowUserException();
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
		{
			// Compatibility for CB Branch.
			if ( ! ar.IsBufferEmpty() )
			{
				CString sSearchKeyword;
				ar >> sSearchKeyword;
			}
		}
	}
}

void CDownload::SerializeOld(CArchive& ar, int nVersion /* DOWNLOAD_SER_VERSION */)
{
	ASSERT( ar.IsLoading() );

	ar >> m_sPath;
	m_sPath += _T(".sd");
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

	CDownloadWithFile::SerializeFile( ar, nVersion );

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

void CDownload::ForceComplete()
{
	m_bPaused = FALSE;
	m_bTempPaused = FALSE;
	m_bVerify = TRI_FALSE;
	MakeComplete();
	StopTrying();
	Share( FALSE );
	OnDownloaded();
}

BOOL CDownload::Launch(int nIndex, CSingleLock* pLock, BOOL bForceOriginal)
{
	if ( nIndex < 0 )
		nIndex = SelectFile( pLock );
	if ( nIndex < 0 || ! Downloads.Check( this ) )
		return FALSE;

	BOOL bResult = TRUE;
	CString strPath = GetPath( nIndex );
	CString strName = GetName( nIndex );
	CString strExt = strName.Mid( strName.ReverseFind( '.' ) );
	if ( IsCompleted() )
	{
		// Run complete file
		if ( pLock ) pLock->Unlock();

		bResult = CFileExecutor::Execute( strPath, strExt );

		if ( pLock ) pLock->Lock();
	}
	else if ( CanPreview( nIndex ) )
	{
		if ( ! bForceOriginal  )
		{
			// Previewing...
			if ( pLock ) pLock->Unlock();

			TRISTATE bSafe = CFileExecutor::IsSafeExecute( strExt, strName );

			if ( pLock ) pLock->Lock();

			if ( bSafe == TRI_UNKNOWN )
				return FALSE;
			else if ( bSafe == TRI_FALSE )
				return TRUE;

			if ( ! Downloads.Check( this ) )
				return TRUE;

			if ( PreviewFile( nIndex, pLock ) )
				return TRUE;
		}

		// Run file as is
		if ( pLock ) pLock->Unlock();

		bResult = CFileExecutor::Execute( strPath, strExt );

		if ( pLock ) pLock->Lock();
	}

	return bResult;
}

BOOL CDownload::Enqueue(int nIndex, CSingleLock* pLock)
{
	if ( nIndex < 0 )
		nIndex = SelectFile( pLock );
	if ( nIndex < 0 || ! Downloads.Check( this ) )
		return TRUE;

	BOOL bResult = TRUE;
	CString strPath = GetPath( nIndex );
	CString strName = GetName( nIndex );
	CString strExt = strName.Mid( strName.ReverseFind( '.' ) );
	if ( IsStarted() )
	{
		if ( pLock ) pLock->Unlock();

		bResult = CFileExecutor::Enqueue( strPath, strExt );

		if ( pLock ) pLock->Lock();
	}

	return bResult;
}

bool CDownload::Resize(QWORD nNewSize)
{
	if ( m_nSize == nNewSize )
		return false;

	// Check for possible change to multi-file download
	if ( ! m_oSHA1 && ! m_oTiger && ! m_oED2K && ! m_oMD5 && ( m_oBTH || IsTorrent() ) && IsFileOpen() )
	{
		// Remove old file
		AbortTask();
		ClearVerification();
		CloseFile();
		DeleteFile();

		// Create a fresh one
		AttachFile( new CFragmentedFile );
	}

	SetSize( nNewSize );

	return true;
}
