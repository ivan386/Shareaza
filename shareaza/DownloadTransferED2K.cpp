//
// DownloadTransferED2K.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadTransferED2K.h"
#include "FragmentedFile.h"
#include "Datagrams.h"
#include "EDClients.h"
#include "EDClient.h"
#include "EDPacket.h"
#include "Network.h"
#include "Buffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const DWORD BUFFER_SIZE = 8192;


//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K construction

CDownloadTransferED2K::CDownloadTransferED2K(CDownloadSource* pSource)
	: CDownloadTransfer	( pSource, PROTOCOL_ED2K )
	, m_pClient			( NULL )
	, m_bHashset		( false )
	, m_tRanking		( 0ul )
	, m_bUDP			( false )
	, m_pInflatePtr		( NULL )
	, m_pInflateBuffer	( new CBuffer() )
	, m_nInflateOffset	( 0ull )
	, m_nInflateLength	( 0ull )
	, m_nInflateRead	( 0ull )
	, m_nInflateWritten	( 0ull )
{
	ASSERT( m_pDownload->m_oED2K );
}

CDownloadTransferED2K::~CDownloadTransferED2K()
{
	ASSUME_LOCK( Transfers.m_pSection );

	// This never happens
	if ( m_pClient ) m_pClient->m_mInput.pLimit = m_pClient->m_mOutput.pLimit = NULL;

	ClearRequests();
	delete m_pInflateBuffer;

	ASSERT( m_pClient == NULL );
	ASSERT( ! EDClients.IsMyDownload( this ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K initiate

BOOL CDownloadTransferED2K::Initiate()
{
	ASSERT( m_pClient == NULL );
	ASSERT( m_nState == dtsNull );

    if ( ! m_pDownload->m_oED2K || m_pDownload->m_nSize == SIZE_UNKNOWN )
	{
		Close( TRI_FALSE );
		return FALSE;
	}

	m_pClient = EDClients.Connect(
		m_pSource->m_pAddress.S_un.S_addr,
		m_pSource->m_nPort,
		m_pSource->m_nServerPort ? &m_pSource->m_pServerAddress : NULL,
		m_pSource->m_nServerPort,
		m_pSource->m_oGUID );

	if ( m_pClient == NULL )
	{
		Close( EDClients.IsFull() ? TRI_TRUE : TRI_FALSE );
		return FALSE;
	}

	SetState( dtsConnecting );
	m_tConnected = GetTickCount();

	if ( ! m_pClient->AttachDownload( this ) )
	{
		SetState( dtsNull );
		m_pClient = NULL;
		Close( TRI_TRUE );
		return FALSE;
	}

	m_pHost			= m_pClient->m_pHost;
	m_sAddress		= m_pClient->m_sAddress;
	if( m_sAddress.IsEmpty() )
		m_sAddress	= inet_ntoa( m_pHost.sin_addr );
	UpdateCountry();
	m_pClient->m_mInput.pLimit = &m_nBandwidth;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K close

void CDownloadTransferED2K::Close(TRISTATE bKeepSource, DWORD nRetryAfter)
{
	ASSUME_LOCK( Transfers.m_pSection );

	SetState( dtsNull );

	if ( m_pClient != NULL )
	{
		m_pClient->OnDownloadClose();
		m_pClient = NULL;
	}

	CDownloadTransfer::Close( bKeepSource, nRetryAfter );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K bandwidth control

void CDownloadTransferED2K::Boost()
{
	if ( m_pClient == NULL ) return;
	m_pClient->m_mInput.pLimit = NULL;
}

DWORD CDownloadTransferED2K::GetMeasuredSpeed()
{
	// Return if there is no client
	if ( m_pClient == NULL ) return 0;

	// Calculate Input
	m_pClient->MeasureIn();

	// Return calculated speed
	return m_pClient->m_mInput.nMeasure;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K run event

BOOL CDownloadTransferED2K::OnRun()
{
	return OnRunEx( GetTickCount() );
}

BOOL CDownloadTransferED2K::OnRunEx(DWORD tNow)
{
	if ( !Network.IsConnected() || ( !Settings.eDonkey.EnableToday && Settings.Connection.RequireForTransfers ) )
	{
		Close( TRI_TRUE );
		return FALSE;
	}

	switch ( m_nState )
	{
	case dtsConnecting:
		if ( tNow > m_tConnected && tNow - m_tConnected > Settings.Connection.TimeoutConnect * 2 )
		{
			theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_CONNECT_TIMEOUT, (LPCTSTR)m_sAddress );
			Close( TRI_UNKNOWN );
			return FALSE;
		}
		break;
	case dtsRequesting:
	case dtsEnqueue:
		if ( tNow > m_tRequest && tNow - m_tRequest > Settings.Connection.TimeoutHandshake * 2 )
		{
			theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_HANDSHAKE_TIMEOUT, (LPCTSTR)m_sAddress );
			Close( TRI_UNKNOWN );
			return FALSE;
		}
		break;
	case dtsQueued:
		return RunQueued( tNow );
	case dtsDownloading:
	case dtsHashset:
		if ( tNow > m_pClient->m_mInput.tLast &&
			 tNow - m_pClient->m_mInput.tLast > Settings.Connection.TimeoutTraffic * 2 )
		{
			theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_CLOSED, (LPCTSTR)m_sAddress );
			Close( TRI_TRUE );
			return FALSE;
		}
		break;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K connection established event

BOOL CDownloadTransferED2K::OnConnected()
{
	ASSERT( m_pClient != NULL );
	ASSERT( m_pSource != NULL );

	m_pHost		= m_pClient->m_pHost;
	m_sAddress	= m_pClient->m_sAddress;
	UpdateCountry();

	m_pSource->m_oGUID		= m_pClient->m_oGUID;
	m_pSource->m_sServer	= m_sUserAgent = m_pClient->m_sUserAgent;
	m_pSource->m_sNick		= m_pClient->m_sNick;
	m_pSource->SetLastSeen();

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_CONNECTED, (LPCTSTR)m_sAddress );

	return SendPrimaryRequest();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K connection dropped event

void CDownloadTransferED2K::OnDropped()
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_nState == dtsQueued )
	{
		theApp.Message( MSG_INFO, IDS_DOWNLOAD_QUEUE_DROP,
			(LPCTSTR)m_pDownload->GetDisplayName() );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DROPPED, (LPCTSTR)m_sAddress );
		Close( TRI_UNKNOWN );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K packet handlers

BOOL CDownloadTransferED2K::OnFileReqAnswer(CEDPacket* /*pPacket*/)
{
	if ( m_pDownload->m_nSize <= ED2K_PART_SIZE )
	{
		m_pAvailable.assign( 1, true );
		m_pSource->m_oAvailable.insert( m_pSource->m_oAvailable.end(),
			Fragments::Fragment( 0, m_pDownload->m_nSize ) );
		SendSecondaryRequest();
	}
	// Not really interested
	return TRUE;
}

BOOL CDownloadTransferED2K::OnFileNotFound(CEDPacket* /*pPacket*/)
{
	theApp.Message( MSG_ERROR, IDS_DOWNLOAD_FILENOTFOUND,
		(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );

	Close( TRI_FALSE );
	return FALSE;
}

BOOL CDownloadTransferED2K::OnFileStatus(CEDPacket* pPacket)
{
	if ( m_nState <= dtsConnecting ) return TRUE;

    if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount + 2 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close( TRI_FALSE );
		return FALSE;
	}

	Hashes::Ed2kHash oED2K;
	pPacket->Read( oED2K );

	if ( validAndUnequal( oED2K, m_pDownload->m_oED2K ) )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_HASH,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
		return TRUE;
	}

	DWORD nBlocks = pPacket->ReadShortLE();

	if ( nBlocks == (DWORD)( ( m_pDownload->m_nSize + ED2K_PART_SIZE - 1 ) / ED2K_PART_SIZE ) )
	{
        m_pSource->m_oAvailable.clear();

		m_pAvailable.assign( nBlocks, false );

		for ( DWORD nBlock = 0 ; nBlock < nBlocks && pPacket->GetRemaining() ; )
		{
			BYTE nByte = pPacket->ReadByte();

			for ( int nBit = 0 ; nBit < 8 && nBlock < nBlocks ; nBit++, nBlock++ )
			{
				if ( nByte & ( 1 << nBit ) )
				{
					QWORD nFrom = ED2K_PART_SIZE * nBlock;
					QWORD nTo = nFrom + ED2K_PART_SIZE;
					nTo = min( nTo, m_pDownload->m_nSize );

					m_pSource->m_oAvailable.insert( m_pSource->m_oAvailable.end(),
						Fragments::Fragment( nFrom, nTo ) );
					m_pAvailable[ nBlock ] = true;
				}
			}
		}
	}
	else if ( nBlocks == 0 )
	{
		m_pSource->m_oAvailable.clear();

		m_pAvailable.clear();
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close( TRI_FALSE );
		return FALSE;
	}

	SendSecondaryRequest();

	return TRUE;
}

BOOL CDownloadTransferED2K::OnHashsetAnswer(CEDPacket* pPacket)
{
	if ( m_nState != dtsHashset ) return TRUE;

    if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount + 2 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close( TRI_FALSE );
		return FALSE;
	}

    Hashes::Ed2kHash oED2K;
	pPacket->Read( oED2K );

	if ( validAndUnequal( oED2K, m_pDownload->m_oED2K ) )
	{
		return TRUE;	// Hack
//		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_HASHSET_ERROR, (LPCTSTR)m_sAddress );
//		Close( TRI_FALSE );
//		return FALSE;
	}

	m_bHashset = TRUE;

	DWORD nBlocks = pPacket->ReadShortLE();
	bool bNullBlock = ( m_pDownload->m_nSize % ED2K_PART_SIZE == 0 && m_pDownload->m_nSize );
	QWORD nBlocksFromSize = ( m_pDownload->m_nSize + ED2K_PART_SIZE - 1 ) / ED2K_PART_SIZE;

	if ( bNullBlock )
		nBlocksFromSize++;

	if ( nBlocks == 0 ) nBlocks = 1;

	if ( nBlocks != nBlocksFromSize )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_HASHSET_ERROR, (LPCTSTR)m_sAddress );
	}
	else if ( m_pDownload->SetHashset(	pPacket->m_pBuffer + pPacket->m_nPosition,
										pPacket->GetRemaining() ) )
	{
		return SendSecondaryRequest();
	}

	Close( TRI_FALSE );
	return FALSE;
}

