//
// DCClient.cpp
//
// Copyright (c) Shareaza Development Team, 2010-2011.
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
#include "Download.h"
#include "Downloads.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadTransferDC.h"
#include "DCClient.h"
#include "DCClients.h"
#include "DCNeighbour.h"
#include "HostCache.h"
#include "Network.h"
#include "Neighbours.h"
#include "Settings.h"
#include "UploadTransfer.h"
#include "UploadTransferDC.h"
#include "VendorCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDCClient::CDCClient(const IN_ADDR* pHubAddress, WORD nHubPort, LPCTSTR szNick, LPCTSTR szRemoteNick)
	: CTransfer				( PROTOCOL_DC )
	, m_sNick				( DCClients.CreateNick( szNick ) )
	, m_bExtended			( FALSE )
	, m_pDownloadTransfer	( NULL )
	, m_pUploadTransfer		( NULL )
	, m_bDirection			( TRI_UNKNOWN )
	, m_bNumberSent			( FALSE )
	, m_nNumber				( GenerateNumber() )
	, m_nRemoteNumber		( -1 )
	, m_bLogin				( FALSE )
	, m_bKey				( FALSE )
{
	TRACE( "[DC++] Creating client 0x%08x\n", (LPVOID)this );

	m_mInput.pLimit = &Settings.Bandwidth.Request;
	m_mOutput.pLimit = &Settings.Bandwidth.Request;

	ZeroMemory( &m_pServer, sizeof( m_pServer ) );
	m_pServer.sin_family = AF_INET;
	if ( pHubAddress ) m_pServer.sin_addr = *pHubAddress;
	if ( nHubPort ) m_pServer.sin_port = htons( nHubPort );

	if ( szRemoteNick && *szRemoteNick )
	{
		m_sRemoteNick = szRemoteNick;
		DCClients.CreateGUID( m_sRemoteNick, m_oGUID );
	}
}

CDCClient::~CDCClient()
{
	TRACE( "[DC++] Destroying client 0x%08x\n", (LPVOID)this );

	ASSERT( ! IsValid() );
	ASSERT( m_pUploadTransfer == NULL );
	ASSERT( m_pDownloadTransfer == NULL );

	DCClients.Remove( this );
}

CString CDCClient::GetUserAgent()
{
	if ( ! m_sUserAgent.IsEmpty() )
		return m_sUserAgent;

	CSingleLock oLock( &Network.m_pSection, TRUE );

	CDCNeighbour* pDCNeighbour = NULL;

	// Get existing hub by address
	if ( CNeighbour* pNeighbour = Neighbours.Get( m_pServer.sin_addr ) )
	{
		if ( pNeighbour->m_nProtocol == PROTOCOL_DC )
		{
			pDCNeighbour = static_cast< CDCNeighbour* >( pNeighbour );
		}
	}

	// Get existing hub by user name
	if ( ! pDCNeighbour )
		pDCNeighbour = DCClients.GetHub( m_sRemoteNick );

	if ( pDCNeighbour )
	{
		if ( CChatUser* pUser = pDCNeighbour->GetUser( m_sRemoteNick ) )
		{
			m_sUserAgent = pUser->m_sUserAgent;
			m_bClientExtended = VendorCache.IsExtended( m_sUserAgent );
		}
	}

	return m_sUserAgent.IsEmpty() ? protocolNames[ m_nProtocol ] : m_sUserAgent;
}

void CDCClient::Merge(CDCClient* pClient)
{
	ASSERT( pClient != NULL );

	TRACE( "[DC++] Merging client 0x%08x to 0x%08x\n", (LPVOID)pClient, (LPVOID)this );

	if ( pClient->m_pDownloadTransfer )
	{
		DetachDownload();

		m_pDownloadTransfer = pClient->m_pDownloadTransfer;
		m_pDownloadTransfer->m_pClient = this;
		pClient->m_pDownloadTransfer = NULL;
	}

	if ( pClient->m_pUploadTransfer )
	{
		DetachUpload();

		m_pUploadTransfer = pClient->m_pUploadTransfer;
		m_pUploadTransfer->m_pClient = this;
		pClient->m_pUploadTransfer = NULL;
	}

	pClient->Remove();
}

