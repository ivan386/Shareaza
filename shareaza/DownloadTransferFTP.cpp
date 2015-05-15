//
// DownloadTransferFTP.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadTransferFTP.h"
#include "FragmentedFile.h"
#include "Network.h"
#include "ShareazaURL.h"
#include "GProfile.h"
#include "Security.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Service functions

inline void MakePORTArgs(const SOCKADDR_IN& host, CString& strValue)
{
	strValue.Format( _T("%u,%u,%u,%u,%hu,%hu"),
		unsigned( host.sin_addr.S_un.S_un_b.s_b1 ),
		unsigned( host.sin_addr.S_un.S_un_b.s_b2 ),
		unsigned( host.sin_addr.S_un.S_un_b.s_b3 ),
		unsigned( host.sin_addr.S_un.S_un_b.s_b4 ),
		(BYTE)(   host.sin_port       & 0xff ),
		(BYTE)( ( host.sin_port >> 8) & 0xff ) );
}

inline bool ParsePASVArgs(const CString& args, SOCKADDR_IN& host)
{
	CString strValue (args);
	int begin = strValue.Find (_T('('));
	int end = strValue.Find (_T(')'));
	if (begin == -1 || end == -1 || end - begin < 12)
		return false;
	strValue = strValue.Mid (begin + 1, end - begin - 1);
	ZeroMemory (&host, sizeof (host));
	host.sin_family = AF_INET;
	int d;
	// h1
	d = strValue.Find (_T(','));
	if (d == -1)
		return false;
	host.sin_addr.S_un.S_un_b.s_b1 = (unsigned char) (_tstoi (strValue.Mid (0, d)) & 0xff);
	strValue = strValue.Mid (d + 1);
	// h2
	d = strValue.Find (_T(','));
	if (d == -1)
		return false;
	host.sin_addr.S_un.S_un_b.s_b2 = (unsigned char) (_tstoi (strValue.Mid (0, d)) & 0xff);
	strValue = strValue.Mid (d + 1);
	// h3
	d = strValue.Find (_T(','));
	if (d == -1)
		return false;
	host.sin_addr.S_un.S_un_b.s_b3 = (unsigned char) (_tstoi (strValue.Mid (0, d)) & 0xff);
	strValue = strValue.Mid (d + 1);
	// h4
	d = strValue.Find (_T(','));
	if (d == -1)
		return false;
	host.sin_addr.S_un.S_un_b.s_b4 = (unsigned char) (_tstoi (strValue.Mid (0, d)) & 0xff);
	strValue = strValue.Mid (d + 1);
	// p1
	d = strValue.Find (_T(','));
	if (d == -1)
		return false;
	host.sin_port = (unsigned char) (_tstoi (strValue.Mid (0, d)) & 0xff);
	strValue = strValue.Mid (d + 1);
	// p2
	host.sin_port += (unsigned char) (_tstoi (strValue) & 0xff) * 256;
	return true;
}

