//
// TorrentBuilder.cpp
//
// Copyright (c) Shareaza Development Team, 2007-2010.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "StdAfx.h"
#include "TorrentWizard.h"
#include "TorrentBuilder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTorrentBuilder, CWinThread)

BEGIN_MESSAGE_MAP(CTorrentBuilder, CWinThread)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTorrentBuilder construction

CTorrentBuilder::CTorrentBuilder()
: m_bActive( FALSE )
, m_bFinished ( FALSE )
, m_bAbort( FALSE )
, m_nTotalSize( 0 )
, m_nTotalPos( 0 )
, m_bSHA1( FALSE )
, m_bED2K( FALSE )
, m_bMD5( FALSE )
, m_pFileSize( NULL )
, m_pFileSHA1( NULL )
, m_pFileED2K( NULL )
, m_pFileMD5( NULL )
, m_pPieceSHA1( NULL )
, m_nPieceSize( 0 )
, m_nPieceCount( 0 )
, m_nPiecePos( 0 )
, m_nPieceUsed( 0 )
, m_bAutoPieces( TRUE )
, m_pBuffer( NULL )
, m_nBuffer( 0 )
{
	m_bAutoDelete = FALSE;
}

CTorrentBuilder::~CTorrentBuilder()
{
	Stop();
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentBuilder setup operations

BOOL CTorrentBuilder::SetName(LPCTSTR pszName)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_bActive ) return FALSE;
	m_sName = pszName;
	return TRUE;
}

void CTorrentBuilder::SetPieceSize(int nPieceIndex)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( nPieceIndex == -1 )
		m_nPieceSize = 0;
	else
		m_nPieceSize = 1 << ( nPieceIndex + 15 );
}

void CTorrentBuilder::Enable(BOOL bSHA1, BOOL bED2K, BOOL bMD5)
{
	m_bSHA1 = bSHA1;
	m_bED2K = bED2K;
	m_bMD5 = bMD5;
}

BOOL CTorrentBuilder::SetOutputFile(LPCTSTR pszPath)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_bActive ) return FALSE;
	
	m_sOutput = pszPath;
	
	if ( m_sName.IsEmpty() )
	{
		if ( LPCTSTR pszSlash = _tcsrchr( pszPath, '\\' ) )
		{
			m_sName = pszSlash + 1;
			
			int nPos = m_sName.ReverseFind( '.' );
			if ( nPos >= 0 ) m_sName = m_sName.Left( nPos );
			nPos = m_sName.ReverseFind( '.' );
			if ( nPos >= 0 ) m_sName = m_sName.Left( nPos );
		}
	}
	
	return TRUE;
}

BOOL CTorrentBuilder::AddFile(LPCTSTR pszPath)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_bActive ) return FALSE;
	m_pFiles.AddTail( pszPath );
	return TRUE;
}

BOOL CTorrentBuilder::AddTrackerURL(LPCTSTR pszURL)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_bActive ) return FALSE;
	m_sTracker = pszURL;
	return TRUE;
}

BOOL CTorrentBuilder::SetComment(LPCTSTR pszComment)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_bActive ) return FALSE;
	m_sComment = pszComment;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentBuilder run operations

BOOL CTorrentBuilder::Start()
{
	if ( m_hThread != NULL ) Stop();
	
	if ( m_sName.IsEmpty() || m_sOutput.IsEmpty() ) return FALSE;
	if ( m_pFiles.GetCount() == 0 ) return FALSE;
	
	m_bActive	= TRUE;
	m_bFinished	= FALSE;
	m_bAbort	= FALSE;
	m_sMessage.Empty();
	
	CreateThread();
	
	return TRUE;
}

void CTorrentBuilder::Stop()
{
	if ( m_hThread == NULL ) return;
	m_bAbort = TRUE;
	
	int nAttempt = 5;
	for ( nAttempt ; nAttempt > 0 ; nAttempt-- )
	{
		DWORD nCode = 0;
		if ( ! GetExitCodeThread( m_hThread, &nCode ) || nCode != STILL_ACTIVE ) break;
		Sleep( 1000 );
	}
	
	if ( nAttempt <= 0 )
	{
		TerminateThread( m_hThread, 1 );
	}
	
	m_hThread	= NULL;
	m_bActive	= FALSE;
	m_bFinished	= FALSE;
	m_bAbort	= FALSE;

	if ( m_pBuffer != NULL ) {
		delete [] m_pBuffer;
		m_pBuffer = NULL;
	}
}

