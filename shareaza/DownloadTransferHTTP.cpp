//
// DownloadTransferHTTP.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "Download.h"
#include "Downloads.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadTransferHTTP.h"
#include "FragmentedFile.h"
#include "Network.h"
#include "Buffer.h"
#include "SourceURL.h"
#include "GProfile.h"
#include "SHA.h"
#include "ED2K.h"
#include "TigerTree.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP construction

CDownloadTransferHTTP::CDownloadTransferHTTP(CDownloadSource* pSource) : CDownloadTransfer( pSource, PROTOCOL_HTTP )
{
	m_nRequests		= 0;
	m_tContent		= 0;
	
	m_bBadResponse	= FALSE;
	m_bBusyFault	= FALSE;
	m_bRangeFault	= FALSE;
	m_bHashMatch	= FALSE;
	m_bTigerFetch	= FALSE;
	m_bTigerIgnore	= FALSE;
	m_bMetaFetch	= FALSE;
	m_bMetaIgnore	= FALSE;
	
	m_nRetryDelay	= Settings.Downloads.RetryDelay;
}

CDownloadTransferHTTP::~CDownloadTransferHTTP()
{
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP initiate connection

BOOL CDownloadTransferHTTP::Initiate()
{
	theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_CONNECTING,
		(LPCTSTR)CString( inet_ntoa( m_pSource->m_pAddress ) ), m_pSource->m_nPort,
		(LPCTSTR)m_pDownload->GetDisplayName() );
	
	if ( ConnectTo( &m_pSource->m_pAddress, m_pSource->m_nPort ) )
	{
		SetState( dtsConnecting );
		
		if ( ! m_pDownload->IsBoosted() )
			m_mInput.pLimit = m_mOutput.pLimit = &Downloads.m_nLimitGeneric;
		
		return TRUE;
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CONNECT_ERROR, (LPCTSTR)m_sAddress );
		Close( TS_UNKNOWN );
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP accept push

BOOL CDownloadTransferHTTP::AcceptPush(CConnection* pConnection)
{
	AttachTo( pConnection );
	
	theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_PUSHED, (LPCTSTR)m_sAddress,
		(LPCTSTR)m_pDownload->GetDisplayName() );
	
	if ( ! m_pDownload->IsBoosted() )
		m_mInput.pLimit = m_mOutput.pLimit = &Downloads.m_nLimitGeneric;
	
	if ( StartNextFragment() ) return TRUE;
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP close

void CDownloadTransferHTTP::Close(TRISTATE bKeepSource)
{
	if ( m_pSource != NULL && m_nState == dtsDownloading )
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
	
	CDownloadTransfer::Close( bKeepSource );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP speed controls

void CDownloadTransferHTTP::Boost()
{
	m_mInput.pLimit = m_mOutput.pLimit = NULL;
}

DWORD CDownloadTransferHTTP::GetAverageSpeed()
{
	if ( m_nState == dtsDownloading )
	{
		DWORD nTime = ( GetTickCount() - m_tContent ) / 1000;
		if ( nTime > 0 ) m_pSource->m_nSpeed = (DWORD)( m_nPosition / nTime );
	}
	
	return m_pSource->m_nSpeed;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP connection handler

BOOL CDownloadTransferHTTP::OnConnected()
{
	theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_CONNECTED, (LPCTSTR)m_sAddress );
	
	m_tConnected = GetTickCount();
	
	return StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP fragment allocation

BOOL CDownloadTransferHTTP::StartNextFragment()
{
	ASSERT( this != NULL );
	if ( this == NULL ) return FALSE;

	m_nOffset			= SIZE_UNKNOWN;
	m_nPosition			= 0;
	m_bWantBackwards	= FALSE;
	m_bRecvBackwards	= FALSE;
	m_bTigerFetch		= FALSE;
	m_bMetaFetch		= FALSE;
	
	if ( m_pInput == NULL || m_pOutput == NULL /* ||
		 m_pDownload->GetTransferCount( dtsDownloading ) >= Settings.Downloads.MaxFileTransfers */ )
	{
		theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_CLOSING_EXTRA, (LPCTSTR)m_sAddress );
		Close( TS_TRUE );
		return FALSE;
	}
	
	// this needs to go for pipeline
	
	if ( m_pInput->m_nLength > 0 && m_nRequests > 0 )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CLOSING_OVERFLOW, (LPCTSTR)m_sAddress );
		Close( TS_TRUE );
		return FALSE;
	}
	
	if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
	{
		return SendRequest();
	}
	else if ( m_pDownload->NeedTigerTree() && m_sTigerTree.GetLength() )
	{
		theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_TIGER_REQUEST,
			(LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress );
		
		m_bTigerFetch	= TRUE;
		m_bTigerIgnore	= TRUE;
		
		return SendRequest();
	}
	else if ( m_pDownload->m_pXML == NULL && m_sMetadata.GetLength() )
	{
		theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_METADATA_REQUEST,
			(LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress );
		
		m_bMetaFetch	= TRUE;
		m_bMetaIgnore	= TRUE;
		
		return SendRequest();
	}
	else if ( m_pDownload->GetFragment( this ) )
	{
		ChunkifyRequest( &m_nOffset, &m_nLength, Settings.Downloads.ChunkSize, TRUE );
		
		theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_FRAGMENT_REQUEST,
			m_nOffset, m_nOffset + m_nLength - 1,
			(LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress );
		
		return SendRequest();
	}
	else
	{
		if ( m_pSource != NULL ) m_pSource->SetAvailableRanges( NULL );
		
		theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_FRAGMENT_END, (LPCTSTR)m_sAddress );
		Close( TS_TRUE );
		
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP subtract pending requests

BOOL CDownloadTransferHTTP::SubtractRequested(FF::SimpleFragmentList& ppFragments)
{
	if ( m_nOffset < SIZE_UNKNOWN && m_nLength < SIZE_UNKNOWN )
	{
		if ( m_nState == dtsRequesting || m_nState == dtsDownloading )
		{
            ppFragments.erase( FF::SimpleFragment( m_nOffset, m_nOffset + m_nLength ) );
			return TRUE;
		}
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP send request

BOOL CDownloadTransferHTTP::SendRequest()
{
	CString strLine;
	
	CSourceURL pURL;
	if ( ! pURL.ParseHTTP( m_pSource->m_sURL, FALSE ) ) return FALSE;
	
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
		m_pOutput->Print( strLine );
		
		strLine.Format( _T("Host: %s\r\n"), (LPCTSTR)pURL.m_sAddress );
		m_pOutput->Print( strLine );
	}
	else
	{
		strLine.Format( _T("GET %s HTTP/1.0\r\n"), (LPCTSTR)pURL.m_sPath );
	}
	
	theApp.Message( MSG_DEBUG, _T("%s: DOWNLOAD REQUEST: %s"),
		(LPCTSTR)m_sAddress, (LPCTSTR)pURL.m_sPath );
	
	m_pOutput->Print( "Connection: Keep-Alive\r\n" ); //BearShare assumes close

	if ( Settings.Gnutella2.EnableToday ) m_pOutput->Print( "X-Features: g2/1.0\r\n" );
	
	if ( m_bTigerFetch )
	{
		m_pOutput->Print( "Accept: application/dime, application/tigertree-breadthfirst\r\n" );
	}
	else if ( m_bMetaFetch )
	{
		m_pOutput->Print( "Accept: text/xml\r\n" );
	}
	
	if ( m_nOffset != SIZE_UNKNOWN && ! m_bTigerFetch && ! m_bMetaFetch )
	{
		if ( m_nOffset + m_nLength == m_pDownload->m_nSize )
		{
			strLine.Format( _T("Range: bytes=%I64i-\r\n"), m_nOffset );
		}
		else
		{
			strLine.Format( _T("Range: bytes=%I64i-%I64i\r\n"), m_nOffset, m_nOffset + m_nLength - 1 );
		}
		m_pOutput->Print( strLine );
	}
	else
	{
		m_pOutput->Print( "Range: bytes=0-\r\n" );
	}
	
	if ( m_bWantBackwards && Settings.Downloads.AllowBackwards )
	{
		m_pOutput->Print( "Accept-Encoding: backwards\r\n" );
	}
	
	strLine = Settings.SmartAgent( Settings.General.UserAgent );
	
	if ( strLine.GetLength() )
	{
		strLine = _T("User-Agent: ") + strLine + _T("\r\n");
		m_pOutput->Print( strLine );
	}
	
	if ( m_nRequests == 0 )
	{
		if ( m_bInitiated ) SendMyAddress();
		
		strLine = MyProfile.GetNick().Left( 255 );
		
		if ( strLine.GetLength() > 0 )
		{
			strLine = _T("X-Nick: ") + URLEncode( strLine ) + _T("\r\n");
			m_pOutput->Print( strLine );
		}
	}
	
	if ( m_pSource->m_nPort == INTERNET_DEFAULT_HTTP_PORT )
	{
		int nSlash = m_pSource->m_sURL.ReverseFind( '/' );
		if ( nSlash > 0 )
		{
			strLine = _T("Referrer: ") + m_pSource->m_sURL.Left( nSlash + 1 ) + _T("\r\n");
			m_pOutput->Print( strLine );
		}
	}
	
	m_pOutput->Print( "X-Queue: 0.1\r\n" );
	
	if ( m_pSource->m_bSHA1 && Settings.Library.SourceMesh && ! m_bTigerFetch && ! m_bMetaFetch )
	{
		CString strURN = CSHA::HashToString( &m_pDownload->m_pSHA1, TRUE );
		
		m_pOutput->Print( "X-Content-URN: " );
		m_pOutput->Print( strURN + _T("\r\n") );
		
		strLine = m_pDownload->GetSourceURLs( &m_pSourcesSent, 15, TRUE, m_pSource );
		
		if ( strLine.GetLength() )
		{
			m_pOutput->Print( "Alt-Location: " );
			m_pOutput->Print( strLine + _T("\r\n") );
		}
		
		if ( m_pDownload->IsShared() && m_pDownload->IsStarted() && Network.IsStable() )
		{
			strLine.Format( _T("http://%s:%i/uri-res/N2R?%s "),
				(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
				htons( Network.m_pHost.sin_port ),
				(LPCTSTR)strURN );
			strLine += TimeToString( time( NULL ) - 180 );
			m_pOutput->Print( "Alt-Location: " );
			m_pOutput->Print( strLine + _T("\r\n") );
		}
	}
	
	m_pOutput->Print( "\r\n" );
	
	SetState( dtsRequesting );
	m_tRequest			= GetTickCount();
	m_bBusyFault		= FALSE;
	m_bRangeFault		= FALSE;
	m_bKeepAlive		= FALSE;
	m_bHashMatch		= FALSE;
	m_bGotRange			= FALSE;
	m_bGotRanges		= FALSE;
	m_bQueueFlag		= FALSE;
	m_nContentLength	= SIZE_UNKNOWN;
	m_sContentType.Empty();
	
	m_sTigerTree.Empty();
	m_nRequests++;
	
	m_pSource->SetLastSeen();
	
	CDownloadTransfer::OnWrite();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP run handler

BOOL CDownloadTransferHTTP::OnRun()
{
	CDownloadTransfer::OnRun();
	
	DWORD tNow = GetTickCount();
	
	switch ( m_nState )
	{
	case dtsConnecting:
		if ( tNow - m_tConnected > Settings.Connection.TimeoutConnect )
		{
			theApp.Message( MSG_ERROR, IDS_CONNECTION_TIMEOUT_CONNECT, (LPCTSTR)m_sAddress );
			if ( m_pSource != NULL ) m_pSource->PushRequest();
			Close( TS_UNKNOWN );
			return FALSE;
		}
		break;

	case dtsRequesting:
	case dtsHeaders:
		if ( tNow - m_tRequest > Settings.Connection.TimeoutHandshake )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_REQUEST_TIMEOUT, (LPCTSTR)m_sAddress );
			Close( m_bBusyFault || m_bQueueFlag ? TS_TRUE : TS_UNKNOWN );
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
			Close( TS_TRUE );
			return FALSE;
		}
		break;

	case dtsBusy:
		if ( tNow - m_tRequest > 1000 )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BUSY, (LPCTSTR)m_sAddress, Settings.Downloads.RetryDelay / 1000 );
			Close( TS_TRUE );
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
	CString strLine, strCode, strMessage;
	
	if ( ! m_pInput->ReadLine( strLine ) ) return TRUE;
	if ( strLine.IsEmpty() ) return TRUE;

	if ( strLine.GetLength() > 512 ) strLine = _T("#LINE_TOO_LONG#");
	
	theApp.Message( MSG_DEBUG, _T("%s: DOWNLOAD RESPONSE: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strLine );
	
	if ( strLine.GetLength() >= 12 && strLine.Left( 9 ) == _T("HTTP/1.1 ") )
	{
		strCode		= strLine.Mid( 9, 3 );
		strMessage	= strLine.Mid( 12 );
	}
	else if ( strLine.GetLength() >= 12 && strLine.Left( 9 ) == _T("HTTP/1.0 ") )
	{
		strCode		= strLine.Mid( 9, 3 );
		strMessage	= strLine.Mid( 12 );
	}
	else if ( strLine.GetLength() >= 8 && strLine.Left( 4 ) == _T("HTTP") )
	{
		strCode		= strLine.Mid( 5, 3 );
		strMessage	= strLine.Mid( 8 );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_NOHTTP, (LPCTSTR)m_sAddress );
		Close( TS_FALSE );
		return FALSE;
	}
	
	if ( strCode == _T("200") || strCode == _T("206") )
	{
		SetState( dtsHeaders );
	}
	else if ( strCode == _T("503") )
	{
		if ( _tcsistr( strMessage, _T("range") ) != NULL )
		{
			m_bRangeFault = TRUE;
		}
		else
		{
			m_bBusyFault = TRUE;
		}
		
		SetState( dtsHeaders );
	}
	else if ( strCode == _T("416") )
	{
		m_bRangeFault = TRUE;
		SetState( dtsHeaders );
	}
	else if ( FALSE && ( strCode == _T("301") || strCode == _T("302") ) )
	{
		// TODO: Read "Location:" header and re-request
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
	
	m_pHeaderName.RemoveAll();
	m_pHeaderValue.RemoveAll();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read header lines

BOOL CDownloadTransferHTTP::OnHeaderLine(CString& strHeader, CString& strValue)
{
	theApp.Message( MSG_DEBUG, _T("%s: DOWNLOAD HEADER: %s: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strHeader, (LPCTSTR)strValue );
	
	if ( strHeader.CompareNoCase( _T("Server") ) == 0 )
	{
		m_sUserAgent = strValue;
		
		if ( IsAgentBlocked() )
		{
			Close( TS_FALSE );
			return FALSE;
		}
		
		m_pSource->m_sServer = strValue;
		if ( strValue.GetLength() > 64 ) strValue = strValue.Left( 64 );
		
		if ( _tcsistr( m_sUserAgent, _T("shareaza") ) != NULL ) m_pSource->SetGnutella( 3 );
		if ( _tcsistr( m_sUserAgent, _T("trustyfiles") ) != NULL ) m_pSource->SetGnutella( 3 );
		if ( _tcsistr( m_sUserAgent, _T("gnucdna") ) != NULL ) m_pSource->SetGnutella( 3 );
		if ( _tcsistr( m_sUserAgent, _T("adagio") ) != NULL ) m_pSource->SetGnutella( 2 );
	}
	else if ( strHeader.CompareNoCase( _T("Connection") ) == 0 )
	{
		if ( strValue.CompareNoCase( _T("Keep-Alive") ) == 0 ) m_bKeepAlive = TRUE;
	}
	else if ( strHeader.CompareNoCase( _T("Content-Length") ) == 0 )
	{
		_stscanf( strValue, _T("%I64i"), &m_nContentLength );
	}
	else if ( strHeader.CompareNoCase( _T("Content-Range") ) == 0 )
	{
		QWORD nFirst = 0, nLast = 0, nTotal = 0;
		
		if ( _stscanf( strValue, _T("bytes %I64i-%I64i/%I64i"), &nFirst, &nLast, &nTotal ) != 3 )
			_stscanf( strValue, _T("bytes=%I64i-%I64i/%I64i"), &nFirst, &nLast, &nTotal );
		
		if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
		{
			m_pDownload->m_nSize = nTotal;
		}
		else if ( m_bTigerFetch || m_bMetaFetch )
		{
			m_nOffset = nFirst;
			m_nLength = nLast + 1 - nFirst;
			if ( m_nContentLength == SIZE_UNKNOWN ) m_nContentLength = m_nLength;
			return TRUE;
		}
		else if ( m_pDownload->m_nSize != nTotal )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_SIZE, (LPCTSTR)m_sAddress,
				(LPCTSTR)m_pDownload->GetDisplayName() );
			Close( TS_FALSE );
			return FALSE;
		}
		
		if ( m_nOffset == SIZE_UNKNOWN && ! m_pDownload->GetFragment( this ) )
		{
			Close( TS_TRUE );
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
			
			theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_USEFUL_RANGE, (LPCTSTR)m_sAddress,
				m_nOffset, m_nOffset + m_nLength - 1, (LPCTSTR)m_pDownload->GetDisplayName() );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_RANGE, (LPCTSTR)m_sAddress,
				(LPCTSTR)m_pDownload->GetDisplayName() );
			Close( TS_TRUE );
			
			return FALSE;
		}
		
		if ( m_nContentLength == SIZE_UNKNOWN ) m_nContentLength = m_nLength;
		m_bGotRange = TRUE;
	}
	else if ( strHeader.CompareNoCase( _T("Content-Type") ) == 0 )
	{
		m_sContentType = strValue;
	}
	else if ( strHeader.CompareNoCase( _T("Content-Encoding") ) == 0 )
	{
		if ( Settings.Downloads.AllowBackwards && _tcsistr( strValue, _T("backwards") ) ) m_bRecvBackwards = TRUE;
	}
	else if (	strHeader.CompareNoCase( _T("X-Gnutella-Content-URN") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Content-URN") ) == 0 ||
				strHeader.CompareNoCase( _T("Content-URN") ) == 0 )
	{
		for ( CString strURNs = strValue + ',' ; ; )
		{
			int nPos = strURNs.Find( ',' );
			if ( nPos < 0 ) break;
			
			strValue	= strURNs.Left( nPos );
			strURNs		= strURNs.Mid( nPos + 1 );
			strValue.TrimLeft();
			
			SHA1 pSHA1;
			if ( CSHA::HashFromURN( strValue, &pSHA1 ) )
			{
				if ( m_pSource->CheckHash( &pSHA1 ) )
				{
					m_bHashMatch = TRUE;
				}
				else
				{
					theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_HASH, (LPCTSTR)m_sAddress,
						(LPCTSTR)m_pDownload->GetDisplayName() );
					Close( TS_FALSE );
					return FALSE;
				}
			}
			
			// TODO: Remove " ! m_bHashMatch "
			
			TIGEROOT pTiger;
			if ( ! m_bHashMatch && CTigerNode::HashFromURN( strValue, &pTiger ) )
			{
				if ( m_pSource->CheckHash( &pTiger ) )
				{
					m_bHashMatch = TRUE;
				}
				else
				{
					theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_HASH, (LPCTSTR)m_sAddress,
						(LPCTSTR)m_pDownload->GetDisplayName() );
					Close( TS_FALSE );
					return FALSE;
				}
			}
			
			MD4 pED2K;
			if ( CED2K::HashFromURN( strValue, &pED2K ) )
			{
				if ( m_pSource->CheckHash( &pED2K ) )
				{
					m_bHashMatch = TRUE;
				}
				else
				{
					theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_HASH, (LPCTSTR)m_sAddress,
						(LPCTSTR)m_pDownload->GetDisplayName() );
					Close( TS_FALSE );
					return FALSE;
				}
			}
		}
		m_pSource->SetGnutella( 1 );
	}
	else if ( strHeader.CompareNoCase( _T("X-Metadata-Path") ) == 0 )
	{
		if ( ! m_bMetaIgnore && Settings.Downloads.Metadata ) m_sMetadata = strValue;
	}
	else if ( strHeader.CompareNoCase( _T("X-TigerTree-Path") ) == 0 )
	{
		if ( Settings.Downloads.VerifyTiger && ! m_bTigerIgnore && m_sTigerTree.IsEmpty() )
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
			if ( StartsWith( strValue, _T("/") ) )
			{
				m_sTigerTree = strValue.SpanExcluding( _T("; ") );
				Replace( m_sTigerTree, _T("ed2k=0"), _T("ed2k=1") );
			}
		}
		m_pSource->SetGnutella( 1 );
	}
	else if (	strHeader.CompareNoCase( _T("X-Gnutella-Alternate-Location") ) == 0 ||
				strHeader.CompareNoCase( _T("Alt-Location") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Alt") ) == 0 )
	{
		if ( Settings.Library.SourceMesh )
		{
			if ( strValue.Find( _T("Zhttp://") ) < 0 )
			{
				m_pDownload->AddSourceURLs( strValue, m_bHashMatch );
			}
		}
		m_pSource->SetGnutella( 1 );
	}
	else if ( strHeader.CompareNoCase( _T("X-Available-Ranges") ) == 0 )
	{
		m_bGotRanges = TRUE;
		m_pSource->SetAvailableRanges( strValue );
		m_pSource->SetGnutella( 1 );
	}
	else if ( strHeader.CompareNoCase( _T("X-Queue") ) == 0 )
	{
		m_pSource->SetGnutella( 1 );
		
		m_bQueueFlag = TRUE;
		CharLower( strValue.GetBuffer() );
		strValue.ReleaseBuffer();
		
		int nPos = strValue.Find( _T("position=") );
		if ( nPos >= 0 ) _stscanf( strValue.Mid( nPos + 9 ), _T("%i"), &m_nQueuePos );
		
		nPos = strValue.Find( _T("length=") );
		if ( nPos >= 0 ) _stscanf( strValue.Mid( nPos + 7 ), _T("%i"), &m_nQueueLen );
		
		DWORD nLimit;
		
		nPos = strValue.Find( _T("pollmin=") );
		if ( nPos >= 0 && _stscanf( strValue.Mid( nPos + 8 ), _T("%u"), &nLimit ) == 1 )
		{
			m_nRetryDelay = max( m_nRetryDelay, nLimit * 1000 + 3000  );
		}
		
		nPos = strValue.Find( _T("pollmax=") );
		if ( nPos >= 0 && _stscanf( strValue.Mid( nPos + 8 ), _T("%u"), &nLimit ) == 1 )
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
	else if (	strHeader.CompareNoCase( _T("X-PerHost") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Gnutella-maxSlotsPerHost") ) == 0 )
	{
		int nLimit = 0;
		
		if ( _stscanf( strValue, _T("%i"), &nLimit ) != 1 )
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
	
	return CTransfer::OnHeaderLine( strHeader, strValue );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP end of headers

BOOL CDownloadTransferHTTP::OnHeadersComplete()
{
	if ( m_bBadResponse )
	{
		Close( TS_FALSE );
		return FALSE;
	}
	else if ( ! m_pSource->CanInitiate( TRUE, TRUE ) )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DISABLED,
			(LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
		Close( TS_FALSE );
		return FALSE;
	}
	else if ( m_bBusyFault )
	{
		m_nOffset = SIZE_UNKNOWN;
		
		if ( Settings.Downloads.QueueLimit > 0 && m_nQueuePos > Settings.Downloads.QueueLimit )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_QUEUE_HUGE,
				(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName(), m_nQueuePos );
			Close( TS_FALSE );
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
		if ( m_pHost.sin_addr.S_un.S_addr == Network.m_pHost.sin_addr.S_un.S_addr )
		{
			Close( TS_FALSE );
			return FALSE;
		}
		
		m_nOffset = SIZE_UNKNOWN;
		SetState( dtsFlushing );
		m_tContent = m_mInput.tLast = GetTickCount();
		
		return ReadFlush();
	}
	else if ( m_nContentLength == SIZE_UNKNOWN )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_SIZE, (LPCTSTR)m_sAddress,
			(LPCTSTR)m_pDownload->GetDisplayName() );
		Close( TS_FALSE );
		return FALSE;
	}
	else if ( m_bTigerFetch )
	{
		if ( ! m_bGotRange )
		{
			m_nOffset = 0;
			m_nLength = m_nContentLength;
		}
		else if ( m_nOffset > 0 )
		{
			theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_TIGER_RANGE, (LPCTSTR)m_sAddress );
			Close( TS_FALSE );
			return FALSE;
		}
		
		if (	m_sContentType.CompareNoCase( _T("application/tigertree-breadthfirst") ) &&
				m_sContentType.CompareNoCase( _T("application/dime") ) )
		{
			theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_TIGER_RANGE, (LPCTSTR)m_sAddress );
			Close( TS_FALSE );
			return FALSE;
		}
		
		SetState( dtsTiger );
		m_tContent = m_mInput.tLast = GetTickCount();
		
		theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_TIGER_RECV, (LPCTSTR)m_sAddress,
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
		
		theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_METADATA_RECV,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_pSource->m_sServer );
		
		return ReadMetadata();
	}
	else if ( ! m_bGotRange )
	{
		if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
		{
			m_pDownload->m_nSize = m_nContentLength;
		}
		else if ( m_pDownload->m_nSize != m_nContentLength )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_SIZE, (LPCTSTR)m_sAddress,
				(LPCTSTR)m_pDownload->GetDisplayName() );
			Close( TS_FALSE );
			return FALSE;
		}
		
		if ( m_nOffset == SIZE_UNKNOWN && ! m_pDownload->GetFragment( this ) )
		{
			Close( TS_TRUE );
			return FALSE;
		}
		
		if ( ! m_pDownload->IsPositionEmpty( 0 ) )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_RANGE, (LPCTSTR)m_sAddress,
				(LPCTSTR)m_pDownload->GetDisplayName() );
			Close( TS_TRUE );
			return FALSE;
		}
		
		m_nOffset = 0;
		m_nLength = m_nContentLength;
	}
	
	if ( m_nContentLength != m_nLength )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_RANGE, (LPCTSTR)m_sAddress,
			(LPCTSTR)m_pDownload->GetDisplayName() );
		Close( TS_FALSE );
		return FALSE;
	}
	
	if ( ! m_bKeepAlive ) m_pSource->m_bCloseConn = TRUE;
	
	theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_CONTENT, (LPCTSTR)m_sAddress,
		(LPCTSTR)m_pSource->m_sServer );
	
	SetState( dtsDownloading );
	m_nPosition = 0;
	m_tContent = m_mInput.tLast = GetTickCount();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read content

