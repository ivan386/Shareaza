//
// ShareazaURL.cpp
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
#include "ShareazaURL.h"
#include "Transfer.h"
#include "QuerySearch.h"
#include "DiscoveryServices.h"

#include "TigerTree.h"
#include "SHA.h"
#include "MD5.h"
#include "ED2K.h"
#include "BTInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CShareazaURL construction

CShareazaURL::CShareazaURL()
{
	m_pTorrent = NULL;
	Clear();
}

CShareazaURL::CShareazaURL(CBTInfo* pTorrent)
{
	m_pTorrent = NULL;
	Clear();
	m_nAction	= uriDownload;
	m_pTorrent	= pTorrent;
	m_oBTH		= pTorrent->m_oInfoBTH;
	m_oSHA1		= pTorrent->m_oDataBTH;
	m_sName		= pTorrent->m_sName;
	m_bSize		= TRUE;
	m_nSize		= pTorrent->m_nTotalSize;
}

CShareazaURL::~CShareazaURL()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL clear

void CShareazaURL::Clear()
{
	m_nAction	= uriNull;
	m_oSHA1.Clear();
	m_oTiger.Clear();
	m_oMD5.Clear();
	m_oED2K.Clear();
	m_oBTH.Clear();
	m_bSize		= FALSE;
	m_nPort		= GNUTELLA_DEFAULT_PORT;
	
	if ( m_pTorrent != NULL ) delete m_pTorrent;
	m_pTorrent = NULL;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL root parser

BOOL CShareazaURL::Parse(LPCTSTR pszURL)
{
	Clear();
	
	if ( _tcsnicmp( pszURL, _T("magnet:?"), 8 ) == 0 )
	{
		pszURL += 8;
		return ParseMagnet( pszURL );
	}
	else if (	_tcsnicmp( pszURL, _T("shareaza:"), 9 ) == 0 ||
				_tcsnicmp( pszURL, _T("gnutella:"), 9 ) == 0 )
	{
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
	else if ( _tcsnicmp( pszURL, _T("http://"), 7 ) == 0 )
	{
		m_sURL		= pszURL;
		m_nAction	= uriSource;
		return TRUE;
	}
	else if ( _tcsnicmp( pszURL, _T("ftp://"), 6 ) == 0 )
	{
		m_sURL		= pszURL;
		m_nAction	= uriSource;
		return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse "magnet:" URLs

BOOL CShareazaURL::ParseMagnet(LPCTSTR pszURL)
{
	CString strURL( pszURL );
	CHashSHA1 oSHA1;
	CHashTiger oTiger;
	CHashMD5 oMD5;
	CHashED2K oED2K;
	for ( strURL += '&' ; strURL.GetLength() ; )
	{
		CString strPart = strURL.SpanExcluding( _T("&") );
		strURL = strURL.Mid( strPart.GetLength() + 1 );
		
		int nEquals = strPart.Find( '=' );
		if ( nEquals < 0 ) continue;
		
		CString strKey		= CTransfer::URLDecode( strPart.Left( nEquals ) );
		CString strValue	= CTransfer::URLDecode( strPart.Mid( nEquals + 1 ) );
		
		SafeString( strKey );
		SafeString( strValue );
		
		if ( strKey.IsEmpty() || strValue.IsEmpty() ) continue;
		
		if ( _tcsicmp( strKey, _T("xt") ) == 0 ||
			 _tcsicmp( strKey, _T("xs") ) == 0 ||
			 _tcsicmp( strKey, _T("as") ) == 0 )
		{
			if (	_tcsnicmp( strValue, _T("urn:"), 4 ) == 0 ||
					_tcsnicmp( strValue, _T("sha1:"), 5 ) == 0 ||
					_tcsnicmp( strValue, _T("bitprint:"), 9 ) == 0 ||
					_tcsnicmp( strValue, _T("tree:tiger:"), 11 ) == 0 ||
					_tcsnicmp( strValue, _T("tree:tiger/:"), 12 ) == 0 ||
					_tcsnicmp( strValue, _T("tree:tiger/1024:"), 16 ) == 0 ||
					_tcsnicmp( strValue, _T("md5:"), 4 ) == 0 ||
					_tcsnicmp( strValue, _T("ed2k:"), 5 ) == 0 )
			{
				if ( oSHA1.FromURN( strValue ) ) m_oSHA1 = oSHA1;
				if ( oTiger.FromURN( strValue ) ) m_oTiger = oTiger;
				if ( oMD5.FromURN( strValue ) ) m_oMD5 = oMD5;
				if ( oED2K.FromURN( strValue ) ) m_oED2K = oED2K;
			}
			else if (	_tcsnicmp( strValue, _T("http://"), 7 ) == 0 ||
						_tcsnicmp( strValue, _T("http%3A//"), 9 ) == 0 ||
						_tcsnicmp( strValue, _T("ftp://"), 6 ) == 0 ||
						_tcsnicmp( strValue, _T("ftp%3A//"), 8 ) == 0 )
			{
				Replace( strValue, _T(" "), _T("%20") );
				Replace( strValue, _T("p%3A//"), _T("p://") );
				
				if ( _tcsicmp( strKey, _T("xt") ) == 0 )
				{
					CString strURL = _T("@") + strValue;
					
					if ( m_sURL.GetLength() )
						m_sURL = strURL + _T(", ") + m_sURL;
					else
						m_sURL = strURL;
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
			m_oSHA1.Clear();
		}
	}
	
	if ( m_oSHA1.IsValid() || m_oTiger.IsValid() || m_oMD5.IsValid() || m_oED2K.IsValid() || m_sURL.GetLength() )
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
	int nIP[4];
	
	if ( _stscanf( pszURL, _T("%i.%i.%i.%i"), &nIP[0], &nIP[1], &nIP[2], &nIP[3] ) == 4 )
	{
		return ParseShareazaHost( pszURL );
	}
	
	if ( _tcsnicmp( pszURL, _T("host:"), 5 ) == 0 ||
		 _tcsnicmp( pszURL, _T("node:"), 5 ) == 0 )
	{
		return ParseShareazaHost( pszURL + 5 );
	}
	else if ( _tcsnicmp( pszURL, _T("hub:"), 4 ) == 0 )
	{
		return ParseShareazaHost( pszURL + 4 );
	}
	else if ( _tcsnicmp( pszURL, _T("server:"), 7 ) == 0 )
	{
		return ParseShareazaHost( pszURL + 7 );
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
	
	m_sName.TrimLeft();
	m_sName.TrimRight();
	
	m_nAction = bBrowse ? uriBrowse : uriHost;
	
	return m_sName.GetLength();
}

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse shareaza file URL

BOOL CShareazaURL::ParseShareazaFile(LPCTSTR pszURL)
{
	CString strURL( pszURL );
	CHashSHA1 oSHA1;
	CHashTiger oTiger;
	CHashMD5 oMD5;
	CHashED2K oED2K;
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
				_tcsnicmp( strPart, _T("ed2k:"), 5 ) == 0 )
		{
			if ( oSHA1.FromURN( strPart ) ) m_oSHA1 = oSHA1;
			if ( oTiger.FromURN( strPart ) ) m_oTiger = oTiger;
			if ( oMD5.FromURN( strPart ) ) m_oMD5 = oMD5;
			if ( oED2K.FromURN( strPart ) ) m_oED2K = oED2K;
		}
		else if ( _tcsnicmp( strPart, _T("source:"), 7 ) == 0 )
		{
			CString strSource = CTransfer::URLDecode( strPart.Mid( 7 ) );
			SafeString( strSource );

			if ( m_sURL.GetLength() ) m_sURL += ',';
			m_sURL += _T("http://");
			m_sURL += CTransfer::URLEncode( strSource );
			m_sURL += _T("/(^name^)");
		}
		else if (	_tcsnicmp( strPart, _T("name:"), 5 ) == 0 ||
					_tcsnicmp( strPart, _T("file:"), 5 ) == 0 )
		{
			m_sName = CTransfer::URLDecode( strPart.Mid( 5 ) );
			SafeString( m_sName );
		}
		else if ( _tcschr( strPart, ':' ) == NULL )
		{
			m_sName = CTransfer::URLDecode( strPart );
			SafeString( m_sName );
		}
	}
	
	if ( m_sURL.GetLength() )
	{
		if ( m_sName.GetLength() )
		{
			Replace( m_sURL, _T("(^name^)"), CTransfer::URLEncode( m_sName ) );
			Replace( m_sURL, _T("\\"), _T("/") );
		}
		else
		{
			m_sURL.Empty();
		}
	}
	
	if ( m_oSHA1.IsValid() || m_oTiger.IsValid() || m_oMD5.IsValid() || m_oED2K.IsValid() || m_sURL.GetLength() )
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
	if ( _tcsnicmp( pszURL, _T("|file|"), 6 ) == 0 )
	{
		return ParseDonkeyFile( pszURL + 6 );
	}
	else if ( _tcsnicmp( pszURL, _T("|server|"), 8 ) == 0 )
	{
		return ParseDonkeyServer( pszURL + 8 );
	}
	else if ( _tcsnicmp( pszURL, _T("|meturl|"), 8 ) == 0 )
	{
		return ParseDiscovery( pszURL + 8, CDiscoveryService::dsServerMet );
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
	
	nSep = strURL.Find( '|' );
	if ( nSep < 0 ) return FALSE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );
	
	m_sName = CTransfer::URLDecode( strPart );
	SafeString( m_sName );
	if ( m_sName.IsEmpty() ) return FALSE;
	
	nSep = strURL.Find( '|' );
	if ( nSep < 0 ) return FALSE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );
	
	if ( _stscanf( strPart, _T("%I64i"), &m_nSize ) != 1 ) return FALSE;
	m_bSize = TRUE;
	
	nSep = strURL.Find( '|' );
	if ( nSep < 0 ) return FALSE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );
	
	m_oED2K.FromString( strPart );
	
	m_nAction = uriDownload;
	
	return TRUE;
}

// ed2k://|file|Shareaza1600.exe|789544|3fb626ed1a9f4cb9921107f510148370|/

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse eDonkey2000 server URL

BOOL CShareazaURL::ParseDonkeyServer(LPCTSTR pszURL)
{
	LPCTSTR pszPort = _tcschr( pszURL, '|' );
	if ( pszPort == NULL ) return FALSE;
	
	if ( _stscanf( pszPort + 1, _T("%i"), &m_nPort ) != 1 ) return FALSE;
	
	m_sName = pszURL;
	m_sName = m_sName.Left( pszPort - pszURL );
	
	m_sName.TrimLeft();
	m_sName.TrimRight();
	if ( m_sName.IsEmpty() ) return FALSE;
	
	m_nAction	= uriDonkeyServer;
	
	return TRUE;
}

// ed2k://|server|1.2.3.4|4661|/

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse "mp2p:" URLs

BOOL CShareazaURL::ParsePiolet(LPCTSTR pszURL)
{
	if ( _tcsnicmp( pszURL, _T("file|"), 5 ) == 0 )
	{
		return ParsePioletFile( pszURL + 5 );
	}
	else if ( _tcsnicmp( pszURL, _T("|file|"), 6 ) == 0 )
	{
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
	
	m_sName = CTransfer::URLDecode( strPart );
	SafeString( m_sName );
	if ( m_sName.IsEmpty() ) return FALSE;
	
	nSep = strURL.Find( '|' );
	if ( nSep < 0 ) return FALSE;
	strPart	= strURL.Left( nSep );
	strURL	= strURL.Mid( nSep + 1 );
	
	if ( _stscanf( strPart, _T("%I64i"), &m_nSize ) != 1 ) return FALSE;
	m_bSize = TRUE;
	
	strPart = strURL.SpanExcluding( _T(" |/") );
	m_oSHA1.FromString( strPart );
	
	m_nAction = uriDownload;
	
	return TRUE;
}

// mp2p://file|Shareaza1600.exe|789544|3fb626ed1a9f4cb9921107f510148370/

//////////////////////////////////////////////////////////////////////
// CShareazaURL parse discovery service URL

BOOL CShareazaURL::ParseDiscovery(LPCTSTR pszURL, int nType)
{
	if ( _tcsncmp( pszURL, _T("http://"), 7 ) != 0 &&
		 _tcsncmp( pszURL, _T("https://"), 8 ) != 0 ) return FALSE;
	
	m_nAction	= uriDiscovery;
	m_sURL		= pszURL;
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
		if ( nChar >= 0 && nChar < 32 ) strInput.SetAt( nIndex, '_' );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaURL query constructor

CQuerySearch* CShareazaURL::ToQuery()
{
	if ( m_nAction != uriDownload && m_nAction != uriSearch ) return FALSE;
	
	CQuerySearch* pSearch = new CQuerySearch();
	
	if ( m_sName.GetLength() )
	{
		pSearch->m_sSearch = m_sName;
	}
	
	if ( m_oSHA1.IsValid() )
	{
		pSearch->m_oSHA1 = m_oSHA1;
	}
	
	if ( m_oED2K.IsValid() )
	{
		pSearch->m_oED2K = m_oED2K;
	}
	
	return pSearch;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaURL shell registration

void CShareazaURL::Register()
{
	RegisterShellType( _T("shareaza"), _T("URL:Shareaza P2P"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
	RegisterMagnetHandler( _T("Shareaza"), _T("Shareaza Peer to Peer"), _T("Shareaza can automatically search for and download the selected content its peer-to-peer networks."), _T("Shareaza"), IDR_MAINFRAME );
	
	if ( Settings.Web.Magnet )
	{
		RegisterShellType( _T("magnet"), _T("URL:Magnet Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( _T("magnet") );
	}
	
	if ( Settings.Web.Gnutella )
	{
		RegisterShellType( _T("gnutella"), _T("URL:Gnutella Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
		RegisterShellType( _T("gnet"), _T("URL:Gnutella Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( _T("gnutella") );
		UnregisterShellType( _T("gnet") );
	}
	
	if ( Settings.Web.ED2K )
	{
		RegisterShellType( _T("ed2k"), _T("URL:eDonkey2000 Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( _T("ed2k") );
	}
	
	if ( Settings.Web.Piolet )
	{
		RegisterShellType( _T("mp2p"), _T("URL:Piolet Protocol"), NULL, _T("Shareaza"), _T("URL"), IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( _T("mp2p") );
	}
	
	if ( Settings.Web.Torrent )
	{
		RegisterShellType( _T("bittorrent"), _T("TORRENT File"), _T(".torrent"),
			_T("Shareaza"), _T("TORRENT"), IDR_MAINFRAME );
	}
	else
	{
		UnregisterShellType( _T("bittorrent") );
	}
	
	RegisterShellType( _T("Shareaza.Collection"), _T("Shareaza Collection File"),
		_T(".co"), _T("Shareaza"), _T("COLLECTION"), IDI_COLLECTION );
	
	RegisterShellType( _T("Shareaza.Collection"), _T("Shareaza Collection File"),
		_T(".collection"), _T("Shareaza"), _T("COLLECTION"), IDI_COLLECTION );
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaURL shell registration helper

BOOL CShareazaURL::RegisterShellType(LPCTSTR pszProtocol, LPCTSTR pszName, LPCTSTR pszType, LPCTSTR pszApplication, LPCTSTR pszTopic, UINT nIDIcon, BOOL bOverwrite)
{
	HKEY hKey, hSub1, hSub2, hSub3, hSub4;
	CString strProgram, strValue;
	DWORD nDisposition;
	TCHAR szPath[128];
	
	if ( RegCreateKeyEx( HKEY_CLASSES_ROOT, pszProtocol, 0, NULL, 0,
		KEY_ALL_ACCESS, NULL, &hKey, &nDisposition ) ) return FALSE;
	
	if ( nDisposition == REG_OPENED_EXISTING_KEY && ! bOverwrite )
	{
		RegCloseKey( hKey );
		return FALSE;
	}
	
	BOOL bProtocol = _tcsncmp( pszName, _T("URL:"), 4 ) == 0;
	GetModuleFileName( NULL, szPath, 128 );
	strProgram = szPath;
	
	RegSetValueEx( hKey, NULL, 0, REG_SZ, (LPBYTE)pszName, sizeof(TCHAR) * ( _tcslen( pszName ) + 1 ) );
	
	if ( bProtocol )
	{
		RegSetValueEx( hKey, _T("URL Protocol"), 0, REG_SZ, (LPBYTE)(LPCTSTR)strValue, sizeof(TCHAR) );
	}
	
	if ( ! RegCreateKey( hKey, _T("DefaultIcon"), &hSub1 ) )
	{
		strValue.Format( _T("\"%s\",-%u"), (LPCTSTR)strProgram, nIDIcon );
		RegSetValueEx( hSub1, NULL, 0, REG_SZ, (LPBYTE)(LPCTSTR)strValue, sizeof(TCHAR) * ( strValue.GetLength() + 1 ) );
		RegCloseKey( hSub1 );
	}
	
	if ( ! RegCreateKey( hKey, _T("shell"), &hSub1 ) )
	{
		if ( ! RegCreateKey( hSub1, _T("open"), &hSub2 ) )
		{
			if ( ! RegCreateKey( hSub2, _T("command"), &hSub3 ) )
			{
				strValue.Format( _T("\"%s\" \"%%%c\""), (LPCTSTR)strProgram, bProtocol ? 'L' : '1' );
				RegSetValueEx( hSub3, NULL, 0, REG_SZ, (LPBYTE)(LPCTSTR)strValue, sizeof(TCHAR) * ( strValue.GetLength() + 1 ) );
				RegCloseKey( hSub3 );
			}
			
			if ( ! RegCreateKey( hSub2, _T("ddeexec"), &hSub3 ) )
			{
				RegSetValueEx( hSub3, NULL, 0, REG_SZ, (LPBYTE)_T("%1"), sizeof(TCHAR) * 3 );
				
				if ( ! RegCreateKey( hSub3, _T("Application"), &hSub4 ) )
				{
					RegSetValueEx( hSub4, NULL, 0, REG_SZ, (LPBYTE)pszApplication,
						sizeof(TCHAR) * ( _tcslen( pszApplication ) + 1 ) );
					RegCloseKey( hSub4 );
				}
				
				if ( ! RegCreateKey( hSub3, _T("Topic"), &hSub4 ) )
				{
					RegSetValueEx( hSub4, NULL, 0, REG_SZ, (LPBYTE)pszTopic,
						sizeof(TCHAR) * ( _tcslen( pszTopic ) + 1 ) );
					RegCloseKey( hSub4 );
				}
				
				RegCloseKey( hSub3 );
			}
			
			RegCloseKey( hSub2 );
		}
		
		RegCloseKey( hSub1 );
	}
	
	if ( pszType != NULL && _tcscmp( pszType, _T(".torrent") ) == 0 )
	{
		BYTE pData[4] = { 0x00, 0x00, 0x01, 0x00 };
		RegSetValueEx( hKey, _T("EditFlags"), 0, REG_BINARY, pData, 4 );
	}
	
	RegCloseKey( hKey );
	
	if ( pszType != NULL )
	{
		if ( !	RegCreateKeyEx( HKEY_CLASSES_ROOT, pszType, 0, NULL, 0,
				KEY_ALL_ACCESS, NULL, &hKey, &nDisposition ) )
		{
			RegSetValueEx( hKey, NULL, 0, REG_SZ, (LPBYTE)pszProtocol,
				sizeof(TCHAR) * ( _tcslen( pszProtocol ) + 1 ) );
			RegCloseKey( hKey );
		}
	}
	
	return TRUE;
}

BOOL CShareazaURL::IsRegistered(LPCTSTR pszProtocol)
{
	HKEY hKey[4];
	
	if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, pszProtocol, 0, KEY_READ, &hKey[0] ) ) return FALSE;
	
	TCHAR szApp[128];
	szApp[0] = 0;
	
	if ( RegOpenKeyEx( hKey[0], _T("shell"), 0, KEY_READ, &hKey[1] ) == 0 )
	{
		if ( RegOpenKeyEx( hKey[1], _T("open"), 0, KEY_READ, &hKey[2] ) == 0 )
		{
			if ( RegOpenKeyEx( hKey[2], _T("command"), 0, KEY_READ, &hKey[3] ) == 0 )
			{
				DWORD nType	= REG_SZ;
				DWORD nApp	= sizeof(TCHAR) * 127;
				RegQueryValueEx( hKey[3], NULL, NULL, &nType, (LPBYTE)szApp, &nApp );
				szApp[ nApp / sizeof(TCHAR) ] = 0;
				RegCloseKey( hKey[3] );
			}
			RegCloseKey( hKey[2] );
		}
		RegCloseKey( hKey[1] );
	}
	
	RegCloseKey( hKey[0] );
	
	TCHAR szPath[128];
	GetModuleFileName( NULL, szPath, 128 );
	
	return _tcsistr( szApp, szPath ) != NULL;
}

BOOL CShareazaURL::UnregisterShellType(LPCTSTR pszProtocol)
{
	if ( ! IsRegistered( pszProtocol ) ) return FALSE;
	
	DeleteKey( HKEY_CLASSES_ROOT, pszProtocol );
	RegDeleteKey( HKEY_CLASSES_ROOT, pszProtocol );
	
	return TRUE;
}

void CShareazaURL::DeleteKey(HKEY hParent, LPCTSTR pszKey)
{
	CStringArray pList;
	HKEY hKey;
	
	if ( RegOpenKeyEx( hParent, pszKey, 0, KEY_ALL_ACCESS, &hKey ) ) return;
	
	for ( DWORD dwIndex = 0 ; ; dwIndex++ )
	{
		DWORD dwName = 64 * sizeof(TCHAR);
		TCHAR szName[64];
		
		LRESULT lResult = RegEnumKeyEx( hKey, dwIndex, szName, &dwName, NULL, NULL, 0, NULL );
		if ( lResult != ERROR_SUCCESS ) break;
		
		szName[ dwName / sizeof(TCHAR) ] = 0;
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
	
	lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("Software"), 0, KEY_ALL_ACCESS,
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
	
	CString strAppPath, strIcon, strCommand;
	TCHAR szPath[128];
	
	GetModuleFileName( NULL, szPath, 128 );
	strAppPath = szPath;
	
	strIcon.Format( _T("\"%s\",-%u"), (LPCTSTR)strAppPath, nIDIcon );
	strCommand.Format( _T("\"%s\" \"%%URL\""), (LPCTSTR)strAppPath );
	
	RegSetValueEx( hHandler, _T(""), 0, REG_SZ, (LPBYTE)pszName, sizeof(TCHAR) * ( _tcslen( pszName ) + 1 ) );
	RegSetValueEx( hHandler, _T("Description"), 0, REG_SZ,
		(LPBYTE)pszDescription, sizeof(TCHAR) * ( _tcslen( pszDescription ) + 1 ) );
	
	RegSetValueEx( hHandler, _T("DefaultIcon"), 0, REG_SZ,
		(LPBYTE)(LPCTSTR)strIcon, sizeof(TCHAR) * ( strIcon.GetLength() + 1 ) );
	
	RegSetValueEx( hHandler, _T("ShellExecute"), 0, REG_SZ,
		(LPBYTE)(LPCTSTR)strCommand, sizeof(TCHAR) * ( strCommand.GetLength() + 1 ) );
	
	RegSetValueEx( hHandler, _T("DdeApplication"), 0, REG_SZ,
		(LPBYTE)pszApplication, sizeof(TCHAR) * ( _tcslen( pszApplication ) + 1 ) );
	
	RegSetValueEx( hHandler, _T("DdeTopic"), 0, REG_SZ, (LPBYTE)_T("URL"), sizeof(TCHAR) * 4 );
	
	RegCloseKey( hHandler );
	RegCloseKey( hHandlers );
	RegCloseKey( hMagnetRoot );
	RegCloseKey( hSoftware );
	
	return TRUE;
}

