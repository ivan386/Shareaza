//
// CDownloadTransferFTP.cpp
//
// Copyright (c) Nikolay Raspopov, 2002-2004.
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
#include "DownloadTransferFTP.h"
#include "FragmentedFile.h"
#include "Network.h"
#include "Buffer.h"
#include "SourceURL.h"
#include "GProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP construction

CDownloadTransferFTP::CDownloadTransferFTP (CDownloadSource* pSource) :
	CDownloadTransfer ( pSource, PROTOCOL_FTP ),
	m_pListTransfer (NULL),
	m_bListTransferValid (FALSE),
	m_pDataTransfer (NULL),
	m_bDataTransferValid (FALSE)
{
	TRACE ("0x%08x : CDownloadTransferFTP::CDownloadTransferFTP (0x%08x)\n", this, pSource);

	m_nRequests		= 0;
	m_nOffset		= 0;
	m_tContent		= 0;
	m_bBusyFault	= FALSE;
	m_nRetryDelay	= Settings.Downloads.RetryDelay;
}

CDownloadTransferFTP::~CDownloadTransferFTP ()
{
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP initiate connection

BOOL CDownloadTransferFTP::Initiate()
{
	TRACE ("0x%08x : CDownloadTransferFTP::Initiate()\n", this);

	theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_CONNECTING,
		(LPCTSTR)CString( inet_ntoa( m_pSource->m_pAddress ) ), m_pSource->m_nPort,
		(LPCTSTR)m_pDownload->GetDisplayName() );

	m_tConnected	= GetTickCount();
	m_FtpState		= ftpConnecting;

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
// CDownloadTransferFTP close

void CDownloadTransferFTP::Close(TRISTATE bKeepSource)
{
	TRACE ("0x%08x : CDownloadTransferFTP::Close(%d)\n", this, bKeepSource);

	m_FtpState = ftpConnecting;
	
	CDownloadTransfer::Close( bKeepSource );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP speed controls

void CDownloadTransferFTP::Boost()
{
	TRACE ("0x%08x : CDownloadTransferFTP::Boost()\n", this);

	m_mInput.pLimit = m_mOutput.pLimit = NULL;

	if (m_bDataTransferValid)
		return m_pDataTransfer->Boost();
	if (m_bListTransferValid)
		return m_pListTransfer->Boost();
}

DWORD CDownloadTransferFTP::GetAverageSpeed ()
{
	if (m_bDataTransferValid)
		return m_pDataTransfer->GetAverageSpeed ();
	if (m_bListTransferValid)
		return m_pListTransfer->GetAverageSpeed ();
	return m_pSource->m_nSpeed;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP connection handler

BOOL CDownloadTransferFTP::OnConnected()
{
	TRACE ("0x%08x : CDownloadTransferFTP::OnConnected()\n", this);

	theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_CONNECTED, (LPCTSTR)m_sAddress );
	
	m_tConnected = GetTickCount();
	
	SetState( dtsRequesting );

	return StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP fragment allocation

BOOL CDownloadTransferFTP::StartNextFragment()
{
	TRACE ("0x%08x : CDownloadTransferFTP::StartNextFragment()\n", this);

	ASSERT( this != NULL );
	if ( this == NULL ) return FALSE;

	m_nOffset			= SIZE_UNKNOWN;
	m_nPosition			= 0;
	m_bWantBackwards	= FALSE;
	m_bRecvBackwards	= FALSE;
	
	if ( m_pInput == NULL || m_pOutput == NULL )
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
	
	if ( m_FtpState == ftpConnecting )
	{
		// Handshaking
		return SendRequest();
	}
	else if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
	{
		// Getting file size
		m_FtpState = ftpTYPE1;
		return SendRequest();
	}
	else if ( m_pDownload->GetFragment( this ) )
	{
		// Donloading
		ChunkifyRequest( &m_nOffset, &m_nLength, Settings.Downloads.ChunkSize, TRUE );
		theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_FRAGMENT_REQUEST,
			m_nOffset, m_nOffset + m_nLength - 1,
			(LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress );
		m_FtpState = ftpTYPE2;
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
// CDownloadTransferFTP subtract pending requests

BOOL CDownloadTransferFTP::SubtractRequested(CFileFragmentList& Fragments)
{
	TRACE ("0x%08x : CDownloadTransferFTP::SubtractRequested()\n", this);

	if ( m_nOffset < SIZE_UNKNOWN && m_nLength < SIZE_UNKNOWN )
	{
		if ( m_nState == dtsRequesting || m_nState == dtsDownloading )
		{
			Fragments.Subtract( m_nOffset, m_nOffset + m_nLength );
			return TRUE;
		}
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP send request

BOOL CDownloadTransferFTP::SendRequest()
{
	TRACE ("0x%08x : CDownloadTransferFTP::SendRequest()\n", this);

	CString strLine;
	
	CSourceURL pURL;
	if ( ! pURL.ParseFTP( m_pSource->m_sURL, FALSE ) ) return FALSE;
	
	theApp.Message( MSG_DEBUG, _T("%s: DOWNLOAD REQUEST: %s"),
		(LPCTSTR)m_sAddress, (LPCTSTR)pURL.m_sPath );

	switch (m_FtpState) {
		case ftpConnecting:
			// Waiting for server reply
			break;
		case ftpUSER:
			strLine = _T("USER anonymous\r\n");
			break;
		case ftpPASS:
			strLine = _T("PASS anonymous@anonymous.com\r\n");
			break;
		case ftpTYPE1:
			strLine = _T("TYPE A\r\n");
			break;
		case ftpTYPE2:
			strLine = _T("TYPE I\r\n");
			break;
		case ftpPASV1:
		case ftpPASV2:
			strLine = _T("PASV\r\n");
			break;
		case ftpLIST1:
			strLine.Format( _T("LIST %s\r\n"), (LPCTSTR) pURL.m_sPath );
			break;
		case ftpREST2:
			strLine.Format( _T("REST %d\r\n"), m_nOffset );
			break;
		case ftpRETR2:
			strLine.Format( _T("RETR %s\r\n"), (LPCTSTR) pURL.m_sPath );		
			break;
		default:
			ASSERT (FALSE);
	}
	TRACE ("-> %ls", strLine);
	m_pOutput->Print( strLine );	
	m_tRequest			= GetTickCount();
	m_bBusyFault		= FALSE;
	m_bQueueFlag		= FALSE;	
	m_nRequests++;	
	m_pSource->SetLastSeen();	
	CDownloadTransfer::OnWrite();	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP run handler

BOOL CDownloadTransferFTP::OnRun()
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

	/*case dtsQueued:
		if ( tNow >= m_tRequest )
		{
			return StartNextFragment();
		}
		break;*/

	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP read handler

BOOL CDownloadTransferFTP::OnRead()
{
	CDownloadTransfer::OnRead();
	
	CString strLine;
	
	while ( m_pInput->ReadLine( strLine ) )
	{
		if ( strLine.GetLength() > 20480 ) strLine = _T("#LINE_TOO_LONG#");
		strLine = strLine.Trim (_T(" \t\r\n"));
		if ( strLine.IsEmpty() ) {
			m_sLastHeader.Empty();
			return OnHeadersComplete();
		} else {
			m_sLastHeader		= strLine.Left( 3 );
			CString strValue	= strLine.Mid( 4 );
			strValue.TrimLeft();
			strValue.TrimRight();			
			if ( strValue.GetLength() > 0 )
			{
				if ( ! OnHeaderLine( m_sLastHeader, strValue ) ) return FALSE;
			}
		}
	}
	
	OnWrite();
	
	return TRUE;
}

BOOL ParsePASV (CString& strValue, SOCKADDR_IN& host)
{
	int begin = strValue.Find (_T('('));
	int end = strValue.Find (_T(')'));
	if (begin == -1 || end == -1 || end - begin < 12)
		return FALSE;
	strValue = strValue.Mid (begin + 1, end - begin - 1);

	ZeroMemory (&host, sizeof (host));
	host.sin_family = AF_INET;
	int d;

	// h1
	d = strValue.Find (_T(','));
	if (d == -1)
		return FALSE;
	host.sin_addr.S_un.S_un_b.s_b1 = (unsigned char) (_tstoi (strValue.Mid (0, d)) & 0xff);
	strValue = strValue.Mid (d + 1);
	// h2
	d = strValue.Find (_T(','));
	if (d == -1)
		return FALSE;
	host.sin_addr.S_un.S_un_b.s_b2 = (unsigned char) (_tstoi (strValue.Mid (0, d)) & 0xff);
	strValue = strValue.Mid (d + 1);
	// h3
	d = strValue.Find (_T(','));
	if (d == -1)
		return FALSE;
	host.sin_addr.S_un.S_un_b.s_b3 = (unsigned char) (_tstoi (strValue.Mid (0, d)) & 0xff);
	strValue = strValue.Mid (d + 1);
	// h4
	d = strValue.Find (_T(','));
	if (d == -1)
		return FALSE;
	host.sin_addr.S_un.S_un_b.s_b4 = (unsigned char) (_tstoi (strValue.Mid (0, d)) & 0xff);
	strValue = strValue.Mid (d + 1);
	// p1
	d = strValue.Find (_T(','));
	if (d == -1)
		return FALSE;
	host.sin_port = (unsigned char) (_tstoi (strValue.Mid (0, d)) & 0xff) * 256;
	strValue = strValue.Mid (d + 1);
	// p2
	host.sin_port += (unsigned char) (_tstoi (strValue) & 0xff);
	return TRUE;
}

bool Split (CString& in, TCHAR token, CString& out)
{
	in = in.Trim (_T(" \t\r\n"));
	if (!in.GetLength ()) {
		out.Empty ();
		return false;
	}
	int p = in.ReverseFind (token);
	if (p != -1) {
		out = in.Mid (p + 1);
		in = in.Mid (0, p);
	} else {
		out = in;
		in.Empty ();
	}
	return true;
}

bool ExtractFileSize (CString data, QWORD& size)
{
	CString tmp;
	for (int n = 0; Split (data, _T(' '), tmp); ++n) {
		for (int i = 0; i < tmp.GetLength (); ++i)
			if (!isdigit (tmp [i]))
				break;
		if (i == tmp.GetLength () && tmp [0] != _T('0') && n != 2) {
			size = _tstoi (tmp);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP read header lines

BOOL CDownloadTransferFTP::OnHeaderLine(CString& strHeader, CString& strValue)
{
	TRACE ("0x%08x : CDownloadTransferFTP::OnHeaderLine(\"%ls\",\"%ls\")\n", this, strHeader, strValue);

	theApp.Message( MSG_DEBUG, _T("%s: DOWNLOAD HEADER: %s: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strHeader, (LPCTSTR)strValue );

	switch (m_FtpState) {
		case ftpConnecting:
			SetState (dtsHeaders);
			if ( strHeader == _T("220") ) // Connected
			{
				m_sUserAgent = strValue;
				if ( IsAgentBlocked() )
				{
					Close( TS_FALSE );
					return FALSE;
				}
				m_FtpState = ftpUSER;
				return SendRequest ();
			} else {
				Close( TS_UNKNOWN );
				return FALSE;
			}

		case ftpUSER:
			if ( strHeader == _T("331") ) // Anonymous access allowed
			{		
				m_FtpState = ftpPASS;
				return SendRequest ();
			} else 
			if ( strHeader == _T("220") )	// Extra headers
				return TRUE;
			else {
				Close( TS_UNKNOWN );
				return FALSE;
			}

		case ftpPASS:
			if ( strHeader == _T("230")) // Logged in
			{
				return StartNextFragment();
			} else {
				Close( TS_UNKNOWN );
				return FALSE;
			}

		case ftpTYPE1:
			SetState (dtsRequesting);
			if ( strHeader == _T("200")) // Type A setted
			{
				m_FtpState = ftpPASV1;
			} else
			if ( strHeader == _T("230")) // Extra "Logged in" messages
				return TRUE;
			return SendRequest ();

		case ftpPASV1:
			if ( strHeader == _T("227")) // Entered passive mode
			{
				SOCKADDR_IN host;
				if (ParsePASV (strValue, host)) {
					m_FtpState = ftpLIST1;

					// Open data connection for listing
					m_pListTransfer = new CDownloadTransferFTPLIST ( m_pSource, host,
						&m_sListData, &m_bListTransferValid);
					m_pListTransfer->Initiate ();
				}
			}
			return SendRequest ();

		case ftpLIST1:
			if ( strHeader == _T("125") || strHeader == _T("150") ) // Transfer started
			{
				// Downloading...
			} else
			if ( strHeader == _T("226") ) // Transfer completed
			{
				TRACE ("List: \"%ls\"\n", m_sListData);

				// Extract file size
				if (ExtractFileSize (m_sListData, m_pDownload->m_nSize))
						return StartNextFragment();
				m_FtpState = ftpTYPE1;
				return SendRequest ();
			} else {
				m_FtpState = ftpTYPE1;
				return SendRequest ();
			}
			return TRUE;

		case ftpTYPE2:
			SetState (dtsRequesting);
			if ( strHeader == _T("200") ) // Type I setted
			{
				m_FtpState = ftpPASV2;
			}
			return SendRequest ();

		case ftpPASV2:
			SetState (dtsDownloading);
			if ( strHeader == _T("227") ) // Entered passive mode
			{
				SOCKADDR_IN host;
				if (ParsePASV (strValue, host)) {
					m_FtpState = ftpREST2;

					// Open data connection for data
					SetState (dtsDownloading);
					m_pDataTransfer = new CDownloadTransferFTPDATA ( m_pSource, host,
						m_nOffset, m_nLength, &m_bDataTransferValid);
					m_pDataTransfer->Initiate ();
				}
			}
			return SendRequest ();

		case ftpREST2:
			if ( strHeader == _T("350")) // Offset setted
			{
				m_FtpState = ftpRETR2;
			} else
				m_FtpState = ftpTYPE2;
			return SendRequest ();

		case ftpRETR2:
			if ( strHeader == _T("125") || strHeader == _T("150") ) // Transfer started
			{
				// Downloading...
			} else
			if ( strHeader == _T("226") || strHeader == _T("426") ) // Transfer completed
			{
				m_FtpState = ftpCompleted;
				return StartNextFragment();
			} else {
				m_FtpState = ftpTYPE2;
				return SendRequest ();
			}
			return TRUE;

		default:
			ASSERT (FALSE);
	}
	
	return CTransfer::OnHeaderLine( strHeader, strValue );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP dropped connection handler

void CDownloadTransferFTP::OnDropped(BOOL bError)
{
	TRACE ("0x%08x : CDownloadTransferFTP::OnDropped(%d)\n", this, bError);

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
	} else {
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

