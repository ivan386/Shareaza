//
// DownloadTransferDC.cpp 
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
#include "Settings.h"
#include "DCClient.h"
#include "DCClients.h"
#include "DCNeighbour.h"
#include "Download.h"
#include "DownloadTransfer.h"
#include "DownloadTransferDC.h"
#include "DownloadSource.h"
#include "Neighbour.h"
#include "Neighbours.h"
#include "Network.h"
#include "Transfers.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDownloadTransferDC::CDownloadTransferDC(CDownloadSource* pSource, CDCClient* pClient)
	: CDownloadTransfer( pSource, PROTOCOL_DC )
	, m_pClient( pClient )
{
	SetState( dtsConnecting );

	m_sQueueName = _T("DC++");
}

CDownloadTransferDC::~CDownloadTransferDC()
{
	ASSERT( m_pClient == NULL );
}

void CDownloadTransferDC::AttachTo(CConnection* pConnection)
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( m_pClient == NULL );

	m_pClient = static_cast< CDCClient* >( pConnection );
	m_pClient->m_pDownloadTransfer = this;

	m_tConnected = m_pClient->m_tConnected;

	// Get nick from connected hub
	CSingleLock oLock( &Network.m_pSection );
	if ( oLock.Lock( 250 ) )
	{
		CNeighbour* pNeighbour = Neighbours.Get( m_pSource->m_pServerAddress );
		if ( pNeighbour &&
			 pNeighbour->m_nProtocol == PROTOCOL_DC &&
			 pNeighbour->m_nState == nrsConnected )
		{
			m_pClient->m_sNick = static_cast< CDCNeighbour* >( pNeighbour )->m_sNick;
		}
	}
	if ( m_pClient->m_sNick.IsEmpty() )
		m_pClient->m_sNick = DCClients.GetDefaultNick();

	m_pClient->Handshake();
}

BOOL CDownloadTransferDC::Initiate()
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( m_pClient == NULL );
	ASSERT( m_nState == dtsNull );

	/*theApp.Message( MSG_INFO, IDS_DOWNLOAD_CONNECTING,
		(LPCTSTR)CString( inet_ntoa( m_pSource->m_pServerAddress ) ),
		m_pSource->m_nServerPort,
		(LPCTSTR)m_pDownload->GetDisplayName() );

	m_pClient = DCClients.Connect( this );
	if ( ! m_pClient )
	{
		Close( TRI_FALSE );
		return FALSE;
	}

	if ( m_pClient->m_nState == nrsNull )
	{
		delete m_pClient;
		m_pClient = NULL;
		Close( TRI_TRUE, 10 );	// Wait 10 seconds for completing connection to hub
		return FALSE;
	}

	SetState( dtsConnecting );
	m_tConnected = GetTickCount();*/

	return FALSE;
}

void CDownloadTransferDC::Close(TRISTATE bKeepSource, DWORD nRetryAfter)
{
	if ( m_pClient != NULL )
	{
		m_pClient->m_pDownloadTransfer = NULL;
		m_pClient->Close();
		m_pClient = NULL;
	}

	CDownloadTransfer::Close( bKeepSource, nRetryAfter );
}

DWORD CDownloadTransferDC::GetMeasuredSpeed()
{
	if ( m_pClient == NULL )
		return 0;

	m_pClient->MeasureIn();

	return m_pClient->m_mInput.nMeasure;
}

BOOL CDownloadTransferDC::SubtractRequested(Fragments::List& ppFragments)
{
	if ( m_nOffset != SIZE_UNKNOWN && m_nLength != SIZE_UNKNOWN &&
		( m_nState == dtsRequesting || m_nState == dtsDownloading ) )
	{
		ppFragments.erase( Fragments::Fragment( m_nOffset, m_nOffset + m_nLength ) );
		return TRUE;
	}
	return FALSE;
}

BOOL CDownloadTransferDC::OnConnected()
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( m_pClient != NULL );
	ASSERT( m_pSource != NULL );

	m_sUserAgent	= m_pClient->m_sUserAgent;
	m_pHost			= m_pClient->m_pHost;
	m_sAddress		= m_pClient->m_sAddress;
	UpdateCountry();

	m_pSource->m_sServer		= m_sUserAgent;
	m_pSource->m_sCountry		= m_sCountry;
	m_pSource->m_sCountryName	= m_sCountryName;

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_CONNECTED, (LPCTSTR)m_sAddress );

	if ( ! m_pDownload->PrepareFile() )
	{
		Close( TRI_TRUE );
		return FALSE;
	}

	return StartNextFragment();
}

