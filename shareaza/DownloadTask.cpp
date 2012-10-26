//
// DownloadTask.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "BTInfo.h"
#include "Download.h"
#include "DownloadGroups.h"
#include "DownloadTask.h"
#include "Downloads.h"
#include "FragmentedFile.h"
#include "HttpRequest.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "LibraryMaps.h"
#include "SharedFile.h"
#include "Transfers.h"
#include "Uploads.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const DWORD BUFFER_SIZE = 2 * 1024 * 1024u;


/////////////////////////////////////////////////////////////////////////////
// CDownloadTask construction

void CDownloadTask::Copy(CDownload* pDownload)
{
	CDownloadTask* pTask = new CDownloadTask( pDownload, dtaskCopy );
	if ( ! pTask )
		// Out of memory
		return;

	pTask->BeginThread( "Download Task : Copy" );
}

void CDownloadTask::PreviewRequest(CDownload* pDownload, LPCTSTR szURL)
{
	CDownloadTask* pTask = new CDownloadTask( pDownload, dtaskPreviewRequest );
	if ( ! pTask )
		// Out of memory
		return;

	pTask->m_pRequest.Attach( new CHttpRequest() );
	if ( ! pTask->m_pRequest )
		// Out of memory
		return;

	pTask->m_pRequest->SetURL( szURL );
	pTask->m_pRequest->AddHeader( _T("Accept"), _T("image/jpeg") );
	pTask->m_pRequest->LimitContentLength( Settings.Search.MaxPreviewLength );

	pTask->BeginThread( "Download Task : Preview" );
}

void CDownloadTask::MergeFile(CDownload* pDownload, CList< CString >* pFiles, BOOL bValidation, const Fragments::List* pGaps)
{
	CDownloadTask* pTask = new CDownloadTask( pDownload, dtaskMergeFile );
	if ( ! pTask )
		// Out of memory
		return;

	pTask->m_oMergeFiles.AddTail( pFiles );
	if ( pGaps )
		pTask->m_oMergeGaps = *pGaps;
	pTask->m_bMergeValidation = bValidation;

	pTask->BeginThread( "Download Task : Merge" );
}

CDownloadTask::CDownloadTask(CDownload* pDownload, dtask nTask)
	: m_nTask			( nTask )
	, m_bSuccess		( false )
	, m_sFilename		( pDownload->m_sPath )
	, m_sDestination	( DownloadGroups.GetCompletedPath( pDownload ).TrimRight( _T("\\") ) )
	, m_nFileError		( NO_ERROR )
	, m_pDownload		( pDownload )
	, m_nSize			( pDownload->m_nSize )
	, m_oMergeGaps		( pDownload->m_nSize )
	, m_bMergeValidation( FALSE )
	, m_posTorrentFile	( NULL )
	, m_fProgress		( 0 )
{
	ASSERT( ! pDownload->IsTasking() );
	pDownload->SetTask( this );

	if ( pDownload->IsTorrent() )
	{
		m_posTorrentFile = pDownload->m_pTorrent.m_pFiles.GetHeadPosition();
	}
}

CDownloadTask::~CDownloadTask()
{
}

bool CDownloadTask::HasSucceeded() const
{
	return m_bSuccess;
}

DWORD CDownloadTask::GetFileError() const
{
	return m_nFileError;
}

dtask CDownloadTask::GetTaskType() const
{
	return m_nTask;
}

CString CDownloadTask::GetRequest() const
{
	return m_pRequest ? m_pRequest->GetURL() : CString();
}

