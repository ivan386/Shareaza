//
// DownloadTransferBT.cpp
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
	ASSERT( m_pDownload->m_bBTH );
	ASSERT( m_pDownload->m_nSize != SIZE_UNKNOWN );
	
	m_pClient			= pClient;
	m_nState			= pClient ? dtsConnecting : dtsNull;
	m_sUserAgent		= _T("BitTorrent");
	
	m_bChoked			= TRUE;
	m_bInterested		= FALSE;
	
	m_pAvailable		= NULL;
	m_pRequested		= NULL;
	m_nRequested		= 0;
	
	m_tRunThrottle		= 0;
	m_tSourceRequest	= GetTickCount();
}

CDownloadTransferBT::~CDownloadTransferBT()
{
	ASSERT( m_pClient == NULL );
	m_pRequested->DeleteChain();
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
		
		Close( TS_FALSE );
		return FALSE;
	}
	
	SetState( dtsConnecting );
	m_tConnected	= GetTickCount();
	m_pHost			= m_pClient->m_pHost;
	m_sAddress		= m_pClient->m_sAddress;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT close

void CDownloadTransferBT::Close(TRISTATE bKeepSource)
{
	if ( m_pClient != NULL )
	{
		m_pClient->m_pDownloadTransfer = NULL;
		
		if ( m_pClient->IsOnline() )
		{
			m_pClient->Send( CBTPacket::New( BT_PACKET_NOT_INTERESTED ) );
		}
		else
		{
			m_pClient->Close();
		}
		
		m_pClient = NULL;
	}
	
	CDownloadTransfer::Close( bKeepSource );
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
	if ( m_pClient == NULL ) return 0;
	m_pClient->Measure();
	return m_pClient->m_mInput.nMeasure;
}