std::string CDCClient::GenerateLock() const
{
	char cLock[ 256 ] = {};
	sprintf_s( cLock,
		"EXTENDEDPROTOCOL::" CLIENT_NAME "::%s:%u",
		inet_ntoa( m_pHost.sin_addr ), htons( m_pHost.sin_port ) );
	return cLock;
}

int CDCClient::GenerateNumber() const
{
	return GetRandomNum( 0u, 0x7FFFu );
}

BOOL CDCClient::CanDownload() const
{
	return m_pDownloadTransfer &&
		( m_bDirection == TRI_FALSE || m_nNumber > m_nRemoteNumber );
}

BOOL CDCClient::CanUpload() const
{
	return ! m_pDownloadTransfer ||
		( m_bDirection == TRI_TRUE && m_nNumber < m_nRemoteNumber );
}

BOOL CDCClient::Connect()
{
	m_tRequest	= GetTickCount();

	DCClients.Add( this );

	CSingleLock oLock( &Network.m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return FALSE;

	CDCNeighbour* pDCNeighbour = NULL;

	// Get existing hub by address
	if ( CNeighbour* pNeighbour = Neighbours.Get( m_pServer.sin_addr ) )
	{
		if ( pNeighbour->m_nProtocol == PROTOCOL_DC )
		{
			pDCNeighbour = static_cast< CDCNeighbour* >( pNeighbour );
		}
		else
		{
			// Multi-protocol hub?
			return FALSE;
		}
	}

	// Get existing hub by user name
	if ( ! pDCNeighbour )
		pDCNeighbour = DCClients.GetHub( m_sRemoteNick );

	if ( pDCNeighbour )
	{
		if ( CChatUser* pUser = pDCNeighbour->GetUser( m_sRemoteNick ) )
		{
			m_sUserAgent = pUser->m_sUserAgent;
			m_bClientExtended = VendorCache.IsExtended( m_sUserAgent );
		}
		return pDCNeighbour->ConnectToMe( m_sRemoteNick );
	}

	// Connect to new hub
	pDCNeighbour = new CDCNeighbour();
	if ( pDCNeighbour )
	{
		if ( pDCNeighbour->ConnectTo( &m_pServer.sin_addr, ntohs( m_pServer.sin_port ), FALSE ) )
		{
			return FALSE;
		}

		// Can't connect
		delete pDCNeighbour;
	}

	return FALSE;
}

BOOL CDCClient::ConnectTo(const IN_ADDR* pAddress, WORD nPort)
{
	CString sHost( inet_ntoa( *pAddress ) );

	if ( CTransfer::ConnectTo( pAddress, nPort ) )
	{
		WSAEventSelect( m_hSocket, Network.GetWakeupEvent(),
			FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE );

		theApp.Message( MSG_INFO, IDS_CONNECTION_ATTEMPTING,
			(LPCTSTR)sHost, htons( m_pHost.sin_port ) );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_CONNECTION_CONNECT_FAIL,
			(LPCTSTR)sHost );
		return FALSE;
	}

	DCClients.Add( this );

	return TRUE;
}

void CDCClient::AttachTo(CConnection* pConnection)
{
	CTransfer::AttachTo( pConnection );

	m_tConnected = GetTickCount();

	DCClients.Add( this );
}

void CDCClient::Close(UINT nError)
{
	m_bLogin = FALSE;

	m_mInput.pLimit = NULL;
	m_mOutput.pLimit = NULL;

	CTransfer::Close( nError );
}

void CDCClient::Remove()
{
	Close();

	DetachUpload();
	DetachDownload();

	delete this;
}

BOOL CDCClient::IsOnline() const
{
	return m_bConnected && m_bLogin;
}

BOOL CDCClient::IsDownloading() const
{
	return m_pDownloadTransfer &&
		( m_pDownloadTransfer->m_nState == dtsDownloading ||
		  m_pDownloadTransfer->m_nState == dtsTiger );
}