BOOL CTorrentBuilder::SetPriority(int nPriority)
{
	if ( m_hThread != NULL )
	{
		SetThreadPriority( nPriority );
		return TRUE;
	}
	
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentBuilder state operations

BOOL CTorrentBuilder::IsRunning()
{
	CSingleLock pLock( &m_pSection, TRUE );
	return m_bActive;
}

BOOL CTorrentBuilder::IsFinished()
{
	CSingleLock pLock( &m_pSection, TRUE );
	return ! m_bActive && m_bFinished;
}

BOOL CTorrentBuilder::GetTotalProgress(DWORD& nPosition, DWORD& nScale)
{
	CSingleLock pLock( &m_pSection, TRUE );
	// if ( ! m_bActive ) return FALSE;
	nPosition	= (DWORD)( (double)(__int64)m_nTotalPos / (double)(__int64)m_nTotalSize * 26000.0f );
	nScale		= (DWORD)26000;
	return TRUE;
}

BOOL CTorrentBuilder::GetCurrentFile(CString& strFile)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( ! m_bActive ) return FALSE;
	strFile = m_sThisFile;
	return TRUE;
}

BOOL CTorrentBuilder::GetMessageString(CString& strMessage)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_sMessage.IsEmpty() ) return FALSE;
	strMessage = m_sMessage;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentBuilder run

