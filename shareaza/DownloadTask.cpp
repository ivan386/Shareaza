//
// DownloadTask.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "LibraryFolders.h"

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

#define METHOD_BUFFERED				0x00000000
#define FILE_DEVICE_FILE_SYSTEM		0x00000009
#define CTL_CODE(DeviceType,Function,Method,Access) (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#define FSCTL_SET_SPARSE CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 49, METHOD_BUFFERED, FILE_WRITE_DATA)
#define BUFFER_SIZE					(2*1024*1024)


/////////////////////////////////////////////////////////////////////////////
// CDownloadTask construction

CDownloadTask::CDownloadTask(CDownload* pDownload, int nTask)
{
	ASSERT( pDownload->m_pTask == NULL );
	pDownload->m_pTask = this;
	
	m_nTask		= nTask;
	m_pDownload	= pDownload;
	m_bSuccess	= FALSE;
	m_nTorrentFile = 0;
	m_nSize		= pDownload->m_nSize;
	m_sName		= pDownload->m_sRemoteName;
	m_sFilename	= pDownload->m_sLocalName;
	m_sPath		= DownloadGroups.GetCompletedPath( pDownload );

	int nExt = m_sFilename.ReverseFind( '.' );
	if ( nExt >= 2 )
	{
		CString sExtention = m_sFilename.Mid( nExt );
		CharLower( sExtention.GetBuffer() );
		sExtention.ReleaseBuffer();

		if( ( sExtention == ".collection" ) || ( sExtention == ".co" ) )
		{
			m_sPath	= Settings.Downloads.CollectionPath;
			CreateDirectory( Settings.Downloads.CollectionPath, NULL );
			LibraryFolders.AddFolder( Settings.Downloads.CollectionPath );
		}
		else if( sExtention == ".torrent" )
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
	
	m_pEvent = NULL;
	m_bAutoDelete = TRUE;
	
	CreateThread();
	SetThreadPriority( THREAD_PRIORITY_BELOW_NORMAL );
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
	}
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask allocate

void CDownloadTask::RunAllocate()
{
	HANDLE hFile = CreateFile( m_sFilename, GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL );
	
	if ( hFile == INVALID_HANDLE_VALUE ) return;
	
	if ( GetFileSize( hFile, NULL ) != 0 )
	{
		CloseHandle( hFile );
		return;
	}
	
	DWORD dwOut = 0;
	DeviceIoControl( hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwOut, NULL );
	
	if ( m_nSize > 0 )
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
	
	int nExt = m_sName.ReverseFind( '.' );
	
	CString strName( nExt > 0 ? m_sName.Left( nExt ) : m_sName );
	CString strExt(  nExt > 0 ? m_sName.Mid(  nExt ) : _T( "" ) );

	{
		CTransfers::Lock oLock;

		Uploads.OnRename( m_sFilename );
		
		TCHAR szOpFrom[MAX_PATH], szOpTo[MAX_PATH];
		SHFILEOPSTRUCT pOp;
		
		ZeroMemory( &pOp, sizeof(pOp) );
		pOp.wFunc		= FO_MOVE;
		pOp.pFrom		= szOpFrom;
		pOp.pTo			= szOpTo;
		pOp.fFlags		= FOF_MULTIDESTFILES|FOF_NOERRORUI|FOF_SILENT;
		
		ZeroMemory( szOpFrom, sizeof(TCHAR) * MAX_PATH );
		ZeroMemory( szOpTo, sizeof(TCHAR) * MAX_PATH );
		_tcsncpy( szOpFrom, m_sFilename, MAX_PATH - 2 );
		
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
		
		Uploads.OnRename( m_sFilename, m_sFilename );
	}
	
	HANDLE hSource = CreateFile( m_sFilename, GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	
	if ( hSource == INVALID_HANDLE_VALUE ) return;
	
	strTarget.Format( _T("%s\\%s%s"),
		(LPCTSTR)m_sPath, (LPCTSTR)strName, (LPCTSTR)strExt );
	
	theApp.Message( MSG_DEBUG, _T("Copying \"%s\" to \"%s\"..."),
			(LPCTSTR)m_sFilename, (LPCTSTR)strTarget );
	
	m_bSuccess = CopyFile( hSource, strTarget, m_nSize );

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

		CTransfers::Lock oLock;

		if ( SHFileOperation( &pOp ) == 0 )
		{
			Uploads.OnRename( m_sFilename, strTarget );
			m_bSuccess	= TRUE;
			m_sFilename	= strTarget;
			return;
		}
	}
	
	if ( m_bSuccess )
	{
		{
			CTransfers::Lock oLock;

			Uploads.OnRename( m_sFilename, NULL );
			Uploads.OnRename( m_sFilename, strTarget );
		}
		
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

/////////////////////////////////////////////////////////////////////////////
// CDownloadTask copy file

BOOL CDownloadTask::CopyFile(HANDLE hSource, LPCTSTR pszTarget, QWORD nLength)
{
	HANDLE hTarget = CreateFile( pszTarget, GENERIC_WRITE,
		0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );
	
	if ( hTarget == INVALID_HANDLE_VALUE ) return FALSE;
	
	BYTE* pBuffer = new BYTE[ BUFFER_SIZE ];
	
	while ( nLength )
	{
		DWORD nBuffer	= (DWORD)min( nLength, QWORD(BUFFER_SIZE) );
		DWORD nSuccess	= 0;
		DWORD tStart	= GetTickCount();
		
		ReadFile( hSource, pBuffer, nBuffer, &nBuffer, NULL );
		WriteFile( hTarget, pBuffer, nBuffer, &nSuccess, NULL );
		
		if ( nSuccess == nBuffer )
		{
			nLength -= nBuffer;
		}
		else
		{
			break;
		}
		
		if ( m_pEvent != NULL ) break;
		tStart = ( GetTickCount() - tStart ) / 2;
		Sleep( min( tStart, DWORD(50) ) );
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
	
	if ( LPCTSTR pszExt = _tcsrchr( strName, '.' ) )
	{
		if ( _tcsicmp( pszExt, _T(".sd") ) == 0 )
			strName += _T("x");
	}
	
	if ( strName.GetLength() > 210 )
		strName = strName.Left( 200 ) + strName.Right( 4 );
	
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