BOOL CDownloadTransferED2K::OnQueueRank(CEDPacket* pPacket)
{
	if ( m_nState <= dtsConnecting ) return TRUE;

	if ( pPacket->GetRemaining() < 4 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close( TRI_FALSE );
		return FALSE;
	}

	m_nQueuePos	= pPacket->ReadLongLE();

	if ( m_nQueuePos > 0 )
	{
		SetQueueRank( m_nQueuePos );
	}
	else
	{
		m_pSource->m_tAttempt = GetTickCount() + Settings.eDonkey.ReAskTime * 1000;
		Close( TRI_UNKNOWN );
	}

	return TRUE;
}

BOOL CDownloadTransferED2K::OnRankingInfo(CEDPacket* pPacket)
{
	if ( m_nState <= dtsConnecting ) return TRUE;

	if ( pPacket->GetRemaining() < 12 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close( TRI_FALSE );
		return FALSE;
	}

	m_nQueuePos	= pPacket->ReadShortLE();
	m_nQueueLen	= pPacket->ReadShortLE();

	SetQueueRank( m_nQueuePos );

	return TRUE;
}

BOOL CDownloadTransferED2K::OnFileComment(CEDPacket* pPacket)
{
	BYTE nFileRating;
	DWORD nLength;
	CString sFileComment;

	// Read in the file rating
	nFileRating = pPacket->ReadByte();

	nLength = pPacket->ReadLongLE();
	if ( nLength > 0 )
	{
		if ( nLength > ED2K_COMMENT_MAX ) nLength = ED2K_COMMENT_MAX;

		// Read in comment
		if ( m_pClient && m_pClient->m_bEmUnicode )
			sFileComment = pPacket->ReadStringUTF8( nLength );
		else
			sFileComment = pPacket->ReadStringASCII( nLength );
	}

	if ( m_pDownload && m_pClient )
	{
		return m_pDownload->AddReview( &m_pClient->m_pHost.sin_addr, 3, nFileRating, m_pClient->m_sNick, sFileComment );
	}

	return FALSE;
}

