//
// DownloadTask.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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
#include "ImageFile.h"
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


/////////////////////////////////////////////////////////////////////////////
// CDownloadTask construction

CDownloadTask::CDownloadTask(CDownload* pDownload)
	: m_pDownload		( pDownload )
	, m_nTask			( dtaskNone )
	, m_bSuccess		( false )
	, m_nFileError		( NO_ERROR )
	, m_bMergeValidation( FALSE )
	, m_fProgress		( 0 )
	, m_oMergeGaps		( 0 )
{
}

CDownloadTask::~CDownloadTask()
{
	Abort();
}

void CDownloadTask::Construct(dtask nTask)
{
	ASSERT_VALID( m_pDownload );
	ASSERT( m_nTask == dtaskNone );
	ASSERT( ! IsThreadAlive() );

	m_nTask = nTask;
	m_pRequest.Free();
	m_bSuccess = false;
	m_sDestination.Empty();
	m_nFileError = NO_ERROR;
	m_oMergeFiles.RemoveAll();
	m_bMergeValidation = FALSE;
	m_fProgress = 0;
}

void CDownloadTask::Allocate()
{
	Construct( dtaskAllocate );

	VERIFY( BeginThread( "Download Task : Allocate" ) );
}

void CDownloadTask::Copy()
{
	Construct( dtaskCopy );

	m_sDestination = DownloadGroups.GetCompletedPath( m_pDownload ).TrimRight( _T("\\") );

	VERIFY( BeginThread( "Download Task : Copy" ) );
}

void CDownloadTask::PreviewRequest(LPCTSTR szURL)
{
	Construct( dtaskPreviewRequest );

	m_pRequest.Attach( new CHttpRequest() );
	if ( ! m_pRequest )
		// Out of memory
		return;

	m_pRequest->SetURL( szURL );
	m_pRequest->AddHeader( _T("Accept"), _T("image/jpeg") );
	m_pRequest->LimitContentLength( Settings.Search.MaxPreviewLength );

	VERIFY( BeginThread( "Download Task : Preview" ) );
}

