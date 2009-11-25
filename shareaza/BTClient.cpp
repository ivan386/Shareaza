//
// BTClient.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "BTClient.h"
#include "BTClients.h"
#include "BTPacket.h"
#include "BENode.h"
#include "Buffer.h"

#include "Download.h"
#include "Downloads.h"
#include "DownloadSource.h"
#include "DownloadTransferBT.h"
#include "Uploads.h"
#include "UploadTransferBT.h"
#include "ShareazaURL.h"
#include "GProfile.h"
#include "Datagrams.h"
#include "Transfers.h"
#include "VendorCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CBTClient construction

CBTClient::CBTClient()
	: m_bExchange			( FALSE )
	, m_bExtended			( FALSE )
	, m_pUpload				( NULL )
	, m_pDownload			( NULL )
	, m_pDownloadTransfer	( NULL )
	, m_bShake				( FALSE )
	, m_bOnline				( FALSE )
	, m_bClosing			( FALSE )
	, m_tLastKeepAlive		( GetTickCount() )
	, m_dUtMetadataID		( 0 )
	, m_dUtMetadataSize		( 0 )
{
	m_sUserAgent = _T("BitTorrent");
	m_mInput.pLimit = m_mOutput.pLimit = &Settings.Bandwidth.Request;

	if ( Settings.General.DebugBTSources )
		theApp.Message( MSG_DEBUG, L"Adding BT client to collection: %s", m_sAddress );

	BTClients.Add( this );
}

CBTClient::~CBTClient()
{
	ASSERT( ! IsValid() );
	ASSERT( m_pDownloadTransfer == NULL );
	ASSERT( m_pDownload == NULL );
	ASSERT( m_pUpload == NULL );

	if ( Settings.General.DebugBTSources )
		theApp.Message( MSG_DEBUG, L"Removing BT client from collection: %s", m_sAddress );

	BTClients.Remove( this );
}

CDownloadSource* CBTClient::GetSource() const
{
	return m_pDownloadTransfer ? m_pDownloadTransfer->GetSource() : NULL;
}

//////////////////////////////////////////////////////////////////////
// CBTClient initiate a new connection

BOOL CBTClient::Connect(CDownloadTransferBT* pDownloadTransfer)
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_bClosing ) return FALSE;
	ASSERT( ! IsValid() );
	ASSERT( m_pDownload == NULL );

	const CDownloadSource* pSource = pDownloadTransfer->GetSource();

	if ( ! CTransfer::ConnectTo( &pSource->m_pAddress, pSource->m_nPort ) ) return FALSE;
	
	m_pDownload			= pDownloadTransfer->GetDownload();
	m_pDownloadTransfer	= pDownloadTransfer;

	theApp.Message( MSG_INFO, IDS_BT_CLIENT_CONNECTING, m_sAddress );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient attach to existing connection

void CBTClient::AttachTo(CConnection* pConnection)
{
	if ( m_bClosing ) return;
	ASSERT( ! IsValid() );

	CTransfer::AttachTo( pConnection );
	if ( Settings.General.DebugBTSources )
		theApp.Message( MSG_DEBUG, L"Attaching new BT client connection: %s", m_sAddress );

	ASSERT( m_mInput.pLimit != NULL );
	m_tConnected = GetTickCount();
	theApp.Message( MSG_INFO, IDS_BT_CLIENT_ACCEPTED, m_sAddress );
}

//////////////////////////////////////////////////////////////////////
// CBTClient close

void CBTClient::Close()
{
	ASSERT( this != NULL );

	m_mInput.pLimit = NULL;
	m_mOutput.pLimit = NULL;

	if ( m_bClosing ) return;
	m_bClosing = TRUE;

	if ( Settings.General.DebugBTSources )
		theApp.Message( MSG_DEBUG, L"Deleting BT client: %s", m_sAddress );

	if ( m_pUpload != NULL ) m_pUpload->Close();
	ASSERT( m_pUpload == NULL );

	if ( m_pDownloadTransfer != NULL )
	{
		if ( ( m_pDownload == NULL ) || ( m_pDownload->IsCompleted() ) )
			m_pDownloadTransfer->Close( TRI_FALSE );
		else
			m_pDownloadTransfer->Close( TRI_UNKNOWN );
	}
	ASSERT( m_pDownloadTransfer == NULL );

	m_pDownload = NULL;

	CTransfer::Close();

	delete this;
}

//////////////////////////////////////////////////////////////////////
// CBTClient send a packet

void CBTClient::Send(CBTPacket* pPacket, BOOL bRelease)
{
	ASSERT( IsValid() );
	ASSERT( m_bOnline );
	
	if ( pPacket != NULL )
	{
		ASSERT( pPacket->m_nProtocol == PROTOCOL_BT );
		
		Write( pPacket );
		if ( bRelease ) pPacket->Release();
	}
	
	OnWrite();
}

//////////////////////////////////////////////////////////////////////
// CBTClient run event