BOOL CDownloadTransferED2K::OnStartUpload(CEDPacket* /*pPacket*/)
{
	SetState( dtsDownloading );
	m_pClient->m_mInput.tLast = GetTickCount();

	ClearRequests();

	return SendFragmentRequests();
}

BOOL CDownloadTransferED2K::OnFinishUpload(CEDPacket* /*pPacket*/)
{
	return SendPrimaryRequest();
}

BOOL CDownloadTransferED2K::OnSendingPart(CEDPacket* pPacket)
{
    if ( pPacket->GetRemaining() <= Hashes::Ed2kHash::byteCount + 8 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close( TRI_FALSE );
		return FALSE;
	}

	Hashes::Ed2kHash oED2K;
	pPacket->Read( oED2K );

	if ( oED2K != m_pDownload->m_oED2K )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_HASH,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
		// Close( TRI_FALSE );
		// return FALSE;
		return TRUE;
	}

	QWORD nOffset = pPacket->ReadLongLE();
	QWORD nLength = pPacket->ReadLongLE();

	if ( nLength <= nOffset )
	{
		if ( nLength == nOffset ) return TRUE;
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close( TRI_FALSE );
		return FALSE;
	}

	nLength -= nOffset;

	if ( nLength > (QWORD)pPacket->GetRemaining() )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close( TRI_FALSE );
		return FALSE;
	}

	/*BOOL bUseful =*/ m_pDownload->SubmitData( nOffset, pPacket->m_pBuffer + pPacket->m_nPosition, nLength );

	m_oRequested.erase( Fragments::Fragment( nOffset, nOffset + nLength ) );

	m_pSource->AddFragment( nOffset, nLength, ( nOffset % ED2K_PART_SIZE ) ? TRUE : FALSE );

	m_nDownloaded += nLength;

	m_pSource->SetValid();

	return SendFragmentRequests();
}

