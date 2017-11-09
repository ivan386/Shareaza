//
// UploadTransferDC.cpp 
//
// Copyright (c) Shareaza Development Team, 2010-2017.
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
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define FILE_NOT_AVAILABLE	_T("$Error File Not Available|")
#define UPLOAD_BUSY			_T("$MaxedOut|")
#define UPLOAD_QUEUE		_T("$MaxedOut %i|")


CUploadTransferDC::CUploadTransferDC(CDCClient* pClient)
	: CUploadTransfer	( PROTOCOL_DC )
	, m_pClient			( pClient )
	, m_tRankingCheck	( 0 )
	, m_bGet			( FALSE )
{
	ASSERT( pClient != NULL );

	m_pServer = pClient->m_pServer;
	m_sRemoteNick = pClient->m_sRemoteNick;
}

CUploadTransferDC::~CUploadTransferDC()
{
	ASSERT( m_pClient == NULL );
}

BOOL CUploadTransferDC::IsIdle() const
{
	return ( m_nState < upsUploading );
}

void CUploadTransferDC::Close(UINT nError)
{
	if ( CDCClient* pClient = m_pClient )
	{
		m_pClient = NULL;

		pClient->OnUploadClose();
	}

	Cleanup();

	CUploadTransfer::Close( nError );
}

void CUploadTransferDC::Cleanup(BOOL bDequeue)
{
	if ( bDequeue )
		UploadQueues.Dequeue( this );

	if ( m_nState == upsUploading )
	{
		if ( m_pBaseFile && m_nLength != SIZE_UNKNOWN )
			m_pBaseFile->AddFragment( m_nOffset, m_nPosition );

		CloseFile();
	}

	ClearRequest();

	m_pBaseFile	= NULL;

	m_nState = upsReady;
}

DWORD CUploadTransferDC::GetMeasuredSpeed()
{
	if ( m_pClient == NULL )
		return 0;

	m_pClient->MeasureOut();

	return m_pClient->m_mOutput.nMeasure;
}

void CUploadTransferDC::OnDropped()
{
	if ( m_nState == upsQueued )
	{
		theApp.Message( MSG_INFO, IDS_UPLOAD_QUEUE_DROP, (LPCTSTR)m_sAddress );
	}
	else
	{
		Close( IDS_UPLOAD_DROPPED );
	}
}

BOOL CUploadTransferDC::OnRun()
{
	if ( ! CUploadTransfer::OnRun() )
		return FALSE;

	DWORD tNow = GetTickCount();

	switch ( m_nState )
	{
	case upsReady:
	case upsRequest:
		if ( tNow > m_tRequest + Settings.Connection.TimeoutHandshake )
		{
			Close( IDS_UPLOAD_REQUEST_TIMEOUT );
			return FALSE;
		}
		break;

	case upsQueued:
		if ( tNow > m_tRequest + Settings.DC.DequeueTime )
		{
			Close( IDS_UPLOAD_QUEUE_TIMEOUT );
			return FALSE;
		}
		else if ( tNow > m_tRankingCheck + 5 * 1000 ) // Re-check every 5 seconds
		{
			return CheckRanking();
		}
		break;

	case upsUploading:
	case upsResponse:
	case upsBrowse:
	case upsTigerTree:
		if ( tNow > m_mOutput.tLast + Settings.Connection.TimeoutTraffic )
		{
			Close( IDS_UPLOAD_TRAFFIC_TIMEOUT );
			return FALSE;
		}
		break;

	default:
		;
	}

	return TRUE;
}

