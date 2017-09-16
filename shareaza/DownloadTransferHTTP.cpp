//
// DownloadTransferHTTP.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
#include "DownloadGroups.h"
#include "Downloads.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadTransferHTTP.h"
#include "FragmentedFile.h"
#include "Network.h"
#include "Buffer.h"
#include "ShareazaURL.h"
#include "GProfile.h"
#include "XML.h"
#include "VendorCache.h"
#include "Transfers.h"
#include "Security.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP construction

CDownloadTransferHTTP::CDownloadTransferHTTP(CDownloadSource* pSource) :
	CDownloadTransfer( pSource, PROTOCOL_HTTP ),
	m_nRequests( 0 ),
	m_tContent( 0 ),
	m_bBadResponse( FALSE ),
	m_bBusyFault( FALSE ),
	m_bRangeFault( FALSE ),
	m_bKeepAlive( FALSE ),
	m_bTigerFetch( FALSE ),
	m_bTigerIgnore( FALSE ),
	m_bMetaFetch( FALSE ),
	m_bGotRange( FALSE ),
	m_bGotRanges( FALSE ),
	m_bQueueFlag( FALSE ),
	m_nContentLength( SIZE_UNKNOWN ),
	m_nRetryDelay( Settings.Downloads.RetryDelay ),
	m_nRetryAfter( 0 ),
	m_bRedirect( FALSE ),
	m_bGzip( FALSE ),
	m_bCompress( FALSE ),
	m_bDeflate( FALSE ),
	m_bChunked( FALSE ),
	m_ChunkState( Header ),
	m_nChunkLength( SIZE_UNKNOWN )
{
}

CDownloadTransferHTTP::~CDownloadTransferHTTP()
{
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP initiate connection

BOOL CDownloadTransferHTTP::Initiate()
{
	ASSUME_LOCK( Transfers.m_pSection );

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_CONNECTING,
		(LPCTSTR)CString( inet_ntoa( m_pSource->m_pAddress ) ), m_pSource->m_nPort,
		(LPCTSTR)m_pDownload->GetDisplayName() );

	if ( ConnectTo( &m_pSource->m_pAddress, m_pSource->m_nPort ) )
	{
		SetState( dtsConnecting );

		if ( ! m_pDownload->IsBoosted() )
			m_mInput.pLimit = m_mOutput.pLimit = &m_nBandwidth;

		return TRUE;
	}
	else
	{
		// Couldn't connect, keep the source but add to the m_pFailedSources
		// Mark it as an offline source, it might be good later...
		m_pDownload->AddFailedSource( m_pSource, true, true );
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CONNECT_ERROR, (LPCTSTR)m_sAddress );
		Close( TRI_UNKNOWN );
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP accept push

void CDownloadTransferHTTP::AttachTo(CConnection* pConnection)
{
	CDownloadTransfer::AttachTo( pConnection );

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_PUSHED, (LPCTSTR)m_sAddress,
		(LPCTSTR)m_pDownload->GetDisplayName() );

	if ( ! m_pDownload->IsBoosted() )
		m_mInput.pLimit = m_mOutput.pLimit = &m_nBandwidth;

	StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP close

void CDownloadTransferHTTP::Close( TRISTATE bKeepSource, DWORD nRetryAfter )
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_pSource != NULL && m_nState == dtsDownloading && m_nPosition )
	{
		if ( m_bRecvBackwards )
		{
			m_pSource->AddFragment( m_nOffset + m_nLength - m_nPosition, m_nPosition );
		}
		else
		{
			m_pSource->AddFragment( m_nOffset, m_nPosition );
		}
	}

	CDownloadTransfer::Close( bKeepSource, nRetryAfter );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP speed controls

void CDownloadTransferHTTP::Boost()
{
	m_mInput.pLimit = m_mOutput.pLimit = NULL;
}

DWORD CDownloadTransferHTTP::GetAverageSpeed()
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_nState == dtsDownloading )
	{
		DWORD nTime = GetTickCount() - m_tContent;
		if ( nTime )
			m_pSource->m_nSpeed = (DWORD)( ( m_nPosition * 1000 ) / nTime );
	}

	return m_AverageSpeed( m_pSource->m_nSpeed );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP connection handler

BOOL CDownloadTransferHTTP::OnConnected()
{
	theApp.Message( MSG_INFO, IDS_DOWNLOAD_CONNECTED, (LPCTSTR)m_sAddress );

	m_tConnected = GetTickCount();

	return StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP fragment allocation

BOOL CDownloadTransferHTTP::StartNextFragment()
{
	ASSUME_LOCK( Transfers.m_pSection );

	ASSERT( this != NULL );
	if ( this == NULL ) return FALSE;

	m_nOffset			= SIZE_UNKNOWN;
	m_nPosition			= 0;
	m_bWantBackwards	= FALSE;
	m_bRecvBackwards	= FALSE;
	m_bTigerFetch		= FALSE;
	m_bMetaFetch		= FALSE;

	if ( ! IsInputExist() || ! IsOutputExist() /* ||
		 m_pDownload->GetTransferCount( dtsDownloading ) >= Settings.Downloads.MaxFileTransfers */ )
	{
		theApp.Message( MSG_INFO, IDS_DOWNLOAD_CLOSING_EXTRA, (LPCTSTR)m_sAddress );
		Close( TRI_TRUE );
		return FALSE;
	}

	// this needs to go for pipeline

	if ( GetInputLength() > 0 && m_nRequests > 0 )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CLOSING_OVERFLOW, (LPCTSTR)m_sAddress );
		Close( TRI_TRUE );
		return FALSE;
	}

	if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
	{
		return SendRequest();
	}

	if ( m_pDownload->NeedTigerTree() )
	{
		if ( m_sTigerTree.IsEmpty() && m_pDownload->m_oTiger && Settings.Downloads.VerifyTiger && ! m_bTigerIgnore )
		{
			// Converting urn containing tiger tree root to
			// "/gnutella/thex/v1?urn:tree:tiger/:{TIGER_ROOT}&depth={TIGER_HEIGHT}&ed2k={0/1}"
			// in case if "X-Thex-URI" and "X-TigerTree-Path" headers will be absent or it is a "GIV" (push) connection
			m_sTigerTree.Format( L"/gnutella/thex/v1?%s&depth=%u&ed2k=%d", (LPCTSTR)m_pDownload->m_oTiger.toUrn(),
				Settings.Library.TigerHeight, ( Settings.Downloads.VerifyED2K ? 1 : 0 ) );
		}

		if ( ! m_sTigerTree.IsEmpty() )
		{
			theApp.Message( MSG_INFO,IDS_DOWNLOAD_TIGER_REQUEST, (LPCTSTR)m_pDownload->GetDisplayName(),(LPCTSTR)m_sAddress );

			m_bTigerFetch = TRUE;
			m_bTigerIgnore = TRUE;

			return SendRequest();
		}
	}

	if ( m_pSource && ! m_pSource->m_bMetaIgnore && ! m_sMetadata.IsEmpty() )
	{
		theApp.Message( MSG_INFO, IDS_DOWNLOAD_METADATA_REQUEST, (LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress );

		m_bMetaFetch = TRUE;
		m_pSource->m_bMetaIgnore = TRUE;

		return SendRequest();
	}

	if ( m_pDownload->GetFragment( this ) )
	{
		ChunkifyRequest( &m_nOffset, &m_nLength, Settings.Downloads.ChunkSize, TRUE );

		theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_REQUEST, m_nOffset, m_nOffset + m_nLength - 1, (LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress );

		return SendRequest();
	}

	if ( m_pSource != NULL ) m_pSource->SetAvailableRanges( NULL );

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_END, (LPCTSTR)m_sAddress );
	Close( TRI_TRUE );

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP subtract pending requests