BOOL CDownloadTransferED2K::OnCompressedPart(CEDPacket* pPacket)
{
    if ( pPacket->GetRemaining() <= Hashes::Ed2kHash::byteCount + 8 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close( TRI_FALSE );
		return FALSE;
	}

	Hashes::Ed2kHash oED2K;
	pPacket->Read( oED2K );

	if ( validAndUnequal( oED2K, m_pDownload->m_oED2K ) )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_HASH,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
		// Close( TRI_FALSE );
		// return FALSE;
		return TRUE;
	}

	QWORD nBaseOffset = pPacket->ReadLongLE();
	QWORD nBaseLength = pPacket->ReadLongLE();

	if ( m_pInflatePtr == NULL || m_nInflateOffset != nBaseOffset || m_nInflateLength != nBaseLength )
	{
		CBuffer::InflateStreamCleanup( m_pInflatePtr );

		m_nInflateOffset	= nBaseOffset;
		m_nInflateLength	= nBaseLength;
		m_nInflateRead		= 0;
		m_nInflateWritten	= 0;
		m_pInflateBuffer->Clear();

		m_pInflatePtr = new z_stream;
		ZeroMemory( m_pInflatePtr, sizeof(z_stream) );

		if ( inflateInit( m_pInflatePtr ) != Z_OK )
		{
			delete m_pInflatePtr;
			m_pInflatePtr = NULL;

			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_INFLATE_ERROR,
				(LPCTSTR)m_pDownload->GetDisplayName() );

			Close( TRI_FALSE );
			return FALSE;
		}
	}

	m_pInflateBuffer->Add( pPacket->m_pBuffer + pPacket->m_nPosition, pPacket->GetRemaining() );

	auto_array< BYTE > pBuffer( new BYTE[ BUFFER_SIZE ] );

	if ( m_pInflateBuffer->m_nLength > 0 && m_nInflateRead < m_nInflateLength )
	{
		m_pInflatePtr->next_in	= m_pInflateBuffer->m_pBuffer;
		m_pInflatePtr->avail_in	= m_pInflateBuffer->m_nLength;

		do
		{
			m_pInflatePtr->next_out		= pBuffer.get();
			m_pInflatePtr->avail_out	= BUFFER_SIZE;

			CBuffer::Inflate( m_pInflatePtr, Z_SYNC_FLUSH );

			if ( m_pInflatePtr->avail_out < BUFFER_SIZE )
			{
				QWORD nOffset = m_nInflateOffset + m_nInflateWritten;
				QWORD nLength = BUFFER_SIZE - m_pInflatePtr->avail_out;

				m_pDownload->SubmitData( nOffset, pBuffer.get(), nLength );

				m_oRequested.erase( Fragments::Fragment( nOffset, nOffset + nLength ) );

				m_pSource->AddFragment( nOffset, nLength,
					( nOffset % ED2K_PART_SIZE ) ? TRUE : FALSE );

				m_nDownloaded += nLength;
				m_nInflateWritten += nLength;
			}
		}
		while ( m_pInflatePtr->avail_out == 0 );

		if ( m_pInflatePtr->avail_in < m_pInflateBuffer->m_nLength )
		{
			m_nInflateRead += ( m_pInflateBuffer->m_nLength - m_pInflatePtr->avail_in );
			m_pInflateBuffer->Remove( m_pInflateBuffer->m_nLength - m_pInflatePtr->avail_in );
		}
	}

	if ( m_nInflateRead >= m_nInflateLength )
	{
		CBuffer::InflateStreamCleanup( m_pInflatePtr );
		m_pInflateBuffer->Clear();
	}

	m_pSource->SetValid();

	return SendFragmentRequests();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K send

