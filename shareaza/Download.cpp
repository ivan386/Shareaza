//
// Download.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "FileExecutor.h"
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
,	m_tCompleted	( 0 )
,	m_nRunCookie	( 0 )
,	m_nSaveCookie	( 0 )
,	m_nGroupCookie	( 0 )

,	m_bTempPaused	( FALSE )
,	m_bPaused		( FALSE )
,	m_bBoosted		( FALSE )
,	m_bShared		( Settings.Uploads.SharePartials )
,	m_bComplete		( false )
,	m_tSaved		( 0 )
,	m_tBegan		( 0 )
,	m_bDownloading	( false )
{
}

CDownload::~CDownload()
{
	AbortTask();
	DownloadGroups.Unlink( this );
}

//////////////////////////////////////////////////////////////////////
// CDownload control : pause

void CDownload::Pause(BOOL bRealPause)
{
	if ( m_bComplete || m_bPaused )
		return;

	theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_PAUSED, GetDisplayName() );

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

	if ( IsFileOpen() )
	{
		for ( CDownloadSource* pSource = GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
		{
			pSource->OnResume();
		}
	}

	m_bPaused				= FALSE;
	m_bTempPaused			= FALSE;
	m_tReceived				= GetTickCount();
	m_bTorrentTrackerError	= FALSE;

	// Try again
	ClearFileError();

	if ( IsTorrent() )
	{
		if ( Downloads.GetTryingCount( true ) < Settings.BitTorrent.DownloadTorrents )
			SetStartTimer();
	}
	else
	{
		if ( Downloads.GetTryingCount() < ( Settings.Downloads.MaxFiles + Settings.Downloads.MaxFileSearches ) )
			SetStartTimer();
	}

	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownload control : remove

void CDownload::Remove(bool bDelete)
{
	CloseTorrent();
	CloseTransfers();
	AbortTask();

	if ( IsTrying() )
		Downloads.StopTrying( IsTorrent() );

	if ( bDelete || ! IsCompleted() )
	{
		theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_REMOVE, (LPCTSTR)GetDisplayName() );
		DeleteFile();
	}
	else
		CloseFile();

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
// CDownload control : rename

bool CDownload::Rename(const CString strName)
{
	// Don't bother if renaming to same name.
	if ( m_sName == strName )
		return false;

	// Set new name
	m_sName = strName;

	SetModified();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownload control : Stop trying

void CDownload::StopTrying()
{
	if ( m_bComplete )
		return;

	if ( IsTrying() )
		Downloads.StopTrying( IsTorrent() );

	m_tBegan = 0;
	m_bDownloading	= false;

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
// CDownload control : SetStartTimer

void CDownload::SetStartTimer()
{
	Downloads.StartTrying( IsTorrent() );
	m_tBegan = GetTickCount();
	SetModified();
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
	return m_bBoosted != 0;
}

// Is the download currently trying to download?
bool CDownload::IsTrying() const
{
	return  m_tBegan != 0 ;
}

bool CDownload::IsShared() const
{
	if ( m_bShared )
		return true;

	if ( Settings.eDonkey.EnableToday && m_oED2K )
		return true;

	if ( Settings.BitTorrent.EnableToday && IsTorrent()
		&& ( IsSeeding() || IsStarted() ) )
	{
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// CDownload run handler

void CDownload::OnRun()
{
	// Set the currently downloading state
	// (Used to optimize display in Ctrl/Wnd functions)
	m_bDownloading = false;

	DWORD tNow = GetTickCount();

	if ( ! m_bTempPaused )
	{
		if ( GetFileError() != ERROR_SUCCESS  )
		{
			// File or disk errors
			Pause( FALSE );
		}
		else if ( IsMoving() )
		{
			if ( !m_bComplete && !IsTasking() )
				OnDownloaded();
		}
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
			if ( RunTorrent( tNow ) )
			{
				RunSearch( tNow );

				if ( m_bSeeding )
				{
					// Mark as collapsed to get correct heights when dragging files
					if ( !Settings.General.DebugBTSources && m_bExpanded )
						m_bExpanded = FALSE;

					RunValidation();
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

				RunValidation();

				if ( IsComplete() && IsFileOpen() )
				{
					if ( ValidationCanFinish() )
						OnDownloaded();
				}
				else if ( CheckTorrentRatio() )
				{
					if ( Network.IsConnected() )
						StartTransfersIfNeeded( tNow );
					else
						m_tBegan = 0;
				}
			} // if ( RunTorrent( tNow ) )

			// Calculate the current downloading state
			if( HasActiveTransfers() )
				m_bDownloading = true;
		}
		else if ( ! m_bComplete && m_bVerify != TRI_TRUE )
		{
			//If this download isn't trying to download, see if it can try
			if ( IsDownloading() )
			{	// This download was probably started by a push/etc
				SetStartTimer();
			}
			if ( Network.IsConnected() )
			{
				if ( IsTorrent() )
				{	//Torrents only try when 'ready to go'. (Reduce tracker load)
					if ( Downloads.GetTryingCount( true ) < Settings.BitTorrent.DownloadTorrents )
						SetStartTimer();
				}
				else
				{	//We have extra regular downloads 'trying' so when a new slot is ready, a download
					//has sources and is ready to go.
					if ( Downloads.GetTryingCount() < ( Settings.Downloads.MaxFiles + Settings.Downloads.MaxFileSearches ) )
						SetStartTimer();
				}
			}
			else
				m_tBegan = 0;
		}
	}

	// Don't save Downloads with many sources too often, since it's slow
	if ( tNow - m_tSaved >=
		( GetCount() > 20 ? 5 * Settings.Downloads.SaveInterval : Settings.Downloads.SaveInterval ) )
	{
		if ( m_nCookie != m_nSaveCookie )
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

	theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_COMPLETED, GetDisplayName() );
	m_tCompleted = GetTickCount();
	m_bDownloading = false;

	CloseTransfers();

	// AppendMetadata();

	if ( IsTasking() && ( GetTaskType() == CDownloadTask::dtaskMergeFile ||
		GetTaskType() == CDownloadTask::dtaskPreviewRequest ) )
	{
		AbortTask();
	}

	SetMoving( true );

	ASSERT( !IsTasking() );
	SetTask( new CDownloadTask( this, CDownloadTask::dtaskCopy ) );

	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownload task completion

void CDownload::OnTaskComplete(CDownloadTask* pTask)
{
	ASSERT( CheckTask( pTask ) );
	SetTask( NULL );

	// Check if task was aborted
	if ( pTask->WasAborted() )
		return;

	if ( pTask->GetTaskType() == CDownloadTask::dtaskPreviewRequest )
	{
		OnPreviewRequestComplete( pTask );
	}
	else if ( pTask->GetTaskType() == CDownloadTask::dtaskCopy )
	{
		if ( !pTask->HasSucceeded() )
			SetFileError( pTask->GetFileError() );
		else
			OnMoved();
	}
}

//////////////////////////////////////////////////////////////////////
// CDownload moved handler

void CDownload::OnMoved()
{
	// We just completed torrent
	if ( m_nTorrentBlock > 0 && m_nTorrentSuccess >= m_nTorrentBlock )
	{
		CloseTorrentUploads();
		SendCompleted();
		if ( IsTrying() )
			Downloads.StopTrying( IsTorrent() );
		m_bSeeding = TRUE;
		m_tBegan = 0;
		m_bDownloading	= false;
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

	ClearSources();

	ASSERT( ! m_sPath.IsEmpty() );
	DeleteFileEx( m_sPath + _T(".png"), FALSE, FALSE, TRUE );
	DeleteFileEx( m_sPath + _T(".sav"), FALSE, FALSE, TRUE );
	DeleteFileEx( m_sPath, FALSE, FALSE, TRUE );
	m_sPath.Empty();

	// Download finalized, tracker notified, set flags that we completed
	m_bComplete		= true;
	m_tCompleted	= GetTickCount();
	SetMoving( false );
}

//////////////////////////////////////////////////////////////////////
// CDownload load and save

BOOL CDownload::Load(LPCTSTR pszName)
{
	ASSERT( m_sPath.IsEmpty() );
	m_sPath = pszName;

	BOOL bSuccess = FALSE;
	CFile pFile;
	if ( pFile.Open( m_sPath, CFile::modeRead ) )
	{
		TRY
		{
			CArchive ar( &pFile, CArchive::load );
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
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_FILE_OPEN_ERROR, m_sPath );
		}
		END_CATCH_ALL

			pFile.Close();
	}

	if ( ! bSuccess && pFile.Open( m_sPath + _T(".sav"), CFile::modeRead ) )
	{
		TRY
		{
			CArchive ar( &pFile, CArchive::load );
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
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_FILE_OPEN_ERROR, m_sPath + _T(".sav") );
		}
		END_CATCH_ALL

			pFile.Close();

		if ( bSuccess )
			Save();
	}

	m_bGotPreview = GetFileAttributes( m_sPath + _T(".png") ) != INVALID_FILE_ATTRIBUTES;
	m_nSaveCookie = m_nCookie;

	return bSuccess;
}

BOOL CDownload::Save(BOOL bFlush)
{
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
	if ( ! pFile.Open( m_sPath + _T(".sav"),
		CFile::modeReadWrite|CFile::modeCreate|CFile::osWriteThrough ) )
		return FALSE;

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
			pException->Delete();
			return FALSE;
		}
	}

	if ( Settings.Downloads.FlushSD || bFlush )
		pFile.Flush();

	pFile.SeekToBegin();

	CHAR szID[3] = { 0, 0, 0 };
	pFile.Read( szID, 3 );
	pFile.Close();

	BOOL bSuccess = FALSE;
	if ( szID[0] == 'S' && szID[1] == 'D' && szID[2] == 'L' )
	{
		bSuccess = ::MoveFileEx( CString( _T("\\\\?\\") ) + m_sPath + _T(".sav"),
			CString( _T("\\\\?\\") ) + m_sPath,
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH );
	}
	else
		DeleteFileEx( m_sPath + _T(".sav"), FALSE, FALSE, FALSE );

	ASSERT( bSuccess );
	return bSuccess;
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
	SetVerifyStatus( TRI_FALSE );
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
		return TRUE;

	BOOL bResult = TRUE;
	CString strPath = GetPath( nIndex );
	CString strName = GetName( nIndex );
	CString strExt = strName.Mid( strName.ReverseFind( '.' ) );
	if ( IsCompleted() )
	{
		// Run complete file
		if ( m_bVerify == TRI_FALSE )
		{
			CString strFormat, strMessage;
			LoadString( strFormat, IDS_LIBRARY_VERIFY_FAIL );
			strMessage.Format( strFormat, (LPCTSTR)strName );

			if ( pLock ) pLock->Unlock();

			INT_PTR nResponse = AfxMessageBox( strMessage,
				MB_ICONEXCLAMATION | MB_YESNOCANCEL | MB_DEFBUTTON2 );

			if ( pLock ) pLock->Lock();

			if ( nResponse == IDCANCEL )
				return FALSE;
			if ( nResponse == IDNO )
				return TRUE;
		}

		if ( pLock ) pLock->Unlock();

		bResult = CFileExecutor::Execute( strPath, FALSE, strExt );

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

		bResult = CFileExecutor::Execute( strPath, FALSE, strExt );

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

		bResult = CFileExecutor::Enqueue( strPath, FALSE, strExt );

		if ( pLock ) pLock->Lock();
	}

	return bResult;
}