BOOL CDownloadTransferDC::OnRun()
{
	if ( ! CDownloadTransfer::OnRun() )
		return FALSE;

	DWORD tNow = GetTickCount();

	switch ( m_nState )
	{
	case dtsConnecting:
		if ( tNow - m_tConnected > Settings.Connection.TimeoutConnect )
		{
			theApp.Message( MSG_ERROR, IDS_CONNECTION_TIMEOUT_CONNECT,
				(LPCTSTR)m_sAddress );
			Close( TRI_TRUE );
			return FALSE;
		}
		break;

	case dtsRequesting:
		if ( tNow - m_tRequest > Settings.Connection.TimeoutHandshake )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_REQUEST_TIMEOUT,
				(LPCTSTR)m_sAddress );
			Close( TRI_TRUE );
			return FALSE;
		}
		break;

	case dtsDownloading:
	case dtsFlushing:
	case dtsTiger:
		if ( tNow - m_mInput.tLast > Settings.Connection.TimeoutTraffic )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_TRAFFIC_TIMEOUT,
				(LPCTSTR)m_sAddress );
			Close( TRI_TRUE );
			return FALSE;
		}
		break;

	case dtsBusy:
		if ( tNow - m_tRequest > 1000 )
		{
			DWORD nDelay = Settings.Downloads.RetryDelay / 1000;
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BUSY,
				(LPCTSTR)m_sAddress, nDelay );
			Close( TRI_TRUE, nDelay );
			return FALSE;
		}
		break;

	case dtsQueued:
		if ( tNow >= m_tRequest )
		{
			return StartNextFragment();
		}
		break;
	}

	return TRUE;
}

BOOL CDownloadTransferDC::OnRead()
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( m_pClient );

	m_mInput.tLast = m_pClient->m_mInput.tLast;

	CLockedBuffer pInput( m_pClient->GetInput() );

	QWORD nLength = min( (QWORD)pInput->m_nLength, m_nLength - m_nPosition );

	BOOL bSubmit = m_pDownload->SubmitData( m_nOffset + m_nPosition,
		pInput->m_pBuffer, nLength );

	pInput->Clear();

	m_nPosition += nLength;
	m_nDownloaded += nLength;

	if ( ! bSubmit )
	{
		BOOL bUseful = m_pDownload->IsRangeUsefulEnough( this,
			m_nOffset + m_nPosition, m_nLength - m_nPosition );
		if ( ! bUseful )
		{
			theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_OVERLAP, (LPCTSTR)m_sAddress );
			Close( TRI_TRUE );
			return FALSE;
		}
	}

	if ( m_nPosition >= m_nLength )
	{
		m_pSource->AddFragment( m_nOffset, m_nLength );

		return StartNextFragment();
	}

	return TRUE;
}

BOOL CDownloadTransferDC::OnDownload(const std::string& strType, const std::string& strFilename, QWORD nOffset, QWORD nLength, const std::string& strOptions)
{
	ASSERT( m_pClient );

	m_pClient->m_mInput.pLimit = &m_nBandwidth;
	m_pClient->m_mOutput.pLimit	= &Settings.Bandwidth.Request;

	BOOL bZip = ( strOptions.find("ZL1") != std::string::npos );

	if ( strType == "file" )
	{
		if ( strFilename.substr( 0, 4 ) == "TTH/" )
		{
			Hashes::TigerHash oTiger;
			if ( ! oTiger.fromString( CA2W( strFilename.substr( 4 ).c_str() ) ) )
			{
				// Invalid TigerTree hash encoding
				return FALSE;
			}

			if ( ! m_pDownload || m_pDownload->m_oTiger != oTiger ||
				m_nLength != nLength ||
				m_nOffset != nOffset ||
				bZip )
			{
				// Invalid answer
				return FALSE;
			}

			SetState( dtsDownloading );

			return TRUE;
		}
	}

	// Unsupported
	return FALSE;
}

BOOL CDownloadTransferDC::OnQueue(int nQueue)
{
	ASSERT( m_pClient );

	m_pSource->SetLastSeen();

	m_nOffset	= SIZE_UNKNOWN;
	m_nPosition	= 0;

	SetState( dtsQueued );

	m_tRequest	= GetTickCount() + Settings.Downloads.RetryDelay;
	m_nQueuePos	= nQueue;
	m_nQueueLen	= 0;	// TODO: Read total upload slots

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_QUEUED,
		(LPCTSTR)m_sAddress, m_nQueuePos, m_nQueueLen,
		(LPCTSTR)m_sQueueName );

	return TRUE;
}

BOOL CDownloadTransferDC::StartNextFragment()
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( m_pClient );

	ASSERT( this != NULL );
	if ( this == NULL )
		return FALSE;

	m_pSource->SetLastSeen();

	m_nOffset	= SIZE_UNKNOWN;
	m_nPosition	= 0;

	if ( m_pDownload->GetFragment( this ) )
	{
		// Downloading
		ChunkifyRequest( &m_nOffset, &m_nLength, Settings.Downloads.ChunkSize, TRUE );

		theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_REQUEST,
			m_nOffset, m_nOffset + m_nLength - 1,
			(LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress );

		CString strRequest;
		strRequest.Format( _T("$ADCGET file TTH/%s %I64u %I64u|"),
			m_pDownload->m_oTiger.toString(), m_nOffset, m_nLength );

		m_pClient->SendCommand( strRequest );

		// Sending request
		SetState( dtsRequesting );

		m_tRequest = GetTickCount();

		return TRUE;
	}
	else
	{
		if ( m_pSource )
			m_pSource->SetAvailableRanges( NULL );

		theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_END, (LPCTSTR)m_sAddress );
		Close( TRI_TRUE );
		return FALSE;
	}
}
