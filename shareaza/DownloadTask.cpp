//
// DownloadTask.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "DownloadGroups.h"
#include "Transfers.h"
#include "Uploads.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "LibraryMaps.h"
#include "SharedFile.h"
#include "HttpRequest.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CDownloadTask, CWinThread)

BEGIN_MESSAGE_MAP(CDownloadTask, CWinThread)
	//{{AFX_MSG_MAP(CDownloadTask)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//#define METHOD_BUFFERED				0x00000000
//#define FILE_DEVICE_FILE_SYSTEM		0x00000009
//#define CTL_CODE(DeviceType,Function,Method,Access) (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
//#define FSCTL_SET_SPARSE CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 49, METHOD_BUFFERED, FILE_WRITE_DATA)
const DWORD BUFFER_SIZE = 2 * 1024 * 1024u;


/////////////////////////////////////////////////////////////////////////////
// CDownloadTask construction

CDownloadTask::CDownloadTask(CDownload* pDownload, int nTask)
: m_nTask(nTask)
, m_pEvent(NULL)
, m_pDownload(pDownload)
, m_bSuccess(FALSE)
, m_nTorrentFile(0)
, m_hSelectedFile(INVALID_HANDLE_VALUE)
{
	Construct( pDownload );
	SetThreadPriority( THREAD_PRIORITY_BELOW_NORMAL );
	SetThreadName( m_nThreadID, "Download Task 1" );
}

CDownloadTask::CDownloadTask(CDownload* pDownload, const CString& strPreviewURL)
: m_nTask(dtaskPreviewRequest)
, m_pEvent(NULL)
, m_pDownload(pDownload)
, m_bSuccess(FALSE)
, m_nTorrentFile(0)
, m_hSelectedFile(INVALID_HANDLE_VALUE)
{
	m_pRequest.SetURL( strPreviewURL );
	m_pRequest.AddHeader( _T("Accept"), _T("image/jpeg") );
	m_pRequest.LimitContentLength( Settings.Search.MaxPreviewLength );
	Construct( pDownload );
	SetThreadPriority( THREAD_PRIORITY_NORMAL );
	SetThreadName( m_nThreadID, "Download Task 2" );
}

CDownloadTask::CDownloadTask(CDownload* pDownload, HANDLE hSelectedFile)
: m_nTask(dtaskMergeFile)
, m_pEvent(NULL)
, m_pDownload(pDownload)
, m_bSuccess(FALSE)
, m_nTorrentFile(0)
, m_hSelectedFile(hSelectedFile)
{
	Construct( pDownload );
	SetThreadPriority( THREAD_PRIORITY_NORMAL );
	SetThreadName( m_nThreadID, "Download Task 3" );
}

void CDownloadTask::Construct(CDownload* pDownload)
{
	ASSERT( pDownload->m_pTask == NULL );
	pDownload->m_pTask = this;

	CString strLocalName = pDownload->m_sDisplayName;

	m_nSize		= pDownload->m_nSize;
	m_sName		= pDownload->m_sDisplayName;
	m_sFilename	= pDownload->m_sDiskName;
	m_sPath		= DownloadGroups.GetCompletedPath( pDownload );

	int nExt = strLocalName.ReverseFind( '.' );
	if ( nExt >= 2 )
	{
		CString sExtention = strLocalName.Mid( nExt );
		ToLower( sExtention );

		if ( ( sExtention == ".collection" ) || ( sExtention == ".co" ) )
		{
			m_sPath	= Settings.Downloads.CollectionPath;
			CreateDirectory( Settings.Downloads.CollectionPath, NULL );
			LibraryFolders.AddFolder( Settings.Downloads.CollectionPath );
		}
		else if ( sExtention == ".torrent" )
		{
			m_sPath	= Settings.Downloads.TorrentPath;
			CreateDirectory( Settings.Downloads.TorrentPath, NULL );
			LibraryFolders.AddFolder( Settings.Downloads.TorrentPath, FALSE );
		}
	}


	if ( m_nTask == dtaskCopySimple && m_pDownload->m_pTorrent.m_nFiles > 1 )
	{
		m_nTask = dtaskCopyTorrent;
		m_pTorrent.Copy( &m_pDownload->m_pTorrent );
	}

	m_bAutoDelete = TRUE;
	CreateThread();
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
	if ( m_hSelectedFile != INVALID_HANDLE_VALUE ) CloseHandle( m_hSelectedFile );
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
	if ( m_hSelectedFile != INVALID_HANDLE_VALUE ) CloseHandle( m_hSelectedFile );
	m_hSelectedFile = INVALID_HANDLE_VALUE;
}