void CDownloadTransferED2K::Send(CEDPacket* pPacket, BOOL bRelease)
{
	ASSERT( m_nState > dtsConnecting );
	ASSERT( m_pClient != NULL );
	m_pClient->Send( pPacket, bRelease );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K file requestors

BOOL CDownloadTransferED2K::SendPrimaryRequest()
{
	ASSERT( m_pClient != NULL );
	DWORD tNow = GetTickCount();

	/*
	if ( m_pDownload->GetVolumeRemaining() == 0 )
	{
		theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_END, (LPCTSTR)m_sAddress );
		Close( TRI_TRUE );
		return FALSE;
	}
	*/

	//This source is current requesting
	SetState( dtsRequesting );

	//Set the 'last requested' time
	m_tRequest	= tNow;

	ClearRequests();

	//Send ed2k file request
	CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_FILEREQUEST );
	pPacket->Write( m_pDownload->m_oED2K );

	if ( Settings.eDonkey.ExtendedRequest >= 1 && m_pClient->m_bEmRequest >= 1 )
	{
		m_pClient->WritePartStatus( pPacket, m_pDownload );
	}

	//It's not very accurate
	if ( Settings.eDonkey.ExtendedRequest >= 2 && m_pClient->m_bEmRequest >= 2 )
	{
		pPacket->WriteShortLE( (WORD) m_pDownload->GetED2KCompleteSourceCount() );
	}

	Send( pPacket );

	if ( m_pDownload->m_nSize <= ED2K_PART_SIZE )
	{
		// Don't ask for status - if the client answers, we know the file is complete anyway
	}
	else
	{
		//Send ed2k status request
		pPacket = CEDPacket::New( ED2K_C2C_FILESTATUSREQUEST );
		pPacket->Write( m_pDownload->m_oED2K );
		Send( pPacket );
	}

	// TODO: Add new option "SourceExchangePeriod" (default: 10 minutes) like BitTorrent has
	if ( ( m_pDownload->GetSourceCount() < Settings.Downloads.SourcesWanted ) &&
		 ( tNow >= m_tSourceRequest + 10 * 60 * 1000 ) &&
		 m_pClient->m_bEmule )
	{
		// Set 'last asked for sources' time
		m_tSourceRequest = tNow;
		// Send ed2k request for sources packet
		pPacket = CEDPacket::New( ED2K_C2C_REQUESTSOURCES, ED2K_PROTOCOL_EMULE );
		pPacket->Write( m_pDownload->m_oED2K );
		Send( pPacket );
	}

	return TRUE;
}

BOOL CDownloadTransferED2K::SendSecondaryRequest()
{
	ASSERT( m_pClient != NULL );
	ASSERT( m_nState > dtsConnecting );
	// ASSERT( m_nState == dtsRequesting || m_nState == dtsHashset );

	if ( ! m_pDownload->PrepareFile() )
	{
		Close( TRI_TRUE );
		return FALSE;
	}

	if ( m_bHashset == FALSE && m_pDownload->NeedHashset() )
	{
		CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_HASHSETREQUEST );
		pPacket->Write( m_pDownload->m_oED2K );
		Send( pPacket );

		SetState( dtsHashset );
		m_pClient->m_mInput.tLast = GetTickCount();
	}
	else if ( m_pSource->HasUsefulRanges() )
	{
		CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_QUEUEREQUEST );
		pPacket->Write( m_pDownload->m_oED2K );
		Send( pPacket );

		SetState( dtsEnqueue );
		m_tRequest = GetTickCount();
	}
	else
	{
		m_pSource->m_tAttempt = GetTickCount() + Settings.eDonkey.ReAskTime * 500;
		m_pSource->SetAvailableRanges( NULL );
		theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_END, (LPCTSTR)m_sAddress );
		Close( TRI_TRUE );
		return FALSE;
	}

	ClearRequests();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K fragment request manager

