//
// DownloadTask.cpp
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
#include "DownloadGroups.h"
#include "Transfers.h"
#include "Uploads.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "LibraryMaps.h"
#include "SharedFile.h"
#include "HttpRequest.h"
#include "FragmentedFile.h"

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

CDownloadTask::CDownloadTask(CDownload* pDownload, dtask nTask, LPCTSTR szParam1 /*=NULL*/)
: m_nTask		( nTask )
, m_pDownload	( pDownload )
, m_bSuccess	( FALSE )
, m_nSize		( pDownload->m_nSize )
, m_sName		( pDownload->m_sName )
, m_sFilename	( pDownload->m_sPath )
, m_sPath		( DownloadGroups.GetCompletedPath( pDownload ) )
, m_pEvent		( NULL )
, m_posTorrentFile( NULL )
, m_dwFileError	( 0 )
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

	CString sExtention = PathFindExtension( m_sName );
	sExtention.MakeLower();
	if ( sExtention == _T(".co") ||
		 sExtention == _T(".collection") ||
		 sExtention == _T(".emulecollection") )
	{
		m_sPath	= Settings.Downloads.CollectionPath;
		CreateDirectory( Settings.Downloads.CollectionPath );
		LibraryFolders.AddFolder( Settings.Downloads.CollectionPath );
	}
	else if ( sExtention == _T(".torrent") )
	{
		m_sPath	= Settings.Downloads.TorrentPath;
		CreateDirectory( Settings.Downloads.TorrentPath );
		LibraryFolders.AddFolder( Settings.Downloads.TorrentPath, FALSE );
	}

	if ( m_pDownload->IsTorrent() )
	{
		m_pTorrent.Copy( &m_pDownload->m_pTorrent );

		m_posTorrentFile = m_pTorrent.m_pFiles.GetHeadPosition();

		if ( m_nTask == dtaskCopySimple && ! m_pDownload->IsSingleFileTorrent() )
		{
			m_nTask = dtaskCopyTorrent;
		}
	}

	m_bAutoDelete = TRUE;
	CreateThread( "Download Task" );
}

