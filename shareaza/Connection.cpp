//
// Connection.cpp
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
#include "Connection.h"
#include "Network.h"
#include "Security.h"
#include "Buffer.h"
#include "Statistics.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define TEMP_BUFFER			10240
#define METER_SECOND		1000
#define METER_MINIMUM		100
//#define METER_LENGTH		24
//#define METER_PERIOD		2000
#define METER_LENGTH		64
#define METER_PERIOD		6000


//////////////////////////////////////////////////////////////////////
// CConnection construction

CConnection::CConnection()
{
	m_hSocket		= INVALID_SOCKET;
	m_pInput		= NULL;
	m_pOutput		= NULL;
	m_bInitiated	= FALSE;
	m_bConnected	= FALSE;
	m_tConnected	= 0;
	m_nQueuedRun	= 0;
	
	ZeroMemory( &m_mInput, sizeof(m_mInput) );
	ZeroMemory( &m_mOutput, sizeof(m_mOutput) );
}

CConnection::~CConnection()
{
	CConnection::Close();
}

//////////////////////////////////////////////////////////////////////
// CConnection connect to

BOOL CConnection::ConnectTo(SOCKADDR_IN* pHost)
{
	return ConnectTo( &pHost->sin_addr, htons( pHost->sin_port ) );
}

