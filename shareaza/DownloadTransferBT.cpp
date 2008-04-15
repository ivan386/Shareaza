//
// DownloadTransferBT.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "BTClients.h"
#include "BTClient.h"
#include "BTPacket.h"
#include "Download.h"
#include "Downloads.h"
#include "DownloadSource.h"
#include "DownloadTransferBT.h"
#include "FragmentedFile.h"
#include "Network.h"
#include "Buffer.h"
#include "BENode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT construction

CDownloadTransferBT::CDownloadTransferBT(CDownloadSource* pSource, CBTClient* pClient) : CDownloadTransfer( pSource, PROTOCOL_BT )
{
	ASSERT( m_pDownload->IsTorrent() );
	ASSERT( m_pDownload->m_nSize != SIZE_UNKNOWN );
	
	m_pClient			= pClient;
	m_nState			= pClient ? dtsConnecting : dtsNull;
	m_sUserAgent		= _T("BitTorrent");
	
	m_bChoked			= TRUE;
	m_bInterested		= FALSE;
	
	m_pAvailable		= NULL;
	
	m_tRunThrottle		= 0;
	m_tSourceRequest	= GetTickCount();
}

CDownloadTransferBT::~CDownloadTransferBT()
{
	ASSERT( m_pClient == NULL );

	// This never happens
	if ( m_pClient ) m_pClient->m_mInput.pLimit = m_pClient->m_mOutput.pLimit = NULL;

	if ( m_pAvailable != NULL ) delete [] m_pAvailable;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT initiate

BOOL CDownloadTransferBT::Initiate()
{
	ASSERT( m_pClient == NULL );
	ASSERT( m_nState == dtsNull );
	m_pClient = new CBTClient();
	if ( ! m_pClient->Connect( this ) )
	{
		delete m_pClient;
		m_pClient = NULL;
		Close( TRI_FALSE );
		return FALSE;
	}
	SetState( dtsConnecting );
	m_tConnected	= GetTickCount();
	m_pHost			= m_pClient->m_pHost;
	m_sAddress		= m_pClient->m_sAddress;
	UpdateCountry();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT close

void CDownloadTransferBT::Close(TRISTATE bKeepSource, DWORD nRetryAfter)
{
	if ( m_pClient != NULL )
	{
		if ( Settings.General.DebugBTSources )
			theApp.Message( MSG_DEBUG, L"Closing BT download connection: %s", m_pClient->m_sAddress );

		m_pClient->m_pDownloadTransfer = NULL;
		if ( m_pClient->IsOnline() )
		{
			m_pClient->m_mInput.pLimit = &Settings.Bandwidth.Request;
			m_pClient->Send( CBTPacket::New( BT_PACKET_NOT_INTERESTED ) );
		}
		else
		{
			m_pClient->Close();
		}
		m_pClient = NULL;
	}
	CDownloadTransfer::Close( bKeepSource, nRetryAfter );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT bandwidth control

void CDownloadTransferBT::Boost()
{
	if ( m_pClient == NULL ) return;
	m_pClient->m_mInput.pLimit = NULL;
}

DWORD CDownloadTransferBT::GetAverageSpeed()
{
	return m_pSource->m_nSpeed = GetMeasuredSpeed();
}

DWORD CDownloadTransferBT::GetMeasuredSpeed()
{
	// Return if there is no client
	if ( m_pClient == NULL ) return 0;

	// Calculate Input
	m_pClient->MeasureIn();

	// Return calculated speed
	return m_pClient->m_mInput.nMeasure;
}

CString CDownloadTransferBT::GetStateText(BOOL bLong)
{
	if ( m_nState == dtsTorrent )
	{
		CString str;
		if ( ! m_bInterested ) LoadString( str, IDS_STATUS_UNINTERESTED );
		else if ( m_bChoked ) LoadString( str, IDS_STATUS_CHOKED );
		else LoadString( str, IDS_STATUS_REQUESTING );
		return str;
	}
	return CDownloadTransfer::GetStateText( bLong );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT send packet helper

void CDownloadTransferBT::Send(CBTPacket* pPacket, BOOL bRelease)
{
	ASSERT( m_pClient != NULL );
	m_pClient->Send( pPacket, bRelease );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT run event

BOOL CDownloadTransferBT::OnRun()
{
	DWORD tNow = GetTickCount();
	if ( tNow - m_tRunThrottle >= 2000 )
	{
		m_tRunThrottle = tNow;
		ShowInterest();
		if ( m_nState == dtsTorrent || m_nState == dtsRequesting || m_nState == dtsDownloading )
		{
			if ( ! SendRequests() ) return FALSE;
		}
	}
	if ( m_pClient->m_bExchange && tNow - m_tSourceRequest >= Settings.BitTorrent.SourceExchangePeriod * 60000 )
	{
		Send( CBTPacket::New( BT_PACKET_SOURCE_REQUEST ) );
		m_tSourceRequest = tNow;
	}
	return CDownloadTransfer::OnRun();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT connection established event

BOOL CDownloadTransferBT::OnConnected()
{
	ASSERT( m_pClient != NULL );
	ASSERT( m_pSource != NULL );

	SetState( dtsTorrent );
	m_pHost		= m_pClient->m_pHost;
	m_sAddress	= m_pClient->m_sAddress;
	UpdateCountry();

	// not deleting source for source exchange.
	if ( m_pDownload->IsCompleted() )
	{
		// This source is only here to push start torrent uploads. (We don't want to download)
		m_bInterested = FALSE;
		theApp.Message( MSG_INFO, _T("Initiated push start for upload to %s"), (LPCTSTR)m_sAddress );
	}
	else
	{
		// Regular download
		m_pClient->m_mInput.pLimit = &m_nBandwidth;
		theApp.Message( MSG_INFO, IDS_DOWNLOAD_CONNECTED, (LPCTSTR)m_sAddress );
		if ( ! m_pDownload->PrepareFile() )
		{
			Close( TRI_TRUE );
			return FALSE;
		}
		m_pClient->m_mInput.pLimit = &m_nBandwidth;
	}

	m_pSource->SetLastSeen();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT bitfields

BOOL CDownloadTransferBT::OnBitfield(CBTPacket* pPacket)
{
	QWORD nBlockSize	= m_pDownload->m_pTorrent.m_nBlockSize;
	DWORD nBlockCount	= m_pDownload->m_pTorrent.m_nBlockCount;
	
	m_pSource->m_oAvailable.clear();
	
	if ( m_pAvailable != NULL ) delete [] m_pAvailable;
	m_pAvailable = NULL;
	
	if ( nBlockSize == 0 || nBlockCount == 0 ) return TRUE;
	
	m_pAvailable = new BYTE[ nBlockCount ];
	ZeroMemory( m_pAvailable, nBlockCount );
	
	for ( DWORD nBlock = 0 ; nBlock < nBlockCount && pPacket->GetRemaining() ; )
	{
		BYTE nByte = pPacket->ReadByte();
		
		for ( int nBit = 7 ; nBit >= 0 && nBlock < nBlockCount ; nBit--, nBlock++ )
		{
			if ( nByte & ( 1 << nBit ) )
			{
				QWORD nOffset = nBlockSize * nBlock;
				QWORD nLength = min( nBlockSize, m_pDownload->m_nSize - nOffset );
				m_pSource->m_oAvailable.insert( m_pSource->m_oAvailable.end(),
					Fragments::Fragment( nOffset, nOffset + nLength ) );
				m_pAvailable[ nBlock ] = TRUE;
			}
		}
	}
	
	ShowInterest();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT have block updates

void CDownloadTransferBT::SendFinishedBlock(DWORD nBlock)
{
	if ( m_pClient == NULL || ! m_pClient->IsOnline() ) return;
	CBTPacket* pPacket = CBTPacket::New( BT_PACKET_HAVE );
	pPacket->WriteLongBE( nBlock );
	Send( pPacket );
}

BOOL CDownloadTransferBT::OnHave(CBTPacket* pPacket)
{
	if ( pPacket->GetRemaining() != sizeof(int) ) return TRUE;
	QWORD nBlockSize	= m_pDownload->m_pTorrent.m_nBlockSize;
	DWORD nBlockCount	= m_pDownload->m_pTorrent.m_nBlockCount;
	DWORD nBlock		= pPacket->ReadLongBE();
	if ( nBlock >= nBlockCount ) return TRUE;
	QWORD nOffset = nBlockSize * nBlock;
	QWORD nLength = min( nBlockSize, m_pDownload->m_nSize - nOffset );
	m_pSource->m_oAvailable.insert( Fragments::Fragment( nOffset, nOffset + nLength ) );
	
	if ( m_pAvailable == NULL )
	{
		m_pAvailable = new BYTE[ nBlockCount ];
		ZeroMemory( m_pAvailable, nBlockCount );
	}
	
	m_pAvailable[ nBlock ] = TRUE;
	ShowInterest();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT interest control

void CDownloadTransferBT::ShowInterest()
{
	BOOL bInterested = FALSE;
	
	// TODO: Use an algorithm similar to CDownloadWithTiger::FindNext.., rather
	// than relying on that algorithm to complete verifications here.
	
	if ( m_pAvailable == NULL )
	{
		// Never interested if we don't know what they have
		// bInterested = m_pDownload->GetVolumeRemaining() != 0;
	}
	else if ( QWORD nBlockSize = m_pDownload->m_pTorrent.m_nBlockSize )
	{
		for ( Fragments::List::const_iterator pFragment = m_pDownload->GetEmptyFragmentList().begin();
			!bInterested && pFragment != m_pDownload->GetEmptyFragmentList().end(); ++pFragment )
		{
			DWORD nBlock = DWORD( pFragment->begin() / nBlockSize );

			for ( DWORD nEnd = DWORD( ( pFragment->end() - 1 ) / nBlockSize );
				nBlock <= nEnd; ++nBlock )
			{
				if ( m_pAvailable[ nBlock ] )
				{
					bInterested = TRUE;
					break;
				}
			}
		}
	}
	
	if ( bInterested != m_bInterested )
	{
		m_bInterested = bInterested;
		Send( CBTPacket::New( bInterested ? BT_PACKET_INTERESTED : BT_PACKET_NOT_INTERESTED ) );
		
		if ( ! bInterested )
		{
            m_oRequested.clear();
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT choking

BOOL CDownloadTransferBT::OnChoked(CBTPacket* /*pPacket*/)
{
	if ( m_bChoked ) return TRUE;
	m_bChoked = TRUE;
	SetState( dtsTorrent );
	theApp.Message( MSG_DEBUG, _T("Download from %s was choked."), (LPCTSTR)m_sAddress );
	/*for ( Fragments::Queue::const_iterator pFragment = m_oRequested.begin();
		pFragment != m_oRequested.end() ; ++pFragment )
	{
		CBTPacket* pPacket = CBTPacket::New( BT_PACKET_CANCEL );
		pPacket->WriteLongBE( (DWORD)( pFragment->begin() / m_pDownload->m_pTorrent.m_nBlockSize ) );
		pPacket->WriteLongBE( (DWORD)( pFragment->begin() % m_pDownload->m_pTorrent.m_nBlockSize ) );
		pPacket->WriteLongBE( (DWORD)pFragment->size() );
		Send( pPacket );
	}*/
	m_oRequested.clear();
	return TRUE;
}

BOOL CDownloadTransferBT::OnUnchoked(CBTPacket* /*pPacket*/)
{
	m_bChoked = FALSE;
	SetState( dtsTorrent );
	m_oRequested.clear();
	
	theApp.Message( MSG_DEBUG, _T("Download from %s was Unchoked."), (LPCTSTR)m_sAddress );
	
	return SendRequests();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT request pipe

BOOL CDownloadTransferBT::SendRequests()
{
	ASSERT( m_nState == dtsTorrent || m_nState == dtsRequesting || m_nState == dtsDownloading );
	if ( m_bChoked || ! m_bInterested )
	{
		if ( m_oRequested.empty() ) SetState( dtsTorrent );
		return TRUE;
	}
	if ( m_oRequested.size() >= (int)Settings.BitTorrent.RequestPipe )
	{
		if ( m_nState != dtsDownloading ) 
		{
			theApp.Message( MSG_DEBUG, L"Too many requests per host, staying in the requested state" );
			SetState( dtsRequesting );
		}
		return TRUE;
	}
	QWORD nBlockSize = m_pDownload->m_pTorrent.m_nBlockSize;
	ASSERT( nBlockSize != 0 );
	if ( nBlockSize == 0 ) return TRUE;
	
	Fragments::List oPossible( m_pDownload->GetEmptyFragmentList() );
	
	if ( ! m_pDownload->m_bTorrentEndgame )
	{
		for ( CDownloadTransfer* pTransfer = m_pDownload->GetFirstTransfer() ; pTransfer && !oPossible.empty() ; pTransfer = pTransfer->m_pDlNext )
		{
			pTransfer->SubtractRequested( oPossible );
		}
	}
	while ( m_oRequested.size() < (int)Settings.BitTorrent.RequestPipe )
	{
		QWORD nOffset, nLength;
		if ( SelectFragment( oPossible, nOffset, nLength ) )
		{
			ChunkifyRequest( &nOffset, &nLength, Settings.BitTorrent.RequestSize, FALSE );
			
			Fragments::Fragment Selected( nOffset, nOffset + nLength );
			oPossible.erase( Selected );
			
			m_oRequested.push_back( Selected );
			
			int nType	= ( m_nDownloaded == 0 || ( nOffset % nBlockSize ) == 0 )
						? MSG_INFO : MSG_DEBUG;
			theApp.Message( nType, IDS_DOWNLOAD_FRAGMENT_REQUEST,
				nOffset, nOffset + nLength - 1,
				(LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress );
#ifdef _DEBUG
			DWORD ndBlock1 = (DWORD)( nOffset / nBlockSize );
			DWORD ndBlock2 = (DWORD)( ( nOffset + nLength - 1 ) / nBlockSize );
			ASSERT( ndBlock1 < m_pDownload->m_pTorrent.m_nBlockCount );
			ASSERT( ndBlock1 == ndBlock2 );
			ASSERT( nLength <= nBlockSize );
#endif
			CBTPacket* pPacket = CBTPacket::New( BT_PACKET_REQUEST );
			pPacket->WriteLongBE( (DWORD)( nOffset / nBlockSize ) );
			pPacket->WriteLongBE( (DWORD)( nOffset % nBlockSize ) );
			pPacket->WriteLongBE( (DWORD)nLength );
			Send( pPacket );
		}
		else
		{
			break;
		}
	}
	// If there are no more possible chunks to request, and endgame is available but not active
	if ( oPossible.empty() && Settings.BitTorrent.Endgame && ! m_pDownload->m_bTorrentEndgame )
	{
		// And the torrent is at least 95% complete
		if ( m_pDownload->GetProgress() > 95.0f )
		{
			// Then activate endgame
			m_pDownload->m_bTorrentEndgame = TRUE;
			theApp.Message( MSG_DEBUG, _T("Torrent EndGame mode activated for %s"), m_pDownload->m_sDisplayName );
		}
	}
	
	if ( !m_oRequested.empty() && m_nState != dtsDownloading )
	{
		theApp.Message( MSG_DEBUG, L"Request for piece sent, switching to the requested state" );
		SetState( dtsRequesting );
	}
	if ( m_oRequested.empty() ) 
		SetState( dtsTorrent );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT fragment selection

BOOL CDownloadTransferBT::SelectFragment(const Fragments::List& oPossible, QWORD& nOffset, QWORD& nLength)
{
	Fragments::Fragment oSelection(
		selectBlock( oPossible, m_pDownload->m_pTorrent.m_nBlockSize, m_pAvailable ) );

	if ( oSelection.size() == 0 ) return FALSE;

	nOffset = oSelection.begin();
	nLength = oSelection.size();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT multi-source fragment handling

BOOL CDownloadTransferBT::SubtractRequested(Fragments::List& ppFragments)
{
	if ( m_oRequested.empty() || m_bChoked ) 
		return FALSE;
	ppFragments.erase( m_oRequested.begin(), m_oRequested.end() );
	return TRUE;
}

BOOL CDownloadTransferBT::UnrequestRange(QWORD nOffset, QWORD nLength)
{
	if ( m_oRequested.empty() ) 
		return FALSE;
	ASSERT( m_pDownload->m_pTorrent.m_nBlockSize != 0 );
	if ( m_pDownload->m_pTorrent.m_nBlockSize == 0 ) return FALSE;

	Fragments::Queue oUnrequests = extract_range( m_oRequested,
		Fragments::Fragment( nOffset, nOffset + nLength ) );

	for ( Fragments::Queue::const_iterator pFragment = oUnrequests.begin();
		pFragment != oUnrequests.end(); ++pFragment )
	{
		CBTPacket* pPacket = CBTPacket::New( BT_PACKET_CANCEL );
		pPacket->WriteLongBE( (DWORD)( pFragment->begin() / m_pDownload->m_pTorrent.m_nBlockSize ) );
		pPacket->WriteLongBE( (DWORD)( pFragment->begin() % m_pDownload->m_pTorrent.m_nBlockSize ) );
		pPacket->WriteLongBE( (DWORD)pFragment->size() );
		Send( pPacket );
	}

	return !oUnrequests.empty();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT piece reception

BOOL CDownloadTransferBT::OnPiece(CBTPacket* pPacket)
{
	ASSERT( m_pClient != NULL );
	if ( pPacket->GetRemaining() < 8 ) return TRUE;
	if ( m_nState != dtsRequesting && m_nState != dtsDownloading ) return TRUE;
	SetState( dtsDownloading );
	DWORD nBlock	= pPacket->ReadLongBE();
	QWORD nOffset	= pPacket->ReadLongBE();
	QWORD nLength	= pPacket->GetRemaining();
	nOffset += (QWORD)nBlock * m_pDownload->m_pTorrent.m_nBlockSize;
	m_nDownloaded += nLength;
	m_pDownload->m_nTorrentDownloaded += nLength;
	m_pDownload->m_pTorrent.m_nTotalDownload += nLength;
	m_pSource->AddFragment( nOffset, nLength );
	m_pSource->SetValid();
	m_oRequested.erase( Fragments::Fragment( nOffset, nOffset + nLength ) );
	
	m_pDownload->SubmitData( nOffset,
		pPacket->m_pBuffer + pPacket->m_nPosition, nLength );
	// TODO: SendRequests and ShowInterest could be combined.. SendRequests
	// is probably going to tell us if we are interested or not
	ShowInterest();
	return SendRequests();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT source exchange

BOOL CDownloadTransferBT::OnSourceResponse(CBTPacket* pPacket)
{
	CBuffer pInput;
	pInput.Add( pPacket->m_pBuffer, pPacket->GetRemaining() );
	CBENode* pRoot = CBENode::Decode( &pInput );
	if ( pRoot == NULL ) return TRUE;
	CBENode* pPeers = pRoot->GetNode( "peers" );
	if ( ! pPeers->IsType( CBENode::beList ) )
	{
		delete pRoot;
		return TRUE;
	}
	
	int nCount = 0;
	
	for ( int nPeer = 0 ; nPeer < pPeers->GetCount() ; nPeer++ )
	{
		CBENode* pPeer = pPeers->GetNode( nPeer );
		if ( ! pPeer->IsType( CBENode::beDict ) ) continue;
		
		CBENode* pURL = pPeer->GetNode( "url" );
		
		if ( pURL->IsType( CBENode::beString ) )
		{
			nCount += m_pDownload->AddSourceURL( pURL->GetString(), TRUE );
		}
		else
		{
			CBENode* pID = pPeer->GetNode( "peer id" );
            if ( ! pID->IsType( CBENode::beString ) || pID->m_nValue != Hashes::BtGuid::byteCount ) continue;
			
			CBENode* pIP = pPeer->GetNode( "ip" );
			if ( ! pIP->IsType( CBENode::beString ) ) continue;
			
			CBENode* pPort = pPeer->GetNode( "port" );
			if ( ! pPort->IsType( CBENode::beInt ) ) continue;
			
			SOCKADDR_IN saPeer;
			if ( ! Network.Resolve( pIP->GetString(), (int)pPort->GetInt(), &saPeer ) ) continue;
			
			theApp.Message( MSG_DEBUG, _T("CDownloadTransferBT::OnSourceResponse(): %s: %s:%i"),
				(LPCTSTR)m_sAddress,
				(LPCTSTR)CString( inet_ntoa( saPeer.sin_addr ) ), htons( saPeer.sin_port ) );
			
			Hashes::BtGuid tmp( *static_cast< const Hashes::BtGuid::RawStorage* >(
				pID->m_pValue ) );
			nCount += m_pDownload->AddSourceBT( tmp,
				&saPeer.sin_addr, htons( saPeer.sin_port ) );
		}
	}
	
	delete pRoot;
	
	theApp.Message( MSG_INFO, IDS_BT_CLIENT_EXCHANGE, nCount, (LPCTSTR)m_sAddress );
	
	return TRUE;
}