BOOL CDCClient::IsUploading() const
{
	return m_pUploadTransfer &&
		m_pUploadTransfer->m_nState >= upsUploading;
}

BOOL CDCClient::IsIdle() const
{
	return ( ! m_pDownloadTransfer || m_pDownloadTransfer->IsIdle() ) &&
		   ( ! m_pUploadTransfer   || m_pUploadTransfer->IsIdle() );
}

BOOL CDCClient::OnPush()
{
	return Network.OnPush( m_oGUID, this );
}

BOOL CDCClient::OnConnected()
{
	if ( ! CTransfer::OnConnected() )
		return FALSE;

	Greetings();

	LogOutgoing();

	return OnWrite();
}

void CDCClient::OnDropped()
{
	BOOL bTransfer = ( m_pDownloadTransfer || m_pUploadTransfer );

	if ( m_pDownloadTransfer )
	{
		m_pDownloadTransfer->OnDropped();
	}

	if ( m_pUploadTransfer )
	{
		m_pUploadTransfer->OnDropped();
	}

	if ( bTransfer )
	{
		Close();
	}
	else if ( m_nState == nrsConnecting )
	{
		Close( IDS_CONNECTION_REFUSED );
	}
	else
	{
		Close( IDS_CONNECTION_DROPPED );
	}
}

