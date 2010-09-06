//
// DCClient.cpp
//
// Copyright (c) Shareaza Development Team, 2010.
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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDCClient::CDCClient(LPCTSTR szNick)
	: CTransfer				( PROTOCOL_DC )
	, m_bExtended			( FALSE )
	, m_pDownloadTransfer	( NULL )
	, m_pUploadTransfer		( NULL )
	, m_bDirection			( TRI_UNKNOWN )
	, m_nNumber				( GetRandomNum( 0u, 0x7FFFu ) )
	, m_nRemoteNumber		( -1 )
	, m_bLock				( FALSE )
	, m_bClosing			( FALSE )
{
	m_bAutoDelete = TRUE;

	if ( szNick ) m_sNick = szNick;
	m_sUserAgent = _T("DC++");

	m_mInput.pLimit = &Settings.Bandwidth.Request;
	m_mOutput.pLimit = &Settings.Bandwidth.Request;

	DCClients.Add( this );
}

CDCClient::~CDCClient()
{
	ASSERT( ! IsValid() );

	DCClients.Remove( this );
}

std::string CDCClient::GenerateLock() const
{
	char cLock[ 256 ] = {};
	sprintf_s( cLock,
		"EXTENDEDPROTOCOL::" CLIENT_NAME "::%s:%u",
		inet_ntoa( m_pHost.sin_addr ), htons( m_pHost.sin_port ) );
	return cLock;
}

BOOL CDCClient::CanDownload() const
{
	return m_pDownloadTransfer &&
		( m_bDirection == TRI_FALSE || m_nNumber > m_nRemoteNumber );
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

	m_nState = nrsConnecting;

	return TRUE;
}

void CDCClient::AttachTo(CConnection* pConnection)
{
	CTransfer::AttachTo( pConnection );

	m_nState = nrsConnected;
}

void CDCClient::Close(UINT nError)
{
	ASSERT( this != NULL );

	if ( m_bClosing )
		return;
	m_bClosing = TRUE;

	m_mInput.pLimit = NULL;
	m_mOutput.pLimit = NULL;

	if ( m_pUploadTransfer != NULL )
	{
		m_pUploadTransfer->Close();
		ASSERT( m_pUploadTransfer == NULL );
	}

	if ( m_pDownloadTransfer != NULL )
	{
		m_pDownloadTransfer->Close( TRI_UNKNOWN );
		ASSERT( m_pDownloadTransfer == NULL );
	}

	CTransfer::Close( nError );
}

BOOL CDCClient::OnConnected()
{
	if ( ! CTransfer::OnConnected() )
		return FALSE;

	m_nState = nrsConnected;

	Greetings();

	LogOutgoing();

	return OnWrite();
}

void CDCClient::OnDropped()
{
	/*if ( m_nState == upsUploading && m_pBaseFile )
	{
		m_pBaseFile->AddFragment( m_nOffset, m_nPosition );
		m_pBaseFile = NULL;
	}*/

	if ( m_nState == nrsConnecting )
	{
		Close( IDS_CONNECTION_REFUSED );
	}
	else
	{
		Close( IDS_UPLOAD_DROPPED );
	}
}

