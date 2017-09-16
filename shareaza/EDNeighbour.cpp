//
// EDNeighbour.cpp
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
#include "EDClients.h"
#include "EDNeighbour.h"
#include "EDPacket.h"
#include "GProfile.h"
#include "HostCache.h"
#include "Library.h"
#include "Neighbours.h"
#include "Network.h"
#include "QueryHit.h"
#include "QuerySearch.h"
#include "Schema.h"
#include "Security.h"
#include "SharedFile.h"
#include "Statistics.h"
#include "Transfers.h"
#include "UploadQueues.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CEDNeighbour construction

CEDNeighbour::CEDNeighbour()
	: CNeighbour	( PROTOCOL_ED2K )
	, m_nClientID	( 0 )
	, m_nUserCount	( 0 )
	, m_nUserLimit	( 0 )
	, m_nFileLimit	( 1000 )
	, m_nTCPFlags	( 0 )
	, m_nUDPFlags	( 0 )
	, m_nFilesSent	( 0 )
{
	m_nNodeType		= ntHub;
}

CEDNeighbour::~CEDNeighbour()
{
}

DWORD CEDNeighbour::GetID() const
{
	return CEDPacket::IsLowID( m_nClientID ) ? m_nClientID :
		( Network.IsConnected() ? Network.m_pHost.sin_addr.s_addr : 0 );
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour connect to

BOOL CEDNeighbour::ConnectTo(const IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic)
{
	if ( CConnection::ConnectTo( pAddress, nPort ) )
	{
		WSAEventSelect( m_hSocket, Network.GetWakeupEvent(), FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE );

		theApp.Message( MSG_INFO, IDS_ED2K_SERVER_CONNECTING,
			(LPCTSTR)m_sAddress, htons( m_pHost.sin_port ) );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_CONNECTION_CONNECT_FAIL,
			(LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ) );
		return FALSE;
	}

	m_nState		= nrsConnecting;
	m_bAutomatic	= bAutomatic;

	Neighbours.Add( this );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour send packet

BOOL CEDNeighbour::Send(CPacket* pPacket, BOOL bRelease, BOOL /*bBuffered*/)
{
	BOOL bSuccess = FALSE;

	if ( pPacket )
	{
		if ( pPacket->m_nProtocol == PROTOCOL_ED2K )
		{
			pPacket->SmartDump( &m_pHost, FALSE, TRUE, (DWORD_PTR)this );

			m_nOutputCount++;
			Statistics.Current.eDonkey.Outgoing++;

			if ( m_pZOutput )
				pPacket->ToBuffer( m_pZOutput );
			else
				Write( pPacket );

			QueueRun();

			bSuccess = TRUE;

			if ( bRelease ) pPacket->Release();
		}
	}
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour run event

BOOL CEDNeighbour::OnRun()
{
	if ( ! CNeighbour::OnRun() )
		return FALSE;

	DWORD tNow = GetTickCount();

	if ( m_nState == nrsConnected )
	{
		if ( m_nClientID == 0 )
		{
			if ( tNow - m_tConnected > Settings.Connection.TimeoutHandshake )
			{
				Close( IDS_HANDSHAKE_TIMEOUT );
				return FALSE;
			}
		}
		else if ( Settings.eDonkey.ForceHighID && CEDPacket::IsLowID( m_nClientID ) && Network.IsStable() && ! Network.IsFirewalled() )
		{
			// We got a low ID when we should have gotten a high ID.
			// Most likely, the user's router needs to get a few UDP packets before it opens up.
			if ( tNow - Neighbours.m_tLastED2KServerHop > ( Neighbours.m_nLowIDCount + 1 ) * 10 * 60 * 1000 )	// 10 minutes x attempts
			{
				Neighbours.m_tLastED2KServerHop = tNow;
				Neighbours.m_nLowIDCount++;

				// Try another server.
				theApp.Message( MSG_DEBUG, _T("eDonkey server %s fake low ID detected."), (LPCTSTR)m_sAddress );

				Close( IDS_CONNECTION_CLOSED );
				return FALSE;
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour connection event

BOOL CEDNeighbour::OnConnected()
{
	if ( ! CNeighbour::OnConnected() )
		return FALSE;

	theApp.Message( MSG_INFO, IDS_ED2K_SERVER_CONNECTED, (LPCTSTR)m_sAddress );

	m_nState = nrsHandshake1;

	return SendLogin();
}

void CEDNeighbour::OnDropped()
{
	if ( m_nState < nrsConnected )
	{
		HostCache.OnFailure( &m_pHost.sin_addr, htons( m_pHost.sin_port ), PROTOCOL_ED2K, false );
	}

	if ( m_nState == nrsConnecting )
	{
		Close( IDS_CONNECTION_REFUSED );
	}
	else
	{
		Close( IDS_CONNECTION_DROPPED );
	}
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour read event

BOOL CEDNeighbour::OnRead()
{
	CNeighbour::OnRead();

	return ProcessPackets();
}

BOOL CEDNeighbour::ProcessPackets()
{
	CLockedBuffer pInputLocked( GetInput() );

	CBuffer* pInput = m_pZInput ? m_pZInput : (CBuffer*)pInputLocked;

	return ProcessPackets( pInput );
}

BOOL CEDNeighbour::ProcessPackets(CBuffer* pInput)
{
	if ( ! pInput )
		return FALSE;

	BOOL bSuccess = TRUE;

	while ( CEDPacket* pPacket = CEDPacket::ReadBuffer( pInput ) )
	{
		try
		{
			bSuccess = OnPacket( pPacket );
		}
		catch ( CException* pException )
		{
			pException->Delete();
		}

		pPacket->Release();
		if ( ! bSuccess )
			break;
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour packet handler

BOOL CEDNeighbour::OnPacket(CEDPacket* pPacket)
{
	pPacket->SmartDump( &m_pHost, FALSE, FALSE, (DWORD_PTR)this );

	m_nInputCount++;
	Statistics.Current.eDonkey.Incoming++;
	m_tLastPacket = GetTickCount();

	switch ( pPacket->m_nType )
	{
	case ED2K_S2C_REJECTED:
		return OnRejected( pPacket );
	case ED2K_S2C_SERVERMESSAGE:
		return OnServerMessage( pPacket );
	case ED2K_S2C_IDCHANGE:
		return OnIdChange( pPacket );
	case ED2K_S2C_SERVERLIST:
		return OnServerList( pPacket );
	case ED2K_S2C_SERVERSTATUS:
		return OnServerStatus( pPacket );
	case ED2K_S2C_SERVERIDENT:
		return OnServerIdent( pPacket );
	case ED2K_S2C_CALLBACKREQUESTED:
		return OnCallbackRequested( pPacket );
	case ED2K_S2C_SEARCHRESULTS:
		return OnSearchResults( pPacket );
	case ED2K_S2C_FOUNDSOURCES:
		return OnFoundSources( pPacket );
	default:
		DEBUG_ONLY( pPacket->Debug( _T("Unknown packet type from ") + m_sAddress + _T(".") ) );
		break;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour server packets

BOOL CEDNeighbour::OnRejected(CEDPacket* /*pPacket*/)
{
	Close( IDS_ED2K_SERVER_REJECTED );
	return FALSE;
}

BOOL CEDNeighbour::OnServerMessage(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < 4 ) return TRUE;

	CString	strMessage = pPacket->ReadEDString(
		( m_nTCPFlags & ED2K_SERVER_TCP_UNICODE ) != 0 );

	while ( strMessage.GetLength() > 0 )
	{
		CString strLine = strMessage.SpanExcluding( _T("\r\n") );

		if ( strLine.GetLength() > 0 )
		{
			strMessage = strMessage.Mid( strLine.GetLength() );
			theApp.Message( MSG_NOTICE, IDS_ED2K_SERVER_MESSAGE,
				(LPCTSTR)m_sAddress, (LPCTSTR)strLine );
		}

		if ( strMessage.GetLength() > 0 ) strMessage = strMessage.Mid( 1 );
	}

	return TRUE;
}

BOOL CEDNeighbour::OnIdChange(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < 4 ) return TRUE;

	DWORD nClientID = pPacket->ReadLongLE();

	if ( nClientID == 0 )
	{
		Close( IDS_ED2K_SERVER_REFUSED );
		return FALSE;
	}

	if ( pPacket->GetRemaining() >= 4 )
	{
		m_nTCPFlags = pPacket->ReadLongLE();
	}

	if ( m_nClientID == 0 )
	{
		theApp.Message( MSG_INFO, IDS_ED2K_SERVER_ONLINE, (LPCTSTR)m_sAddress, nClientID );

		SendSharedFiles();

		if ( Settings.eDonkey.LearnNewServers )
		{
			Send( CEDPacket::New( ED2K_C2S_GETSERVERLIST ) );
		}
	}
	else
	{
		theApp.Message( MSG_INFO, IDS_ED2K_SERVER_IDCHANGE, (LPCTSTR)m_sAddress, m_nClientID, nClientID );
	}

	theApp.Message( MSG_DEBUG, _T("eDonkey server %s TCP flags: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)GetED2KServerTCPFlags( m_nTCPFlags ) );

	m_nState	= nrsConnected;
	m_nClientID	= nClientID;

	if ( ! CEDPacket::IsLowID( m_nClientID ) )
	{
		Neighbours.m_nLowIDCount = 0;

		IN_ADDR pMyAddress;
		pMyAddress.s_addr = m_nClientID;
		Network.AcquireLocalAddress( pMyAddress );
	}
	else if ( Settings.eDonkey.ForceHighID )
	{
		theApp.Message( MSG_WARNING, _T("eDonkey server %s gave a low-id when we were expecting a high-id."), (LPCTSTR)m_sAddress );
	}

	return TRUE;
}

BOOL CEDNeighbour::OnServerList(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < 1 ) return TRUE;

	DWORD nCount = pPacket->ReadByte();
	if ( pPacket->GetRemaining() < nCount * 6u ) return TRUE;

	while ( nCount-- > 0 )
	{
		DWORD nAddress	= pPacket->ReadLongLE();
		WORD nPort		= pPacket->ReadShortLE();

		theApp.Message( MSG_DEBUG, _T("CEDNeighbour::OnServerList(): %s: %s:%i"),
			(LPCTSTR)m_sAddress,
			(LPCTSTR)CString( inet_ntoa( (IN_ADDR&)nAddress ) ), nPort );

		if ( Settings.eDonkey.LearnNewServers )
		{
			HostCache.eDonkey.Add( (IN_ADDR*)&nAddress, nPort );
		}
	}

	return TRUE;
}

BOOL CEDNeighbour::OnServerStatus(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < 8 ) return TRUE;

	m_nUserCount	= pPacket->ReadLongLE();
	m_nFileCount	= pPacket->ReadLongLE();

	CQuickLock oLock( HostCache.eDonkey.m_pSection );

	if ( CHostCacheHostPtr pHost = HostCache.eDonkey.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) ) )
	{
		pHost->m_nFailures = 0;
		pHost->m_tFailure = 0;
		pHost->m_bCheckedLocally = TRUE;
		pHost->m_nUserCount = m_nUserCount;
		pHost->m_nUserLimit = max( pHost->m_nUserLimit, m_nUserCount );
	}

	return TRUE;
}

BOOL CEDNeighbour::OnServerIdent(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < Hashes::Guid::byteCount + 6 + 4 ) return TRUE;

	pPacket->Read( m_oGUID );

	pPacket->ReadLongLE();	// IP
	pPacket->ReadShortLE();	// Port

	DWORD nTags = pPacket->ReadLongLE();
	CString strDescription;

	while ( nTags-- > 0 && pPacket->GetRemaining() > 1 )
	{
		CEDTag pTag;
		if ( ! pTag.Read( pPacket,
			( m_nTCPFlags & ED2K_SERVER_TCP_UNICODE ) != 0 ) )
			break;

		switch ( pTag.m_nKey )
		{
		case ED2K_ST_SERVERNAME:
			// if ( pTag.m_nType == ED2K_TAG_STRING ) // "Short strings" may be possible...
			m_sServerName = pTag.m_sValue;
			break;
		case ED2K_ST_DESCRIPTION:
			strDescription = pTag.m_sValue;
			break;
		case ED2K_ST_MAXUSERS:
			m_nUserLimit = (DWORD)pTag.m_nValue;
			break;
/*
		case ED2K_ST_MAXFILES:
			nMaxFiles = pTag.m_nValue;
			break;
		case ED2K_ST_UDPFLAGS:
			nUDPFlags = pTag.m_nValue;
			break;
*/

#ifdef _DEBUG
		default:
			CString str;
			str.Format( _T("Unrecognised Server Ident packet opcode 0x%x:0x%x from %s"),
				int( pTag.m_nKey ), int( pTag.m_nType ), LPCTSTR( m_sAddress ) );
			pPacket->Debug( str );
#endif // _DEBUG
		}
	}

	if ( *m_oGUID.begin() == 0x2A2A2A2A )
		m_sUserAgent = _T("eFarm");
	else
		m_sUserAgent = protocolNames[ PROTOCOL_ED2K ];
	m_sUserAgent += _T(" Server");

	CQuickLock oLock( HostCache.eDonkey.m_pSection );

	if ( CHostCacheHostPtr pHost = HostCache.eDonkey.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) ) )
	{
		pHost->m_sName			= m_sServerName;
		pHost->m_sDescription	= strDescription;
		pHost->m_nUserLimit		= m_nUserLimit;
		pHost->m_nTCPFlags		= m_nTCPFlags;

		// We can assume some UDP flags based on TCP flags
		if ( m_nTCPFlags & ED2K_SERVER_TCP_DEFLATE )
		{
			pHost->m_nUDPFlags |= ED2K_SERVER_UDP_GETSOURCES;
			pHost->m_nUDPFlags |= ED2K_SERVER_UDP_GETFILES;
		}
		if ( m_nTCPFlags & ED2K_SERVER_TCP_UNICODE )
			pHost->m_nUDPFlags |= ED2K_SERVER_UDP_UNICODE;
		if ( m_nTCPFlags & ED2K_SERVER_TCP_GETSOURCES2 )
			pHost->m_nUDPFlags |= ED2K_SERVER_UDP_GETSOURCES2;
	}

	theApp.Message( MSG_NOTICE, IDS_ED2K_SERVER_IDENT, (LPCTSTR)m_sAddress, (LPCTSTR)m_sServerName );

	return TRUE;
}

BOOL CEDNeighbour::OnCallbackRequested(CEDPacket* pPacket)
{
	// Check if packet is too small
	if ( pPacket->GetRemaining() < 6 )
	{
		// Ignore packet and return that it was handled
		theApp.Message( MSG_NOTICE, IDS_PROTOCOL_SIZE_PUSH, (LPCTSTR)m_sAddress );
		++Statistics.Current.eDonkey.Dropped;
		++m_nDropCount;
		return TRUE;
	}

	// Get IP address and port
	DWORD nAddress	= pPacket->ReadLongLE();
	WORD nPort		= pPacket->ReadShortLE();

	// Check the security list to make sure the IP address isn't on it
	if ( Security.IsDenied( (IN_ADDR*)&nAddress ) )
	{
		// Failed security check, ignore packet and return that it was handled
		++Statistics.Current.Gnutella1.Dropped;
		++m_nDropCount;
		return TRUE;
	}

	// Check that remote client has a port number, isn't firewalled or using a
	// reserved address
	if ( !nPort
		|| Network.IsFirewalledAddress( (IN_ADDR*)&nAddress )
		|| Network.IsReserved( (IN_ADDR*)&nAddress ) )
	{
		// Can't push open a connection, ignore packet and return that it was
		// handled
		theApp.Message( MSG_NOTICE, IDS_PROTOCOL_ZERO_PUSH, (LPCTSTR)m_sAddress );
		++Statistics.Current.eDonkey.Dropped;
		++m_nDropCount;
		return TRUE;
	}

	// Set up push connection
	EDClients.PushTo( nAddress, nPort );

	// Return that packet was handled
	return TRUE;
}

BOOL CEDNeighbour::OnSearchResults(CEDPacket* pPacket)
{
	if ( m_pQueries.GetCount() == 0 )
	{
		Statistics.Current.eDonkey.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	Hashes::Guid oGUID = m_pQueries.RemoveHead();
	CQueryHit* pHits = CQueryHit::FromEDPacket( pPacket, &m_pHost, m_nTCPFlags & ED2K_SERVER_TCP_UNICODE, oGUID );

	if ( pHits == NULL )
	{
		if ( pPacket->m_nLength != 17 && pPacket->m_nLength != 5 )
		{
			DEBUG_ONLY( pPacket->Debug( _T("BadSearchResult") ) );
			theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT, (LPCTSTR)m_sAddress );
			Statistics.Current.eDonkey.Dropped++;
			m_nDropCount++;
		}

		return TRUE;
	}

	// Process the 'more results available' packet.
	if ( pPacket->m_nType == ED2K_S2C_SEARCHRESULTS && pPacket->GetRemaining() == 1 && m_pQueries.IsEmpty() )
	{
		if ( pPacket->ReadByte() == TRUE )
		{	// This will be remembered by the neighbour, and if the search continues, more results can be requested.
			m_oMoreResultsGUID = oGUID;
			theApp.Message( MSG_DEBUG, _T("Additional results packet received.") );
		}
	}

	Network.OnQueryHits( pHits );

	return TRUE;
}

BOOL CEDNeighbour::OnFoundSources(CEDPacket* pPacket)
{
	CQueryHit* pHits	= CQueryHit::FromEDPacket( pPacket, &m_pHost, m_nTCPFlags & ED2K_SERVER_TCP_UNICODE );

	if ( pHits == NULL )
	{

		if ( pPacket->m_nLength != 17 && pPacket->m_nLength != 5 )
		{
			DEBUG_ONLY( pPacket->Debug( _T("BadSearchResult") ) );
			theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT, (LPCTSTR)m_sAddress );
			Statistics.Current.eDonkey.Dropped++;
			m_nDropCount++;
		}

		return TRUE;
	}

	Network.OnQueryHits( pHits );

	return TRUE;
}

bool CEDNeighbour::IsGoodSize(QWORD nFileSize) const
{
	return ( nFileSize != SIZE_UNKNOWN ) &&
		   ( ( Settings.eDonkey.MinServerFileSize == 0 ) ||
		     ( nFileSize >= (QWORD)Settings.eDonkey.MinServerFileSize * 1024ull * 1024ull ) ) &&
		   ( ( nFileSize <= MAX_SIZE_32BIT ) ||
		     ( Settings.eDonkey.LargeFileSupport && ( m_nTCPFlags & ED2K_SERVER_TCP_64BITSIZE ) ) );
}

// The login message is the first message send by the client to the server after TCP connection establishment.

BOOL CEDNeighbour::SendLogin()
{
	CEDPacket* pPacket = CEDPacket::New( ED2K_C2S_LOGINREQUEST );
	if ( ! pPacket )
		// Out of memory
		return FALSE;

	Hashes::Guid oGUID = MyProfile.oGUID;
	oGUID[ 5 ] = 14;
	oGUID[ 14 ] = 111;
	pPacket->Write( oGUID );

	pPacket->WriteLongLE( m_nClientID );
	pPacket->WriteShortLE( htons( Network.m_pHost.sin_port ) );

	// Number of tags
	pPacket->WriteLongLE( Settings.eDonkey.SendPortServer ? 5 : 4 );

	// Tags sent to the server

	// 1 - User name
	if ( Settings.eDonkey.LearnNewServers )
		CEDTag( ED2K_CT_NAME, MyProfile.GetNick().Left( 255 ) ).Write( pPacket );
	else
		// nolistsrvs in the nick say to the server that we don't want server list
		CEDTag( ED2K_CT_NAME, MyProfile.GetNick().Left( 255 - 13 ) + _T(" - nolistsrvs") ).Write( pPacket );

	// 2 - Version ('ed2k version')
	CEDTag( ED2K_CT_VERSION, ED2K_VERSION ).Write( pPacket );

	// 3 - Flags indicating capability
	CEDTag( ED2K_CT_SERVER_FLAGS, ED2K_SRVCAP_ZLIB | ED2K_SRVCAP_NEWTAGS | ED2K_SRVCAP_UNICODE |
		( Settings.eDonkey.LargeFileSupport ? ED2K_SRVCAP_LARGEFILES : 0 )
		).Write( pPacket );

	// 4 - Software Version ('Client Version').
	CEDTag( ED2K_CT_SOFTWAREVERSION,
		( ( ( ED2K_CLIENT_ID & 0xFF ) << 24 ) |
		( ( theApp.m_nVersion[0] & 0x7F ) << 17 ) |
		( ( theApp.m_nVersion[1] & 0x7F ) << 10 ) |
		( ( theApp.m_nVersion[2] & 0x07 ) << 7  ) |
		( ( theApp.m_nVersion[3] & 0x7F )       ) ) ).Write( pPacket );

	// 5 - Port
	if ( Settings.eDonkey.SendPortServer )
		CEDTag( ED2K_CT_PORT, htons( Network.m_pHost.sin_port ) ).Write( pPacket );

	return Send( pPacket );
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour file advertising

// This function sends the list of shared files to the ed2k server. Note that it's best to
// keep it as brief as possible to reduce the load. This means the metadata should be limited
// to known good values, and only files that are actually available for upload right now should
// be sent.
BOOL CEDNeighbour::SendSharedFiles()
{
	bool bDeflate = ( m_nTCPFlags & ED2K_SERVER_TCP_DEFLATE ) != 0;

	// Set the limits for number of files sent to the ed2k server
	m_nFileLimit = Settings.eDonkey.MaxShareCount;

	{
		CQuickLock oLock( HostCache.eDonkey.m_pSection );

		CHostCacheHost *pServer = HostCache.eDonkey.Find( &m_pHost.sin_addr );
		if ( pServer && ( pServer->m_nFileLimit > 10 ) )
		{
			m_nFileLimit = min( m_nFileLimit, pServer->m_nFileLimit );
		}
	}

	CEDPacket* pPacket = CEDPacket::New( ED2K_C2S_OFFERFILES );
	if ( ! pPacket )
		// Out of memory
		return FALSE;

	m_nFilesSent = 0;

	pPacket->WriteLongLE( m_nFilesSent );		//Write number of files. (update this later)

	// Send files on download list to ed2k server (partials)
	{
		CQuickLock oLock( Transfers.m_pSection );

		for ( POSITION pos = Downloads.GetIterator() ; pos != NULL && ( m_nFilesSent < m_nFileLimit ) ; )
		{
			const CDownload* pDownload = Downloads.GetNext( pos );
			const QWORD nSize = pDownload->m_nSize;

			if ( ( pDownload->m_oED2K ) &&
				 ( IsGoodSize( nSize ) ) &&
				 ( pDownload->IsStarted() ) &&
				 ( ! pDownload->NeedHashset() ) &&
				 ( ! pDownload->IsMoving() ) )
			{
				pPacket->WriteFile( pDownload, nSize, NULL, this, TRUE );

				m_nFilesSent++;
			}
		}
	}

	// Send files in library to ed2k server (Complete files)
	{
		CQuickLock oLock( Library.m_pSection );

		for ( POSITION pos = LibraryMaps.GetFileIterator() ; pos != NULL && ( m_nFilesSent < m_nFileLimit ) ; )
		{
			const CLibraryFile* pFile = LibraryMaps.GetNextFile( pos );
			const QWORD nSize = pFile->GetSize();

			if ( ( pFile->m_oED2K ) &&
				 ( pFile->IsShared() ) &&
				 ( IsGoodSize( nSize ) ) &&
				 ( UploadQueues.CanUpload( PROTOCOL_ED2K, pFile ) ) )
			{
				// Send the file to the ed2k server
				pPacket->WriteFile( pFile, nSize, NULL, this, FALSE );

				m_nFilesSent++;
			}
		}
	}

	*(DWORD*)pPacket->m_pBuffer = m_nFilesSent;	// Correct the number of files sent

	// Compress if the server supports it
	pPacket->m_bDeflate = bDeflate;

	return Send( pPacket );	// Send the packet
}

// This function adds a download to the ed2k server file list.
BOOL CEDNeighbour::SendSharedDownload(const CDownloadWithTiger* pDownload)
{
	bool bDeflate = ( m_nTCPFlags & ED2K_SERVER_TCP_DEFLATE ) != 0;

	// Don't send this file if we aren't properly connected yet, don't have an ed2k hash/hashset,
	// or have already sent too many files.
	if ( m_nState < nrsConnected ) return FALSE;
	if ( ! pDownload->m_oED2K || pDownload->NeedHashset() ) return FALSE;
	if ( ! IsGoodSize( pDownload->m_nSize ) ) return FALSE;
	if ( m_nFilesSent >= m_nFileLimit ) return FALSE;

	CEDPacket* pPacket = CEDPacket::New( ED2K_C2S_OFFERFILES );
	if ( ! pPacket )
		// Out of memory
		return FALSE;

	// Send one file
	pPacket->WriteLongLE( 1 );

	// Send the file
	pPacket->WriteFile( pDownload, pDownload->m_nSize, NULL, this, true );

	// Increment the number of files sent
	m_nFilesSent ++;

	// Compress if the server supports it
	pPacket->m_bDeflate = bDeflate;

	return Send( pPacket );
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour file searching

BOOL CEDNeighbour::SendQuery(const CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal)
{
	// If the caller didn't give us a packet, or one that isn't for our protocol, leave now
	if ( pPacket == NULL || pPacket->m_nProtocol != PROTOCOL_ED2K )
		return FALSE;

	if ( ! bLocal )
		return FALSE;	// Non local searches disabled

	if ( m_nClientID == 0 )
		return FALSE;	// We're not ready

	// Don't add the GUID for GetSources
	if ( ( ! pSearch->m_oED2K ) || ( pSearch->m_bWantDN && Settings.eDonkey.MagnetSearch ) )
		m_pQueries.AddTail( pSearch->m_oGUID );

	return CNeighbour::SendQuery( pSearch, pPacket, bLocal );
}
