//
// DCClient.cpp
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
#include "HostCache.h"
#include "Library.h"
#include "SharedFolder.h"
#include "LibraryFolders.h"
#include "Network.h"
#include "Neighbour.h"
#include "Neighbours.h"
#include "GProfile.h"
#include "Settings.h"
#include "Statistics.h"
#include "Uploads.h"
#include "UploadFile.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "UploadTransfer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define FILE_NOT_AVAILABLE	_P("$Error File Not Available|")
#define CANNOT_UPLOAD		_P("$MaxedOut|")

CDCClient::CDCClient()
{
	m_mInput.pLimit = &Settings.Bandwidth.Request;
	m_mOutput.pLimit = &Settings.Bandwidth.Request;
}

CDCClient::~CDCClient()
{
}

BOOL CDCClient::ConnectTo(const IN_ADDR* pAddress, WORD nPort)
{
	CString sHost( inet_ntoa( *pAddress ) );

	if ( __super::ConnectTo( pAddress, nPort ) )
	{
		WSAEventSelect( m_hSocket, Network.GetWakeupEvent(),
			FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE );

		theApp.Message( MSG_INFO, IDS_CONNECTION_ATTEMPTING,
			(LPCTSTR)sHost, htons( m_pHost.sin_port ) );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_CONNECTION_CONNECT_FAIL,
			(LPCTSTR)sHost );
		return FALSE;
	}

	m_nState = upsConnecting;

	return TRUE;
}

BOOL CDCClient::OnConnected()
{
	if ( ! __super::OnConnected() )
		return FALSE;

	m_nState = upsRequest;

	Write( _P("$MyNick ") );
	Write( m_sNick );
	Write( _P("|") );

	std::string sLock = GenerateLock();

	Write( _P("$Lock ") );
	Write( sLock.c_str(), sLock.size() );
	Write( _P(" Pk=" CLIENT_NAME "|") );

	LogOutgoing();

	return TRUE;
}

void CDCClient::OnDropped()
{
	if ( m_nState == upsUploading && m_pBaseFile )
	{
		m_pBaseFile->AddFragment( m_nOffset, m_nPosition );
		m_pBaseFile = NULL;
	}

	if ( m_nState == upsConnecting )
	{
		Close( IDS_CONNECTION_REFUSED );
	}
	else
	{
		Close( IDS_UPLOAD_DROPPED );
	}
}

BOOL CDCClient::OnWrite()
{
	if ( GetOutputLength() == 0 )
	{
		// No data left to transfer
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
				Write( pBuffer.get(), (DWORD)nRead );

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
	}

	return __super::OnWrite();
}

BOOL CDCClient::OnCommand(const std::string& strCommand, const std::string& strParams)
{
	if ( strCommand == "$Direction" )
	{
		// $Direction (Download|Upload) Number

		std::string::size_type nPos = strParams.find( ' ' );
		if ( nPos == std::string::npos )
			// Invalid command
			return FALSE;
		std::string strDirection = strParams.substr( 0, nPos );
		if ( strDirection == "Download" )
			m_bDownload = TRUE;
		else if ( strDirection == "Upload" )
			m_bDownload = FALSE;
		else
			// Invalid command
			return FALSE;
		int nNumber = atoi( strParams.substr( nPos + 1 ).c_str() );
		if ( nNumber > 0x7FFF )
			// Invalid command
			return FALSE;

		return TRUE;
	}
	else if ( strCommand == "$ADCGET" )
	{
		// $ADCGET (list|file|tthl) Filename Offset Length [ZL1]

		std::string::size_type nPos1 = strParams.find( ' ' );
		if ( nPos1 == std::string::npos )
			// Invalid command
			return FALSE;
		std::string strType = strParams.substr( 0, nPos1 );
		std::string::size_type nPos2 = strParams.find( ' ', nPos1 + 1 );
		if ( nPos2 == std::string::npos )
			// Invalid command
			return FALSE;
		std::string strFilename = strParams.substr( nPos1 + 1, nPos2 - nPos1 - 1 );
		std::string::size_type nPos3 = strParams.find( ' ', nPos2 + 1 );
		if ( nPos3 == std::string::npos )
			// Invalid command
			return FALSE;
		std::string strOffset = strParams.substr( nPos2 + 1, nPos3 - nPos2 - 1 );
		std::string::size_type nPos4 = strParams.find( ' ', nPos3 + 1 );
		std::string strLength, strOptions;
		if ( nPos4 == std::string::npos )
		{
			strLength = strParams.substr( nPos3 + 1 );
		}
		else
		{
			strLength = strParams.substr( nPos3 + 1, nPos4 - nPos3 - 1 );
			strOptions = strParams.substr( nPos4 + 1 );
		}
		QWORD nOffset;
		if ( sscanf_s( strOffset.c_str(), "%I64u", &nOffset ) != 1 )
			// Invalid command
			return FALSE;
		QWORD nLength;
		if ( sscanf_s( strLength.c_str(), "%I64d", &nLength ) != 1 )
			// Invalid command
			return FALSE;

		return OnGet( strType, strFilename, nOffset, nLength, strOptions );
	}
	else if ( strCommand == "$Get" ||
		strCommand == "$GetZBlock" ||
		strCommand == "$UGetBlock" ||
		strCommand == "$UGetZBlock" ||
		strCommand == "$GetListLen" )
	{
		Write( _P("$Error Command not supported. Use ADCGET/ADCSND instead|") );
	}
	else
	{
		Write( _P("$Error Unknown command|") );
	}

	LogOutgoing();

	return TRUE;
}

