//
// ShakeNeighbour.cpp
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
#include "Buffer.h"
#include "HostCache.h"
#include "DiscoveryServices.h"
#include "Neighbours.h"
#include "ShakeNeighbour.h"
#include "G1Neighbour.h"
#include "G2Neighbour.h"
#include "Packet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CShakeNeighbour construction

CShakeNeighbour::CShakeNeighbour() : CNeighbour( PROTOCOL_NULL )
{
	m_bSentAddress		= FALSE;

	m_bG2Send			= FALSE;
	m_bG2Accept			= FALSE;
	
	m_bDeflateSend		= FALSE;
	m_bDeflateAccept	= FALSE;
	//ToDo: Check this - G1 setting?
	m_bCanDeflate		= Neighbours.IsG2Leaf() ? ( Settings.Gnutella.DeflateHub2Hub || Settings.Gnutella.DeflateLeaf2Hub ) : ( Settings.Gnutella.DeflateHub2Hub || Settings.Gnutella.DeflateHub2Hub );
	
	m_bUltraPeerSet		= TS_UNKNOWN;
	m_bUltraPeerNeeded	= TS_UNKNOWN;
	m_bUltraPeerLoaded	= TS_UNKNOWN;
}

CShakeNeighbour::~CShakeNeighbour()
{
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour connect to

BOOL CShakeNeighbour::ConnectTo(IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic, BOOL bNoUltraPeer)
{
	if ( CConnection::ConnectTo( pAddress, nPort ) )
	{
		WSAEventSelect( m_hSocket, Network.m_pWakeup, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE );
		
		theApp.Message( MSG_DEFAULT, IDS_CONNECTION_ATTEMPTING,
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
	m_bUltraPeerSet	= bNoUltraPeer ? TS_FALSE : TS_UNKNOWN;
	
	Neighbours.Add( this );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour accept from

void CShakeNeighbour::AttachTo(CConnection* pConnection)
{
	CConnection::AttachTo( pConnection );
	WSAEventSelect( m_hSocket, Network.m_pWakeup, FD_READ|FD_WRITE|FD_CLOSE );
	
	m_nState = nrsHandshake1;
	
	Neighbours.Add( this );
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour close

void CShakeNeighbour::Close(UINT nError, BOOL bRetry04)
{
	if ( m_bInitiated )
	{
		if ( bRetry04 && m_bShake06 && m_nState == nrsHandshake1 && ( Settings.Gnutella1.EnableToday && Settings.Gnutella1.Handshake04 ) )
		{
			Neighbours.Remove( this );
			CConnection::Close();
			
			m_bShake06 = FALSE;
			theApp.Message( MSG_ERROR, IDS_HANDSHAKE_RETRY, (LPCTSTR)m_sAddress );
			
			IN_ADDR pAddress = m_pHost.sin_addr;
			WORD nPort = htons( m_pHost.sin_port );
			
			ConnectTo( &pAddress, nPort, m_bAutomatic );
			
			return;
		}
		else if ( m_nState < nrsHandshake2 )
		{
			HostCache.OnFailure( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
		}
	}
	
	CNeighbour::Close( nError );
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour connection event

BOOL CShakeNeighbour::OnConnected()
{
	CConnection::OnConnected();

	theApp.Message( MSG_DEFAULT, IDS_CONNECTION_CONNECTED, (LPCTSTR)m_sAddress );

	if ( m_bShake06 )
	{
		m_pOutput->Print( "GNUTELLA CONNECT/0.6\r\n" );
		SendPublicHeaders();
		SendHostHeaders();
		m_pOutput->Print( "\r\n" );
	}
	else
	{
		m_pOutput->Print( "GNUTELLA CONNECT/0.4\n\n" );
	}
	
	m_nState = nrsHandshake1;

	OnWrite();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour connection loss event

void CShakeNeighbour::OnDropped(BOOL bError)
{
	if ( m_nState == nrsConnecting )
	{
		Close( IDS_CONNECTION_REFUSED );
	}
	else
	{
		Close( IDS_CONNECTION_DROPPED, TRUE );
	}
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour read and write events

BOOL CShakeNeighbour::OnRead()
{
	CConnection::OnRead();

	if ( m_nState == nrsHandshake1 && ! ReadResponse() ) return FALSE;
	if ( m_nState == nrsHandshake2 && ! ReadHeaders() )  return FALSE;
	if ( m_nState == nrsHandshake1 && ! ReadResponse() ) return FALSE;
	if ( m_nState == nrsHandshake3 && ! ReadHeaders() )  return FALSE;
	if ( m_nState == nrsRejected   && ! ReadHeaders() )  return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour run event

BOOL CShakeNeighbour::OnRun()
{
	DWORD nTimeNow = GetTickCount();
	
	switch ( m_nState )
	{
	case nrsConnecting:

		if ( nTimeNow - m_tConnected > Settings.Connection.TimeoutConnect )
		{
			DiscoveryServices.OnGnutellaFailed( &m_pHost.sin_addr );
			Close( IDS_CONNECTION_TIMEOUT_CONNECT );
			return FALSE;
		}
		break;

	case nrsHandshake1:
	case nrsHandshake2:
	case nrsHandshake3:
	case nrsRejected:

		if ( nTimeNow - m_tConnected > Settings.Connection.TimeoutHandshake )
		{
			Close( IDS_HANDSHAKE_TIMEOUT, TRUE );
			return FALSE;
		}
		break;

	case nrsClosing:

		Close( 0 );
		return FALSE;

	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour handshake header dispatch

void CShakeNeighbour::SendMinimalHeaders()
{
	CString strHeader = Settings.SmartAgent( Settings.General.UserAgent );
	
	if ( strHeader.GetLength() )
	{
		strHeader = _T("User-Agent: ") + strHeader + _T("\r\n");
		m_pOutput->Print( strHeader );
	}
	
	if ( Settings.Gnutella2.EnableToday )
	{
		if ( m_bInitiated )
		{
			m_pOutput->Print( "Accept: application/x-gnutella2,application/x-gnutella-packets\r\n" );
		}
		else if ( m_bG2Accept )
		{
			m_pOutput->Print( "Accept: application/x-gnutella2\r\n" );
			m_pOutput->Print( "Content-Type: application/x-gnutella2\r\n" );
		}
	}
}

void CShakeNeighbour::SendPublicHeaders()
{
	CString strHeader = Settings.SmartAgent( Settings.General.UserAgent );
	
	if ( strHeader.GetLength() )
	{
		strHeader = _T("User-Agent: ") + strHeader + _T("\r\n");
		m_pOutput->Print( strHeader );
	}
	
	m_bSentAddress |= SendMyAddress();
	
	strHeader.Format( _T("Remote-IP: %s\r\n"),
		(LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ) );
	m_pOutput->Print( strHeader );
	
	if ( Settings.Gnutella2.EnableToday )
	{
		if ( m_bInitiated )
		{
			m_pOutput->Print( "Accept: application/x-gnutella2,application/x-gnutella-packets\r\n" );
		}
		else if ( m_bG2Accept )
		{
			m_pOutput->Print( "Accept: application/x-gnutella2\r\n" );
			m_pOutput->Print( "Content-Type: application/x-gnutella2\r\n" );
		}
	}
	
	if ( m_bCanDeflate )
	{
		m_pOutput->Print( "Accept-Encoding: deflate\r\n" );
		
		if ( ! m_bInitiated && m_bDeflateAccept )
		{
			m_pOutput->Print( "Content-Encoding: deflate\r\n" );
		}
	}
	
	if ( Settings.Gnutella1.EnableToday )
	{
		if ( Settings.Gnutella1.EnableGGEP ) m_pOutput->Print( "GGEP: 0.5\r\n" );
		m_pOutput->Print( "Pong-Caching: 0.1\r\n" );
		if ( Settings.Gnutella1.VendorMsg ) m_pOutput->Print( "Vendor-Message: 0.1\r\n" );
		m_pOutput->Print( "X-Query-Routing: 0.1\r\n" );
	}
	
	if ( m_bInitiated && m_bUltraPeerSet == TS_FALSE )
	{
		m_bUltraPeerSet = TS_UNKNOWN;
	}
	else
	{
		if ( 1 )	//Add proper G2 detection here
		{
			if ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() )
			{
				m_pOutput->Print( "X-Ultrapeer: True\r\n" );
			}
			else if ( Settings.Gnutella2.ClientMode != MODE_HUB ) //( Settings.Gnutella.LeafEnable )
			{
				m_pOutput->Print( "X-Ultrapeer: False\r\n" );
			}
		}
		else
		{
			if ( Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() )
			{
				m_pOutput->Print( "X-Ultrapeer: True\r\n" );
			}
			else if ( Settings.Gnutella1.ClientMode != MODE_ULTRAPEER )
			{
				m_pOutput->Print( "X-Ultrapeer: False\r\n" );
			}
		}
	}
}

void CShakeNeighbour::SendPrivateHeaders()
{
	if ( ! m_bSentAddress ) m_bSentAddress = SendMyAddress();
	
	if ( m_bInitiated && m_bG2Send && m_bG2Accept )
	{
		m_pOutput->Print( "Accept: application/x-gnutella2\r\n" );
		m_pOutput->Print( "Content-Type: application/x-gnutella2\r\n" );
	}
	
	if ( m_bInitiated )
	{
		if ( m_bDeflateSend ) 
		{
			m_pOutput->Print( "Accept-Encoding: deflate\r\n" );
		}
		if ( m_bDeflateAccept ) 
		{
			m_pOutput->Print( "Content-Encoding: deflate\r\n" );
		}
	}
}

void CShakeNeighbour::SendHostHeaders(LPCTSTR pszMessage)
{
	DWORD nTime = time( NULL );
	CString strHosts, strHost;
	CHostCacheHost* pHost;
	
	if ( pszMessage )
	{
		m_pOutput->Print( pszMessage );
		m_pOutput->Print( "\r\n" );
		SendMinimalHeaders();
	}
	
	int nCount = Settings.Gnutella1.PongCount;
	
	if ( m_bG2Accept || m_bG2Send || m_bShareaza )
	{
		for ( pHost = HostCache.Gnutella2.GetNewest() ; pHost && nCount > 0 ; pHost = pHost->m_pPrevTime )
		{
			if ( pHost->CanQuote( nTime ) )
			{
				strHost = pHost->ToString();
				if ( strHosts.GetLength() ) strHosts += _T(",");
				strHosts += strHost;
				nCount--;
			}
		}
	}
	else
	{
		for ( pHost = HostCache.Gnutella1.GetNewest() ; pHost && nCount > 0 ; pHost = pHost->m_pPrevTime )
		{
			if ( pHost->CanQuote( nTime ) )
			{
				strHost = pHost->ToString();
				if ( strHosts.GetLength() ) strHosts += _T(",");
				strHosts += strHost;
				nCount--;
			}
		}
	}
	
	if ( strHosts.GetLength() )
	{
		m_pOutput->Print( "X-Try-Ultrapeers: " );
		m_pOutput->Print( strHosts );
		m_pOutput->Print( "\r\n" );
	}

	if ( pszMessage ) m_pOutput->Print( "\r\n" );
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour handshake response processor

BOOL CShakeNeighbour::ReadResponse()
{
	CString strLine;

	if ( ! m_pInput->ReadLine( strLine ) ) return TRUE;
	if ( strLine.GetLength() > 1024 ) strLine = _T("#LINE_TOO_LONG#");

	theApp.Message( MSG_DEBUG, _T("%s: HANDSHAKE: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strLine );

	if ( strLine == _T("GNUTELLA OK") && m_bInitiated )
	{
		m_bShake06 = FALSE;
		OnHandshakeComplete();
		return FALSE;
	}
	else if ( strLine.GetLength() > 13 && strLine.Left( 12 ) == _T("GNUTELLA/0.6") )
	{
		if ( strLine != _T("GNUTELLA/0.6 200 OK") )
		{
			strLine = strLine.Mid( 13 );
			theApp.Message( MSG_ERROR, IDS_HANDSHAKE_REJECTED, (LPCTSTR)m_sAddress, (LPCTSTR)strLine );
			m_nState = nrsRejected;
		}
		else if ( ! m_bInitiated )
		{
			m_nState = nrsHandshake3;
		}
		else
		{
			m_bShake06 = TRUE;
			m_nState = nrsHandshake2;
		}
	}
	else if ( strLine == _T("GNUTELLA CONNECT/0.4") && ! m_bInitiated )
	{
		if ( Neighbours.IsG1Leaf() )
		{
			DelayClose( IDS_HANDSHAKE_IAMLEAF );
			return FALSE;
		}
		else if ( ! Neighbours.NeedMoreHubs( TS_FALSE ) )
		{
			DelayClose( IDS_HANDSHAKE_SURPLUS );
			return FALSE;
		}
		else
		{
			m_bShake06 = FALSE;
			m_pOutput->Print( "GNUTELLA OK\n\n" );
			OnHandshakeComplete();
			return FALSE;
		}
	}
	else if ( strLine.GetLength() > 17 && strLine.Left( 17 ) == _T("GNUTELLA CONNECT/") && ! m_bInitiated )
	{
		m_bShake06 = TRUE;
		m_nState = nrsHandshake2;
	}
	else if ( strLine.GetLength() > 21 && strLine.Left( 21 ) == _T("SHAREAZABETA CONNECT/") && ! m_bInitiated )
	{
		m_bShake06 = TRUE;
		m_nState = nrsHandshake2;
	}
	else
	{
		Close( IDS_HANDSHAKE_FAIL, TRUE );
		return FALSE;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour handshake header processing

BOOL CShakeNeighbour::OnHeaderLine(CString& strHeader, CString& strValue)
{
	theApp.Message( MSG_DEBUG, _T("%s: GNUTELLA HEADER: %s: %s"),
		(LPCTSTR)m_sAddress, (LPCTSTR)strHeader, (LPCTSTR)strValue );
	
	if ( strHeader.CompareNoCase( _T("User-Agent") ) == 0 )
	{
		m_sUserAgent = strValue;
		if ( _tcsistr( m_sUserAgent, _T("Shareaza") ) ) m_bShareaza = TRUE;
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

		if ( nColon > 0 )
		{
			int nPort = GNUTELLA_DEFAULT_PORT;

			if ( _stscanf( strValue.Mid( nColon + 1 ), _T("%lu"), &nPort ) == 1 && nPort != 0 )
			{
				m_pHost.sin_port = htons( nPort );
			}
		}
	}
	else if ( strHeader.CompareNoCase( _T("Pong-Caching") ) == 0 )
	{
		m_bPongCaching = TRUE;
	}
	else if ( strHeader.CompareNoCase( _T("Vendor-Message") ) == 0 )
	{
		m_bVendorMsg = TRUE;
	}
	else if ( strHeader.CompareNoCase( _T("X-Query-Routing") ) == 0 )
	{
		m_bQueryRouting =  ( strValue != _T("0") && strValue != _T("0.0") );
	}
	else if (	strHeader.CompareNoCase( _T("X-Ultrapeer") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Hub") ) == 0 )
	{
		m_bUltraPeerSet = ( strValue.CompareNoCase( _T("True") ) == 0 ) ? TS_TRUE : TS_FALSE;
	}
	else if (	strHeader.CompareNoCase( _T("X-Ultrapeer-Needed") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Hub-Needed") ) == 0 )
	{
		m_bUltraPeerNeeded = ( strValue.CompareNoCase( _T("True") ) == 0 ) ? TS_TRUE : TS_FALSE;
	}
	else if (	strHeader.CompareNoCase( _T("X-Ultrapeer-Loaded") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Hub-Loaded") ) == 0 )
	{
		m_bUltraPeerLoaded = ( strValue.CompareNoCase( _T("True") ) == 0 ) ? TS_TRUE : TS_FALSE;
	}
	else if ( strHeader.CompareNoCase( _T("GGEP") ) == 0 )
	{
		if ( Settings.Gnutella1.EnableGGEP )
		{
			m_bGGEP = ( strValue != _T("0") && strValue != _T("0.0") );
		}
	}
	else if ( strHeader.CompareNoCase( _T("Accept") ) == 0 && Settings.Gnutella2.EnableToday )
	{
		m_bG2Accept |= ( strValue.Find( _T("application/x-gnutella2") ) >= 0 );
		m_bG2Accept |= ( strValue.Find( _T("application/x-shareaza") ) >= 0 );
	}
	else if ( strHeader.CompareNoCase( _T("Content-Type") ) == 0 && Settings.Gnutella2.EnableToday )
	{
		m_bG2Send |= ( strValue.Find( _T("application/x-gnutella2") ) >= 0 );
		m_bG2Send |= ( strValue.Find( _T("application/x-shareaza") ) >= 0 );
	}
	else if ( strHeader.CompareNoCase( _T("Accept-Encoding") ) == 0 && m_bCanDeflate )
	{
		m_bDeflateAccept |= ( strValue.Find( _T("deflate") ) >= 0 );
	}
	else if ( strHeader.CompareNoCase( _T("Content-Encoding") ) == 0 && m_bCanDeflate )
	{
		m_bDeflateSend |= ( strValue.Find( _T("deflate") ) >= 0 );
	}
	else if (	strHeader.CompareNoCase( _T("X-Try-Ultrapeers") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Try-Hubs") ) == 0 )
	{
		int nCount = 0;

		for ( strValue += ',' ; ; )
		{
			int nPort	= Settings.Connection.InPort;
			int nPos	= strValue.Find( ',' );
			
			if ( nPos < 0 ) break;
			
			CString strHost = strValue.Left( nPos );
			strValue = strValue.Mid( nPos + 1 );
			
			if ( m_bG2Accept || m_bG2Send || m_bShareaza )
			{
				if ( HostCache.Gnutella2.Add( strHost, 0, m_bShareaza ? SHAREAZA_VENDOR_T : NULL ) ) nCount++;
			}
			else
			{
				if ( HostCache.Gnutella1.Add( strHost, 0, m_bShareaza ? SHAREAZA_VENDOR_T : NULL ) ) nCount++;
			}
		}
		
		DiscoveryServices.OnGnutellaAdded( &m_pHost.sin_addr, nCount );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour handshake end of headers

BOOL CShakeNeighbour::OnHeadersComplete()
{
	if ( ! m_bG2Accept || ( m_bInitiated && ! m_bG2Send ) )
		return OnHeadersCompleteG1();
	else
		return OnHeadersCompleteG2();
}

BOOL CShakeNeighbour::OnHeadersCompleteG2()
{
	theApp.Message( MSG_DEFAULT, _T("Headers Complete: G2 client") ); //****temp
/*
//Shouldn't be needed any more. G1 clients should head on over to the other function
	if ( ! Settings.Gnutella1.EnableToday && m_nState < nrsRejected )
	{
		if ( ! m_bG2Accept || ( m_bInitiated && ! m_bG2Send ) )
		{
			m_pOutput->Print( "GNUTELLA/0.6 503 G2 Required\r\n" );
			SendMinimalHeaders();
			m_pOutput->Print( "\r\n" );
			HostCache.OnFailure( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
			DelayClose( IDS_HANDSHAKE_NOTG2 );
			return FALSE;
		}
	}
*/
	
	if ( m_nState == nrsRejected )
	{
		Close( 0 );
		return FALSE;
	}
	else if ( m_nState == nrsHandshake3 )
	{
		if ( m_bUltraPeerSet == TS_FALSE && m_nNodeType == ntNode &&
			 ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() ) )
		{
			theApp.Message( MSG_DEFAULT, IDS_HANDSHAKE_BACK2LEAF, (LPCTSTR)m_sAddress );
			m_nNodeType = ntLeaf;
		}
		
		OnHandshakeComplete();
		return FALSE;
	}
	else if ( m_bInitiated )
	{
		BOOL bFallback = FALSE;

		if ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() )
		{
			if ( m_bUltraPeerSet == TS_FALSE )
			{
				m_nNodeType = ntLeaf;
			}
			else if (	m_bUltraPeerSet == TS_TRUE && m_bUltraPeerNeeded == TS_TRUE
						/* && Network.NeedMoreHubs() */ )
			{
				m_nNodeType = ntNode;
			}
			else if ( m_bUltraPeerSet == TS_TRUE && m_bUltraPeerNeeded == TS_FALSE )
			{
				if ( Neighbours.GetCount( PROTOCOL_G2, nrsConnected, ntLeaf ) > 0 )
				{
					SendHostHeaders( _T("GNUTELLA/0.6 503 I have leaves") );
					DelayClose( IDS_HANDSHAKE_CANTBEPEER );
					return FALSE;
				}

				if ( Settings.Gnutella2.ClientMode == MODE_HUB )
				{
					SendHostHeaders( _T("GNUTELLA/0.6 503 Ultrapeer disabled") );
					DelayClose( IDS_HANDSHAKE_NOULTRAPEER );
					return FALSE;
				}

				m_nNodeType = ntHub;
				bFallback = TRUE;
			}
		}
		else if ( m_bUltraPeerSet == TS_TRUE )
		{
			if ( Settings.Gnutella2.ClientMode == MODE_HUB )
			{
				SendHostHeaders( _T("GNUTELLA/0.6 503 Ultrapeer disabled") );
				DelayClose( IDS_HANDSHAKE_NOULTRAPEER );
				return FALSE;
			}

			m_nNodeType = ntHub;
		}
		else if ( m_bUltraPeerSet != TS_TRUE )
		{
			if ( Settings.Gnutella2.ClientMode == MODE_LEAF )
			{
				SendHostHeaders( _T("GNUTELLA/0.6 503 Need an Ultrapeer") );
				DelayClose( IDS_HANDSHAKE_NEEDAPEER );
				return FALSE;
			}
		}

		m_pOutput->Print( "GNUTELLA/0.6 200 OK\r\n" );
		SendPrivateHeaders();
		if ( bFallback ) m_pOutput->Print( "X-Ultrapeer: False\r\n" );
		m_pOutput->Print( "\r\n" );

		OnHandshakeComplete();
		return FALSE;
	}
	else
	{
		if ( Neighbours.IsG2Leaf() )
		{
			SendHostHeaders( _T("GNUTELLA/0.6 503 Shielded leaf node") );
			DelayClose( IDS_HANDSHAKE_IAMLEAF );
			return FALSE;
		}
		else if ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() )
		{
			if ( m_bUltraPeerSet == TS_FALSE )
			{
				m_nNodeType = ntLeaf;
			}
			else if ( m_bUltraPeerSet == TS_TRUE )
			{
				m_nNodeType = ntNode;
			}
		}
		else if ( m_bUltraPeerSet == TS_TRUE && ( Settings.Gnutella2.ClientMode != MODE_HUB ) )
		{
			m_nNodeType = ntHub;
		}
		
		if ( ( m_nNodeType == ntLeaf && ! Neighbours.NeedMoreHubs( TS_TRUE ) &&
			 ! Neighbours.NeedMoreLeafs( TS_TRUE ) ) ||
			 ( m_nNodeType != ntLeaf && ! Neighbours.NeedMoreHubs( TS_TRUE ) ) )
		{
			SendHostHeaders( _T("GNUTELLA/0.6 503 Maximum connections reached") );
			DelayClose( IDS_HANDSHAKE_SURPLUS );
			return FALSE;
		}
		else if ( ( m_nNodeType == ntHub && ( Settings.Gnutella2.ClientMode == MODE_HUB ) ) ||
				  ( m_nNodeType == ntLeaf && ( Settings.Gnutella2.ClientMode == MODE_LEAF ) ) )
		{
			SendHostHeaders( _T("GNUTELLA/0.6 503 Ultrapeer disabled") );
			DelayClose( IDS_HANDSHAKE_NOULTRAPEER );
			return FALSE;
		}
		else if ( m_nNodeType != ntHub && ( Settings.Gnutella2.ClientMode == MODE_LEAF ) )
		{
			SendHostHeaders( _T("GNUTELLA/0.6 503 Need an Ultrapeer") );
			DelayClose( IDS_HANDSHAKE_NEEDAPEER );
			return FALSE;
		}
		else
		{
			m_pOutput->Print( "GNUTELLA/0.6 200 OK\r\n" );
			SendPublicHeaders();
			SendPrivateHeaders();
			SendHostHeaders();

			if ( m_nNodeType != ntLeaf &&
				 Neighbours.NeedMoreHubs( TS_TRUE ) )
			{
				if ( Settings.Gnutella2.ClientMode != MODE_LEAF )
				{
					m_pOutput->Print( "X-Ultrapeer-Needed: True\r\n" );
				}
			}
			else
			{
				if ( Settings.Gnutella2.ClientMode != MODE_HUB )
				{
					m_pOutput->Print( "X-Ultrapeer-Needed: False\r\n" );
				}
			}

			m_pOutput->Print( "\r\n" );
			m_nState = nrsHandshake1;
		}
	}

	return TRUE;
}

BOOL CShakeNeighbour::OnHeadersCompleteG1()
{
	theApp.Message( MSG_DEFAULT, _T("Headers Complete: G1 client") ); //****temp

	//Check if Gnutella1 is enabled before connecting to a gnutella client
	if ( ! Settings.Gnutella1.EnableToday && m_nState < nrsRejected )
	{
		m_pOutput->Print( "GNUTELLA/0.6 503 G2 Required\r\n" );
		SendMinimalHeaders();
		m_pOutput->Print( "\r\n" );
		HostCache.OnFailure( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
		DelayClose( IDS_HANDSHAKE_NOTG2 );
		return FALSE;
	}
	//
	
	if ( m_nState == nrsRejected )
	{
		Close( 0 );
		return FALSE;
	}
	else if ( m_nState == nrsHandshake3 )
	{
		if ( m_bUltraPeerSet == TS_FALSE && m_nNodeType == ntNode &&
			 ( Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() ) )
		{
			theApp.Message( MSG_DEFAULT, IDS_HANDSHAKE_BACK2LEAF, (LPCTSTR)m_sAddress );
			m_nNodeType = ntLeaf;
		}
		
		OnHandshakeComplete();
		return FALSE;
	}
	else if ( m_bInitiated )
	{
		BOOL bFallback = FALSE;

		if ( Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() )
		{
			if ( m_bUltraPeerSet == TS_FALSE )
			{
				m_nNodeType = ntLeaf;
			}
			else if (	m_bUltraPeerSet == TS_TRUE && m_bUltraPeerNeeded == TS_TRUE )
			{
				m_nNodeType = ntNode;
			}
			else if ( m_bUltraPeerSet == TS_TRUE && m_bUltraPeerNeeded == TS_FALSE )
			{
				if ( Neighbours.GetCount( PROTOCOL_G1, nrsConnected, ntLeaf ) > 0 )
				{
					SendHostHeaders( _T("GNUTELLA/0.6 503 I have leaves") );
					DelayClose( IDS_HANDSHAKE_CANTBEPEER );
					return FALSE;
				}

				if ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER )
				{
					SendHostHeaders( _T("GNUTELLA/0.6 503 Ultrapeer disabled") );
					DelayClose( IDS_HANDSHAKE_NOULTRAPEER );
					return FALSE;
				}

				m_nNodeType = ntHub;
				bFallback = TRUE;
			}
		}
		else if ( m_bUltraPeerSet == TS_TRUE )
		{
			if ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER )
			{
				SendHostHeaders( _T("GNUTELLA/0.6 503 Ultrapeer disabled") );
				DelayClose( IDS_HANDSHAKE_NOULTRAPEER );
				return FALSE;
			}

			m_nNodeType = ntHub;
		}
		else if ( m_bUltraPeerSet != TS_TRUE )
		{
			if ( Settings.Gnutella1.ClientMode == MODE_LEAF )
			{
				SendHostHeaders( _T("GNUTELLA/0.6 503 Need an Ultrapeer") );
				DelayClose( IDS_HANDSHAKE_NEEDAPEER );
				return FALSE;
			}
		}

		m_pOutput->Print( "GNUTELLA/0.6 200 OK\r\n" );
		SendPrivateHeaders();
		if ( bFallback ) m_pOutput->Print( "X-Ultrapeer: False\r\n" );
		m_pOutput->Print( "\r\n" );

		OnHandshakeComplete();
		return FALSE;
	}
	else
	{
		if ( Neighbours.IsG1Leaf() )
		{
			SendHostHeaders( _T("GNUTELLA/0.6 503 Shielded leaf node") );
			DelayClose( IDS_HANDSHAKE_IAMLEAF );
			return FALSE;
		}
		else if ( Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() )
		{
			if ( m_bUltraPeerSet == TS_FALSE )
			{
				m_nNodeType = ntLeaf;
			}
			else if ( m_bUltraPeerSet == TS_TRUE )
			{
				m_nNodeType = ntNode;
			}
		}
		else if ( m_bUltraPeerSet == TS_TRUE && ( Settings.Gnutella1.ClientMode != MODE_ULTRAPEER ) )
		{
			m_nNodeType = ntHub;
		}
		
		if ( ( m_nNodeType == ntLeaf && ! Neighbours.NeedMoreHubs( TS_FALSE ) &&
			 ! Neighbours.NeedMoreLeafs( TS_FALSE ) ) ||
			 ( m_nNodeType != ntLeaf && ! Neighbours.NeedMoreHubs( TS_FALSE ) ) )
		{
			SendHostHeaders( _T("GNUTELLA/0.6 503 Maximum connections reached") );
			DelayClose( IDS_HANDSHAKE_SURPLUS );
			return FALSE;
		}
		else if ( ( m_nNodeType == ntHub && ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER ) ) ||
				  ( m_nNodeType == ntLeaf && ( Settings.Gnutella1.ClientMode == MODE_LEAF ) ) )
		{
			SendHostHeaders( _T("GNUTELLA/0.6 503 Ultrapeer disabled") );
			DelayClose( IDS_HANDSHAKE_NOULTRAPEER );
			return FALSE;
		}
		else if ( m_nNodeType != ntHub && ( Settings.Gnutella1.ClientMode == MODE_LEAF ) )
		{
			SendHostHeaders( _T("GNUTELLA/0.6 503 Need an Ultrapeer") );
			DelayClose( IDS_HANDSHAKE_NEEDAPEER );
			return FALSE;
		}
		else
		{
			m_pOutput->Print( "GNUTELLA/0.6 200 OK\r\n" );
			SendPublicHeaders();
			SendPrivateHeaders();
			SendHostHeaders();

			if ( m_nNodeType != ntLeaf && Neighbours.NeedMoreHubs( TS_FALSE ) )
			{
				if ( Settings.Gnutella1.ClientMode != MODE_LEAF )
				{
					m_pOutput->Print( "X-Ultrapeer-Needed: True\r\n" );
				}
			}
			else
			{
				if ( Settings.Gnutella1.ClientMode != MODE_ULTRAPEER )
				{
					m_pOutput->Print( "X-Ultrapeer-Needed: False\r\n" );
				}
			}

			m_pOutput->Print( "\r\n" );
			m_nState = nrsHandshake1;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour handshake completed

void CShakeNeighbour::OnHandshakeComplete()
{
	Neighbours.Remove( this );

	switch ( m_nNodeType )
	{
	case ntHub:
		m_mInput.pLimit		= &Settings.Bandwidth.HubIn;
		m_mOutput.pLimit	= &Settings.Bandwidth.HubOut;
		break;
	case ntLeaf:
		m_mInput.pLimit		= &Settings.Bandwidth.LeafIn;
		m_mOutput.pLimit	= &Settings.Bandwidth.LeafOut;
		break;
	case ntNode:
		m_mInput.pLimit		= &Settings.Bandwidth.PeerIn;
		m_mOutput.pLimit	= &Settings.Bandwidth.PeerOut;
		break;
	}

	if ( m_bDeflateSend )	m_pZInput	= new CBuffer();
	if ( m_bDeflateAccept )	m_pZOutput	= new CBuffer();

	if ( Settings.Connection.SendBuffer )
	{
		setsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF, (LPSTR)&Settings.Connection.SendBuffer, 4 );
	}

	CNeighbour* pNeighbour = NULL;

	if ( m_bG2Send && m_bG2Accept )
	{
		m_bQueryRouting = TRUE;
		pNeighbour = new CG2Neighbour( this );
	}
	else
	{
		m_bShareaza = FALSE;
		pNeighbour = new CG1Neighbour( this );
	}

	if ( m_nNodeType == ntHub )
	{
		theApp.Message( MSG_DEFAULT, IDS_HANDSHAKE_GOTPEER );
		Neighbours.PeerPrune();
	}
	else if ( m_nNodeType == ntLeaf )
	{
		theApp.Message( MSG_DEFAULT, IDS_HANDSHAKE_GOTLEAF, (LPCTSTR)m_sAddress );
	}

	m_pZInput	= NULL;
	m_pZOutput	= NULL;
	
	delete this;
}