BOOL CDCClient::OnRun()
{
	if ( ! CTransfer::OnRun() )
		return FALSE;

	if ( m_pDownloadTransfer )
	{
		if ( ! m_pDownloadTransfer->OnRun() )
			return FALSE;
	}

	if ( m_pUploadTransfer )
	{
		if ( ! m_pUploadTransfer->OnRun() )
			return FALSE;
	}

	if ( ! m_pDownloadTransfer && ! m_pUploadTransfer && ! IsValid() )
	{
		if ( GetTickCount() > m_tRequest + Settings.Downloads.PushTimeout )
		{
			// Delete unused client
			Remove();
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CDCClient::OnRead()
{
	if ( ! CTransfer::OnRead() )
		return FALSE;

	for ( ;; )
	{
		// Download mode
		if ( IsDownloading() )
		{
			return m_pDownloadTransfer->OnRead();
		}

		// Command mode
		std::string strLine;
		if ( ! ReadCommand( strLine ) )
			break;

		theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING,
			_T("%s >> %s|"), (LPCTSTR)m_sAddress, (LPCTSTR)CA2CT( strLine.c_str() ) );

		std::string strCommand, strParams;
		std::string::size_type nPos = strLine.find( ' ' );
		if ( nPos != std::string::npos )
		{
			strCommand = strLine.substr( 0, nPos );
			strParams = strLine.substr( nPos + 1 );
		}
		else
			strCommand = strLine;

		if ( ! OnCommand( strCommand, strParams ) )
			return FALSE;
	}

	return TRUE;
}

BOOL CDCClient::ReadCommand(std::string& strLine)
{
	strLine.clear();

	CLockedBuffer pInput( GetInput() );

	if ( ! pInput->m_nLength )
		return FALSE;

	DWORD nLength = 0;
	for ( ; nLength < pInput->m_nLength ; nLength++ )
	{
		if ( pInput->m_pBuffer[ nLength ] == '|' )
			break;
	}

	if ( nLength >= pInput->m_nLength )
		return FALSE;

	strLine.append( (const char*)pInput->m_pBuffer, nLength );

	pInput->Remove( nLength + 1 );

	return TRUE;
}

BOOL CDCClient::OnWrite()
{
	// Upload mode
	if ( m_pUploadTransfer )
	{
		if ( ! m_pUploadTransfer->OnWrite() )
			return FALSE;
	}

	return CTransfer::OnWrite();
}

BOOL CDCClient::OnCommand(const std::string& strCommand, const std::string& strParams)
{
	if ( strCommand == "$MyNick" )
	{
		return OnMyNick( strParams );
	}
	else if ( strCommand == "$Lock" )
	{
		return OnLock( strParams );
	}
	else if ( strCommand == "$Supports" )
	{
		return OnSupports( strParams );
	}
	else if ( strCommand == "$Key" )
	{
		return OnKey( strParams );
	}
	else if ( strCommand == "$Direction" )
	{
		return OnDirection( strParams );
	}
	else if ( strCommand == "$ADCGET" )
	{
		return OnADCGet( strParams );
	}
	else if ( strCommand == "$ADCSND" )
	{
		return OnADCSnd( strParams );
	}
	else if ( strCommand == "$Get" )
	{
		return OnGet( strParams );
	}
	else if ( strCommand == "$Send" )
	{
		return OnSend( strParams );
	}
	else if ( strCommand == "$MaxedOut" )
	{
		return OnMaxedOut( strParams );
	}
	else if ( strCommand == "$Error" )
	{
		return OnError( strParams );
	}
	else if (
		strCommand == "$GetZBlock" ||
		strCommand == "$UGetBlock" ||
		strCommand == "$UGetZBlock" ||
		strCommand == "$GetListLen" )
	{
		Write( _P("$Error Command not supported. Use ADCGET/ADCSND instead|") );
	}
	else
	{
		Write( _P("$Error Unknown command|") );
	}

	LogOutgoing();

	return OnWrite();
}

BOOL CDCClient::OnMyNick(const std::string& strParams)
{
	// $MyNick RemoteNick|

	m_sRemoteNick = UTF8Decode( strParams.c_str() );
	DCClients.CreateGUID( m_sRemoteNick, m_oGUID );

	return ! DCClients.Merge( this );
}

BOOL CDCClient::OnLock(const std::string& strParams)
{
	// $Lock [EXTENDEDPROTOCOL]Challenge Pk=Vendor[Version][Ref=URL]|

	if ( m_sRemoteNick.IsEmpty() )
		// $Lock without $MyNick
		return FALSE;

	m_bExtended = ( strParams.substr( 0, 16 ) == "EXTENDEDPROTOCOL" );

	// Reset direction number
	m_bNumberSent = FALSE;
	m_nNumber = GenerateNumber();

	std::string strLock;
	std::string::size_type nPos = strParams.find( " Pk=" );
	if ( nPos != std::string::npos )
	{
		// Good way
		strLock = strParams.substr( 0, nPos );
		//std::string strUserAgent = strParams.substr( nPos + 4 );
		//nPos = strUserAgent.find( "Ref=" );
		//if ( nPos != std::string::npos )
		//	sPk = strUserAgent.substr( 0, nPos ).c_str();
		//else
		//	sPk = strUserAgent.c_str();
	}
	else
	{
		// Bad way
		nPos = strParams.find( ' ' );
		if ( nPos != std::string::npos )
			strLock = strParams.substr( 0, nPos );
		else
			// Very bad way
			strLock = strParams;
	}

	m_bLogin = TRUE;
	m_strKey = DCClients.MakeKey( strLock );

	if ( ! m_bInitiated )
	{
		if ( m_pDownloadTransfer )
		{
			if ( StartDownload() )
				return TRUE;
		}

		if ( Network.OnPush( m_oGUID, this ) )
			return TRUE;

		return Handshake();
	}

	return TRUE;
}

BOOL CDCClient::OnSupports(const std::string& strParams)
{
	// $Supports [option1]...[optionN]

	m_bExtended = TRUE;

	m_oFeatures.RemoveAll();
	for ( CString strFeatures( strParams.c_str() ); ! strFeatures.IsEmpty(); )
	{
		CString strFeature = strFeatures.SpanExcluding( _T(" ") );
		strFeatures = strFeatures.Mid( strFeature.GetLength() + 1 );
		if ( strFeature.IsEmpty() )
			continue;
		strFeature.MakeLower();
		if ( m_oFeatures.Find( strFeature ) == NULL )
		{
			m_oFeatures.AddTail( strFeature );
		}				
	}

	return TRUE;
}

BOOL CDCClient::OnKey(const std::string& strParams)
{
	// $Key key

	std::string sKey = DCClients.MakeKey( GenerateLock() );
	if ( sKey != strParams )
		// Wrong key
		return FALSE;

	// Right key
	m_bKey = TRUE;

	if ( m_bInitiated )
		return Handshake();

	return StartDownload();
}

BOOL CDCClient::StartDownload()
{
	if ( ! m_bKey )
		// Need a key
		return Handshake();

	if ( CanDownload() )
	{
		// Start download
		DetachUpload();
		return m_pDownloadTransfer->OnConnected();
	}

	// Can't download
	Close();

	return TRUE;
}

BOOL CDCClient::OnDirection(const std::string& strParams)
{
	// $Direction (Download|Upload) Number

	std::string::size_type nPos = strParams.find( ' ' );
	if ( nPos == std::string::npos )
		// Invalid command
		return FALSE;
	std::string strDirection = strParams.substr( 0, nPos );
	if ( strDirection == "Download" )
		m_bDirection = TRI_TRUE;
	else if ( strDirection == "Upload" )
		m_bDirection = TRI_FALSE;
	else
		// Invalid command
		return FALSE;

	m_nRemoteNumber = atoi( strParams.substr( nPos + 1 ).c_str() );
	if ( m_nRemoteNumber < 0 || m_nRemoteNumber > 0x7FFF )
	{
		// Invalid number
		return FALSE;
	}

	if ( m_bNumberSent )
	{
		if ( m_nRemoteNumber == m_nNumber )
			// Same numbers
			return FALSE;
	}
	else
	{
		// Change number until its differ from remote one
		while ( m_nRemoteNumber == m_nNumber )
		{
			m_nNumber = GenerateNumber();
		}
	}

	return TRUE;
}

BOOL CDCClient::OnADCGet(const std::string& strParams)
{
	// $ADCGET (list|file|tthl) Filename Offset Length [ZL1]

	std::string::size_type nPos1 = strParams.find( ' ' );
	if ( nPos1 == std::string::npos )
		// Invalid command
		return FALSE;
	std::string strType = strParams.substr( 0, nPos1 );
	std::string::size_type nPos2 = strParams.find( ' ', nPos1 + 1 );
	if ( nPos2 == std::string::npos )
		// Invalid command
		return FALSE;
	std::string strFilename = strParams.substr( nPos1 + 1, nPos2 - nPos1 - 1 );
	std::string::size_type nPos3 = strParams.find( ' ', nPos2 + 1 );
	if ( nPos3 == std::string::npos )
		// Invalid command
		return FALSE;
	std::string strOffset = strParams.substr( nPos2 + 1, nPos3 - nPos2 - 1 );
	std::string::size_type nPos4 = strParams.find( ' ', nPos3 + 1 );
	std::string strLength, strOptions;
	if ( nPos4 == std::string::npos )
	{
		strLength = strParams.substr( nPos3 + 1 );
	}
	else
	{
		strLength = strParams.substr( nPos3 + 1, nPos4 - nPos3 - 1 );
		strOptions = strParams.substr( nPos4 + 1 );
	}
	QWORD nOffset;
	if ( sscanf_s( strOffset.c_str(), "%I64u", &nOffset ) != 1 )
		// Invalid command
		return FALSE;
	QWORD nLength;
	if ( sscanf_s( strLength.c_str(), "%I64u", &nLength ) != 1 )
		// Invalid command
		return FALSE;

	if ( CanUpload() )
	{
		// Start uploading...
		DetachDownload();

		if ( ! m_pUploadTransfer )
			m_pUploadTransfer = new CUploadTransferDC( this );
		if ( ! m_pUploadTransfer )
			// Out of memory
			return FALSE;

		m_pUploadTransfer->m_sRemoteNick = m_sRemoteNick;

		// Start upload
		return m_pUploadTransfer->OnUpload( strType, strFilename, nOffset, nLength, strOptions );
	}

	// Unexpected request
	DetachUpload();

	TRACE( "[DC++] Got $ADCGET but can't uplod!\n" );

	return FALSE;
}

BOOL CDCClient::OnADCSnd(const std::string& strParams)
{
	// $ADCSND (list|file|tthl) Filename Offset Length [ZL1]

	std::string::size_type nPos1 = strParams.find( ' ' );
	if ( nPos1 == std::string::npos )
		// Invalid command
		return FALSE;
	std::string strType = strParams.substr( 0, nPos1 );
	std::string::size_type nPos2 = strParams.find( ' ', nPos1 + 1 );
	if ( nPos2 == std::string::npos )
		// Invalid command
		return FALSE;
	std::string strFilename = strParams.substr( nPos1 + 1, nPos2 - nPos1 - 1 );
	std::string::size_type nPos3 = strParams.find( ' ', nPos2 + 1 );
	if ( nPos3 == std::string::npos )
		// Invalid command
		return FALSE;
	std::string strOffset = strParams.substr( nPos2 + 1, nPos3 - nPos2 - 1 );
	std::string::size_type nPos4 = strParams.find( ' ', nPos3 + 1 );
	std::string strLength, strOptions;
	if ( nPos4 == std::string::npos )
	{
		strLength = strParams.substr( nPos3 + 1 );
	}
	else
	{
		strLength = strParams.substr( nPos3 + 1, nPos4 - nPos3 - 1 );
		strOptions = strParams.substr( nPos4 + 1 );
	}

	QWORD nOffset;
	if ( sscanf_s( strOffset.c_str(), "%I64u", &nOffset ) != 1 )
		// Invalid command
		return FALSE;

	QWORD nLength;
	if ( sscanf_s( strLength.c_str(), "%I64u", &nLength ) != 1 )
		// Invalid command
		return FALSE;

	if ( CanDownload() )
	{
		// Start downloading...
		return m_pDownloadTransfer->OnDownload( strType, strFilename, nOffset, nLength, strOptions );
	}

	// Unexpected request
	DetachDownload();

	TRACE( "[DC++] Got $ADCSND but can't download!\n" );

	return FALSE;
}

BOOL CDCClient::OnGet(const std::string& strParams)
{
	// $Get Filename$Offset|
	// Offset counted from 1.

	std::string::size_type nPos1 = strParams.find( '$' );
	if ( nPos1 == std::string::npos )
		// Invalid command
		return FALSE;
	std::string strFilename = strParams.substr( 0, nPos1 );
	std::string strOffset = strParams.substr( nPos1 + 1 );

	QWORD nOffset;
	if ( sscanf_s( strOffset.c_str(), "%I64u", &nOffset ) != 1 || nOffset < 1 )
		// Invalid command
		return FALSE;
	nOffset--;

	if ( CanUpload() )
	{
		// Start uploading...
		DetachDownload();

		if ( ! m_pUploadTransfer )
			m_pUploadTransfer = new CUploadTransferDC( this );
		if ( ! m_pUploadTransfer )
			// Out of memory
			return FALSE;

		m_pUploadTransfer->m_sRemoteNick = m_sRemoteNick;

		// Start upload
		return m_pUploadTransfer->OnUpload( "get", strFilename, nOffset, SIZE_UNKNOWN, "" );
	}

	// Unexpected request
	DetachUpload();

	TRACE( "[DC++] Got $Get but can't uplod!\n" );

	return FALSE;
}

BOOL CDCClient::OnSend(const std::string& strParams)
{
	// $Send|

	if ( ! m_pUploadTransfer || ! strParams.empty() )
		return FALSE;

	// Start upload
	return m_pUploadTransfer->OnUpload( "send", "", SIZE_UNKNOWN, SIZE_UNKNOWN, "" );
}

BOOL CDCClient::OnMaxedOut(const std::string& strParams)
{
	// No free upload slots
	// $MaxedOut[ QueuePosition]|

	if ( m_pDownloadTransfer )
	{
		if ( strParams.empty() )
			return m_pDownloadTransfer->OnBusy();

		return m_pDownloadTransfer->OnQueue( atoi( strParams.c_str() ) );
	}

	TRACE( "[DC++] Got $MaxedOut but have no downloads!\n" );

	return TRUE;
}

BOOL CDCClient::OnError(const std::string& strParams)
{
	// $Error Error message|

	if ( m_pDownloadTransfer )
	{
		if ( ! m_pDownloadTransfer->OnError() )
			return FALSE;
	}

	/*if ( m_pUploadTransfer )
	{
		if ( ! m_pUploadTransfer->OnError() )
			return FALSE;
	}*/

	TRACE( "[DC++] Got $Error: \"%s\"\n", strParams.c_str() );
	strParams;

	return TRUE;
}

BOOL CDCClient::Greetings()
{
	ASSERT( IsValid() );
	ASSERT( ! m_sNick.IsEmpty() );

	Write( _P("$MyNick ") );
	Write( m_sNick );
	Write( _P("|") );

	std::string sLock = GenerateLock();

	Write( _P("$Lock ") );
	Write( sLock.c_str(), sLock.size() );
	Write( _P(" Pk=" CLIENT_NAME "/") );
	Write( theApp.m_sVersion );
	Write( _P("|") );

	return TRUE;
}

void CDCClient::AttachDownload(CDownloadTransferDC* pTransfer)
{
	m_pDownloadTransfer = pTransfer;

	pTransfer->m_tConnected = m_tConnected;

	// Get nick from connected hub
	CSingleLock oLock( &Network.m_pSection );
	if ( oLock.Lock( 250 ) )
	{
		if ( CDownloadSource* pSource = pTransfer->GetSource() )
		{
			CNeighbour* pNeighbour = Neighbours.Get( pSource->m_pServerAddress );
			if ( pNeighbour &&
				 pNeighbour->m_nProtocol == m_nProtocol &&
				 pNeighbour->m_nState == nrsConnected )
			{
				m_sNick = static_cast< CDCNeighbour* >( pNeighbour )->m_sNick;
			}
		}
	}
	m_sNick = DCClients.CreateNick( m_sNick );

	StartDownload();
}

void CDCClient::OnDownloadClose()
{
	BOOL bDownloading = IsDownloading();
	BOOL bIdle = IsIdle();

	m_pDownloadTransfer = NULL;

	m_mInput.pLimit = &Settings.Bandwidth.Request;

	if ( bDownloading )
	{
		// Bad close
		Close();
	}
	else if ( bIdle )
	{
		// Grace close - lets start next download if any
		Network.OnPush( m_oGUID, this );
	}
}

void CDCClient::OnUploadClose()
{
	BOOL bUploading = IsUploading();

	m_pUploadTransfer = NULL;

	m_mOutput.pLimit = &Settings.Bandwidth.Request;

	if ( bUploading )
	{
		Close();
	}
}

void CDCClient::DetachDownload()
{
	if ( m_pDownloadTransfer != NULL )
	{
		m_pDownloadTransfer->Close( TRI_UNKNOWN );
		ASSERT( m_pDownloadTransfer == NULL );
	}
}

void CDCClient::DetachUpload()
{
	if ( m_pUploadTransfer != NULL )
	{
		m_pUploadTransfer->Close();
		ASSERT( m_pUploadTransfer == NULL );
	}
}

BOOL CDCClient::Handshake()
{
	if ( ! m_bInitiated )
	{
		Greetings();
	}

	Write( _P(DC_CLIENT_SUPPORTS) );

	CString strDirection;
	strDirection.Format( _T("$Direction %s %i|"),
		( m_pDownloadTransfer ? _T("Download") : _T("Upload") ),
		 m_nNumber );
	Write( strDirection );

	m_bNumberSent = TRUE;

	if ( ! m_strKey.empty() )
	{
		Write( _P("$Key ") );
		Write( m_strKey.c_str(), m_strKey.size() );
		Write( _P("|") );
	}

	LogOutgoing();

	return OnWrite();
}

BOOL CDCClient::SendCommand(const CString& strSend)
{
	Write( strSend );

	LogOutgoing();

	return OnWrite();
}