CDownloadTask::~CDownloadTask()
{
	Transfers.m_pSection.Lock();

	if ( Downloads.Check( m_pDownload ) )
	{
		m_pDownload->OnTaskComplete( this );
		ASSERT( m_pDownload->m_pTask != this );
	}

	CEvent* pEvent = m_pEvent;
	Transfers.m_pSection.Unlock();
	if ( pEvent != NULL ) pEvent->SetEvent();
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

BOOL CDownloadTask::WasAborted()
{
	return m_pEvent != NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask run

int CDownloadTask::Run()
{
	switch ( m_nTask )
	{
	case dtaskAllocate:
		RunAllocate();
		break;
	case dtaskCopySimple:
		RunCopySimple();
		break;
	case dtaskCopyTorrent:
		RunCopyTorrent();
		if ( m_bSuccess == FALSE && m_pEvent == NULL )
		{
			if ( m_sPath != Settings.Downloads.CompletePath )
			{
				m_sPath = Settings.Downloads.CompletePath;
				RunCopyTorrent();
			}
		}
		break;
	case dtaskPreviewRequest:
		m_pRequest.Execute( FALSE ); // without threading
		break;
	case dtaskCheckHash:
		break;
	case dtaskMergeFile:
		RunMerge();
		break;
	case dtaskCreateBatch:
		MakeBatchTorrent();
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask allocate

void CDownloadTask::RunAllocate()
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
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask simple copy

void CDownloadTask::RunCopySimple()
{
	CString strSafeName = SafeFilename( m_sName );
	int nExt = strSafeName.ReverseFind( _T('.') );
	CString strName( nExt > 0 ? strSafeName.Left( nExt ) : strSafeName );
	CString strExt(  nExt > 0 ? strSafeName.Mid(  nExt ) : _T("") );

	// Create list of possible destinations
	CList< CString > oPathList;
	oPathList.AddTail( m_sPath );

	if ( m_sPath.CompareNoCase( Settings.Downloads.CompletePath ) != 0 )
		oPathList.AddTail( Settings.Downloads.CompletePath );

	CString sDefaultCompletePath = GetDocumentsFolder() + _T("\\Shareaza Downloads");
	if ( m_sPath.CompareNoCase( sDefaultCompletePath ) != 0 )
		oPathList.AddTail( sDefaultCompletePath );

	CString sInPlacePath = m_sFilename.Left( m_sFilename.ReverseFind( _T('\\') ) );
	if ( m_sPath.CompareNoCase( sInPlacePath ) != 0 )
		oPathList.AddTail( sInPlacePath );

	for ( POSITION pos = oPathList.GetHeadPosition(); pos; )
	{
		CString sPath = oPathList.GetNext( pos );

		if ( ! CreateDirectory( sPath ) )
		{
			m_dwFileError = GetLastError();
			continue;
		}

		sPath.TrimRight( _T("\\") );

		for ( int nCopy = 0; nCopy < 10 ; nCopy++ )
		{
			CString strTarget;
			if ( nCopy == 0 )
			{
				strTarget.Format( _T("%s\\%s%s"),
					(LPCTSTR)sPath, (LPCTSTR)strName, (LPCTSTR)strExt );
			}
			else
			{
				strTarget.Format( _T("%s\\%s (%i)%s"),
					(LPCTSTR)sPath, (LPCTSTR)strName, nCopy, (LPCTSTR)strExt );
			}

			// Disconnect all uploads for that file (i.e. close the file handle)
			while( !Uploads.OnRename( m_sFilename ) )
				Sleep( 250ul );

			theApp.Message( MSG_DEBUG, _T("Moving \"%s\" to \"%s\"..."),
				(LPCTSTR)m_sFilename, (LPCTSTR)strTarget );

			// Using very long filenames
			if ( MoveFileWithProgress( CString( _T("\\\\?\\") ) + m_sFilename,
				CString( _T("\\\\?\\") ) + strTarget, CopyProgressRoutine, this,
				MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH ) )
			{
				m_dwFileError = 0;

				MarkFileAsDownload( strTarget );

				while( !Uploads.OnRename( m_sFilename, strTarget ) )
					Sleep( 250ul );

				m_bSuccess	= TRUE;
				m_sFilename	= strTarget;
				return;
			}
			m_dwFileError = GetLastError();

			// Moving failed, so we allow uploads using the old filename for the time being
			while( !Uploads.OnRename( m_sFilename, m_sFilename ) )
				Sleep( 250ul );

			if ( m_dwFileError == ERROR_REQUEST_ABORTED || m_pEvent != NULL )
				// Aborted
				return;

			theApp.Message( MSG_DEBUG, _T("Moving \"%s\" failed with error: %s"),
				(LPCTSTR)m_sFilename, (LPCTSTR)GetErrorString( m_dwFileError ) );

			if ( m_dwFileError == ERROR_NOT_READY ||
				 m_dwFileError == ERROR_HANDLE_DISK_FULL )
				// Try next path
				break;
		}
	}
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
// CDownloadTask torrent copy

void CDownloadTask::RunCopyTorrent()
{
	ASSERT( m_pTorrent.IsAvailable() );
	ASSERT( m_pTorrent.GetCount() > 1 );

	// Check for space before copying torrent
	if ( ! Downloads.IsSpaceAvailable( m_pTorrent.m_nTotalSize, Downloads.dlPathComplete ) )
	{
		m_dwFileError = ERROR_HANDLE_DISK_FULL;
		return;
	}

	HANDLE hSource = CreateFile( m_sFilename, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	m_dwFileError = GetLastError();
	VERIFY_FILE_ACCESS( hSource, m_sFilename )
	if ( hSource == INVALID_HANDLE_VALUE ) return;

	QWORD nOffset = 0;

	for ( ; m_posTorrentFile ; )
	{
		CBTInfo::CBTFile* pFile = m_pTorrent.m_pFiles.GetNext( m_posTorrentFile );

		DWORD nOffsetLow	= (DWORD)( nOffset & 0x00000000FFFFFFFF );
		DWORD nOffsetHigh	= (DWORD)( ( nOffset & 0xFFFFFFFF00000000 ) >> 32 );
		SetFilePointer( hSource, nOffsetLow, (PLONG)&nOffsetHigh, FILE_BEGIN );
		nOffset += pFile->m_nSize;

		CString strPath;
		strPath.Format( _T("%s\\%s"), (LPCTSTR)m_sPath, (LPCTSTR)pFile->m_sPath );
		CreatePathForFile( m_sPath, pFile->m_sPath );

		// Do nothing if it was an empty folder
		if ( pFile->m_sPath.Right( 1 ) == L"\\" ) continue;

		theApp.Message( MSG_DEBUG, _T("Extracting %s..."), (LPCTSTR)strPath );

		if ( ! CopyFile( hSource, strPath, pFile->m_nSize ) )
		{
			// try to copy in place
			strPath.Format( _T("%s\\%s"), LPCTSTR( m_sFilename.Left(
				m_sFilename.ReverseFind( '\\' ) ) ), LPCTSTR( pFile->m_sPath ) );
			theApp.Message( MSG_DEBUG, _T("Extraction failed, trying %s..."),
				LPCTSTR( strPath ) );
			CreatePathForFile( m_sFilename.Left( m_sFilename.ReverseFind( '\\' ) ),
				pFile->m_sPath );

			if ( ! CopyFile( hSource, strPath, pFile->m_nSize ) )
			{
				CloseHandle( hSource );
				return;
			}
		}

		MarkFileAsDownload( strPath );

		if ( m_pEvent != NULL )
		{
			CloseHandle( hSource );
			return;
		}
	}

	CloseHandle( hSource );
	m_bSuccess = TRUE;
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
		m_bSuccess = TRUE;
		return;
	}

	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	if ( ! Downloads.Check( m_pDownload ) ||
		  m_pDownload->IsCompleted() ||
		  m_pDownload->IsMoving() ||
		! m_pDownload->PrepareFile() )
	{
		// Download almost completed
		pLock.Unlock();
		CloseHandle( hSource );
		m_bSuccess = TRUE;
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
		m_bSuccess = TRUE;
		return;
	}

	const Fragments::List oList( m_pDownload->GetEmptyFragmentList() );
	if ( ! oList.size() )
	{
		// No available fragments
		pLock.Unlock();
		CloseHandle( hSource );
		m_bSuccess = TRUE;
		return;
	}

	QWORD qwSourceOffset = 0;
	if ( m_pDownload->IsTorrent() && ! m_pDownload->IsSingleFileTorrent() )
	{
		CString sFilename( PathFindFileName( m_sMergeFilename ) );
		QWORD qwOffset = 0;
		for ( POSITION pos = m_pTorrent.m_pFiles.GetHeadPosition() ; pos ; )
		{
			CBTInfo::CBTFile* pFile = m_pTorrent.m_pFiles.GetNext( pos );
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
			m_pDownload->RunValidation(FALSE);
		}
	}

	CloseHandle( hSource );
	m_bSuccess = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask copy file

BOOL CDownloadTask::CopyFile(HANDLE hSource, LPCTSTR pszTarget, QWORD nLength)
{
	HANDLE hTarget = CreateFile( pszTarget, GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	m_dwFileError = GetLastError();
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
			m_dwFileError = GetLastError();
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
	if ( nLength > 0 ) DeleteFile( pszTarget );

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

BOOL CDownloadTask::MakeBatchTorrent()
{
	QWORD nOffset = 0;
	QWORD nTotal = 0;
	ASSERT( m_pDownload->m_pFile == NULL );

	for ( POSITION pos = m_pDownload->m_pTorrent.m_pFiles.GetHeadPosition() ; pos ; )
	{
		CBTInfo::CBTFile* pFile = m_pDownload->m_pTorrent.m_pFiles.GetNext( pos );
		nTotal += pFile->m_nSize;
	}

	m_pDownload->m_pFile = new CFragmentedFile();
	m_pDownload->m_pFile->Create( m_pDownload->m_sPath, nTotal );
	bool bMissingFile = false;

	for ( POSITION pos = m_pDownload->m_pTorrent.m_pFiles.GetHeadPosition() ; pos ; )
	{
		CBTInfo::CBTFile* pFile = m_pDownload->m_pTorrent.m_pFiles.GetNext( pos );
		CString strSource = m_pDownload->FindTorrentFile( pFile );
		HANDLE hSource = INVALID_HANDLE_VALUE;

		if ( strSource.GetLength() > 0 )
		{
			hSource = CreateFile( strSource, GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_DELETE,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			VERIFY_FILE_ACCESS( hSource, strSource )
		}

		if ( hSource == INVALID_HANDLE_VALUE )
		{
			//CString strFormat;
			//LoadString(strFormat, IDS_BT_SEED_SOURCE_LOST );
			//m_sMessage.Format( strFormat, (LPCTSTR)pFile->m_sPath );
			if ( Settings.Experimental.TestBTPartials )
			{
				bMissingFile = true;
				nOffset += pFile->m_nSize;
				continue;
			}
			else
				return FALSE;
		}

		DWORD nSizeHigh	= 0;
		DWORD nSizeLow	= GetFileSize( hSource, &nSizeHigh );
		QWORD nSize		= (QWORD)nSizeLow + ( (QWORD)nSizeHigh << 32 );

		if ( nSize != pFile->m_nSize )
		{
			CloseHandle( hSource );
			//  m_sMessage.Format( IDS_BT_SEED_SOURCE_SIZE,
			//	pFile->m_sPath,
			//	Settings.SmartVolume( pFile->m_nSize ),
			//	Settings.SmartVolume( nSize ) );
			if ( Settings.Experimental.TestBTPartials )
			{
				bMissingFile = true;
				nOffset += pFile->m_nSize;
				continue;
			}
			else
				return FALSE;
		}

		BOOL bSuccess = CopyFileToBatch( hSource, nOffset, pFile->m_nSize, pFile->m_sPath );

		CloseHandle( hSource );

		if ( ! bSuccess )
			return FALSE;
		else
			nOffset += pFile->m_nSize;
	}

	if ( bMissingFile )
	{
		m_pDownload->ClearVerification();
		m_pDownload->m_bSeeding = FALSE;
		m_pDownload->m_bComplete = FALSE;
	}

	return TRUE;
}

BOOL CDownloadTask::CopyFileToBatch(HANDLE hSource, QWORD nOffset, QWORD nLength, LPCTSTR /*pszPath*/)
{
	auto_array< BYTE > pBuffer( new BYTE[ BUFFER_SIZE ] );

	while ( nLength )
	{
		DWORD nBuffer	= (DWORD)min( nLength, BUFFER_SIZE );
		DWORD tStart	= GetTickCount();

		if ( ! ReadFile( hSource, pBuffer.get(), nBuffer, &nBuffer, NULL ) ||
			 ! m_pDownload->m_pFile->WriteRange( nOffset, pBuffer.get(), nBuffer ) )
		{
			return FALSE;
		}

		nOffset += nBuffer;
		nLength -= nBuffer;

		tStart = ( GetTickCount() - tStart ) / 2;
		Sleep( min( tStart, 50ul ) );
	};

	if ( nLength == 0 )
	{
		return TRUE;
	}
	else
	{
		// CString strFormat;
		// LoadString(strFormat, IDS_BT_SEED_COPY_FAIL );
		// m_sMessage.Format( strFormat, (LPCTSTR)pszPath );
		return FALSE;
	}
}