BOOL CDCClient::OnRun()
{
	if ( ! CTransfer::OnRun() )
		return FALSE;

	DWORD tNow = GetTickCount();

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

	if ( m_nState == nrsConnecting )
	{
		if ( tNow - m_tConnected > Settings.Connection.TimeoutConnect )
		{
			Close( IDS_CONNECTION_TIMEOUT_CONNECT );

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
		if ( m_pDownloadTransfer &&
			( m_pDownloadTransfer->m_nState == dtsDownloading ||
			  m_pDownloadTransfer->m_nState == dtsTiger ) )
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
	if ( m_pUploadTransfer && m_pUploadTransfer->m_nState >= upsUploading )
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
		// $MyNick RemoteNick|

		m_sRemoteNick = strParams.c_str();

		return TRUE;
	}
	else if ( strCommand == "$Lock" )
	{
		// $Lock [EXTENDEDPROTOCOL]Challenge Pk=Vendor|

		m_bExtended = ( strParams.substr( 0, 16 ) == "EXTENDEDPROTOCOL" );

		std::string strLock;
		std::string::size_type nPos = strParams.find( " Pk=" );
		if ( nPos != std::string::npos )
		{
			// Good way
			strLock = strParams.substr( 0, nPos );
			m_sUserAgent = strParams.substr( nPos + 4 ).c_str();
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

		m_strKey = DCClients.MakeKey( strLock );

		if ( m_sRemoteNick.IsEmpty() )
			// $Lock without $MyNick
			return FALSE;

		m_bLock = TRUE;

		if ( ! m_bInitiated )
		{
			Hashes::Guid oGUID;
			CMD5 pMD5;
			pMD5.Add( (LPCTSTR)m_sRemoteNick, m_sRemoteNick.GetLength() * sizeof( TCHAR ) );
			pMD5.Finish();
			pMD5.GetHash( &oGUID[ 0 ] );
			oGUID.validate();

			Downloads.OnPush( oGUID, this );
		}

		return TRUE;
	}
	else if ( strCommand == "$Supports" )
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
	else if ( strCommand == "$Key" )
	{
		// $Key key

		std::string sKey = DCClients.MakeKey( GenerateLock() );
		if ( sKey == strParams )
		{
			// Right key
			if ( m_bInitiated )
			{
				return Handshake();
			}
			else
			{
				if ( CanDownload() )
				{
					// Start download
					return m_pDownloadTransfer->OnConnected();
				}

				// Waiting for remote command
				return TRUE;
			}
		}
		else
		{
			// Wrong key
			return FALSE;
		}
	}
	else if ( strCommand == "$Direction" )
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
			// Invalid command
			return FALSE;

		return TRUE;
	}
	else if ( strCommand == "$ADCGET" )
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
		if ( sscanf_s( strLength.c_str(), "%I64d", &nLength ) != 1 )
			// Invalid command
			return FALSE;

		// Start uploading...
		if ( ! m_pDownloadTransfer )
		{
			if ( ! m_pUploadTransfer )
				m_pUploadTransfer = new CUploadTransferDC( this );
			if ( ! m_pUploadTransfer )
				// Out of memory
				return FALSE;

			// Start upload
			return m_pUploadTransfer->OnUpload( strType, strFilename, nOffset, nLength, strOptions );
		}

		// Unexpected request
		return FALSE;
	}
	else if ( strCommand == "$Get" ||
		strCommand == "$GetZBlock" ||
		strCommand == "$UGetBlock" ||
		strCommand == "$UGetZBlock" ||
		strCommand == "$GetListLen" )
	{
		Write( _P("$Error Command not supported. Use ADCGET/ADCSND instead|") );
	}
	else if ( strCommand == "$ADCSND" )
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
		if ( sscanf_s( strLength.c_str(), "%I64d", &nLength ) != 1 )
			// Invalid command
			return FALSE;

		if ( CanDownload() )
		{
			// Start downloading...
			return m_pDownloadTransfer->OnDownload( strType, strFilename, nOffset, nLength, strOptions );
		}
		else
			// Unexpected download
			return FALSE;
	}
	else if ( strCommand == "$MaxedOut" )
	{
		// No free upload slots
		// $MaxedOut[ QueuePosition]|

		int nQueue = strParams.empty() ? 0 : atoi( strParams.c_str() );

		if ( m_pDownloadTransfer )
		{
			return m_pDownloadTransfer->OnQueue( nQueue );
		}

		return TRUE;
	}
	else if ( strCommand == "$Error" )
	{
		// $Error Error message|

		return TRUE;
	}
	else
	{
		Write( _P("$Error Unknown command|") );
	}

	LogOutgoing();

	return OnWrite();
}

BOOL CDCClient::OnChat(const std::string& /*strMessage*/)
{
	return TRUE;
}

BOOL CDCClient::Greetings()
{
	ASSERT( m_nState == nrsConnected );
	ASSERT( ! m_sNick.IsEmpty() );

	Write( _P("$MyNick ") );
	Write( m_sNick );
	Write( _P("|") );

	std::string sLock = GenerateLock();

	Write( _P("$Lock ") );
	Write( sLock.c_str(), sLock.size() );
	Write( _P(" Pk=" CLIENT_NAME "|") );

	return TRUE;
}

BOOL CDCClient::Handshake()
{
	ASSERT( m_nState == nrsConnected );

	if ( ! m_bInitiated )
	{
		Greetings();
	}

	Write( _P("$Supports MiniSlots XmlBZList ADCGet TTHF ZLIG |") );

	CString strDirection;
	strDirection.Format( _T("$Direction %s %u|"),
		( m_pDownloadTransfer ? _T("Download") : _T("Upload") ),
		 m_nNumber );
	Write( strDirection );

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

