//
// UploadTransferDC.cpp 
//
// Copyright (c) Shareaza Development Team, 2010.
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
#include "DCClient.h"
#include "DCClients.h"
#include "Library.h"
#include "SharedFolder.h"
#include "LibraryFolders.h"
#include "UploadTransfer.h"
#include "UploadTransferDC.h"
#include "Settings.h"
#include "Statistics.h"
#include "Transfers.h"
#include "Uploads.h"
#include "UploadFile.h"
#include "UploadQueue.h"
#include "UploadQueues.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CUploadTransferDC::CUploadTransferDC(CDCClient* pClient)
	: CUploadTransfer	( PROTOCOL_DC )
	, m_pClient			( pClient )
{
	ASSERT( pClient != NULL );
}

CUploadTransferDC::~CUploadTransferDC()
{
	ASSERT( m_pClient == NULL );
}

void CUploadTransferDC::Close(UINT nError)
{
	if ( m_pClient != NULL )
	{
		m_pClient->m_pUploadTransfer = NULL;
		m_pClient = NULL;
	}

	CUploadTransfer::Close( nError );
}

DWORD CUploadTransferDC::GetMeasuredSpeed()
{
	if ( m_pClient == NULL )
		return 0;

	m_pClient->MeasureOut();

	return m_pClient->m_mOutput.nMeasure;
}

BOOL CUploadTransferDC::OnRun()
{
	DWORD tNow = GetTickCount();

	if ( m_nState >= upsUploading )
	{
		if ( tNow > m_mOutput.tLast &&
			 tNow - m_mOutput.tLast > Settings.Connection.TimeoutTraffic )
		{
			Close( IDS_UPLOAD_TRAFFIC_TIMEOUT );
			return FALSE;
		}
	}

	return CUploadTransfer::OnRun();
}

BOOL CUploadTransferDC::OnWrite()
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( m_pClient );

	m_pClient->m_mOutput.pLimit = &m_nBandwidth;
	m_mOutput.tLast = m_pClient->m_mOutput.tLast;

	if ( m_pClient->GetOutputLength() != 0 )
		// There is data in output buffer
		return TRUE;

	if ( m_nState == upsUploading )
	{
		// No file data left to transfer
		if ( m_nPosition >= m_nLength )
		{
			// File completed

			Uploads.SetStable( GetAverageSpeed() );

			m_nState = upsRequest;

			m_pBaseFile->AddFragment( m_nOffset, m_nLength );

			theApp.Message( MSG_INFO, IDS_UPLOAD_FINISHED,
				(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );
		}
		else
		{
			// Reading next data chunk of file
			QWORD nToRead = min( m_nLength - m_nPosition, 256 * 1024ull );
			QWORD nRead = 0;
			auto_array< BYTE > pBuffer( new BYTE[ nToRead ] );
			if ( ! ReadFile( m_nFileBase + m_nOffset + m_nPosition,
				pBuffer.get(), nToRead, &nRead ) || nToRead != nRead )
			{
				// File error
				return FALSE;
			}
			m_pClient->Write( pBuffer.get(), (DWORD)nRead );

			m_nPosition += nRead;
			m_nUploaded += nRead;

			Statistics.Current.Uploads.Volume += ( nRead / 1024 );
		}
	}
	else if ( m_nState >= upsResponse )
	{
		// Current transfer completed

		m_nState = ( m_nState == upsPreQueue ) ? upsQueued : upsRequest;
	}

	return TRUE;
}