void CDownloadTask::MergeFile(CList< CString >* pFiles, BOOL bValidation, const Fragments::List* pGaps)
{
	Construct( dtaskMergeFile );

	m_oMergeFiles.AddTail( pFiles );
	if ( pGaps )
		m_oMergeGaps = *pGaps;
	else
		m_oMergeGaps = Fragments::List( m_pDownload->m_nSize );
	m_bMergeValidation = bValidation;

	VERIFY( BeginThread( "Download Task : Merge" ) );
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
	if ( m_nTask == dtaskNone )
		return;

	Exit();

	if ( m_pRequest )
	{
		m_pRequest->Cancel();
	}

	CloseThread();

	m_nTask = dtaskNone;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask run

void CDownloadTask::OnRun()
{
	switch ( m_nTask )
	{
	case dtaskAllocate:
		RunAllocate();
		break;

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

	m_nTask = dtaskNone;
	m_fProgress = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask allocate

void CDownloadTask::RunAllocate()
{
	HANDLE hFile = CreateFile( m_pDownload->m_sPath, GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	VERIFY_FILE_ACCESS( hFile, m_pDownload->m_sPath )
	if ( hFile == INVALID_HANDLE_VALUE )
		return;

	if ( GetFileSize( hFile, NULL ) != 0 )
	{
		CloseHandle( hFile );
		return;
	}

	if ( Settings.Downloads.SparseThreshold &&
		 m_pDownload->m_nSize != SIZE_UNKNOWN &&
		 m_pDownload->m_nSize >= Settings.Downloads.SparseThreshold * 1024 )
	{
		DWORD dwOut = 0;
		if ( ! DeviceIoControl( hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwOut, NULL ) )
		{
			DWORD nError = GetLastError();
			theApp.Message( MSG_ERROR, _T("Unable to set sparse file: \"%s\", Win32 error %x."), (LPCTSTR)m_pDownload->m_sPath, nError );
		}
	}

	if ( m_pDownload->m_nSize > 0 && m_pDownload->m_nSize != SIZE_UNKNOWN )
	{
		DWORD nOffsetLow	= (DWORD)( ( m_pDownload->m_nSize - 1 ) & 0x00000000FFFFFFFF );
		DWORD nOffsetHigh	= (DWORD)( ( ( m_pDownload->m_nSize - 1 ) & 0xFFFFFFFF00000000 ) >> 32 );
		SetFilePointer( hFile, nOffsetLow, (PLONG)&nOffsetHigh, FILE_BEGIN );

		BYTE bZero = 0;
		DWORD nSize = 0;
		WriteFile( hFile, &bZero, 1, &nSize, NULL );
	}

	CloseHandle( hFile );

	m_bSuccess = true;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask simple copy

void CDownloadTask::RunCopy()
{
	m_nFileError = m_pDownload->MoveFile( m_sDestination, CopyProgressRoutine, this );

	m_bSuccess = ( m_nFileError == ERROR_SUCCESS );

	// Check if task was aborted
	if ( IsThreadEnabled() )
	{
		if ( m_bSuccess )
			m_pDownload->OnMoved();
		else
			m_pDownload->SetFileError( GetFileError(), _T("") );
	}
}

DWORD CALLBACK CDownloadTask::CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
	LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER /*StreamSize*/,
	LARGE_INTEGER /*StreamBytesTransferred*/, DWORD /*dwStreamNumber*/,
	DWORD /*dwCallbackReason*/, HANDLE /*hSourceFile*/, HANDLE /*hDestinationFile*/,
	LPVOID lpData)
{
	CDownloadTask* pThis = (CDownloadTask*)lpData;

	if ( TotalFileSize.QuadPart )
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

	// Check if task was aborted
	if ( IsThreadEnabled() )
	{
		// Save downloaded preview as png-file
		const CString strPath = m_pDownload->m_sPath + _T(".png");
		CImageFile pImage;
		CBuffer* pBuffer = IsPreviewAnswerValid( m_pDownload->m_oSHA1 );
		if ( pBuffer && pBuffer->m_pBuffer && pBuffer->m_nLength &&
			 pImage.LoadFromMemory( _T(".jpg"), pBuffer->m_pBuffer, pBuffer->m_nLength, FALSE, TRUE ) &&
			 pImage.SaveToFile( (LPCTSTR)strPath, 100 ) )
		{
			// Make it hidden, so the files won't be shared
			SetFileAttributes( (LPCTSTR)strPath, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM );
		}
		else
		{
			// Failed
			m_pDownload->m_bWaitingPreview = FALSE;
			theApp.Message( MSG_ERROR, IDS_SEARCH_DETAILS_PREVIEW_FAILED, (LPCTSTR)GetRequest() );
		}
	}
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

	const CString strURN = m_pRequest->GetHeader( L"X-Previewed-URN" );
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
			if ( ! LibraryMaps.LookupFileBySHA1( oSHA1 ) )
				bValid = false;
		}
		if ( ! bValid )
		{
			theApp.Message( MSG_DEBUG, L"Preview failed: wrong URN." );
			return NULL;
		}
	}

	const CString strMIME = m_pRequest->GetHeader( L"Content-Type" );
	if ( strMIME.CompareNoCase( L"image/jpeg" ) != 0 )
	{
		theApp.Message( MSG_DEBUG, L"Preview failed: unacceptable content type." );
		return NULL;
	}

	return m_pRequest->GetResponseBuffer();
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask merge

void CDownloadTask::RunMerge()
{
	bool bSuccess = true;
	for ( POSITION pos = m_oMergeFiles.GetHeadPosition(); IsThreadEnabled() && pos; )
	{
		bSuccess = m_pDownload->RunMergeFile( m_oMergeFiles.GetNext( pos ), m_bMergeValidation, m_oMergeGaps, this ) && bSuccess;
	}

	m_bSuccess = bSuccess;
}