BOOL CUploadTransferDC::OnWrite()
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( m_pClient );

	m_mOutput.tLast = m_pClient->m_mOutput.tLast;

	if ( m_pClient->GetOutputLength() != 0 )
		// There is data in output buffer
		return TRUE;

	if ( m_nState == upsUploading )
	{
		ASSERT( m_nLength != SIZE_UNKNOWN );

		// No file data left to transfer
		if ( m_nPosition >= m_nLength )
		{
			// File completed

			Uploads.SetStable( GetAverageSpeed() );

			m_nState = upsRequest;
			m_tRequest = GetTickCount();

			m_pBaseFile->AddFragment( m_nOffset, m_nLength );

			theApp.Message( MSG_INFO, IDS_UPLOAD_FINISHED,
				(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );
		}
		else
		{
			// Reading next data chunk of file
			QWORD nToRead = min( m_nLength - m_nPosition, 1024000ull ); // 1000 KB
			QWORD nRead = 0;
			auto_array< BYTE > pBuffer( new BYTE[ (size_t)nToRead ] );
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
		m_nState	= ( m_nState == upsPreQueue ) ? upsQueued : upsRequest;
		m_tRequest	= GetTickCount();
	}

	return TRUE;
}

BOOL CUploadTransferDC::OnUpload(const std::string& strType, const std::string& strFilename, QWORD nOffset, QWORD nLength, const std::string& strOptions)
{
	ASSERT( m_pClient );

	if ( m_nState >= upsUploading )
	{
		// Drop unsent data
		CLockedBuffer pOutput( m_pClient->GetOutput() );
		pOutput->Clear();

		m_nState = upsRequest;
	}

	ClearRequest();

	m_sUserAgent = m_pClient->GetUserAgent();
	m_pHost = m_pClient->m_pHost;
	m_sAddress = m_pClient->m_sAddress;
	UpdateCountry();

	m_pClient->m_mInput.pLimit	= &Settings.Bandwidth.Request;
	m_pClient->m_mOutput.pLimit = &m_nBandwidth;

	m_tRequest = GetTickCount();

	BOOL bZip = ( strOptions.find("ZL1") != std::string::npos );

	if ( strType == "tthl" )
	{
		m_bGet = FALSE;

		if ( strFilename.substr( 0, 4 ) == "TTH/" )
		{
			Hashes::TigerHash oTiger;
			if ( oTiger.fromString( CA2W( strFilename.substr( 4 ).c_str() ) ) )
			{
				CSingleLock oLock( &Library.m_pSection );
				if ( oLock.Lock( 1000 ) )
				{
					if ( CLibraryFile* pFile = LibraryMaps.LookupFileByTiger( oTiger, TRUE, TRUE ) )
					{
						if ( RequestTigerTree( pFile, nOffset, nLength ) )
							return TRUE;
					}
				}
			}
		}
	}
	else if ( strType == "file" || strType =="get" )
	{
		m_bGet = ( strType == "get" );

		if ( strFilename == "files.xml" || strFilename == "files.xml.bz2" )
		{
			if ( RequestFileList( TRUE, bZip, strFilename, nOffset, nLength ) )
				return TRUE;
		}
		else if ( strFilename.substr( 0, 4 ) == "TTH/" )
		{
			Hashes::TigerHash oTiger;
			if ( oTiger.fromString( CA2W( strFilename.substr( 4 ).c_str() ) ) )
			{
				CSingleLock oLock( &Library.m_pSection );
				if ( oLock.Lock( 1000 ) )
				{
					if ( CLibraryFile* pFile = LibraryMaps.LookupFileByTiger( oTiger, TRUE, TRUE ) )
					{
						if ( RequestFile( pFile, nOffset, nLength ) )
							return TRUE;
					}
				}
			}
		}
	}
	else if ( strType == "list" )
	{
		m_bGet = FALSE;

		if ( RequestFileList( FALSE, bZip, strFilename, nOffset, nLength ) )
			return TRUE;
	}
	else if ( strType == "send" )
	{
		if ( m_bGet )
		{
			if ( m_pXML.GetCount() )
			{
				// Send cached file list
				m_bGet = FALSE;

				StartSending( upsBrowse );

				m_pClient->Write( &m_pXML );

				m_pXML.Clear();

				return TRUE;
			}
			else
			{
				// Send already requested file
				if ( SendFile() )
					return TRUE;
			}
		}
		// else $Send without $Get
	}
	else
	{
		// Invalid request type
		return FALSE;
	}

	theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND,
		(LPCTSTR)m_sAddress, (LPCTSTR)CA2CT( strFilename.c_str() ) );

	m_pClient->SendCommand( FILE_NOT_AVAILABLE );

	return TRUE;
}