BOOL CUploadTransferDC::OnUpload(const std::string& strType, const std::string& strFilename, QWORD nOffset, QWORD nLength, const std::string& strOptions)
{
	ASSERT( m_nState < upsUploading );
	ASSERT( m_pClient );

	ClearRequest();

	m_sNick = m_pClient->m_sRemoteNick;
	m_sUserAgent = m_pClient->m_sUserAgent;
	m_pHost = m_pClient->m_pHost;
	m_sAddress = m_pClient->m_sAddress;
	UpdateCountry();

	m_pClient->m_mOutput.pLimit = &m_nBandwidth;

	BOOL bZip = ( strOptions.find("ZL1") != std::string::npos );

	if ( strType == "tthl" )
	{
		if ( strFilename.substr( 0, 4 ) != "TTH/" )
		{
			// Invalid hash prefix
			return FALSE;
		}

		Hashes::TigerHash oTiger;
		if ( ! oTiger.fromString( CA2W( strFilename.substr( 4 ).c_str() ) ) )
		{
			// Invalid TigerTree hash encoding
			return FALSE;
		}

		CSingleLock oLock( &Library.m_pSection );
		if ( oLock.Lock( 1000 ) )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByTiger( oTiger, TRUE, TRUE ) )
			{
				if ( RequestTigerTree( strFilename, nOffset, nLength, pFile ) )
				{
					return TRUE;
				}
			}
		}
	}
	else if ( strType == "file" )
	{
		if ( strFilename == "files.xml" || strFilename == "files.xml.bz2" )
		{
			if ( RequestFileList( TRUE, bZip, strFilename, nOffset, nLength ) )
				return TRUE;
		}
		else if ( strFilename.substr( 0, 4 ) == "TTH/" )
		{
			Hashes::TigerHash oTiger;
			if ( ! oTiger.fromString( CA2W( strFilename.substr( 4 ).c_str() ) ) )
			{
				// Invalid TigerTree hash encoding
				return FALSE;
			}

			CSingleLock oLock( &Library.m_pSection );
			if ( oLock.Lock( 1000 ) )
			{
				if ( CLibraryFile* pFile = LibraryMaps.LookupFileByTiger( oTiger, TRUE, TRUE ) )
				{
					if ( RequestFile( strFilename, nOffset, nLength, pFile ) )
					{
						return TRUE;
					}
				}
			}
		}
	}
	else if ( strType == "list" )
	{
		if ( RequestFileList( FALSE, bZip, strFilename, nOffset, nLength ) )
			return TRUE;
	}
	else
	{
		// Invalid request type
		return FALSE;
	}

	theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND,
		(LPCTSTR)m_sAddress, (LPCTSTR)CA2CT( strFilename.c_str() ) );

	m_pClient->Write( FILE_NOT_AVAILABLE );
	m_pClient->LogOutgoing();

	return TRUE;
}

BOOL CUploadTransferDC::RequestFile(const std::string& strFilename, QWORD nOffset, QWORD nLength, CLibraryFile* pFile)
{
	if ( ! RequestComplete( pFile ) )
	{
		ASSERT( FALSE );
		return FALSE;
	}

	if ( ! UploadQueues.CanUpload( PROTOCOL_DC, pFile, FALSE ) )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_sName );

		m_pClient->Write( CANNOT_UPLOAD );	
		m_pClient->LogOutgoing();

		return TRUE;
	}

	m_nOffset = nOffset;
	if ( m_nOffset >= m_nSize )
		m_nLength = SIZE_UNKNOWN;
	else
		m_nLength = min( ( ( nLength == SIZE_UNKNOWN ) ? m_nSize : nLength ), m_nSize - m_nOffset );

	if ( m_nLength > m_nSize || m_nOffset + m_nLength > m_nSize )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BAD_RANGE,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_sName );

		return FALSE;
	}

	AllocateBaseFile();

	UINT nError = 0;
	int nPosition = 0;

	if ( m_bStopTransfer )
	{
		m_tRotateTime = 0;
		m_bStopTransfer	= FALSE;

		CUploadQueue* pQueue = m_pQueue;
		if ( pQueue )
			pQueue->Dequeue( this );
	}

	if ( Uploads.CanUploadFileTo( &m_pHost.sin_addr, this ) )
	{
		nPosition = UploadQueues.GetPosition( this, TRUE );
		if ( nPosition == 0 )
		{
			// Queued, and ready to send
			return SendFile( strFilename );
		}
		else if ( nPosition > 0 )
		{
			// Queued, but must wait
			nError = IDS_UPLOAD_BUSY_OLD;
		}
		else if ( UploadQueues.Enqueue( this ) )
		{
			nPosition = UploadQueues.GetPosition( this, TRUE );
			if ( nPosition == 0 )
			{
				// Queued, and ready to send
				return SendFile( strFilename );
			}
			else if ( nPosition > 0 )
			{
				// Queued, but must wait
				nError = IDS_UPLOAD_BUSY_OLD;
			}
			else
				// Client can't queue, so dequeue and return busy
				nError = IDS_UPLOAD_BUSY_OLD;
		}
		else
			// Unable to queue anywhere
			nError = IDS_UPLOAD_BUSY_QUEUE;
	}
	else
		// Too many from this host
		nError = IDS_UPLOAD_BUSY_HOST;

	if ( nError )
	{
		UploadQueues.Dequeue( this );
		ASSERT( m_pQueue == NULL );
	}
		
	if ( m_pQueue )
	{
		CString strAnswer;
		DWORD nReaskMultiplier = ( nPosition <= 9 ) ? ( ( nPosition + 1 ) / 2 ) : 5;
		DWORD nTimeScale = 1000 / nReaskMultiplier;
		
		CSingleLock pLock( &UploadQueues.m_pSection, TRUE );
		if ( UploadQueues.Check( m_pQueue ) )
		{
			CString strName = m_pQueue->m_sName;
			strName.Replace( _T("\""), _T("'") );

			strAnswer.Format( _T(" position=%i,length=%i,limit=%i,pollMin=%lu,pollMax=%lu,id=\"%s\""),
				nPosition,
				m_pQueue->GetQueuedCount(),
				m_pQueue->GetTransferCount( TRUE ),
				Settings.Uploads.QueuePollMin / nTimeScale,
				Settings.Uploads.QueuePollMax / nTimeScale,
				(LPCTSTR)strName );

			theApp.Message( MSG_INFO, IDS_UPLOAD_QUEUED, (LPCTSTR)m_sName,
				(LPCTSTR)m_sAddress, nPosition, m_pQueue->GetQueuedCount(),
				(LPCTSTR)strName );
		}

		m_pClient->Write( _P("$Queued") );
		m_pClient->Write( strAnswer );
		m_pClient->Write( _P("|") );
		m_pClient->LogOutgoing();

		StartSending( upsPreQueue );
	}
	else
	{ 
		theApp.Message( MSG_ERROR, nError,
			(LPCTSTR)m_sName, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );

		m_pClient->Write( CANNOT_UPLOAD );
		m_pClient->LogOutgoing();
	}

	return TRUE;
}

