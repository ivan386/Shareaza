//
// ShareazaURL.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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
#include "ShareazaURL.h"
#include "Transfer.h"
#include "QuerySearch.h"
#include "DiscoveryServices.h"
#include "Network.h"
#include "BTInfo.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CShareazaURL construction

CShareazaURL::CShareazaURL(LPCTSTR pszURL)
	: m_nProtocol		( PROTOCOL_NULL )
	, m_nAction			( uriNull )
	, m_pTorrent		( NULL )
	, m_pAddress		(  )
	, m_nPort			( 0 )
	, m_pServerAddress	(  )
	, m_nServerPort		( 0 )
	, m_bSize			( FALSE )
{
	if ( pszURL != NULL ) Parse( pszURL );
}

CShareazaURL::CShareazaURL(CBTInfo* pTorrent)
	: CShareazaFile( static_cast< const CShareazaFile& >( *pTorrent ) )
	, m_nProtocol		( PROTOCOL_NULL )
	, m_nAction			( uriDownload )
	, m_pTorrent		( pTorrent )
	, m_pAddress		(  )
	, m_nPort			( 0 )
	, m_pServerAddress	(  )
	, m_nServerPort		( 0 )
	, m_bSize			( TRUE )
{
}

CShareazaURL::CShareazaURL(const CShareazaURL& pURL)
	: CShareazaFile( static_cast< const CShareazaFile& >( pURL ) )
	, m_nProtocol		( pURL.m_nProtocol )
	, m_nAction			( pURL.m_nAction )
	, m_pTorrent		( pURL.m_pTorrent )
	, m_sAddress		( pURL.m_sAddress )
	, m_pAddress		( pURL.m_pAddress )
	, m_nPort			( pURL.m_nPort )
	, m_pServerAddress	( pURL.m_pServerAddress )
	, m_nServerPort		( pURL.m_nServerPort )
	, m_bSize			( pURL.m_bSize )
	, m_sLogin			( pURL.m_sLogin )
	, m_sPassword		( pURL.m_sPassword )
	, m_oBTC			( pURL.m_oBTC )
{
}

