//
// EDNeighbour.cpp
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
#include "Network.h"
#include "Statistics.h"
#include "Neighbours.h"
#include "EDNeighbour.h"
#include "EDPacket.h"
#include "Security.h"
#include "GProfile.h"
#include "HostCache.h"
#include "EDClients.h"

#include "Library.h"
#include "SharedFile.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "QuerySearch.h"
#include "QueryHit.h"

#include "UploadQueues.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CEDNeighbour construction

CEDNeighbour::CEDNeighbour() : CNeighbour( PROTOCOL_ED2K )
{
	m_nNodeType		= ntHub;
	m_nClientID		= 0;
	m_nUserCount	= 0;
	m_nUserLimit	= 0;
	m_nFlags		= 0;
	m_pMoreResultsGUID = NULL;
}

CEDNeighbour::~CEDNeighbour()
{
	for ( POSITION pos = m_pQueries.GetHeadPosition() ; pos ; )
	{
		delete (GGUID*)m_pQueries.GetNext( pos );
	}
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour connect to

BOOL CEDNeighbour::ConnectTo(IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic)
{
	if ( CConnection::ConnectTo( pAddress, nPort ) )
	{
		WSAEventSelect( m_hSocket, Network.m_pWakeup, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE );
		
		theApp.Message( MSG_DEFAULT, IDS_ED2K_SERVER_CONNECTING,
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

BOOL CEDNeighbour::Send(CPacket* pPacket, BOOL bRelease, BOOL bBuffered)
{
	BOOL bSuccess = FALSE;
	
	if ( ( m_nState == nrsHandshake1 || m_nState >= nrsConnected ) &&
		 pPacket->m_nProtocol == PROTOCOL_ED2K )
	{
		m_nOutputCount++;
		Statistics.Current.eDonkey.Outgoing++;
		
		pPacket->ToBuffer( m_pZOutput ? m_pZOutput : m_pOutput );
		QueueRun();
		
		pPacket->SmartDump( this, NULL, TRUE );
		// pPacket->Debug( _T("DonkeyOut") );
		bSuccess = TRUE;
	}
	
	if ( bRelease ) pPacket->Release();
	
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour run event

BOOL CEDNeighbour::OnRun()
{
	DWORD tNow = GetTickCount();
	
	if ( m_nState != nrsConnected )
	{
		if ( tNow - m_tConnected > Settings.Connection.TimeoutConnect )
		{
			Close( IDS_CONNECTION_TIMEOUT_CONNECT );
			return FALSE;
		}
	}
	else if ( m_nClientID == 0 )
	{
		if ( tNow - m_tConnected > Settings.Connection.TimeoutHandshake )
		{
			Close( IDS_HANDSHAKE_TIMEOUT );
			return FALSE;
		}
	}
	else
	{
		if ( tNow - m_tLastPacket > 20 * 60 * 1000 )
		{
			Close( IDS_CONNECTION_TIMEOUT_TRAFFIC );
			return FALSE;
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour connection event

BOOL CEDNeighbour::OnConnected()
{
	theApp.Message( MSG_DEFAULT, IDS_ED2K_SERVER_CONNECTED, (LPCTSTR)m_sAddress );
	
	CEDPacket* pPacket = CEDPacket::New(  ED2K_C2S_LOGINREQUEST );
	
	GGUID pGUID	= MyProfile.GUID;
	pGUID.n[5]	= 14;
	pGUID.n[14]	= 111;
	pPacket->Write( &pGUID, 16 );
	
	pPacket->WriteLongLE( m_nClientID );
	pPacket->WriteShortLE( htons( Network.m_pHost.sin_port ) );
	pPacket->WriteLongLE( 4 );
	
	CEDTag( ED2K_CT_NAME, MyProfile.GetNick().Left( 255 ) ).Write( pPacket );
	CEDTag( ED2K_CT_VERSION, ED2K_VERSION ).Write( pPacket );
	CEDTag( ED2K_CT_PORT, htons( Network.m_pHost.sin_port ) ).Write( pPacket );
	CEDTag( ED2K_CT_COMPRESSION, 1 ).Write( pPacket );
	
	m_nState = nrsHandshake1;
	Send( pPacket );
	
	return TRUE;
}

void CEDNeighbour::OnDropped(BOOL bError)
{
	if ( m_nState < nrsConnected )
	{
		HostCache.OnFailure( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
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
	BOOL bSuccess = TRUE;
	CEDPacket* pPacket;
	
	CNeighbour::OnRead();
	
	while ( pPacket = CEDPacket::ReadBuffer( m_pZInput ? m_pZInput : m_pInput, ED2K_PROTOCOL_EDONKEY ) )
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
		if ( ! bSuccess ) break;
	}
	
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour packet handler

BOOL CEDNeighbour::OnPacket(CEDPacket* pPacket)
{
	pPacket->SmartDump( this, NULL, FALSE );
	// pPacket->Debug( _T("DonkeyIn") );
	
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
		return OnCallbackRequest( pPacket );
	case ED2K_S2C_SEARCHRESULTS:
		return OnSearchResults( pPacket );
	case ED2K_S2C_FOUNDSOURCES:
		return OnFoundSources( pPacket );
	default:
		pPacket->Debug( _T("Unknown") );
		break;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour server packets

BOOL CEDNeighbour::OnRejected(CEDPacket* pPacket)
{
	Close( IDS_ED2K_SERVER_REJECTED );
	return FALSE;
}

BOOL CEDNeighbour::OnServerMessage(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < 4 ) return TRUE;
	
	CString	strMessage = pPacket->ReadEDString();
	
	while ( strMessage.GetLength() > 0 )
	{
		CString strLine = strMessage.SpanExcluding( _T("\r\n") );
		
		if ( strLine.GetLength() > 0 )
		{
			strMessage = strMessage.Mid( strLine.GetLength() );
			theApp.Message( MSG_SYSTEM, IDS_ED2K_SERVER_MESSAGE,
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
		m_nFlags = pPacket->ReadLongLE();
	}
	
	if ( m_nClientID == 0 )
	{
		theApp.Message( MSG_DEFAULT, IDS_ED2K_SERVER_ONLINE, (LPCTSTR)m_sAddress, nClientID );
		SendSharedFiles();
		Send( CEDPacket::New(  ED2K_C2S_GETSERVERLIST ) );
		HostCache.eDonkey.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
	}
	else
	{
		theApp.Message( MSG_DEFAULT, IDS_ED2K_SERVER_IDCHANGE, (LPCTSTR)m_sAddress, m_nClientID, nClientID );
	}
	
	m_nState	= nrsConnected;
	m_nClientID	= nClientID;
	
	if ( ! CEDPacket::IsLowID( m_nClientID ) )
	{
		if ( Settings.Connection.InHost.IsEmpty() )
		{
			Network.m_pHost.sin_addr.S_un.S_addr = m_nClientID;
		}
	}
	
	return TRUE;
}

BOOL CEDNeighbour::OnServerList(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < 1 ) return TRUE;
	
	int nCount = pPacket->ReadByte();
	if ( pPacket->GetRemaining() < nCount * 6 ) return TRUE;
	
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
	
	HostCache.eDonkey.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
	
	return TRUE;
}

BOOL CEDNeighbour::OnServerStatus(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < 8 ) return TRUE;
	
	m_nUserCount	= pPacket->ReadLongLE();
	m_nFileCount	= pPacket->ReadLongLE();
	
	if ( CHostCacheHost* pHost = HostCache.eDonkey.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) ) )
	{
		// pHost->m_nUserCount = max( pHost->m_nUserCount, m_nUserCount );
		pHost->m_nUserCount = m_nUserCount;
		pHost->m_nUserLimit = max( pHost->m_nUserLimit, m_nUserCount );
	}
	
	return TRUE;
}

BOOL CEDNeighbour::OnServerIdent(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < sizeof(GGUID) + 6 + 4 ) return TRUE;
	
	m_bGUID = TRUE;
	pPacket->Read( &m_pGUID, sizeof(GGUID) );
	
	pPacket->ReadLongLE();	// IP
	pPacket->ReadShortLE();	// Port
	
	DWORD nTags = pPacket->ReadLongLE();
	CString strDescription;
	
	while ( nTags-- > 0 && pPacket->GetRemaining() > 1 )
	{
		CEDTag pTag;
		if ( ! pTag.Read( pPacket ) ) break;
		
		if ( pTag.Check( ED2K_ST_SERVERNAME, ED2K_TAG_STRING ) )
		{
			m_sServerName = pTag.m_sValue;
		}
		else if ( pTag.Check( ED2K_ST_DESCRIPTION, ED2K_TAG_STRING ) )
		{
			strDescription = pTag.m_sValue;
		}
		else if ( pTag.Check( ED2K_ST_MAXUSERS, ED2K_TAG_INT ) )
		{
			m_nUserLimit = pTag.m_nValue;
		}
	}
	
	if ( (DWORD&)m_pGUID == 0x2A2A2A2A )
		m_sUserAgent = _T("eFarm Server");
	else
		m_sUserAgent = _T("eDonkey2000 Server");
	
	if ( CHostCacheHost* pHost = HostCache.eDonkey.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) ) )
	{
		pHost->m_sName			= m_sServerName;
		pHost->m_sDescription	= strDescription;
		pHost->m_nUserLimit		= m_nUserLimit;
	}
	
	theApp.Message( MSG_DEFAULT, IDS_ED2K_SERVER_IDENT, (LPCTSTR)m_sAddress, (LPCTSTR)m_sServerName );
	
	return TRUE;
}

BOOL CEDNeighbour::OnCallbackRequest(CEDPacket* pPacket)
{
	if ( pPacket->GetRemaining() < 6 ) return TRUE;
	
	DWORD nAddress	= pPacket->ReadLongLE();
	WORD nPort		= pPacket->ReadShortLE();
	
	if ( Network.IsFirewalledAddress( &nAddress ) ) return TRUE;
	
	EDClients.PushTo( nAddress, nPort );
	
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
	
	GGUID* pGUID		= (GGUID*)m_pQueries.RemoveHead();
	CQueryHit* pHits	= CQueryHit::FromPacket( pPacket, &m_pHost, pGUID );
	
	if ( pHits == NULL )
	{
		delete pGUID;
		
		if ( pPacket->m_nLength != 17 && pPacket->m_nLength != 5 )
		{
			pPacket->Debug( _T("BadSearchResult") );
			theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT, (LPCTSTR)m_sAddress );
			Statistics.Current.eDonkey.Dropped++;
			m_nDropCount++;
		}
		
		return TRUE;
	}
	
	if ( pPacket->m_nType == ED2K_S2C_SEARCHRESULTS && pPacket->GetRemaining() == 1 && m_pQueries.IsEmpty() )
	{
		if ( pPacket->ReadByte() == TRUE )
		{
			//theApp.Message( MSG_DEBUG, _T("Additional results packet recieved.") );
			m_pMoreResultsGUID = pGUID;
			pGUID = NULL;
		}
	}
	
	Network.OnQueryHits( pHits );
	
	if ( pGUID != NULL ) delete pGUID;
	
	return TRUE;
}

BOOL CEDNeighbour::OnFoundSources(CEDPacket* pPacket)
{
	return OnSearchResults( pPacket );
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour file adverising

void CEDNeighbour::SendSharedFiles()
{
	CEDPacket* pPacket = CEDPacket::New( ED2K_C2S_OFFERFILES );
	BYTE pPadding[6] = { 0, 0, 0, 0, 0, 0 };
	POSITION pos;
	
	int nCount = 0, nLimit = Settings.eDonkey.MaxShareCount;
	
	pPacket->WriteLongLE( nCount );
	
	CSingleLock pLock1( &Library.m_pSection );
	CSingleLock pLock2( &Transfers.m_pSection );
	
	//Send files in library to ed2k server
	pLock1.Lock();

	for ( pos = LibraryMaps.GetFileIterator() ; pos != NULL && ( nLimit == 0 || nCount < nLimit ) ; )
	{
		CLibraryFile* pFile = LibraryMaps.GetNextFile( pos );
		
		if ( pFile->IsShared() && pFile->m_bED2K )	//If file is shared and has an ed2k hash
		{
			if ( UploadQueues.CanUpload( PROTOCOL_ED2K, pFile, FALSE ) ) // Check if a queue exists
			{
				//Send the file hash and name to the ed2k server
				pPacket->Write( &pFile->m_pED2K, sizeof(MD4) );
				pPacket->Write( pPadding, 6 );
				pPacket->WriteLongLE( 2 );
				CEDTag( ED2K_FT_FILENAME, pFile->m_sName ).Write( pPacket );
				CEDTag( ED2K_FT_FILESIZE, (DWORD)pFile->GetSize() ).Write( pPacket );
				//Increment count of files sent
				nCount++;
/*							
				CString str;
				str.Format( _T("ED2K list- File Added: %s"), pFile->m_sName );
				theApp.Message( MSG_DEFAULT, str );
			}
			else
			{
				CString str;
				str.Format( _T("ED2K list- File not added: %s"), pFile->m_sName );
				theApp.Message( MSG_DEFAULT, str );
*/
			}
		}
	}
	
	pLock1.Unlock();

	//Send files on download list to ed2k server
	pLock2.Lock();
	
	for ( pos = Downloads.GetIterator() ; pos != NULL && ( nLimit == 0 || nCount < nLimit ) ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		if ( pDownload->m_bED2K && pDownload->IsStarted() && ! pDownload->IsMoving() &&
			 pDownload->NeedHashset() == FALSE )	//If the file has and ed2k hash, has started, etc...
		{
			//Send the file hash and name to the ed2k server
			pPacket->Write( &pDownload->m_pED2K, sizeof(MD4) );
			pPacket->Write( pPadding, 6 );
			pPacket->WriteLongLE( 2 );
			CEDTag( ED2K_FT_FILENAME, pDownload->m_sRemoteName ).Write( pPacket );
			CEDTag( ED2K_FT_FILESIZE, (DWORD)pDownload->m_nSize ).Write( pPacket );
			//Increment count of files sent
			nCount++;
		}
	}
	
	pLock2.Unlock();
	
	*(DWORD*)pPacket->m_pBuffer = nCount;
	
	if ( m_nFlags & ED2K_SERVER_TCP_DEFLATE ) pPacket->Deflate();
	Send( pPacket );
}

BOOL CEDNeighbour::SendSharedDownload(CDownload* pDownload)
{
	if ( m_nState < nrsConnected ) return FALSE;
	if ( ! pDownload->m_bED2K || pDownload->NeedHashset() ) return FALSE;
	
	CEDPacket* pPacket = CEDPacket::New(  ED2K_C2S_OFFERFILES );
	BYTE pPadding[6] = { 0, 0, 0, 0, 0, 0 };
	
	pPacket->WriteLongLE( 1 );
	pPacket->Write( &pDownload->m_pED2K, sizeof(MD4) );
	pPacket->Write( pPadding, 6 );
	pPacket->WriteLongLE( 2 );
	CEDTag( ED2K_FT_FILENAME, pDownload->m_sRemoteName ).Write( pPacket );
	CEDTag( ED2K_FT_FILESIZE, (DWORD)pDownload->m_nSize ).Write( pPacket );
	
	if ( m_nFlags & ED2K_SERVER_TCP_DEFLATE ) pPacket->Deflate();
	Send( pPacket );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDNeighbour file adverising

BOOL CEDNeighbour::SendQuery(CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal)
{
	if ( m_nState != nrsConnected || m_nClientID == 0 )
	{
		return FALSE;	// We're not ready
	}
	else if ( pPacket == NULL || pPacket->m_nProtocol != PROTOCOL_ED2K || ! bLocal )
	{
		return FALSE;	// Packet is bad
	}
	
	m_pQueries.AddTail( new GGUID( pSearch->m_pGUID ) );
	Send( pPacket, FALSE, FALSE );
	
	return TRUE;
}