BOOL CUploadTransferDC::SendFile(const std::string& strFilename)
{
	if ( ! IsFileOpen() && ! OpenFile() )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_CANTOPEN,
			(LPCTSTR)m_sName , (LPCTSTR)m_sAddress);

		m_pClient->Write( FILE_NOT_AVAILABLE );
		m_pClient->LogOutgoing();

		return FALSE;
	}

	CString sAnswer;
	sAnswer.Format( _T("$ADCSND file %hs %I64u %I64u|"),
		strFilename.c_str(), m_nOffset, m_nLength );
	m_pClient->Write( sAnswer );
	m_pClient->LogOutgoing();
	StartSending( upsUploading );

	if ( m_pBaseFile->m_nRequests++ == 0 )
	{
		theApp.Message( MSG_NOTICE, IDS_UPLOAD_FILE,
			(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );
		
		ASSERT( m_pBaseFile->m_sPath.GetLength() );
		PostMainWndMessage( WM_NOWUPLOADING, 0, (LPARAM)new CString( m_pBaseFile->m_sPath ) );
	}
	
	theApp.Message( MSG_INFO, IDS_UPLOAD_CONTENT,
		m_nOffset, m_nOffset + m_nLength - 1, (LPCTSTR)m_sName,
		(LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );

	return TRUE;
}

BOOL CUploadTransferDC::RequestFileList(BOOL bFile, BOOL bZip, const std::string& strFilename, QWORD nOffset, QWORD nLength)
{
	theApp.Message( MSG_NOTICE, IDS_UPLOAD_BROWSE,
		(LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );

	BOOL bBZip = bFile && ( strFilename == "files.xml.bz2" );
	CString strFile( bFile ? "/" : strFilename.c_str() );

	// Create library file list
	CBuffer pXML;
	LibraryToFileList( strFile, pXML );

	// TODO: Implement partial request of file list
	nOffset = 0;
	nLength = pXML.m_nLength;

	if ( bBZip )
	{
		// BZip it
		if ( ! pXML.BZip() )
		{
			// Out of memory
			return FALSE;
		}
		bZip = FALSE;
		nLength = pXML.m_nLength;
	}

	if ( bZip )
	{
		// Zip it
		if ( ! pXML.Deflate() )
		{
			// Out of memory
			return FALSE;
		}
	}

	CString sAnswer;
	sAnswer.Format( _T("$ADCSND %hs %hs %I64u %I64u%hs|"),
		( bFile ? "file" : "list" ), strFilename.c_str(),
		nOffset, nLength, ( bZip ? " ZL1" : "") );
	m_pClient->Write( sAnswer );
	m_pClient->LogOutgoing();

	m_pClient->Write( &pXML );

	// Start upload
	m_nOffset = nOffset;
	m_nLength = pXML.m_nLength;
	m_sName = strFile;
	StartSending( upsBrowse );

	return TRUE;
}

BOOL CUploadTransferDC::RequestTigerTree(const std::string& strFilename, QWORD nOffset, QWORD nLength, CLibraryFile* pFile)
{
	theApp.Message( MSG_INFO, IDS_UPLOAD_TIGER_SEND,
		(LPCTSTR)pFile->m_sName, (LPCTSTR)m_sAddress );

	auto_ptr< CTigerTree > pTigerTree( pFile->GetTigerTree() );
	BYTE* pSerialTree = NULL;
	DWORD nSerialTree = 0;
	if ( ! pTigerTree.get() || ! pTigerTree->ToBytes( &pSerialTree, &nSerialTree ) )
	{
		return FALSE;
	}

	// TODO: Find out and fix TigerTree hashes placement in output buffer
	nSerialTree = 24; // Root hash only

	if ( nOffset >= nSerialTree )
		nLength = SIZE_UNKNOWN;
	else
		nLength = min( ( ( nLength == SIZE_UNKNOWN ) ? nSerialTree : nLength ), nSerialTree - nOffset );

	if ( nLength > nSerialTree || nOffset + nLength > nSerialTree )
	{
		GlobalFree( pSerialTree );
		return FALSE;
	}

	CString sAnswer;
	sAnswer.Format( _T("$ADCSND tthl %hs %I64u %I64u|"),
		strFilename.c_str(), nOffset, nLength );
	m_pClient->Write( sAnswer );
	m_pClient->LogOutgoing();
	
	m_pClient->Write( pSerialTree + nOffset, nLength );

	// Start uploading
	m_nOffset = nOffset;
	m_nLength = nLength;
	m_sName = pFile->m_sName;

	StartSending( upsTigerTree );

	GlobalFree( pSerialTree );

	return TRUE;
}

void CUploadTransferDC::LibraryToFileList(const CString& strRoot, CBuffer& pXML)
{
	CSingleLock oLock( &Library.m_pSection, TRUE );
			
	CString strFileListing;
	strFileListing.Format( _T("<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\r\n")
		_T("<FileListing Version=\"1\" Base=\"%s\" Generator=\"%s\">\r\n"),
		strRoot, theApp.m_sSmartAgent );
	pXML.Print( strFileListing, CP_UTF8 );

	if ( strRoot == _T("/") )
	{
		// All folders
		for ( POSITION pos = LibraryFolders.GetFolderIterator() ; pos ; )
		{
			FolderToFileList( LibraryFolders.GetNextFolder( pos ), pXML );
		}
	}
	else if ( CLibraryFolder* pFolder = LibraryFolders.GetFolderByName( strRoot ) )
	{
		// Specified folder
		FolderToFileList( pFolder, pXML );
	}

	pXML.Add( _P("</FileListing>\r\n") );
}

void CUploadTransferDC::FolderToFileList(CLibraryFolder* pFolder, CBuffer& pXML)
{
	if ( ! pFolder || ! pFolder->IsShared() )
		return;

	CString strFolder;
	strFolder.Format( _T("<Directory Name=\"%s\">\r\n"),
		pFolder->m_sName );
	pXML.Print( strFolder, CP_UTF8 );

	for ( POSITION pos = pFolder->GetFolderIterator() ; pos ; )
	{
		FolderToFileList( pFolder->GetNextFolder( pos ), pXML );
	}

	for ( POSITION pos = pFolder->GetFileIterator() ; pos ; )
	{
		FileToFileList( pFolder->GetNextFile( pos ), pXML );
	}

	pXML.Add( _P("</Directory>\r\n") );
}

void CUploadTransferDC::FileToFileList(CLibraryFile* pFile, CBuffer& pXML)
{
	if ( ! pFile || ! pFile->IsShared() )
		return;

	CString strFile;
	strFile.Format( _T("<File Name=\"%s\" Size=\"%I64u\" TTH=\"%s\"/>\r\n"),
		pFile->m_sName, pFile->m_nSize, pFile->m_oTiger.toString() );
	pXML.Print( strFile, CP_UTF8 );
}