BOOL CDownloadTask::WasAborted()
{
	return m_pEvent != NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask run

BOOL CDownloadTask::InitInstance()
{
	return TRUE;
}

int CDownloadTask::Run() 
{
	switch ( m_nTask )
	{
	case dtaskAllocate:
		RunAllocate();
		break;
	case dtaskCopySimple:
		RunCopySimple();
		if ( m_bSuccess == FALSE && m_pEvent == NULL )
		{
			if ( m_sPath != Settings.Downloads.CompletePath )
			{
				m_sPath = Settings.Downloads.CompletePath;
				RunCopySimple();
			}
		}
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
	}
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask allocate

void CDownloadTask::RunAllocate()
{
	HANDLE hFile = CreateFile( m_sFilename, GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | ( theApp.m_bNT ? FILE_SHARE_DELETE : 0 ),
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	VERIFY_FILE_ACCESS( hFile, m_sFilename )
	if ( hFile == INVALID_HANDLE_VALUE ) return;
	
	if ( GetFileSize( hFile, NULL ) != 0 )
	{
		CloseHandle( hFile );
		return;
	}
	
	if ( Settings.Downloads.SparseThreshold > 0 && theApp.m_bNT &&
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
	CString strTarget;
	CString strSafeName = SafeFilename( m_sName );
	
	int nExt = strSafeName.ReverseFind( '.' );
	
	CString strName( nExt > 0 ? strSafeName.Left( nExt ) : strSafeName );
	CString strExt(  nExt > 0 ? strSafeName.Mid(  nExt ) : _T( "" ) );

	// Only try to move if the drive letters match else we copy always
	if ( m_sFilename[ 0 ] == m_sPath[ 0 ] && m_sFilename[ 0 ] != '\\' )
	{
		TCHAR szOpFrom[MAX_PATH] = { 0 };
		_tcsncpy( szOpFrom, m_sFilename, MAX_PATH - 2 );
		TCHAR szOpTo[MAX_PATH] = { 0 };
		SHFILEOPSTRUCT pOp = { 0 };
		
		pOp.wFunc  = FO_MOVE;
		pOp.pFrom  = szOpFrom;
		pOp.pTo    = szOpTo;
		pOp.fFlags = FOF_MULTIDESTFILES|FOF_NOERRORUI|FOF_SILENT;
		
		CQuickLock oLock( Transfers.m_pSection );

		// Disconnect all uploads for that file (i.e. close the file handle)
		Uploads.OnRename( m_sFilename );

		for ( int nCopy = 0 ; nCopy < 10 ; nCopy++ )
		{
			if ( nCopy )
			{
				strTarget.Format( _T("%s\\%s (%i)%s"),
					(LPCTSTR)m_sPath, (LPCTSTR)strName, nCopy, (LPCTSTR)strExt );
			}
			else
			{
				strTarget.Format( _T("%s\\%s%s"),
					(LPCTSTR)m_sPath, (LPCTSTR)strName, (LPCTSTR)strExt );
			}
			
			theApp.Message( MSG_DEBUG, _T("Moving \"%s\" to \"%s\"..."),
				(LPCTSTR)m_sFilename, (LPCTSTR)strTarget );
			
			_tcsncpy( szOpTo, strTarget, MAX_PATH - 2 );
			
			if ( GetFileAttributes( strTarget ) == 0xFFFFFFFF &&
				SHFileOperation( &pOp ) == 0 )
			{
				Uploads.OnRename( m_sFilename, strTarget );
				m_bSuccess	= TRUE;
				m_sFilename	= strTarget;
				return;
			}
		}
		
		// Moving failed, so we allow uploads using the old filename for the time being
		Uploads.OnRename( m_sFilename, m_sFilename );
	}
	
	// Moving failed or the destination drive differs from the source drive
	HANDLE hSource = CreateFile( m_sFilename, GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	VERIFY_FILE_ACCESS( hSource, m_sFilename )
	if ( hSource == INVALID_HANDLE_VALUE ) return;
	
	for ( int nCopy = 0; !m_bSuccess && nCopy < 10; ++nCopy )
	{
		if ( nCopy )
		{
			strTarget.Format( _T("%s\\%s (%i)%s"),
				(LPCTSTR)m_sPath, (LPCTSTR)strName, nCopy, (LPCTSTR)strExt );
		}
		else
		{
			strTarget.Format( _T("%s\\%s%s"),
				(LPCTSTR)m_sPath, (LPCTSTR)strName, (LPCTSTR)strExt );
		}
		
		theApp.Message( MSG_DEBUG, _T("Copying \"%s\" to \"%s\"..."),
			(LPCTSTR)m_sFilename, (LPCTSTR)strTarget );

		m_bSuccess = CopyFile( hSource, strTarget, m_nSize );
	}

	CloseHandle( hSource );

	if ( !m_bSuccess )
	{
		// rename in place
		strTarget.Format( _T( "%s\\%s%s" ), LPCTSTR( m_sFilename.Left( m_sFilename.ReverseFind( '\\' ) ) ),
			LPCTSTR( strName ), LPCTSTR( strExt ) );
		TCHAR szOpFrom[MAX_PATH] = { 0 };
		_tcsncpy( szOpFrom, m_sFilename, MAX_PATH - 2 );
		TCHAR szOpTo[MAX_PATH] = { 0 };
		_tcsncpy( szOpTo, strTarget, MAX_PATH - 2 );

		SHFILEOPSTRUCT pOp = { 0 };
		pOp.wFunc		= FO_MOVE;
		pOp.pFrom		= szOpFrom;
		pOp.pTo			= szOpTo;
		pOp.fFlags		= FOF_MULTIDESTFILES|FOF_NOERRORUI|FOF_SILENT;

		CQuickLock oLock( Transfers.m_pSection );

		Uploads.OnRename( m_sFilename );

		if ( SHFileOperation( &pOp ) == 0 )
		{
			Uploads.OnRename( m_sFilename, strTarget );
			m_bSuccess	= TRUE;
			m_sFilename	= strTarget;
			return;
		}

		Uploads.OnRename( m_sFilename, m_sFilename );
	}
	
	if ( m_bSuccess )
	{
		CQuickLock oLock( Transfers.m_pSection );
		Uploads.OnRename( m_sFilename, NULL );
		Uploads.OnRename( m_sFilename, strTarget );
		
		if ( ! DeleteFile( m_sFilename ) )
			theApp.WriteProfileString( _T("Delete"), m_sFilename, _T("") );
		m_sFilename = strTarget;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask torrent copy

void CDownloadTask::RunCopyTorrent()
{
	ASSERT( m_pTorrent.IsAvailable() );
	ASSERT( m_pTorrent.m_nFiles > 1 );
	
	HANDLE hSource = CreateFile( m_sFilename, GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	VERIFY_FILE_ACCESS( hSource, m_sFilename )
	if ( hSource == INVALID_HANDLE_VALUE ) return;

	m_bSuccess = FALSE;

	// Check for space before copying torrent
	if ( ! Downloads.IsSpaceAvailable( m_pTorrent.m_nTotalSize, Downloads.dlPathComplete ) ) return;
	
	QWORD nOffset = 0;
	
	for ( ; m_nTorrentFile < m_pTorrent.m_nFiles ; ++m_nTorrentFile )
	{
		const CBTInfo::CBTFile& rFile = m_pTorrent.m_pFiles[ m_nTorrentFile ];
		
		DWORD nOffsetLow	= (DWORD)( nOffset & 0x00000000FFFFFFFF );
		DWORD nOffsetHigh	= (DWORD)( ( nOffset & 0xFFFFFFFF00000000 ) >> 32 );
		SetFilePointer( hSource, nOffsetLow, (PLONG)&nOffsetHigh, FILE_BEGIN );
		nOffset += rFile.m_nSize;
		
		CString strPath;
		strPath.Format( _T("%s\\%s"), (LPCTSTR)m_sPath, (LPCTSTR)rFile.m_sPath );
		CreatePathForFile( m_sPath, rFile.m_sPath );
		
		// Do nothing if it was an empty folder
		if ( rFile.m_sPath.Right( 1 ) == L"\\" ) continue;

		theApp.Message( MSG_DEBUG, _T("Extracting %s..."), (LPCTSTR)strPath );
		
		if ( !CopyFile( hSource, strPath, rFile.m_nSize ) )
		{
			// try to copy in place
			strPath.Format( _T("%s\\%s"), LPCTSTR( m_sFilename.Left( m_sFilename.ReverseFind( '\\' ) ) ),
				LPCTSTR( rFile.m_sPath ) );
			theApp.Message( MSG_DEBUG, _T("Extraction failed, trying %s..."), LPCTSTR( strPath ) );
			CreatePathForFile( m_sFilename.Left( m_sFilename.ReverseFind( '\\' ) ), rFile.m_sPath );

			if ( !CopyFile( hSource, strPath, rFile.m_nSize ) )
			{
				CloseHandle( hSource );
				return; // try again later ( m_bSuccess is still FALSE )
			}
		}
		
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
	CString strMessage;
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	if ( ! Downloads.Check( m_pDownload ) ||
		m_pDownload->IsCompleted() ||
		m_pDownload->IsMoving() ||
		! m_pDownload->PrepareFile() )
	{
		// Download almost completed
		pLock.Unlock();
		CloseHandle( m_hSelectedFile );
		m_hSelectedFile = INVALID_HANDLE_VALUE;
		m_bSuccess = TRUE;
		return;
	}

	if ( m_pDownload->NeedTigerTree() &&
		 m_pDownload->NeedHashset() &&
		! m_pDownload->IsTorrent() )
	{
		// No hashsets
		pLock.Unlock();
		LoadString( strMessage, IDS_DOWNLOAD_EDIT_COMPLETE_NOHASH );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		CloseHandle( m_hSelectedFile );
		m_hSelectedFile = INVALID_HANDLE_VALUE;
		m_bSuccess = TRUE;
		return;
	}
	const Fragments::List oList( m_pDownload->GetEmptyFragmentList() );
	if ( ! oList.size() )
	{
		// No available fragments
		pLock.Unlock();
		CloseHandle( m_hSelectedFile );
		m_hSelectedFile = INVALID_HANDLE_VALUE;
		m_bSuccess = TRUE;
		return;
	}
	pLock.Unlock();

	// Read missing file fragments from selected file
	BYTE Buf [65536];
	DWORD dwToRead, dwReaded;
	for ( Fragments::List::const_iterator pFragment = oList.begin();
		pFragment != oList.end(); ++pFragment )
	{
		QWORD qwLength = pFragment->end() - pFragment->begin();
		QWORD qwOffset = pFragment->begin();
		LONG nOffsetHigh = (LONG)( qwOffset >> 32 );
		LONG nOffsetLow = (LONG)( qwOffset & 0xFFFFFFFF );
		SetFilePointer( m_hSelectedFile, nOffsetLow, &nOffsetHigh, FILE_BEGIN );
		if ( GetLastError() == NO_ERROR )
		{
			while ( ( dwToRead = (DWORD)min( qwLength, (QWORD)sizeof( Buf ) ) ) != 0 )
			{
				if ( ReadFile( m_hSelectedFile, Buf, dwToRead, &dwReaded, NULL ) && dwReaded != 0 )
				{
					pLock.Lock();
					m_pDownload->SubmitData( qwOffset, Buf, (QWORD) dwReaded );
					qwOffset += (QWORD) dwReaded;
					qwLength -= (QWORD) dwReaded;
					pLock.Unlock();
				}
				else
				{
					// File error or end of file. Not Fatal
					break;
				}
				Sleep( 1 );
				if ( m_hSelectedFile == INVALID_HANDLE_VALUE ) return;		// Aborting
			}
			m_pDownload->RunValidation(FALSE);
		}
	}

	CloseHandle( m_hSelectedFile );
	m_hSelectedFile = INVALID_HANDLE_VALUE;
	m_bSuccess = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask copy file

BOOL CDownloadTask::CopyFile(HANDLE hSource, LPCTSTR pszTarget, QWORD nLength)
{
	HANDLE hTarget = CreateFile( pszTarget, GENERIC_WRITE,
		0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );
	VERIFY_FILE_ACCESS( hTarget, pszTarget )
	if ( hTarget == INVALID_HANDLE_VALUE ) return FALSE;
	
	BYTE* pBuffer = new BYTE[ BUFFER_SIZE ];
	
	while ( nLength )
	{
		DWORD nBuffer	= min( nLength, BUFFER_SIZE );
		DWORD nSuccess	= 0;
		DWORD tStart	= GetTickCount();
		
		if ( !ReadFile( hSource, pBuffer, nBuffer, &nBuffer, NULL )
			|| !nBuffer
			|| !WriteFile( hTarget, pBuffer, nBuffer, &nSuccess, NULL )
			|| nSuccess != nBuffer ) break;
		
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
		{
			if ( theApp.m_bNT ) continue;
		}
		else
		{
			if ( IsCharacter( cChar ) ) continue;
			if ( _tcschr( pszValid, cChar ) != NULL ) continue;
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
	CreateDirectory( strBase, NULL );
	
	for ( int nPos = 0 ; nPos < strPath.GetLength() ; nPos++ )
	{
		if ( strPath.GetAt( nPos ) == '\\' )
		{
			CString strFolder = strBase + '\\' + strPath.Left( nPos );
			CreateDirectory( strFolder, NULL );
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