float CDownloadTask::GetProgress() const
{
	return m_fProgress;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask aborting

void CDownloadTask::Abort()
{
	Exit();

	if ( m_pRequest )
	{
		m_pRequest->Cancel();
	}

	CloseThread();
}

bool CDownloadTask::WasAborted() const
{
	return ! IsThreadEnabled();
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask run

void CDownloadTask::OnRun()
{
	switch ( m_nTask )
	{
	case dtaskCopy:
		RunCopy();
		break;

	case dtaskPreviewRequest:
		RunPreviewRequest();
		break;

	case dtaskMergeFile:
		RunMerge();
		break;

	default:
		;
	}

	{
		CQuickLock oLock( Transfers.m_pSection );

		if ( Downloads.Check( m_pDownload ) )
			m_pDownload->OnTaskComplete( this );
	}

	delete this;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask allocate

/*void CDownloadTask::RunAllocate()
{
	HANDLE hFile = CreateFile( m_sFilename, GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	VERIFY_FILE_ACCESS( hFile, m_sFilename )
	if ( hFile == INVALID_HANDLE_VALUE ) return;

	if ( GetFileSize( hFile, NULL ) != 0 )
	{
		CloseHandle( hFile );
		return;
	}

	if ( Settings.Downloads.SparseThreshold > 0 &&
		 m_nSize != SIZE_UNKNOWN &&
		 m_nSize >= Settings.Downloads.SparseThreshold * 1024 )
	{
		DWORD dwOut = 0;
		if ( ! DeviceIoControl( hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwOut, NULL ) )
		{
			DWORD nError = GetLastError();
			theApp.Message( MSG_ERROR, _T("Unable to set sparse file: \"%s\", Win32 error %x."), m_sFilename, nError );
		}
	}

	if ( m_nSize > 0 && m_nSize != SIZE_UNKNOWN )
	{
		DWORD nOffsetLow	= (DWORD)( ( m_nSize - 1 ) & 0x00000000FFFFFFFF );
		DWORD nOffsetHigh	= (DWORD)( ( ( m_nSize - 1 ) & 0xFFFFFFFF00000000 ) >> 32 );
		SetFilePointer( hFile, nOffsetLow, (PLONG)&nOffsetHigh, FILE_BEGIN );

		BYTE bZero = 0;
		DWORD nSize = 0;
		WriteFile( hFile, &bZero, 1, &nSize, NULL );
	}

	CloseHandle( hFile );
	m_bSuccess = TRUE;
}*/

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask simple copy

void CDownloadTask::RunCopy()
{
	m_fProgress = 0;
	m_nFileError = m_pDownload->MoveFile( m_sDestination, CopyProgressRoutine, this );
	m_fProgress = 100.0f;

	m_bSuccess = ( m_nFileError == ERROR_SUCCESS );
}

DWORD CALLBACK CDownloadTask::CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
	LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER /*StreamSize*/,
	LARGE_INTEGER /*StreamBytesTransferred*/, DWORD /*dwStreamNumber*/,
	DWORD /*dwCallbackReason*/, HANDLE /*hSourceFile*/, HANDLE /*hDestinationFile*/,
	LPVOID lpData)
{
	CDownloadTask* pThis = (CDownloadTask*)lpData;

	if ( TotalFileSize.QuadPart) 
		pThis->m_fProgress = (float)( TotalBytesTransferred.QuadPart * 100 ) / (float)TotalFileSize.QuadPart;
	else
		pThis->m_fProgress = 100.0f;

	return pThis->IsThreadEnabled() ? PROGRESS_CONTINUE : PROGRESS_CANCEL;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask preview request

void CDownloadTask::RunPreviewRequest()
{
	m_bSuccess = m_pRequest->Execute( false ); // without threading
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask merge

void CDownloadTask::RunMerge()
{
	bool bSuccess = true;
	for ( POSITION pos = m_oMergeFiles.GetHeadPosition(); IsThreadEnabled() && pos; )
	{
		bSuccess = m_pDownload->MergeFile( m_oMergeFiles.GetNext( pos ), m_bMergeValidation, m_oMergeGaps, this ) && bSuccess;
	}

	m_bSuccess = bSuccess;
}

CBuffer* CDownloadTask::IsPreviewAnswerValid(const Hashes::Sha1Hash& oRequestedSHA1) const
{
	if ( m_nTask != dtaskPreviewRequest || ! m_pRequest->IsFinished() )
		return NULL;

	m_pRequest->GetStatusCode();

	if ( m_pRequest->GetStatusSuccess() == FALSE )
	{
		theApp.Message( MSG_DEBUG, L"Preview failed: HTTP status code %i", m_pRequest->GetStatusCode() );
		return NULL;
	}

	CString strURN = m_pRequest->GetHeader( L"X-Previewed-URN" );

	if ( strURN.GetLength() )
	{
		Hashes::Sha1Hash oSHA1;
		bool bValid = true;

		if ( oRequestedSHA1 )
		{
			if ( oSHA1.fromUrn( strURN ) && validAndUnequal( oSHA1, oRequestedSHA1 ) )
				bValid = false;
		}
		else
		{
			CSingleLock oLock( &Library.m_pSection, TRUE );
			CLibraryFile* pFile = LibraryMaps.LookupFileBySHA1( oSHA1 );
			if ( pFile == NULL )
				bValid = false;
			oLock.Unlock();
		}
		if ( !bValid )
		{
			theApp.Message( MSG_DEBUG, L"Preview failed: wrong URN." );
			return NULL;
		}
	}

	CString strMIME = m_pRequest->GetHeader( L"Content-Type" );

	if ( strMIME.CompareNoCase( L"image/jpeg" ) != 0 )
	{
		theApp.Message( MSG_DEBUG, L"Preview failed: unacceptable content type." );
		return NULL;
	}

	return m_pRequest->GetResponseBuffer();
}