inline bool FTPisOK(const CString& str)
{
	return ( str.GetLength () == 3 && str [0] == _T('2') );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP construction

CDownloadTransferFTP::CDownloadTransferFTP(CDownloadSource* pSource) :
	CDownloadTransfer( pSource, PROTOCOL_FTP ),
	m_FtpState( ftpConnecting ),
	m_bPassive( TRUE /* FALSE */ ),
	m_bSizeChecked( FALSE ),
	m_bMultiline( FALSE )
{
	m_RETR.SetOwner( this );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP initiate connection

BOOL CDownloadTransferFTP::Initiate()
{
	theApp.Message( MSG_INFO, IDS_DOWNLOAD_CONNECTING,
		(LPCTSTR)CString( inet_ntoa( m_pSource->m_pAddress ) ), m_pSource->m_nPort,
		(LPCTSTR)m_pDownload->GetDisplayName() );

	m_tConnected	= GetTickCount();
	m_FtpState		= ftpConnecting;

	if ( ConnectTo( &m_pSource->m_pAddress, m_pSource->m_nPort ) )
	{
		SetState( dtsConnecting );
		
		if ( !m_pDownload->IsBoosted() )
		{
			m_mInput.pLimit = &m_nBandwidth;
			m_mOutput.pLimit = &Settings.Bandwidth.Request;
		}
		
		return TRUE;
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CONNECT_ERROR, (LPCTSTR)m_sAddress );
		Close();
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP close

void CDownloadTransferFTP::Close (TRISTATE bKeepSource, DWORD nRetryAfter)
{
	if ( m_pSource != NULL && m_nState == dtsDownloading && m_FtpState == ftpRETR)
		m_pSource->AddFragment( m_nOffset, m_nPosition );

	m_LIST.Close ();
	m_RETR.Close ();

	m_FtpState = ftpConnecting;
	m_bSizeChecked = FALSE;
	
	CDownloadTransfer::Close( bKeepSource, nRetryAfter );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP bandwidth control

void CDownloadTransferFTP::Boost()
{
	m_mInput.pLimit = m_mOutput.pLimit =
		m_LIST.m_mInput.pLimit = m_LIST.m_mOutput.pLimit =
		m_RETR.m_mInput.pLimit = m_RETR.m_mOutput.pLimit = NULL;
}

DWORD CDownloadTransferFTP::GetMeasuredSpeed()
{
	// Calculate Input
	MeasureIn();		
	m_LIST.MeasureIn();
	m_RETR.MeasureIn();

	// Return calculated speed
	return m_mInput.nMeasure + m_LIST.m_mInput.nMeasure + m_RETR.m_mInput.nMeasure;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP connection handler

BOOL CDownloadTransferFTP::OnConnected()
{
	theApp.Message( MSG_INFO, IDS_DOWNLOAD_CONNECTED, (LPCTSTR)m_sAddress );
	
	m_tConnected = GetTickCount();
	SetState( dtsRequesting );

	return StartNextFragment();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP fragment allocation

BOOL CDownloadTransferFTP::StartNextFragment()
{
	ASSUME_LOCK( Transfers.m_pSection );

	ASSERT( this != NULL );
	if ( this == NULL )
		return FALSE;

	m_nOffset			= SIZE_UNKNOWN;
	m_nPosition			= 0;
	m_bWantBackwards	= FALSE;
	m_bRecvBackwards	= FALSE;
	
	if ( ! IsInputExist() || ! IsOutputExist() )
	{
		theApp.Message( MSG_INFO, IDS_DOWNLOAD_CLOSING_EXTRA, (LPCTSTR)m_sAddress );
		Close();
		return FALSE;
	}
	else if ( m_FtpState == ftpConnecting )
	{
		// Handshaking
		m_tRequest = GetTickCount();
		return TRUE;
	}
	else if ( m_pDownload->m_nSize == SIZE_UNKNOWN || !m_bSizeChecked )
	{
		// Getting file size
		m_FtpState = ftpSIZE_TYPE; // ftpLIST_TYPE;
		SetState( dtsRequesting );
		return SendCommand ();
	}
	else if ( m_pDownload->GetFragment( this ) )
	{
		// Downloading
		// ChunkifyRequest( &m_nOffset, &m_nLength, Settings.Downloads.ChunkSize, TRUE );
		theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_REQUEST,
			m_nOffset, m_nOffset + m_nLength - 1,
			(LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)m_sAddress );
		// Sending request
		m_FtpState = ftpRETR_TYPE;
		SetState( dtsDownloading );
		return SendCommand ();
	}
	else
	{
		if ( m_pSource )
			m_pSource->SetAvailableRanges( NULL );
		theApp.Message( MSG_INFO, IDS_DOWNLOAD_FRAGMENT_END, (LPCTSTR)m_sAddress );
		Close();
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP subtract pending requests

BOOL CDownloadTransferFTP::SubtractRequested(Fragments::List& ppFragments) const
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
			if ( m_pSource != NULL )
				m_pSource->PushRequest();
			Close();
			return FALSE;
		}
		break;

	case dtsRequesting:
	case dtsHeaders:
		if ( tNow - m_tRequest > Settings.Connection.TimeoutHandshake )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_REQUEST_TIMEOUT, (LPCTSTR)m_sAddress );
			Close();
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
			Close();
			return FALSE;
		}
		break;

	case dtsBusy:
		if ( tNow - m_tRequest > 1000 )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BUSY, (LPCTSTR)m_sAddress, Settings.Downloads.RetryDelay / 1000 );
			Close();
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
// CDownloadTransferFTP read handler

BOOL CDownloadTransferFTP::OnRead()
{
	CDownloadTransfer::OnRead();
	
	CString strLine;
	CString Number;
	while ( Read( strLine ) )
	{
		BOOL bNumber = ( strLine.GetLength() >= 3 ) &&
			_istdigit( strLine[0] ) && _istdigit( strLine[1] ) && _istdigit( strLine[2] );
		if ( bNumber )
			Number = strLine.Left( 3 );

		if ( ! m_bMultiline && bNumber && strLine[3] == _T('-') )
		{
			// Got first line of multi-line reply
			m_bMultiline = TRUE;
			m_sMultiNumber = Number;
			m_sMultiReply = strLine.Mid( 4 );
		}
		else if ( ! m_bMultiline && bNumber )
		{
			// Got single-line reply
			if ( ! OnHeaderLine( Number, strLine.Mid( 4 ).Trim( _T(" \t\r\n") ) ) )
				return FALSE;
		}
		else if ( m_bMultiline && bNumber && strLine[3] == _T(' ') &&
			m_sMultiNumber == Number )
		{
			// Got last line of multi-line reply
			m_bMultiline = FALSE;
			m_sMultiReply += _T("\n");
			m_sMultiReply += strLine.Mid( 4 );
			if ( ! OnHeaderLine( Number, m_sMultiReply.Trim( _T(" \t\r\n") ) ) )
				return FALSE;
		}
		else if ( m_bMultiline )
		{
			// Got next line of multi-line reply
			m_sMultiReply += _T("\n");
			m_sMultiReply += strLine;
		}
		// else Got strange extra line - ignoring
			 
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP read header lines

BOOL CDownloadTransferFTP::OnHeaderLine( CString& strHeader, CString& strValue )
{
	if ( ! CDownloadTransfer::OnHeaderLine( strHeader, strValue ) )
		return FALSE;

	m_pSource->SetLastSeen();

	switch( m_FtpState )
	{
	case ftpConnecting:
		if ( strHeader == _T("220") )			// Connected
		{
			m_LIST.m_sUserAgent = m_RETR.m_sUserAgent = m_sUserAgent =
				m_pSource->m_sServer = strValue.Trim( _T(" \t\r\n-=_") );

			// Empty User Agent is ok for secure FTP
			if ( ! m_sUserAgent.IsEmpty() && Security.IsAgentBlocked( m_sUserAgent ) )
			{
				// Ban
				Close( TRI_FALSE );
				return FALSE;
			}

			// Sending login
			m_FtpState = ftpUSER;
			SetState( dtsRequesting );
			return SendCommand();
		}
		// break;

	case ftpUSER:
		if ( strHeader == _T("331") )			// Access allowed
		{		
			// Sending password
			m_FtpState = ftpPASS;
			SetState( dtsRequesting );
			return SendCommand ();
		}
		// break;

	case ftpPASS:
		if ( strHeader == _T("230") )			// Logged in
		{
			// Downloading
			m_FtpState = ftpDownloading;
			SetState( dtsRequesting );
			return StartNextFragment();
		}
		else if ( FTPisOK( strHeader ) )		// Extra headers
			// Bypass
			return TRUE;
		// Wrong login, password or other errors
		// 530: This FTP server is anonymous only.
		// 530: Login incorrect.
		// etc.
		break;

	case ftpSIZE_TYPE:
		if ( strHeader == _T("200") )			// Type I setted
		{
			// Getting file size
			m_FtpState = ftpSIZE;
			SetState( dtsRequesting );
			return SendCommand ();
		}
		else if ( FTPisOK( strHeader ) )		// Extra headers, may be some 230
			// Bypass
			return TRUE;
		break;

	case ftpSIZE:
		if ( strHeader == _T("213") )			// SIZE reply
		{
			QWORD nTotal;
			if ( _stscanf( strValue, _T("%I64u"), &nTotal ) != 1 || nTotal == 0 )
			{
				// Wrong SIZE reply format
				ASSERT( FALSE );
				// Ban
				Close( TRI_FALSE );
				return FALSE;
			}
			if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
			{
				m_pDownload->SetSize( nTotal );
			}
			else if ( m_pDownload->m_nSize != nTotal )
			{
				// File size changed!
				theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_SIZE,
					(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
				// Ban
				Close( TRI_FALSE );
				return FALSE;
			}
			m_bSizeChecked = TRUE;

			// Downloading
			m_FtpState = ftpDownloading;
			SetState( dtsRequesting );
			return StartNextFragment();
		}
		else if ( FTPisOK( strHeader ) )		// Extra headers, may be some 230
		{
			// Bypass
			return TRUE;
		}
		else if ( strHeader == _T("550") )		// File unavailable
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_FILENOTFOUND,
				(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
			// Ban
			Close( TRI_FALSE );
			return FALSE;
		}
		break;

	case ftpLIST_TYPE:
		if ( strHeader == _T("200") )			// Type A setted
		{
			// Mode choosing
			m_FtpState = ftpLIST_PASVPORT;
			SetState( dtsRequesting );
			return SendCommand ();
		}
		else if ( FTPisOK( strHeader ) )		// Extra headers, may be some 230
			// Bypass
			return TRUE;
		break;

	case ftpLIST_PASVPORT:
		if ( strHeader == _T("227") ||
			 strHeader == _T("200") )			// Entered passive or active mode
		{
			// Getting file size
			if ( m_bPassive )
			{
				// Passive mode
				SOCKADDR_IN host;
				if ( ! ParsePASVArgs( strValue, host ) )
				{
					// Wrong PASV reply format
					ASSERT( FALSE );
					Close( TRI_FALSE );
					return FALSE;
				}
				if ( !m_LIST.ConnectTo( &host ) )
				{
					// Cannot connect
					theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CONNECT_ERROR,
						(LPCTSTR)m_sAddress );
					Close();
					return FALSE;
				}
			}
			m_FtpState = ftpLIST;
			SetState( dtsRequesting );
			return SendCommand ();
		}
		else if ( FTPisOK( strHeader ) )		// Extra headers
			// Bypass
			return TRUE;
		break;

	case ftpLIST:
		if ( strHeader == _T("125") ||
			 strHeader == _T("150") )			// Transfer started
		{
			// Downloading
			return TRUE;
		}
		else if ( strHeader == _T("226") )		// Transfer completed
		{
			// Extract file size
			QWORD size = m_LIST.ExtractFileSize();
			if ( size == SIZE_UNKNOWN )
			{
				// Wrong LIST reply format
				ASSERT( FALSE );
				// Ban
				Close( TRI_FALSE );
				return FALSE;
			}
			if ( m_pDownload->m_nSize == SIZE_UNKNOWN )
			{
				m_pDownload->SetSize( size );
			}
			else if ( m_pDownload->m_nSize != size )
			{
				// File size changed!
				theApp.Message( MSG_ERROR, IDS_DOWNLOAD_WRONG_SIZE,
					(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
				// Ban
				Close( TRI_FALSE );
				return FALSE;
			}
			m_bSizeChecked = TRUE;

			// Aborting
			m_LIST.Close();
			m_FtpState = ftpABOR;
			SetState( dtsRequesting );
			return SendCommand();
		}
		else if ( strHeader == _T("550") )		// File unavailable
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_FILENOTFOUND,
				(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
			// Ban
			Close( TRI_FALSE );
			return FALSE;
		}
		break;

	case ftpRETR_TYPE:
		if ( strHeader == _T("200") )			// Type I setted
		{
			// Mode choosing
			m_FtpState = ftpRETR_PASVPORT;
			SetState( dtsDownloading );
			return SendCommand();
		}
		else if ( FTPisOK( strHeader ) )		// Extra headers, may be some 230
			// Bypass
			return TRUE;
		break;

	case ftpRETR_PASVPORT:
		if ( strHeader == _T("227") ||
			 strHeader == _T("200") )			// Entered passive or active mode
		{
			// File fragment choosing
			if ( m_bPassive )
			{
				SOCKADDR_IN host;
				if ( ! ParsePASVArgs( strValue, host ) )
				{
					// Wrong PASV reply format
					ASSERT( FALSE );
					Close( TRI_FALSE );
					return FALSE;
				}
				if ( ! m_RETR.ConnectTo( &host ) )
				{
					// Cannot connect
					theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CONNECT_ERROR,
						(LPCTSTR)m_sAddress );
					Close();
					return FALSE;
				}
			}
			m_FtpState = ftpRETR_REST;
			SetState (dtsDownloading);
			return SendCommand ();
		}
		else if ( FTPisOK( strHeader ) )		// Extra headers
			// Bypass
			return TRUE;
		break;

	case ftpRETR_REST:
		if ( strHeader == _T("350") )			// Offset setted
		{
			// Downloading
			m_FtpState = ftpRETR;
			SetState( dtsDownloading );
			return SendCommand ();
		}
		break;

	case ftpRETR:
		if ( strHeader == _T("125") ||
			 strHeader == _T("150") )			// Transfer started
		{
			// Downloading
			return TRUE;
		}
		else if ( strHeader == _T("226") )			// Transfer completed
		{
			// Waiting for last chunk
			return TRUE;
		}
		else if ( strHeader == _T("426") )			// Transfer completed
		{
			// Aborting
			m_RETR.Close();
			m_FtpState = ftpABOR;
			SetState( dtsDownloading );
			return SendCommand();
		}
		else if ( strHeader == _T("550") )		// File unavailable
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_FILENOTFOUND,
				(LPCTSTR)m_sAddress, (LPCTSTR)m_pDownload->GetDisplayName() );
			// Ban
			Close( TRI_FALSE );
			return FALSE;
		}
		break;

	case ftpABOR:
		// Downloading
		m_FtpState = ftpDownloading;
		SetState( dtsDownloading );
		return StartNextFragment();

	default:
		// Really unexpected errors
		ASSERT( FALSE );
	}

	theApp.Message( MSG_ERROR, IDS_DOWNLOAD_HTTPCODE,
		(LPCTSTR)m_sAddress, (LPCTSTR)strHeader, (LPCTSTR)strValue);
	Close();
	return FALSE;
}

BOOL CDownloadTransferFTP::SendCommand(LPCTSTR /*args*/)
{
	CShareazaURL pURL;
	if ( !pURL.Parse( m_pSource->m_sURL, FALSE ) || pURL.m_nProtocol != PROTOCOL_FTP )
		return FALSE;

	CString strLine;
	switch( m_FtpState )
	{
	case ftpUSER:
		// Sending login
		strLine = _T("USER ");
		strLine += pURL.m_sLogin;
		break;

	case ftpPASS:
		// Sending password
		strLine = _T("PASS ");
		strLine += pURL.m_sPassword;
		break;

	case ftpLIST_PASVPORT:
		// Selecting passive or active mode
		if ( m_bPassive )
			strLine = _T("PASV");
/*		else
		{
			SOCKADDR_IN host;
			if ( !Handshakes.Add( &m_LIST, host ) )
			{
				// Unexpected errors
				Close();
				return FALSE;
			}
			CString args;
			MakePORTArgs( host, args );
			strLine = _T("PORT ");
			strLine += args;
		}*/
		break;

	case ftpSIZE:
		// Listing file size
		strLine = _T("SIZE ");
		strLine += URLDecode( pURL.m_sPath );
		break;

	case ftpLIST_TYPE:
		// Selecting ASCII type for transfer
		strLine = _T("TYPE A");
		break;

	case ftpLIST:
		// Listing file attributes
		strLine = _T("LIST ");
		strLine += URLDecode( pURL.m_sPath );
		break;

	case ftpSIZE_TYPE:
	case ftpRETR_TYPE:
		// Selecting BINARY type for transfer
		strLine = _T("TYPE I");
		break;

	case ftpRETR_PASVPORT:
		// Selecting passive or active mode
		if ( m_bPassive )
			strLine = _T("PASV");
/*		else
		{
			SOCKADDR_IN host;
			if ( !Handshakes.Add( &m_RETR, host ) )
			{
				// Unexpected errors
				Close();
				return FALSE;
			}
			CString args;
			MakePORTArgs( host, args );
			strLine = _T("PORT ");
			strLine += args;
		}*/
		break;

	case ftpRETR_REST:
		// Restarting from offset position
		strLine.Format( _T("REST %I64u"), m_nOffset );
		break;

	case ftpRETR:
		// Retriving file
		strLine = _T("RETR ");
		strLine += URLDecode( pURL.m_sPath );
		break;

	case ftpABOR:
		// Transfer aborting
		strLine = _T("ABOR");
		break;

	default:
		return TRUE;
	}
	
	theApp.Message( MSG_DEBUG | MSG_FACILITY_OUTGOING, _T("%s << %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strLine );

	m_tRequest = GetTickCount();
	Write( strLine + _T("\r\n") );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransferFTP dropped connection handler

void CDownloadTransferFTP::OnDropped()
{
	if ( m_nState == dtsConnecting )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CONNECT_ERROR, (LPCTSTR)m_sAddress );
		if ( m_pSource != NULL )
			m_pSource->PushRequest();
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DROPPED, (LPCTSTR)m_sAddress );
	}
	Close();
}