CShareazaURL::~CShareazaURL()
{
	delete m_pTorrent;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL clear

void CShareazaURL::Clear()
{
	// CShareazaFile
	m_sName.Empty();
	m_nSize					= SIZE_UNKNOWN;
	m_oSHA1.clear();
	m_oTiger.clear();
	m_oMD5.clear();
	m_oED2K.clear();
	m_oBTH.clear();
	m_sURL.Empty();
	m_sPath.Empty();

	// CShareazaURL
	m_nProtocol				= PROTOCOL_NULL;
	m_nAction				= uriNull;
	delete m_pTorrent;
	m_pTorrent				= NULL;
	m_sAddress.Empty();
	m_pAddress.s_addr		= 0;
	m_nPort					= 0;
	m_pServerAddress.s_addr = 0;
	m_nServerPort			= 0;
	m_bSize					= FALSE;
	m_sLogin.Empty ();
	m_sPassword.Empty ();
	m_oBTC.clear();
}

//////////////////////////////////////////////////////////////////////
// Parse URL list

BOOL CShareazaURL::Parse(const CString& sText, CList< CString >& pURLs, BOOL bResolve)
{
	pURLs.RemoveAll();

	// Split text to reverse string list
	CString sPart;
	int curPos = 0;
	CList< CString > oReverse;
	while ( ( sPart = sText.Tokenize( _T("\n"), curPos ) ).GetLength() )
	{
		oReverse.AddHead( sPart.Trim( _T("\r\n\t >< ") ) ); // second space is #160
	}

	CString sBuf;
	for ( POSITION pos = oReverse.GetHeadPosition(); pos; )
	{
		CString sLine( oReverse.GetNext( pos ) );
		if ( sLine.IsEmpty() )
			// Empty strings breaks URL
			sBuf.Empty();
		else
		{
			// Append new line to current URL and parse
			sBuf.Insert( 0, sLine );
			if ( Parse( sBuf, bResolve ) )
			{
				// OK, new URL found
				pURLs.AddTail( sBuf );
				sBuf.Empty();
			}
		}
	}

	return ! pURLs.IsEmpty();
}

//////////////////////////////////////////////////////////////////////
// Parse single URL

BOOL CShareazaURL::Parse(LPCTSTR pszURL, BOOL bResolve)
{
	// Parse "good" URL
	if ( ParseRoot( pszURL, bResolve ) )
		return TRUE;
	else
		// Parse "bad" URL
		return ParseRoot( URLDecode( pszURL ), bResolve );
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL root parser

BOOL CShareazaURL::ParseRoot(LPCTSTR pszURL, BOOL bResolve)
{
	if ( _tcsncmp( pszURL, _T("http://"), 7 ) == 0 )
	{
		return ParseHTTP( pszURL, bResolve );
	}
	else if ( _tcsncmp( pszURL, _T("ftp://"), 6 ) == 0 )
	{
		return ParseFTP( pszURL, bResolve );
	}
	else if ( _tcsnicmp( pszURL, _T("ed2kftp://"), 10 ) == 0 )
	{
		return ParseED2KFTP( pszURL, bResolve );
	}
	else if ( _tcsnicmp( pszURL, _T("btc://"), 6 ) == 0 )
	{
		return ParseBTC( pszURL, bResolve );
	}
	else if ( _tcsnicmp( pszURL, _T("magnet:?"), 8 ) == 0 )
	{
		return ParseMagnet( pszURL + 8 );
	}
	else if ( _tcsnicmp( pszURL, _T("foxy:"), 5 ) == 0 )	// Foxy
	{
		pszURL += 5;
		if ( ! _tcsnicmp( pszURL, _T("//download?"), 11 ) )			// Original
		{
			pszURL += 11;
			return ParseMagnet( pszURL );
		}
		else if ( ! _tcsnicmp( pszURL, _T("//download/?"), 12 ) )	// "Fixed" by IE
		{
			pszURL += 12;
			return ParseMagnet( pszURL );
		}
	}
	else if (	_tcsnicmp( pszURL, _T("shareaza:"), 9 ) == 0 ||
				_tcsnicmp( pszURL, _T("gnutella:"), 9 ) == 0 )
	{
		SkipSlashes( pszURL, 9 );
		return ParseShareaza( pszURL );
	}
	else if ( _tcsnicmp( pszURL, _T("gwc:"), 4 ) == 0 )
	{
		CString strTemp;
		strTemp.Format( _T("shareaza:%s"), pszURL );
		pszURL = strTemp;
		SkipSlashes( pszURL, 9 );
		return ParseShareaza( pszURL );
	}
	else if ( _tcsnicmp( pszURL, _T("gnet:"), 5 ) == 0 )
	{
		SkipSlashes( pszURL, 5 );
		return ParseShareaza( pszURL );
	}
	else if ( _tcsnicmp( pszURL, _T("ed2k:"), 5 ) == 0 )
	{
		SkipSlashes( pszURL, 5 );
		return ParseDonkey( pszURL );
	}
	else if ( _tcsnicmp( pszURL, _T("mp2p:"), 5 ) == 0 )
	{
		SkipSlashes( pszURL, 5 );
		return ParsePiolet( pszURL );
	}
	else if (	_tcsnicmp( pszURL, _T("uhc:"), 4 ) == 0 ||
				_tcsnicmp( pszURL, _T("ukhl:"), 5 ) == 0 ||
				_tcsnicmp( pszURL, _T("gnutella1:"), 10 ) == 0 ||
				_tcsnicmp( pszURL, _T("gnutella2:"), 10 ) == 0 )
	{
		return ParseShareaza( pszURL );
	}
	else if ( _tcsnicmp( pszURL, _T("dchub://"), 8 ) == 0 )
	{
		return ParseDCHub( pszURL, bResolve );
	}

	Clear();

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL HTTP

BOOL CShareazaURL::ParseHTTP(LPCTSTR pszURL, BOOL bResolve)
{
	Clear();

	CString strURL = pszURL + 7;

	int nSlash = strURL.Find( _T('/') );

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

	int nAt = m_sAddress.Find( _T('@') );
	if ( nAt >= 0 ) m_sAddress = m_sAddress.Mid( nAt + 1 );

	if ( m_sAddress.IsEmpty() ) return FALSE;

	if ( _tcsnicmp( m_sPath, _T("/uri-res/N2R?"), 13 ) == 0 )
	{
		strURL = m_sPath.Mid( 13 );
		if ( m_oSHA1.fromUrn( strURL ) );
		else if ( m_oTiger.fromUrn( strURL ) );
		else if ( m_oED2K.fromUrn( strURL ) );
		else if ( m_oBTH.fromUrn( strURL ) );
		else if ( m_oBTH.fromUrn< Hashes::base16Encoding >( strURL ) );
		else if ( m_oMD5.fromUrn( strURL ) );
		else
			return FALSE;

		m_nAction	= uriSource;
	}
	else
	{
		m_nAction	= uriDownload;

		int nPos = m_sPath.ReverseFind( '/' );
		if ( nPos >= 0 )
		{
			CString sName( URLDecode(
				m_sPath.Mid( nPos + 1 ).SpanExcluding( _T("?") ) ) );
			if ( sName.GetLength() )
			{
				m_sName = sName;
			}
		}
	}

	SOCKADDR_IN saHost;

	BOOL bResult = Network.Resolve( m_sAddress, INTERNET_DEFAULT_HTTP_PORT, &saHost, bResolve );

	m_pAddress	= saHost.sin_addr;
	m_nPort		= htons( saHost.sin_port );

	m_sURL		= pszURL;
	m_nProtocol	= PROTOCOL_HTTP;

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL FTP

BOOL CShareazaURL::ParseFTP(LPCTSTR pszURL, BOOL bResolve)
{
	// URI format
	// ftp://[user[:password]@]host[:port][/path]

	Clear();

	CString strURL ( pszURL + 6 );

	int nSlash = strURL.Find( _T('/') );

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

	int nAt = m_sAddress.Find( _T('@') );
	if ( nAt >= 0 )
	{
		m_sLogin = m_sAddress.Left( nAt );
		m_sAddress = m_sAddress.Mid( nAt + 1 );

		int nColon = m_sLogin.Find( _T(':') );
		if ( nColon >= 0 )
		{
			m_sPassword = m_sLogin.Mid( nColon + 1 );
			m_sLogin = m_sLogin.Left( nColon );
		}
	}
	else
	{
		m_sLogin = _T("anonymous");
		m_sPassword = _T("guest@shareaza.com");
	}

	if ( m_sAddress.IsEmpty() || m_sLogin.IsEmpty() )
		return FALSE;

	//add fix set name
	int nPos = m_sPath.ReverseFind( '/' );
	if ( !m_sName.GetLength() && nPos >= 0 )
	{
		CString sName( URLDecode(
			m_sPath.Mid( nPos + 1 ).SpanExcluding( _T("?") ) ) );
		if ( sName.GetLength() )
		{
			m_sName = sName;
		}
	}

	SOCKADDR_IN saHost;

	BOOL bResult = Network.Resolve( m_sAddress, INTERNET_DEFAULT_FTP_PORT, &saHost, bResolve );

	m_pAddress	= saHost.sin_addr;
	m_nPort		= htons( saHost.sin_port );

	m_sURL		= pszURL;
	m_nProtocol	= PROTOCOL_FTP;
	m_nAction	= uriDownload;

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL ED2KFTP

BOOL CShareazaURL::ParseED2KFTP(LPCTSTR pszURL, BOOL bResolve)
{
	Clear();

	CString strURL = pszURL + 10;
	BOOL bPush = FALSE;

	int nSlash = strURL.Find( _T('/') );
	if ( nSlash < 7 ) return FALSE;

	m_sAddress	= strURL.Left( nSlash );
	strURL		= strURL.Mid( nSlash + 1 );

	nSlash = strURL.Find( _T('/') );
	if ( nSlash != 32 ) return FALSE;

	CString strHash	= strURL.Left( 32 );
	strURL			= strURL.Mid( 33 );

	if ( !m_oED2K.fromString( strHash ) ) return FALSE;

	m_bSize = _stscanf( strURL, _T("%I64i"), &m_nSize ) == 1;
	if ( ! m_bSize ) return FALSE;

	nSlash = m_sAddress.Find( _T('@') );

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
	m_nAction	= uriDownload;

	return bResult;
}


//////////////////////////////////////////////////////////////////////
// CShareazaURL DC

BOOL CShareazaURL::ParseDCHub(LPCTSTR pszURL, BOOL bResolve)
{
	Clear();

	CString strURL = pszURL + 8;		// "dchub://"

	int nSlash = strURL.Find( _T('/') );
	if ( nSlash != -1 )
	{
		m_sAddress	= strURL.Left( nSlash );
		strURL		= strURL.Mid( nSlash + 1 ).TrimLeft( _T("/") );

		int nAt = m_sAddress.Find( _T('@') );
		if ( nAt >= 0 )
		{
			m_sLogin = URLDecode( m_sAddress.Left( nAt ) );
			m_sAddress = m_sAddress.Mid( nAt + 1 );
		}
	}
	if ( nSlash == -1 || strURL.IsEmpty() )
	{
		// Short version - hub address only
		m_sAddress.Empty();
		m_nPort = DC_DEFAULT_PORT;
		if ( ! ParseShareazaHost( pszURL + 8, FALSE ) )
			return FALSE;
		m_nProtocol = PROTOCOL_DC;
		return TRUE;
	}

	// Full version - file URL
	int nHash = strURL.Find( _T("TTH:") );
	if ( nHash != -1 )
	{
		CString strHash = strURL.Mid( nHash + 4, 39 );
		strURL = strURL.Mid( nHash + 4 + 39 );

		if ( ! m_oTiger.fromString( strHash ) )
			return FALSE;

		if ( _stscanf( strURL, _T("/%I64i"), &m_nSize ) == 1 )
		{
			m_bSize = TRUE;
		}
	}
	else
	{
		m_sName = URLDecode( strURL );
	}

	SOCKADDR_IN saHost = {};
	BOOL bResult = Network.Resolve( m_sAddress, DC_DEFAULT_PORT, &saHost, bResolve );

	m_pServerAddress	= saHost.sin_addr;
	m_nServerPort		= htons( saHost.sin_port );
	m_sURL				= pszURL;
	m_nProtocol			= PROTOCOL_DC;
	m_nAction			= uriDownload;

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL BTC

BOOL CShareazaURL::ParseBTC(LPCTSTR pszURL, BOOL bResolve)
{
	Clear();

	CString strURL = pszURL + 6;

	int nSlash = strURL.Find( _T('/') );
	if ( nSlash < 7 ) return FALSE;

	m_sAddress	= strURL.Left( nSlash );
	strURL		= strURL.Mid( nSlash + 1 );

	nSlash = strURL.Find( _T('/') );
	m_oBTC.clear();

	if ( nSlash == 32 )
	{
		CString strGUID	= strURL.Left( 32 );
		m_oBTC.fromString( strGUID );
	}
	else if ( nSlash < 0 ) return FALSE;

	strURL = strURL.Mid( nSlash + 1 );

	if ( !m_oBTH.fromString( strURL ) ) return FALSE;

	SOCKADDR_IN saHost;
	BOOL bResult = Network.Resolve( m_sAddress, ED2K_DEFAULT_PORT, &saHost, bResolve );

	m_pAddress	= saHost.sin_addr;
	m_nPort		= htons( saHost.sin_port );

	m_sURL		= pszURL;
	m_nProtocol	= PROTOCOL_BT;
	m_nAction	= uriDownload;

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse "magnet:" URLs

BOOL CShareazaURL::ParseMagnet(LPCTSTR pszURL)
{
	Clear();

	CString strURL( pszURL );
	CBTInfo* pTorrent = new CBTInfo();

	for ( strURL += '&' ; strURL.GetLength() ; )
	{
		CString strPart = strURL.SpanExcluding( _T("&") );
		strURL = strURL.Mid( strPart.GetLength() + 1 );

		int nEquals = strPart.Find( '=' );
		if ( nEquals < 0 ) continue;

		CString strKey		= URLDecode( strPart.Left( nEquals ) );
		CString strValue	= URLDecode( strPart.Mid( nEquals + 1 ) );

		SafeString( strKey );
		SafeString( strValue );

		if ( strKey.IsEmpty() || strValue.IsEmpty() ) continue;

		if ( _tcsicmp( strKey, _T("xt") ) == 0 ||
			 _tcsicmp( strKey, _T("xs") ) == 0 ||
			 _tcsicmp( strKey, _T("as") ) == 0 ||
			 _tcsicmp( strKey, _T("tr") ) == 0 )
		{
			if (	_tcsnicmp( strValue, _T("urn:"), 4 ) == 0 ||
					_tcsnicmp( strValue, _T("sha1:"), 5 ) == 0 ||
					_tcsnicmp( strValue, _T("bitprint:"), 9 ) == 0 ||
					_tcsnicmp( strValue, _T("tree:tiger:"), 11 ) == 0 ||
					_tcsnicmp( strValue, _T("tree:tiger/:"), 12 ) == 0 ||
					_tcsnicmp( strValue, _T("tree:tiger/1024:"), 16 ) == 0 ||
					_tcsnicmp( strValue, _T("md5:"), 4 ) == 0 ||
					_tcsnicmp( strValue, _T("btih:"), 5 ) == 0 ||
					_tcsnicmp( strValue, _T("ed2k:"), 5 ) == 0 )
			{
				if ( !m_oSHA1 ) m_oSHA1.fromUrn( strValue );
				if ( !m_oTiger ) m_oTiger.fromUrn( strValue );
				if ( !m_oMD5 ) m_oMD5.fromUrn( strValue );
				if ( !m_oED2K ) m_oED2K.fromUrn( strValue );
				if ( !m_oBTH ) m_oBTH.fromUrn( strValue );
				if ( !m_oBTH ) m_oBTH.fromUrn< Hashes::base16Encoding >( strValue );
			}
			else if (	_tcsnicmp( strValue, _T("http://"), 7 ) == 0 ||
						_tcsnicmp( strValue, _T("http%3A//"), 9 ) == 0 ||
						_tcsnicmp( strValue, _T("ftp://"), 6 ) == 0 ||
						_tcsnicmp( strValue, _T("ftp%3A//"), 8 ) == 0 ||
						_tcsnicmp( strValue, _T("dchub://"), 8 ) == 0 ||
						_tcsnicmp( strValue, _T("dchub%3A//"), 8 ) == 0 )
			{
				strValue.Replace( _T(" "), _T("%20") );
				strValue.Replace( _T("%3A//"), _T("://") );

				if ( _tcsicmp( strKey, _T("xt") ) == 0 )
				{
					CString strURL = _T("@") + strValue;

					if ( m_sURL.GetLength() )
						m_sURL = strURL + _T(", ") + m_sURL;
					else
						m_sURL = strURL;
				}
				else if( _tcsicmp( strKey, _T("tr") ) == 0 )
				{
					pTorrent->SetTracker( strValue );
				}
				else
				{
					if ( m_sURL.GetLength() ) m_sURL += _T(", ");
					m_sURL += strValue;
				}
			}
		}
		else if ( _tcsicmp( strKey, _T("dn") ) == 0 )
		{
			m_sName = strValue;
		}
		else if ( _tcsicmp( strKey, _T("kt") ) == 0 )
		{
			m_sName = strValue;
			m_oSHA1.clear();
			m_oTiger.clear();
			m_oED2K.clear();
			m_oMD5.clear();
			m_oBTH.clear();
		}
		else if ( _tcsicmp( strKey, _T("xl") ) == 0 ||
			_tcsicmp( strKey, _T("sz") ) == 0 ||	// Non-standard
			_tcsicmp( strKey, _T("fs") ) == 0 )		// Foxy
		{
			QWORD nSize;
			if ( ( ! m_bSize ) && ( _stscanf( strValue, _T("%I64i"), &nSize ) == 1 ) && ( nSize > 0 ) )
			{
				m_nSize = nSize;
				m_bSize = TRUE;
			}
		}
	}

	if ( m_oBTH && ! m_pTorrent )
	{
		pTorrent->SetTrackerMode( pTorrent->GetTrackerCount() > 1 ? CBTInfo::tMultiFinding : CBTInfo::tSingle );
		
		m_pTorrent = pTorrent;
		pTorrent = NULL;
		m_pTorrent->m_oMD5			= m_oMD5;
		m_pTorrent->m_oBTH			= m_oBTH;
		m_pTorrent->m_oSHA1			= m_oSHA1;
		m_pTorrent->m_oED2K			= m_oED2K;
		m_pTorrent->m_oTiger		= m_oTiger;
		m_pTorrent->m_sName			= m_sName;
		m_pTorrent->m_nSize			= m_nSize;
	}
	
	delete pTorrent;

	if ( HasHash() || m_sURL.GetLength() )
	{
		m_nAction = uriDownload;
		return TRUE;
	}
	else if ( m_sName.GetLength() )
	{
		m_nAction = uriSearch;
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse "shareaza:" URLs

BOOL CShareazaURL::ParseShareaza(LPCTSTR pszURL)
{
	Clear();

	int nIP[4];

	if ( _stscanf( pszURL, _T("%i.%i.%i.%i"), &nIP[0], &nIP[1], &nIP[2], &nIP[3] ) == 4 )
	{
		return ParseShareazaHost( pszURL, FALSE );
	}

	if ( _tcsnicmp( pszURL, _T("host:"), 5 ) == 0 ||
		 _tcsnicmp( pszURL, _T("node:"), 5 ) == 0 )
	{
		return ParseShareazaHost( pszURL + 5, FALSE );
	}
	else if ( _tcsnicmp( pszURL, _T("hub:"), 4 ) == 0 )
	{
		return ParseShareazaHost( pszURL + 4, FALSE );
	}
	else if ( _tcsnicmp( pszURL, _T("server:"), 7 ) == 0 )
	{
		return ParseShareazaHost( pszURL + 7, FALSE );
	}
	else if ( _tcsnicmp( pszURL, _T("browse:"), 7 ) == 0 )
	{
		return ParseShareazaHost( pszURL + 7, TRUE );
	}
	else if ( _tcsnicmp( pszURL, _T("gwc:"), 4 ) == 0 )
	{
		return ParseDiscovery( pszURL + 4, CDiscoveryService::dsWebCache );
	}
	else if ( _tcsnicmp( pszURL, _T("meturl:"), 7 ) == 0 )
	{
		return ParseDiscovery( pszURL + 7, CDiscoveryService::dsServerMet );
	}
	else if (	_tcsnicmp( pszURL, _T("uhc:"), 4 ) == 0 ||
				_tcsnicmp( pszURL, _T("ukhl:"), 5 ) == 0 ||
				_tcsnicmp( pszURL, _T("gnutella1:host:"), 15 ) == 0 ||
				_tcsnicmp( pszURL, _T("gnutella2:host:"), 15 ) == 0 )
	{
		return ParseDiscovery( pszURL, CDiscoveryService::dsGnutella );
	}
	else if ( _tcsnicmp( pszURL, _T("url:"), 4 ) == 0 )
	{
		return Parse( pszURL + 4 );
	}
	else
	{
		return ParseShareazaFile( pszURL );
	}
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse shareaza host URL

BOOL CShareazaURL::ParseShareazaHost(LPCTSTR pszURL, BOOL bBrowse)
{
	m_sName = pszURL;
	m_sName = m_sName.SpanExcluding( _T("/\\") );

	int nPos = m_sName.Find( ':' );

	if ( nPos >= 0 )
	{
		_stscanf( m_sName.Mid( nPos + 1 ), _T("%i"), &m_nPort );
		m_sName = m_sName.Left( nPos );
	}

	int nAt = m_sName.Find( _T('@') );
	if ( nAt >= 0 )
	{
		m_sLogin = URLDecode( m_sName.Left( nAt ) );
		m_sName = m_sName.Mid( nAt + 1 );
	}

	m_sName.TrimLeft();
	m_sName.TrimRight();

	m_nProtocol = PROTOCOL_G2;
	m_nAction = bBrowse ? uriBrowse : uriHost;

	return m_sName.GetLength();
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse shareaza file URL

BOOL CShareazaURL::ParseShareazaFile(LPCTSTR pszURL)
{
	CString strURL( pszURL );

	for ( strURL += '/' ; strURL.GetLength() ; )
	{
		CString strPart = strURL.SpanExcluding( _T("/|") );
		strURL = strURL.Mid( strPart.GetLength() + 1 );

		strPart.TrimLeft();
		strPart.TrimRight();

		if ( strPart.IsEmpty() ) continue;

		if (	_tcsnicmp( strPart, _T("urn:"), 4 ) == 0 ||
				_tcsnicmp( strPart, _T("sha1:"), 5 ) == 0 ||
				_tcsnicmp( strPart, _T("bitprint:"), 9 ) == 0 ||
				_tcsnicmp( strPart, _T("tree:tiger:"), 11 ) == 0 ||
				_tcsnicmp( strPart, _T("tree:tiger/:"), 12 ) == 0 ||
				_tcsnicmp( strPart, _T("tree:tiger/1024:"), 16 ) == 0 ||
				_tcsnicmp( strPart, _T("md5:"), 4 ) == 0 ||
				_tcsnicmp( strPart, _T("btih:"), 5 ) == 0 ||
				_tcsnicmp( strPart, _T("ed2k:"), 5 ) == 0 )
		{
			if ( !m_oSHA1 ) m_oSHA1.fromUrn( strPart );
			if ( !m_oTiger ) m_oTiger.fromUrn( strPart );
			if ( !m_oMD5 ) m_oMD5.fromUrn( strPart );
			if ( !m_oED2K ) m_oED2K.fromUrn( strPart );
			if ( !m_oBTH ) m_oBTH.fromUrn( strPart );
			if ( !m_oBTH ) m_oBTH.fromUrn< Hashes::base16Encoding >( strPart );
		}
		else if ( _tcsnicmp( strPart, _T("source:"), 7 ) == 0 )
		{
			CString strSource = URLDecode( strPart.Mid( 7 ) );
			SafeString( strSource );

			if ( m_sURL.GetLength() ) m_sURL += ',';
			m_sURL += _T("http://");
			m_sURL += URLEncode( strSource );
			m_sURL += _T("/(^name^)");
		}
		else if (	_tcsnicmp( strPart, _T("name:"), 5 ) == 0 ||
					_tcsnicmp( strPart, _T("file:"), 5 ) == 0 )
		{
			m_sName = URLDecode( strPart.Mid( 5 ) );
			SafeString( m_sName );
		}
		else if ( _tcschr( strPart, ':' ) == NULL )
		{
			m_sName = URLDecode( strPart );
			SafeString( m_sName );
		}
	}

	if ( m_sURL.GetLength() )
	{
		if ( m_sName.GetLength() )
		{
			m_sURL.Replace( _T("(^name^)"), URLEncode( m_sName ) );
			m_sURL.Replace( _T("\\"), _T("/") );
		}
		else
		{
			m_sURL.Empty();
		}
	}

	if ( HasHash() || m_sURL.GetLength() )
	{
		m_nAction = uriDownload;
		return TRUE;
	}
	else if ( m_sName.GetLength() )
	{
		m_nAction = uriSearch;
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse "ed2k:" URLs

BOOL CShareazaURL::ParseDonkey(LPCTSTR pszURL)
{
	Clear();

	if ( _tcsnicmp( pszURL, _T("|file|"), 6 ) == 0 )
	{
		// ed2k://|file|Shareaza1600.exe|789544|3fb626ed1a9f4cb9921107f510148370|/
		// ed2k://|file|Shareaza_2.1.0.0.exe|3304944|A63D221505E99043B7E7308C67F81986|h=XY5VGKFVGJFYWMOAR5XS44YCEPXSL2JZ|/|sources,1.2.3.4:5555|/

		return ParseDonkeyFile( pszURL + 6 );
	}
	else if ( _tcsnicmp( pszURL, _T("|server|"), 8 ) == 0 )
	{
		// ed2k://|server|1.2.3.4|4661|/

		return ParseDonkeyServer( pszURL + 8 );
	}
	else if ( _tcsnicmp( pszURL, _T("|meturl|"), 8 ) == 0 )
	{
		return ParseDiscovery( pszURL + 8, CDiscoveryService::dsServerMet );
	}
	else if ( _tcsnicmp( pszURL, _T("|serverlist|"), 12 ) == 0 )
	{
		return ParseDiscovery( pszURL + 12, CDiscoveryService::dsServerMet );
	}
	else if ( _tcsnicmp( pszURL, _T("|search|"), 8 ) == 0 )
	{
		// ed2k://|search|Shareaza|/

		CString sURL( pszURL + 8 );

		int nSep = sURL.Find( _T('|') );
		if ( nSep <= 0 )
			return FALSE;

		m_sName = URLDecode( sURL.Mid( 0, nSep ) ).Trim();

		if ( m_sName.IsEmpty() )
			return FALSE;

		m_nAction = uriSearch;

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse eDonkey2000 file URL

BOOL CShareazaURL::ParseDonkeyFile(LPCTSTR pszURL)
{
	CString strURL( pszURL ), strPart;
	int nSep;

	// Name
	nSep = strURL.Find( '|' );
	if ( nSep < 0 ) return FALSE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );

	m_sName = URLDecode( strPart );
	SafeString( m_sName );
	if ( m_sName.IsEmpty() ) return FALSE;

	// Size
	nSep = strURL.Find( '|' );
	if ( nSep < 0 ) return FALSE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );

	if ( _stscanf( strPart, _T("%I64i"), &m_nSize ) != 1 ) return FALSE;
	m_bSize = TRUE;

	// Hash
	nSep = strURL.Find( '|' );
	if ( nSep < 0 ) return FALSE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );

	m_oED2K.fromString( strPart );

	// URL is valid
	m_nAction = uriDownload;

	// AICH hash (h), HTTP source (s) and/or hash set (p)
	nSep = strURL.Find( '|' );
	if ( nSep < 0 ) return TRUE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );
	while ( strPart != _T("/") )
	{

		if ( _tcsncmp( strPart, _T("h="), 2 ) == 0 )
		{
			// AICH hash
			 // theApp.Message(MSG_INFO, _T("AICH") );
			 strPart = strPart.Mid( 2 );
		}
		else if ( _tcsncmp( strPart, _T("s="), 2 ) == 0 )
		{
			// HTTP source
			// theApp.Message(MSG_INFO, _T("HTTP") );
			strPart = strPart.Mid( 2 );

			if ( m_sURL.GetLength() ) m_sURL += _T(", ");
			SafeString( strPart );
			m_sURL += strPart;
		}
		else if ( _tcsncmp( strPart, _T("p="), 2 ) == 0 )
		{
			// Hash set
			// theApp.Message(MSG_INFO, _T("hash set") );
			strPart = strPart.Mid( 2 );
		}

		// Read in next chunk
		nSep = strURL.Find( '|' );
		if ( nSep < 0 ) return TRUE;
		strPart	= strURL.Left( nSep );
		strURL	= strURL.Mid( nSep + 1 );

	}

	while ( strURL.GetLength() > 8 )
	{
		// Source (Starts with |/|sources,
		nSep = strURL.Find( ',' );
		if ( nSep < 0 ) return TRUE;
		strPart	= strURL.Left( nSep );
		strURL	= strURL.Mid( nSep + 1 );

		if ( _tcsncmp( strPart, _T("sources"), 7 ) != 0 ) return TRUE;

		nSep = strURL.Find( '|' );
		if ( nSep < 0 ) return TRUE;
		strPart	= strURL.Left( nSep );
		strURL	= strURL.Mid( nSep + 1 );

		// Now we have the source in x.x.x.x:port format.
		CString strEDFTP;
		strEDFTP.Format( _T("ed2kftp://%s/%s/%I64i/"), strPart, (LPCTSTR)m_oED2K.toString(), m_nSize );
		SafeString( strEDFTP );
		if ( m_sURL.GetLength() ) m_sURL += _T(", ");
		m_sURL += strEDFTP;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse eDonkey2000 server URL

BOOL CShareazaURL::ParseDonkeyServer(LPCTSTR pszURL)
{
	LPCTSTR pszPort = _tcschr( pszURL, '|' );
	if ( pszPort == NULL ) return FALSE;

	if ( _stscanf( pszPort + 1, _T("%i"), &m_nPort ) != 1 ) return FALSE;

	m_sName = pszURL;
	m_sName = m_sName.Left( static_cast< int >( pszPort - pszURL ) );

	m_sName.TrimLeft();
	m_sName.TrimRight();
	if ( m_sName.IsEmpty() ) return FALSE;

	m_nAction	= uriDonkeyServer;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse "mp2p:" URLs

BOOL CShareazaURL::ParsePiolet(LPCTSTR pszURL)
{
	Clear();

	if ( _tcsnicmp( pszURL, _T("file|"), 5 ) == 0 )
	{
		// mp2p://file|Shareaza1600.exe|789544|3fb626ed1a9f4cb9921107f510148370/

		return ParsePioletFile( pszURL + 5 );
	}
	else if ( _tcsnicmp( pszURL, _T("|file|"), 6 ) == 0 )
	{
		// mp2p://|file|Shareaza1600.exe|789544|3fb626ed1a9f4cb9921107f510148370/

		return ParsePioletFile( pszURL + 6 );
	}
	else
	{
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse Piolet file URL

BOOL CShareazaURL::ParsePioletFile(LPCTSTR pszURL)
{
	CString strURL( pszURL ), strPart;
	int nSep;

	nSep = strURL.Find( '|' );
	if ( nSep < 0 ) return FALSE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );

	m_sName = URLDecode( strPart );
	SafeString( m_sName );
	if ( m_sName.IsEmpty() ) return FALSE;

	nSep = strURL.Find( '|' );
	if ( nSep < 0 ) return FALSE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );

	if ( _stscanf( strPart, _T("%I64i"), &m_nSize ) != 1 ) return FALSE;
	m_bSize = TRUE;

	strPart = strURL.SpanExcluding( _T(" |/") );
	m_oSHA1.fromString( strPart );

	m_nAction = uriDownload;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse discovery service URL

BOOL CShareazaURL::ParseDiscovery(LPCTSTR pszURL, int nType)
{
	if ( _tcsncmp( pszURL, _T("http://"), 7 ) != 0 &&
		 _tcsncmp( pszURL, _T("https://"), 8 ) != 0 &&
		 _tcsncmp( pszURL, _T("uhc:"), 4 ) != 0 &&
		 _tcsncmp( pszURL, _T("ukhl:"), 5 ) != 0 &&
		 _tcsncmp( pszURL, _T("gnutella1:host:"), 15 ) != 0 &&
		 _tcsncmp( pszURL, _T("gnutella2:host:"), 15 ) != 0 ) return FALSE;

	int nPos;
	CString strURL, strNets, strTemp = pszURL;
	m_nProtocol = PROTOCOL_NULL;

	nPos = strTemp.Find( '?' );

	if ( nPos >= 0 )
	{
		strURL = strTemp.Left( nPos );
		strNets = strTemp.Mid( nPos + 1 );
	}
	else
		strURL = strTemp;

	if ( _tcsnicmp( strNets, _T("nets="), 5 ) == 0 )
	{
		BOOL bG1 = FALSE, bG2 = FALSE;

		if ( _tcsistr( strNets, (LPCTSTR)_T("gnutella2") ) )
		{
			bG2 = TRUE;
			strNets.Replace( _T("gnutella2"), _T("") );
		}

		if ( _tcsistr( strNets, (LPCTSTR)_T("gnutella") ) )
			bG1 = TRUE;

		if ( bG1 && bG2 )
			;
		else if ( bG2 )
		{
			m_nProtocol = PROTOCOL_G2;
		}
		else if ( bG1 )
		{
			if ( Settings.Discovery.EnableG1GWC )
				m_nProtocol = PROTOCOL_G1;
			else
				return FALSE;
		}
		else
			return FALSE;
	}

	nPos = strURL.Find( '|' );
	if ( nPos >= 0 )
		strURL = strURL.Left( nPos );

	m_nAction	= uriDiscovery;
	m_sURL		= strURL;
	m_nSize		= nType;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL URL string helpers

void CShareazaURL::SkipSlashes(LPCTSTR& pszURL, int nAdd)
{
	pszURL += nAdd;
	while ( *pszURL == '/' ) pszURL++;
}

void CShareazaURL::SafeString(CString& strInput)
{
	strInput.TrimLeft();
	strInput.TrimRight();

	for ( int nIndex = 0 ; nIndex < strInput.GetLength() ; nIndex++ )
	{
		TCHAR nChar = strInput.GetAt( nIndex );
		if ( nChar < 32 )
			strInput.SetAt( nIndex, '_' );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaURL query constructor

CQuerySearchPtr CShareazaURL::ToQuery() const
{
	if ( m_nAction != uriDownload && m_nAction != uriSearch )
		return CQuerySearchPtr();

	CQuerySearchPtr pSearch = new CQuerySearch();

	if ( m_sName.GetLength() )
	{
		pSearch->m_sSearch = m_sName;
	}

	if ( m_oSHA1 )
	{
		pSearch->m_oSHA1 = m_oSHA1;
	}

	if ( m_oTiger )
	{
		pSearch->m_oTiger = m_oTiger;
	}

	if ( m_oED2K )
	{
		pSearch->m_oED2K = m_oED2K;
	}

	if ( m_oBTH )
	{
		pSearch->m_oBTH = m_oBTH;
	}

	if ( m_oMD5 )
	{
		pSearch->m_oMD5 = m_oMD5;
	}

	return pSearch;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaURL shell registration

void CShareazaURL::Register(BOOL bRegister, BOOL bOnStartup)
{
	if ( bRegister )
	{
		RegisterShellType( NULL, _T("shareaza"), _T("URL:Shareaza P2P"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );

		RegisterMagnetHandler( _T("Shareaza"), _T("Shareaza Peer to Peer"), _T("Shareaza can automatically search for and download the selected content its peer-to-peer networks."), _T("Shareaza"), IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( _T("shareaza") );
	}

	if ( Settings.Web.Magnet && bRegister )
	{
		RegisterShellType( NULL, _T("magnet"), _T("URL:Magnet Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( _T("magnet") );
	}

	if ( Settings.Web.DC && bRegister )
	{
		RegisterShellType( NULL, _T("dchub"), _T("URL:DirectConnect Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
		RegisterShellType( NULL, _T("dcfile"), _T("URL:DirectConnect Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( _T("dchub") );
		UnregisterShellType( _T("dcfile") );
	}

	if ( Settings.Web.Foxy && bRegister )
	{
		RegisterShellType( NULL, _T("foxy"), _T("URL:Foxy Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( _T("foxy") );
	}

	if ( Settings.Web.Gnutella && bRegister )
	{
		RegisterShellType( NULL, _T("gnutella"), _T("URL:Gnutella Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
		RegisterShellType( NULL, _T("gnet"), _T("URL:Gnutella Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
		RegisterShellType( NULL, _T("uhc"), _T("URL:Gnutella1 UDP Host Cache"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
		RegisterShellType( NULL, _T("ukhl"), _T("URL:Gnutella2 UDP known Hub Cache"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
		RegisterShellType( NULL, _T("gnutella1"), _T("URL:Gnutella1 Bootstrap"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
		RegisterShellType( NULL, _T("gnutella2"), _T("URL:Gnutella2 Bootstrap"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
		RegisterShellType( NULL, _T("gwc"), _T("URL:GWC Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( _T("gnutella") );
		UnregisterShellType( _T("gnet") );
		UnregisterShellType( _T("uhc") );
		UnregisterShellType( _T("ukhl") );
		UnregisterShellType( _T("gnutella1") );
		UnregisterShellType( _T("gnutella2") );
		UnregisterShellType( _T("gwc") );
	}

	if ( Settings.Web.ED2K && bRegister )
	{
		RegisterShellType( NULL, _T("ed2k"), _T("URL:eDonkey2000 Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( _T("ed2k") );
	}

	if ( Settings.Web.Piolet && bRegister )
	{
		RegisterShellType( NULL, _T("mp2p"), _T("URL:Piolet Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( _T("mp2p") );
	}

	if ( Settings.Web.Torrent && bRegister )
	{
		if ( ! bOnStartup || ! Settings.Live.FirstRun )
		{
			RegisterShellType( NULL, _T("bittorrent"), _T("TORRENT File"), _T(".torrent"),
				_T("Shareaza"), _T("RAZAFORMAT"), IDR_MAINFRAME );
			RegisterShellType( _T("Applications\\Shareaza.exe"), NULL, _T("TORRENT File"), _T(".torrent"),
				_T("Shareaza"), _T("RAZAFORMAT"), IDR_MAINFRAME );
		}
	}
	else
	{
		UnregisterShellType( _T(".torrent") );
		UnregisterShellType( _T("bittorrent") );
	}

	if (  bRegister )
	{
		RegisterShellType( NULL, _T("Shareaza.Collection"), _T("Shareaza Collection File"),
			_T(".co"), _T("Shareaza"), _T("RAZAFORMAT"), IDI_COLLECTION );
		RegisterShellType( _T("Applications\\Shareaza.exe"), NULL, _T("Shareaza Collection File"),
			_T(".co"), _T("Shareaza"), _T("RAZAFORMAT"), IDI_COLLECTION );
		RegisterShellType( NULL, _T("Shareaza.Collection"), _T("Shareaza Collection File"),
			_T(".collection"), _T("Shareaza"), _T("RAZAFORMAT"), IDI_COLLECTION );
		RegisterShellType( _T("Applications\\Shareaza.exe"), NULL, _T("Shareaza Collection File"),
			_T(".collection"), _T("Shareaza"), _T("RAZAFORMAT"), IDI_COLLECTION );
		
		RegisterShellType( NULL, _T("eMule"), _T("eMule Collection File"),
			_T(".emulecollection"), _T("Shareaza"), _T("RAZAFORMAT"), IDI_COLLECTION );
		RegisterShellType( _T("Applications\\Shareaza.exe"), NULL, _T("eMule Collection File"),
			_T(".emulecollection"), _T("Shareaza"), _T("RAZAFORMAT"), IDI_COLLECTION );
	}
	else
	{
		UnregisterShellType( _T(".co") );		
		UnregisterShellType( _T(".collection") );
		UnregisterShellType( _T("Shareaza.Collection") );

		UnregisterShellType( _T(".emulecollection") );
		UnregisterShellType( _T("eMule") );
	}

	if ( ! bOnStartup )
		SHChangeNotify( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaURL shell registration helper

BOOL CShareazaURL::RegisterShellType(LPCTSTR pszRoot, LPCTSTR pszProtocol, LPCTSTR pszName,
									 LPCTSTR pszType, LPCTSTR pszApplication, LPCTSTR pszTopic,
									 UINT nIDIcon, BOOL bOverwrite)
{
	HKEY hRootKey = AfxGetPerUserRegistration() ? HKEY_CURRENT_USER : HKEY_CLASSES_ROOT;
	LPCTSTR szRootKey = AfxGetPerUserRegistration() ? _T("Software\\Classes") : _T("");

	HKEY hKey, hSub1, hSub2, hSub3, hSub4;
	CString strValue;
	DWORD nDisposition;

	CString strSubKey = szRootKey;
	if ( pszRoot )
	{
		if ( ! strSubKey.IsEmpty() )
			strSubKey += _T("\\");
		strSubKey += pszRoot;
	}
	if ( pszProtocol )
	{
		if ( ! strSubKey.IsEmpty() )
			strSubKey += _T("\\");
		strSubKey += pszProtocol;
	}

	if ( RegCreateKeyEx( hRootKey, (LPCTSTR)strSubKey, 0, NULL, 0,
		KEY_ALL_ACCESS, NULL, &hKey, &nDisposition ) )
		return FALSE;

	if ( nDisposition == REG_OPENED_EXISTING_KEY && ! bOverwrite )
	{
		RegCloseKey( hKey );
		return FALSE;
	}

	BOOL bProtocol = _tcsncmp( pszName, _T("URL:"), 4 ) == 0;

	if ( ! pszRoot || _tcscmp( pszRoot, _T("Applications\\Shareaza.exe") ) != 0 )
	{
		RegSetValueEx( hKey, NULL, 0, REG_SZ, (LPBYTE)pszName,
			static_cast< DWORD >( sizeof(TCHAR) * ( _tcslen( pszName ) + 1 ) ) );

		if ( bProtocol )
		{
			RegSetValueEx( hKey, _T("URL Protocol"), 0, REG_SZ, (LPBYTE)(LPCTSTR)strValue, sizeof(TCHAR) );
		}

		if ( ! RegCreateKey( hKey, _T("DefaultIcon"), &hSub1 ) )
		{
			strValue = Skin.GetImagePath( nIDIcon );
			RegSetValueEx( hSub1, NULL, 0, REG_SZ,
				(LPBYTE)(LPCTSTR)strValue, sizeof(TCHAR) * ( strValue.GetLength() + 1 ) );
			RegCloseKey( hSub1 );
		}
	}
	else if ( pszType != NULL )
	{
		HKEY hKeySupported;
		if ( ! RegCreateKey( hKey, _T("SupportedTypes"), &hKeySupported ) )
		{
			RegSetValueEx( hKeySupported, pszType, 0, REG_SZ, NULL, 0 );
			RegCloseKey( hKeySupported );
		}
	}

	if ( ! RegCreateKey( hKey, _T("shell"), &hSub1 ) )
	{
		if ( ! RegCreateKey( hSub1, _T("open"), &hSub2 ) )
		{
			if ( ! RegCreateKey( hSub2, _T("command"), &hSub3 ) )
			{
				strValue.Format( _T("\"%s\" \"%%%c\""), theApp.m_strBinaryPath, bProtocol ? 'L' : '1' );
				RegSetValueEx( hSub3, NULL, 0, REG_SZ, (LPBYTE)(LPCTSTR)strValue, sizeof(TCHAR) * ( strValue.GetLength() + 1 ) );
				RegCloseKey( hSub3 );
			}

			if ( ! RegCreateKey( hSub2, _T("ddeexec"), &hSub3 ) )
			{
				RegSetValueEx( hSub3, NULL, 0, REG_SZ, (LPBYTE)_T("%1"), sizeof(TCHAR) * 3 );

				if ( ! RegCreateKey( hSub3, _T("Application"), &hSub4 ) )
				{
					RegSetValueEx( hSub4, NULL, 0, REG_SZ, (LPBYTE)pszApplication,
						static_cast< DWORD >( sizeof(TCHAR) * ( _tcslen( pszApplication ) + 1 ) ) );
					RegCloseKey( hSub4 );
				}

				if ( ! RegCreateKey( hSub3, _T("Topic"), &hSub4 ) )
				{
					RegSetValueEx( hSub4, NULL, 0, REG_SZ, (LPBYTE)pszTopic,
						static_cast< DWORD >( sizeof(TCHAR) * ( _tcslen( pszTopic ) + 1 ) ) );
					RegCloseKey( hSub4 );
				}

				RegCloseKey( hSub3 );
			}

			RegCloseKey( hSub2 );
		}

		RegCloseKey( hSub1 );
	}

	if ( pszType != NULL && _tcsncmp( pszType, _T("."), 1 ) == 0 )
	{
		BYTE pData[4] = { 0x00, 0x11, 0x21, 0x00 };
		RegSetValueEx( hKey, _T("EditFlags"), 0, REG_BINARY, pData, 4 );
	}

	RegCloseKey( hKey );

	if ( pszType != NULL && pszProtocol != NULL )
	{
		strSubKey = szRootKey;
		if ( pszRoot )
		{
			if ( ! strSubKey.IsEmpty() )
				strSubKey += _T("\\");
			strSubKey += pszRoot;
		}
		if ( ! strSubKey.IsEmpty() )
			strSubKey += _T("\\");
		strSubKey += pszType;

		if ( !	RegCreateKeyEx( hRootKey, (LPCTSTR)strSubKey, 0, NULL, 0,
				KEY_ALL_ACCESS, NULL, &hKey, &nDisposition ) )
		{
			RegSetValueEx( hKey, NULL, 0, REG_SZ, (LPBYTE)pszProtocol,
				static_cast< DWORD >( sizeof(TCHAR) * ( _tcslen( pszProtocol ) + 1 ) ) );
			RegCloseKey( hKey );
		}
	}

	return TRUE;
}

BOOL CShareazaURL::UnregisterShellType(LPCTSTR pszRoot)
{
	HKEY hRootKey = AfxGetPerUserRegistration() ? HKEY_CURRENT_USER : HKEY_CLASSES_ROOT;
	LPCTSTR szRootKey = AfxGetPerUserRegistration() ? _T("Software\\Classes") : _T("");

	CString strSubKey = szRootKey, strOldKey;
	if ( pszRoot )
	{
		if ( ! strSubKey.IsEmpty() )
			strSubKey += _T("\\");
		strSubKey += pszRoot;
	}

	BOOL bRegisteredUser = FALSE;
	HKEY hKey;

	if ( pszRoot && *pszRoot == _T('.') )
	{
		// Get real key for file extension
		if ( RegOpenKeyEx( hRootKey, strSubKey, 0,
			KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
		{
			CString strPath;
			DWORD dwType;
			DWORD dwSize = MAX_PATH * sizeof( TCHAR );
			RegQueryValueEx( hKey, NULL,
				NULL, &dwType, (LPBYTE)strPath.GetBuffer( MAX_PATH ), &dwSize );
			strPath.ReleaseBuffer();
			if ( ! strPath.IsEmpty() )
			{
				strOldKey = strSubKey;

				strSubKey = szRootKey;
				if ( ! strSubKey.IsEmpty() )
					strSubKey += _T("\\");
				strSubKey += strPath;
			}
			RegCloseKey( hKey );
		}
	}

	if ( RegOpenKeyEx( hRootKey, strSubKey + _T("\\shell\\open\\command"), 0,
		KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
	{
		CString strPath;
		DWORD dwType;
		DWORD dwSize = MAX_PATH * sizeof( TCHAR );
		RegQueryValueEx( hKey, NULL,
			NULL, &dwType, (LPBYTE)strPath.GetBuffer( MAX_PATH ), &dwSize );
		strPath.ReleaseBuffer();
		if ( _tcsistr( strPath, theApp.m_strBinaryPath ) != NULL )
		{
			bRegisteredUser = TRUE;
		}
		RegCloseKey( hKey );
	}

	if ( ! bRegisteredUser )
	{
		if ( RegOpenKeyEx( hRootKey, strSubKey + _T("\\shell\\open\\ddeexec\\Application"), 0,
			KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
		{
			CString strPath;
			DWORD dwType;
			DWORD dwSize = MAX_PATH * sizeof( TCHAR );
			RegQueryValueEx( hKey, NULL,
				NULL, &dwType, (LPBYTE)strPath.GetBuffer( MAX_PATH ), &dwSize );
			strPath.ReleaseBuffer();
			if ( _tcsistr( strPath, _T("Shareaza") ) != NULL )
			{
				bRegisteredUser = TRUE;
			}
			RegCloseKey( hKey );
		}
	}

	if ( bRegisteredUser )
	{
		if ( strOldKey.IsEmpty() )
		{
			DeleteKey( hRootKey, (LPCTSTR)strSubKey );
			RegDeleteKey( hRootKey, (LPCTSTR)strSubKey );
		}
		else
		{
			DeleteKey( hRootKey, (LPCTSTR)strOldKey );
			RegDeleteKey( hRootKey, (LPCTSTR)strOldKey );
		}
	}

	return bRegisteredUser;
}

void CShareazaURL::DeleteKey(HKEY hParent, LPCTSTR pszKey)
{
	CArray< CString > pList;
	HKEY hKey;

	if ( RegOpenKeyEx( hParent, pszKey, 0, KEY_ALL_ACCESS, &hKey ) ) return;

	for ( DWORD dwIndex = 0 ; ; dwIndex++ )
	{
		DWORD dwName = 64; // Input parameter in TCHARs
		TCHAR szName[64];

		LRESULT lResult = RegEnumKeyEx( hKey, dwIndex, szName, &dwName, NULL, NULL, 0, NULL );
		if ( lResult != ERROR_SUCCESS ) break;

		szName[ dwName ] = 0;
		pList.Add( szName );
		DeleteKey( hKey, szName );
	}

	for ( int nItem = 0 ; nItem < pList.GetSize() ; nItem++ )
	{
		RegDeleteKey( hKey, pList.GetAt( nItem ) );
	}

	RegCloseKey( hKey );
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaURL magnet registration helper

BOOL CShareazaURL::RegisterMagnetHandler(LPCTSTR pszID, LPCTSTR pszName, LPCTSTR pszDescription, LPCTSTR pszApplication, UINT nIDIcon)
{
	HKEY hSoftware, hMagnetRoot, hHandlers, hHandler;
	DWORD dwDisposition;
	LONG lResult;

	lResult = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Software"), 0, KEY_ALL_ACCESS,
		&hSoftware );

	if ( lResult != ERROR_SUCCESS ) return FALSE;

	lResult = RegCreateKeyEx( hSoftware, _T("Magnet"), 0, NULL, 0, KEY_ALL_ACCESS,
		NULL, &hMagnetRoot, &dwDisposition );

	if ( lResult != ERROR_SUCCESS )
	{
		RegCloseKey( hSoftware );
		return FALSE;
	}

	lResult = RegCreateKeyEx( hMagnetRoot, _T("Handlers"), 0, NULL, 0, KEY_ALL_ACCESS,
		NULL, &hHandlers, &dwDisposition );

	if ( lResult != ERROR_SUCCESS )
	{
		RegCloseKey( hMagnetRoot );
		RegCloseKey( hSoftware );
		return FALSE;
	}

	lResult = RegCreateKeyEx( hHandlers, pszID, 0, NULL, 0, KEY_ALL_ACCESS,
		NULL, &hHandler, &dwDisposition );

	if ( lResult != ERROR_SUCCESS )
	{
		RegCloseKey( hHandler );
		RegCloseKey( hMagnetRoot );
		RegCloseKey( hSoftware );
		return FALSE;
	}

	CString strIcon, strCommand;

	strIcon = Skin.GetImagePath( nIDIcon );
	strCommand.Format( _T("\"%s\" \"%%URL\""), theApp.m_strBinaryPath );

	RegSetValueEx( hHandler, _T(""), 0, REG_SZ, (LPBYTE)pszName, static_cast< DWORD >( sizeof(TCHAR) * ( _tcslen( pszName ) + 1 ) ) );
	RegSetValueEx( hHandler, _T("Description"), 0, REG_SZ,
		(LPBYTE)pszDescription, static_cast< DWORD >( sizeof(TCHAR) * ( _tcslen( pszDescription ) + 1 ) ) );

	RegSetValueEx( hHandler, _T("DefaultIcon"), 0, REG_SZ,
		(LPBYTE)(LPCTSTR)strIcon, sizeof(TCHAR) * ( strIcon.GetLength() + 1 ) );

	RegSetValueEx( hHandler, _T("ShellExecute"), 0, REG_SZ,
		(LPBYTE)(LPCTSTR)strCommand, sizeof(TCHAR) * ( strCommand.GetLength() + 1 ) );

	RegSetValueEx( hHandler, _T("DdeApplication"), 0, REG_SZ,
		(LPBYTE)pszApplication, static_cast< DWORD >( sizeof(TCHAR) * ( _tcslen( pszApplication ) + 1 ) ) );

	RegSetValueEx( hHandler, _T("DdeTopic"), 0, REG_SZ, (LPBYTE)_T("URL"), sizeof(TCHAR) * 4 );

	RegCloseKey( hHandler );
	RegCloseKey( hHandlers );
	RegCloseKey( hMagnetRoot );
	RegCloseKey( hSoftware );

	return TRUE;
}
