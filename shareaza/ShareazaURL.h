//
// ShareazaURL.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

#pragma once

#include "DiscoveryServices.h"
#include "QuerySearch.h"

class CBTInfo;


class CShareazaURL : public CShareazaFile
{
public:
	CShareazaURL(LPCTSTR pszURL = NULL);
	CShareazaURL(CBTInfo* pTorrent);
	CShareazaURL(const CShareazaURL& pURL);
	virtual ~CShareazaURL();

	enum URI_TYPE
	{
		uriNull, uriSource, uriDownload, uriSearch, uriHost, uriBrowse, uriDiscovery, uriCommand
	};

	PROTOCOLID			m_nProtocol;
	URI_TYPE			m_nAction;
	CAutoPtr< CBTInfo >	m_pTorrent;
	CString				m_sAddress;
	IN_ADDR				m_pAddress;
	WORD				m_nPort;
	IN_ADDR				m_pServerAddress;
	WORD				m_nServerPort;
	CString				m_sLogin;
	CString				m_sPassword;
	Hashes::BtGuid		m_oBTC;

	// Parse URL list
	BOOL	Parse(const CString& sText, CList< CString >& pURLs, BOOL bResolve = FALSE);
	// Parse single URL
	BOOL	Parse(LPCTSTR pszURL, BOOL bResolve = TRUE);
	// Construct CQuerySearch object
	CQuerySearchPtr ToQuery() const;

	CDiscoveryService::Type	GetDiscoveryService() const
	{
		return (CDiscoveryService::Type)(int)m_nSize;
	}

protected:
	void	Clear();

	BOOL	ParseRoot(LPCTSTR pszURL, BOOL bResolve);
	// http://[user[:password]@]host[:port]/[filepath], where {filepath} is a regular path or "/uri-res/N2R?hash"
	BOOL	ParseHTTP(LPCTSTR pszURL, BOOL bResolve);
	// ftp://[user[:password]@]host[:port][/path]
	BOOL	ParseFTP(LPCTSTR pszURL, BOOL bResolve);
	// ed2kftp://[client_id@]address:port/{md4_hash}/{size}/
	BOOL	ParseED2KFTP(LPCTSTR pszURL, BOOL bResolve);
	// dchub://[login@]address:port/[filepath], where {filepath} can be a regular path or "files.xml.bz2" or "TTH:tiger_hash/size/"
	BOOL	ParseDCHub(LPCTSTR pszURL, BOOL bResolve);
	// btc://address:port/[{node_guid}]/{btih_hash}/
	BOOL	ParseBTC(LPCTSTR pszURL, BOOL bResolve);
	// magnet:?{params}
	BOOL	ParseMagnet(LPCTSTR pszURL);
	// Host
	//	shareaza:[//]{verb}{[user@]address[:port]}, where {verb} is "" (empty), "host:", "hub:", "server:", "browse:" or "btnode:"
	// WebCache
	//	shareaza:[//]gwc:{url}[?nets={net_list}], where {net_list} is "gnutella" or "gnutella2"
	// ServerMet
	//	shareaza:[//]meturl:{url}
	// Discovery
	//	shareaza:[//]{verb}{url}, where {verb} is "uhc:", "ukhl:", "gnutella1:host:" or "gnutella2:host:"
	// URL
	//	shareaza:[//]url:{nested_url}
	BOOL	ParseShareaza(LPCTSTR pszURL);
	BOOL	ParseShareazaHost(LPCTSTR pszURL, BOOL bBrowse = FALSE, PROTOCOLID nProtocol = PROTOCOL_G2);
	BOOL	ParseShareazaFile(LPCTSTR pszURL);
	BOOL	ParseDiscovery(LPCTSTR pszURL, int nType);
	// ed2k://|file|{name}|{size}|{md4_hash}|/
	// ed2k://|server|{address}|{port}|/
	// ed2k://|search|{query}|/
	BOOL	ParseDonkey(LPCTSTR pszURL);
	BOOL	ParseDonkeyFile(LPCTSTR pszURL);
	BOOL	ParseDonkeyServer(LPCTSTR pszURL);
	// mp2p://[|]file|{name}|{size}|{sha1_hash}/
	BOOL	ParsePiolet(LPCTSTR pszURL);
	BOOL	ParsePioletFile(LPCTSTR pszURL);

// Registration Operations
public:
	static void	Register(BOOL bRegister = TRUE, BOOL bOnStartup = FALSE);

protected:
	static BOOL	RegisterShellType(LPCTSTR pszRoot, LPCTSTR pszProtocol, 
		LPCTSTR pszName, LPCTSTR pszType, LPCTSTR pszApplication, LPCTSTR pszTopic, UINT nIDIcon, BOOL bOverwrite = TRUE);
	static BOOL	RegisterMagnetHandler(LPCTSTR pszID, LPCTSTR pszName, LPCTSTR pszDescription, LPCTSTR pszApplication, UINT nIDIcon);
	static BOOL	UnregisterShellType(LPCTSTR pszRoot);
};