BOOL CUploadTransferDC::CheckRanking()
{
	ASSERT( m_nState == upsQueued );

	m_tRankingCheck = GetTickCount();

	int nPosition = UploadQueues.GetPosition( this, TRUE );
	if ( nPosition < 0 )
	{
		// Invalid queue position, or queue deleted
		Close( IDS_UPLOAD_DROPPED );
		return FALSE;
	}
	else if ( nPosition == 0 )
	{
		// Ready to send
		m_tRankingCheck += 60*1000;

		if ( m_pClient->IsOnline() )
			return SendFile();
		else
			return m_pClient->Connect();
	}

	// Continue waiting in queue
	return TRUE;
}

BOOL CUploadTransferDC::RequestFile(CLibraryFile* pFile, QWORD nOffset, QWORD nLength)
{
	m_pXML.Clear();

	if ( ! RequestComplete( pFile ) )
	{
		ASSERT( FALSE );
		return FALSE;
	}

	if ( ! UploadQueues.CanUpload( PROTOCOL_DC, pFile, FALSE ) )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_sName );

		m_pClient->SendCommand( FILE_NOT_AVAILABLE );

		return TRUE;
	}

	m_nOffset = nOffset;
	if ( m_nOffset >= m_nSize )
		m_nLength = SIZE_UNKNOWN;
	else
		m_nLength = min( ( ( nLength == SIZE_UNKNOWN ) ? m_nSize : nLength ), m_nSize - m_nOffset );
	m_nPosition = 0;

	if ( m_nLength > m_nSize || m_nOffset + m_nLength > m_nSize )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BAD_RANGE,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_sName );

		m_pClient->SendCommand( FILE_NOT_AVAILABLE );	

		return TRUE;
	}

	AllocateBaseFile();

	if ( m_bStopTransfer )
	{
		m_tRotateTime = 0;
		m_bStopTransfer	= FALSE;

		CUploadQueue* pQueue = m_pQueue;
		if ( pQueue )
			pQueue->Dequeue( this );
	}

	int nPosition = UploadQueues.GetPosition( this, TRUE );
	if ( nPosition < 0 && UploadQueues.Enqueue( this ) )
	{
		nPosition = UploadQueues.GetPosition( this, TRUE );
	}

	if ( nPosition == 0 )
	{
		// Ready to send
		if ( m_bGet)
			// Wait for $Send
			return TRUE;
		else
			return SendFile();
	}
	else if ( nPosition > 0 )
	{
		// Queued
		theApp.Message( MSG_INFO, IDS_UPLOAD_QUEUED, (LPCTSTR)m_sName,
			(LPCTSTR)m_sAddress, nPosition, m_pQueue->GetQueuedCount(),
			(LPCTSTR)m_pQueue->m_sName );

		CString strQueued;
		strQueued.Format( UPLOAD_QUEUE, nPosition );

		m_pClient->SendCommand( strQueued );

		StartSending( upsPreQueue );

		m_tRankingCheck = GetTickCount();

		return TRUE;
	}
	else
	{
		// Unable to queue anywhere
		UploadQueues.Dequeue( this );
		ASSERT( m_pQueue == NULL );

		theApp.Message( MSG_ERROR, IDS_UPLOAD_BUSY_QUEUE,
			(LPCTSTR)m_sName, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );

		m_pClient->SendCommand( UPLOAD_BUSY );

		return TRUE;
	}
}