int CTorrentBuilder::Run() 
{
	if ( m_pSection.Lock() )
	{
		m_nTotalSize	= 0;
		m_nTotalPos		= 0;
		m_pFileSize		= NULL;
		m_pFileSHA1		= NULL;
		m_pFileED2K		= NULL;
		m_pFileMD5		= NULL;
		m_pPieceSHA1	= NULL;
		m_pSection.Unlock();
	}
	
	if ( ScanFiles() && ! m_bAbort )
	{
		if ( ProcessFiles() )
		{
			if ( WriteOutput() )
			{
				m_bFinished = TRUE;
			}
		}
	}
	
	if ( m_pSection.Lock() )
	{
		delete [] m_pPieceSHA1;
		m_pPieceSHA1	= NULL;
		delete [] m_pFileMD5;
		m_pFileMD5		= NULL;
		delete [] m_pFileED2K;
		m_pFileED2K		= NULL;
		delete [] m_pFileSHA1;
		m_pFileSHA1		= NULL;
		delete [] m_pFileSize;
		m_pFileSize		= NULL;
		
		m_sThisFile.Empty();
		m_bActive = FALSE;
		
		m_pSection.Unlock();
	}
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentBuilder run : scan files

BOOL CTorrentBuilder::ScanFiles()
{
	m_pSection.Lock();
	m_nTotalSize = 0;
	if ( m_pBuffer != NULL )
		delete [] m_pBuffer;
	m_sThisFile = _T(" Prescanning files...");
	m_pSection.Unlock();
	
	delete [] m_pFileSize;
	m_pFileSize = new QWORD[ m_pFiles.GetCount() ];
	int nFile = 0;
	
	for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos && ! m_bAbort ; nFile++ )
	{
		CString strFile = m_pFiles.GetNext( pos );
		
		HANDLE hFile = CreateFile( strFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
		
		if ( hFile == INVALID_HANDLE_VALUE )
		{
			m_pSection.Lock();
			CString strFormat;
			strFormat.LoadString( IDS_BUILDER_CANT_OPEN );
			m_sMessage.Format( strFormat, (LPCTSTR)strFile );
			m_bAbort = TRUE;
			m_pSection.Unlock();
			break;
		}
		
		DWORD nLow, nHigh;
		nLow = GetFileSize( hFile, &nHigh );
		CloseHandle( hFile );
		
		QWORD nSize = ( (QWORD)nHigh << 32 ) + (QWORD)nLow;
		m_pFileSize[ nFile ] = nSize;
		m_nTotalSize += nSize;
	}
	
	m_pSection.Lock();
	m_sThisFile.Empty();

	if ( m_nPieceSize == 0 )
	{
		m_nPieceSize = 1;
		QWORD nCompare = 1 << 20;
		if ( m_nTotalSize <= 50 * nCompare )
			m_nPieceSize <<= 15;
		else if ( m_nTotalSize <= 150i64 * nCompare )
			m_nPieceSize <<= 16;
		else if ( m_nTotalSize <= 350i64 * nCompare )
			m_nPieceSize <<= 17;
		else if ( m_nTotalSize <= 512i64 * nCompare )
			m_nPieceSize <<= 18;
		else if ( m_nTotalSize <= 1024i64 * nCompare )
			m_nPieceSize <<= 19;
		else if ( m_nTotalSize <= 2048i64 * nCompare )
			m_nPieceSize <<= 20;
		else
			m_nPieceSize <<= 21;
	}

	m_nBuffer = m_nPieceSize;
	m_pBuffer = new BYTE[ m_nBuffer ];
	m_pSection.Unlock();
	
	return m_bAbort == FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentBuilder run : process files

BOOL CTorrentBuilder::ProcessFiles()
{
	if ( m_bSHA1 ) m_oDataSHA1.Reset();
	if ( m_bED2K ) m_oDataED2K.BeginFile( m_nTotalSize );
	if ( m_bMD5 ) m_oDataMD5.Reset();
	m_oPieceSHA1.Reset();

	m_nPieceUsed	= 0;
	m_nPiecePos		= 0;
	m_nPieceCount	= (DWORD)( ( m_nTotalSize + (QWORD)m_nPieceSize - 1 ) / (QWORD)m_nPieceSize );

	
	delete [] m_pPieceSHA1;
	m_pPieceSHA1	= new CSHA[ m_nPieceCount ];

	delete [] m_pFileSHA1;
	m_pFileSHA1 = NULL;
	if ( m_bSHA1 ) m_pFileSHA1	= new CSHA[ m_pFiles.GetCount() ];

	delete [] m_pFileED2K;
	m_pFileED2K = NULL;
	if ( m_bED2K ) m_pFileED2K	= new CED2K[ m_pFiles.GetCount() ];
	
	delete [] m_pFileMD5;
	m_pFileMD5 = NULL;
	if ( m_bMD5 ) m_pFileMD5	= new CMD5[ m_pFiles.GetCount() ];

	int nFile = 0;
	
	for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos && ! m_bAbort ; nFile++ )
	{
		CString strFile = m_pFiles.GetNext( pos );

		if ( ! ProcessFile( nFile, strFile ) )
		{
			if ( m_bAbort ) break;
			m_pSection.Lock();
			CString strFormat;
			strFormat.LoadString( IDS_BUILDER_CANT_OPEN );
			m_sMessage.Format( strFormat, (LPCTSTR)strFile );
			m_bAbort = TRUE;
			m_pSection.Unlock();
			break;
		}
	}
	
	if ( m_nPieceUsed > 0 )
	{
		m_oPieceSHA1.Finish();
		m_pPieceSHA1[ m_nPiecePos++ ] = m_oPieceSHA1;
	}
	
	if ( m_bSHA1 ) m_oDataSHA1.Finish();
	if ( m_bED2K ) m_oDataED2K.FinishFile();
	if ( m_bMD5 ) m_oDataMD5.Finish();
	
	return ( m_bAbort == FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentBuilder run : process a single file

BOOL CTorrentBuilder::ProcessFile(DWORD nFile, LPCTSTR pszFile)
{
	m_pSection.Lock();
	m_sThisFile = pszFile;
	m_pSection.Unlock();
	
	HANDLE hFile = CreateFile( pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		return FALSE;
	
	DWORD nLow, nHigh;
	nLow = GetFileSize( hFile, &nHigh );
	QWORD nSize = ( (QWORD)nHigh << 32 ) + (QWORD)nLow;

	if ( m_bSHA1 ) m_pFileSHA1[ nFile ].Reset();
	if ( m_bED2K ) m_pFileED2K[ nFile ].BeginFile( nSize );
	if ( m_bMD5 ) m_pFileMD5[ nFile ].Reset();
	
	while ( nSize > 0 && ! m_bAbort )
	{
		DWORD nLimit	= min( m_nBuffer, m_nPieceSize - m_nPieceUsed );
		DWORD nRead		= ( nSize > (QWORD)nLimit ) ? nLimit : (DWORD)nSize;
		
		if ( ! ReadFile( hFile, m_pBuffer, nRead, &nRead, NULL ) || nRead == 0 )
			break;
		
		nSize -= (QWORD)nRead;
		m_nTotalPos += (QWORD)nRead;
		
		m_oPieceSHA1.Add( m_pBuffer, nRead );
		m_nPieceUsed += nRead;
		
		if ( m_bSHA1 )
		{
			m_oDataSHA1.Add( m_pBuffer, nRead );
			m_pFileSHA1[ nFile ].Add( m_pBuffer, nRead );
		}

		if ( m_bED2K )
		{
			m_oDataED2K.AddToFile( m_pBuffer, nRead );
			m_pFileED2K[ nFile ].AddToFile( m_pBuffer, nRead );
		}

		if ( m_bMD5 )
		{
			m_oDataMD5.Add( m_pBuffer, nRead );
			m_pFileMD5[ nFile ].Add( m_pBuffer, nRead );
		}

		if ( m_nPieceUsed >= m_nPieceSize )
		{
			m_oPieceSHA1.Finish();
			m_pPieceSHA1[ m_nPiecePos++ ] = m_oPieceSHA1;
			m_oPieceSHA1.Reset();
			m_nPieceUsed = 0;
		}
	}

	if ( m_bSHA1 ) m_pFileSHA1[ nFile ].Finish();
	if ( m_bED2K ) m_pFileED2K[ nFile ].FinishFile();
	if ( m_bMD5 ) m_pFileMD5[ nFile ].Finish();

	CloseHandle( hFile );
	
	return ( nSize == 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentBuilder run : write output

BOOL CTorrentBuilder::WriteOutput()
{
	CBENode pRoot;
	if ( m_sTracker.GetLength() > 0 )
	{
		CBENode* pAnnounce = pRoot.Add( "announce" );
		pAnnounce->SetString( m_sTracker );
	}
	{
		CBENode* pDate = pRoot.Add( "creation date" );
		pDate->SetInt( (QWORD)time( NULL ) );
	}
	CBENode* pInfo = pRoot.Add( "info" );	
	{
		CBENode* pPL = pInfo->Add( "piece length" );
		pPL->SetInt( m_nPieceSize );
	}
	{
		CSHA::Digest* pPieceSHA1 = new CSHA::Digest[ m_nPieceCount ];
		for ( DWORD i = 0 ; i < m_nPieceCount; ++i )
			m_pPieceSHA1[ i ].GetHash( (uchar*)&pPieceSHA1[ i ][ 0 ] );
		CBENode* pPieces = pInfo->Add( "pieces" );
		pPieces->SetString( pPieceSHA1, m_nPieceCount * sizeof CSHA::Digest );
		delete [] pPieceSHA1;
	}
	if ( m_bSHA1 )
	{
		CSHA::Digest pDataSHA1;
		m_oDataSHA1.GetHash( (uchar*)&pDataSHA1[ 0 ] );
		CBENode* pSHA1 = pInfo->Add( "sha1" );
		pSHA1->SetString( &pDataSHA1, sizeof CSHA::Digest );
	}
	if ( m_bED2K )
	{
		CMD4::Digest pDataED2K;
		m_oDataED2K.GetRoot( (uchar*)&pDataED2K[ 0 ] );
		CBENode* pED2K = pInfo->Add( "ed2k" );
		pED2K->SetString( &pDataED2K, sizeof CMD4::Digest );
	}
	if ( m_bMD5 )
	{
		CMD5::Digest pDataMD5;
		m_oDataMD5.GetHash( (uchar*)&pDataMD5[ 0 ] );
		CBENode* pMD5 = pInfo->Add( "md5sum" );
		pMD5->SetString( &pDataMD5, sizeof CMD5::Digest );
	}	
	CString strFirst = m_pFiles.GetHead();
	
	if ( m_pFiles.GetCount() == 1 )
	{
		int nPos = strFirst.ReverseFind( '\\' );
		if ( nPos >= 0 ) strFirst = strFirst.Mid( nPos + 1 );
		
		{
			CBENode* pName = pInfo->Add( "name" );
			pName->SetString( strFirst );
		}
		{
			CBENode* pLength = pInfo->Add( "length" );
			pLength->SetInt( m_nTotalSize );
		}
	}
	else
	{
		{
			CBENode* pName = pInfo->Add( "name" );
			pName->SetString( m_sName );
		}	
		CBENode* pFiles = pInfo->Add( "files" );
		int nCommonPath = 32000;
		int nFile = 0;		
		POSITION pos = m_pFiles.GetHeadPosition();
		for ( ; pos ; nFile++ )
		{
			CString strThis = m_pFiles.GetNext( pos );
			
			if ( nFile == 0 ) continue;
			
			LPCTSTR pszFirst	= strFirst;
			LPCTSTR pszThis		= strThis;
			
			for ( int nPos = 0, nSlash = 0 ; nPos < nCommonPath ; nPos++ )
			{
				if ( pszThis[nPos] != pszFirst[nPos] ||
					 pszThis[nPos] == 0 || pszFirst[nPos] == 0 )
				{
					nCommonPath = nSlash;
					break;
				}
				else if ( pszThis[nPos] == '\\' )
				{
					nSlash = nPos;
				}
			}
		}
		nCommonPath ++;
		pos = m_pFiles.GetHeadPosition();
		for ( nFile = 0 ; pos ; nFile++ )
		{
			CString strFile = m_pFiles.GetNext( pos );
			CBENode* pFile = pFiles->Add( NULL, NULL );
			{
				CBENode* pLength = pFile->Add( "length" );
				pLength->SetInt( m_pFileSize[ nFile ] );
			}
			{
				CBENode* pPath = pFile->Add( "path" );
				strFile = strFile.Mid( nCommonPath );			
				while ( strFile.GetLength() )
				{
					CString strPart = strFile.SpanExcluding( _T("\\/") );
					if ( strPart.IsEmpty() ) break;
					
					pPath->Add( NULL, NULL )->SetString( strPart );
					
					strFile = strFile.Mid( strPart.GetLength() );
					if ( strFile.GetLength() ) strFile = strFile.Mid( 1 );
				}
			}
			if ( m_bSHA1 )
			{
				CSHA::Digest pFileSHA1;
				m_pFileSHA1[ nFile ].GetHash( (uchar*)&pFileSHA1[ 0 ] );
				CBENode* pSHA1 = pFile->Add( "sha1" );
				pSHA1->SetString( &pFileSHA1, sizeof CSHA::Digest );
			}
			if ( m_bED2K )
			{
				CMD4::Digest pFileED2K;
				m_pFileED2K[ nFile ].GetRoot( (uchar*)&pFileED2K[ 0 ] );
				CBENode* pED2K = pFile->Add( "ed2k" );
				pED2K->SetString( &pFileED2K, sizeof CMD4::Digest );
			}
			if ( m_bMD5 )
			{
				CMD5::Digest pFileMD5;
				m_pFileMD5[ nFile ].GetHash( (uchar*)&pFileMD5[ 0 ] );
				CBENode* pMD5 = pFile->Add( "md5sum" );
				pMD5->SetString( &pFileMD5, sizeof CMD5::Digest );
			}
		}
	}
	{
		CBENode* pAgent = pRoot.Add( "created by" );
		CString strAgent = _T("TorrentWizard ") + theApp.m_sVersion;
		pAgent->SetString( strAgent );
	}	
	if ( m_sComment.GetLength() > 0 )
	{
		CBENode* pComment = pRoot.Add( "comment" );
		pComment->SetString( m_sComment );
	}
	
	CBuffer pOutput;
	pRoot.Encode( &pOutput );
	
	HANDLE hFile = CreateFile( m_sOutput, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL );
	
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		m_pSection.Lock();
		CString strFormat;
		strFormat.LoadString( IDS_BUILDER_CANT_SAVE );
		m_sMessage.Format( strFormat, (LPCTSTR)m_sOutput );
		m_bAbort = TRUE;
		m_pSection.Unlock();
		return FALSE;
	}
	
	DWORD nWrote = 0;
	WriteFile( hFile, pOutput.m_pBuffer, pOutput.m_nLength, &nWrote, NULL );
	CloseHandle( hFile );
	
	return TRUE;
}