BOOL CDCClient::OnChat(const std::string& strMessage)
{
	return TRUE;
}

BOOL CDCClient::OnGet(const std::string& strType, const std::string& strFilename, QWORD nOffset, QWORD nLength, const std::string& strOptions)
{
	ClearRequest();

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
			if ( RequestFileList( strFilename, nOffset, nLength ) )
			{
				return TRUE;
			}
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
		// Library listing
	}
	else
	{
		// Invalid request type
		return FALSE;
	}

	Write( FILE_NOT_AVAILABLE );

	LogOutgoing();

	theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND,
		(LPCTSTR)m_sAddress, (LPCTSTR)CA2CT( strFilename.c_str() ) );

	return TRUE;
}

BOOL CDCClient::RequestFileList(const std::string& strFilename, QWORD nOffset, QWORD nLength)
{
	m_sName = strFilename.c_str();

	// Create library file list
	CBuffer pXML;
	LibraryToFileList( pXML );

	// BZip it
	if ( strFilename == "files.xml.bz2" )
	{
		if ( ! pXML.BZip() )
		{
			// Out of memory
			return FALSE;
		}
	}

	// TODO: Implement partial request of file list
	nOffset, nLength;
	m_nOffset = 0;
	m_nLength = pXML.m_nLength;

	CString sAnswer;
	sAnswer.Format( _T("$ADCSND file %hs %I64u %I64u|"),
		strFilename.c_str(), m_nOffset, m_nLength );
	Write( sAnswer );
	
	LogOutgoing();

	Write( &pXML );

	StartSending( upsBrowse );

	theApp.Message( MSG_NOTICE, IDS_UPLOAD_BROWSE,
		(LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );

	return TRUE;
}

BOOL CDCClient::RequestTigerTree(const std::string& strFilename, QWORD nOffset, QWORD nLength, CLibraryFile* pFile)
{
	m_sName = pFile->m_sName;

	auto_ptr< CTigerTree > pTigerTree( pFile->GetTigerTree() );
	BYTE* pSerialTree = NULL;
	DWORD nSerialTree = 0;
	if ( ! pTigerTree.get() || ! pTigerTree->ToBytes( &pSerialTree, &nSerialTree ) )
	{
		ClearHashes();
		return FALSE;
	}

	// TODO: Find out and fix TigerTree hashes placement in output buffer
	nSerialTree = 24; // Root hash only

	m_nOffset = nOffset;
	if ( m_nOffset >= nSerialTree )
		m_nLength = SIZE_UNKNOWN;
	else
		m_nLength = min( ( ( nLength == SIZE_UNKNOWN ) ? nSerialTree : nLength ), nSerialTree - m_nOffset );

	if ( m_nLength > nSerialTree || m_nOffset + m_nLength > nSerialTree )
	{
		GlobalFree( pSerialTree );

		ClearHashes();

		theApp.Message( MSG_ERROR, IDS_UPLOAD_BAD_RANGE,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_sName );

		return FALSE;
	}

	CString sAnswer;
	sAnswer.Format( _T("$ADCSND tthl %hs %I64u %I64u|"),
		strFilename.c_str(), m_nOffset, m_nLength );
	Write( sAnswer );

	LogOutgoing();

	Write( pSerialTree + m_nOffset, m_nLength );

	GlobalFree( pSerialTree );

	StartSending( upsTigerTree );

	theApp.Message( MSG_INFO, IDS_UPLOAD_TIGER_SEND,
		(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );

	return TRUE;
}

BOOL CDCClient::RequestFile(const std::string& strFilename, QWORD nOffset, QWORD nLength, CLibraryFile* pFile)
{
	RequestComplete( pFile );

	if ( ! UploadQueues.CanUpload( PROTOCOL_DC, pFile, FALSE ) )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_sName );

		Write( CANNOT_UPLOAD );
			
		LogOutgoing();

		OnWrite();

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
	
		ClearHashes();

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

		Write( _P("$Queued") );
		Write( strAnswer );
		Write( _P("|") );

		LogOutgoing();

		StartSending( upsPreQueue );
	}
	else
	{ 
		theApp.Message( MSG_ERROR, nError,
			(LPCTSTR)m_sName, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );

		Write( CANNOT_UPLOAD );

		LogOutgoing();

		OnWrite();
	}

	return TRUE;
}

