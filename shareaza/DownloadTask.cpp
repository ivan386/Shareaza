//
// DownloadTask.cpp
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
#include "DownloadGroups.h"
#include "Transfers.h"
#include "Uploads.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "LibraryMaps.h"
#include "SharedFile.h"
#include "HttpRequest.h"
#include "FragmentedFile.h"
#include "BTInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CDownloadTask, CRazaThread)

BEGIN_MESSAGE_MAP(CDownloadTask, CRazaThread)
	//{{AFX_MSG_MAP(CDownloadTask)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

const DWORD BUFFER_SIZE = 2 * 1024 * 1024u;


/////////////////////////////////////////////////////////////////////////////
// CDownloadTask construction

CDownloadTask::CDownloadTask(CDownload* pDownload, dtask nTask,
							 LPCTSTR szParam1 /*=NULL*/)
:	m_nTask				( nTask )
,	m_bSuccess			( false )
,	m_sFilename			( pDownload->m_sPath )
,	m_sDestination		( DownloadGroups.GetCompletedPath( pDownload ).TrimRight( _T("\\") ) )
,	m_nFileError		( NO_ERROR )
,	m_pDownload			( pDownload )
,	m_nSize				( pDownload->m_nSize )
,	m_posTorrentFile	( NULL )
,	m_pEvent			( NULL )
{
	ASSERT( pDownload->m_pTask == NULL );
	pDownload->m_pTask = this;

	if ( m_nTask == dtaskPreviewRequest )
	{
		m_pRequest.SetURL( szParam1 );
		m_pRequest.AddHeader( _T("Accept"), _T("image/jpeg") );
		m_pRequest.LimitContentLength( Settings.Search.MaxPreviewLength );
	}
	else if ( m_nTask == dtaskMergeFile )
	{
		m_sMergeFilename = szParam1;
	}

	if ( m_pDownload->IsTorrent() )
	{
		m_posTorrentFile = m_pDownload->m_pTorrent.m_pFiles.GetHeadPosition();
	}

	m_bAutoDelete = TRUE;
	CreateThread( "Download Task" );
}

CDownloadTask::~CDownloadTask()
{
	BOOL bCOM = SUCCEEDED( OleInitialize( NULL ) );

	Transfers.m_pSection.Lock();

	if ( Downloads.Check( m_pDownload ) )
	{
		m_pDownload->OnTaskComplete( this );
		ASSERT( m_pDownload->m_pTask != this );
	}

	CEvent* pEvent = m_pEvent;
	Transfers.m_pSection.Unlock();
	if ( pEvent != NULL )
		pEvent->SetEvent();

	if ( bCOM )
		OleUninitialize();
}

bool CDownloadTask::HasSucceeded() const
{
	return m_bSuccess;
}

