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
	ASSERT( m_pDownload->m_oBTH.IsValid() );
	ASSERT( m_pDownload->m_nSize != SIZE_UNKNOWN );
	
	m_pClient			= pClient;
	m_nState			= pClient ? dtsConnecting : dtsNull;
	m_sUserAgent		= _T("BitTorrent");
	
	m_bChoked			= TRUE;
	m_bInterested		= FALSE;
	
	ASSERT( m_oRequested.IsEmpty() );
	
	m_tRunThrottle		= 0;
	m_tSourceRequest	= GetTickCount();
	m_nBitFieldSize		= ( m_pDownload->m_pTorrent.m_nBlockCount + 31 ) >> 5;
	m_pBitFields		= new DWORD[ 2 * m_nBitFieldSize ];
	m_nOldBitField		= 0;
	m_nNewBitField		= m_nBitFieldSize;
	ZeroMemory( m_pBitFields , m_nBitFieldSize * sizeof ( m_pBitFields ) );
	pSource->m_oAvailable.Delete();
}

CDownloadTransferBT::~CDownloadTransferBT()
{
	ASSERT( m_pClient == NULL );
	m_oRequested.Delete();
	delete [] m_pBitFields;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT initiate

BOOL CDownloadTransferBT::Initiate()
{
	ASSERT( m_pClient == NULL );
	ASSERT( m_nState == dtsNull );
	m_pClient = new CBTClient();
	if ( m_pClient->Connect( this ) )
	{
		SetState( dtsConnecting );
		m_tConnected	= GetTickCount();
		m_pHost			= m_pClient->m_pHost;
		m_sAddress		= m_pClient->m_sAddress;
		return TRUE;
	}
	else
	{
		delete m_pClient;
		m_pClient = NULL;
		Close( TS_FALSE );
		return FALSE;
	}
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
		if ( ( m_nState == dtsTorrent || m_nState == dtsRequesting || m_nState == dtsDownloading )
			&& ( ! SendRequests() ) ) return FALSE;
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
	QWORD nBlockSize = m_pDownload->m_pTorrent.m_nBlockSize;
	DWORD nBlockCount = m_pDownload->m_pTorrent.m_nBlockCount;
	DWORD nBlock32 = 0;
	DWORD nRead32, nAnd32;
	QWORD nOffset, nNext;
	int nRemaining;
	if ( !nBlockSize || !nBlockCount ) return TRUE;
	CFileFragmentList oAdd, oSubtract;
	while ( ( nBlock32 < m_nBitFieldSize ) && ( pPacket->GetRemaining() > 4 ) )
	{
		nRead32 = pPacket->ReadLongBE();
		if ( nRead32 =
			( ( m_pBitFields[ m_nNewBitField + nBlock32 ] = nRead32 ) ^ m_pBitFields[ m_nOldBitField + nBlock32 ] ) )
		{
			if ( nAnd32 = ( nRead32 & m_pBitFields[ m_nNewBitField + nBlock32 ] ) )
			{
				nOffset = ( nBlock32 << 5 ) * nBlockSize;
				nNext = nOffset + nBlockSize;
				do
				{
					if ( nAnd32 & 0x80000000 ) oAdd.Add( nOffset, nNext );
					nOffset = nNext;
					nNext += nBlockSize;
				}
				while ( nAnd32 <<= 1 );
			}
			if ( nAnd32 = ( nRead32 & m_pBitFields[ m_nOldBitField + nBlock32 ] ) )
			{
				nOffset = ( nBlock32 << 5 ) * nBlockSize;
				nNext = nOffset + nBlockSize;
				do
				{
					if ( nAnd32 & 0x80000000 ) oSubtract.Add( nOffset, nNext );
					nOffset = nNext;
					nNext += nBlockSize;
				}
				while ( nAnd32 <<= 1 );
			}
		}
		nBlock32++;
	}
	if ( ( nBlock32 < m_nBitFieldSize ) && ( nRemaining = pPacket->GetRemaining() ) )
	{
		if ( nRemaining <= 2 )
		{
			if ( nRemaining == 1 )
			{
				nRead32 = ( (DWORD)pPacket->ReadByte() ) << 24;
			}
			else
			{
				nRead32 = ( (DWORD)pPacket->ReadShortBE() ) << 16;
			}
		}
		else
		{
			if ( nRemaining == 3 )
			{
				nRead32 = ( (DWORD)pPacket->ReadShortBE() ) << 8;
				nRead32 = ( nRead32 | (DWORD)pPacket->ReadByte() ) << 8;
			}
			else
			{
				nRead32 = pPacket->ReadLongBE() << 16;
			}
		}
		if ( nRead32 =
			( ( m_pBitFields[ m_nNewBitField + nBlock32 ] = nRead32 ) ^ m_pBitFields[ m_nOldBitField + nBlock32 ] ) )
		{
			if ( nAnd32 = ( nRead32 & m_pBitFields[ m_nNewBitField + nBlock32 ] ) )
			{
				nOffset = ( nBlock32 << 5 ) * nBlockSize;
				nNext = nOffset + nBlockSize;
				do
				{
					if ( nNext > m_pDownload->m_nSize ) nNext = m_pDownload->m_nSize;
					if ( nAnd32 & 0x80000000 ) oAdd.Add( nOffset, nNext );
					nOffset = nNext;
					nNext += nBlockSize;
				}
				while ( nAnd32 <<= 1 );
			}
			if ( nAnd32 = ( nRead32 & m_pBitFields[ m_nOldBitField + nBlock32 ] ) )
			{
				nOffset = ( nBlock32 << 5 ) * nBlockSize;
				nNext = nOffset + nBlockSize;
				do
				{
					if ( nNext > m_pDownload->m_nSize ) nNext = m_pDownload->m_nSize;
					if ( nAnd32 & 0x80000000 ) oSubtract.Add( nOffset, nNext );
					nOffset = nNext;
					nNext += nBlockSize;
				}
				while ( nAnd32 <<= 1 );
			}
		}
	}
	m_nNewBitField = m_nBitFieldSize - ( m_nOldBitField = m_nNewBitField );
	m_pSource->m_oAvailable.Subtract( oSubtract );
	m_pSource->m_oAvailable.Merge( oAdd );

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
	m_pSource->m_oAvailable.Add( nOffset, nOffset + nLength );
	ShowInterest();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT interest control

void CDownloadTransferBT::ShowInterest()
{
	BOOL bInterested;
	m_oPossible.GetAnd( m_pDownload->m_pFile->m_oFree, m_pSource->m_oAvailable );
	
	// TODO: Use an algorithm similar to CDownloadWithTiger::FindNext.., rather
	// than relying on that algorithm to complete verifications here.
	
	if ( m_bInterested != ( bInterested = ! m_oPossible.IsEmpty() ) )
	{
		m_bInterested = bInterested;
		Send( CBTPacket::New( bInterested ? BT_PACKET_INTERESTED : BT_PACKET_NOT_INTERESTED ) );
		if ( ! bInterested ) m_oRequested.Delete();
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
	CFileFragment* pFragment = m_oRequested.GetFirst();
	while ( pFragment )
	{
		CBTPacket* pPacket = CBTPacket::New( BT_PACKET_CANCEL );
		pPacket->WriteLongBE( (DWORD)( pFragment->Offset() / m_pDownload->m_pTorrent.m_nBlockSize ) );
		pPacket->WriteLongBE( (DWORD)( pFragment->Offset() % m_pDownload->m_pTorrent.m_nBlockSize ) );
		pPacket->WriteLongBE( (DWORD)( pFragment->Length() ) );
		Send( pPacket );
		pFragment = pFragment->GetNext();
	}
	m_oRequested.Delete();
	return TRUE;
}

BOOL CDownloadTransferBT::OnUnchoked(CBTPacket* pPacket)
{
	m_bChoked = FALSE;
	SetState( dtsTorrent );
	m_oRequested.Delete();
	theApp.Message( MSG_DEBUG, _T("Download from %s was Unchoked."), (LPCTSTR)m_sAddress );
	ShowInterest();
	return SendRequests();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT request pipe

BOOL CDownloadTransferBT::SendRequests()
{
	ASSERT( m_nState == dtsTorrent || m_nState == dtsRequesting || m_nState == dtsDownloading );
	if ( m_bChoked || ! m_bInterested )
	{
		if ( m_oRequested.IsEmpty() ) SetState( dtsTorrent );
		return TRUE;
	}
	if ( m_oRequested.GetCount() >= Settings.BitTorrent.RequestPipe )
	{
		if ( m_nState != dtsDownloading ) SetState( dtsRequesting );
		return TRUE;
	}
	QWORD nBlockSize = m_pDownload->m_pTorrent.m_nBlockSize;
	ASSERT( nBlockSize != 0 );
	if ( nBlockSize == 0 ) return TRUE;
	if ( ! m_pDownload->m_bTorrentEndgame )
	{
		for ( CDownloadTransfer* pTransfer = m_pDownload->GetFirstTransfer();
			pTransfer && ( ! m_oPossible.IsEmpty() ) ; pTransfer = pTransfer->m_pDlNext )
		{
			pTransfer->SubtractRequested( m_oPossible );
		}
	}
	while ( m_oRequested.GetCount() < Settings.BitTorrent.RequestPipe )
	{
		QWORD nOffset, nLength;
		if ( SelectFragment( m_oPossible, nOffset, nLength ) )
		{
			ChunkifyRequest( &nOffset, &nLength, Settings.BitTorrent.RequestSize, FALSE );
			m_oPossible.Subtract( nOffset, nOffset + nLength );
			m_oRequested.Add( nOffset, nOffset + nLength );
			int nType	= ( m_nDownloaded == 0 || ( nOffset % nBlockSize ) == 0 ) ? MSG_DEFAULT : MSG_DEBUG;
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
	if ( m_oPossible.IsEmpty() && m_pDownload->m_bTorrentEndgame == FALSE )
	{
		m_pDownload->m_bTorrentEndgame = Settings.BitTorrent.Endgame;
	}
	if ( m_oRequested.GetCount() > 0 && m_nState != dtsDownloading ) SetState( dtsRequesting );
	if ( m_oRequested.IsEmpty() ) SetState( dtsTorrent );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT fragment selection

BOOL CDownloadTransferBT::SelectFragment(const CFileFragmentList& oPossible, QWORD& nOffset, QWORD& nLength)
{
	if ( oPossible.IsEmpty() ) return FALSE;
	DWORD nBlock, nBlockSize = m_pDownload->m_pTorrent.m_nBlockSize;
	DWORD nModMask;
	QWORD nDivMask;
	BYTE nShift;
	DWORD nLengthCount, nFound = 0;
	DWORD* aBlocks;
	CFileFragment *pFragment = oPossible.GetFirst();
	if ( (QWORD)0x100000000 % nBlockSize )
	{	// nBlockSize != 2 exp (n)
		do
		{
			if ( pFragment->Offset() % nBlockSize )
			{	// the start of a block is complete, but part is missing
				nOffset = pFragment->Offset();
				nLength = min( nBlockSize - ( pFragment->Offset() % nBlockSize ), pFragment->Length() );
				return TRUE;
			}
			else if ( ( pFragment->Next() % nBlockSize ) && ( pFragment->Next() < m_pDownload->m_nSize ) )
			{	// the end of a block is complete, but part is missing
				nOffset = pFragment->Next() - ( pFragment->Next() % nBlockSize );
				nLength = pFragment->Next() - nOffset;
				return TRUE;
			}
		}
		while ( pFragment = pFragment->GetNext() );
		aBlocks = new DWORD[m_pDownload->m_pTorrent.m_nBlockCount];
		pFragment = oPossible.GetFirst();
		do
		{	// all Fragments contain aligned Blocks
			nBlock = (DWORD)( pFragment->Offset() / nBlockSize );
			nLengthCount = (DWORD)( ( pFragment->Length() + nBlockSize - 1 ) / nBlockSize);
			while ( nLengthCount-- ) aBlocks[ nFound++ ] = nBlock++;
		}
		while ( pFragment = pFragment->GetNext() );
	}
	else
	{
		nModMask = nBlockSize - 1;
		nDivMask = 0 - (QWORD)nBlockSize;
		do
		{
			if ( pFragment->Offset() & nModMask )
			{	// the start of a block is complete, but part is missing
				nOffset = pFragment->Offset();
				nLength = min( nBlockSize - ( pFragment->Offset() & nModMask ), pFragment->Length() );
				return TRUE;
			}
			else if ( ( pFragment->Next() & nModMask ) && ( pFragment->Next() < m_pDownload->m_nSize ) )
			{	// the end of a block is complete, but part is missing
				nOffset = pFragment->Next() & nDivMask;
				nLength = pFragment->Next() - nOffset;
				return TRUE;
			}
		}
		while ( pFragment = pFragment->GetNext() );
		nShift = 1;
		while ( nModMask >>= 1 ) nShift++;
		nModMask = nBlockSize - 1;
		aBlocks = new DWORD[m_pDownload->m_pTorrent.m_nBlockCount];
		pFragment = oPossible.GetFirst();
		do
		{	// all Fragments contain aligned Blocks
			nBlock = (DWORD)( pFragment->Offset() >> nShift );
			nLengthCount = (DWORD)( ( pFragment->Length() + nModMask ) >> nShift );
			while ( nLengthCount-- ) aBlocks[ nFound++ ] = nBlock++;
		}
		while ( pFragment = pFragment->GetNext() );
	}
	nOffset = (QWORD)(aBlocks[ ( nFound * rand() ) >> 15 ]) * (QWORD)nBlockSize;
	nLength = nOffset + nBlockSize > m_pDownload->m_nSize ? m_pDownload->m_nSize - nOffset : nBlockSize;
	delete [] aBlocks;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferBT multi-source fragment handling

BOOL CDownloadTransferBT::SubtractRequested(CFileFragmentList& Fragments)
{
	if ( m_oRequested.IsEmpty() || m_bChoked ) return FALSE;
	Fragments.Subtract( m_oRequested );
	return TRUE;
}

BOOL CDownloadTransferBT::UnrequestRange(QWORD nOffset, QWORD nLength)
{
	if ( m_oRequested.IsEmpty() ) return FALSE;
	ASSERT( m_pDownload->m_pTorrent.m_nBlockSize != 0 );
	if ( m_pDownload->m_pTorrent.m_nBlockSize == 0 ) return FALSE;
	BOOL bMatch = FALSE;
	CFileFragmentList m_oUnrequest;
	m_oUnrequest.Extract( m_oRequested, nOffset, nOffset + nLength );
	CFileFragment *pFragment;
	if ( bMatch = ( pFragment = m_oUnrequest.GetFirst() ) != NULL ) do
	{
		CBTPacket* pPacket = CBTPacket::New( BT_PACKET_CANCEL );
		pPacket->WriteLongBE( (DWORD)( pFragment->Offset() / m_pDownload->m_pTorrent.m_nBlockSize ) );
		pPacket->WriteLongBE( (DWORD)( pFragment->Offset() % m_pDownload->m_pTorrent.m_nBlockSize ) );
		pPacket->WriteLongBE( (DWORD)( pFragment->Length() ) );
		Send( pPacket );
	}
	while ( pFragment = pFragment->GetNext() );
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
	nOffset += (QWORD)nBlock * (QWORD)m_pDownload->m_pTorrent.m_nBlockSize;
	m_nDownloaded += nLength;
	m_pDownload->m_nTorrentDownloaded += nLength;
	m_pSource->AddFragment( nOffset, nLength );
	m_pSource->SetValid();
	m_oRequested.Subtract( nOffset, nOffset + nLength );
	m_pDownload->SubmitData( nOffset, pPacket->m_pBuffer + pPacket->m_nPosition, nLength );
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
			if ( ! pID->IsType( CBENode::beString ) || pID->m_nValue != GUIDBT_SIZE  ) continue;
			
			CBENode* pIP = pPeer->GetNode( "ip" );
			if ( ! pIP->IsType( CBENode::beString ) ) continue;
			
			CBENode* pPort = pPeer->GetNode( "port" );
			if ( ! pPort->IsType( CBENode::beInt ) ) continue;
			
			SOCKADDR_IN saPeer;
			if ( ! Network.Resolve( pIP->GetString(), (int)pPort->GetInt(), &saPeer ) ) continue;
			
			theApp.Message( MSG_DEBUG, _T("CDownloadTransferBT::OnSourceResponse(): %s: %s:%i"),
				(LPCTSTR)m_sAddress,
				(LPCTSTR)CString( inet_ntoa( saPeer.sin_addr ) ), htons( saPeer.sin_port ) );
			
			nCount += m_pDownload->AddSourceBT( (CGUIDBT*)pID->m_pValue,
				&saPeer.sin_addr, htons( saPeer.sin_port ) );
		}
	}
	
	delete pRoot;
	
	theApp.Message( MSG_DEFAULT, IDS_BT_CLIENT_EXCHANGE, nCount, (LPCTSTR)m_sAddress );
	
	return TRUE;
}