BOOL CDCClient::SendFile(const std::string& strFilename)
{
	if ( ! IsFileOpen() && ! OpenFile() )
	{
		Write( FILE_NOT_AVAILABLE );
		
		LogOutgoing();

		theApp.Message( MSG_ERROR, IDS_UPLOAD_CANTOPEN,
			(LPCTSTR)m_sName , (LPCTSTR)m_sAddress);

		return FALSE;
	}

	/*{ TODO: DC++ not support download chunk size change by uploader
		CSingleLock pLock( &UploadQueues.m_pSection, TRUE );
		
		if ( m_pQueue && UploadQueues.Check( m_pQueue ) && m_pQueue->m_bRotate )
		{
			DWORD nLimit = m_pQueue->m_nRotateChunk;
			if ( nLimit == 0 ) nLimit = Settings.Uploads.RotateChunkLimit;
			if ( nLimit > 0 ) m_nLength = min( m_nLength, nLimit );
		}
	}*/

	CString sAnswer;
	sAnswer.Format( _T("$ADCSND file %hs %I64u %I64u|"),
		strFilename.c_str(), m_nOffset, m_nLength );
	Write( sAnswer );

	LogOutgoing();

	m_mOutput.pLimit = &m_nBandwidth;

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

BOOL CDCClient::OnKey()
{
	if ( m_bExtended )
	{
		Write( _P("$Supports MiniSlots XmlBZList ADCGet TTHL TTHF |") );
	}

	if ( m_bDownload )
		Write( _P("$Direction Upload 0|") );
	else
		Write( _P("$Direction Download 0|") );

	Write( _P("$Key ") );
	Write( m_strKey.c_str(), m_strKey.size() );
	Write( _P("|") );

	LogOutgoing();

	return TRUE;
}

void CDCClient::LibraryToFileList(CBuffer& pXML)
{
	CSingleLock oLock( &Library.m_pSection, TRUE );
			
	CString strFileListing;
	strFileListing.Format( _T("<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\r\n")
		_T("<FileListing Version=\"1\" CID=\"%s\" Base=\"/\" Generator=\"%s\">\r\n"),
		_T("SKCB4ZF4PZUDF7RKQ5LX6SVAARQER7QEVELZ2TY"), theApp.m_sSmartAgent );
	pXML.Print( strFileListing, CP_UTF8 );

	for ( POSITION pos = LibraryFolders.GetFolderIterator() ; pos ; )
	{
		FolderToFileList( LibraryFolders.GetNextFolder( pos ), pXML );
	}

	pXML.Add( _P("</FileListing>\r\n") );
}

void CDCClient::FolderToFileList(CLibraryFolder* pFolder, CBuffer& pXML)
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

void CDCClient::FileToFileList(CLibraryFile* pFile, CBuffer& pXML)
{
	if ( ! pFile || ! pFile->IsShared() )
		return;

	CString strFile;
	strFile.Format( _T("<File Name=\"%s\" Size=\"%I64u\" TTH=\"%s\"/>\r\n"),
		pFile->m_sName, pFile->m_nSize, pFile->m_oTiger.toString() );
	pXML.Print( strFile, CP_UTF8 );
}