BOOL CUploadTransferDC::SendFile()
{
	if ( ! OpenFile() )
	{
		m_pClient->SendCommand( FILE_NOT_AVAILABLE );

		return FALSE;
	}

	if ( m_bGet )
		m_bGet = FALSE;
	else
	{
		CString sAnswer;
		sAnswer.Format( _T("$ADCSND file TTH/%s %I64u %I64u|"),
			(LPCTSTR)m_oTiger.toString(), m_nOffset, m_nLength );

		m_pClient->SendCommand( sAnswer );
	}

	StartSending( upsUploading );

	if ( m_pBaseFile->m_nRequests++ == 0 )
	{
		theApp.Message( MSG_NOTICE, IDS_UPLOAD_FILE,
			(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );
		
		PostMainWndMessage( WM_NOWUPLOADING, 0, (LPARAM)new CString( m_pBaseFile->m_sPath ) );
	}
	
	theApp.Message( MSG_INFO, IDS_UPLOAD_CONTENT,
		m_nOffset, m_nOffset + m_nLength - 1, (LPCTSTR)m_sName,
		(LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );

	return TRUE;
}

BOOL CUploadTransferDC::RequestTigerTree(CLibraryFile* pFile, QWORD nOffset, QWORD nLength)
{
	m_pXML.Clear();

	if ( ! RequestComplete( pFile ) )
	{
		ASSERT( FALSE );
		return FALSE;
	}

	theApp.Message( MSG_INFO, IDS_UPLOAD_TIGER_SEND,
		(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );

	CAutoPtr< const CTigerTree > pTigerTree( pFile->GetTigerTree() );
	if ( ! pTigerTree )
	{
		return FALSE;
	}

	BYTE* pSerialTree = NULL;
	DWORD nSerialTree = 0;
	if ( ! pTigerTree->ToBytesLevel1( &pSerialTree, &nSerialTree ) )
	{
		return FALSE;
	}

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
	sAnswer.Format( _T("$ADCSND tthl TTH/%s %I64u %I64u|"),
		(LPCTSTR)m_oTiger.toString(), nOffset, nLength );

	m_pClient->SendCommand( sAnswer );
	
	m_pClient->Write( pSerialTree + nOffset, (size_t)nLength );

	// Start uploading
	m_nOffset = nOffset;
	m_nLength = nLength;
	m_nPosition = 0;

	StartSending( upsTigerTree );

	GlobalFree( pSerialTree );

	return TRUE;
}

BOOL CUploadTransferDC::RequestFileList(BOOL bFile, BOOL bZip, const std::string& strFilename, QWORD nOffset, QWORD nLength)
{
	m_pXML.Clear();

	theApp.Message( MSG_NOTICE, IDS_UPLOAD_BROWSE, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );

	BOOL bBZip = bFile && ( strFilename == "files.xml.bz2" );
	m_sName = ( bFile ? "/" : strFilename.c_str() );

	// Create library file list
	CAutoPtr< CXMLElement > pXML( LibraryFolders.CreateXML( m_sName, TRUE, xmlDC ) );
	if ( ! pXML )
		// Out of memory
		return FALSE;

	m_pXML.Print( pXML->ToString( TRUE, TRUE, TRUE, TRI_TRUE ), CP_UTF8 );

	// TODO: Implement partial request of file list
	nOffset = 0;
	nLength = m_pXML.m_nLength;

	if ( bBZip )
	{
		// BZip it
		if ( ! m_pXML.BZip() )
			// Out of memory
			return FALSE;

		bZip = FALSE;
		nLength = m_pXML.m_nLength;
	}

	if ( bZip )
	{
		// Zip it
		if ( ! m_pXML.Deflate() )
			// Out of memory
			return FALSE;
	}

	m_nOffset = nOffset;
	m_nLength = m_pXML.m_nLength;
	m_nPosition = 0;

	if ( m_bGet )
	{
		CString sAnswer;
		sAnswer.Format( _T("$FileLength %I64u|"), nLength );

		m_pClient->SendCommand( sAnswer );

		return TRUE;
	}

	CString sAnswer;
	sAnswer.Format( _T("$ADCSND %hs %hs %I64u %I64u%hs|"),
		( bFile ? "file" : "list" ), strFilename.c_str(),
		nOffset, nLength, ( bZip ? " ZL1" : "") );

	m_pClient->SendCommand( sAnswer );

	StartSending( upsBrowse );

	m_pClient->Write( &m_pXML );

	m_pXML.Clear();

	return TRUE;
}
