//
// DDEServer.cpp
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
#include "DDEServer.h"

#include "ShareazaURL.h"
#include "BTInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDDEServer DDEServer( CLIENT_NAME_T );


//////////////////////////////////////////////////////////////////////
// CDDEServer construction

CDDEServer* CDDEServer::m_pServer = NULL;


CDDEServer::CDDEServer(LPCTSTR pszService)
{
	m_pServer		= this;
	m_hInstance		= NULL;
	m_hszService	= NULL;
	m_sService		= pszService;
}

CDDEServer::~CDDEServer()
{
	Close();
	m_pServer = NULL;
}

//////////////////////////////////////////////////////////////////////
// CDDEServer create

BOOL CDDEServer::Create()
{
	// Used transactions XTYP_CONNECT and XTYP_EXECUTE only, all others filtered
	UINT uiResult = DdeInitialize( &m_hInstance, DDECallback, APPCLASS_STANDARD |
		CBF_FAIL_ADVISES | CBF_FAIL_POKES | CBF_FAIL_REQUESTS | CBF_FAIL_SELFCONNECTIONS |
		CBF_SKIP_CONNECT_CONFIRMS | CBF_SKIP_DISCONNECTS |
		CBF_SKIP_REGISTRATIONS | CBF_SKIP_UNREGISTRATIONS, 0 );
	if ( uiResult != DMLERR_NO_ERROR ) return FALSE;

	m_hszService = DdeCreateStringHandle( m_hInstance, (LPCTSTR)m_sService, CP_WINUNICODE );

	DdeNameService( m_hInstance, m_hszService, NULL, DNS_REGISTER );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDDEServer close

void CDDEServer::Close()
{
	if ( m_hInstance == NULL ) return;

	DdeNameService( m_hInstance, m_hszService, NULL, DNS_UNREGISTER );

	DdeFreeStringHandle( m_hInstance, m_hszService );

	DdeUninitialize( m_hInstance );
	m_hInstance = NULL;
}

//////////////////////////////////////////////////////////////////////
// CDDEServer static callback

HDDEDATA CALLBACK CDDEServer::DDECallback(UINT wType, UINT /*wFmt*/, HCONV /*hConv*/, HSZ hsz1, HSZ /*hsz2*/, HDDEDATA hData, ULONG_PTR /*dwData1*/, ULONG_PTR /*dwData2*/)
{
	HDDEDATA hResult = NULL;
	if ( m_pServer )
	{
		switch ( wType )
		{
		case XTYP_CONNECT:
			hResult = m_pServer->CheckAccept( m_pServer->StringFromHsz( hsz1 ) ) ?
				(HDDEDATA)TRUE : (HDDEDATA)FALSE;
			break;

		case XTYP_EXECUTE:
			hResult = m_pServer->Execute( m_pServer->StringFromHsz( hsz1 ), hData ) ?
				(HDDEDATA)DDE_FACK : (HDDEDATA)DDE_FNOTPROCESSED;
			break;
		}
	}
	return hResult;
}

//////////////////////////////////////////////////////////////////////
// CDDEServer HSZ to string helper

CString CDDEServer::StringFromHsz(HSZ hsz)
{
	CString str;

	DWORD nLen = DdeQueryString( m_hInstance, hsz, NULL, 0, CP_WINUNICODE );
	if ( nLen == 0 ) return str;

	LPTSTR pBuf = new TCHAR[ nLen + 1 ];
	DdeQueryString( m_hInstance, hsz, pBuf, nLen + 1, CP_WINUNICODE );
	pBuf[nLen] = 0;

	str = pBuf;
	delete [] pBuf;

	return str;
}

//////////////////////////////////////////////////////////////////////
// CDDEServer argument helper

CString CDDEServer::ReadArgument(LPCTSTR& pszMessage)
{
	BOOL bEscape = FALSE;
	CString strPath;

	for ( pszMessage += 7 ; *pszMessage ; pszMessage++ )
	{
		if ( bEscape )
		{
			strPath += *pszMessage;
			bEscape = FALSE;
		}
		else if ( *pszMessage == '\"' )
		{
			if ( pszMessage[1] == '\"' )
			{
				strPath += '\"';
				pszMessage++;
			}
			else
			{
				break;
			}
		}
		else if ( *pszMessage == '\\' )
		{
			bEscape = TRUE;
		}
		else
		{
			strPath += *pszMessage;
		}
	}

	return strPath;
}

//////////////////////////////////////////////////////////////////////
// CDDEServer check accept

BOOL CDDEServer::CheckAccept(LPCTSTR pszTopic)
{
	BOOL bResult = _tcsicmp( pszTopic, _T("URL") ) == 0 ||
			_tcsicmp( pszTopic, _T("RAZAFORMAT") ) == 0;
	if ( !bResult )
		theApp.Message( MSG_ERROR, _T("Received an unsupported topic in the DDE message: %s"), pszTopic );

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CDDEServer execute HDDEDATA mode

BOOL CDDEServer::Execute(LPCTSTR pszTopic, HDDEDATA hData)
{
	DWORD nLength = 0;
	BOOL bResult = FALSE;

	LPVOID pData = DdeAccessData( hData, &nLength );
	ASSERT( pData );
	ASSERT( nLength );

	if ( pData )
	{
		bResult = Execute( pszTopic, pData, nLength );
		DdeUnaccessData( hData );
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CDDEServer execute LPCVOID mode

BOOL CDDEServer::Execute(LPCTSTR pszTopic, LPCVOID pData, DWORD nLength)
{
	ASSERT( pData );
	ASSERT( nLength );

	// Copy data info a buffer
	LPWSTR pszData = new WCHAR[ nLength + 1 ];
	CopyMemory( pszData, pData, nLength );

	// Ensure it has a null terminator
	pszData[ nLength ] = 0;

	// Assign it to a CString and remove buffer
	CString str( pszData );
	delete [] pszData;

	return Execute( pszTopic, str );
}

//////////////////////////////////////////////////////////////////////
// CDDEServer execute string mode

BOOL CDDEServer::Execute(LPCTSTR pszTopic, LPCTSTR pszMessage)
{
	ASSERT( pszMessage );

	if ( _tcscmp( pszTopic, _T("URL") ) == 0 )
	{
		return theApp.OpenURL( pszMessage, TRUE );
	}
	else if ( _tcscmp( pszTopic, _T("RAZAFORMAT") ) == 0 )
	{
		return theApp.Open( pszMessage, TRUE, TRUE );
	}

	return FALSE;
}
