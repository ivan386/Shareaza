//
// UploadTransferED2K.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "Datagrams.h"
#include "UploadFile.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "UploadTransferED2K.h"
#include "Statistics.h"
#include "EDClient.h"
#include "EDPacket.h"

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

CUploadTransferED2K::CUploadTransferED2K(CEDClient* pClient)
	: CUploadTransfer( PROTOCOL_ED2K )
	, m_pClient			( pClient )
	, m_nRanking		( 0 )
	, m_tRankingSent	( 0 )
	, m_tRankingCheck	( 0 )
	, m_tLastRun		( 0 )
{
	ASSERT( pClient != NULL );

	m_pHost				= pClient->m_pHost;
	m_sAddress			= pClient->m_sAddress;
	UpdateCountry();

	m_sUserAgent		= pClient->m_sUserAgent;
	m_bClientExtended	= pClient->m_bClientExtended;
	m_sRemoteNick		= pClient->m_sNick;
	m_nState			= upsReady;

	m_pClient->m_mOutput.pLimit = &m_nBandwidth;
}

CUploadTransferED2K::~CUploadTransferED2K()
{
	ASSERT( m_pClient == NULL );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K request

BOOL CUploadTransferED2K::Request(const Hashes::Ed2kHash& oED2K)
{
	ASSERT( oED2K );
	BOOL bSame = validAndEqual( m_oED2K, oED2K );

	Cleanup( ! bSame );

	CSingleLock oLock( &Library.m_pSection );
	BOOL bLocked = oLock.Lock( 1000 );
	if ( CLibraryFile* pFile = ( bLocked ? LibraryMaps.LookupFileByED2K( oED2K, TRUE, TRUE ) : NULL ) )
	{
		// Send comments if necessary
		if ( m_pClient ) m_pClient->SendCommentsPacket( pFile->m_nRating, pFile->m_sComments );

		RequestComplete( pFile );
		oLock.Unlock();
	}
	else
	{
		if ( bLocked )
			oLock.Unlock();

		if ( CDownload* pDownload = Downloads.FindByED2K( oED2K, TRUE ) )
		{
			RequestPartial( pDownload );
		}
		else
		{
			UploadQueues.Dequeue( this );

			theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress,
				(LPCTSTR)oED2K.toUrn() );

			CEDPacket* pReply = CEDPacket::New( ED2K_C2C_FILENOTFOUND );
			pReply->Write( oED2K );
			Send( pReply );

			Close();
			return FALSE;
		}
	}

	if ( UploadQueues.GetPosition( this, FALSE ) < 0 && ! UploadQueues.Enqueue( this ) )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BUSY_QUEUE,
			(LPCTSTR)m_sName, (LPCTSTR)m_sAddress, _T("ED2K") );

		CEDPacket* pReply = CEDPacket::New( ED2K_C2C_FILENOTFOUND );
		pReply->Write( oED2K );
		Send( pReply );

		Close();
		return FALSE;
	}

	AllocateBaseFile();

	if ( m_pBaseFile->m_nRequests++ == 0 )
	{
		theApp.Message( MSG_NOTICE, IDS_UPLOAD_FILE,
			(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );

		PostMainWndMessage( WM_NOWUPLOADING, 0, (LPARAM)new CString( m_pBaseFile->m_sPath ) );
	}

	m_nRanking = -1;
	return CheckRanking();
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K close

void CUploadTransferED2K::Close(UINT nError)
{
	if ( m_nState == upsNull )
	{
		if ( m_pClient != NULL ) m_pClient->OnUploadClose();
		m_pClient = NULL;
		return;
	}

	if ( m_pBaseFile != NULL && m_pClient && m_pClient->IsOnline() )
	{
		if ( m_nState == upsUploading || m_nState == upsQueued )
		{
			Send( CEDPacket::New( ED2K_C2C_FINISHUPLOAD ) );
		}
	}

	Cleanup();

	ASSERT( m_pClient != NULL );
	m_pClient->OnUploadClose();
	m_pClient = NULL;

	CUploadTransfer::Close( nError );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K run

BOOL CUploadTransferED2K::OnRun()
{
	return OnRunEx( GetTickCount() );
}

BOOL CUploadTransferED2K::OnRunEx(DWORD tNow)
{
	// Limit per-source packet rate
	if ( tNow - m_tLastRun < Settings.eDonkey.SourceThrottle )
		return FALSE;

	m_tLastRun = tNow;

	if ( m_nState == upsQueued )
	{
		if ( m_pClient->IsOnline() == FALSE && tNow > m_tRequest &&
			 tNow - m_tRequest >= Settings.eDonkey.DequeueTime * 1000 )
		{
			Close( IDS_UPLOAD_QUEUE_TIMEOUT );
			return FALSE;
		}
		else
		{
			DWORD nCheckThrottle;	// Throttle for how often ED2K clients have queue rank checked
			if ( m_nRanking <= 2 )
				nCheckThrottle = 2 * 1000;
			else if ( m_nRanking < 10 )
				nCheckThrottle = 15 * 1000;
			else if ( m_nRanking < 50 )
				nCheckThrottle = 1 * 60 * 1000;
			else if ( m_nRanking < 200 )
				nCheckThrottle = 4 * 60 * 1000;
			else
				nCheckThrottle = 8 * 60 * 1000;

			if ( tNow > m_tRankingCheck && tNow - m_tRankingCheck >= nCheckThrottle )
			{
				// Check the queue rank. Start upload or send rank update if required.
				if ( !CheckRanking() )
					return FALSE;
			}
		}
	}
	else if ( m_nState == upsUploading )
	{
		if ( !ServeRequests() )
			return FALSE;

		if ( tNow > m_pClient->m_mOutput.tLast &&
			 tNow - m_pClient->m_mOutput.tLast > Settings.Connection.TimeoutTraffic * 3 )
		{
			Close( IDS_UPLOAD_TRAFFIC_TIMEOUT );
			return FALSE;
		}
	}
	else if ( m_nState == upsReady || m_nState == upsRequest )
	{
		if ( tNow > m_tRequest && tNow - m_tRequest > Settings.Connection.TimeoutHandshake )
		{
			Close( IDS_UPLOAD_REQUEST_TIMEOUT );
			return FALSE;
		}
	}
	else if ( m_nState == upsConnecting )
	{
		if ( tNow > m_tRequest && tNow - m_tRequest > Settings.Connection.TimeoutConnect + Settings.Connection.TimeoutHandshake + 10000 )
		{
			Close( IDS_UPLOAD_DROPPED );
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

	return CheckRanking();
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K connection lost

void CUploadTransferED2K::OnDropped()
{
	if ( m_nState == upsQueued )
	{
		theApp.Message( MSG_INFO, IDS_UPLOAD_QUEUE_DROP, (LPCTSTR)m_sAddress );

		m_tRequest = GetTickCount();

		m_oRequested.clear();

		m_oServed.clear();
	}
	else
	{
		Close( IDS_UPLOAD_DROPPED );
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
		CheckRanking();
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K speed measurement

DWORD CUploadTransferED2K::GetMeasuredSpeed()
{
	// Return if there is no client
	if ( m_pClient == NULL ) return 0;

	// Calculate Output
	m_pClient->MeasureOut();

	// Return calculated speed
	return m_pClient->m_mOutput.nMeasure;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K queue release

BOOL CUploadTransferED2K::OnQueueRelease(CEDPacket* /*pPacket*/)
{
	Cleanup();
	Close( IDS_UPLOAD_DROPPED );
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K part request

BOOL CUploadTransferED2K::OnRequestParts(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount + 4 * 3 * 2 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close();
		return FALSE;
	}

	Hashes::Ed2kHash oED2K;
	pPacket->Read( oED2K );

	if ( validAndUnequal( oED2K, m_oED2K ) )
	{
		if ( ! Request( oED2K ) ) return FALSE;
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
		if ( nOffset[1][nRequest] <= m_nSize )
		{
			// Valid (or null) request
			if ( nOffset[0][nRequest] < nOffset[1][nRequest] )
			{
				// Add non-null ranges to the list
				AddRequest( nOffset[0][nRequest], nOffset[1][nRequest] - nOffset[0][nRequest] );
			}
		}
		else
		{
			// Invalid request- had an impossible range.
			theApp.Message( MSG_ERROR, IDS_UPLOAD_BAD_RANGE, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
			// They probably have an incorrent hash associated with a file. Calling close now
			// will send "file not found" to stop them re-asking, then close the connection.
			Close();
			return FALSE;
		}
	}

	ServeRequests();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K cleanup

void CUploadTransferED2K::Cleanup(BOOL bDequeue)
{
	if ( bDequeue )
		UploadQueues.Dequeue( this );

	if ( m_nState == upsUploading )
	{
		ASSERT( m_pBaseFile != NULL );
		if ( m_nLength < SIZE_UNKNOWN )
			m_pBaseFile->AddFragment( m_nOffset, m_nPosition );

		CloseFile();
	}

	ClearRequest();

	m_oRequested.clear();

	m_oServed.clear();

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

	Fragments::Fragment oRequest( nOffset, nOffset + nLength );

	if ( std::find( m_oRequested.begin(), m_oRequested.end(), oRequest ) == m_oRequested.end() )
	{
		m_oRequested.push_back( oRequest );
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K serve requests

BOOL CUploadTransferED2K::ServeRequests()
{
	if ( m_nState != upsUploading && m_nState != upsRequest )
		return TRUE;

	if ( !m_pClient || !m_pClient->IsOutputExist() )
		return TRUE;

	if ( m_pClient->GetOutputLength() > 512000ul ) // 500 KB
		return TRUE;

	ASSERT( m_pBaseFile != NULL );

	if ( m_nLength == SIZE_UNKNOWN )
	{
		// Check has just finished
		if ( m_bStopTransfer )
		{
			m_tRotateTime = 0;
			m_bStopTransfer	= FALSE;

			if ( CUploadQueue* pQueue = m_pQueue )
			{
				pQueue->Dequeue( this );
				pQueue->Enqueue( this, TRUE, FALSE );
			}

			int nQpos = UploadQueues.GetPosition( this, TRUE );
			if ( nQpos != 0 )
			{
				if ( m_pBaseFile != NULL && m_pClient->IsOnline() )
				{
					Send( CEDPacket::New( ED2K_C2C_FINISHUPLOAD ) );
				}

				if ( nQpos > 0 )	// If we aren't uploading any more (the queue wasn't empty)
				{
					// Set state to queued, and reset ranking to send a queue ranking packet.
					m_tRequest = GetTickCount();
					m_nState = upsQueued;
					m_nRanking = -1;
				}

				return TRUE;
			}
		}

		ASSERT( m_nState == upsRequest || m_nState == upsUploading );
		ASSERT( m_pBaseFile != NULL );

		if ( ! OpenFile() )
		{
			CEDPacket* pReply = CEDPacket::New( ED2K_C2C_FILENOTFOUND );
			pReply->Write( m_oED2K );
			Send( pReply );

			Cleanup();
			Close();
			return FALSE;
		}

		if ( ! StartNextRequest() )
			return FALSE;
	}

	if ( m_nLength != SIZE_UNKNOWN )
	{
		if ( DispatchNextChunk() )
		{
			CheckFinishedRequest();

			if ( !Settings.eDonkey.EnableToday && Settings.Connection.RequireForTransfers )
			{
				Send( CEDPacket::New( ED2K_C2C_FINISHUPLOAD ) );
				Cleanup();
				Close();
				return FALSE;
			}
		}
		else
		{
			Send( CEDPacket::New( ED2K_C2C_FINISHUPLOAD ) );
			Cleanup();
			Close();
			return FALSE;
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K start the next request

BOOL CUploadTransferED2K::StartNextRequest()
{
	ASSERT( m_nState == upsUploading || m_nState == upsRequest );
	ASSERT( IsFileOpen() );

	while ( !m_oRequested.empty() && m_nLength == SIZE_UNKNOWN )
	{
		Fragments::Queue::const_iterator iRequested = m_oRequested.begin();
		if ( std::find( m_oServed.begin(), m_oServed.end(), *iRequested ) == m_oServed.end()
			// This should be redundant (Camper)
			&& iRequested->begin() < m_nSize
			&& iRequested->end() <= m_nSize )
		{
			m_nOffset = iRequested->begin();
			m_nLength = iRequested->size();
			m_nPosition = 0;
		}
		m_oRequested.pop_front();
	}

	if ( m_nLength == SIZE_UNKNOWN )
		// Waiting for new requests
		return TRUE;
	
	if ( !Settings.eDonkey.EnableToday && Settings.Connection.RequireForTransfers )
	{
		Send( CEDPacket::New( ED2K_C2C_FINISHUPLOAD ) );
		Cleanup();
		Close();
		return FALSE;
	}

	m_nState	= upsUploading;
	m_tContent	= m_pClient->m_mOutput.tLast = GetTickCount();

	theApp.Message( MSG_INFO, IDS_UPLOAD_CONTENT,
		m_nOffset, m_nOffset + m_nLength - 1,
		(LPCTSTR)m_sName, (LPCTSTR)m_sAddress,
		(LPCTSTR)m_sUserAgent );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K chunk dispatch

BOOL CUploadTransferED2K::DispatchNextChunk()
{
	ASSERT( m_nState == upsUploading );

	if ( ! IsFileOpen() )
		return FALSE;

	ASSERT( m_nLength < SIZE_UNKNOWN );
	ASSERT( m_nPosition < m_nLength );

	QWORD nPacket = min( m_nLength - m_nPosition, 1024000ull ); // 1000 KB

	Statistics.Current.Uploads.Volume += ( nPacket / 1024 );

	while ( nPacket )
	{
		QWORD nChunk = min( nPacket, (QWORD)Settings.eDonkey.FrameSize );
		QWORD nOffset = m_nOffset + m_nPosition;
		bool bI64Offset =	( nOffset & 0xffffffff00000000 ) || ( ( nOffset + nChunk ) & 0xffffffff00000000 );
#if 1
		// Use packet form
		CEDPacket* pPacket;
		if ( bI64Offset )
		{
			pPacket = CEDPacket::New( ED2K_C2C_SENDINGPART_I64, ED2K_PROTOCOL_EMULE );
			if ( ! pPacket )
				// Out of memory
				return FALSE;

			pPacket->Write( m_oED2K );
			pPacket->WriteLongLE( nOffset & 0x00000000ffffffff );
			pPacket->WriteLongLE( ( nOffset & 0xffffffff00000000 ) >> 32 );
			pPacket->WriteLongLE( ( nOffset + nChunk ) & 0x00000000ffffffff );
			pPacket->WriteLongLE( ( ( nOffset + nChunk ) & 0xffffffff00000000 ) >> 32);
		}
		else
		{
			pPacket = CEDPacket::New( ED2K_C2C_SENDINGPART );
			if ( ! pPacket )
				// Out of memory
				return FALSE;

			pPacket->Write( m_oED2K );
			pPacket->WriteLongLE( (DWORD)nOffset );
			pPacket->WriteLongLE( (DWORD)( nOffset + nChunk ) );
		}
		if ( ! ReadFile( m_nFileBase + nOffset, pPacket->WriteGetPointer( (DWORD)nChunk ), nChunk, &nChunk ) || nChunk == 0 )
		{
			// File error
			pPacket->Release();
			return FALSE;
		}
		Send( pPacket );
#else
		// Raw write
		CBuffer pBuffer;
		if ( bI64Offset )
		{
			if ( ! pBuffer.EnsureBuffer( sizeof(ED2K_PART_HEADER_I64) + nChunk ) )
				// Out of memory
				return FALSE;

			ED2K_PART_HEADER_I64* pHeader = (ED2K_PART_HEADER_I64*)( pBuffer.m_pBuffer + pBuffer.m_nLength );

			if ( ! ReadFile( m_nFileBase + nOffset, &pHeader[1], nChunk, &nChunk ) || nChunk == 0 )
				// File error
				return FALSE;

			pHeader->nProtocol	= ED2K_PROTOCOL_EMULE;
			pHeader->nType		= ED2K_C2C_SENDINGPART_I64;
			pHeader->nLength	= (DWORD)( 1 + m_oED2K.byteCount + 16 + nChunk );
			CopyMemory( &*pHeader->pMD4.begin(), &*m_oED2K.begin(), m_oED2K.byteCount );
			pHeader->nOffset1	= nOffset;
			pHeader->nOffset2	= nOffset + nChunk;

			pBuffer.m_nLength += (DWORD)( sizeof(ED2K_PART_HEADER_I64) + nChunk );
		}
		else
		{
			if ( ! pBuffer.EnsureBuffer( sizeof(ED2K_PART_HEADER) + nChunk ) )
				// Out of memory
				return FALSE;

			ED2K_PART_HEADER* pHeader = (ED2K_PART_HEADER*)( pBuffer.m_pBuffer + pBuffer.m_nLength );

			if ( ! ReadFile( m_nFileBase + nOffset, &pHeader[1], nChunk, &nChunk ) || nChunk == 0 )
				// File error
				return FALSE;

			pHeader->nProtocol	= ED2K_PROTOCOL_EDONKEY;
			pHeader->nType		= ED2K_C2C_SENDINGPART;
			pHeader->nLength	= (DWORD)( 1 + m_oED2K.byteCount + 8 + nChunk );
			CopyMemory( &*pHeader->pMD4.begin(), &*m_oED2K.begin(), m_oED2K.byteCount );
			pHeader->nOffset1	= (DWORD)nOffset;
			pHeader->nOffset2	= (DWORD)( nOffset + nChunk );

			pBuffer.m_nLength += (DWORD)( sizeof(ED2K_PART_HEADER) + nChunk );
		}
		m_pClient->Write( &pBuffer );
#endif
		nPacket -= nChunk;
		m_nPosition += nChunk;
		m_nUploaded += nChunk;
	}

	//m_pClient->Send( NULL );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K request

BOOL CUploadTransferED2K::CheckFinishedRequest()
{
	ASSERT( m_nState == upsUploading );

	if ( m_nPosition < m_nLength &&
		( Settings.eDonkey.EnableToday ||
		!Settings.Connection.RequireForTransfers ) )
		return FALSE;

	theApp.Message( MSG_INFO, IDS_UPLOAD_FINISHED,
		(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );

	m_oServed.push_back( Fragments::Fragment( m_nOffset, m_nOffset + m_nLength ) );
	m_pBaseFile->AddFragment( m_nOffset, m_nLength );
	m_nLength = SIZE_UNKNOWN;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K ranking update

BOOL CUploadTransferED2K::CheckRanking()
{
	DWORD tNow = GetTickCount();
	int nPosition = UploadQueues.GetPosition( this, TRUE );

	if ( nPosition < 0 )
	{
		// Invalid queue position, or queue deleted. Drop client and exit.
		Cleanup();
		Close( IDS_UPLOAD_DROPPED );
		return FALSE;
	}

	// Update 'ranking checked' timer
	m_tRankingCheck = tNow;

	// If queue ranking hasn't changed, don't bother sending an update
	// Note: if a rank was requested by the remote client, then m_nRanking will be set to -1.
	if ( m_nRanking == nPosition ) return TRUE;


	if ( nPosition == 0 )
	{
		//Ready to start uploading

		if ( m_pClient->IsOnline() )
		{
			if ( m_nState != upsUploading )
			{
				m_nState = upsRequest;
				Send( CEDPacket::New( ED2K_C2C_STARTUPLOAD ) );
			}
		}
		else
		{
			m_nState = upsConnecting;
			m_pClient->Connect();
		}

		// Update the 'request sent' timer
		m_tRequest = m_tRankingCheck;

		// Update the 'ranking sent' variables
		m_nRanking = nPosition;
		m_tRankingSent = tNow;
	}
	else if ( m_pClient->IsOnline() )
	{
		//Upload is queued

		// Check if we should send a ranking packet- If we have not sent one in a while, or one was requested
		if ( ( tNow > m_tRankingSent && tNow - m_tRankingSent >= Settings.eDonkey.QueueRankThrottle ) ||
			 (  m_nRanking == -1 ) )

		{
			// Send a queue rank packet
			CSingleLock pLock( &UploadQueues.m_pSection, TRUE );

			if ( UploadQueues.Check( m_pQueue ) )
			{
				theApp.Message( MSG_INFO, IDS_UPLOAD_QUEUED, (LPCTSTR)m_sName,
					(LPCTSTR)m_sAddress, nPosition, m_pQueue->GetQueuedCount(),
					(LPCTSTR)m_pQueue->m_sName );
			}

			pLock.Unlock();

			m_nState = upsQueued;

			if ( m_pClient->m_bEmule )
			{	//eMule queue ranking
				CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_QUEUERANKING, ED2K_PROTOCOL_EMULE );
				pPacket->WriteShortLE( WORD( nPosition ) );
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

			// Update the 'ranking sent' variables
			m_nRanking = nPosition;
			m_tRankingSent = tNow;
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
	pPacket->WriteShortLE( WORD( nPosition ) );
	Datagrams.Send( &m_pClient->m_pHost.sin_addr, m_pClient->m_nUDP, pPacket );

	m_tRequest = GetTickCount();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// CUploadTransferED2K 64bit Large file support

BOOL CUploadTransferED2K::OnRequestParts64(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount + 4 * 3 * 2 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close();
		return FALSE;
	}

	Hashes::Ed2kHash oED2K;
	pPacket->Read( oED2K );

	if ( validAndUnequal( oED2K, m_oED2K ) )
	{
		if ( ! Request( oED2K ) ) return FALSE;
	}

	if ( m_nState != upsQueued && m_nState != upsRequest && m_nState != upsUploading )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close();
		return FALSE;
	}

	QWORD nOffset[2][3];
	pPacket->Read( nOffset, sizeof(QWORD) * 2 * 3 );

	for ( int nRequest = 0 ; nRequest < 3 ; nRequest++ )
	{
		if ( nOffset[1][nRequest] <= m_nSize )
		{
			// Valid (or null) request
			if ( nOffset[0][nRequest] < nOffset[1][nRequest] )
			{
				// Add non-null ranges to the list
				AddRequest( nOffset[0][nRequest], nOffset[1][nRequest] - nOffset[0][nRequest] );
			}
		}
		else
		{
			// Invalid request- had an impossible range.
			theApp.Message( MSG_ERROR, IDS_UPLOAD_BAD_RANGE, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
			// They probably have an incorrent hash associated with a file. Calling close now
			// will send "file not found" to stop them re-asking, then close the connection.
			Close();
			return FALSE;
		}
	}

	ServeRequests();

	return TRUE;
}
