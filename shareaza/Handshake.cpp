//
// Handshake.cpp
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
#include "Handshakes.h"
#include "Handshake.h"
#include "Neighbours.h"
#include "Downloads.h"
#include "Uploads.h"
#include "UploadTransfer.h"
#include "ChatCore.h"
#include "Network.h"
#include "Buffer.h"
#include "GProfile.h"

#include "BTClients.h"
#include "EDClients.h"
#include "EDPacket.h"

#include "WndMain.h"
#include "WndChat.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CHandshake construction

CHandshake::CHandshake()
{
	m_bPushing = FALSE;
	m_mInput.pLimit = m_mOutput.pLimit = &Settings.Bandwidth.Request;
}

CHandshake::CHandshake(SOCKET hSocket, SOCKADDR_IN* pHost)
{
	m_bPushing = FALSE;
	
	AcceptFrom( hSocket, pHost );
	
	m_mInput.pLimit = m_mOutput.pLimit = &Settings.Bandwidth.Request;
	
	theApp.Message( MSG_DEFAULT, IDS_CONNECTION_ACCEPTED,
		(LPCTSTR)m_sAddress, htons( m_pHost.sin_port ) );
}

CHandshake::CHandshake(CHandshake* pCopy)
{
	AttachTo( pCopy );
	
	m_bPushing		= pCopy->m_bPushing;
	m_nIndex		= pCopy->m_nIndex;
	
	m_mInput.pLimit = m_mOutput.pLimit = &Settings.Bandwidth.Request;
}

CHandshake::~CHandshake()
{
}

//////////////////////////////////////////////////////////////////////
// CHandshake push

