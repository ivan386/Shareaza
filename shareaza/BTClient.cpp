//
// BTClient.cpp
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
#include "BENode.h"
#include "BTClient.h"
#include "BTClients.h"
#include "BTPacket.h"
#include "Buffer.h"
#include "Datagrams.h"
#include "Download.h"
#include "DownloadSource.h"
#include "DownloadTransferBT.h"
#include "Downloads.h"
#include "GProfile.h"
#include "Network.h"
#include "Security.h"
#include "ShareazaURL.h"
#include "Statistics.h"
#include "Transfers.h"
#include "UploadTransferBT.h"
#include "Uploads.h"
#include "VendorCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CBTClient construction

CBTClient::CBTClient()
	: CTransfer				( PROTOCOL_BT )
	, m_bExtended			( FALSE )
	, m_bDHTPort			( FALSE )
	, m_pUploadTransfer		( NULL )
	, m_bSeeder				( FALSE )
	, m_bPrefersEncryption	( FALSE )
	, m_pDownload			( NULL )
	, m_pDownloadTransfer	( NULL )
	, m_bShake				( FALSE )
	, m_bOnline				( FALSE )
	, m_bClosing			( FALSE )
	, m_tLastKeepAlive		( GetTickCount() )
	, m_tLastUtPex			( 0 )
	, m_nUtMetadataID		( 0 )	// 0 or BT_EXTENSION_UT_METADATA
	, m_nUtMetadataSize		( 0 )
	, m_nUtPexID			( 0 )	// 0 or BT_EXTENSION_UT_PEX
	, m_nLtTexID			( 0 )	// 0 or BT_EXTENSION_LT_TEX
	, m_nSrcExchangeID		( 0 )	// 0 or BT_HANDSHAKE_SOURCE
{
	m_sUserAgent = protocolNames[ PROTOCOL_BT ];
	m_mInput.pLimit = m_mOutput.pLimit = &Settings.Bandwidth.Request;

	if ( Settings.General.DebugBTSources )
		theApp.Message( MSG_DEBUG, L"Adding BT client to collection: %s", (LPCTSTR)m_sAddress );

	BTClients.Add( this );
}

CBTClient::~CBTClient()
{
	ASSERT( ! IsValid() );
	ASSERT( m_pDownloadTransfer == NULL );
	ASSERT( m_pDownload == NULL );
	ASSERT( m_pUploadTransfer == NULL );

	if ( Settings.General.DebugBTSources )
		theApp.Message( MSG_DEBUG, L"Removing BT client from collection: %s", (LPCTSTR)m_sAddress );

	BTClients.Remove( this );
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

	theApp.Message( MSG_INFO, IDS_BT_CLIENT_CONNECTING, (LPCTSTR)m_sAddress );

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
		theApp.Message( MSG_DEBUG, L"Attaching new BT client connection: %s", (LPCTSTR)m_sAddress );

	ASSERT( m_mInput.pLimit != NULL );
	m_tConnected = GetTickCount();
	theApp.Message( MSG_INFO, IDS_BT_CLIENT_ACCEPTED, (LPCTSTR)m_sAddress );
}

//////////////////////////////////////////////////////////////////////
// CBTClient close

void CBTClient::Close(UINT nError)
{
	ASSERT( this != NULL );

	m_mInput.pLimit = NULL;
	m_mOutput.pLimit = NULL;

	if ( m_bClosing ) return;
	m_bClosing = TRUE;

	if ( Settings.General.DebugBTSources )
		theApp.Message( MSG_DEBUG, L"Deleting BT client: %s", (LPCTSTR)m_sAddress );

	if ( m_pUploadTransfer != NULL ) m_pUploadTransfer->Close();
	ASSERT( m_pUploadTransfer == NULL );

	if ( m_pDownloadTransfer != NULL )
	{
		if ( ( m_pDownload == NULL ) || ( m_pDownload->IsCompleted() ) )
			m_pDownloadTransfer->Close( TRI_FALSE );
		else
			m_pDownloadTransfer->Close( TRI_UNKNOWN );
	}
	ASSERT( m_pDownloadTransfer == NULL );

	m_pDownload = NULL;

	CTransfer::Close( nError );

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

		Statistics.Current.BitTorrent.Outgoing++;

		pPacket->SmartDump( &m_pHost, FALSE, TRUE );

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
			Close( IDS_BT_CLIENT_CONNECT_TIMEOUT );
			return FALSE;
		}
	}
	else if ( ! m_bOnline )
	{
		if ( tNow - m_tConnected > Settings.Connection.TimeoutHandshake )
		{
			Close( IDS_BT_CLIENT_HANDSHAKE_TIMEOUT );
			return FALSE;
		}
	}
	else
	{
		if ( tNow - m_mInput.tLast > Settings.BitTorrent.LinkTimeout * 2 )
		{
			Close( IDS_BT_CLIENT_LOST );
			return FALSE;
		}
		else if ( tNow - m_tLastKeepAlive > Settings.BitTorrent.LinkPing )
		{
			Send( CBTPacket::New( BT_PACKET_KEEPALIVE ) );
			m_tLastKeepAlive = tNow;
		}

		ASSERT ( m_pUploadTransfer != NULL );
		if ( tNow > m_tLastUtPex + Settings.BitTorrent.UtPexPeriod )
		{
			SendUtPex( m_tLastUtPex );
			m_tLastUtPex = tNow;
		}

		if ( m_pDownloadTransfer != NULL && ! m_pDownloadTransfer->OnRun() ) return FALSE;
		if ( m_pUploadTransfer == NULL || ! m_pUploadTransfer->OnRun() ) return FALSE;
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
		Close( IDS_BT_CLIENT_DROP_CONNECTING );
	else if ( ! m_bOnline )
		Close( IDS_BT_CLIENT_DROP_HANDSHAKE );
	else
		Close( IDS_BT_CLIENT_DROP_CONNECTED );
}

//////////////////////////////////////////////////////////////////////
// CBTClient write event

BOOL CBTClient::OnWrite()
{
	if ( m_bClosing )
		return FALSE;

	CTransfer::OnWrite();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient read event

BOOL CBTClient::OnRead()
{
	if ( m_bClosing )
		return FALSE;

	if ( ! CTransfer::OnRead() )
		return FALSE;

	CLockedBuffer pInput( GetInput() );

	BOOL bSuccess = TRUE;
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
				if ( ! m_bOnline )
					bSuccess = FALSE;
			}

			pPacket->Release();

			if ( ! bSuccess )
				break;

			if ( m_bClosing )
				return FALSE;
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
		}
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CBTClient handshaking