bool CDownloadTransferED2K::SendFragmentRequests()
{
	ASSUME_LOCK( Transfers.m_pSection );

	ASSERT( m_pClient != NULL );

	if ( m_nState != dtsDownloading )
		return true;

	if ( m_oRequested.size() >= (int)Settings.eDonkey.RequestPipe )
		return true;

	Fragments::List oPossible( m_pDownload->GetEmptyFragmentList() );

	if ( !m_pClient->m_bEmLargeFile && ( m_pDownload->m_nSize & 0xffffffff00000000 ) )
	{
		Fragments::Fragment Selected( 0x100000000, m_pDownload->m_nSize - 1 );
		oPossible.erase( Selected );
	}

	if ( ! m_pDownload->m_bTorrentEndgame )
	{
		for ( const CDownloadTransfer* pTransfer = m_pDownload->GetFirstTransfer(); pTransfer && !oPossible.empty(); pTransfer = pTransfer->m_pDlNext )
		{
			pTransfer->SubtractRequested( oPossible );
		}
	}

	typedef std::map<QWORD ,Fragments::Fragment> _TRequest;
	typedef _TRequest::iterator _TRequestIndex;
	_TRequest oRequesting;
	while ( m_oRequested.size() < (int)Settings.eDonkey.RequestPipe )
	{
		QWORD nOffset, nLength;

		if ( SelectFragment( oPossible, nOffset, nLength, m_pDownload->m_bTorrentEndgame ) )
		{
			ChunkifyRequest( &nOffset, &nLength, Settings.eDonkey.RequestSize, FALSE );

			Fragments::Fragment Selected( nOffset, nOffset + nLength );
			oPossible.erase( Selected );

			m_oRequested.push_back( Selected );

			oRequesting.insert( _TRequest::value_type(nOffset, Selected) );

		}
		else
		{
			break;
		}
	}

	while ( !oRequesting.empty() )
	{
		DWORD nCount=0;
		QWORD nOffsetBegin[3]={0,0,0}, nOffsetEnd[3]={0,0,0};
		bool  bI64Offset = false;

		while ( nCount < 3 && !oRequesting.empty() )
		{
			_TRequestIndex iIndex = oRequesting.begin();
			nOffsetBegin[nCount] = QWORD((*iIndex).second.begin());
			nOffsetEnd[nCount] = QWORD((*iIndex).second.end());
			bI64Offset |= ( ( ( nOffsetBegin[nCount] & 0xffffffff00000000 ) ) ||
						( ( nOffsetEnd[nCount] & 0xffffffff00000000 ) ) );
			oRequesting.erase(iIndex);
			nCount++;
		}

		if ( bI64Offset )
		{
			CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_REQUESTPARTS_I64, ED2K_PROTOCOL_EMULE );
			pPacket->Write( m_pDownload->m_oED2K );

			/* this commented out code is for BigEndian, only needed when ported to different platform.
			pPacket->WriteLongLE( (DWORD)( nOffsetBegin[0] & 0x00000000ffffffff ) );
			pPacket->WriteLongLE( (DWORD)( ( nOffsetBegin[0] & 0xffffffff00000000 ) >> 32 ) );
			pPacket->WriteLongLE( (DWORD)( nOffsetBegin[1] & 0x00000000ffffffff ) );
			pPacket->WriteLongLE( (DWORD)( ( nOffsetBegin[1] & 0xffffffff00000000 ) >> 32 ) );
			pPacket->WriteLongLE( (DWORD)( nOffsetBegin[2] & 0x00000000ffffffff ) );
			pPacket->WriteLongLE( (DWORD)( ( nOffsetBegin[2] & 0xffffffff00000000 ) >> 32 ) );
			pPacket->WriteLongLE( (DWORD)( nOffsetEnd[0] & 0x00000000ffffffff ) );
			pPacket->WriteLongLE( (DWORD)( ( nOffsetEnd[0] & 0xffffffff00000000 ) >> 32 ) );
			pPacket->WriteLongLE( (DWORD)( nOffsetEnd[1] & 0x00000000ffffffff ) );
			pPacket->WriteLongLE( (DWORD)( ( nOffsetEnd[1] & 0xffffffff00000000 ) >> 32 ) );
			pPacket->WriteLongLE( (DWORD)( nOffsetEnd[2] & 0x00000000ffffffff ) );
			pPacket->WriteLongLE( (DWORD)( ( nOffsetEnd[2] & 0xffffffff00000000 ) >> 32 ) );
			*/

			// If little Endian, no need to use above code
			pPacket->Write( &nOffsetBegin[0], 8 );
			pPacket->Write( &nOffsetBegin[1], 8 );
			pPacket->Write( &nOffsetBegin[2], 8 );
			pPacket->Write( &nOffsetEnd[0], 8 );
			pPacket->Write( &nOffsetEnd[1], 8 );
			pPacket->Write( &nOffsetEnd[2], 8 );
			Send( pPacket );
		}
		else
		{
			CEDPacket* pPacket = CEDPacket::New( ED2K_C2C_REQUESTPARTS );
			pPacket->Write( m_pDownload->m_oED2K );

			/* this commented out code is for BigEndian, only needed when ported to different platform.
			pPacket->WriteLongLE( (DWORD)( nOffsetBegin[0] & 0x00000000ffffffff ) );
			pPacket->WriteLongLE( (DWORD)( nOffsetBegin[1] & 0x00000000ffffffff ) );
			pPacket->WriteLongLE( (DWORD)( nOffsetBegin[2] & 0x00000000ffffffff ) );
			pPacket->WriteLongLE( (DWORD)( nOffsetEnd[0] & 0x00000000ffffffff ) );
			pPacket->WriteLongLE( (DWORD)( nOffsetEnd[1] & 0x00000000ffffffff ) );
			pPacket->WriteLongLE( (DWORD)( nOffsetEnd[2] & 0x00000000ffffffff ) );
			*/

			pPacket->Write( &nOffsetBegin[0], 4 );
			pPacket->Write( &nOffsetBegin[1], 4 );
			pPacket->Write( &nOffsetBegin[2], 4 );
			pPacket->Write( &nOffsetEnd[0], 4 );
			pPacket->Write( &nOffsetEnd[1], 4 );
			pPacket->Write( &nOffsetEnd[2], 4 );
			Send( pPacket );
		}

		while ( nCount-- )
		{
			int nType = ( m_nDownloaded == 0 || ( nOffsetBegin[nCount] % ED2K_PART_SIZE ) == 0 )
				? MSG_INFO : MSG_DEBUG;

			theApp.Message( (WORD)nType, IDS_DOWNLOAD_FRAGMENT_REQUEST,
				nOffsetBegin[nCount], nOffsetEnd[nCount],
				(LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress );
		}
	}

	// If there are no more possible chunks to request, and endgame is available but not active
	if ( oPossible.empty() && Settings.eDonkey.Endgame && ! m_pDownload->m_bTorrentEndgame )
	{
		// And the file is at least 100MB, with less than 1MB to go
		if ( ( m_pDownload->GetVolumeComplete() > 100*1024*1024 ) &&
			 ( m_pDownload->GetVolumeRemaining() <  1*1024*1024 ) )
		{
			// Then activate endgame
			m_pDownload->m_bTorrentEndgame = true;
			theApp.Message( MSG_DEBUG, _T("Activating endgame for ed2k transfer %s"), (LPCTSTR)m_pDownload->m_sName );
		}
	}

	if ( !m_oRequested.empty() )
		return true;

	Send( CEDPacket::New( ED2K_C2C_QUEUERELEASE ) );

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_END, (LPCTSTR)m_sAddress );
	Close( TRI_TRUE );

	return false;
}