BOOL CConnection::ConnectTo(IN_ADDR* pAddress, WORD nPort)
{
	if ( m_hSocket != INVALID_SOCKET ) return FALSE;
	
	if ( pAddress == NULL || nPort == 0 ) return FALSE;
	if ( pAddress->S_un.S_addr == 0 ) return FALSE;
	
	if ( Security.IsDenied( pAddress ) )
	{
		theApp.Message( MSG_ERROR, IDS_NETWORK_SECURITY_OUTGOING,
			(LPCTSTR)CString( inet_ntoa( *pAddress ) ) );
		return FALSE;
	}
	
	if ( pAddress != &m_pHost.sin_addr )
	{
		ZeroMemory( &m_pHost, sizeof(m_pHost) );
		m_pHost.sin_addr	= *pAddress;
	}
	
	m_pHost.sin_family	= PF_INET;
	m_pHost.sin_port	= htons( nPort );
	
	m_sAddress		= inet_ntoa( m_pHost.sin_addr );
	m_hSocket		= socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	// m_hSocket		= WSASocket( PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED );
	
	DWORD dwValue = 1;
	ioctlsocket( m_hSocket, FIONBIO, &dwValue );
	
	if ( Settings.Connection.OutHost.GetLength() )
	{
		SOCKADDR_IN pOutgoing;
		Network.Resolve( Settings.Connection.OutHost, 0, &pOutgoing );
		if ( pOutgoing.sin_addr.S_un.S_addr )
			bind( m_hSocket, (SOCKADDR*)&pOutgoing, sizeof(SOCKADDR_IN) );
	}
	
	// if ( connect( m_hSocket, (SOCKADDR*)&m_pHost, sizeof(SOCKADDR_IN) ) ) 
	if ( WSAConnect( m_hSocket, (SOCKADDR*)&m_pHost, sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL ) )
	{
		UINT nError = WSAGetLastError();
		
		if ( nError != WSAEWOULDBLOCK )
		{
			theApp.Message( MSG_DEBUG, _T("connect() error 0x%x"), nError );
			closesocket( m_hSocket );
			m_hSocket = INVALID_SOCKET;
			return FALSE;
		}
	}
	
	if ( m_pInput ) delete m_pInput;	// Safe
	if ( m_pOutput ) delete m_pOutput;
	
	m_pInput		= new CBuffer( &Settings.Bandwidth.PeerIn );
	m_pOutput		= new CBuffer( &Settings.Bandwidth.PeerOut );
	m_bInitiated	= TRUE;
	m_tConnected	= GetTickCount();
	
	Statistics.Current.Connections.Outgoing++;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection accept an incoming connection

void CConnection::AcceptFrom(SOCKET hSocket, SOCKADDR_IN* pHost)
{
	ASSERT( m_hSocket == INVALID_SOCKET );
	
	m_pHost			= *pHost;
	m_sAddress		= inet_ntoa( m_pHost.sin_addr );
	m_hSocket		= hSocket;
	m_pInput		= new CBuffer( &Settings.Bandwidth.PeerIn );
	m_pOutput		= new CBuffer( &Settings.Bandwidth.PeerOut );
	m_bInitiated	= FALSE;
	m_bConnected	= TRUE;
	m_tConnected	= GetTickCount();
	
	DWORD dwValue = 1;
	ioctlsocket( m_hSocket, FIONBIO, &dwValue );
	
	Statistics.Current.Connections.Incoming++;
}

//////////////////////////////////////////////////////////////////////
// CConnection attach to an existing connection

void CConnection::AttachTo(CConnection* pConnection)
{
	ASSERT( m_hSocket == INVALID_SOCKET );
	ASSERT( pConnection != NULL );
	ASSERT( pConnection->m_hSocket != INVALID_SOCKET );
	
	m_pHost			= pConnection->m_pHost;
	m_sAddress		= pConnection->m_sAddress;
	m_hSocket		= pConnection->m_hSocket;
	m_bInitiated	= pConnection->m_bInitiated;
	m_bConnected	= pConnection->m_bConnected;
	m_tConnected	= pConnection->m_tConnected;
	m_pInput		= pConnection->m_pInput;
	m_pOutput		= pConnection->m_pOutput;
	m_sUserAgent	= pConnection->m_sUserAgent;
	// m_mInput		= pConnection->m_mInput;
	// m_mOutput	= pConnection->m_mOutput;
	
	m_mInput.tLast = m_mOutput.tLast = GetTickCount();
	
	pConnection->m_hSocket	= INVALID_SOCKET;
	pConnection->m_pInput	= NULL;
	pConnection->m_pOutput	= NULL;
	ZeroMemory( &pConnection->m_mInput, sizeof(m_mInput) );
	ZeroMemory( &pConnection->m_mOutput, sizeof(m_mOutput) );
}

//////////////////////////////////////////////////////////////////////
// CConnection close

void CConnection::Close()
{
	ASSERT( this != NULL );
	ASSERT( AfxIsValidAddress( this, sizeof(*this) ) );
	
	if ( m_hSocket != INVALID_SOCKET )
	{
		closesocket( m_hSocket );
		m_hSocket = INVALID_SOCKET;
	}
	
	if ( m_pOutput ) delete m_pOutput;
	m_pOutput = NULL;
	
	if ( m_pInput ) delete m_pInput;
	m_pInput = NULL;
	
	m_bConnected = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CConnection run function

BOOL CConnection::DoRun()
{
	if ( m_hSocket == INVALID_SOCKET ) return OnRun();
	
	WSANETWORKEVENTS pEvents;
	WSAEnumNetworkEvents( m_hSocket, NULL, &pEvents );
	
	if ( pEvents.lNetworkEvents & FD_CONNECT )
	{
		if ( pEvents.iErrorCode[ FD_CONNECT_BIT ] != 0 )
		{
			OnDropped( TRUE );
			return FALSE;
		}
		
		m_bConnected = TRUE;
		m_tConnected = m_mInput.tLast = m_mOutput.tLast = GetTickCount();

		if ( ! OnConnected() ) return FALSE;
	}
	
	BOOL bClosed = ( pEvents.lNetworkEvents & FD_CLOSE ) ? TRUE : FALSE;
	
	if ( bClosed ) m_mInput.pLimit = NULL;
	
	m_nQueuedRun = 1;
	
	// if ( pEvents.lNetworkEvents & FD_WRITE )
	{
		if ( ! OnWrite() ) return FALSE;
	}

	// if ( pEvents.lNetworkEvents & FD_READ )
	{
		if ( ! OnRead() ) return FALSE;
	}
	
	if ( bClosed )
	{
		OnDropped( pEvents.iErrorCode[ FD_CLOSE_BIT ] != 0 );
		return FALSE;
	}
	
	if ( ! OnRun() ) return FALSE;
	if ( m_nQueuedRun == 2 && ! OnWrite() ) return FALSE;
	m_nQueuedRun = 0;

	return TRUE;
}

void CConnection::QueueRun()
{
	if ( m_nQueuedRun )
	{
		m_nQueuedRun = 2;
	}
	else
	{
		OnWrite();
	}
}

//////////////////////////////////////////////////////////////////////
// CConnection socket event handlers

BOOL CConnection::OnConnected()
{
	return TRUE;
}

void CConnection::OnDropped(BOOL bError)
{
}

BOOL CConnection::OnRun()
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection read event handler

BOOL CConnection::OnRead()
{
	if ( m_hSocket == INVALID_SOCKET ) return FALSE;

	BYTE pData[TEMP_BUFFER];

	DWORD tNow		= GetTickCount();
	DWORD nLimit	= 0xFFFFFFFF;
	DWORD nTotal	= 0;
	
	if ( m_mInput.pLimit && *m_mInput.pLimit && ( Settings.Live.BandwidthScale <= 100 || m_mInput.bUnscaled ) )
	{
		DWORD tCutoff	= tNow - METER_SECOND;
		DWORD* pHistory	= m_mInput.pHistory;
		DWORD* pTime	= m_mInput.pTimes;
		nLimit			= 0;
		
		for ( int nSeek = METER_LENGTH ; nSeek ; nSeek--, pHistory++, pTime++ )
		{
			if ( *pTime >= tCutoff ) nLimit += *pHistory;
		}
		
		DWORD nActual = *m_mInput.pLimit;

		if ( Settings.Live.BandwidthScale < 100 && ! m_mInput.bUnscaled )
		{
			nActual = nActual * Settings.Live.BandwidthScale / 100;
		}

		tCutoff = nActual * ( tNow - m_mInput.tLastAdd ) / 1000;
		m_mInput.tLastAdd = tNow;
		
		nLimit = ( nLimit >= nActual ) ? 0 : ( nActual - nLimit );
		nLimit = min( nLimit, tCutoff );
	}

	while ( nLimit )
	{
		int nLength = min( ( nLimit & 0xFFFFF ), TEMP_BUFFER );
		
		nLength = recv( m_hSocket, (char*)pData, nLength, 0 );
		if ( nLength <= 0 ) break;
		
		m_mInput.tLast = tNow;
		
		if ( nLength > 0 && nLength <= TEMP_BUFFER )
		{
			m_pInput->Add( pData, nLength );
		}
		
		nTotal += nLength;
		if ( nLimit != 0xFFFFFFFF ) nLimit -= nLength;
	}

	if ( m_mInput.pHistory && nTotal )
	{
		if ( tNow - m_mInput.tLastSlot < METER_MINIMUM )
		{
			m_mInput.pHistory[ m_mInput.nPosition ] += nTotal;
		}
		else
		{
			m_mInput.nPosition = ( m_mInput.nPosition + 1 ) % METER_LENGTH;
			m_mInput.pTimes[ m_mInput.nPosition ]		= tNow;
			m_mInput.pHistory[ m_mInput.nPosition ]	= nTotal;
			m_mInput.tLastSlot = tNow;
		}
	}
	
	m_mInput.nTotal += nTotal;
	Statistics.Current.Bandwidth.Incoming += nTotal;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection write event handler

BOOL CConnection::OnWrite()
{
	if ( m_hSocket == INVALID_SOCKET ) return FALSE;
	if ( m_pOutput->m_nLength == 0 ) return TRUE;
	
	DWORD tNow		= GetTickCount();
	DWORD nLimit	= 0xFFFFFFFF;
	
	if ( m_mOutput.pLimit && *m_mOutput.pLimit && ( Settings.Live.BandwidthScale <= 100 || m_mOutput.bUnscaled ) )
	{
		DWORD tCutoff	= tNow - METER_SECOND;
		DWORD* pHistory	= m_mOutput.pHistory;
		DWORD* pTime	= m_mOutput.pTimes;
		DWORD nUsed		= 0;
		
		for ( int nSeek = METER_LENGTH ; nSeek ; nSeek--, pHistory++, pTime++ )
		{
			if ( *pTime >= tCutoff ) nUsed += *pHistory;
		}
		
		nLimit = *m_mOutput.pLimit;
		
		if ( Settings.Live.BandwidthScale < 100 && ! m_mOutput.bUnscaled )
		{
			nLimit = nLimit * Settings.Live.BandwidthScale / 100;
		}
		
		if ( Settings.Uploads.ThrottleMode )
		{
			nLimit = ( nUsed >= nLimit ) ? 0 : ( nLimit - nUsed );
		}
		else
		{
			tCutoff = nLimit * ( tNow - m_mOutput.tLastAdd ) / 1000;
			nLimit = ( nUsed >= nLimit ) ? 0 : ( nLimit - nUsed );
			nLimit = min( nLimit, tCutoff );
			m_mOutput.tLastAdd = tNow;
		}
	}
	
	BYTE* pBuffer = m_pOutput->m_pBuffer;
	DWORD nBuffer = m_pOutput->m_nLength;
	
	while ( nLimit && nBuffer )
	{
		int nLength = (int)min( nLimit, nBuffer );
		
		nLength = send( m_hSocket, (char*)pBuffer, nLength, 0 );
		if ( nLength <= 0 ) break;
		
		pBuffer += nLength;
		nBuffer -= nLength;
		
		if ( nLimit != 0xFFFFFFFF ) nLimit -= nLength;
	}
	
	DWORD nTotal = ( m_pOutput->m_nLength - nBuffer );
	
	if ( nTotal )
	{
		m_pOutput->Remove( nTotal );
		m_mOutput.tLast = tNow;
		
		if ( tNow - m_mOutput.tLastSlot < METER_MINIMUM )
		{
			m_mOutput.pHistory[ m_mOutput.nPosition ]	+= nTotal;
		}
		else
		{
			m_mOutput.nPosition = ( m_mOutput.nPosition + 1 ) % METER_LENGTH;
			m_mOutput.pTimes[ m_mOutput.nPosition ]		= tNow;
			m_mOutput.pHistory[ m_mOutput.nPosition ]	= nTotal;
			m_mOutput.tLastSlot = tNow;
		}
		
		m_mOutput.nTotal += nTotal;
		Statistics.Current.Bandwidth.Outgoing += nTotal;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection measure

void CConnection::Measure()
{
	DWORD tCutoff		= GetTickCount() - METER_PERIOD;
	DWORD* pInHistory	= m_mInput.pHistory;
	DWORD* pInTime		= m_mInput.pTimes;
	DWORD* pOutHistory	= m_mOutput.pHistory;
	DWORD* pOutTime		= m_mOutput.pTimes;
	DWORD nInput		= 0;
	DWORD nOutput		= 0;
	
	for ( int tNow = METER_LENGTH ; tNow ; tNow-- )
	{
		if ( *pInTime >= tCutoff ) nInput += *pInHistory;
		if ( *pOutTime >= tCutoff ) nOutput += *pOutHistory;
		pInHistory++, pInTime++;
		pOutHistory++, pOutTime++;
	}
	
	m_mInput.nMeasure	= nInput * 1000 / METER_PERIOD;
	m_mOutput.nMeasure	= nOutput * 1000 / METER_PERIOD;
}

//////////////////////////////////////////////////////////////////////
// CConnection HTML header reading

BOOL CConnection::ReadHeaders()
{
	CString strLine;
	
	while ( m_pInput->ReadLine( strLine ) )
	{
		if ( strLine.GetLength() > 20480 ) strLine = _T("#LINE_TOO_LONG#");
		
		int nPos = strLine.Find( _T(":") );
		
		if ( strLine.IsEmpty() )
		{
			m_sLastHeader.Empty();
			return OnHeadersComplete();
		}
		else if ( _istspace( strLine.GetAt( 0 ) ) )
		{
			if ( m_sLastHeader.GetLength() )
			{
				strLine.TrimLeft();
				strLine.TrimRight();
				
				if ( strLine.GetLength() > 0 )
				{
					if ( ! OnHeaderLine( m_sLastHeader, strLine ) ) return FALSE;
				}
			}
		}
		else if ( nPos > 1 && nPos < 64 )
		{
			m_sLastHeader		= strLine.Left( nPos );
			CString strValue	= strLine.Mid( nPos + 1 );
			
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

BOOL CConnection::OnHeaderLine(CString& strHeader, CString& strValue)
{
	if ( strHeader.CompareNoCase( _T("User-Agent") ) == 0 )
	{
		m_sUserAgent = strValue;
		return TRUE;
	}
	else if ( strHeader.CompareNoCase( _T("Remote-IP") ) == 0 )
	{
		Network.AcquireLocalAddress( strValue );
	}
	else if (	strHeader.CompareNoCase( _T("X-My-Address") ) == 0 ||
				strHeader.CompareNoCase( _T("Listen-IP") ) == 0 ||
				strHeader.CompareNoCase( _T("Node") ) == 0 )
	{
		int nColon = strValue.Find( ':' );
		
		if ( ! m_bInitiated && nColon > 0 )
		{
			int nPort = GNUTELLA_DEFAULT_PORT;
			
			if ( _stscanf( strValue.Mid( nColon + 1 ), _T("%lu"), &nPort ) == 1 && nPort != 0 )
			{
				m_pHost.sin_port = htons( nPort );
			}
		}
	}
	
	return TRUE;
}

BOOL CConnection::OnHeadersComplete()
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection header output helpers

BOOL CConnection::SendMyAddress()
{
	if ( Network.IsListening() )
	{
		CString strHeader;
		strHeader.Format( _T("Listen-IP: %s:%lu\r\n"),
			(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
			htons( Network.m_pHost.sin_port ) );
		m_pOutput->Print( strHeader );
		return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CConnection blocked agent filter

BOOL CConnection::IsAgentBlocked()
{
	if ( m_sUserAgent.IsEmpty() ) return FALSE;
	if ( m_sUserAgent == _T("Fake Shareaza") ) return TRUE;
	if ( Settings.Uploads.BlockAgents.IsEmpty() ) return FALSE;
	
	CString strBlocked = Settings.Uploads.BlockAgents;
	strBlocked.MakeLower();
	
	CString strAgent = m_sUserAgent;
	strAgent.MakeLower();
	
	for ( strBlocked += '|' ; strBlocked.GetLength() ; )
	{
		CString strBrowser	= strBlocked.SpanExcluding( _T("|;,") );
		strBlocked			= strBlocked.Mid( strBrowser.GetLength() + 1 );
		
		if ( strBrowser.GetLength() > 0 &&
			 strAgent.Find( strBrowser ) >= 0 ) return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CConnection URL encodings

CString CConnection::URLEncode(LPCTSTR pszInputT)
{
	static LPCTSTR pszHex	= _T("0123456789ABCDEF");
	static LPCSTR pszUnsafe	= "<>\"#%{}|\\^~[]+?&@=:,";
	
	CString strOutput;
	if ( pszInputT == NULL || *pszInputT == 0 ) return strOutput;
	
#ifdef _UNICODE
	int nUTF8 = WideCharToMultiByte( CP_UTF8, 0, pszInputT, -1, NULL, 0, NULL, NULL );
	if ( nUTF8 < 2 ) return strOutput;
	LPSTR pszUTF8 = new CHAR[ nUTF8 ];
	WideCharToMultiByte( CP_UTF8, 0, pszInputT, -1, pszUTF8, nUTF8, NULL, NULL );
	pszUTF8[ nUTF8 - 1 ] = 0;
	LPCSTR pszInput = pszUTF8;
#else
	LPCSTR pszInput = pszInputT;
#endif
	
	LPTSTR pszOutput = strOutput.GetBuffer( strlen( pszInput ) * 3 + 1 );
	
	for ( ; *pszInput ; pszInput++ )
	{
		if ( *pszInput <= 32 || *pszInput > 127 ||
			 strchr( pszUnsafe, *pszInput ) != NULL )
		{
			*pszOutput++ = _T('%');
			*pszOutput++ = pszHex[ ( *pszInput >> 4 ) & 0x0F ];
			*pszOutput++ = pszHex[ *pszInput & 0x0F ];
		}
		else
		{
			*pszOutput++ = (TCHAR)*pszInput;
		}
	}
	
	*pszOutput = 0;
	strOutput.ReleaseBuffer();
	
#ifdef _UNICODE
	delete [] pszUTF8;
#endif
	
	return strOutput;
}

CString CConnection::URLDecode(LPCTSTR pszInput)
{
	TCHAR szHex[3] = { 0, 0, 0 };
	CString strOutput;
	int nHex;
	
#ifdef _UNICODE
	LPSTR pszBytes = new CHAR[ _tcslen( pszInput ) + 1 ];
	LPSTR pszOutput = pszBytes;
#else
	LPSTR pszOutput = strOutput.GetBuffer( strlen( pszInput ) );
#endif
	
	for ( ; *pszInput ; pszInput++ )
	{
		if ( *pszInput == '%' )
		{
			if ( ! ( szHex[0] = pszInput[1] ) ) break;
			if ( ! ( szHex[1] = pszInput[2] ) ) break;
			if ( _stscanf( szHex, _T("%x"), &nHex ) != 1 ) break;
			if ( nHex < 1 ) break;
			*pszOutput++ = nHex;
			pszInput += 2;
		}
		else if ( *pszInput == '+' )
		{
			*pszOutput++ = ' ';
		}
		else
		{
			*pszOutput++ = (CHAR)*pszInput;
		}
	}
	
	*pszOutput = 0;
	
#ifdef _UNICODE
	int nLength = MultiByteToWideChar( CP_UTF8, 0, pszBytes, -1, NULL, 0 );
	MultiByteToWideChar( CP_UTF8, 0, pszBytes, -1, strOutput.GetBuffer( nLength ), nLength );
	strOutput.ReleaseBuffer();
	delete [] pszBytes;
#else
	strOutput.ReleaseBuffer();
#endif
	
	return strOutput;
}

BOOL CConnection::StartsWith(LPCTSTR pszInput, LPCTSTR pszText)
{
	return _tcsnicmp( pszInput, pszText, _tcslen( pszText ) ) == 0;
}