BOOL CDownloadTransferHTTP::SubtractRequested(Fragments::List& ppFragments) const
{
	if ( m_nOffset < SIZE_UNKNOWN && m_nLength < SIZE_UNKNOWN )
	{
		if ( m_nState == dtsRequesting || m_nState == dtsDownloading )
		{
			ppFragments.erase( Fragments::Fragment( m_nOffset, m_nOffset + m_nLength ) );
			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP send request

BOOL CDownloadTransferHTTP::SendRequest()
{
	ASSUME_LOCK( Transfers.m_pSection );

	CString strLine;

	CShareazaURL pURL;
	if ( ! pURL.Parse( m_pSource->m_sURL, FALSE ) || pURL.m_nProtocol != PROTOCOL_HTTP ) return FALSE;

	if ( m_bTigerFetch )
	{
		pURL.m_sPath = m_sTigerTree;
		m_sTigerTree.Empty();
	}
	else if ( m_bMetaFetch )
	{
		pURL.m_sPath = m_sMetadata;
		m_sMetadata.Empty();
	}

	if ( Settings.Downloads.RequestHTTP11 )
	{
		strLine.Format( _T("GET %s HTTP/1.1\r\n"), (LPCTSTR)pURL.m_sPath );
		Write( strLine );

		strLine.Format( _T("Host: %s\r\n"), (LPCTSTR)pURL.m_sAddress );
		Write( strLine );
	}
	else
	{
		strLine.Format( _T("GET %s HTTP/1.0\r\n"), (LPCTSTR)m_pSource->m_sURL );
		Write( strLine );
	}

	Write( _P("Connection: Keep-Alive\r\n") ); //BearShare assumes close

	if ( Settings.Gnutella2.EnableToday ) Write( _P("X-Features: g2/1.0\r\n") );

	if ( m_bTigerFetch )
	{
		Write( _P("Accept: application/dime, application/tigertree-breadthfirst\r\n") );
	}
	else if ( m_bMetaFetch )
	{
		Write( _P("Accept: text/xml\r\n") );
	}

	if ( m_nOffset != SIZE_UNKNOWN && ! m_bTigerFetch && ! m_bMetaFetch )
	{
		if ( m_nLength == SIZE_UNKNOWN || m_nLength == 0 || m_nOffset + m_nLength == m_pDownload->m_nSize )
		{
			strLine.Format( _T("Range: bytes=%I64u-\r\n"), m_nOffset );
		}
		else
		{
			strLine.Format( _T("Range: bytes=%I64u-%I64u\r\n"), m_nOffset, m_nOffset + m_nLength - 1 );
		}
		Write( strLine );
	}
	else
	{
		Write( _P("Range: bytes=0-\r\n") );
	}

	if ( m_bWantBackwards && Settings.Downloads.AllowBackwards )
	{
		Write( _P("Accept-Encoding: backwards\r\n") );
	}

	strLine = Settings.SmartAgent();

	if ( strLine.GetLength() )
	{
		Write( _P("User-Agent: ") );
		Write( strLine );
		Write( _P("\r\n") );
	}

	if ( m_nRequests == 0 )
	{
		if ( m_bInitiated ) SendMyAddress();

		strLine = MyProfile.GetNick().Left( 255 );

		if ( strLine.GetLength() > 0 )
		{
			Write( _P("X-Nick: ") );
			Write( URLEncode( strLine ) );
			Write( _P("\r\n") );
		}
	}

	if ( m_pSource->m_nPort == INTERNET_DEFAULT_HTTP_PORT )
	{
		int nSlash = m_pSource->m_sURL.ReverseFind( '/' );
		if ( nSlash > 0 )
		{
			Write( _P("Referrer: ") );
			Write( m_pSource->m_sURL.Left( nSlash + 1 ) );
			Write( _P("\r\n") );
		}
	}

	Write( _P("X-Queue: 0.1\r\n") );

	if ( ! m_bTigerFetch && ! m_bMetaFetch )
	{
		if ( m_pDownload->m_oSHA1 || m_pDownload->m_oTiger )
		{
			Write( _P("X-Content-URN: ") );
			Write( m_pDownload->GetBitprint() );
			Write( _P("\r\n") );
		}
		if ( m_pDownload->m_oED2K )
		{
			Write( _P("X-Content-URN: ") );
			Write( m_pDownload->m_oED2K.toUrn() );
			Write( _P("\r\n") );
		}
		if ( m_pDownload->m_oBTH )
		{
			Write( _P("X-Content-URN: ") );
			Write( m_pDownload->m_oBTH.toUrn() );
			Write( _P("\r\n") );
		}
		if ( m_pDownload->m_oMD5 )
		{
			Write( _P("X-Content-URN: ") );
			Write( m_pDownload->m_oMD5.toUrn() );
			Write( _P("\r\n") );
		}
		if ( m_pSource->m_bSHA1 && Settings.Library.SourceMesh )
		{
			strLine = m_pDownload->GetSourceURLs( &m_pSourcesSent, 15,
				( m_pSource->m_nGnutella < 2 ) ? PROTOCOL_G1 : PROTOCOL_HTTP, m_pSource );
			if ( strLine.GetLength() )
			{
				if ( m_pSource->m_nGnutella < 2 )
					Write( _P("X-Alt: ") );
				else
					Write( _P("Alt-Location: ") );
				Write( strLine );
				Write( _P("\r\n") );
			}

			if ( m_pDownload->IsShared() && m_pDownload->IsStarted() && Network.IsStable() )
			{
				if ( m_pSource->m_nGnutella < 2 )
				{
					strLine.Format( _T("%s:%u"),
						(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
						htons( Network.m_pHost.sin_port ) );
					Write( _P("X-Alt: ") );
				}
				else
				{
					strLine = m_pDownload->GetURL( Network.m_pHost.sin_addr,
						htons( Network.m_pHost.sin_port ) ) + _T(" ") +
						TimeToString( time( NULL ) - 180 );
					Write( _P("Alt-Location: ") );
				}
				Write( strLine );
				Write( _P("\r\n") );

				if ( m_pSource->m_nGnutella < 2 )
				{
					strLine = m_pDownload->GetTopFailedSources( 15, PROTOCOL_G1 );
					if ( strLine.GetLength() )
					{
						Write( _P("X-NAlt: ") );
						Write( strLine );
						Write( _P("\r\n") );
					}
				}
			}
		}
	}

	LogOutgoing();

	Write( _P("\r\n") );

	SetState( dtsRequesting );
	m_tRequest			= GetTickCount();
	m_bBusyFault		= FALSE;
	m_bRangeFault		= FALSE;
	m_bKeepAlive		= FALSE;
	m_bGotRange			= FALSE;
	m_bGotRanges		= FALSE;
	m_bQueueFlag		= FALSE;
	m_nContentLength	= SIZE_UNKNOWN;
	m_sContentType.Empty();
	m_bGzip				= FALSE;
	m_bCompress			= FALSE;
	m_bDeflate			= FALSE;
	m_bChunked			= FALSE;
	m_ChunkState		= Header;
	m_nChunkLength		= SIZE_UNKNOWN;

	m_nRequests++;

	m_pSource->SetLastSeen();

	CDownloadTransfer::OnWrite();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP run handler

BOOL CDownloadTransferHTTP::OnRun()
{
	ASSUME_LOCK( Transfers.m_pSection );

	CDownloadTransfer::OnRun();

	DWORD tNow = GetTickCount();

	switch ( m_nState )
	{
	case dtsConnecting:
		if ( tNow - m_tConnected > Settings.Connection.TimeoutConnect )
		{
			theApp.Message( MSG_ERROR, IDS_CONNECTION_TIMEOUT_CONNECT, (LPCTSTR)m_sAddress );
			if ( m_pSource != NULL ) m_pSource->PushRequest();
			Close( TRI_UNKNOWN );
			return FALSE;
		}
		break;

	case dtsRequesting:
	case dtsHeaders:
		if ( tNow - m_tRequest > Settings.Connection.TimeoutHandshake )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_REQUEST_TIMEOUT, (LPCTSTR)m_sAddress );
			Close( m_bBusyFault || m_bQueueFlag ? TRI_TRUE : TRI_UNKNOWN );
			return FALSE;
		}
		break;

	case dtsDownloading:
	case dtsFlushing:
	case dtsTiger:
	case dtsMetadata:
		if ( tNow - m_mInput.tLast > Settings.Connection.TimeoutTraffic * 2 )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_TRAFFIC_TIMEOUT, (LPCTSTR)m_sAddress );
			Close( TRI_TRUE );
			return FALSE;
		}
		break;

	case dtsBusy:
		if ( tNow - m_tRequest > 1000 )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BUSY, (LPCTSTR)m_sAddress, Settings.Downloads.RetryDelay / 1000 );
			Close( TRI_TRUE );
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

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read handler

BOOL CDownloadTransferHTTP::OnRead()
{
	CDownloadTransfer::OnRead();

	switch ( m_nState )
	{
	case dtsRequesting:
		if ( ! ReadResponseLine() ) return FALSE;
		if ( m_nState != dtsHeaders ) break;

	case dtsHeaders:
		if ( ! ReadHeaders() ) return FALSE;
		if ( m_nState != dtsDownloading ) break;

	case dtsDownloading:
		return ReadContent();

	case dtsTiger:
		return ReadTiger();

	case dtsMetadata:
		return ReadMetadata();

	case dtsFlushing:
		return ReadFlush();

	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read response line

BOOL CDownloadTransferHTTP::ReadResponseLine()
{
	ASSUME_LOCK( Transfers.m_pSection );

	CString strLine, strCode, strMessage;

	if ( ! Read( strLine ) ) return TRUE;
	if ( strLine.IsEmpty() ) return TRUE;

	if ( strLine.GetLength() > HTTP_HEADER_MAX_LINE ) strLine = _T("#LINE_TOO_LONG#");

	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, _T("%s >> DOWNLOAD RESPONSE: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strLine );

	if ( strLine.GetLength() >= 12 && strLine.Left( 9 ) == _T("HTTP/1.1 ") )
	{
		strCode		= strLine.Mid( 9, 3 );
		strMessage	= strLine.Mid( 12 );
		m_bKeepAlive = TRUE;
	}
	else if ( strLine.GetLength() >= 12 && strLine.Left( 9 ) == _T("HTTP/1.0 ") )
	{
		strCode		= strLine.Mid( 9, 3 );
		strMessage	= strLine.Mid( 12 );
		m_bKeepAlive = FALSE;
	}
	else if ( strLine.GetLength() >= 8 && strLine.Left( 4 ) == _T("HTTP") )
	{
		strCode		= strLine.Mid( 5, 3 );
		strMessage	= strLine.Mid( 8 );
		theApp.Message( MSG_DEBUG, _T("HTTP with unknown version: %s"), (LPCTSTR)strLine );
		m_bKeepAlive = FALSE;
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_NOHTTP, (LPCTSTR)m_sAddress );
		Close( TRI_FALSE );
		return FALSE;
	}

	if ( strCode == _T("200") || strCode == _T("206") )
	{
		SetState( dtsHeaders );
		m_pSource->m_nFailures = 0;
		m_pSource->m_nBusyCount = 0;
	}
	else if ( strCode == _T("503") )
	{
		// 503 response without an X-Available-Ranges header means the complete file is available
		if ( _tcsistr( strMessage, _T("range") ) != NULL )
		{
			m_bRangeFault = TRUE;
		}
		else
		{
			m_bBusyFault = TRUE;
			m_pSource->m_nFailures = 0;
		}

		SetState( dtsHeaders );
	}
	else if ( strCode == _T("416") )
	{
		m_bRangeFault = TRUE;
		SetState( dtsHeaders );
	}
	else if ( strCode == _T("301") || strCode == _T("302") )
	{
		m_bRedirect = TRUE;
		SetState( dtsHeaders );
	}
	else
	{
		strMessage.TrimLeft();
		if ( strMessage.GetLength() > 128 ) strMessage = _T("No Message");
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_HTTPCODE, (LPCTSTR)m_sAddress,
			(LPCTSTR)strCode, (LPCTSTR)strMessage );
		SetState( dtsHeaders );
		m_bBadResponse = TRUE;
	}

	ClearHeaders();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read header lines

BOOL CDownloadTransferHTTP::OnHeaderLine(CString& strHeader, CString& strValue)
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( ! CDownloadTransfer::OnHeaderLine( strHeader, strValue ) )
		return FALSE;

	if ( strHeader.CompareNoCase( _T("Server") ) == 0 )
	{
		m_sUserAgent = strValue;
		m_bClientExtended = VendorCache.IsExtended( m_sUserAgent );

		if ( Security.IsAgentBlocked( m_sUserAgent ) )
		{
			Close( TRI_FALSE );
			return FALSE;
		}

		m_pSource->m_sServer = strValue;
		if ( m_pSource->m_sServer.GetLength() > 64 ) m_pSource->m_sServer = m_pSource->m_sServer.Left( 64 );

		if ( m_bClientExtended ) m_pSource->SetGnutella( 3 );
		if ( _tcsistr( m_sUserAgent, _T("trustyfiles") ) != NULL ) m_pSource->SetGnutella( 3 );
		if ( _tcsistr( m_sUserAgent, _T("gnucdna") ) != NULL ) m_pSource->SetGnutella( 3 );
		if ( _tcsistr( m_sUserAgent, _T("vagaa") ) != NULL ) m_pSource->SetGnutella( 3 );
		if ( _tcsistr( m_sUserAgent, _T("mxie") ) != NULL ) m_pSource->SetGnutella( 3 );
		if ( _tcsistr( m_sUserAgent, _T("adagio") ) != NULL ) m_pSource->SetGnutella( 2 );
	}
	else if ( strHeader.CompareNoCase( _T("Connection") ) == 0 )
	{
		if ( strValue.CompareNoCase( _T("Keep-Alive") ) == 0 ) m_bKeepAlive = TRUE;
		if ( strValue.CompareNoCase( _T("close") ) == 0 ) m_bKeepAlive = FALSE;
	}
	else if ( strHeader.CompareNoCase( _T("Content-Length") ) == 0 )
	{
		QWORD nTotal;
		if ( _stscanf( strValue, _T("%I64u"), &nTotal ) == 1 && nTotal > 0 )
		{
			m_nContentLength = nTotal;
		}
	}
	else if ( strHeader.CompareNoCase( _T("Content-Range") ) == 0 )
	{
		QWORD nFirst, nLast, nTotal;
		if ( ( _stscanf( strValue, _T("bytes %I64u-%I64u/%I64u"), &nFirst, &nLast, &nTotal ) == 3 ||
			   _stscanf( strValue, _T("bytes=%I64u-%I64u/%I64u"), &nFirst, &nLast, &nTotal ) == 3 ) &&
			   nFirst <= nLast && nTotal > 0 && nLast < nTotal )
		{
			if ( m_bTigerFetch || m_bMetaFetch )
			{
				m_nOffset = nFirst;
				m_nLength = nLast + 1 - nFirst;
				if ( m_nContentLength == SIZE_UNKNOWN ) m_nContentLength = m_nLength;
				return TRUE;
			}
			else if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
			{
				m_pDownload->SetSize( nTotal );
			}
			else if ( m_pDownload->m_nSize != nTotal )
			{
				theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_SIZE, (LPCTSTR)m_sAddress,
					(LPCTSTR)m_pDownload->GetDisplayName() );
				Close( TRI_FALSE );
				return FALSE;
			}

			if ( m_nOffset == SIZE_UNKNOWN && ! m_pDownload->GetFragment( this ) )
			{
				Close( TRI_TRUE );
				return FALSE;
			}

			BOOL bUseful = m_pDownload->IsPositionEmpty( nFirst );
			// BOOL bUseful = m_pDownload->IsRangeUseful( nFirst, nLast - nFirst + 1 );

			if ( nFirst == m_nOffset && nLast == m_nOffset + m_nLength - 1 && bUseful )
			{
				// Perfect match, good
			}
			else if ( nFirst >= m_nOffset && nFirst < m_nOffset + m_nLength && bUseful )
			{
				m_nOffset = nFirst;
				m_nLength = nLast - nFirst + 1;

				theApp.Message( MSG_INFO, IDS_DOWNLOAD_USEFUL_RANGE, (LPCTSTR)m_sAddress,
					m_nOffset, m_nOffset + m_nLength - 1, (LPCTSTR)m_pDownload->GetDisplayName() );
			}
			else
			{
				theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_RANGE, (LPCTSTR)m_sAddress,
					(LPCTSTR)m_pDownload->GetDisplayName() );
				Close( TRI_TRUE );

				return FALSE;
			}

			if ( m_nContentLength == SIZE_UNKNOWN ) m_nContentLength = m_nLength;
			m_bGotRange = TRUE;
		}
	}
	else if ( strHeader.CompareNoCase( _T("Content-Type") ) == 0 )
	{
		m_sContentType = strValue;
	}
	else if ( strHeader.CompareNoCase( _T("Content-Encoding") ) == 0 )
	{
		if ( _tcsistr( strValue, _T("backwards") ) )
		{
			m_bRecvBackwards = TRUE;
			if ( ! Settings.Downloads.AllowBackwards )
			{
				theApp.Message( MSG_DEBUG, _T( "Backwards encoding disabled" ) );
				Close( TRI_FALSE );
				return FALSE;
			}
		}
		if ( _tcsistr( strValue, _T("gzip") ) )		// gzip or x-gzip
		{
			m_bGzip = TRUE;
			theApp.Message( MSG_DEBUG, _T( "Gzip encoding not supported" ) );
			Close( TRI_FALSE );
			return FALSE;
		}
		if ( _tcsistr( strValue, _T("compress") ) )	// compress or x-compress
		{
			m_bCompress = TRUE;
			theApp.Message( MSG_DEBUG, _T( "Compress encoding not supported" ) );
			Close( TRI_FALSE );
			return FALSE;
		}
		if ( _tcsistr( strValue, _T("deflate") ) )	// deflate
		{
			m_bDeflate = TRUE;
			theApp.Message( MSG_DEBUG, _T( "Deflate encoding not supported" ) );
			Close( TRI_FALSE );
			return FALSE;
		}
	}
	else if ( strHeader.CompareNoCase( _T("Content-Language") ) == 0 )
	{
		// It would be nice to show in the future
	}
	else if ( strHeader.CompareNoCase( _T("Transfer-Encoding") ) == 0 )
	{
		if ( _tcsistr( strValue, _T("chunked") ) )
		{
			m_bChunked = TRUE;
			m_ChunkState = Header;
			m_nChunkLength = SIZE_UNKNOWN;
		}
		else
		{
			theApp.Message( MSG_DEBUG, _T( "Unknown transfer encoding: %s" ), (LPCTSTR)strValue );
			Close( TRI_FALSE );
			return FALSE;
		}
	}
	else if (	strHeader.CompareNoCase( _T("X-Gnutella-Content-URN") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Content-URN") ) == 0 ||
				strHeader.CompareNoCase( _T("Content-URN") ) == 0 )
	{
		Hashes::Sha1Hash oSHA1;
		Hashes::TigerHash oTiger;
		Hashes::Ed2kHash oED2K;
		Hashes::BtHash oBTH;
		Hashes::Md5Hash oMD5;
		CString strURNs = strValue + ',';
		for ( int nPos = strURNs.Find( ',' ); nPos >= 0; nPos = strURNs.Find( ',' ) )
		{
			strValue = strURNs.Left( nPos ).TrimLeft();
			strURNs = strURNs.Mid( nPos + 1 );

			if (   ( !oSHA1 .fromUrn( strValue ) || m_pSource->CheckHash( oSHA1  ) )
				&& ( !oTiger.fromUrn( strValue ) || m_pSource->CheckHash( oTiger ) )
				&& ( !oED2K .fromUrn( strValue ) || m_pSource->CheckHash( oED2K  ) )
				&& ( !oMD5  .fromUrn( strValue ) || m_pSource->CheckHash( oMD5   ) )
				&& ( !oBTH  .fromUrn( strValue ) || ( m_pSource->CheckHash( oBTH   ), TRUE ) ) )
			{
				continue;
			}
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_HASH, (LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
			Close( TRI_FALSE );
			return FALSE;
		}
		m_pSource->SetGnutella( 1 );
	}
	else if ( strHeader.CompareNoCase( _T("X-Metadata-Path") ) == 0 )
	{
		if ( Settings.Downloads.Metadata )
			m_sMetadata = strValue;
	}
	else if ( strHeader.CompareNoCase( _T("X-TigerTree-Path") ) == 0 )
	{
		if ( Settings.Downloads.VerifyTiger && ! m_bTigerIgnore )
		{
			if ( strValue.Find( _T("tigertree/v1") ) < 0 &&
				 strValue.Find( _T("tigertree/v2") ) < 0 )
			{
				m_sTigerTree = strValue;
			}
		}
	}
	else if ( strHeader.CompareNoCase( _T("X-Thex-URI") ) == 0 )
	{
		if ( Settings.Downloads.VerifyTiger && ! m_bTigerIgnore )
		{
			if ( strValue[ 0 ] == _T('/') )
			{
				m_sTigerTree = strValue.SpanExcluding( _T("; ") );
				m_sTigerTree.Replace( _T("ed2k=0"), _T("ed2k=1") );
			}
		}
		m_pSource->SetGnutella( 1 );
	}
	else if (	strHeader.CompareNoCase( _T("X-Gnutella-Alternate-Location") ) == 0 ||
				strHeader.CompareNoCase( _T("Alt-Location") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Alt") ) == 0 )
	{
		if ( Settings.Library.SourceMesh )
			m_pDownload->AddSourceURLs( strValue );
		m_pSource->SetGnutella( 1 );
	}
	else if ( strHeader.CompareNoCase( _T("X-Available-Ranges") ) == 0 )
	{
		m_bGotRanges = TRUE;
		m_pSource->SetAvailableRanges( strValue );
		m_pSource->SetGnutella( 1 );
		if ( m_pSource->m_oAvailable.empty() )
		{
			theApp.Message( MSG_DEBUG, _T( "header did not include valid ranges, dropping source..." ) );
			Close( TRI_FALSE );
			return FALSE;
		}
	}
	else if ( strHeader.CompareNoCase( _T("X-Queue") ) == 0 )
	{
		m_pSource->SetGnutella( 1 );

		m_bQueueFlag = TRUE;
		ToLower( strValue );

		int nPos = strValue.Find( _T("position=") );
		if ( nPos >= 0 ) _stscanf( strValue.Mid( nPos + 9 ), _T("%lu"), &m_nQueuePos );

		nPos = strValue.Find( _T("length=") );
		if ( nPos >= 0 ) _stscanf( strValue.Mid( nPos + 7 ), _T("%lu"), &m_nQueueLen );

		DWORD nLimit = 0;

		nPos = strValue.Find( _T("pollmin=") );
		if ( nPos >= 0 && _stscanf( strValue.Mid( nPos + 8 ), _T("%lu"), &nLimit ) == 1 )
		{
			m_nRetryDelay = max( m_nRetryDelay, nLimit * 1000 + 3000  );
		}

		nPos = strValue.Find( _T("pollmax=") );
		if ( nPos >= 0 && _stscanf( strValue.Mid( nPos + 8 ), _T("%lu"), &nLimit ) == 1 )
		{
			m_nRetryDelay = min( m_nRetryDelay, nLimit * 1000 - 8000 );
		}

		nPos = strValue.Find( _T("id=") );
		if ( nPos >= 0 )
		{
			m_sQueueName = strValue.Mid( nPos + 3 );
			m_sQueueName.TrimLeft();
			if ( m_sQueueName.Find( '\"' ) == 0 )
			{
				m_sQueueName = m_sQueueName.Mid( 1 ).SpanExcluding( _T("\"") );
			}
			else
			{
				m_sQueueName = m_sQueueName.SpanExcluding( _T("\" ") );
			}
			if ( m_sQueueName == _T("s") ) m_sQueueName = _T("Small Queue");
			else if ( m_sQueueName == _T("l") ) m_sQueueName = _T("Large Queue");
		}
	}
	else if ( strHeader.CompareNoCase( _T("Retry-After") ) == 0 )
	{
		DWORD nLimit = 0;

		if ( _stscanf( strValue, _T("%lu"), &nLimit ) == 1 )
		{
			m_nRetryAfter = nLimit;
		}
	}
	else if (	strHeader.CompareNoCase( _T("X-PerHost") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Gnutella-maxSlotsPerHost") ) == 0 )
	{
		DWORD nLimit = 0;

		if ( _stscanf( strValue, _T("%lu"), &nLimit ) != 1 )
		{
			Downloads.SetPerHostLimit( &m_pHost.sin_addr, nLimit );
		}
	}
	else if ( strHeader.CompareNoCase( _T("X-Delete-Source") ) == 0 )
	{
		m_bBadResponse = TRUE;
	}
	else if (	strHeader.CompareNoCase( _T("X-Nick") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Name") ) == 0 ||
				strHeader.CompareNoCase( _T("X-UserName") ) == 0 )
	{
		m_pSource->m_sNick = URLDecode( strValue );
	}
	else if ( strHeader.CompareNoCase( _T("X-Features") ) == 0 )
	{
		if ( _tcsistr( strValue, _T("g2/") ) != NULL ) m_pSource->SetGnutella( 2 );
		if ( _tcsistr( strValue, _T("gnet2/") ) != NULL ) m_pSource->SetGnutella( 2 );
		if ( _tcsistr( strValue, _T("gnutella2/") ) != NULL ) m_pSource->SetGnutella( 2 );
		m_pSource->SetGnutella( 1 );
	}
	else if ( strHeader.CompareNoCase( _T("Location") ) == 0 )
	{
		m_sRedirectionURL = strValue;
		m_pDownload->SetStableName( false );
	}
	else if ( strHeader.CompareNoCase( _T("X-NAlt") ) == 0 ||
			  strHeader.CompareNoCase( _T("X-PAlt") ) == 0 ||
			  strHeader.CompareNoCase( _T("FP-1a") ) == 0 ||
			  strHeader.CompareNoCase( _T("FP-Auth-Challenge") ) == 0 )
	{
		m_pSource->SetGnutella( 1 );
	}
	else if ( strHeader.CompareNoCase( _T("Content-Disposition") ) == 0 )
	{
		if ( ! m_pDownload->HasStableName() )
		{
			const int nPos = strValue.Find( _T("filename=") );
			if ( nPos >= 0 )
			{
				// If exactly, it should follow RFC 2184 rules
				const CString strFilename = URLDecode( strValue.Mid( nPos + 9 ).Trim( _T("\" \t\r\n") ) );
				if ( m_pDownload->Rename( strFilename ) )
				{
					m_pDownload->SetStableName();
					DownloadGroups.Link( m_pDownload );
				}
			}
		}
	}
	else if ( strHeader.CompareNoCase( _T("Content-MD5") ) == 0 )
	{
		Hashes::Md5Hash oMD5;
		oMD5.fromString( strValue );
		theApp.Message( MSG_DEBUG, _T("Content-MD5: %s"), (LPCTSTR)oMD5.toString() );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP end of headers

BOOL CDownloadTransferHTTP::OnHeadersComplete()
{
	// Close parameters:
	// TRI_FALSE   - the source will be added to m_pFailedSources in CDownloadWithSources,
	//			    removed from the sources and can be distributed in the Source Mesh as X-Nalt
	// TRI_TRUE    - keeps the source and will be distributed as X-Alt
	// TRI_UNKNOWN - keeps the source and will be dropped after several retries, will be
	//            - added to m_pFailedSources when removed

	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_bBadResponse )
	{
		Close( TRI_FALSE );
		return FALSE;
	}
	else if ( m_bRedirect )
	{
		CShareazaURL pURL( m_sRedirectionURL );
		m_pDownload->AddSourceHit( pURL, TRUE, m_pSource->m_nRedirectionCount + 1 );
		Close( TRI_FALSE );
		return FALSE;
	}
	else if ( ! m_pSource->CanInitiate( TRUE, TRUE ) )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DISABLED,
			(LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
		Close( m_pSource->m_bED2K ? TRI_FALSE : TRI_UNKNOWN );
		return FALSE;
	}
	else if ( m_bBusyFault )
	{
		m_nOffset = SIZE_UNKNOWN;

		if ( Settings.Downloads.QueueLimit > 0 && m_nQueuePos > Settings.Downloads.QueueLimit )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_QUEUE_HUGE,
				(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName(), m_nQueuePos );
			Close( TRI_FALSE );
			return FALSE;
		}
		else if ( m_bQueueFlag && m_nRetryDelay >= 600000 )
		{
			m_pSource->m_tAttempt = GetTickCount() + m_nRetryDelay;
			m_bQueueFlag = FALSE;
		}

		if ( m_bQueueFlag )
		{
			SetState( dtsFlushing );
			m_tContent = m_mInput.tLast = GetTickCount();
			return ReadFlush();
		}
		else
		{
			SetState( dtsBusy );
			m_pSource->m_nBusyCount++;
			m_tRequest = GetTickCount();
			return TRUE;
		}
	}
	else if ( ! m_bGotRanges && ! m_bTigerFetch && ! m_bMetaFetch )
	{
		m_pSource->SetAvailableRanges( NULL );
	}

	if ( m_bRangeFault )
	{
		if ( Network.IsSelfIP( m_pHost.sin_addr ) )
		{
			Close( TRI_TRUE );
			return FALSE;
		}

		m_nOffset = SIZE_UNKNOWN;
		SetState( dtsFlushing );
		m_tContent = m_mInput.tLast = GetTickCount();

		return ReadFlush();
	}
	else if ( m_bTigerFetch )
	{
		if ( m_nContentLength == SIZE_UNKNOWN && !m_bKeepAlive )
		{
            // This should fix the PHEX TTH problem with closed connection.
            SetState( dtsTiger );
            theApp.Message( MSG_INFO, IDS_DOWNLOAD_TIGER_RECV, (LPCTSTR)m_sAddress,
                (LPCTSTR)m_pSource->m_sServer );

            return ReadTiger(); // doesn't actually read but updates timings
		}
		if ( ! m_bGotRange )
		{
			m_nOffset = 0;
			m_nLength = m_nContentLength;
		}
		else if ( m_nOffset > 0 )
		{
			theApp.Message( MSG_INFO, IDS_DOWNLOAD_TIGER_RANGE, (LPCTSTR)m_sAddress );
			Close( TRI_FALSE );
			return FALSE;
		}

		if (	m_sContentType.CompareNoCase( _T("application/tigertree-breadthfirst") ) &&
				m_sContentType.CompareNoCase( _T("application/dime") ) &&
				m_sContentType.CompareNoCase( _T("application/binary") ) ) // Content Type used by Phex
		{
			theApp.Message( MSG_INFO, IDS_DOWNLOAD_TIGER_RANGE, (LPCTSTR)m_sAddress );
			Close( TRI_TRUE );
			return FALSE;
		}

		SetState( dtsTiger );
		m_tContent = m_mInput.tLast = GetTickCount();

		theApp.Message( MSG_INFO, IDS_DOWNLOAD_TIGER_RECV, (LPCTSTR)m_sAddress,
			(LPCTSTR)m_pSource->m_sServer );

		return ReadTiger();
	}
	else if ( m_bMetaFetch )
	{
		if ( ! m_bGotRange )
		{
			m_nOffset = 0;
			m_nLength = m_nContentLength;
		}

		SetState( dtsMetadata );
		m_tContent = m_mInput.tLast = GetTickCount();

		theApp.Message( MSG_INFO, IDS_DOWNLOAD_METADATA_RECV,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_pSource->m_sServer );

		return ReadMetadata();
	}
	else if ( ! m_bGotRange )
	{
		if ( m_nContentLength != SIZE_UNKNOWN )
		{
			if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
			{
				m_pDownload->SetSize( m_nContentLength );
			}
			else if ( m_pDownload->m_nSize != m_nContentLength )
			{
				theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_SIZE, (LPCTSTR)m_sAddress,
					(LPCTSTR)m_pDownload->GetDisplayName() );
				Close( TRI_FALSE );
				return FALSE;
			}
		}
		if ( m_nOffset == SIZE_UNKNOWN && ! m_pDownload->GetFragment( this ) )
		{
			Close( TRI_TRUE );
			return FALSE;
		}

		if ( ! m_pDownload->IsPositionEmpty( 0 ) )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_RANGE, (LPCTSTR)m_sAddress,
				(LPCTSTR)m_pDownload->GetDisplayName() );
			Close( TRI_TRUE );
			return FALSE;
		}

		m_nOffset = 0;
		m_nLength = m_nContentLength;
	}
	else if ( CFailedSource* pBadSource = m_pDownload->LookupFailedSource( m_pSource->m_sURL ) )
	{
		// We already have it added to the list but the source was offline
		if ( pBadSource->m_bOffline )
		{
			pBadSource->m_bOffline = false;
		}
		else
		{
			// Extend the period of keeping it in the failed sources list
			pBadSource->m_nTimeAdded = GetTickCount();
			Close( TRI_FALSE );
			return FALSE;
		}
	}

	if ( m_nContentLength != m_nLength )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_RANGE, (LPCTSTR)m_sAddress,
			(LPCTSTR)m_pDownload->GetDisplayName() );
		Close( TRI_FALSE );
		return FALSE;
	}

	if ( ! m_bKeepAlive ) m_pSource->m_bCloseConn = TRUE;

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_CONTENT, (LPCTSTR)m_sAddress,
		(LPCTSTR)m_pSource->m_sServer );

	SetState( dtsDownloading );
	if ( ! m_pDownload->IsBoosted() )
		m_mInput.pLimit = m_mOutput.pLimit = &m_nBandwidth;
	m_nPosition = 0;
	m_tContent = m_mInput.tLast = GetTickCount();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read content

BOOL CDownloadTransferHTTP::ReadContent()
{
	ASSUME_LOCK( Transfers.m_pSection );

	CLockedBuffer pInput( GetInput() );

	while ( pInput->m_nLength > 0 )
	{
		m_pSource->SetValid();

		QWORD nLength	= min( (QWORD)pInput->m_nLength, m_nLength - m_nPosition );
		BOOL bSubmit	= FALSE;

		if ( m_bChunked )
		{
			BOOL bBreak	= FALSE;
			switch( m_ChunkState )
			{
			case Header:
				if ( pInput->m_nLength >= 3 )
				{
					// Looking for "Length<CR><LF>"
					DWORD i = 1;
					for ( ; i < pInput->m_nLength - 1; i++ )
					{
						if ( pInput->m_pBuffer[ i ]     == 0x0d &&
							 pInput->m_pBuffer[ i + 1 ] == 0x0a )
						{
							break;
						}
					}
					if ( i < pInput->m_nLength - 1 )
					{
						if ( sscanf_s( (LPCSTR)pInput->m_pBuffer, "%I64x",
							&m_nChunkLength ) == 1 )
						{
							if ( m_nChunkLength == 0 )
							{
								// Got last chunk "0<CR><LF>"
								m_ChunkState = Footer;

								// Now file size is known
								if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
								{
									m_pDownload->SetSize( m_nDownloaded );
									m_pDownload->MakeComplete();
								}
							}
							else
								m_ChunkState = Body;

							// Cut header
							pInput->Remove( i + 2 );

							// Process rest of data
							continue;
						}
						else
						{
							// Bad format
							pInput->Clear();
							Close( TRI_FALSE );
							return FALSE;
						}
					}
				}
				bBreak = TRUE;
				break;

			case Body:
				ASSERT( m_nChunkLength != 0 );
				ASSERT( m_nChunkLength != SIZE_UNKNOWN );

				if ( nLength > m_nChunkLength )
				{
					// m_nChunkLength must be smaller than nLength so this cast
					// is safe
					nLength = m_nChunkLength;
				}
				m_nChunkLength -= nLength;
				if ( m_nChunkLength == 0 )
				{
					// Whole chunk readed
					m_ChunkState = BodyEnd;
					m_nChunkLength = SIZE_UNKNOWN;
				}
				break;

			case BodyEnd:
				ASSERT( m_nChunkLength == SIZE_UNKNOWN );

				// Looking for "<CR><LF>"
				if ( pInput->m_nLength >= 2 )
				{
					if ( pInput->m_pBuffer[ 0 ] == 0x0d &&
						 pInput->m_pBuffer[ 1 ] == 0x0a )
					{
						m_ChunkState = Header;

						// Cut <CR><LF>
						pInput->Remove( 2 );

						// Process rest of data
						continue;
					}
					else
					{
						// Bad Format
						pInput->Clear();
						Close( TRI_FALSE );
						return FALSE;
					}
				}
				bBreak = TRUE;
				break;

			case Footer:
				ASSERT( m_nChunkLength == 0 );

				// Bypass footer
				pInput->Clear();
				bBreak = TRUE;
				break;
			}
			if ( bBreak )
				break;
		}

		if ( m_bRecvBackwards )
		{
			CAutoVectorPtr< BYTE >pBuffer( new BYTE[ (size_t)nLength ] );
			if ( ! pBuffer )
			{
				// Out of memory
				Close( TRI_TRUE );
				return FALSE;
			}
			CBuffer::ReverseBuffer( pInput->m_pBuffer, pBuffer, (size_t)nLength );
			bSubmit = m_pDownload->SubmitData( m_nOffset + m_nLength - m_nPosition - nLength, pBuffer, nLength );
		}
		else
		{
			bSubmit = m_pDownload->SubmitData(
				m_nOffset + m_nPosition, pInput->m_pBuffer, nLength );
		}

		if ( m_bChunked )
			pInput->Remove( (size_t)nLength );
		else
			pInput->Clear();

		m_nPosition += nLength;
		m_nDownloaded += nLength;

		if ( ! bSubmit )
		{
			BOOL bUseful = m_pDownload->IsRangeUsefulEnough( this,
				m_bRecvBackwards ? m_nOffset : m_nOffset + m_nPosition,
				m_nLength - m_nPosition );
			if ( /* m_bInitiated || */ ! bUseful )
			{
				theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_OVERLAP, (LPCTSTR)m_sAddress );
				Close( TRI_TRUE );
				return FALSE;
			}
		}
	}

	if ( m_nPosition >= m_nLength )
	{
		m_pSource->AddFragment( m_nOffset, m_nLength );
		return StartNextFragment();
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read Metadata

BOOL CDownloadTransferHTTP::ReadMetadata()
{
	CLockedBuffer pInput( GetInput() );

	if ( pInput->m_nLength < m_nLength ) return TRUE;

	CString strXML = pInput->ReadString( (DWORD)m_nLength, CP_UTF8 );

	if ( CXMLElement* pXML = CXMLElement::FromString( strXML, TRUE ) )
	{
		m_pDownload->MergeMetadata( pXML );
		delete pXML;
	}

	pInput->Remove( (DWORD)m_nLength );

	return StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read tiger tree

BOOL CDownloadTransferHTTP::ReadTiger(bool bDropped)
{
    // It is a fix for very slow DIME uploads, they get dropped while downloading (e.g. LimeWire).
    m_tContent = m_mInput.tLast = GetTickCount();

    // Fix for PHEX TTH which never tell content length for DIME block to get into DIME decoding
    // until the connection drops, if no content length specified and not keep-alive.
    if ( !m_bKeepAlive && m_nContentLength == SIZE_UNKNOWN ) return TRUE;

	CLockedBuffer pInput( GetInput() );

    if ( pInput->m_nLength < m_nLength ) return TRUE;

	if ( m_sContentType.CompareNoCase( _T("application/tigertree-breadthfirst") ) == 0 )
	{
		m_pDownload->SetTigerTree( pInput->m_pBuffer, (DWORD)m_nLength );
		pInput->Remove( (DWORD)m_nLength );
	}
	else if ( m_sContentType.CompareNoCase( _T("application/dime") ) == 0 ||
			  m_sContentType.CompareNoCase( _T("application/binary") ) == 0 )
	{
		CString strID, strType, strUUID = _T("x");
		DWORD nFlags, nBody;

		while ( pInput->ReadDIME( &nFlags, &strID, &strType, &nBody ) )
		{
			theApp.Message( MSG_DEBUG, _T("THEX DIME: %i, '%s', '%s', %i"),
				nFlags, (LPCTSTR)strID, (LPCTSTR)strType, nBody );

			if ( ( nFlags & 1 ) && strType.CompareNoCase( _T("text/xml") ) == 0 && nBody < 1024*1024 )
			{
				BOOL bSize = FALSE, bDigest = FALSE, bEncoding = FALSE;
				CString strXML;

				strXML = pInput->ReadString( nBody, CP_UTF8 );

				if ( CXMLElement* pXML = CXMLElement::FromString( strXML ) )
				{
					if ( pXML->IsNamed( _T("hashtree") ) )
					{
						if ( CXMLElement* pxFile = pXML->GetElementByName( _T("file") ) )
						{
							QWORD nSize = 0;
							_stscanf( pxFile->GetAttributeValue( _T("size") ), _T("%I64u"), &nSize );
							bSize = ( nSize == m_pDownload->m_nSize );
						}
						if ( CXMLElement* pxDigest = pXML->GetElementByName( _T("digest") ) )
						{
							if ( pxDigest->GetAttributeValue( _T("algorithm") ).CompareNoCase( _T("http://open-content.net/spec/digest/tiger") ) == 0 )
							{
								bDigest = ( pxDigest->GetAttributeValue( _T("outputsize") ) == _T("24") );
							}
						}
						if ( CXMLElement* pxTree = pXML->GetElementByName( _T("serializedtree") ) )
						{
							bEncoding = ( pxTree->GetAttributeValue( _T("type") ).CompareNoCase( _T("http://open-content.net/spec/thex/breadthfirst") ) == 0 );
							strUUID = pxTree->GetAttributeValue( _T("uri") );
						}
					}
					delete pXML;
				}

				theApp.Message( MSG_DEBUG, _T("THEX XML: size=%i, digest=%i, encoding=%i"),
					bSize, bDigest, bEncoding );

				if ( ! bSize || ! bDigest || ! bEncoding ) break;
			}
			else if ( ( strID == strUUID || strID.IsEmpty() ) && strType.CompareNoCase( _T("http://open-content.net/spec/thex/breadthfirst") ) == 0 )
			{
				m_pDownload->SetTigerTree( pInput->m_pBuffer, nBody );
			}
			else if ( strType.CompareNoCase( _T("http://edonkey2000.com/spec/md4-hashset") ) == 0 )
			{
				m_pDownload->SetHashset( pInput->m_pBuffer, nBody );
			}

			pInput->Remove( ( nBody + 3 ) & ~3 );
		}

		pInput->Clear();
	}

    // m_bKeepAlive == FALSE means that it was not keep-alive, so should just get disconnected.
    // after reading of DIME message
    // This might be better with returning FALSE because it is not keep alive connection
    // need to disconnect after the business
    if ( bDropped || ! m_bKeepAlive )
		return TRUE;

	return StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read flushing

BOOL CDownloadTransferHTTP::ReadFlush()
{
	CLockedBuffer pInput( GetInput() );

	if ( m_nContentLength == SIZE_UNKNOWN ) m_nContentLength = 0;

	QWORD nRemove = min( (QWORD)pInput->m_nLength, m_nContentLength );
	m_nContentLength -= nRemove;

	pInput->Remove( (size_t)nRemove );

	if ( m_nContentLength == 0 )
	{
		if ( m_bQueueFlag )
		{
			SetState( dtsQueued );
			if ( ! m_pDownload->IsBoosted() )
				m_mInput.pLimit = m_mOutput.pLimit = &Settings.Bandwidth.Request;
			m_tRequest = GetTickCount() + m_nRetryDelay;

			theApp.Message( MSG_INFO, IDS_DOWNLOAD_QUEUED,
				(LPCTSTR)m_sAddress, m_nQueuePos, m_nQueueLen,
				(LPCTSTR)m_sQueueName );
		}
		else if ( m_bRangeFault && !m_bGotRanges )
        {
			/* we got a "requested range unavailable" error but the source doesn't
			advertise available ranges; don't start to guess, try again later */
			theApp.Message( MSG_INFO, IDS_DOWNLOAD_416_WITHOUT_RANGE, (LPCTSTR)m_sAddress );
			Close( TRI_TRUE );
			return FALSE;
        }
		else if ( m_bRangeFault && m_bGotRanges && m_nRequests >= 2 )
		{
			/* we made two requests already and the source does advertise available
            ranges, but we still managed to request a wrong one */
			// TODO: find the reason why this is happening
			theApp.Message( MSG_ERROR, _T("BUG: Shareaza requested a fragment from host %s, although it knew that the host doesn't have that fragment") , (LPCTSTR)m_sAddress );
			Close( TRI_TRUE );
			return FALSE;
		}
		else
		{
			return StartNextFragment();
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP dropped connection handler

void CDownloadTransferHTTP::OnDropped()
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_nState == dtsConnecting )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CONNECT_ERROR, (LPCTSTR)m_sAddress );
		if ( m_pSource != NULL ) m_pSource->PushRequest();
		Close( TRI_UNKNOWN );
	}
	else if ( m_nState == dtsBusy )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BUSY, (LPCTSTR)m_sAddress, Settings.Downloads.RetryDelay / 1000 );
		Close( TRI_TRUE, m_nRetryAfter );
	}
	else if ( m_nState == dtsTiger )
    {
        // this is basically for PHEX DIME download
        theApp.Message( MSG_DEBUG, _T("Reading THEX from the closed connection...") );
		// It was closed connection with no content length, so assume the content length is equal to the
		// size of buffer when the connection gets cut. It is important to set it because the DIME decoding
		// code check if the content length is equals to size of buffer.
		m_nLength = m_nContentLength = GetInputLength();
		ReadTiger( true );
		// CDownloadTransfer::Close will resume the closed connection
        if ( m_pSource )
			m_pSource->m_bCloseConn = TRUE;
		Close( TRI_TRUE );
    }
	else if ( m_bBusyFault || m_bQueueFlag )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BUSY, (LPCTSTR)m_sAddress, Settings.Downloads.RetryDelay / 1000 );
		Close( TRI_TRUE, m_nRetryAfter );
	}
	else if ( m_nState == dtsDownloading && m_nContentLength == SIZE_UNKNOWN &&
		m_pDownload->m_nSize == SIZE_UNKNOWN )
	{
		// Set file size as is
		m_pDownload->SetSize( m_nDownloaded );
		m_pDownload->MakeComplete();
		Close( TRI_TRUE );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DROPPED, (LPCTSTR)m_sAddress );
		Close( m_nState >= dtsDownloading ? TRI_TRUE : TRI_UNKNOWN );
	}
}