void CDownloadTransferED2K::ClearRequests()
{
	m_oRequested.clear();

	CBuffer::InflateStreamCleanup( m_pInflatePtr );
	m_pInflateBuffer->Clear();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K subtract requested fragments

BOOL CDownloadTransferED2K::SubtractRequested(Fragments::List& ppFragments) const
{
	if ( m_nState != dtsDownloading ) return FALSE;
	ppFragments.erase( m_oRequested.begin(), m_oRequested.end() );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K run queued state

BOOL CDownloadTransferED2K::RunQueued(DWORD tNow)
{
	ASSERT( m_pClient != NULL );
	ASSERT( m_nState == dtsQueued );

	if ( Settings.Downloads.QueueLimit > 0 && m_nQueuePos > Settings.Downloads.QueueLimit )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_QUEUE_HUGE,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName(), m_nQueuePos );
		Close( TRI_FALSE );
		return FALSE;
	}
	else if ( m_pClient->m_bConnected == FALSE && tNow > m_tRanking && tNow - m_tRanking > Settings.eDonkey.ReAskTime * 1000 + 20000 )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_QUEUE_TIMEOUT,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
		Close( TRI_UNKNOWN );
		return FALSE;
	}
	else if ( !( CEDPacket::IsLowID( m_pSource->m_pAddress.S_un.S_addr ) || m_pSource->m_bPushOnly ) &&
				/*!Network.IsFirewalled(CHECK_BOTH)*/!Network.IsFirewalled(CHECK_UDP) && m_pClient->m_nUDP > 0 && ! m_bUDP && tNow > m_tRequest && // Temp disable
				tNow - m_tRequest > Settings.eDonkey.ReAskTime * 1000 - 20000 )
	{
		CEDPacket* pPing = CEDPacket::New( ED2K_C2C_UDP_REASKFILEPING, ED2K_PROTOCOL_EMULE );
		pPing->Write( m_pDownload->m_oED2K );
		Datagrams.Send( &m_pClient->m_pHost.sin_addr, m_pClient->m_nUDP, pPing );
		m_bUDP = TRUE;
		//m_tRequest = GetTickCount();
	}
	else if ( tNow > m_tRequest && tNow - m_tRequest > Settings.eDonkey.ReAskTime * 1000 )
	{
		m_tRequest = GetTickCount();

		if ( m_pClient->IsOnline() )
		{
			return OnConnected();
		}
		else
		{
			m_pClient->Connect();
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K queue rank update

void CDownloadTransferED2K::SetQueueRank(int nRank)
{
	m_tRequest	= m_tRanking = GetTickCount();
	m_nQueuePos	= nRank;
	m_bUDP		= FALSE;

	SetState( dtsQueued );
	ClearRequests();

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_QUEUED,
		(LPCTSTR)m_sAddress, m_nQueuePos, m_nQueueLen,
		(LPCTSTR)m_sQueueName );
}


//////////////////////////////////////////////////////////////////////
// CDownloadTransferED2K 64Bit Large File supports

BOOL CDownloadTransferED2K::OnSendingPart64(CEDPacket* pPacket)
{
	//if ( m_nState != dtsDownloading ) return TRUE;

	if ( pPacket->GetRemaining() <= Hashes::Ed2kHash::byteCount + 16 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close( TRI_FALSE );
		return FALSE;
	}

	Hashes::Ed2kHash oED2K;
	pPacket->Read( oED2K );

	if ( oED2K != m_pDownload->m_oED2K )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_HASH,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
		// Close( TRI_FALSE );
		// return FALSE;
		return TRUE;
	}

	QWORD	nOffset = pPacket->ReadLongLE();
			nOffset = ( (QWORD)pPacket->ReadLongLE() << 32 ) | nOffset;

	QWORD	nLength = pPacket->ReadLongLE();
			nLength = ( (QWORD)pPacket->ReadLongLE() << 32 ) | nLength;

	if ( nLength <= nOffset )
	{
		if ( nLength == nOffset ) return TRUE;
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close( TRI_FALSE );
		return FALSE;
	}

	nLength -= nOffset;

	if ( nLength > (QWORD)pPacket->GetRemaining() )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close( TRI_FALSE );
		return FALSE;
	}

	/*BOOL bUseful =*/ m_pDownload->SubmitData( nOffset, pPacket->m_pBuffer + pPacket->m_nPosition, nLength );

	m_oRequested.erase( Fragments::Fragment( nOffset, nOffset + nLength ) );

	m_pSource->AddFragment( nOffset, nLength, ( nOffset % ED2K_PART_SIZE ) ? TRUE : FALSE );

	m_nDownloaded += nLength;

	m_pSource->SetValid();

	return SendFragmentRequests();
}