BOOL CBTClient::OnRun()
{
	CTransfer::OnRun();
	
	DWORD tNow = GetTickCount();
	
	if ( ! m_bConnected )
	{
		if ( tNow - m_tConnected > Settings.Connection.TimeoutConnect )
		{
			theApp.Message( MSG_ERROR, IDS_BT_CLIENT_CONNECT_TIMEOUT, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
	}
	else if ( ! m_bOnline )
	{
		if ( tNow - m_tConnected > Settings.Connection.TimeoutHandshake )
		{
			theApp.Message( MSG_ERROR, IDS_BT_CLIENT_HANDSHAKE_TIMEOUT, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
	}
	else
	{
		if ( tNow - m_mInput.tLast > Settings.BitTorrent.LinkTimeout * 2 )
		{
			theApp.Message( MSG_ERROR, IDS_BT_CLIENT_LOST, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
		/*else if ( tNow - m_mOutput.tLast > Settings.BitTorrent.LinkPing / 2 && m_pOutput->m_nLength == 0 )
		{
			DWORD dwZero = 0;
			Write( &dwZero, 4 );			// wtf???
			OnWrite();
		}*/
		else if ( tNow - m_tLastKeepAlive > Settings.BitTorrent.LinkPing )
		{
			Send( CBTPacket::New( BT_PACKET_KEEPALIVE ) );
			m_tLastKeepAlive = tNow;
		}

		ASSERT ( m_pUpload != NULL );

		if ( m_pDownloadTransfer != NULL && ! m_pDownloadTransfer->OnRun() ) return FALSE;
		if ( m_pUpload == NULL || ! m_pUpload->OnRun() ) return FALSE;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient connection establishment event

BOOL CBTClient::OnConnected()
{
	theApp.Message( MSG_INFO, IDS_BT_CLIENT_HANDSHAKING, (LPCTSTR)m_sAddress );
	SendHandshake( TRUE, TRUE );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient connection loss event

void CBTClient::OnDropped()
{
	if ( ! m_bConnected )
		theApp.Message( MSG_ERROR, IDS_BT_CLIENT_DROP_CONNECTING, (LPCTSTR)m_sAddress );
	else if ( ! m_bOnline )
		theApp.Message( MSG_ERROR, IDS_BT_CLIENT_DROP_HANDSHAKE, (LPCTSTR)m_sAddress );
	else
		theApp.Message( MSG_ERROR, IDS_BT_CLIENT_DROP_CONNECTED, (LPCTSTR)m_sAddress );
	   
	Close();
}

//////////////////////////////////////////////////////////////////////
// CBTClient write event

BOOL CBTClient::OnWrite()
{
	if ( m_bClosing ) return FALSE;
	CTransfer::OnWrite();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient read event

BOOL CBTClient::OnRead()
{
	BOOL bSuccess = TRUE;
	if ( m_bClosing ) return FALSE;

	CTransfer::OnRead();
	
	CLockedBuffer pInput( GetInput() );

	if ( m_bOnline )
	{
		while ( CBTPacket* pPacket = CBTPacket::ReadBuffer( pInput ) )
		{
			try
			{
				bSuccess = OnPacket( pPacket );
			}
			catch ( CException* pException )
			{
				pException->Delete();
				if ( ! m_bOnline ) bSuccess = FALSE;
			}
			
			pPacket->Release();
			if ( ! bSuccess ) break;
		}
	}
	else
	{
        if ( ! m_bShake && pInput->m_nLength >= BT_PROTOCOL_HEADER_LEN + 8 + Hashes::Sha1Hash::byteCount )
		{
			bSuccess = OnHandshake1();
		}
		
		if ( bSuccess && m_bShake && pInput->m_nLength >= Hashes::Sha1Hash::byteCount )
		{
			bSuccess = OnHandshake2();
		}/*
		else if ( bSuccess && m_bShake )
		{
			DWORD tNow = GetTickCount();
			if ( tNow - m_tConnected > Settings.Connection.TimeoutHandshake / 2 )
			{
				theApp.Message( MSG_ERROR,  _T("No peer-id received, forcing connection") );
				bSuccess = OnNoHandshake2();
			}
		}*/
	}
	
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CBTClient handshaking

void CBTClient::SendHandshake(BOOL bPart1, BOOL bPart2)
{
	if ( m_bClosing ) return;
	ASSERT( m_pDownload != NULL );
	
	if ( bPart1 )
	{
		DWORD dwFlags = 0;
		Write( _P(BT_PROTOCOL_HEADER) );
		Write( &dwFlags, 4 );
		dwFlags = 0x1000;
		Write( &dwFlags, 4 );
		Write( m_pDownload->m_oBTH );
	}
	
	if ( bPart2 )
	{
		if ( ! m_pDownload->m_pPeerID )
			m_pDownload->GenerateTorrentDownloadID();

        Write( m_pDownload->m_pPeerID );
	}
	
	OnWrite();
}

BOOL CBTClient::OnHandshake1()
{	
	//First part of the handshake
	if ( m_bClosing ) return FALSE;

	ASSERT( ! m_bOnline );
	ASSERT( ! m_bShake );

	// Read in the BT protocol header
	if ( ! StartsWith( BT_PROTOCOL_HEADER, BT_PROTOCOL_HEADER_LEN ) )
	{
		theApp.Message( MSG_ERROR, _T("BitTorrent coupling from %s had invalid header"), (LPCTSTR)m_sAddress );
		Close();
		return FALSE;
	}

	m_bExtended = PeekAt( BT_PROTOCOL_HEADER_LEN + 5 ) & 0x10;

	Bypass( BT_PROTOCOL_HEADER_LEN + 8 );

	// Read in the file ID
	Hashes::BtHash oFileHash;
	Read( oFileHash );
	oFileHash.validate();

	if ( m_pDownload != NULL )	// If we initiated download (download has associated, which means we initiated download)
	{
		ASSERT( m_pDownloadTransfer != NULL );

		if ( validAndUnequal( oFileHash, m_pDownload->m_oBTH ) )
		{	//Display an error and exit
			theApp.Message( MSG_ERROR, IDS_BT_CLIENT_WRONG_FILE, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
		else if ( ! m_pDownload->IsTrying() && ! m_pDownload->IsSeeding() )
		{	//Display an error and exit
			theApp.Message( MSG_ERROR, IDS_BT_CLIENT_INACTIVE_FILE, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
	}
	else	// otherwise download has initiated from other side (no download has associated, which means connection initiated by G2 PUSH)
	{
		ASSERT( m_pDownloadTransfer == NULL );

		// Find the requested file
		m_pDownload = Downloads.FindByBTH( oFileHash, TRUE );

		if ( m_pDownload == NULL )				// If we can't find the file
		{	//Display an error and exit
			theApp.Message( MSG_ERROR, IDS_BT_CLIENT_UNKNOWN_FILE, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
		else if ( ! m_pDownload->IsTrying() && ! m_pDownload->IsSeeding() )	// If the file isn't active
		{	//Display an error and exit
			m_pDownload = NULL;
			theApp.Message( MSG_ERROR, IDS_BT_CLIENT_INACTIVE_FILE, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
		else if ( m_pDownload->UploadExists( &m_pHost.sin_addr ) )	// If there is already an upload of this file to this client
		{
			// Display an error and exit
			m_pDownload = NULL;
			theApp.Message( MSG_ERROR, IDS_BT_CLIENT_DUPLICATE, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
		// The file isn't verified yet, close the connection
		else if ( m_pDownload->m_bVerify != TRI_TRUE
			&& ( m_pDownload->IsMoving() ||  m_pDownload->IsCompleted() ) )
		{
			theApp.Message( MSG_ERROR, IDS_BT_CLIENT_INACTIVE_FILE, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}


		// Check we don't have too many active torrent connections 
		// (Prevent routers overloading for very popular torrents)
		if ( ( m_pDownload->GetTransferCount( dtsCountTorrentAndActive ) ) > ( Settings.BitTorrent.DownloadConnections * 1.25 ) ) 
		{
			theApp.Message( MSG_ERROR, IDS_BT_CLIENT_MAX_CONNECTIONS, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}

	}

	// Verify a download and hash
	ASSERT( m_pDownload != NULL );
	ASSERT( validAndEqual( m_pDownload->m_oBTH, oFileHash ) );

	// If we didn't start the connection, then send a handshake
	if ( !m_bInitiated ) SendHandshake( TRUE, TRUE );
	m_bShake = TRUE;

	return TRUE;
}

BOOL CBTClient::OnHandshake2()
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_bClosing ) return FALSE;

	// Second part of the handshake - Peer ID
	Read( m_oGUID );
	m_oGUID.validate();

	ASSERT( m_pDownload != NULL );

	if ( CDownloadSource* pSource = GetSource() )	// Transfer exist, so must be initiated from this side
	{
		pSource->m_oGUID = transformGuid( m_oGUID );

		/*
		//ToDo: This seems to trip when it shouldn't. Should be investigated...
		if ( memcmp( &m_pGUID, &pSource->m_pGUID, 16 ) != 0 )
		{
			theApp.Message( MSG_ERROR, IDS_BT_CLIENT_WRONG_GUID, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
		*/
	}
	else	// no transfer exist, so must be initiated from other side.
	{
		if ( m_pDownload->UploadExists( m_oGUID ) )
		{
			theApp.Message( MSG_ERROR, IDS_BT_CLIENT_DUPLICATE, m_sAddress );
			Close();
			return FALSE;
		}

		if ( ! m_pDownload->IsMoving() && ! m_pDownload->IsPaused() )
		{
			ASSERT( m_pDownloadTransfer == NULL );

			// Download from uploaders, unless the user has turned off downloading for this torrent
			if ( m_pDownload->m_pTorrent.m_nStartDownloads != CBTInfo::dtNever )
			{
				// This seems to be set to null sometimes... DownloadwithTorrent: if ( pSource->m_pTransfer != NULL )
				// May just be clients sending duplicate connection requests, though...
				m_pDownloadTransfer = m_pDownload->CreateTorrentTransfer( this );

				if ( m_pDownloadTransfer == NULL )
				{
					m_pDownload = NULL;
					theApp.Message( MSG_ERROR, IDS_BT_CLIENT_UNKNOWN_FILE, m_sAddress );
					Close();
					return FALSE;
				}
			}
		}
	}

	ASSERT( m_pUpload == NULL );
	if ( Settings.General.DebugBTSources )
		theApp.Message( MSG_DEBUG, L"Creating new BT upload: %s", m_sAddress );
	m_pUpload = new CUploadTransferBT( this, m_pDownload );

	m_bOnline = TRUE;

	DetermineUserAgent();

	return OnOnline();
}

//////////////////////////////////////////////////////////////////////
// CBTClient online handler

CString CBTClient::GetAzureusStyleUserAgent(LPBYTE pVendor, size_t nVendor)
{
	// Azureus style "-SSVVVV-" http://bittorrent.org/beps/bep_0020.html
	struct azureusStyleEntry
	{
		uchar signature[ 2 ];
		LPCTSTR client;
	};
	static const azureusStyleEntry azureusStyleClients[] =
	{
		{ 'A', 'G', L"Ares" },
		{ 'A', 'R', L"Arctic" },
		{ 'A', 'V', L"Avicora" },
		{ 'A', 'X', L"BitPump" },
		{ 'A', 'Z', L"Azureus" },
		{ 'A', '~', L"Ares" },
		{ 'B', 'B', L"BitBuddy" },
		{ 'B', 'C', L"BitComet" },
		{ 'B', 'F', L"Bitflu" },
		{ 'B', 'G', L"BTG" },
		{ 'b', 'k', L"BitKitten" },
		{ 'B', 'O', L"BO" },
		{ 'B', 'R', L"BitRocket" },
		{ 'B', 'S', L"BitSlave" },
		{ 'B', 'X', L"Bittorrent X" },
		{ 'C', 'B', L"ShareazaPlus" },		// RazaCB Core
		{ 'C', 'D', L"Enhanced CTorrent" },
		{ 'C', 'T', L"CTorrent" },
		{ 'D', 'E', L"DelugeTorrent" },
		{ 'E', 'B', L"EBit" },
		{ 'E', 'S', L"Electric Sheep" },
		{ 'F', 'C', L"FileCroc" },
		{ 'G', 'R', L"GetRight" },
		{ 'H', 'L', L"Halite" },
		{ 'H', 'N', L"Hydranode" },
		{ 'K', 'T', L"KTorrent" },
		{ 'L', 'H', L"LH-ABC" },
		{ 'L', 'K', L"Linkage" },
		{ 'L', 'P', L"Lphant" },
		{ 'L', 'T', L"libtorrent" },
		{ 'l', 't', L"rTorrent" },
		{ 'L', 'W', L"LimeWire" },
		{ 'M', 'O', L"Mono Torrent" },
		{ 'M', 'P', L"MooPolice" },
		{ 'M', 'T', L"MoonlightTorrent" },
		{ 'P', 'C', L"CacheLogic" },
		{ 'P', 'D', L"Pando" },
		{ 'P', 'E', L"PeerProject" },
		{ 'p', 'X', L"pHoeniX" },
		{ 'q', 'B', L"qBittorrent" },
		{ 'Q', 'T', L"QT4" },
		{ 'R', 'T', L"Retriever" },
		{ 'S', 'B', L"SwiftBit" },
		{ 'S', 'N', L"ShareNet" },
		{ 'S', 'S', L"Swarmscope" },
		{ 's', 't', L"Sharktorrent" },
		{ 'S', 'T', L"SymTorrent" },
		{ 'S', 'Z', L"Shareaza" },
		{ 'S', '~', L"ShareazaBeta" },
		{ 'T', 'N', L"Torrent.NET" },
		{ 'T', 'R', L"Transmission" },
		{ 'T', 'S', L"TorrentStorm" },
		{ 'T', 'T', L"TuoTu" },
		{ 'U', 'L', L"uLeecher!" },
		{ 'U', 'T', L"\x00B5Torrent" },
		{ 'X', 'L', L"Xunlei" },
		{ 'X', 'T', L"XanTorrent" },
		{ 'X', 'X', L"xTorrent" },
		{ 'Z', 'T', L"ZipTorrent" }
	};
	static const size_t azureusClients =
		sizeof azureusStyleClients / sizeof azureusStyleEntry;

	CString sUserAgent;
	if ( pVendor )
	{
		for ( size_t i = 0; i < azureusClients; ++i )
		{
			if ( pVendor[ 0 ] == azureusStyleClients[ i ].signature[ 0 ] &&
				 pVendor[ 1 ] == azureusStyleClients[ i ].signature[ 1 ] )
			{
				if ( nVendor == 4 )
					sUserAgent.Format( _T( "%s %i.%i" ),
						azureusStyleClients[ i ].client, pVendor[ 2 ], pVendor[ 3 ] );
				else if ( nVendor == 6 )
					sUserAgent.Format( _T( "%s %i.%i.%i.%i" ),
						azureusStyleClients[ i ].client,
						( pVendor[ 2 ] - '0' ), ( pVendor[ 3 ] - '0' ),
						( pVendor[ 4 ] - '0' ), ( pVendor[ 5 ] - '0' ) );
				break;
			}
		}
		if ( sUserAgent.IsEmpty() ) 
			// If we don't want the version, etc.
			sUserAgent.Format( _T("BitTorrent (%c%c)"), pVendor[ 0 ], pVendor[ 1 ] );
	}
	return sUserAgent;
}

void CBTClient::DetermineUserAgent()
{
	ASSUME_LOCK( Transfers.m_pSection );

	int nNickStart = 0, nNickEnd = 13;
	CString strVer, strNick;

	m_sUserAgent.Empty();
	m_bClientExtended = isExtendedBtGuid( m_oGUID );

	if ( m_oGUID[ 0 ] == '-' && m_oGUID[ 7 ] == '-' )
	{
		m_sUserAgent = GetAzureusStyleUserAgent( &(m_oGUID[ 1 ]), 6 );
	}
	else if ( m_oGUID[4] == '-' && m_oGUID[5] == '-' && m_oGUID[6] == '-' && m_oGUID[7] == '-' )
	{	// Shadow style
		switch ( m_oGUID[0] )
		{
		case 'A':
			m_sUserAgent = L"ABC";
			break;
		case 'O':
			m_sUserAgent = L"Osprey";
			break;
		case 'R':
			m_sUserAgent = L"Tribler";
			break;
		case 'S':
			m_sUserAgent = L"Shadow";
			break;
		case 'T':
			m_sUserAgent = L"BitTornado";
			break;
		case 'U':
			m_sUserAgent = L"UPnP NAT BT";
			break;
		default: // Unknown client using this naming.
			m_sUserAgent.Format(_T("%c"), m_oGUID[0]);
		}
		
		strVer.Format( _T(" %i.%i.%i"),
			( m_oGUID[1] - '0' ), ( m_oGUID[2] - '0' ),
			( m_oGUID[3] - '0' ) );
		m_sUserAgent += strVer;
	}
	else if  ( m_oGUID[0] == 'M' && m_oGUID[2] == '-' && m_oGUID[4] == '-' && m_oGUID[6] == '-' )
	{	// BitTorrent (Standard client, newer version)
		m_sUserAgent.Format( _T("BitTorrent %i.%i.%i"), m_oGUID[1] - '0' , m_oGUID[3] - '0' , m_oGUID[5]- '0' );
	}
	else if  ( m_oGUID[0] == 'P' && m_oGUID[1] == 'l' && m_oGUID[2] == 'u' && m_oGUID[3] == 's' )
	{	// BitTorrent Plus
		m_sUserAgent.Format( _T("BitTorrent Plus %i.%i%i%c"), m_oGUID[4] - '0', m_oGUID[5] - '0', m_oGUID[6] - '0', m_oGUID[7] );
	}
	else if  ( m_oGUID[0] == 'e' && m_oGUID[1] == 'x' && m_oGUID[2] == 'b' && m_oGUID[3] == 'c' )
	{	
		// BitLord
		if  ( m_oGUID[6] == 'L' && m_oGUID[7] == 'O' && m_oGUID[8] == 'R' && m_oGUID[9] == 'D' )
			m_sUserAgent.Format( _T("BitLord %i.%02i"), m_oGUID[4], m_oGUID[5] );
		// Old BitComet
		else 
			m_sUserAgent.Format( _T("BitComet %i.%02i"), m_oGUID[4], m_oGUID[5] );
	}
	else if  ( ( m_oGUID[0] == 'B' && m_oGUID[1] == 'S' ) || ( m_oGUID[2] == 'B' && m_oGUID[3] == 'S' ) )
	{	// BitSpirit
		m_sUserAgent.Format( _T("BitSpirit") );
	}
	else if  ( m_oGUID[0] == 'B' && m_oGUID[1] == 'T' && m_oGUID[2] == 'M' )
	{	// BTuga Revolution
		m_sUserAgent.Format( _T("BTuga Rv %i.%i"), m_oGUID[3] - '0', m_oGUID[4] - '0' );
		nNickStart = 5;
	}
	else if  ( ( m_oGUID[0] == 'b' && m_oGUID[1] == 't' && m_oGUID[2] == 'u' && m_oGUID[3] == 'g' && m_oGUID[4] == 'a' ) || ( m_oGUID[0] == 'o' && m_oGUID[1] == 'e' && m_oGUID[2] == 'r' && m_oGUID[3] == 'n' && m_oGUID[4] == 'u' ) )
	{	// BTugaXP
		m_sUserAgent.Format( _T("BTugaXP") );
	}
	else if  ( m_oGUID[0] == 'M' && m_oGUID[1] == 'b' && m_oGUID[2] == 'r' && m_oGUID[3] == 's' && m_oGUID[4] == 't' )
	{	// Burst
		m_sUserAgent.Format( _T("Burst %i.%i.%i"), m_oGUID[5] - '0', m_oGUID[7] - '0', m_oGUID[9] - '0' );
	}
	else if  ( m_oGUID[0] == 'e' && m_oGUID[1] == 'X' )
	{	// eXeem
		m_sUserAgent.Format( _T("eXeem") );
		nNickStart = 2;
	}
	else if ( m_oGUID[0] == '-' && m_oGUID[1] == 'F' && m_oGUID[2] == 'G' )
	{
		m_sUserAgent.Format( _T("FlashGet %i.%i%i"), ( ( m_oGUID[3] - '0' ) * 10 + ( m_oGUID[4] - '0' ) ), m_oGUID[5] - '0', m_oGUID[6] - '0' );
	}
	else if  ( m_oGUID[0] == '-' && m_oGUID[1] == 'G' && m_oGUID[2] == '3' )
	{	// G3 Torrent
		m_sUserAgent.Format( _T("G3 Torrent") );
		nNickStart = 3;
		nNickEnd = 11;
	}
	else if ( m_oGUID[0] == '1' && m_oGUID[1] == '0' && m_oGUID[2] == '-' && m_oGUID[3] == '-' && m_oGUID[4] == '-' && m_oGUID[5] == '-' && m_oGUID[6] == '-' && m_oGUID[7] == '-' && m_oGUID[8] == '-' )
	{
		m_sUserAgent = _T("JVTorrent");
	}
	else if ( m_oGUID[0] == 'L' && m_oGUID[1] == 'I' && m_oGUID[2] == 'M' && m_oGUID[3] == 'E' )
	{
		m_sUserAgent = _T("Limewire");
	}
	else if  ( m_oGUID[2] == 'R' && m_oGUID[3] == 'S' )
	{	// Rufus
		// The first two bits are version numbers. e.g. 0.6.5 = (0)(6+5) = char(0)char(11)
		// The same with BitMagnet
		m_sUserAgent.Format( _T("Rufus") );
		nNickStart = 4;
		nNickEnd = 12;
	}
	else if  ( m_oGUID[2] == 'B' && m_oGUID[3] == 'M' )
	{	// BitMagnet, the former Rufus
		m_sUserAgent.Format( _T("BitMagnet") );
		nNickStart = 4;
		nNickEnd = 12;
	}
	else if  ( m_oGUID[0] == '-' && m_oGUID[1] == 'M' && m_oGUID[2] == 'L' )
	{	// MLdonkey
		m_sUserAgent.Format( _T("MLdonkey %i.%i.%i"), m_oGUID[3] - '0' , m_oGUID[5] - '0' , m_oGUID[7] - '0' );
	}
	else if  ( m_oGUID[0] == 'O' && m_oGUID[1] == 'P' )
	{	// Opera
		m_sUserAgent.Format( _T("Opera %i%i%i%i"), m_oGUID[2] - '0', m_oGUID[3] - '0', m_oGUID[4] - '0', m_oGUID[5] - '0' );
	}
	else if  ( ( m_oGUID[0] == 'a' && m_oGUID[1] == '0' && m_oGUID[2] == '0' && m_oGUID[3] == '-' && m_oGUID[4] == '-' && m_oGUID[5] == '-' && m_oGUID[6] == '0' ) || ( m_oGUID[0] == 'a' && m_oGUID[1] == '0' && m_oGUID[2] == '2' && m_oGUID[3] == '-' && m_oGUID[4] == '-' && m_oGUID[5] == '-' && m_oGUID[6] == '0' ) )
	{	// Swarmy
		m_sUserAgent.Format( _T("Swarmy") );
	}
	else if ( m_oGUID[0] == '3' && m_oGUID[1] == '4' && m_oGUID[2] == '6' && m_oGUID[3] == '-' )
	{
		m_sUserAgent = _T("TorrentTopia");
	}
	else if  ( m_oGUID[0] == 'X' && m_oGUID[1] == 'B' && m_oGUID[2] == 'T' )
	{	// XBT
		m_sUserAgent.Format( _T("XBT %i.%i.%i"), m_oGUID[3] - '0', m_oGUID[4] - '0', m_oGUID[5] - '0' );
	}
	else if  ( !m_oGUID[0] && !m_oGUID[1] && !m_oGUID[2] && !m_oGUID[3] && !m_oGUID[4] && !m_oGUID[5] && !m_oGUID[6] && !m_oGUID[7] && m_oGUID[8] && m_oGUID[9] && m_oGUID[10] && m_oGUID[11] && m_oGUID[12] && m_oGUID[13] && m_oGUID[14] && m_oGUID[15] && m_oGUID[16] == 'U' && m_oGUID[17] == 'D' && m_oGUID[18] == 'P' && m_oGUID[19] == '0' )
	{	// BitSpirit	(Spoofed Client ID)	// GUID 0 - 7: 0	GUID 8 - 15: !0	GUID 16 -19: UDP0	// ToDO: Check that other clients don't use this method
		m_sUserAgent.Format( _T("BitSpirit") );
	}

	if ( ! m_bClientExtended )
	{
		m_bClientExtended = VendorCache.IsExtended( m_sUserAgent );
	}

	if ( m_sUserAgent.IsEmpty() )
	{	// Unknown peer ID string
		m_sUserAgent = m_bClientExtended ? _T("Shareaza") : _T("BitTorrent");
		theApp.Message( MSG_DEBUG, _T("[BT] Unknown client: %.20hs"), (LPCSTR)&m_oGUID[0] );
	}

	if ( nNickStart > 0 )
		for ( int i = nNickStart; i <= nNickEnd; i++ )	// Extract nick from m_oGUID
		{
			if ( m_oGUID[i] == NULL ) break;

			strNick.AppendFormat( _T("%c"), m_oGUID[i] );
		}

	if ( m_pDownloadTransfer != NULL )
	{
		m_pDownloadTransfer->m_sUserAgent = m_sUserAgent;
		m_pDownloadTransfer->m_bClientExtended = m_bClientExtended;
		if ( CDownloadSource* pSource = GetSource() )
		{
			pSource->m_sServer = m_sUserAgent;
			if ( strNick.GetLength() ) pSource->m_sNick = strNick;
			pSource->m_bClientExtended = ( m_bClientExtended && ! pSource->m_bPushOnly);
		}
	}

	if ( m_pUpload != NULL )
	{
		m_pUpload->m_sUserAgent = m_sUserAgent;
		if ( strNick.GetLength() ) m_pUpload->m_sNick = strNick;
		m_pUpload->m_bClientExtended = m_bClientExtended;
	}
}

//////////////////////////////////////////////////////////////////////
// CBTClient online handler

BOOL CBTClient::OnOnline()
{
	if ( m_bClosing ) return FALSE;
	ASSERT( m_bOnline );
	ASSERT( m_pDownload != NULL );
	ASSERT( m_pUpload != NULL );
	
	theApp.Message( MSG_INFO, IDS_BT_CLIENT_ONLINE, (LPCTSTR)m_sAddress,
		(LPCTSTR)m_pDownload->GetDisplayName() );
	
	if ( !m_pDownload->IsTorrent() ) // perhaps we just finished download; investigate this!
	{
		m_pDownload = NULL;
		theApp.Message( MSG_ERROR, IDS_BT_CLIENT_UNKNOWN_FILE, (LPCTSTR)m_sAddress );
		Close();
		return FALSE;
	}

	if ( m_bExtended )
		SendExtendedHandshake();

	if ( m_bClientExtended )
		SendBeHandshake();

	if ( CBTPacket* pBitfield = m_pDownload->CreateBitfieldPacket() )
		Send( pBitfield );
	
	if ( m_pDownloadTransfer != NULL && ! m_pDownloadTransfer->OnConnected() ) return FALSE;
	if ( ! m_pUpload->OnConnected() ) return FALSE;
	if ( Uploads.GetTorrentUploadCount() < Settings.BitTorrent.UploadCount )
	{
		m_pUpload->m_bChoked = FALSE;
		Send( CBTPacket::New( BT_PACKET_UNCHOKE ) );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient packet switch

BOOL CBTClient::OnPacket(CBTPacket* pPacket)
{
	if ( m_bClosing ) return FALSE;

	switch ( pPacket->m_nType )
	{
	case BT_PACKET_KEEPALIVE:
		break;
	case BT_PACKET_CHOKE:
		if ( m_pDownloadTransfer != NULL && ! m_pDownloadTransfer->OnChoked( pPacket ) ) return FALSE;
		m_pDownload->ChokeTorrent();
		break;
	case BT_PACKET_UNCHOKE:
		if ( m_pDownloadTransfer != NULL && ! m_pDownloadTransfer->OnUnchoked( pPacket ) ) return FALSE;
		m_pDownload->ChokeTorrent();
		break;
	case BT_PACKET_INTERESTED:
		if ( ! m_pUpload->OnInterested( pPacket ) ) return FALSE;
		m_pDownload->ChokeTorrent();
		break;
	case BT_PACKET_NOT_INTERESTED:
		if ( ! m_pUpload->OnUninterested( pPacket ) ) return FALSE;
		m_pDownload->ChokeTorrent();
		break;
	case BT_PACKET_HAVE:
		return m_pDownloadTransfer == NULL || m_pDownloadTransfer->OnHave( pPacket );
	case BT_PACKET_BITFIELD:
			return m_pDownloadTransfer == NULL || m_pDownloadTransfer->OnBitfield( pPacket );
	case BT_PACKET_REQUEST:
		return m_pUpload->OnRequest( pPacket );
	case BT_PACKET_PIECE:
		return m_pDownloadTransfer == NULL || m_pDownloadTransfer->OnPiece( pPacket );
	case BT_PACKET_CANCEL:
		return m_pUpload->OnCancel( pPacket );
	case BT_PACKET_DHT_PORT:
		return OnDHTPort( pPacket );
	case BT_PACKET_EXTENSION:
		return OnExtended( pPacket );
	case BT_PACKET_HANDSHAKE:
		if ( ! m_bClientExtended )
			break;
		return OnBeHandshake( pPacket );
	case BT_PACKET_SOURCE_REQUEST:
		if ( ! m_bExchange )
			break;
		return OnSourceRequest( pPacket );
	case BT_PACKET_SOURCE_RESPONSE:
		if ( ! m_bExchange )
			break;
		return m_pDownloadTransfer == NULL || m_pDownloadTransfer->OnSourceResponse( pPacket );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient advanced handshake

void CBTClient::SendBeHandshake()
{	
	if ( m_bClosing ) return;
	// Send extended handshake (for G2 capable clients)
	CBENode pRoot;
	
	CString strNick = MyProfile.GetNick().Left( 255 ); // Truncate to 255 characters
	if ( strNick.GetLength() ) pRoot.Add( "nickname" )->SetString( strNick );

	if ( ( m_pDownload ) && ( m_pDownload->m_pTorrent.m_bPrivate ) )
		pRoot.Add( "source-exchange" )->SetInt( 0 );
	else
		pRoot.Add( "source-exchange" )->SetInt( 2 );
	
	pRoot.Add( "user-agent" )->SetString( Settings.SmartAgent() );
	
	CBuffer pOutput;
	pRoot.Encode( &pOutput );
	
	CBTPacket* pPacket = CBTPacket::New( BT_PACKET_HANDSHAKE );
	pPacket->Write( pOutput.m_pBuffer, pOutput.m_nLength );
	Send( pPacket );
}

BOOL CBTClient::OnBeHandshake(CBTPacket* pPacket)
{	
	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_bClosing ) return TRUE;
	// On extended handshake (for G2 capable clients)
	if ( pPacket->GetRemaining() > 1024 ) return TRUE;
	
	CBuffer pInput;
	pInput.Add( pPacket->m_pBuffer, pPacket->GetRemaining() );
	
	CBENode* pRoot = CBENode::Decode( &pInput );
	if ( pRoot == NULL ) return TRUE;
	
	CBENode* pAgent = pRoot->GetNode( "user-agent" );
	
	if ( pAgent->IsType( CBENode::beString ) )
	{
		m_sUserAgent = pAgent->GetString();
		
		if ( m_pDownloadTransfer != NULL )
		{
			m_pDownloadTransfer->m_sUserAgent = m_sUserAgent;
			if ( CDownloadSource* pSource = GetSource() )
			{
				pSource->m_sServer = m_sUserAgent;
				pSource->m_bClientExtended = TRUE;
			}
		}
		
		if ( m_pUpload != NULL ) 
		{
			m_pUpload->m_sUserAgent = m_sUserAgent;
			m_pUpload->m_bClientExtended = TRUE;
		}
	}
	
	CBENode* pNick = pRoot->GetNode( "nickname" );
	
	if ( pNick->IsType( CBENode::beString ) )
	{
		if ( CDownloadSource* pSource = GetSource() )
		{
			pSource->m_sNick = pNick->GetString();
		}
	}
	
	if ( CBENode* pExchange = pRoot->GetNode( "source-exchange" ) )
	{
		if ( pExchange->GetInt() >= 2 )
		{
			if ( ( m_pDownload ) && ( m_pDownload->m_pTorrent.m_bPrivate ) )
			{
				m_bExchange = FALSE;
			}
			else
			{
				m_bExchange = TRUE;
				
				if ( m_pDownloadTransfer != NULL )
					Send( CBTPacket::New( BT_PACKET_SOURCE_REQUEST ) );
			}
		}
	}
	
	delete pRoot;
	
	theApp.Message( MSG_INFO, IDS_BT_CLIENT_EXTENDED, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient source request

BOOL CBTClient::OnSourceRequest(CBTPacket* /*pPacket*/)
{
	if ( m_bClosing ) return TRUE;
	if ( ( m_pDownload == NULL ) || ( m_pDownload->m_pTorrent.m_bPrivate ) ) return TRUE;
	
	CBENode pRoot;
	CBENode* pPeers = pRoot.Add( "peers" );
	
	for ( POSITION posSource = m_pDownload->GetIterator(); posSource ; )
	{
		CDownloadSource* pSource = m_pDownload->GetNext( posSource );

		if ( ! pSource->IsConnected() ) continue;
		
		if ( pSource->m_nProtocol == PROTOCOL_BT )
		{
			CBENode* pPeer = pPeers->Add();
			CShareazaURL pURL;
			
			if ( pURL.Parse( pSource->m_sURL ) && pURL.m_oBTC )
			{
                pPeer->Add( "peer id" )->SetString( pURL.m_oBTC.begin(), Hashes::BtGuid::byteCount );
			}
			
			pPeer->Add( "ip" )->SetString( CString( inet_ntoa( pSource->m_pAddress ) ) );
			pPeer->Add( "port" )->SetInt( pSource->m_nPort );
		}
		else if (	pSource->m_nProtocol == PROTOCOL_HTTP &&
					pSource->m_bReadContent == TRUE &&
					pSource->m_bPushOnly == FALSE )
		{
			CBENode* pPeer = pPeers->Add();
			pPeer->Add( "url" )->SetString( pSource->m_sURL );
		}
	}
	
	if ( pPeers->GetCount() == 0 ) return TRUE;
	
	CBuffer pOutput;
	pRoot.Encode( &pOutput );
	
	CBTPacket* pResponse = CBTPacket::New( BT_PACKET_SOURCE_RESPONSE );
	pResponse->Write( pOutput.m_pBuffer, pOutput.m_nLength );
	Send( pResponse );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient DHT

BOOL CBTClient::OnDHTPort(CBTPacket* pPacket)
{
	if ( pPacket && pPacket->GetRemaining() == 2 )
	{
//		WORD nPort = pPacket->ReadShortBE();
//		SOCKADDR_IN addr = m_pHost;
//		addr.sin_port = nPort;
//		Datagrams.DHTPing( &addr );
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient Extension

#define EXTENDED_PACKET_HANDSHAKE	0
#define EXTENDED_PACKET_UT_METADATA	1
#define UT_METADATA_REQUEST			0
#define UT_METADATA_DATA			1
#define UT_METADATA_REJECT			2

void CBTClient::SendExtendedPacket(BYTE Type, CBuffer *pOutput)
{
	CBTPacket *pResponse = CBTPacket::New( BT_PACKET_EXTENSION );
	pResponse->WriteByte( Type );
	pResponse->Write( pOutput->m_pBuffer, pOutput->m_nLength );
	Send( pResponse );
}

void CBTClient::SendExtendedHandshake()
{
	CBENode pRoot;

	pRoot.Add( "m" )->Add( "ut_metadata" )->SetInt( EXTENDED_PACKET_UT_METADATA );
	if ( m_pDownload->IsTorrent() && ! m_pDownload->m_pTorrent.m_bPrivate )
	{
		pRoot.Add( "metadata_size" )->SetInt( m_pDownload->m_pTorrent.GetInfoSize() );
	}

	pRoot.Add( "p" )->SetInt( Settings.Connection.InPort );
	pRoot.Add( "v" )->SetString( Settings.SmartAgent() );

	CBuffer pOutput;
	pRoot.Encode( &pOutput );
	pRoot.Clear();

	SendExtendedPacket( EXTENDED_PACKET_HANDSHAKE, &pOutput );
}

BOOL CBTClient::OnExtended(CBTPacket* pPacket)
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( pPacket->GetRemaining() > 100 * 1024 )
		return TRUE;

	if ( m_bClosing )
		return TRUE;

	BYTE nPacketID = pPacket->ReadByte();

	CBuffer pInput;
	pInput.Add( &pPacket->m_pBuffer[ pPacket->m_nPosition ], pPacket->GetRemaining() );

	CBENode* pRoot = CBENode::Decode( &pInput );
	if ( pRoot == NULL )
		return TRUE;

	if ( nPacketID == EXTENDED_PACKET_HANDSHAKE )
	{
		if ( CBENode* pMetadata = pRoot->GetNode( "m" ) )
		{
			CBENode* pUtMetadata = pMetadata->GetNode( "ut_metadata" );
			if ( ! m_dUtMetadataID && pUtMetadata )
			{
				m_dUtMetadataID = pUtMetadata->GetInt();
				if ( m_dUtMetadataID > 0 && ! m_pDownload->m_pTorrent.m_pBlockBTH ) // Send first info request
				{
					int nNextPiece = m_pDownload->m_pTorrent.NextInfoPiece();
					if ( nNextPiece >= 0 )
						SendInfoRequest( nNextPiece );
				}
			}
			
			if ( CBENode* pUtMetadataSize = pMetadata->GetNode( "metadata_size" ) )
			{
				m_dUtMetadataSize = pUtMetadata->GetInt();
			}
		}
	}
	else if ( nPacketID == EXTENDED_PACKET_UT_METADATA ) 
	{ 
		QWORD nMsgType	= pRoot->GetNode( "msg_type" )->GetInt();
		QWORD nPiece	= pRoot->GetNode( "piece" )->GetInt();

		if ( nMsgType == UT_METADATA_REQUEST )
		{
			CBENode pRoot2;
			pRoot2.Add( "piece" )->SetInt( nPiece );

			CBuffer pOutput;

			BYTE* pInfoPiece = NULL;
			DWORD InfoLen = m_pDownload->m_pTorrent.GetInfoPiece( nPiece, pInfoPiece );
			if ( InfoLen == 0 || m_pDownload->m_pTorrent.m_bPrivate )
			{
				pRoot2.Add( "msg_type" )->SetInt( UT_METADATA_REJECT );
				pRoot2.Encode( &pOutput );
			}
			else
			{
				pRoot2.Add( "msg_type"		)->SetInt( UT_METADATA_DATA );
				pRoot2.Add( "total_size"	)->SetInt( m_pDownload->m_pTorrent.GetInfoSize() );
				pRoot2.Encode( &pOutput );
				pOutput.Add( pInfoPiece, InfoLen );
			}

			SendExtendedPacket( m_dUtMetadataID, &pOutput );
		}
		else if ( nMsgType == UT_METADATA_DATA )
		{
			if ( ! m_pDownload->m_pTorrent.m_pBlockBTH )
			{
				QWORD nTotalSize = pRoot->GetNode( "total_size" )->GetInt();
				if ( m_pDownload->m_pTorrent.LoadInfoPiece( nTotalSize, nPiece, pPacket->m_pBuffer, pPacket->m_nLength ) ) // If full info loaded
				{
					 m_pDownload->SetTorrent( m_pDownload->m_pTorrent );
				}
				else
				{
					int nNextPiece = m_pDownload->m_pTorrent.NextInfoPiece();
					if ( nNextPiece >= 0 && m_dUtMetadataID > 0 )
						SendInfoRequest( nNextPiece );
				}
			}
		}
	}
	else
	{
		ASSERT( nPacketID );
	}

	delete pRoot;

	return TRUE;
}

void CBTClient::SendInfoRequest(QWORD nPiece)
{
	ASSERT( m_dUtMetadataID );

	CBENode pRoot;
	pRoot.Add( "msg_type"	)->SetInt( UT_METADATA_REQUEST ); //Request
	pRoot.Add( "piece"		)->SetInt( nPiece );
	
	CBuffer pOutput;
	pRoot.Encode( &pOutput );

	SendExtendedPacket( m_dUtMetadataID, &pOutput );
}