BOOL CHandshake::Push(IN_ADDR* pAddress, WORD nPort, DWORD nIndex)
{
	theApp.Message( MSG_DEFAULT, IDS_UPLOAD_CONNECT,
		(LPCTSTR)CString( inet_ntoa( *pAddress ) ), _T("") );
	
	if ( ! ConnectTo( pAddress, nPort ) ) return FALSE;
	
	WSAEventSelect( m_hSocket, Handshakes.m_pWakeup, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE );
	
	m_bPushing		= TRUE;
	m_tConnected	= GetTickCount();
	m_nIndex		= nIndex;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHandshake run event

BOOL CHandshake::OnRun()
{
	if ( GetTickCount() - m_tConnected > Settings.Connection.TimeoutHandshake )
	{
		theApp.Message( MSG_ERROR, IDS_HANDSHAKE_TIMEOUT, (LPCTSTR)m_sAddress );
		return FALSE;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHandshake connection events

BOOL CHandshake::OnConnected()
{
	CConnection::OnConnected();
	
	CGUID* pID = &MyProfile.GUID;
	CString strGIV;
	
	strGIV.Format( _T("GIV %lu:%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X/\n\n"), m_nIndex,
		pID->m_b[  0 ], pID->m_b[  1 ], pID->m_b[  2 ], pID->m_b[  3 ],
		pID->m_b[  4 ], pID->m_b[  5 ], pID->m_b[  6 ], pID->m_b[  7 ],
		pID->m_b[  8 ], pID->m_b[  9 ], pID->m_b[ 10 ], pID->m_b[ 11 ],
		pID->m_b[ 12 ], pID->m_b[ 13 ], pID->m_b[ 14 ], pID->m_b[ 15 ] );
	
	m_pOutput->Print( strGIV );
	OnWrite();
	
	theApp.Message( MSG_DEFAULT, IDS_UPLOAD_GIV, (LPCTSTR)m_sAddress );
	
	return TRUE;
}

void CHandshake::OnDropped(BOOL bError)
{
	if ( m_bPushing )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_CONNECT_ERROR, (LPCTSTR)m_sAddress );
	}
}

//////////////////////////////////////////////////////////////////////
// CHandshake read event

BOOL CHandshake::OnRead()
{
	CConnection::OnRead();
	
	// Minimum length to make a decision
	
	if ( m_pInput->m_nLength < 7 ) return TRUE;
	
	// Check for eDonkey2000 packet
	
	if ( m_pInput->m_nLength >= 7 &&
		 m_pInput->m_pBuffer[0] == ED2K_PROTOCOL_EDONKEY &&
		 m_pInput->m_pBuffer[5] == ED2K_C2C_HELLO &&
		 m_pInput->m_pBuffer[6] == 0x10 )
	{
		EDClients.OnAccept( this );
		return FALSE;
	}
	
	// Check for BitTorrent handshake

	if ( m_pInput->m_nLength >= BT_PROTOCOL_HEADER_LEN &&
		 memcmp( m_pInput->m_pBuffer, BT_PROTOCOL_HEADER, BT_PROTOCOL_HEADER_LEN ) == 0 )
	{
		BTClients.OnAccept( this );
		return FALSE;
	}
	
	// Check for text-based handshakes (everything else)
	
	CString strLine;
	
	if ( ! m_pInput->ReadLine( strLine, TRUE ) )
	{
		// More than 2048 bytes with no handshake, abort
		return ( m_pInput->m_nLength < 2048 ) ? TRUE : FALSE;
	}
	
	if ( strLine.IsEmpty() )
	{
		// Eat zero lines
		m_pInput->ReadLine( strLine );
		return TRUE;
	}
	
	theApp.Message( MSG_DEBUG, _T("%s: HANDSHAKE: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strLine );
	
	if (	_tcsncmp( strLine, _T("GET"), 3 ) == 0 ||
			_tcsncmp( strLine, _T("HEAD"), 4 ) == 0 )
	{
		Uploads.OnAccept( this, strLine );
	}
	else if ( _tcsnicmp( strLine, _T("GNUTELLA"), 8 ) == 0 )
	{
		Neighbours.OnAccept( this );
	}
	else if ( _tcsnicmp( strLine, _T("PUSH "), 5 ) == 0 )
	{
		OnAcceptPush();
	}
	else if ( _tcsnicmp( strLine, _T("GIV "), 4 ) == 0 )
	{
		OnAcceptGive();
	}
	else if ( _tcsnicmp( strLine, _T("CHAT"), 4 ) == 0 )
	{
		if ( MyProfile.IsValid() && Settings.Community.ChatEnable )
		{
			ChatCore.OnAccept( this );
		}
		else
		{
			m_pOutput->Print( "CHAT/0.2 404 Unavailable\r\n\r\n" );
			OnWrite();
		}
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_HANDSHAKE_FAIL, (LPCTSTR)m_sAddress );
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHandshake accept Gnutella2-style PUSH

BOOL CHandshake::OnAcceptPush()
{
	CString strLine, strGUID;
	CGUID pGUID;
	
	if ( ! m_pInput->ReadLine( strLine ) ) return FALSE;
	
	if ( strLine.GetLength() != 10 + 32 )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BAD_PUSH,
			(LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ) );
		return FALSE;
	}
	
	for ( int nByte = 0 ; nByte < 16 ; nByte++ )
	{
		int nValue;
		_stscanf( strLine.Mid( 10 + nByte * 2, 2 ), _T("%X"), &nValue );
		pGUID.m_b[ nByte ] = (BYTE)nValue;
	}
	
	if ( OnPush( &pGUID ) ) return TRUE;
	
	theApp.Message( MSG_ERROR, IDS_DOWNLOAD_UNKNOWN_PUSH,
		(LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ), _T("Gnutella2") );
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHandshake accept Gnutella1-style GIV

BOOL CHandshake::OnAcceptGive()
{
	CString strLine, strClient, strFile;
	DWORD nFileIndex = 0xFFFFFFFF;
	CGUID pClientID;
	int nPos;
	
	if ( ! m_pInput->ReadLine( strLine ) ) return FALSE;
	
	if ( ( nPos = strLine.Find( '/' ) ) > 0 )
	{
		strFile	= URLDecode( strLine.Mid( nPos + 1 ) );
		strLine	= strLine.Left( nPos );
	}
	
	if ( ( nPos = strLine.Find( ':' ) ) > 4 )
	{
		strClient	= strLine.Mid( nPos + 1 );
		strLine		= strLine.Mid( 4, nPos - 4 );
		_stscanf( strLine, _T("%lu"), &nFileIndex );
	}
	
	if ( nFileIndex == 0xFFFFFFFF || strClient.GetLength() != 32 )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BAD_PUSH,
			(LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ) );
		return FALSE;
	}
	
	for ( int nByte = 0 ; nByte < 16 ; nByte++ )
	{
		_stscanf( strClient.Mid( nByte * 2, 2 ), _T("%X"), &nPos );
		pClientID.m_b[ nByte ] = (BYTE)nPos;
	}
	
	if ( OnPush( &pClientID ) ) return TRUE;
	
	if ( strFile.GetLength() > 256 ) strFile = _T("Invalid Filename");
	
	theApp.Message( MSG_ERROR, IDS_DOWNLOAD_UNKNOWN_PUSH,
		(LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ), (LPCTSTR)strFile );
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHandshake accept a push from a GUID

BOOL CHandshake::OnPush(CGUID* pGUID)
{
	if ( m_hSocket == INVALID_SOCKET ) return FALSE;
	
	if ( Downloads.OnPush( pGUID, this ) ) return TRUE;
	if ( ChatCore.OnPush( pGUID, this ) ) return TRUE;
	
	CSingleLock pWindowLock( &theApp.m_pSection );
	
	if ( pWindowLock.Lock( 250 ) )
	{
		if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
		{
			CWindowManager* pWindows	= &pMainWnd->m_pWindows;
			CChildWnd* pChildWnd		= NULL;

			while ( pChildWnd = pWindows->Find( NULL, pChildWnd ) )
			{
				if ( pChildWnd->OnPush( pGUID, this ) )
					return TRUE;
			}
		}
		
		pWindowLock.Unlock();
	}
	
	return FALSE;
}