BOOL CDownloadTransferHTTP::ReadContent()
{
	if ( m_pInput->m_nLength > 0 )
	{
		m_pSource->SetValid();
		
		DWORD nLength	= (DWORD)min( (QWORD)m_pInput->m_nLength, m_nLength - m_nPosition );
		BOOL bSubmit	= FALSE;
		
		if ( m_bRecvBackwards )
		{
			BYTE* pBuffer = new BYTE[ nLength ];
			CBuffer::ReverseBuffer( m_pInput->m_pBuffer, pBuffer, nLength );
			bSubmit = m_pDownload->SubmitData(
				m_nOffset + m_nLength - m_nPosition - nLength, pBuffer, nLength );
			delete [] pBuffer;
		}
		else
		{
			bSubmit = m_pDownload->SubmitData(
						m_nOffset + m_nPosition, m_pInput->m_pBuffer, nLength );
		}
		
		m_pInput->Clear();	// Clear the buffer, we don't want any crap
		m_nPosition += nLength;
		m_nDownloaded += nLength;
		
		if ( ! bSubmit && m_pDownload->GetProgress() < 0.95f )
		{
			BOOL bUseful = FALSE;
			
			if ( m_bRecvBackwards )
			{
				bUseful = m_pDownload->IsRangeUseful( m_nOffset, m_nLength - m_nPosition );
			}
			else
			{
				bUseful = m_pDownload->IsRangeUseful( m_nOffset + m_nPosition, m_nLength - m_nPosition );
			}
			
			if ( /* m_bInitiated || */ ! bUseful )
			{
				theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_FRAGMENT_OVERLAP, (LPCTSTR)m_sAddress );
				Close( TS_TRUE );
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
	if ( m_pInput->m_nLength < m_nLength ) return TRUE;
	
	CString strXML = m_pInput->ReadString( (DWORD)m_nLength, CP_UTF8 );
	
	if ( CXMLElement* pXML = CXMLElement::FromString( strXML, TRUE ) )
	{
		if ( m_pDownload->m_pXML == NULL )
		{
			m_pDownload->m_pXML = pXML;
		}
		else
		{
			delete pXML;
		}
	}
	
	m_pInput->Remove( (DWORD)m_nLength );
	
	return StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read tiger tree

BOOL CDownloadTransferHTTP::ReadTiger()
{
	if ( m_pInput->m_nLength < m_nLength ) return TRUE;
	
	if ( m_sContentType.CompareNoCase( _T("application/tigertree-breadthfirst") ) == 0 )
	{
		m_pDownload->SetTigerTree( m_pInput->m_pBuffer, (DWORD)m_nLength );
		m_pInput->Remove( (DWORD)m_nLength );
	}
	else if ( m_sContentType.CompareNoCase( _T("application/dime") ) == 0 )
	{
		CString strID, strType, strUUID = _T("x");
		DWORD nFlags, nBody;
		
		while ( m_pInput->ReadDIME( &nFlags, &strID, &strType, &nBody ) )
		{
			theApp.Message( MSG_DEBUG, _T("THEX DIME: %i, '%s', '%s', %i"),
				nFlags, (LPCTSTR)strID, (LPCTSTR)strType, nBody );
			
			if ( ( nFlags & 1 ) && strType.CompareNoCase( _T("text/xml") ) == 0 && nBody < 1024*1024 )
			{
				BOOL bSize = FALSE, bDigest = FALSE, bEncoding = FALSE;
				CString strXML;
				
				strXML = m_pInput->ReadString( nBody, CP_UTF8 );
				
				if ( CXMLElement* pXML = CXMLElement::FromString( strXML ) )
				{
					if ( pXML->IsNamed( _T("hashtree") ) )
					{
						if ( CXMLElement* pxFile = pXML->GetElementByName( _T("file") ) )
						{
							QWORD nSize = 0;
							_stscanf( pxFile->GetAttributeValue( _T("size") ), _T("%I64i"), &nSize );
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
			else if ( strID == strUUID && strType.CompareNoCase( _T("http://open-content.net/spec/thex/breadthfirst") ) == 0 )
			{
				m_pDownload->SetTigerTree( m_pInput->m_pBuffer, nBody );
			}
			else if ( strType.CompareNoCase( _T("http://edonkey2000.com/spec/md4-hashset") ) == 0 )
			{
				m_pDownload->SetHashset( m_pInput->m_pBuffer, nBody );
			}
			
			m_pInput->Remove( ( nBody + 3 ) & ~3 );
			if ( nFlags & 2 ) break;
		}
		
		m_pInput->Clear();
	}
	
	return StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferHTTP read flushing

BOOL CDownloadTransferHTTP::ReadFlush()
{
	if ( m_nContentLength == SIZE_UNKNOWN ) m_nContentLength = 0;
	
	DWORD nRemove = min( m_pInput->m_nLength, (DWORD)m_nContentLength );
	m_nContentLength -= nRemove;
	
	m_pInput->Remove( nRemove );
	
	if ( m_nContentLength == 0 )
	{
		if ( m_bQueueFlag )
		{
			SetState( dtsQueued );
			m_tRequest = GetTickCount() + m_nRetryDelay;

			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_QUEUED,
				(LPCTSTR)m_sAddress, m_nQueuePos, m_nQueueLen,
				(LPCTSTR)m_sQueueName );
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

void CDownloadTransferHTTP::OnDropped(BOOL bError)
{
	if ( m_nState == dtsConnecting )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CONNECT_ERROR, (LPCTSTR)m_sAddress );
		if ( m_pSource != NULL ) m_pSource->PushRequest();
		Close( TS_UNKNOWN );
	}
	else if ( m_nState == dtsBusy )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BUSY, (LPCTSTR)m_sAddress, Settings.Downloads.RetryDelay / 1000 );
		Close( TS_TRUE );
	}
	else
	{
//		if ( m_nState == dtsDownloading && m_nLength && m_pSource )
//			m_pSource->m_bCloseConn = TRUE;
		
		if ( m_bBusyFault || m_bQueueFlag )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BUSY, (LPCTSTR)m_sAddress, Settings.Downloads.RetryDelay / 1000 );
			Close( TS_TRUE );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DROPPED, (LPCTSTR)m_sAddress );
			Close( m_nState >= dtsDownloading ? TS_TRUE : TS_UNKNOWN );
		}
	}
}