void CBTClient::SendHandshake(BOOL bPart1, BOOL bPart2)
{
	if ( m_bClosing )
		return;

	ASSERT( m_pDownload != NULL );

	if ( bPart1 )
	{
		const QWORD nFlags = BT_FLAG_EXTENSION | BT_FLAG_DHT_PORT;
		Write( _P(BT_PROTOCOL_HEADER) );
		Write( &nFlags, 8 );
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
	// First part of the handshake
	ASSERT( ! m_bOnline );
	ASSERT( ! m_bShake );

	// Read in the BT protocol header
	if ( ! StartsWith( BT_PROTOCOL_HEADER, BT_PROTOCOL_HEADER_LEN ) )
	{
		Close( IDS_HANDSHAKE_FAIL );
		return FALSE;
	}
	RemoveFromInput( BT_PROTOCOL_HEADER_LEN );

	QWORD nFlags = 0;
	Read( &nFlags, 8 );
	m_bExtended = ( nFlags & BT_FLAG_EXTENSION ) != 0;
	m_bDHTPort = ( nFlags & BT_FLAG_DHT_PORT ) != 0;

	// Read in the file ID
	Hashes::BtHash oFileHash;
	Read( oFileHash );
	oFileHash.validate();

	theApp.Message( MSG_DEBUG, _T("BitTorrent coupling with %s requested file: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)oFileHash.toString() );

	if ( m_pDownload != NULL )	// If we initiated download (download has associated, which means we initiated download)
	{
		ASSERT( m_pDownloadTransfer != NULL );

		if ( validAndUnequal( oFileHash, m_pDownload->m_oBTH ) )
		{	//Display an error and exit
			Close( IDS_BT_CLIENT_WRONG_FILE );
			return FALSE;
		}
		else if ( ! m_pDownload->IsTrying() && ! m_pDownload->IsSeeding() )
		{	//Display an error and exit
			Close( IDS_BT_CLIENT_INACTIVE_FILE );
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
			Close( IDS_BT_CLIENT_UNKNOWN_FILE );
			return FALSE;
		}
		else if ( ! m_pDownload->IsTrying() && ! m_pDownload->IsSeeding() )	// If the file isn't active
		{	//Display an error and exit
			m_pDownload = NULL;
			Close( IDS_BT_CLIENT_INACTIVE_FILE );
			return FALSE;
		}
		else if ( m_pDownload->UploadExists( &m_pHost.sin_addr ) )	// If there is already an upload of this file to this client
		{
			// Display an error and exit
			m_pDownload = NULL;
			Close( IDS_BT_CLIENT_DUPLICATE );
			return FALSE;
		}
		// The file isn't verified yet, close the connection
		else if ( m_pDownload->m_bVerify != TRI_TRUE
			&& ( m_pDownload->IsMoving() ||  m_pDownload->IsCompleted() ) )
		{
			Close( IDS_BT_CLIENT_INACTIVE_FILE );
			return FALSE;
		}


		// Check we don't have too many active torrent connections
		// (Prevent routers overloading for very popular torrents)
		if ( ( m_pDownload->GetTransferCount( dtsCountTorrentAndActive ) ) > ( Settings.BitTorrent.DownloadConnections * 1.25 ) )
		{
			Close( IDS_BT_CLIENT_MAX_CONNECTIONS );
			return FALSE;
		}

	}

	// Verify a download and hash
	ASSERT( m_pDownload != NULL );
	ASSERT( validAndEqual( m_pDownload->m_oBTH, oFileHash ) );

	// If we didn't start the connection, then send a handshake
	if ( ! m_bInitiated )
		SendHandshake( TRUE, TRUE );
	m_bShake = TRUE;

	return TRUE;
}

BOOL CBTClient::OnHandshake2()
{
	ASSUME_LOCK( Transfers.m_pSection );

	// Second part of the handshake - Peer ID
	Read( m_oGUID );
	m_oGUID.validate();

	DetermineUserAgent( m_oGUID );

	if ( Security.IsClientBanned( m_sUserAgent ) )
	{
		Close( IDS_SECURITY_BANNED_USERAGENT );
		return FALSE;
	}

	ASSERT( m_pDownload != NULL );

	if ( CDownloadSource* pSource = GetSource() )	// Transfer exist, so must be initiated from this side
	{
		pSource->m_oGUID = transformGuid( m_oGUID );

		/*
		//ToDo: This seems to trip when it shouldn't. Should be investigated...
		if ( memcmp( &m_pGUID, &pSource->m_pGUID, 16 ) != 0 )
		{
			Close( IDS_BT_CLIENT_WRONG_GUID );
			return FALSE;
		}
		*/
	}
	else	// no transfer exist, so must be initiated from other side.
	{
		if ( m_pDownload->UploadExists( m_oGUID ) )
		{
			Close( IDS_BT_CLIENT_DUPLICATE );
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
					Close( IDS_BT_CLIENT_UNKNOWN_FILE );
					return FALSE;
				}
			}
		}
	}

	ASSERT( m_pUploadTransfer == NULL );
	if ( Settings.General.DebugBTSources )
		theApp.Message( MSG_DEBUG, L"Creating new BT upload: %s", (LPCTSTR)m_sAddress );
	m_pUploadTransfer = new CUploadTransferBT( this, m_pDownload );

	m_bOnline = TRUE;

	theApp.Message( MSG_INFO, IDS_BT_CLIENT_ONLINE, (LPCTSTR)m_sAddress,
		(LPCTSTR)m_pDownload->GetDisplayName() );

	if ( ! m_pDownload->IsTorrent() ) // perhaps we just finished download; investigate this!
	{
		m_pDownload = NULL;
		Close( IDS_BT_CLIENT_UNKNOWN_FILE );
		return FALSE;
	}

	if ( m_bExtended )
		SendExtendedHandshake();

	if ( m_bDHTPort )
		SendDHTPort();

	if ( m_bClientExtended )
		SendBeHandshake();

	if ( CBTPacket* pBitfield = m_pDownload->CreateBitfieldPacket() )
		Send( pBitfield );

	if ( m_pDownloadTransfer != NULL && ! m_pDownloadTransfer->OnConnected() )
		return FALSE;

	if ( ! m_pUploadTransfer->OnConnected() )
		return FALSE;

	if ( Uploads.GetTorrentUploadCount() < Settings.BitTorrent.UploadCount )
	{
		m_pUploadTransfer->m_bChoked = FALSE;
		UnChoke();
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient online handler

int charint( BYTE ch )
{
	if ( '0' <= ch && ch <= '9' )
		return ch - '0';
	if ( 'A' <= ch && ch <= 'Z' )
		return 10 + ch - 'A';
	if ( 'a' <= ch && ch <= 'z' )
		return 36 + ch - 'a';
	return 0;
}

int strint( LPCBYTE pch, int span )
{
	char tmp[ 64 ];
	memcpy( tmp, pch, span );
	tmp[ span ] = '\0';
	return (int)strtol( tmp, NULL, 0 );
}

LPCTSTR getMnemonicEnd( BYTE ch )
{
	switch ( ch )
	{
	case 'b':
	case 'B':
	return _T( " (Beta)" );
	case 'd':
	return _T( " (Debug)" );
	case 'x':
	case 'X':
	case 'Z':
	return _T( " (Dev)" );
	default:
	return _T( "" );
	}
}

enum
{
	STYLE_2_DIGITS,	// 2 digits
	STYLE_3_DIGITS,	// 3 digits
	STYLE_4_DIGITS,	// 4 digits
	STYLE_2_INTS  ,	// 2 digits (2 chars each)
	STYLE_AZUREUS ,	// Azureus/Vuze style (same as 4 digits)
	STYLE_BUDDY   ,	// BitBuddy style
	STYLE_CTORRENT,	// CTorrent style
	STYLE_ROCKET  ,	// BitRocket style
	STYLE_UTORRENT,	// uTorrent style
	STYLE_XTORRENT,	// xTorrent style
	STYLE_NO_VERS	// no version
};

// Azureus style "-SSVVVV-" http://bittorrent.org/beps/bep_0020.html
CString CBTClient::GetAzureusStyleUserAgent(const BYTE* pVendor, size_t nVendor)
{
	struct azureusStyleEntry
	{
		int style;
		BYTE signature[ 2 ];
		LPCTSTR client;
	};
	static const azureusStyleEntry azureusStyleClients[] =
	{
		{ STYLE_3_DIGITS, 'A', 'G', L"Ares" },
		{ STYLE_4_DIGITS, 'A', 'R', L"Arctic" },
		{ STYLE_4_DIGITS, 'A', 'T', L"Artemis" },
		{ STYLE_4_DIGITS, 'A', 'V', L"Avicora" },
		{ STYLE_2_INTS  , 'A', 'X', L"BitPump" },
		{ STYLE_AZUREUS , 'A', 'Z', L"Azureus" },	// Vuze
		{ STYLE_3_DIGITS, 'A', '~', L"Ares" },
		{ STYLE_BUDDY   , 'B', 'B', L"BitBuddy" },
		{ STYLE_2_INTS  , 'B', 'C', L"BitComet" },
		{ STYLE_4_DIGITS, 'B', 'E', L"BitTorrent SDK" },
		{ STYLE_NO_VERS , 'B', 'F', L"Bitflu" },
		{ STYLE_4_DIGITS, 'B', 'G', L"BTGetit" },
		{ STYLE_4_DIGITS, 'B', 'H', L"BitZilla" },
		{ STYLE_4_DIGITS, 'b', 'k', L"BitKitten" },
		{ STYLE_4_DIGITS, 'B', 'M', L"BitMagnet" },
		{ STYLE_4_DIGITS, 'B', 'O', L"Bits on Wheels" },
		{ STYLE_4_DIGITS, 'B', 'P', L"BitTorrent Pro (Azureus + Spyware)" },
		{ STYLE_ROCKET  , 'B', 'R', L"BitRocket" },
		{ STYLE_4_DIGITS, 'B', 'S', L"BitSlave" },
		{ STYLE_UTORRENT, 'B', 'T', L"BitTorrent" },
		{ STYLE_4_DIGITS, 'B', 'W', L"BitWombat" },
		{ STYLE_4_DIGITS, 'B', 'X', L"BitTorrent X" },
		{ STYLE_4_DIGITS, 'C', 'B', L"ShareazaPlus" },		// RazaCB Core
		{ STYLE_2_INTS  , 'C', 'D', L"Enhanced CTorrent" },
		{ STYLE_CTORRENT, 'C', 'T', L"CTorrent" },
		{ STYLE_4_DIGITS, 'D', 'E', L"DelugeTorrent" },
		{ STYLE_4_DIGITS, 'D', 'P', L"Propogate Data Client" },
		{ STYLE_4_DIGITS, 'E', 'B', L"EBit" },
		{ STYLE_4_DIGITS, 'E', 'N', L"Envy" },
		{ STYLE_4_DIGITS, 'E', 'V', L"Envy" },
		{ STYLE_3_DIGITS, 'E', 'S', L"Electric Sheep" },
		{ STYLE_4_DIGITS, 'F', 'C', L"FileCroc" },
		{ STYLE_4_DIGITS, 'F', 'T', L"FoxTorrent/RedSwoosh" },
		{ STYLE_4_DIGITS, 'G', 'R', L"GetRight" },
		{ STYLE_4_DIGITS, 'G', 'S', L"GSTorrent" },
		{ STYLE_4_DIGITS, 'H', 'K', L"Hekate" },
		{ STYLE_3_DIGITS, 'H', 'L', L"Halite" },
		{ STYLE_4_DIGITS, 'H', 'N', L"Hydranode" },
		{ STYLE_4_DIGITS, 'K', 'G', L"KGet" },
		{ STYLE_3_DIGITS, 'K', 'T', L"KTorrent" },
		{ STYLE_4_DIGITS, 'L', 'C', L"LeechCraft" },
		{ STYLE_4_DIGITS, 'L', 'H', L"LH-ABC" },
		{ STYLE_4_DIGITS, 'L', 'K', L"Linkage" },
		{ STYLE_2_INTS  , 'L', 'P', L"Lphant" },
		{ STYLE_3_DIGITS, 'L', 'T', L"libtorrent" },
		{ STYLE_3_DIGITS, 'l', 't', L"rTorrent" },
		{ STYLE_NO_VERS , 'L', 'W', L"LimeWire" },
		{ STYLE_2_DIGITS, 'M', 'G', L"MediaGet" },
		{ STYLE_4_DIGITS, 'M', 'K', L"Meerkat" },
		{ STYLE_4_DIGITS, 'M', 'O', L"MonoTorrent" },
		{ STYLE_3_DIGITS, 'M', 'P', L"MooPolice" },
		{ STYLE_4_DIGITS, 'M', 'R', L"Miro" },
		{ STYLE_4_DIGITS, 'M', 'T', L"MoonlightTorrent" },
		{ STYLE_4_DIGITS, 'N', 'X', L"Net Transport" },
		{ STYLE_4_DIGITS, 'O', 'S', L"OneSwarm" },
		{ STYLE_4_DIGITS, 'O', 'T', L"OmegaTorrent" },
		{ STYLE_3_DIGITS, 'p', 'b', L"pbTorrent" },
		{ STYLE_4_DIGITS, 'P', 'C', L"CacheLogic" },
		{ STYLE_4_DIGITS, 'P', 'D', L"Pando" },
		{ STYLE_4_DIGITS, 'P', 'E', L"PeerProject" },
		{ STYLE_4_DIGITS, 'p', 'X', L"pHoeniX" },
		{ STYLE_3_DIGITS, 'q', 'B', L"qBittorrent" },
		{ STYLE_4_DIGITS, 'Q', 'D', L"QQDownload" },
		{ STYLE_4_DIGITS, 'Q', 'T', L"QT4" },
		{ STYLE_4_DIGITS, 'R', 'S', L"Rufus" },
		{ STYLE_4_DIGITS, 'R', 'T', L"Retriever" },
		{ STYLE_4_DIGITS, 'R', 'Z', L"RezTorrent" },
		{ STYLE_4_DIGITS, 'S', 'B', L"SwiftBit" },
		{ STYLE_4_DIGITS, 'S', 'D', L"Thunder" },
		{ STYLE_4_DIGITS, 'S', 'M', L"SoMud" },
		{ STYLE_4_DIGITS, 'S', 'N', L"ShareNet" },
		{ STYLE_4_DIGITS, 'S', 'S', L"SwarmScope" },
		{ STYLE_4_DIGITS, 's', 't', L"SharkTorrent" },
		{ STYLE_4_DIGITS, 'S', 'T', L"SymTorrent" },
		{ STYLE_4_DIGITS, 'S', 'Z', L"Shareaza" },
		{ STYLE_4_DIGITS, 'S', '~', L"Shareaza (Beta)" },
		{ STYLE_4_DIGITS, 'T', 'N', L"Torrent.NET" },
		{ STYLE_4_DIGITS, 'T', 'R', L"Transmission" },
		{ STYLE_4_DIGITS, 'T', 'S', L"TorrentStorm" },
		{ STYLE_3_DIGITS, 'T', 'T', L"TuoTu" },
		{ STYLE_UTORRENT, 'U', 'E', L"\x00b5Torrent Embedded" },
		{ STYLE_4_DIGITS, 'U', 'L', L"uLeecher!" },
		{ STYLE_UTORRENT, 'U', 'M', L"\x00b5Torrent Mac" },
		{ STYLE_UTORRENT, 'U', 'T', L"\x00b5Torrent" },
		{ STYLE_4_DIGITS, 'V', 'G', L"Vagaa" },
		{ STYLE_4_DIGITS, 'W', 'T', L"BitLet" },
		{ STYLE_4_DIGITS, 'W', 'Y', L"FireTorrent" },
		{ STYLE_XTORRENT, 'X', 'C', L"xTorrent" },
		{ STYLE_4_DIGITS, 'X', 'L', L"Xunlei" },
		{ STYLE_4_DIGITS, 'X', 'S', L"XSwifter" },
		{ STYLE_4_DIGITS, 'X', 'T', L"XanTorrent" },
		{ STYLE_XTORRENT, 'X', 'X', L"xTorrent" },
		{ STYLE_4_DIGITS, 'Z', 'O', L"Zona" },
		{ STYLE_4_DIGITS, 'Z', 'T', L"ZipTorrent" }
	};

	CString sUserAgent;
	if ( pVendor )
	{
		for ( size_t i = 0; i < _countof( azureusStyleClients ); ++i )
		{
			if ( pVendor[ 0 ] == azureusStyleClients[ i ].signature[ 0 ] &&
				 pVendor[ 1 ] == azureusStyleClients[ i ].signature[ 1 ] )
			{
				sUserAgent = azureusStyleClients[ i ].client;
				/* TODO: Decode this version
				if ( nVendor == 4 )
					sUserAgent.AppendFormat( _T( "(%u)" ), ntohs( *(WORD*)&pVendor[ 2 ] ) );
				else*/ if ( nVendor == 6 )
				{
					sUserAgent += _T(" ");
					switch ( azureusStyleClients[ i ].style )
					{
					case STYLE_AZUREUS:
						if ( pVendor[ 2 ] > '3' || ( pVendor[ 2 ] == '3' && pVendor[ 3 ] >= '1' ) )
							// Vuze starts at version 3.1.0.0
							sUserAgent = L"Vuze";
						// and use STYLE_4_DIGITS
					case STYLE_4_DIGITS:
						sUserAgent.AppendFormat( _T( "%d.%d.%d.%d" ), charint( pVendor[ 2 ] ), charint( pVendor[ 3 ] ), charint( pVendor[ 4 ] ), charint( pVendor[ 5 ] ) );
						break;
					case STYLE_3_DIGITS:
						sUserAgent.AppendFormat( _T( "%d.%d.%d" ), charint( pVendor[ 2 ] ), charint( pVendor[ 3 ] ), charint( pVendor[ 4 ] ) );
						break;
					case STYLE_UTORRENT:
						sUserAgent.AppendFormat( _T( "%d.%d.%d%s" ), strint( &pVendor[ 2 ], 1 ), strint( &pVendor[ 3 ], 1 ), strint( &pVendor[ 4 ], 1 ), getMnemonicEnd( pVendor[ 5 ] ) );
						break;
					case STYLE_2_DIGITS:
						sUserAgent.AppendFormat( _T( "%d.%02d" ), charint( pVendor[ 2 ] ), charint( pVendor[ 3 ] ) );
						break;
					case STYLE_2_INTS:
						sUserAgent.AppendFormat( _T( "%d.%02d" ), strint( &pVendor[ 2 ], 2 ), strint( &pVendor[ 4 ], 2 ) );
						break;
					case STYLE_BUDDY:
						sUserAgent.AppendFormat( _T( "%c.%c%c%c" ), (TCHAR)pVendor[ 2 ], (TCHAR)pVendor[ 3 ], (TCHAR)pVendor[ 4 ], (TCHAR)pVendor[ 5 ] );
						break;
					case STYLE_ROCKET:
						sUserAgent.AppendFormat( _T( "%c.%c (%c%c)" ), (TCHAR)pVendor[ 2 ], (TCHAR)pVendor[ 3 ], (TCHAR)pVendor[ 4 ], (TCHAR)pVendor[ 5 ] );
						break;
					case STYLE_CTORRENT:
						sUserAgent.AppendFormat( _T( "%d.%d.%02d" ), charint( pVendor[ 2 ] ), charint( pVendor[ 3 ] ), strint( &pVendor[ 4 ], 2 ) );
						break;
					case STYLE_XTORRENT:
						sUserAgent.AppendFormat( _T( "%d.%d (%d)" ), charint( pVendor[ 2 ] ), charint( pVendor[ 3 ] ), strint( &pVendor[ 4 ], 2 ) );
						break;
					case STYLE_NO_VERS:
						break;
					}
				}
				break;
			}
		}
		if ( sUserAgent.IsEmpty() )
			// If we don't want the version, etc.
			sUserAgent.Format( _T("BitTorrent (%c%c)"), (TCHAR)pVendor[ 1 ], (TCHAR)pVendor[ 2 ] );
	}
	return sUserAgent;
}

void CBTClient::DetermineUserAgent( const Hashes::BtGuid& oGUID )
{
	ASSUME_LOCK( Transfers.m_pSection );

	int nNickStart = 0, nNickEnd = 13;
	CString sNick, sUserAgent;

	BOOL bClientExtended = isExtendedBtGuid( oGUID );

	if ( oGUID[ 0 ] == '-' && ( oGUID[ 6 ] == '-' || oGUID[ 7 ] == '-' ) )
	{	// Azureus-style
		sUserAgent = GetAzureusStyleUserAgent( &(oGUID[ 1 ]), 6 );
	}
	else if ( oGUID[4] == '-' && oGUID[5] == '-' && oGUID[6] == '-' && oGUID[7] == '-' )
	{	// Shadow style
		switch ( oGUID[0] )
		{
		case 'A':
			sUserAgent = L"ABC";
			break;
		case 'O':
			sUserAgent = L"Osprey";
			break;
		case 'R':
			sUserAgent = L"Tribler";
			break;
		case 'S':
			sUserAgent = L"Shadow";
			break;
		case 'T':
			sUserAgent = L"BitTornado";
			break;
		case 'U':
			sUserAgent = L"UPnP NAT BT";
			break;
		default: // Unknown client using this naming.
			sUserAgent.Format(_T("%c"), oGUID[0]);
		}

		sUserAgent.AppendFormat( _T(" %i.%i.%i"), charint( oGUID[1] ), charint( oGUID[2] ), charint( oGUID[3] ) );
	}
	else if  ( oGUID[0] == 'M' && oGUID[2] == '-' && oGUID[4] == '-' && oGUID[6] == '-' )
	{	// BitTorrent (Standard client, newer version)
		sUserAgent.Format( _T("BitTorrent %i.%i.%i"), charint( oGUID[1] ) , charint( oGUID[3] ), charint( oGUID[5] ) );
	}
	else if  ( oGUID[0] == 'P' && oGUID[1] == 'l' && oGUID[2] == 'u' && oGUID[3] == 's' )
	{	// BitTorrent Plus
		sUserAgent.Format( _T("BitTorrent Plus %i.%i%i%c"), charint( oGUID[4] ), charint( oGUID[5] ), charint( oGUID[6] ), oGUID[7] );
	}
	else if  ( oGUID[0] == 'e' && oGUID[1] == 'x' && oGUID[2] == 'b' && oGUID[3] == 'c' )
	{
		// BitLord
		if  ( oGUID[6] == 'L' && oGUID[7] == 'O' && oGUID[8] == 'R' && oGUID[9] == 'D' )
			sUserAgent.Format( _T("BitLord %u.%02u"), oGUID[4], oGUID[5] );
		// Old BitComet
		else
			sUserAgent.Format( _T("BitComet %u.%02u"), oGUID[4], oGUID[5] );
	}
	else if  ( ( oGUID[0] == 'B' && oGUID[1] == 'S' ) || ( oGUID[2] == 'B' && oGUID[3] == 'S' ) )
	{	// BitSpirit
		sUserAgent.Format( _T("BitSpirit") );
	}
	else if  ( oGUID[0] == 'B' && oGUID[1] == 'T' && oGUID[2] == 'M' )
	{	// BTuga Revolution
		sUserAgent.Format( _T("BTuga Rv %i.%i"), charint( oGUID[3] ), charint( oGUID[4] ) );
		nNickStart = 5;
	}
	else if  ( ( oGUID[0] == 'b' && oGUID[1] == 't' && oGUID[2] == 'u' && oGUID[3] == 'g' && oGUID[4] == 'a' ) ||
		       ( oGUID[0] == 'o' && oGUID[1] == 'e' && oGUID[2] == 'r' && oGUID[3] == 'n' && oGUID[4] == 'u' ) )
	{	// BTugaXP
		sUserAgent.Format( _T("BTugaXP") );
	}
	else if  ( oGUID[0] == 'M' && oGUID[1] == 'b' && oGUID[2] == 'r' && oGUID[3] == 's' && oGUID[4] == 't' )
	{	// Burst
		sUserAgent.Format( _T("Burst %i.%i.%i"), charint( oGUID[5] ), charint( oGUID[7] ), charint( oGUID[9] ) );
	}
	else if  ( oGUID[0] == 'e' && oGUID[1] == 'X' )
	{	// eXeem
		sUserAgent.Format( _T("eXeem") );
		nNickStart = 2;
	}
	else if ( oGUID[0] == '-' && oGUID[1] == 'F' && oGUID[2] == 'G' )
	{
		sUserAgent.Format( _T("FlashGet %i.%i%i"), ( charint( oGUID[3] ) * 10 + charint( oGUID[4] ) ), charint( oGUID[5] ), charint( oGUID[6] ) );
	}
	else if  ( oGUID[0] == '-' && oGUID[1] == 'G' && oGUID[2] == '3' )
	{	// G3 Torrent
		sUserAgent.Format( _T("G3 Torrent") );
		nNickStart = 3;
		nNickEnd = 11;
	}
	else if ( oGUID[0] == '1' && oGUID[1] == '0' && oGUID[2] == '-' && oGUID[3] == '-' && oGUID[4] == '-' && oGUID[5] == '-' && oGUID[6] == '-' && oGUID[7] == '-' && oGUID[8] == '-' )
	{
		sUserAgent = _T("JVTorrent");
	}
	else if ( oGUID[0] == 'L' && oGUID[1] == 'I' && oGUID[2] == 'M' && oGUID[3] == 'E' )
	{
		sUserAgent = _T("Limewire");
	}
	else if  ( oGUID[2] == 'R' && oGUID[3] == 'S' )
	{	// Rufus
		// The first two bits are version numbers. e.g. 0.6.5 = (0)(6+5) = char(0)char(11)
		// The same with BitMagnet
		sUserAgent.Format( _T("Rufus") );
		nNickStart = 4;
		nNickEnd = 12;
	}
	else if  ( oGUID[2] == 'B' && oGUID[3] == 'M' )
	{	// BitMagnet, the former Rufus
		sUserAgent.Format( _T("BitMagnet") );
		nNickStart = 4;
		nNickEnd = 12;
	}
	else if  ( oGUID[0] == '-' && oGUID[1] == 'M' && oGUID[2] == 'L' )
	{	// MLdonkey
		sUserAgent.Format( _T("MLdonkey %i.%i.%i"), charint( oGUID[3] ) , charint( oGUID[5] ) , charint( oGUID[7] ) );
	}
	else if  ( oGUID[0] == 'O' && oGUID[1] == 'P' )
	{	// Opera
		sUserAgent.Format( _T("Opera %i%i%i%i"), oGUID[2] - '0', oGUID[3] - '0', oGUID[4] - '0', oGUID[5] - '0' );
	}
	else if  ( ( oGUID[0] == 'a' && oGUID[1] == '0' && oGUID[2] == '0' && oGUID[3] == '-' && oGUID[4] == '-' && oGUID[5] == '-' && oGUID[6] == '0' ) || ( oGUID[0] == 'a' && oGUID[1] == '0' && oGUID[2] == '2' && oGUID[3] == '-' && oGUID[4] == '-' && oGUID[5] == '-' && oGUID[6] == '0' ) )
	{	// Swarmy
		sUserAgent.Format( _T("Swarmy") );
	}
	else if ( oGUID[0] == '3' && oGUID[1] == '4' && oGUID[2] == '6' && oGUID[3] == '-' )
	{
		sUserAgent = _T("TorrentTopia");
	}
	else if  ( oGUID[0] == 'X' && oGUID[1] == 'B' && oGUID[2] == 'T' )
	{	// XBT
		sUserAgent.Format( _T("XBT %i.%i.%i"), charint( oGUID[3] ), charint( oGUID[4] ), charint( oGUID[5] ) );
	}
	else if  ( !oGUID[0] && !oGUID[1] && !oGUID[2] && !oGUID[3] && !oGUID[4] && !oGUID[5] && !oGUID[6] && !oGUID[7] && oGUID[8] && oGUID[9] && oGUID[10] && oGUID[11] && oGUID[12] && oGUID[13] && oGUID[14] && oGUID[15] && oGUID[16] == 'U' && oGUID[17] == 'D' && oGUID[18] == 'P' && oGUID[19] == '0' )
	{	// BitSpirit	(Spoofed Client ID)	// GUID 0 - 7: 0	GUID 8 - 15: !0	GUID 16 -19: UDP0	// ToDO: Check that other clients don't use this method
		sUserAgent.Format( _T("BitSpirit") );
	}

	if ( ! bClientExtended )
	{
		bClientExtended = VendorCache.IsExtended( sUserAgent );
	}

	if ( sUserAgent.IsEmpty() )
	{	// Unknown peer ID string
		sUserAgent = bClientExtended ? _T("Shareaza") : protocolNames[ PROTOCOL_BT ];
		theApp.Message( MSG_DEBUG, _T("[BT] Unknown client: %.20hs"), (LPCSTR)&oGUID[0] );
	}

	if ( nNickStart > 0 )
		for ( int i = nNickStart; i <= nNickEnd; i++ )	// Extract nick from oGUID
		{
			if ( oGUID[i] == NULL ) break;

			sNick.AppendFormat( _T("%c"), oGUID[i] );
		}

	SetUserAgent( sUserAgent, bClientExtended, sNick );
}

void CBTClient::SetUserAgent(const CString& sUserAgent, BOOL bClientExtended, const CString& sNick)
{
	m_sUserAgent = sUserAgent;
	m_sUserAgent.Replace( _T('\x03bc'), _T('\x00b5') ); // "mu" (Alt + 0181)

	if ( bClientExtended )
		m_bClientExtended = TRUE;

	if ( m_pDownloadTransfer )
	{
		m_pDownloadTransfer->m_sUserAgent = m_sUserAgent;
		m_pDownloadTransfer->m_bClientExtended = m_bClientExtended;
		if ( CDownloadSource* pSource = GetSource() )
		{
			pSource->m_sServer = m_sUserAgent;
			if ( sNick.GetLength() ) pSource->m_sNick = sNick;
			pSource->m_bClientExtended = ( m_bClientExtended && ! pSource->m_bPushOnly );
		}
	}

	if ( m_pUploadTransfer )
	{
		m_pUploadTransfer->m_sUserAgent = m_sUserAgent;
		if ( sNick.GetLength() ) m_pUploadTransfer->m_sRemoteNick = sNick;
		m_pUploadTransfer->m_bClientExtended = m_bClientExtended;
	}
}

CDownloadSource* CBTClient::GetSource() const
{
	return m_pDownloadTransfer ? m_pDownloadTransfer->GetSource() : NULL;
}

//////////////////////////////////////////////////////////////////////
// CBTClient packet switch

BOOL CBTClient::OnPacket(CBTPacket* pPacket)
{
	Statistics.Current.BitTorrent.Incoming++;

	pPacket->SmartDump( &m_pHost, FALSE, FALSE );

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
		if ( ! m_pUploadTransfer->OnInterested( pPacket ) ) return FALSE;
		m_pDownload->ChokeTorrent();
		break;

	case BT_PACKET_NOT_INTERESTED:
		if ( ! m_pUploadTransfer->OnUninterested( pPacket ) ) return FALSE;
		m_pDownload->ChokeTorrent();
		break;

	case BT_PACKET_HAVE:
		return m_pDownloadTransfer == NULL || m_pDownloadTransfer->OnHave( pPacket );

	case BT_PACKET_BITFIELD:
		return m_pDownloadTransfer == NULL || m_pDownloadTransfer->OnBitfield( pPacket );

	case BT_PACKET_REQUEST:
		return m_pUploadTransfer->OnRequest( pPacket );

	case BT_PACKET_PIECE:
		return m_pDownloadTransfer == NULL || m_pDownloadTransfer->OnPiece( pPacket );

	case BT_PACKET_CANCEL:
		return m_pUploadTransfer->OnCancel( pPacket );

	case BT_PACKET_DHT_PORT:
		return OnDHTPort( pPacket );

	case BT_PACKET_EXTENSION:
		switch ( pPacket->m_nExtension )
		{
		case BT_EXTENSION_HANDSHAKE:
			return OnExtendedHandshake( pPacket );

		case BT_EXTENSION_UT_METADATA:
			return OnMetadataRequest( pPacket );

		case BT_EXTENSION_UT_PEX:
			return OnUtPex( pPacket );

		case BT_EXTENSION_LT_TEX:
			return OnLtTex( pPacket );
		}
		break;

	case BT_PACKET_HANDSHAKE:
		return OnBeHandshake( pPacket );

	case BT_PACKET_SOURCE_REQUEST:
		return OnSourceRequest( pPacket );

	case BT_PACKET_SOURCE_RESPONSE:
		return m_pDownloadTransfer == NULL || m_pDownloadTransfer->OnSourceResponse( pPacket );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient advanced handshake

void CBTClient::SendBeHandshake()
{
	// Send extended handshake (for G2 capable clients)
	CBTPacket* pPacket = CBTPacket::New( BT_PACKET_HANDSHAKE );
	CBENode* pRoot = pPacket->m_pNode.get();
	ASSERT( pRoot );

	CString strNick = MyProfile.GetNick().Left( 255 ); // Truncate to 255 characters
	if ( strNick.GetLength() )
		pRoot->Add( BT_DICT_NICKNAME )->SetString( strNick );

	pRoot->Add( BT_DICT_SRC_EXCHANGE )->SetInt( BT_HANDSHAKE_SOURCE );
	pRoot->Add( BT_DICT_USER_AGENT )->SetString( Settings.SmartAgent() );

	Send( pPacket );
}

BOOL CBTClient::OnBeHandshake(CBTPacket* pPacket)
{
	ASSUME_LOCK( Transfers.m_pSection );

	const CBENode* pRoot = pPacket->m_pNode.get();
	ASSERT( pRoot );

	if ( const CBENode* pAgent = pRoot->GetNode( BT_DICT_USER_AGENT ) )
	{
		if ( pAgent->IsType( CBENode::beString ) )
		{
			CString sUserAgent = pAgent->GetString();
			sUserAgent.Trim();

			SetUserAgent( sUserAgent, TRUE );

			if ( Security.IsClientBanned( m_sUserAgent ) )
			{
				Close( IDS_SECURITY_BANNED_USERAGENT );
				return FALSE;
			}
		}
	}

	if ( const CBENode* pNick = pRoot->GetNode( BT_DICT_NICKNAME ) )
	{
		if ( pNick->IsType( CBENode::beString ) )
		{
			if ( CDownloadSource* pSource = GetSource() )
			{
				pSource->m_sNick = pNick->GetString();
			}
		}
	}

	if ( const CBENode* pExchange = pRoot->GetNode( BT_DICT_SRC_EXCHANGE ) )
	{
		m_nSrcExchangeID = (BYTE)pExchange->GetInt();
	}

	theApp.Message( MSG_INFO, IDS_BT_CLIENT_EXTENDED, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient source request

void CBTClient::SendSourceRequest()
{
	if ( m_nSrcExchangeID && m_pDownloadTransfer )
	{
		Send( CBTPacket::New( BT_PACKET_SOURCE_REQUEST ) );
	}
}

BOOL CBTClient::OnSourceRequest(CBTPacket* /*pPacket*/)
{
	if ( ! m_pDownload )
		return TRUE;

	CBTPacket* pResponse = CBTPacket::New( BT_PACKET_SOURCE_RESPONSE );
	CBENode* pRoot = pResponse->m_pNode.get();
	ASSERT( pRoot );

	CBENode* pPeers = pRoot->Add( BT_DICT_PEERS );
	for ( POSITION posSource = m_pDownload->GetIterator(); posSource ; )
	{
		CDownloadSource* pSource = m_pDownload->GetNext( posSource );

		if ( ! pSource->IsConnected() )
			continue;

		if ( pSource->m_nProtocol == PROTOCOL_BT )
		{
			CBENode* pPeer = pPeers->Add();
			CShareazaURL oURL;

			if ( oURL.Parse( pSource->m_sURL ) && oURL.m_oBTC )
				pPeer->Add( BT_DICT_PEER_ID )->SetString( &*oURL.m_oBTC.begin(), oURL.m_oBTC.byteCount );

			pPeer->Add( BT_DICT_PEER_IP )->SetString( CString( inet_ntoa( pSource->m_pAddress ) ) );
			pPeer->Add( BT_DICT_PEER_PORT )->SetInt( pSource->m_nPort );
		}
		else if (	pSource->m_nProtocol == PROTOCOL_HTTP &&
					pSource->m_bReadContent == TRUE &&
					pSource->m_bPushOnly == FALSE )
		{
			CBENode* pPeer = pPeers->Add();
			pPeer->Add( BT_DICT_PEER_URL )->SetString( pSource->m_sURL );
		}
	}

	if ( pPeers->GetCount() == 0 )
	{
		pResponse->Release();
		return TRUE;
	}

	Send( pResponse );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient DHT

void CBTClient::SendDHTPort()
{
	if ( CBTPacket* pResponse = CBTPacket::New( BT_PACKET_DHT_PORT ) )
	{
		pResponse->WriteShortBE( Network.GetPort() );

		Send( pResponse );
	}
}

BOOL CBTClient::OnDHTPort(CBTPacket* pPacket)
{
	if ( pPacket && pPacket->GetRemaining() == 2 )
	{
		// Test this node via UDP
		DHT.Ping( &m_pHost.sin_addr, pPacket->ReadShortBE() );
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTClient Extension

void CBTClient::SendExtendedHandshake()
{
	if ( CBTPacket* pResponse = CBTPacket::New( BT_PACKET_EXTENSION, BT_EXTENSION_HANDSHAKE ) )
	{
		if ( CBENode* pRoot = pResponse->m_pNode.get() )
		{
			if ( CBENode* pM = pRoot->Add( BT_DICT_EXT_MSG ) )
			{
				pM->Add( BT_DICT_UT_PEX )->SetInt( BT_EXTENSION_UT_PEX );

				if (  m_pDownload->IsTorrent() &&
					! m_pDownload->m_pTorrent.m_bPrivate )
				{
					pM->Add( BT_DICT_UT_METADATA )->SetInt( BT_EXTENSION_UT_METADATA );
					if ( m_pDownload->m_pTorrent.GetInfoSize() )
					{
						pRoot->Add( BT_DICT_METADATA_SIZE )->SetInt( m_pDownload->m_pTorrent.GetInfoSize() );
					}
				}

				if (  m_pDownload->IsTorrent() &&
					! m_pDownload->m_pTorrent.m_bPrivate )
				{
					pM->Add( BT_DICT_LT_TEX )->SetInt( BT_EXTENSION_LT_TEX );
					pRoot->Add( BT_DICT_TRACKERS )->SetString( m_pDownload->m_pTorrent.GetTrackerHash() );
				}

				pRoot->Add( BT_DICT_PORT )->SetInt( Network.GetPort() );
				pRoot->Add( BT_DICT_VENDOR )->SetString( Settings.SmartAgent() );

				Send( pResponse, FALSE );
			}
			// else Out of Memory
		}
		// else Out of Memory

		pResponse->Release();
	}
	// else Out of Memory
}

BOOL CBTClient::OnExtendedHandshake(CBTPacket* pPacket)
{
	const CBENode* pRoot = pPacket->m_pNode.get();

	const CBENode* pVendor = pRoot->GetNode( BT_DICT_VENDOR );
	if ( pVendor && pVendor->IsType( CBENode::beString ) )
	{
		CString sUserAgent = pVendor->GetString();
		sUserAgent.Trim();

		// Is this a good vendor string?
		DWORD nDots = 0, nNumbers = 0, nAlphas = 0;
		for ( LPCTSTR ch = sUserAgent; *ch; ++ch )
		{
			if ( ( *ch >= _T( 'a' ) && *ch <= _T( 'z' ) ) || ( *ch >= _T( 'A' ) && *ch <= _T( 'Z' ) ) )
				++nAlphas;
			else if ( *ch >= _T( '0' ) && *ch <= _T( '9' ) )
				++nNumbers;
			else if ( *ch == _T( '.' ) )
				++nDots;
		}
		if ( nDots > 0 && nNumbers > 0 && nAlphas > 3 )
		{
			SetUserAgent( sUserAgent );

			if ( Security.IsClientBanned( m_sUserAgent ) )
			{
				Close( IDS_SECURITY_BANNED_USERAGENT );
				return FALSE;
			}
		}
	}

	const CBENode* pYourIP = pRoot->GetNode( BT_DICT_YOURIP );
	if ( pYourIP && pYourIP->IsType( CBENode::beString ) )
	{
		if ( pYourIP->m_nValue == 4 )
		{
			// IPv4
			Network.AcquireLocalAddress( *(const IN_ADDR*)pYourIP->m_pValue );
		}
	}

	if ( const CBENode* pMetadata = pRoot->GetNode( BT_DICT_EXT_MSG ) )
	{
		if ( const CBENode* pUtMetadata = pMetadata->GetNode( BT_DICT_UT_METADATA ) )
		{
			m_nUtMetadataID = (BYTE)pUtMetadata->GetInt();
			if ( m_nUtMetadataID && ! m_pDownload->m_pTorrent.m_pBlockBTH ) // Send first info request
			{
				int nNextPiece = m_pDownload->m_pTorrent.NextInfoPiece();
				if ( nNextPiece >= 0 )
					SendInfoRequest( nNextPiece );
			}
		}
		if ( const CBENode* pUtMetadataSize = pRoot->GetNode( BT_DICT_METADATA_SIZE ) )
		{
			m_nUtMetadataSize = (QWORD)pUtMetadataSize->GetInt();
		}

		if ( const CBENode* pUtPex = pMetadata->GetNode( BT_DICT_UT_PEX ) )
		{
			BYTE nOldUtPexID = m_nUtPexID;
			m_nUtPexID = (BYTE)pUtPex->GetInt();
			if ( ! nOldUtPexID && m_nUtPexID )
			{
				SendUtPex();
				m_tLastUtPex = GetTickCount();
			}
		}

		if ( const CBENode* pLtTex = pMetadata->GetNode( BT_DICT_LT_TEX ) )
		{
			m_nLtTexID = (BYTE)pLtTex->GetInt();
		}
		if ( const CBENode* pLtTexTrackers = pRoot->GetNode( BT_DICT_TRACKERS ) )
		{
			CString sRemoteHash = pLtTexTrackers->GetString();
			CString sLocalHash = m_pDownload->m_pTorrent.GetTrackerHash();
			if ( sRemoteHash != sLocalHash && m_nLtTexID )
			{
				SendLtTex();
			}
		}
	}

	return TRUE;
}

void CBTClient::SendMetadataRequest(QWORD nPiece)
{
	ASSERT( m_nUtMetadataID );

	CBTPacket* pResponse = CBTPacket::New( BT_PACKET_EXTENSION, m_nUtMetadataID );
	CBENode* pRoot = pResponse->m_pNode.get();
	ASSERT( pRoot );

	pRoot->Add( BT_DICT_PIECE )->SetInt( nPiece );

	CBuffer pOutput;

	BYTE* pInfoPiece = NULL;
	DWORD InfoLen = m_pDownload->m_pTorrent.GetInfoPiece( (DWORD)nPiece, &pInfoPiece );
	if ( InfoLen == 0 || m_pDownload->m_pTorrent.m_bPrivate )
	{
		pRoot->Add( BT_DICT_MSG_TYPE )->SetInt( UT_METADATA_REJECT );
	}
	else
	{
		pRoot->Add( BT_DICT_MSG_TYPE )->SetInt( UT_METADATA_DATA );
		pRoot->Add( BT_DICT_TOTAL_SIZE )->SetInt( m_pDownload->m_pTorrent.GetInfoSize() );

		pResponse->Write( pInfoPiece, InfoLen );
	}

	Send( pResponse );
}

void CBTClient::SendInfoRequest(QWORD nPiece)
{
	BYTE nUtMetadataID = m_nUtMetadataID ? m_nUtMetadataID : BT_EXTENSION_UT_METADATA;

	CBTPacket* pResponse = CBTPacket::New( BT_PACKET_EXTENSION, nUtMetadataID );
	CBENode* pRoot = pResponse->m_pNode.get();

	pRoot->Add( BT_DICT_MSG_TYPE )->SetInt( UT_METADATA_REQUEST ); //Request
	pRoot->Add( BT_DICT_PIECE )->SetInt( nPiece );

	Send( pResponse );
}

BOOL CBTClient::OnMetadataRequest(CBTPacket* pPacket)
{
	ASSUME_LOCK( Transfers.m_pSection );

	const CBENode* pRoot = pPacket->m_pNode.get();

	if ( CBENode* pMsgType = pRoot->GetNode( BT_DICT_MSG_TYPE ) )
	{
		if ( CBENode* pPiece = pRoot->GetNode( BT_DICT_PIECE ) )
		{
			QWORD nMsgType	= (QWORD)pMsgType->GetInt();
			QWORD nPiece	= (QWORD)pPiece->GetInt();

			if ( nMsgType == UT_METADATA_REQUEST )
			{
				if ( m_nUtMetadataID )
					SendMetadataRequest( nPiece );
			}
			else if ( nMsgType == UT_METADATA_DATA )
			{
				if ( CBENode* pTotalSize = pRoot->GetNode( BT_DICT_TOTAL_SIZE ) )
				{
					QWORD nTotalSize = (QWORD)pTotalSize->GetInt();
					ASSERT( ! ( m_nUtMetadataSize && m_nUtMetadataSize != nTotalSize ) );
					if ( ! m_nUtMetadataSize )
						m_nUtMetadataSize = nTotalSize;
				}

				if ( m_nUtMetadataSize && ! m_pDownload->m_pTorrent.m_pBlockBTH )
				{
					if ( m_pDownload->m_pTorrent.LoadInfoPiece( pPacket->m_pBuffer, (DWORD)pPacket->m_nLength, (DWORD)m_nUtMetadataSize, (DWORD)nPiece ) ) // If full info loaded
					{
						 m_pDownload->SetTorrent();
					}
					else
					{
						int nNextPiece = m_pDownload->m_pTorrent.NextInfoPiece();
						if ( nNextPiece >= 0 && m_nUtMetadataID )
							SendInfoRequest( nNextPiece );
					}
				}
			}
		}
	}

	return TRUE;
}

/*
 The PEX message payload is a bencoded dictionary with three keys:
 'added': the set of peers met since the last PEX
 'added.f': a flag for every peer, apparently with the following values:
    \x00: unknown, assuming default
    \x01: Prefers encryption
    \x02: Is seeder
  OR-ing them together is allowed
 'dropped': the set of peers dropped since last PEX
*/

void CBTClient::SendUtPex(DWORD tConnectedAfter)
{
	if ( m_nUtPexID == 0 )
		// Unsupported
		return;

	CBuffer pAddedBuffer;
	CBuffer pAddedFalgsBuffer;
	BYTE* pnFlagsByte = NULL;
	DWORD nPeersCount = 0;
	for ( POSITION posSource = m_pDownload->GetIterator(); posSource ; )
	{
		CDownloadSource* pSource = m_pDownload->GetNext( posSource );

		if ( ! pSource->IsConnected() ||
			   pSource->GetTransfer()->m_tConnected < tConnectedAfter ||
			   pSource->m_nProtocol != PROTOCOL_BT )
			continue;

		WORD nPort = htons( pSource->m_nPort );
		pAddedBuffer.Add( &pSource->m_pAddress, 4 );
		pAddedBuffer.Add( &nPort, 2 );

		nPeersCount += 1;

		const CDownloadTransferBT* pBTDownload =
			static_cast< const CDownloadTransferBT* >( pSource->GetTransfer() );

		BYTE nFlag = (pBTDownload->m_pClient->m_bPrefersEncryption ? 1 : 0 ) |
					 (pBTDownload->m_pClient->m_bSeeder ? 2 : 0);

		DWORD nFalgInBytePos = (nPeersCount - 1 ) % 4;

		if ( nFalgInBytePos == 0 )
		{
			pAddedFalgsBuffer.EnsureBuffer( 1 );
			pnFlagsByte = pAddedFalgsBuffer.GetDataEnd();
			pAddedFalgsBuffer.m_nLength += 1;
			*pnFlagsByte = 0;
		}

		*pnFlagsByte |= ( nFlag & 3 ) << ( 6 - nFalgInBytePos * 2 );
	}

	if ( pAddedBuffer.m_nLength )
	{
		CBTPacket* pResponse = CBTPacket::New( BT_PACKET_EXTENSION, m_nUtPexID );
		CBENode* pRoot = pResponse->m_pNode.get();

		pRoot->Add( BT_DICT_ADDED )->SetString( pAddedBuffer.GetData(), pAddedBuffer.GetCount() );
		pRoot->Add( BT_DICT_ADDED_F )->SetString( pAddedFalgsBuffer.GetData(), pAddedFalgsBuffer.GetCount() );

		Send( pResponse );
	}
}

BOOL CBTClient::OnUtPex(CBTPacket* pPacket)
{
	const CBENode* pRoot = pPacket->m_pNode.get();

	if ( CBENode* pPeersAdd = pRoot->GetNode( BT_DICT_ADDED ) )
	{
		if ( 0 == ( pPeersAdd->m_nValue % 6 ) ) // IPv4?
		{
			const BYTE* pPointer = (const BYTE*)pPeersAdd->m_pValue;

			for ( int nPeer = (int)pPeersAdd->m_nValue / 6 ; nPeer > 0; nPeer --, pPointer += 6 )
			{
				const IN_ADDR* pAddress = (const IN_ADDR*)pPointer;
				WORD nPort = *(const WORD*)( pPointer + 4 );
				m_pDownload->AddSourceBT( Hashes::BtGuid(), pAddress, ntohs( nPort ) );
			}
		}
	}

	if ( CBENode* pPeersDrop = pRoot->GetNode( BT_DICT_DROPPED ) )
	{
		if ( 0 == ( pPeersDrop->m_nValue % 6 ) ) // IPv4?
		{
			for ( POSITION posSource = m_pDownload->GetIterator(); posSource ; )
			{
				CDownloadSource* pSource = m_pDownload->GetNext( posSource );

				if ( pSource->IsConnected() || pSource->m_nProtocol != PROTOCOL_BT )
					continue;

				WORD nPort = htons( pSource->m_nPort );
				const BYTE* pPointer = (const BYTE*)pPeersDrop->m_pValue;

				for ( int nPeer = (int)pPeersDrop->m_nValue / 6 ; nPeer > 0 ; nPeer --, pPointer += 6 )
				{
					if ( memcmp( &pSource->m_pAddress, pPointer, 4 ) == 0
						 && memcmp( &nPort, (pPointer + 4), 2 ) == 0 )
						pSource->m_tAttempt = pSource->CalcFailureDelay();
				}
			}
		}
	}

	return TRUE;
}

void CBTClient::SendLtTex()
{
	if ( m_nLtTexID == 0 )
		// Unsupported
		return;

	if ( m_pDownload->m_pTorrent.m_bPrivate )
		// Don't send private tracker URLs
		return;

	int nCount = m_pDownload->m_pTorrent.GetTrackerCount();
	if ( nCount == 0 )
		return;

	CBTPacket* pResponse = CBTPacket::New( BT_PACKET_EXTENSION, m_nLtTexID );
	CBENode* pRoot = pResponse->m_pNode.get();

	CBENode* pList = pRoot->Add( BT_DICT_ADDED )->Add();
	for ( int i = 0; i < nCount; i++ )
	{
		pList->Add()->SetString(
			m_pDownload->m_pTorrent.GetTrackerAddress( i ) );
	}

	Send( pResponse );
}

BOOL CBTClient::OnLtTex(CBTPacket* pPacket)
{
	const CBENode* pRoot = pPacket->m_pNode.get();

	if ( CBENode* pTrackerList = pRoot->GetNode( BT_DICT_ADDED ) )
	{
		int nCount = pTrackerList->GetCount();
		for ( int i = 0; i < nCount; ++i )
		{
			if ( CBENode* pTracker = pTrackerList->GetNode( i ) )
			{
				m_pDownload->m_pTorrent.AddTracker(
					CBTInfo::CBTTracker( pTracker->GetString() ) );
			}
		}
	}

	return TRUE;
}

void CBTClient::Choke()
{
	Send( CBTPacket::New( BT_PACKET_CHOKE ) );
}

void CBTClient::UnChoke()
{
	Send( CBTPacket::New( BT_PACKET_UNCHOKE ) );
}

void CBTClient::Interested()
{
	Send( CBTPacket::New( BT_PACKET_INTERESTED ) );
}

void CBTClient::NotInterested()
{
	Send( CBTPacket::New( BT_PACKET_NOT_INTERESTED ) );
}

void CBTClient::Request(DWORD nBlock, DWORD nOffset, DWORD nLength)
{
	CBTPacket* pPacket = CBTPacket::New( BT_PACKET_REQUEST );
	pPacket->WriteLongBE( nBlock );
	pPacket->WriteLongBE( nOffset );
	pPacket->WriteLongBE( nLength );
	Send( pPacket );
}

void CBTClient::Cancel(DWORD nBlock, DWORD nOffset, DWORD nLength)
{
	CBTPacket* pPacket = CBTPacket::New( BT_PACKET_CANCEL );
	pPacket->WriteLongBE( nBlock );
	pPacket->WriteLongBE( nOffset );
	pPacket->WriteLongBE( nLength );
	Send( pPacket );
}

void CBTClient::Have(DWORD nBlock)
{
	CBTPacket* pPacket = CBTPacket::New( BT_PACKET_HAVE );
	pPacket->WriteLongBE( nBlock );
	Send( pPacket );
}

void CBTClient::Piece(DWORD nIndex, DWORD nOffset, DWORD nLength, LPCVOID pBuffer)
{
	CBTPacket* pPacket = CBTPacket::New( BT_PACKET_PIECE );
	pPacket->WriteLongBE( nIndex );
	pPacket->WriteLongBE( nOffset );
	pPacket->Write( pBuffer, nLength );
	Send( pPacket );
}