CString CDownloadTransferBT::GetStateText(BOOL bLong)
{
	if ( m_nState == dtsTorrent )
	{
		CString str;
		if ( ! m_bInterested )
			str = _T("Uninterested");
		else if ( m_bChoked )
			str = _T("Choked");
		else
			str = _T("Requesting");
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
	
	m_pSource->SetLastSeen();
	
	m_pClient->m_mInput.pLimit = &Downloads.m_nLimitGeneric;
	
	theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_CONNECTED, (LPCTSTR)m_sAddress );
	
	if ( ! m_pDownload->PrepareFile() )
	{
		Close( TS_TRUE );
		return FALSE;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT bitfields

BOOL CDownloadTransferBT::OnBitfield(CBTPacket* pPacket)
{
	QWORD nBlockSize	= m_pDownload->m_pTorrent.m_nBlockSize;
	DWORD nBlockCount	= m_pDownload->m_pTorrent.m_nBlockCount;
	
	m_pSource->m_pAvailable->DeleteChain();
	m_pSource->m_pAvailable = NULL;
	
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
				CFileFragment::AddMerge( &m_pSource->m_pAvailable, nOffset, nLength );
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
	CFileFragment::AddMerge( &m_pSource->m_pAvailable, nOffset, nLength );
	
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
		for ( CFileFragment* pFragment = m_pDownload->GetFirstEmptyFragment() ; pFragment ; pFragment = pFragment->m_pNext )
		{
			DWORD nBlock = (DWORD)( pFragment->m_nOffset / nBlockSize );
			
			for ( QWORD nLength = pFragment->m_nLength ; ; nBlock ++, nLength -= nBlockSize )
			{
				if ( m_pAvailable[ nBlock ] )
				{
					bInterested = TRUE;
					break;
				}
				
				if ( nLength <= nBlockSize ) break;
			}
			
			if ( bInterested ) break;
		}
	}
	
	if ( bInterested != m_bInterested )
	{
		m_bInterested = bInterested;
		Send( CBTPacket::New( bInterested ? BT_PACKET_INTERESTED : BT_PACKET_NOT_INTERESTED ) );
		
		if ( ! bInterested )
		{
			m_pRequested->DeleteChain();
			m_pRequested = NULL;
			m_nRequested = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT choking

BOOL CDownloadTransferBT::OnChoked(CBTPacket* pPacket)
{
	if ( m_bChoked ) return TRUE;
	m_bChoked = TRUE;
	
	SetState( dtsTorrent );
	theApp.Message( MSG_DEBUG, _T("Download from %s was choked."), (LPCTSTR)m_sAddress );
	
	for ( CFileFragment* pFragment = m_pRequested ; pFragment != NULL ; pFragment = pFragment->m_pNext )
	{
		CBTPacket* pPacket = CBTPacket::New( BT_PACKET_CANCEL );
		pPacket->WriteLongBE( (DWORD)( pFragment->m_nOffset / m_pDownload->m_pTorrent.m_nBlockSize ) );
		pPacket->WriteLongBE( (DWORD)( pFragment->m_nOffset % m_pDownload->m_pTorrent.m_nBlockSize ) );
		pPacket->WriteLongBE( (DWORD)pFragment->m_nLength );
		Send( pPacket );
	}
	
	m_pRequested->DeleteChain();
	m_pRequested = NULL;
	m_nRequested = 0;
	
	return TRUE;
}

BOOL CDownloadTransferBT::OnUnchoked(CBTPacket* pPacket)
{
	m_bChoked = FALSE;
	SetState( dtsTorrent );
	
	m_pRequested->DeleteChain();
	m_pRequested = NULL;
	m_nRequested = 0;
	
	theApp.Message( MSG_DEBUG, _T("Download from %s was UNchoked."), (LPCTSTR)m_sAddress );
	
	return SendRequests();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT request pipe

BOOL CDownloadTransferBT::SendRequests()
{
	ASSERT( m_nState == dtsTorrent || m_nState == dtsRequesting || m_nState == dtsDownloading );
	
	if ( m_bChoked || ! m_bInterested )
	{
		if ( m_nRequested == 0 ) SetState( dtsTorrent );
		return TRUE;
	}
	
	if ( m_nRequested >= (int)Settings.BitTorrent.RequestPipe )
	{
		if ( m_nState != dtsDownloading ) SetState( dtsRequesting );
		return TRUE;
	}
	
	QWORD nBlockSize = m_pDownload->m_pTorrent.m_nBlockSize;
	ASSERT( nBlockSize != 0 );
	if ( nBlockSize == 0 ) return TRUE;
	
	CFileFragment* pPossible = m_pDownload->GetFirstEmptyFragment()->CreateCopy();
	
	if ( ! m_pDownload->m_bTorrentEndgame )
	{
		for ( CDownloadTransfer* pTransfer = m_pDownload->GetFirstTransfer() ; pTransfer && pPossible ; pTransfer = pTransfer->m_pDlNext )
		{
			pTransfer->SubtractRequested( &pPossible );
		}
	}
	
	while ( m_nRequested < (int)Settings.BitTorrent.RequestPipe )
	{
		QWORD nOffset, nLength;
		
		if ( SelectFragment( pPossible, &nOffset, &nLength ) )
		{
			ChunkifyRequest( &nOffset, &nLength, Settings.BitTorrent.RequestSize, FALSE );
			
			CFileFragment::Subtract( &pPossible, nOffset, nLength );
			
			CFileFragment* pRequest = CFileFragment::New( NULL, m_pRequested, nOffset, nLength );
			if ( m_pRequested != NULL ) m_pRequested->m_pPrevious = pRequest;
			m_pRequested = pRequest;
			m_nRequested ++;
			
			int nType	= ( m_nDownloaded == 0 || ( nOffset % nBlockSize ) == 0 )
						? MSG_DEFAULT : MSG_DEBUG;
			
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
	
	if ( pPossible == NULL && m_pDownload->m_bTorrentEndgame == FALSE )
	{
		m_pDownload->m_bTorrentEndgame = Settings.BitTorrent.Endgame;
	}
	
	pPossible->DeleteChain();
	
	if ( m_nRequested > 0 && m_nState != dtsDownloading ) SetState( dtsRequesting );
	if ( m_nRequested == 0 ) SetState( dtsTorrent );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT fragment selection

BOOL CDownloadTransferBT::SelectFragment(CFileFragment* pPossible, QWORD* pnOffset, QWORD* pnLength)
{
	ASSERT( pnOffset != NULL && pnLength != NULL );
	
	if ( pPossible == NULL ) return FALSE;
	
	QWORD nBlockSize = m_pDownload->m_pTorrent.m_nBlockSize;
	CFileFragment* pComplete = NULL;
	DWORD nBlock;
	
	ASSERT( nBlockSize != 0 );
	
	for ( ; pPossible ; pPossible = pPossible->m_pNext )
	{
		if ( pPossible->m_nOffset % nBlockSize )
		{
			// the start of a block is complete, but part is missing
			
			nBlock = (DWORD)( pPossible->m_nOffset / nBlockSize );
			ASSERT( nBlock < m_pDownload->m_pTorrent.m_nBlockCount );
			
			if ( m_pAvailable == NULL || m_pAvailable[ nBlock ] )
			{
				*pnOffset = pPossible->m_nOffset;
				*pnLength = nBlockSize * (QWORD)nBlock + nBlockSize - *pnOffset;
				*pnLength = min( *pnLength, pPossible->m_nLength );
				ASSERT( *pnLength <= nBlockSize );
				
				pComplete->DeleteChain();
				return TRUE;
			}
		}
		else if (	( pPossible->m_nLength % nBlockSize ) &&
					( pPossible->m_nOffset + pPossible->m_nLength < m_pDownload->m_nSize ) )
		{
			// the end of a block is complete, but part is missing
			
			nBlock = (DWORD)( ( pPossible->m_nOffset + pPossible->m_nLength ) / nBlockSize );
			ASSERT( nBlock < m_pDownload->m_pTorrent.m_nBlockCount );
			
			if ( m_pAvailable == NULL || m_pAvailable[ nBlock ] )
			{
				*pnOffset = nBlockSize * (QWORD)nBlock;
				*pnLength = pPossible->m_nOffset + pPossible->m_nLength - *pnOffset;
				ASSERT( *pnLength <= nBlockSize );
				
				pComplete->DeleteChain();
				return TRUE;
			}
		}
		else
		{
			// this fragment contains one or more aligned empty blocks
			
			nBlock = (DWORD)( pPossible->m_nOffset / nBlockSize );
			*pnLength = pPossible->m_nLength;
			ASSERT( *pnLength != 0 );
			
			for ( ; ; nBlock ++, *pnLength -= nBlockSize )
			{
				ASSERT( nBlock < m_pDownload->m_pTorrent.m_nBlockCount );
				
				if ( m_pAvailable == NULL || m_pAvailable[ nBlock ] )
				{
					pComplete = CFileFragment::New( NULL, pComplete, (QWORD)nBlock, 0 );
				}
				
				if ( *pnLength <= nBlockSize ) break;
			}
		}
	}
	
	if ( CFileFragment* pRandom = pComplete->GetRandom() )
	{
		*pnOffset = pRandom->m_nOffset * nBlockSize;
		*pnLength = nBlockSize;
		*pnLength = min( *pnLength, m_pDownload->m_nSize - *pnOffset );
		ASSERT( *pnLength <= nBlockSize );
		
		pComplete->DeleteChain();
		return TRUE;
	}
	else
	{
		ASSERT( pComplete == NULL );
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT multi-source fragment handling

BOOL CDownloadTransferBT::SubtractRequested(CFileFragment** ppFragments)
{
	if ( m_nRequested == 0 || m_bChoked ) return FALSE;
	CFileFragment::Subtract( ppFragments, m_pRequested );
	return TRUE;
}

BOOL CDownloadTransferBT::UnrequestRange(QWORD nOffset, QWORD nLength)
{
	if ( m_nRequested == 0 ) return FALSE;
	
	ASSERT( m_pDownload->m_pTorrent.m_nBlockSize != 0 );
	if ( m_pDownload->m_pTorrent.m_nBlockSize == 0 ) return FALSE;
	
	CFileFragment** ppPrevious = &m_pRequested;
	BOOL bMatch = FALSE;
	
	for ( CFileFragment* pFragment = *ppPrevious ; pFragment ; )
	{
		CFileFragment* pNext = pFragment->m_pNext;
		
		if ( nOffset < pFragment->m_nOffset + pFragment->m_nLength &&
			 nOffset + nLength > pFragment->m_nOffset )
		{
			CBTPacket* pPacket = CBTPacket::New( BT_PACKET_CANCEL );
			pPacket->WriteLongBE( (DWORD)( pFragment->m_nOffset / m_pDownload->m_pTorrent.m_nBlockSize ) );
			pPacket->WriteLongBE( (DWORD)( pFragment->m_nOffset % m_pDownload->m_pTorrent.m_nBlockSize ) );
			pPacket->WriteLongBE( (DWORD)pFragment->m_nLength );
			Send( pPacket );
			
			*ppPrevious = pNext;
			if ( pNext ) pNext->m_pPrevious = pFragment->m_pPrevious;
			pFragment->DeleteThis();
			m_nRequested --;
			bMatch = TRUE;
		}
		else
		{
			ppPrevious = &pFragment->m_pNext;
		}
		
		pFragment = pNext;
	}
	
	return bMatch;
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
	
	m_pSource->AddFragment( nOffset, nLength );
	m_pSource->SetValid();
	
	CFileFragment::Subtract( &m_pRequested, nOffset, nLength );
	m_nRequested = m_pRequested->GetCount();
	
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
			if ( ! pID->IsType( CBENode::beString ) || pID->m_nValue != sizeof(SHA1) ) continue;
			
			CBENode* pIP = pPeer->GetNode( "ip" );
			if ( ! pIP->IsType( CBENode::beString ) ) continue;
			
			CBENode* pPort = pPeer->GetNode( "port" );
			if ( ! pPort->IsType( CBENode::beInt ) ) continue;
			
			SOCKADDR_IN saPeer;
			if ( ! Network.Resolve( pIP->GetString(), (int)pPort->GetInt(), &saPeer ) ) continue;
			
			theApp.Message( MSG_DEBUG, _T("CDownloadTransferBT::OnSourceResponse(): %s: %s:%i"),
				(LPCTSTR)m_sAddress,
				(LPCTSTR)CString( inet_ntoa( saPeer.sin_addr ) ), htons( saPeer.sin_port ) );
			
			nCount += m_pDownload->AddSourceBT( (SHA1*)pID->m_pValue,
				&saPeer.sin_addr, htons( saPeer.sin_port ) );
		}
	}
	
	delete pRoot;
	
	theApp.Message( MSG_DEFAULT, IDS_BT_CLIENT_EXCHANGE, nCount, (LPCTSTR)m_sAddress );
	
	return TRUE;
}