DWORD CDownloadTask::GetFileError() const
{
	return m_nFileError;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask aborting

void CDownloadTask::Abort()
{
	CEvent* pEvent = m_pEvent = new CEvent();
	Transfers.m_pSection.Unlock();
	WaitForSingleObject( *pEvent, INFINITE );
	Transfers.m_pSection.Lock();
	delete pEvent;
}

bool CDownloadTask::WasAborted() const
{
	return m_pEvent != NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask run

int CDownloadTask::Run()
{
	BOOL bCOM = SUCCEEDED( OleInitialize( NULL ) );

	switch ( m_nTask )
	{
	case dtaskCopy:
		ASSERT( Downloads.Check( m_pDownload ) );
		RunCopy();
		break;

	case dtaskPreviewRequest:
		RunPreviewRequest();
		break;

	case dtaskCheckHash:
		break;

	case dtaskMergeFile:
		RunMerge();
		break;
	}

	if ( bCOM )
		OleUninitialize();

	return 0;
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
	m_nFileError = m_pDownload->MoveFile( m_sDestination, CopyProgressRoutine, this );
	m_bSuccess = ( m_nFileError == ERROR_SUCCESS );
}

DWORD CALLBACK CDownloadTask::CopyProgressRoutine(LARGE_INTEGER /*TotalFileSize*/,
	LARGE_INTEGER /*TotalBytesTransferred*/, LARGE_INTEGER /*StreamSize*/,
	LARGE_INTEGER /*StreamBytesTransferred*/, DWORD /*dwStreamNumber*/,
	DWORD /*dwCallbackReason*/, HANDLE /*hSourceFile*/, HANDLE /*hDestinationFile*/,
	LPVOID lpData)
{
	CDownloadTask* pThis = (CDownloadTask*)lpData;

	// TODO: Implement notification dialog

	return ( pThis->m_pEvent == NULL ) ? PROGRESS_CONTINUE : PROGRESS_CANCEL;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask preview request

void CDownloadTask::RunPreviewRequest()
{
	m_pRequest.Execute( FALSE ); // without threading
}

void CDownloadTask::RunMerge()
{
	HANDLE hSource = CreateFile( m_sMergeFilename, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	VERIFY_FILE_ACCESS( hSource, m_sMergeFilename )
	if ( hSource == INVALID_HANDLE_VALUE )
	{
		// File open error
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_FILE_OPEN_ERROR, m_sMergeFilename );
		m_bSuccess = true;
		return;
	}

	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	if ( ! Downloads.Check( m_pDownload ) ||
		  m_pDownload->IsCompleted() ||
		  m_pDownload->IsMoving() )
	{
		// Download almost completed
		pLock.Unlock();
		CloseHandle( hSource );
		m_bSuccess = true;
		return;
	}

	if ( m_pDownload->NeedTigerTree() &&
		 m_pDownload->NeedHashset() &&
		! m_pDownload->IsTorrent() )
	{
		// No hashsets
		pLock.Unlock();
		CString strMessage;
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_COMPLETE_NOHASH );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		CloseHandle( hSource );
		m_bSuccess = true;
		return;
	}

	const Fragments::List oList( m_pDownload->GetEmptyFragmentList() );
	if ( ! oList.size() )
	{
		// No available fragments
		pLock.Unlock();
		CloseHandle( hSource );
		m_bSuccess = true;
		return;
	}

	QWORD qwSourceOffset = 0;
	if ( m_pDownload->IsTorrent() && ! m_pDownload->IsSingleFileTorrent() )
	{
		CString sFilename( PathFindFileName( m_sMergeFilename ) );
		QWORD qwOffset = 0;
		for ( POSITION pos = m_pDownload->m_pTorrent.m_pFiles.GetHeadPosition() ; pos ; )
		{
			CBTInfo::CBTFile* pFile = m_pDownload->m_pTorrent.m_pFiles.GetNext( pos );
			int nSlash = pFile->m_sPath.Find( _T('\\') ) + 1;
			if ( pFile->m_sPath.Mid( nSlash ).CompareNoCase( sFilename ) == 0 )
			{
				// Found
				qwSourceOffset = qwOffset;
				break;
			}
			qwOffset += pFile->m_nSize;
		}
	}
	DWORD dwSourceSizeHigh = 0;
	DWORD dwSourceSizeLow = GetFileSize( hSource, &dwSourceSizeHigh );
	QWORD qwSourceLength = (QWORD)dwSourceSizeLow + ( (QWORD)dwSourceSizeHigh << 32 );

	pLock.Unlock();

	const int nBufferLength = 65536;

	// Read missing file fragments from selected file
	auto_array< BYTE > Buf( new BYTE [nBufferLength] );
	for ( Fragments::List::const_iterator pFragment = oList.begin();
		pFragment != oList.end() && m_pEvent == NULL; ++pFragment )
	{
		QWORD qwLength = pFragment->end() - pFragment->begin();
		QWORD qwOffset = pFragment->begin();

		if ( qwOffset + qwLength <= qwSourceOffset ||
			 qwSourceOffset + qwSourceLength <= qwOffset )
			 // No overlapped fragments
			 continue;

		// Calculate overlapped range end offset
		QWORD qwEnd = min( qwOffset + qwLength, qwSourceOffset + qwSourceLength );
		// Calculate overlapped range start offset
		qwOffset = max( qwOffset, qwSourceOffset );
		// Calculate overlapped range length
		qwLength = qwEnd - qwOffset;
		// Calculate file offset if any
		QWORD qwFileOffset = ( qwOffset > qwSourceOffset ) ? qwOffset - qwSourceOffset : 0;

		LONG nFileOffsetHigh = (LONG)( qwFileOffset >> 32 );
		LONG nFileOffsetLow = (LONG)( qwFileOffset & 0xFFFFFFFF );
		SetFilePointer( hSource, nFileOffsetLow, &nFileOffsetHigh, FILE_BEGIN );
		if ( GetLastError() == NO_ERROR )
		{
			DWORD dwToRead;
			while ( ( dwToRead = (DWORD)min( qwLength, (QWORD)nBufferLength ) ) != 0 &&
				m_pEvent == NULL )
			{
				DWORD dwReaded = 0;
				if ( ReadFile( hSource, Buf.get(), dwToRead, &dwReaded, NULL ) &&
					dwReaded != 0 )
				{
					pLock.Lock();
					m_pDownload->SubmitData( qwOffset, Buf.get(), (QWORD) dwReaded );
					qwOffset += (QWORD) dwReaded;
					qwLength -= (QWORD) dwReaded;
					pLock.Unlock();
				}
				else
				{
					// File error or end of file. Not Fatal
					break;
				}
			}
			m_pDownload->RunValidation();
		}
	}

	CloseHandle( hSource );
	m_bSuccess = true;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask copy file

BOOL CDownloadTask::CopyFile(HANDLE hSource, LPCTSTR pszTarget, QWORD nLength)
{
	HANDLE hTarget = CreateFile( pszTarget, GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	m_nFileError = GetLastError();
	VERIFY_FILE_ACCESS( hTarget, pszTarget )
	if ( hTarget == INVALID_HANDLE_VALUE ) return FALSE;

	BYTE* pBuffer = new BYTE[ BUFFER_SIZE ];

	while ( nLength )
	{
		DWORD nBuffer	= (DWORD)min( nLength, BUFFER_SIZE );
		DWORD nSuccess	= 0;
		DWORD tStart	= GetTickCount();

		if ( !ReadFile( hSource, pBuffer, nBuffer, &nBuffer, NULL )
			|| !nBuffer
			|| !WriteFile( hTarget, pBuffer, nBuffer, &nSuccess, NULL )
			|| nSuccess != nBuffer )
		{
			m_nFileError = GetLastError();
			break;
		}

		nLength -= nBuffer;

		if ( m_pEvent != NULL ) break;
		tStart = ( GetTickCount() - tStart ) / 2;
		Sleep( min( tStart, 50ul ) );
		if ( m_pEvent != NULL ) break;
	}

	delete [] pBuffer;

	CloseHandle( hTarget );
	if ( nLength > 0 )
		DeleteFileEx( pszTarget, TRUE, FALSE, TRUE );

	return ( nLength == 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask filename processor

CString CDownloadTask::SafeFilename(LPCTSTR pszName)
{
	static LPCTSTR pszValid = _T(" `~!@#$%^&()-_=+[]{}';.,");
	CString strName = pszName;

	for ( int nChar = 0 ; nChar < strName.GetLength() ; nChar++ )
	{
		TCHAR cChar = strName.GetAt( nChar );

		if ( (DWORD)cChar > 128 )
			continue;
		else
		{
			if ( IsCharacter( cChar ) )
				continue;
			if ( _tcschr( pszValid, cChar ) != NULL )
				continue;
		}

		strName.SetAt( nChar, '_' );
	}

	LPCTSTR pszExt = _tcsrchr( strName, '.' );
	if ( pszExt )
	{
		if ( _tcsicmp( pszExt, _T(".sd") ) == 0 )
			strName += _T("x");
	}

	// Maximum filepath length is
	// <Windows limit = 256 - 1> - <length of path to download directory> - <length of hash = 39(tiger)> -<space = 1> - <length of ".sd.sav" = 7>
	int nMaxFilenameLength = 256 - 1 - Settings.Downloads.IncompletePath.GetLength() - 47;
	if ( strName.GetLength() > nMaxFilenameLength )
	{
		int nExtLen = pszExt ? static_cast< int >( _tcslen( pszExt ) ) : 0;
		strName = strName.Left( nMaxFilenameLength - nExtLen ) + strName.Right( nExtLen );
	}

	return strName;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask path creator

void CDownloadTask::CreatePathForFile(const CString& strBase, const CString& strPath)
{
	CreateDirectory( strBase );

	for ( int nPos = 0 ; nPos < strPath.GetLength() ; nPos++ )
	{
		if ( strPath.GetAt( nPos ) == '\\' )
		{
			CString strFolder = strBase + '\\' + strPath.Left( nPos );
			CreateDirectory( strFolder );
		}
	}
}

CBuffer* CDownloadTask::IsPreviewAnswerValid()
{
	if ( m_nTask != dtaskPreviewRequest || !m_pRequest.IsFinished() )
		return NULL;

	m_pRequest.GetStatusCode();

	if ( m_pRequest.GetStatusSuccess() == FALSE )
	{
		theApp.Message( MSG_DEBUG, L"Preview failed: HTTP status code %i",
			m_pRequest.GetStatusCode() );
		return NULL;
	}

	CString strURN = m_pRequest.GetHeader( L"X-Previewed-URN" );

	if ( strURN.GetLength() )
	{
		Hashes::Sha1Hash oSHA1;
		bool bValid = true;

		if ( m_pDownload )
		{
			if ( oSHA1.fromUrn( strURN ) && validAndUnequal( oSHA1, m_pDownload->m_oSHA1 ) )
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

	CString strMIME = m_pRequest.GetHeader( L"Content-Type" );

	if ( strMIME.CompareNoCase( L"image/jpeg" ) != 0 )
	{
		theApp.Message( MSG_DEBUG, L"Preview failed: unacceptable content type." );
		return NULL;
	}

	CBuffer* pBuffer = m_pRequest.GetResponseBuffer();
	return pBuffer;
}