BOOL CDownloadTransferED2K::OnCompressedPart64(CEDPacket* pPacket)
{
	//if ( m_nState != dtsDownloading ) return TRUE;

	if ( pPacket->GetRemaining() <= Hashes::Ed2kHash::byteCount + 16 )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_BAD_PACKET, (LPCTSTR)m_sAddress, pPacket->m_nType );
		Close( TRI_FALSE );
		return FALSE;
	}

	Hashes::Ed2kHash oED2K;
	pPacket->Read( oED2K );

	if ( validAndUnequal( oED2K, m_pDownload->m_oED2K ) )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_HASH,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
		// Close( TRI_FALSE );
		// return FALSE;
		return TRUE;
	}

	QWORD	nBaseOffset = pPacket->ReadLongLE();
			nBaseOffset = ( (QWORD)pPacket->ReadLongLE() << 32 ) | nBaseOffset;

	QWORD	nBaseLength = pPacket->ReadLongLE();	// Length of compressed data is 32bit


	if ( m_pInflatePtr == NULL || m_nInflateOffset != nBaseOffset || m_nInflateLength != nBaseLength )
	{
		CBuffer::InflateStreamCleanup( m_pInflatePtr );

		m_nInflateOffset	= nBaseOffset;
		m_nInflateLength	= nBaseLength;
		m_nInflateRead		= 0;
		m_nInflateWritten	= 0;
		m_pInflateBuffer->Clear();

		m_pInflatePtr = new z_stream;
		ZeroMemory( m_pInflatePtr, sizeof(z_stream) );

		if ( inflateInit( m_pInflatePtr ) != Z_OK )
		{
			delete m_pInflatePtr;
			m_pInflatePtr = NULL;

			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_INFLATE_ERROR,
				(LPCTSTR)m_pDownload->GetDisplayName() );

			Close( TRI_FALSE );
			return FALSE;
		}
	}

	m_pInflateBuffer->Add( pPacket->m_pBuffer + pPacket->m_nPosition, pPacket->GetRemaining() );

	auto_array< BYTE > pBuffer( new BYTE[ BUFFER_SIZE ] );

	if ( m_pInflateBuffer->m_nLength > 0 && m_nInflateRead < m_nInflateLength )
	{
		m_pInflatePtr->next_in	= m_pInflateBuffer->m_pBuffer;
		m_pInflatePtr->avail_in	= m_pInflateBuffer->m_nLength;

		do
		{
			m_pInflatePtr->next_out		= pBuffer.get();
			m_pInflatePtr->avail_out	= BUFFER_SIZE;

			CBuffer::Inflate( m_pInflatePtr, Z_SYNC_FLUSH );

			if ( m_pInflatePtr->avail_out < BUFFER_SIZE )
			{
				QWORD nOffset = m_nInflateOffset + m_nInflateWritten;
				QWORD nLength = BUFFER_SIZE - m_pInflatePtr->avail_out;

				m_pDownload->SubmitData( nOffset, pBuffer.get(), nLength );

				m_oRequested.erase( Fragments::Fragment( nOffset, nOffset + nLength ) );

				m_pSource->AddFragment( nOffset, nLength,
					( nOffset % ED2K_PART_SIZE ) ? TRUE : FALSE );

				m_nDownloaded += nLength;
				m_nInflateWritten += nLength;
			}
		}
		while ( m_pInflatePtr->avail_out == 0 );

		if ( m_pInflatePtr->avail_in < m_pInflateBuffer->m_nLength )
		{
			m_nInflateRead += ( m_pInflateBuffer->m_nLength - m_pInflatePtr->avail_in );
			m_pInflateBuffer->Remove( m_pInflateBuffer->m_nLength - m_pInflatePtr->avail_in );
		}
	}

	if ( m_nInflateRead >= m_nInflateLength )
	{
		CBuffer::InflateStreamCleanup( m_pInflatePtr );
		m_pInflateBuffer->Clear();
	}

	m_pSource->SetValid();

	return SendFragmentRequests();
}
