//
// UploadTransferED2K.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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
#include "Datagrams.h"
#include "UploadFile.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "UploadTransferED2K.h"
#include "Statistics.h"
#include "EDClient.h"
#include "EDPacket.h"
#include "ED2K.h"

#include "Buffer.h"
#include "Library.h"
#include "Download.h"
#include "Downloads.h"
#include "SharedFile.h"
#include "TransferFile.h"
#include "FragmentedFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K construction

CUploadTransferED2K::CUploadTransferED2K(CEDClient* pClient) : CUploadTransfer( PROTOCOL_ED2K )
{
	ASSERT( pClient != NULL );
	
	m_pClient		= pClient;
	m_nState		= upsReady;
	m_tRequest		= GetTickCount();
	
	m_sUserAgent	= m_pClient->m_sUserAgent;
	m_pHost			= m_pClient->m_pHost;
	m_sAddress		= inet_ntoa( m_pHost.sin_addr );
	m_sNick			= m_pClient->m_sNick;
	
	m_tRanking		= 0;
	m_pRequested	= NULL;
	m_pServed		= NULL;
	
	m_pClient->m_mOutput.pLimit = &m_nBandwidth;
}

CUploadTransferED2K::~CUploadTransferED2K()
{
	ASSERT( m_pClient == NULL );
	ASSERT( m_pRequested == NULL );
	ASSERT( m_pServed == NULL );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K request

BOOL CUploadTransferED2K::Request(MD4* pMD4)
{
	BOOL bSame = ( m_bED2K && m_pED2K == *pMD4 );
	
	Cleanup( ! bSame );
	
	if ( CLibraryFile* pFile = LibraryMaps.LookupFileByED2K( pMD4, TRUE, TRUE, TRUE ) )
	{
		RequestComplete( pFile );
		Library.Unlock();
	}
	else if ( CDownload* pFile = Downloads.FindByED2K( pMD4, TRUE ) )
	{
		RequestPartial( pFile );
	}
	else
	{
		UploadQueues.Dequeue( this );
		
		theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress,
			(LPCTSTR)CED2K::HashToString( pMD4, TRUE ) );	
		
		CEDPacket* pReply = CEDPacket::New( ED2K_C2C_FILENOTFOUND );
		pReply->Write( pMD4, sizeof(MD4) );
		Send( pReply );
		
		Close();
		return FALSE;
	}
	
	if ( UploadQueues.GetPosition( this, FALSE ) < 0 && ! UploadQueues.Enqueue( this ) )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BUSY_QUEUE,
			(LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress, _T("ED2K") );	
		
		CEDPacket* pReply = CEDPacket::New( ED2K_C2C_FILENOTFOUND );
		pReply->Write( pMD4, sizeof(MD4) );
		Send( pReply );
		
		Close();
		return FALSE;
	}
	
	AllocateBaseFile();
	
	theApp.Message( MSG_SYSTEM, IDS_UPLOAD_FILE,
		(LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
	
	m_nRanking = -1;
	return SendRanking();
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K close

void CUploadTransferED2K::Close(BOOL bMessage)
{
	if ( m_nState == upsNull )
	{
		if ( m_pClient != NULL ) m_pClient->OnUploadClose();
		m_pClient = NULL;
		return;
	}
	
	if ( m_pBaseFile != NULL && m_pClient->IsOnline() )
	{
		if ( m_nState == upsUploading || m_nState == upsQueued )
		{
			Send( CEDPacket::New( ED2K_C2C_FINISHUPLOAD ) );
		}
		
		CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_FILENOTFOUND );
		pPacket->Write( &m_pED2K, sizeof(MD4) );
		Send( pPacket );
	}
	
	Cleanup();
	
	ASSERT( m_pClient != NULL );
	m_pClient->OnUploadClose();
	m_pClient = NULL;
	
	CUploadTransfer::Close( bMessage );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K run

BOOL CUploadTransferED2K::OnRun()
{
	return OnRunEx( GetTickCount() );
}

BOOL CUploadTransferED2K::OnRunEx(DWORD tNow)
{
	if ( m_nState == upsQueued )
	{
		if ( m_pClient->IsOnline() == FALSE && tNow > m_tRequest &&
			 tNow - m_tRequest >= Settings.eDonkey.DequeueTime * 1000 )
		{
			theApp.Message( MSG_ERROR, IDS_UPLOAD_QUEUE_TIMEOUT, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
		else if ( tNow > m_tRanking && tNow - m_tRanking >= 10000 )	//If client hasn't had an update recently
		{	//Then send them one. (Note: Updates are only sent if queue rank has changed)
			if ( ! SendRanking() ) return FALSE;
		}
	}
	else if ( m_nState == upsUploading )
	{
		if ( ! ServeRequests() ) return FALSE;
		
		if ( tNow > m_pClient->m_mOutput.tLast &&
			 tNow - m_pClient->m_mOutput.tLast > Settings.Connection.TimeoutTraffic * 3 )
		{
			theApp.Message( MSG_ERROR, IDS_UPLOAD_TRAFFIC_TIMEOUT, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
	}
	else if ( m_nState == upsReady || m_nState == upsRequest )
	{
		if ( tNow > m_tRequest && tNow - m_tRequest > Settings.Connection.TimeoutHandshake )
		{
			theApp.Message( MSG_ERROR, IDS_UPLOAD_REQUEST_TIMEOUT, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
	}
	else if ( m_nState == upsConnecting )
	{
		if ( tNow > m_tRequest && tNow - m_tRequest > Settings.Connection.TimeoutConnect + Settings.Connection.TimeoutHandshake + 10000 )
		{
			Close( TRUE );
			return FALSE;
		}
	}
	
	return CUploadTransfer::OnRun();
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K connection establishment

BOOL CUploadTransferED2K::OnConnected()
{
	if ( m_nState != upsConnecting ) return TRUE;
	
	m_tRequest = GetTickCount();
	m_nRanking = -1;
	
	m_pClient->m_mOutput.pLimit = &m_nBandwidth;
	
	return SendRanking();
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K connection lost

void CUploadTransferED2K::OnDropped(BOOL bError)
{
	if ( m_nState == upsQueued )
	{
		theApp.Message( MSG_DEFAULT, IDS_UPLOAD_QUEUE_DROP, (LPCTSTR)m_sAddress );
		
		m_tRequest = GetTickCount();
		
		m_pRequested->DeleteChain();
		m_pRequested = NULL;
		
		m_pServed->DeleteChain();
		m_pServed = NULL;
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_DROPPED, (LPCTSTR)m_sAddress );
		Close();
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K queue drop hook

void CUploadTransferED2K::OnQueueKick()
{
	m_nRanking = -1;
	
	if ( m_nState == upsRequest || m_nState == upsUploading )
	{
		if ( UploadQueues.GetPosition( this, TRUE ) == 0 ) return;
		
		if ( m_pBaseFile != NULL && m_pClient->IsOnline() )
		{
			Send( CEDPacket::New( ED2K_C2C_FINISHUPLOAD ) );
		}
		
		Cleanup( FALSE );
	}
	else
	{
		SendRanking();
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K speed measurement

DWORD CUploadTransferED2K::GetMeasuredSpeed()
{
	if ( m_pClient == NULL ) return 0;
	m_pClient->Measure();
	return m_pClient->m_mOutput.nMeasure;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K queue release

BOOL CUploadTransferED2K::OnQueueRelease(CEDPacket* pPacket)
{
	Cleanup();
	Close( TRUE );
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K part request

BOOL CUploadTransferED2K::OnRequestParts(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < sizeof(MD4) + 4 * 3 * 2 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close();
		return FALSE;
	}
	
	MD4 pMD4;
	pPacket->Read( &pMD4, sizeof(MD4) );
	
	if ( pMD4 != m_pED2K )
	{
		if ( ! Request( &pMD4 ) ) return FALSE;
	}
	
	if ( m_nState != upsQueued && m_nState != upsRequest && m_nState != upsUploading )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close();
		return FALSE;
	}
	
	DWORD nOffset[2][3];
	pPacket->Read( nOffset, sizeof(DWORD) * 2 * 3 );
	
	for ( int nRequest = 0 ; nRequest < 3 ; nRequest++ )
	{
		if ( nOffset[0][nRequest] < nOffset[1][nRequest] )
		{
			AddRequest( nOffset[0][nRequest], nOffset[1][nRequest] - nOffset[0][nRequest] );
		}
	}
	
	ServeRequests();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K cleanup

void CUploadTransferED2K::Cleanup(BOOL bDequeue)
{
	if ( bDequeue ) UploadQueues.Dequeue( this );
	
	if ( m_nState == upsUploading )
	{
		ASSERT( m_pBaseFile != NULL );
		if ( m_nLength < SIZE_UNKNOWN ) m_pBaseFile->AddFragment( m_nOffset, m_nPosition );
		
		ASSERT( m_pDiskFile != NULL );
		CloseFile();
	}
	
	ClearRequest();
	
	m_pRequested->DeleteChain();
	m_pRequested = NULL;
	
	m_pServed->DeleteChain();
	m_pServed = NULL;
	
	m_pBaseFile	= NULL;
	m_nState	= upsReady;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K send

void CUploadTransferED2K::Send(CEDPacket* pPacket, BOOL bRelease)
{
	ASSERT( m_nState != upsNull );
	ASSERT( m_pClient != NULL );
	m_pClient->Send( pPacket, bRelease );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K request a fragment

void CUploadTransferED2K::AddRequest(QWORD nOffset, QWORD nLength)
{
	ASSERT( m_pBaseFile != NULL );
	
	for ( CFileFragment* pFragment = m_pRequested ; pFragment ; pFragment = pFragment->m_pNext )
	{
		if ( pFragment->m_nOffset == nOffset && pFragment->m_nLength == nLength ) return;
	}
	
	m_pRequested = CFileFragment::New( NULL, m_pRequested, nOffset, nLength );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K serve requests

BOOL CUploadTransferED2K::ServeRequests()
{
	if ( m_nState != upsUploading && m_nState != upsRequest ) return TRUE;
	ASSERT( m_pBaseFile != NULL );
	
	if ( m_pClient == NULL || m_pClient->m_pOutput == NULL ) return TRUE;
	if ( m_pClient->m_pOutput->m_nLength > Settings.eDonkey.FrameSize ) return TRUE;
	
	if ( m_nLength == SIZE_UNKNOWN )
	{
		if ( ! OpenFile() ) return FALSE;
		if ( ! StartNextRequest() ) return FALSE;
	}
	
	if ( m_nLength != SIZE_UNKNOWN )
	{
		if ( DispatchNextChunk() )
		{
			CheckFinishedRequest();
		}
		else
		{
			Cleanup();
			Close();
			return FALSE;
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K file access

BOOL CUploadTransferED2K::OpenFile()
{
	ASSERT( m_nState == upsRequest || m_nState == upsUploading );
	ASSERT( m_pBaseFile != NULL );
	
	if ( m_pDiskFile != NULL ) return TRUE;
	m_pDiskFile = TransferFiles.Open( m_sFilePath, FALSE, FALSE );
	
	if ( m_pDiskFile != NULL )
	{
		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( m_sFilePath, TRUE, TRUE, TRUE ) )
		{
			pFile->m_nUploadsToday++;
			pFile->m_nUploadsTotal++;
			Library.Unlock();
		}
		
		return TRUE;
	}
	
	theApp.Message( MSG_ERROR, IDS_UPLOAD_CANTOPEN, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );	
	
	CEDPacket* pReply = CEDPacket::New( ED2K_C2C_FILENOTFOUND );
	pReply->Write( &m_pED2K, sizeof(MD4) );
	Send( pReply );
	
	Cleanup();
	Close();
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K start the next request

BOOL CUploadTransferED2K::StartNextRequest()
{
	ASSERT( m_nState == upsUploading || m_nState == upsRequest );
	ASSERT( m_pDiskFile != NULL );
	
	while ( m_pRequested != NULL && m_nLength == SIZE_UNKNOWN )
	{
		CFileFragment* pFragment = m_pRequested;
		m_pRequested = pFragment->m_pNext;
		
		for ( CFileFragment* pOld = m_pServed ; pOld ; pOld = pOld->m_pNext )
		{
			if ( pOld->m_nOffset == pFragment->m_nOffset && pOld->m_nLength == pFragment->m_nLength ) break;
		}
		
		if ( pOld == NULL &&
			 pFragment->m_nOffset < m_nFileSize &&
			 pFragment->m_nOffset + pFragment->m_nLength <= m_nFileSize )
		{
			m_nOffset	= pFragment->m_nOffset;
			m_nLength	= pFragment->m_nLength;
			m_nPosition	= 0;
		}
		
		pFragment->DeleteThis();
	}
	
	if ( m_nLength < SIZE_UNKNOWN )
	{
		m_nState	= upsUploading;
		m_tContent	= m_pClient->m_mOutput.tLast = GetTickCount();
		
		theApp.Message( MSG_DEFAULT, IDS_UPLOAD_CONTENT,
			m_nOffset, m_nOffset + m_nLength - 1,
			(LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress,
			(LPCTSTR)m_sUserAgent );
		
		return TRUE;
	}
	else
	{
		Send( CEDPacket::New( ED2K_C2C_FINISHUPLOAD ) );
		Cleanup();
		Close( TRUE );
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K chunk dispatch

BOOL CUploadTransferED2K::DispatchNextChunk()
{
	ASSERT( m_nState == upsUploading );
	ASSERT( m_pDiskFile != NULL );
	ASSERT( m_nLength < SIZE_UNKNOWN );
	ASSERT( m_nPosition < m_nLength );
	
	QWORD nChunk = m_nLength - m_nPosition;
	nChunk = min( nChunk, Settings.eDonkey.FrameSize );
	
#if 0
	// Use packet form
	
	CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_SENDINGPART );
	pPacket->Write( &m_pED2K, sizeof(MD4) );
	pPacket->WriteLongLE( m_nOffset + m_nPosition );
	pPacket->WriteLongLE( m_nOffset + m_nPosition + nChunk );
	
	m_pDiskFile->Read( m_nFileBase + m_nOffset + m_nPosition, pPacket->GetWritePointer( nChunk ), nChunk, &nChunk );
	// SetFilePointer( hFile, m_nFileBase + m_nOffset + m_nPosition, NULL, FILE_BEGIN );
	// ReadFile( hFile, pPacket->WriteGetPointer( nChunk ), nChunk, &nChunk, NULL );
	
	if ( nChunk == 0 )
	{
		pPacket->Release();
		return FALSE;
	}
	
	pPacket->m_nLength = sizeof(MD4) + 8 + nChunk;
	
	Send( pPacket );
	
#else
	// Raw write
	
	CBuffer* pBuffer = m_pClient->m_pOutput;
	pBuffer->EnsureBuffer( sizeof(ED2K_PART_HEADER) + (DWORD)nChunk );
	
	ED2K_PART_HEADER* pHeader = (ED2K_PART_HEADER*)( pBuffer->m_pBuffer + pBuffer->m_nLength );
	
	if ( ! m_pDiskFile->Read( m_nFileBase + m_nOffset + m_nPosition, &pHeader[1], nChunk, &nChunk ) ) return FALSE;
	// SetFilePointer( hFile, m_nFileBase + m_nOffset + m_nPosition, NULL, FILE_BEGIN );
	// ReadFile( hFile, &pHeader[1], nChunk, &nChunk, NULL );
	if ( nChunk == 0 ) return FALSE;
	
	pHeader->nProtocol	= ED2K_PROTOCOL_EDONKEY;
	pHeader->nType		= ED2K_C2C_SENDINGPART;
	pHeader->nLength	= 1 + sizeof(MD4) + 8 + (DWORD)nChunk;
	pHeader->pMD4		= m_pED2K;
	pHeader->nOffset1	= (DWORD)( m_nOffset + m_nPosition );
	pHeader->nOffset2	= (DWORD)( m_nOffset + m_nPosition + nChunk );
	
	pBuffer->m_nLength += sizeof(ED2K_PART_HEADER) + (DWORD)nChunk;
	m_pClient->Send( NULL );
	
#endif
	
	m_nPosition += nChunk;
	m_nUploaded += nChunk;
	Statistics.Current.Uploads.Volume += ( nChunk / 1024 );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K request

BOOL CUploadTransferED2K::CheckFinishedRequest()
{
	ASSERT( m_nState == upsUploading );
	
	if ( m_nPosition < m_nLength ) return FALSE;
	
	theApp.Message( MSG_DEFAULT, IDS_UPLOAD_FINISHED,
		(LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
	
	m_pServed = CFileFragment::New( NULL, m_pServed, m_nOffset, m_nLength );
	m_pBaseFile->AddFragment( m_nOffset, m_nLength );
	m_nLength = SIZE_UNKNOWN;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K ranking update

BOOL CUploadTransferED2K::SendRanking()
{
	ASSERT( m_pQueue != NULL );
	
	int nPosition = UploadQueues.GetPosition( this, TRUE );
	
	if ( nPosition < 0 )
	{	//Invalid queue position
		Cleanup();
		Close( TRUE );
		return FALSE;
	}
	
	//If queue ranking hasn't changed, don't send (unless specifically requested: m_nRanking set to -1)
	if ( m_nRanking == nPosition ) return TRUE;	
	//Update the 'ranking sent' variables
	m_nRanking = nPosition;
	m_tRanking = GetTickCount();
	
	if ( nPosition == 0 )
	{	//Ready to start uploading
		m_tRequest = m_tRanking;
		
		if ( m_pClient->IsOnline() )
		{
			m_nState = upsRequest;
			Send( CEDPacket::New( ED2K_C2C_STARTUPLOAD ) );
		}
		else
		{
			m_nState = upsConnecting;
			m_pClient->Connect();
		}
	}
	else if ( m_pClient->IsOnline() )
	{	//Upload is queued
		CSingleLock pLock( &UploadQueues.m_pSection, TRUE );
		
		if ( UploadQueues.Check( m_pQueue ) )
		{
			theApp.Message( MSG_DEFAULT, IDS_UPLOAD_QUEUED, (LPCTSTR)m_sFileName,
				(LPCTSTR)m_sAddress, nPosition, m_pQueue->GetQueuedCount(),
				(LPCTSTR)m_pQueue->m_sName );
		}
		
		pLock.Unlock();
		
		m_nState = upsQueued;
		
		if ( m_pClient->m_bEmule )
		{	//eMule queue ranking
			CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_QUEUERANKING, ED2K_PROTOCOL_EMULE );
			pPacket->WriteShortLE( nPosition );
			pPacket->WriteShortLE( 0 );
			pPacket->WriteLongLE( 0 );
			pPacket->WriteLongLE( 0 );
			Send( pPacket );
		}
		else
		{	//older eDonkey style
			CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_QUEUERANK );
			pPacket->WriteLongLE( nPosition );
			Send( pPacket );
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K reask

BOOL CUploadTransferED2K::OnReask()
{
	if ( m_nState != upsQueued ) return FALSE;
	
	int nPosition = UploadQueues.GetPosition( this, TRUE );
	if ( nPosition < 0 ) return FALSE;
	
	CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_UDP_REASKACK, ED2K_PROTOCOL_EMULE );
	pPacket->WriteShortLE( nPosition );
	Datagrams.Send( &m_pClient->m_pHost.sin_addr, m_pClient->m_nUDP, pPacket );
	
	m_tRequest = GetTickCount();
	
	return TRUE;
}
