//
// SourceURL.cpp
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
#include "SourceURL.h"
#include "SHA.h"
#include "ED2K.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CSourceURL construction

CSourceURL::CSourceURL(LPCTSTR pszURL)
{
	Clear();
	if ( pszURL != NULL ) Parse( pszURL );
}

//////////////////////////////////////////////////////////////////////
// CSourceURL clear

void CSourceURL::Clear()
{
	m_sURL.Empty();
	m_nProtocol = PROTOCOL_NULL;
	m_sAddress.Empty();
	m_pAddress.S_un.S_addr = 0;
	m_nPort = 80;
	m_pServerAddress.S_un.S_addr = 0;
	m_nServerPort = 0;
	m_sPath.Empty();
	m_oSHA1.Clear();
	m_oED2K.Clear();
	m_oBTH.Clear();
	m_oBTC.Clear();
	m_bSize	= FALSE;
}

//////////////////////////////////////////////////////////////////////
// CSourceURL root parse

BOOL CSourceURL::Parse(LPCTSTR pszURL, BOOL bResolve)
{
	Clear();
	if ( ParseHTTP( pszURL, bResolve ) ) return TRUE;
	Clear();
	if ( ParseFTP( pszURL, bResolve ) ) return TRUE;
	Clear();
	if ( ParseED2KFTP( pszURL, bResolve ) ) return TRUE;
	Clear();
	if ( ParseBTC( pszURL, bResolve ) ) return TRUE;
	Clear();
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CSourceURL HTTP

BOOL CSourceURL::ParseHTTP(LPCTSTR pszURL, BOOL bResolve)
{
	if ( _tcsncmp( pszURL, _T("http://"), 7 ) != 0 ) return FALSE;
	
	CString strURL = pszURL + 7;
	
	int nSlash = strURL.Find( '/' );
	
	if ( nSlash >= 0 )
	{
		m_sAddress	= strURL.Left( nSlash );
		m_sPath		= strURL.Mid( nSlash );
	}
	else
	{
		m_sAddress = strURL;
		m_sPath = _T("/");
	}
	
	int nAt = m_sAddress.Find( '@' );
	if ( nAt >= 0 ) m_sAddress = m_sAddress.Mid( nAt + 1 );
	
	if ( m_sAddress.IsEmpty() ) return FALSE;
	
	if ( _tcsnicmp( m_sPath, _T("/uri-res/N2R?urn:sha1:"), 22 ) == 0 )
	{
		m_oSHA1.FromURN( m_sPath.Mid( 13 ) );
	}
	else if ( _tcsnicmp( m_sPath, _T("/uri-res/N2R?urn:bitprint:"), 26 ) == 0 )
	{
		m_oSHA1.FromURN( m_sPath.Mid( 13 ) );
	}
	
	SOCKADDR_IN saHost;
	
	BOOL bResult = Network.Resolve( m_sAddress, 80, &saHost, bResolve );
	
	m_pAddress	= saHost.sin_addr;
	m_nPort		= htons( saHost.sin_port );
	
	m_sURL		= pszURL;
	m_nProtocol	= PROTOCOL_HTTP;
	
	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CSourceURL FTP

BOOL CSourceURL::ParseFTP(LPCTSTR pszURL, BOOL bResolve)
{
	if ( _tcsncmp( pszURL, _T("ftp://"), 6 ) != 0 ) return FALSE;
	
	CString strURL = pszURL + 6;
	
	int nSlash = strURL.Find( '/' );
	
	if ( nSlash >= 0 )
	{
		m_sAddress	= strURL.Left( nSlash );
		m_sPath		= strURL.Mid( nSlash );
	}
	else
	{
		m_sAddress = strURL;
		m_sPath = _T("/");
	}
	
	int nAt = m_sAddress.Find( '@' );
	if ( nAt >= 0 ) m_sAddress = m_sAddress.Mid( nAt + 1 );
	
	if ( m_sAddress.IsEmpty() ) return FALSE;
	
	SOCKADDR_IN saHost;
	
	BOOL bResult = Network.Resolve( m_sAddress, 21, &saHost, bResolve );
	
	m_pAddress	= saHost.sin_addr;
	m_nPort		= htons( saHost.sin_port );
	
	m_sURL		= pszURL;
	m_nProtocol	= PROTOCOL_FTP;
	
	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CSourceURL ED2KFTP

BOOL CSourceURL::ParseED2KFTP(LPCTSTR pszURL, BOOL bResolve)
{
	if ( _tcsnicmp( pszURL, _T("ed2kftp://"), 10 ) != 0 ) return FALSE;
	
	CString strURL = pszURL + 10;
	BOOL bPush = FALSE;
	
	int nSlash = strURL.Find( '/' );
	if ( nSlash < 7 ) return FALSE;

	m_sAddress	= strURL.Left( nSlash );
	strURL		= strURL.Mid( nSlash + 1 );
	
	nSlash = strURL.Find( '/' );
	if ( nSlash != 32 ) return FALSE;
	
	CString strHash	= strURL.Left( 32 );
	strURL			= strURL.Mid( 33 );
	
	if ( ! m_oED2K.FromString( strHash ) ) return FALSE;
	
	m_bSize = _stscanf( strURL, _T("%I64i"), &m_nSize ) == 1;
	if ( ! m_bSize ) return FALSE;
	
	nSlash = m_sAddress.Find( '@' );
	
	if ( nSlash > 0 )
	{
		strHash = m_sAddress.Left( nSlash );
		m_sAddress = m_sAddress.Mid( nSlash + 1 );
		if ( _stscanf( strHash, _T("%lu"), &m_pAddress.S_un.S_addr ) != 1 ) return FALSE;
		bPush = TRUE;
	}
	
	SOCKADDR_IN saHost;
	BOOL bResult = Network.Resolve( m_sAddress, ED2K_DEFAULT_PORT, &saHost, bResolve );
	
	if ( bPush )
	{
		m_pServerAddress	= saHost.sin_addr;
		m_nServerPort		= htons( saHost.sin_port );
		m_nPort				= 0;
	}
	else
	{
		m_pAddress	= saHost.sin_addr;
		m_nPort		= htons( saHost.sin_port );
	}
	
	m_sURL		= pszURL;
	m_nProtocol	= PROTOCOL_ED2K;
	
	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CSourceURL BTC

BOOL CSourceURL::ParseBTC(LPCTSTR pszURL, BOOL bResolve)
{
	if ( _tcsnicmp( pszURL, _T("btc://"), 6 ) != 0 ) return FALSE;
	
	CString strURL = pszURL + 6;
	BOOL bPush = FALSE;
	
	int nSlash = strURL.Find( '/' );
	if ( nSlash < 7 ) return FALSE;

	m_sAddress	= strURL.Left( nSlash );
	strURL		= strURL.Mid( nSlash + 1 );
	
	nSlash = strURL.Find( '/' );
	m_oBTC.Clear();
	
	if ( nSlash == 32 )
	{
		CString strGUID	= strURL.Left( 32 );
		m_oBTC.FromString( strGUID );
	}
	else if ( nSlash < 0 ) return FALSE;
	
	strURL = strURL.Mid( nSlash + 1 );
	
	if ( ! m_oBTH.FromString( strURL ) ) return FALSE;
	
	SOCKADDR_IN saHost;
	BOOL bResult = Network.Resolve( m_sAddress, ED2K_DEFAULT_PORT, &saHost, bResolve );
	
	m_pAddress	= saHost.sin_addr;
	m_nPort		= htons( saHost.sin_port );
	
	m_sURL		= pszURL;
	m_nProtocol	= PROTOCOL_BT;
	
	return bResult;
}
