//
// TorrentBuilder.cpp
//
// Copyright (c) Shareaza Pty. Ltd., 2003.
// This file is part of TorrentAid Torrent Wizard (www.torrentaid.com).
//
// TorrentAid Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// TorrentAid is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TorrentAid; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "StdAfx.h"
#include "TorrentWizard.h"
#include "TorrentBuilder.h"
#include "Buffer.h"
#include "BENode.h"
#include "SHA1.h"
#include "ED2K.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTorrentBuilder, CWinThread)

BEGIN_MESSAGE_MAP(CTorrentBuilder, CWinThread)
	//{{AFX_MSG_MAP(CTorrentBuilder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTorrentBuilder construction

CTorrentBuilder::CTorrentBuilder()
{
	m_bAutoDelete	= FALSE;
	m_bActive		= FALSE;
	m_bFinished		= FALSE;
	m_bAbort		= FALSE;
	
	m_nTotalSize	= 0;
	m_nPieceSize	= 0x40000;
	m_nBuffer		= m_nPieceSize;
	m_pBuffer		= new BYTE[ m_nBuffer ];
}

CTorrentBuilder::~CTorrentBuilder()
{
	Stop();
	delete [] m_pBuffer;
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
	
	for ( int nAttempt = 5 ; nAttempt > 0 ; nAttempt-- )
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
		if ( m_pPieceSHA1 ) delete [] m_pPieceSHA1;
		if ( m_pFileED2K ) delete [] m_pFileED2K;
		if ( m_pFileSHA1 ) delete [] m_pFileSHA1;
		if ( m_pFileSize ) delete [] m_pFileSize;
		
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
	m_sThisFile = _T(" Prescanning files...");
	m_pSection.Unlock();
	
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
	m_pSection.Unlock();
	
	return m_bAbort == FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentBuilder run : process files

BOOL CTorrentBuilder::ProcessFiles()
{
	m_phPieceSHA1	= new CSHA1();
	m_phFullSHA1	= new CSHA1();
	m_phFileSHA1	= new CSHA1();
	m_phFullED2K	= new CED2K();
	m_phFileED2K	= new CED2K();
	
	m_nPieceUsed	= 0;
	m_nPiecePos		= 0;
	m_nPieceCount	= (DWORD)( ( m_nTotalSize + (QWORD)m_nPieceSize - 1 ) / (QWORD)m_nPieceSize );
	m_pPieceSHA1	= new CHashSHA1[ m_nPieceCount ];
	m_pFileSHA1		= new CHashSHA1[ m_pFiles.GetCount() ];
	m_pFileED2K		= new CHashMD4[ m_pFiles.GetCount() ];
	
	m_phFullSHA1->Reset();
	m_phFullED2K->Reset();
	
	m_phPieceSHA1->Reset();
	
	int nFile = 0;
	
	for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos && ! m_bAbort ; nFile++ )
	{
		CString strFile = m_pFiles.GetNext( pos );
		
		m_phFileSHA1->Reset();
		m_phFileED2K->Reset();
		
		if ( ! ProcessFile( strFile ) )
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
		
		m_phFileSHA1->Finish();
		m_pFileSHA1[ nFile ] = *m_phFileSHA1;

		m_phFileED2K->Finish();
		m_pFileED2K[ nFile ] = *m_phFileED2K;
	}
	
	if ( m_nPieceUsed > 0 )
	{
		m_phPieceSHA1->Finish();
		m_pPieceSHA1[ m_nPiecePos++ ] = *m_phPieceSHA1;
	}
	
	m_phFullSHA1->Finish();
	m_pDataSHA1 = *m_phFullSHA1;
	
	m_phFullED2K->Finish();
	m_pDataED2K = *m_phFullED2K;
	
	delete m_phPieceSHA1;
	delete m_phFullSHA1;
	delete m_phFileSHA1;
	delete m_phFullED2K;
	delete m_phFileED2K;
	
	return m_bAbort == FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentBuilder run : process a single file

BOOL CTorrentBuilder::ProcessFile(LPCTSTR pszFile)
{
	m_pSection.Lock();
	m_sThisFile = pszFile;
	m_pSection.Unlock();
	
	HANDLE hFile = CreateFile( pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;
	
	DWORD nLow, nHigh;
	nLow = GetFileSize( hFile, &nHigh );
	QWORD nSize = ( (QWORD)nHigh << 32 ) + (QWORD)nLow;
	
	while ( nSize > 0 && ! m_bAbort )
	{
		DWORD nLimit	= min( m_nBuffer, m_nPieceSize - m_nPieceUsed );
		DWORD nRead		= ( nSize > (QWORD)nLimit ) ? nLimit : (DWORD)nSize;
		
		ReadFile( hFile, m_pBuffer, nRead, &nRead, NULL );
		if ( nRead == 0 ) break;
		
		nSize -= (QWORD)nRead;
		m_nTotalPos += (QWORD)nRead;
		
		m_phPieceSHA1->Add( m_pBuffer, nRead );
		m_nPieceUsed += nRead;
		
		m_phFullSHA1->Add( m_pBuffer, nRead );
		m_phFileSHA1->Add( m_pBuffer, nRead );
		m_phFullED2K->Add( m_pBuffer, nRead );
		m_phFileED2K->Add( m_pBuffer, nRead );
		
		if ( m_nPieceUsed >= m_nPieceSize )
		{
			m_phPieceSHA1->Finish();
			m_pPieceSHA1[ m_nPiecePos++ ] = *m_phPieceSHA1;
			m_phPieceSHA1->Reset();
			m_nPieceUsed = 0;
		}
	}
	
	CloseHandle( hFile );
	
	return nSize == 0;
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
	
	CBENode* pDate = pRoot.Add( "creation date" );
	pDate->SetInt( (QWORD)time( NULL ) );
	
	CBENode* pInfo = pRoot.Add( "info" );
	
	CBENode* pPL = pInfo->Add( "piece length" );
	pPL->SetInt( m_nPieceSize );
	
	CBENode* pPieces = pInfo->Add( "pieces" );
	pPieces->SetString( m_pPieceSHA1, m_nPieceCount * sizeof CHashSHA1 );
	
	CBENode* pSHA1 = pInfo->Add( "sha1" );
	pSHA1->SetString( &m_pDataSHA1, sizeof CHashSHA1 );
	
	CBENode* pED2K = pInfo->Add( "ed2k" );
	pED2K->SetString( &m_pDataED2K, sizeof CHashMD4 );
	
	CString strFirst = m_pFiles.GetHead();
	
	if ( m_pFiles.GetCount() == 1 )
	{
		int nPos = strFirst.ReverseFind( '\\' );
		if ( nPos >= 0 ) strFirst = strFirst.Mid( nPos + 1 );
		
		CBENode* pName = pInfo->Add( "name" );
		pName->SetString( strFirst );
		
		CBENode* pLength = pInfo->Add( "length" );
		pLength->SetInt( m_nTotalSize );
	}
	else
	{
		CBENode* pName = pInfo->Add( "name" );
		pName->SetString( m_sName );
		
		CBENode* pFiles = pInfo->Add( "files" );
		int nCommonPath = 32000;
		int nFile = 0;
		
		for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos ; nFile++ )
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
		
		for ( pos = m_pFiles.GetHeadPosition(), nFile = 0 ; pos ; nFile++ )
		{
			CString strFile = m_pFiles.GetNext( pos );
			CBENode* pFile = pFiles->Add( NULL, NULL );
			
			CBENode* pLength = pFile->Add( "length" );
			pLength->SetInt( m_pFileSize[ nFile ] );
			
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
			
			pSHA1 = pFile->Add( "sha1" );
			pSHA1->SetString( &m_pFileSHA1[ nFile ], sizeof CHashSHA1 );
			
			pED2K = pFile->Add( "ed2k" );
			pED2K->SetString( &m_pFileED2K[ nFile ], sizeof CHashMD4 );
		}
	}
	
	CBENode* pAgent = pRoot.Add( "user-agent" );
	pAgent->SetString( _T("TorrentAid ") + theApp.m_sVersion );
	
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
