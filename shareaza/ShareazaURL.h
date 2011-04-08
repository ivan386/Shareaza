//
// ShareazaURL.h
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

#pragma once

#include "QuerySearch.h"

class CBTInfo;


class CShareazaURL : public CShareazaFile
{
// Construction
public:
	CShareazaURL(LPCTSTR pszURL = NULL);
	CShareazaURL(CBTInfo* pTorrent);
	CShareazaURL(const CShareazaURL& pURL);
	virtual ~CShareazaURL();

// Attributes
public:
	enum URI_TYPE
	{
		uriNull, uriSource, uriDownload, uriSearch, uriHost, uriBrowse,
		uriDonkeyServer, uriDiscovery
	};

	PROTOCOLID	m_nProtocol;
	URI_TYPE	m_nAction;
	CBTInfo*	m_pTorrent;
	CString		m_sAddress;
	IN_ADDR		m_pAddress;
	WORD		m_nPort;
	IN_ADDR		m_pServerAddress;
	WORD		m_nServerPort;
	BOOL		m_bSize;
	CString		m_sLogin;
	CString		m_sPassword;
	Hashes::BtGuid m_oBTC;

// Operations
public:
	// Parse URL list
	BOOL	Parse(const CString& sText, CList< CString >& pURLs, BOOL bResolve = FALSE);
	// Parse single URL
	BOOL	Parse(LPCTSTR pszURL, BOOL bResolve = TRUE);
	// Construct CQuerySearch object
	CQuerySearchPtr ToQuery() const;

protected:
	void	Clear();

	BOOL	ParseRoot(LPCTSTR pszURL, BOOL bResolve);
	BOOL	ParseHTTP(LPCTSTR pszURL, BOOL bResolve);
	BOOL	ParseFTP(LPCTSTR pszURL, BOOL bResolve);
	// ed2kftp://[client_id@]address:port/md4_hash/size/
	BOOL	ParseED2KFTP(LPCTSTR pszURL, BOOL bResolve);
	// dchub://[login@]address:port/[filepath]
	// where {filepath} can be a regular path or "files.xml.bz2" or "TTH:tiger_hash/size/"
	BOOL	ParseDCHub(LPCTSTR pszURL, BOOL bResolve);
	// btc://address:port/[node_guid]/btih_hash/
	BOOL	ParseBTC(LPCTSTR pszURL, BOOL bResolve);
	BOOL	ParseMagnet(LPCTSTR pszURL);
	BOOL	ParseShareaza(LPCTSTR pszURL);
	BOOL	ParseShareazaHost(LPCTSTR pszURL, BOOL bBrows);
	BOOL	ParseShareazaFile(LPCTSTR pszURL);
	BOOL	ParseDonkey(LPCTSTR pszURL);
	BOOL	ParseDonkeyFile(LPCTSTR pszURL);
	BOOL	ParseDonkeyServer(LPCTSTR pszURL);
	BOOL	ParsePiolet(LPCTSTR pszURL);
	BOOL	ParsePioletFile(LPCTSTR pszURL);
	BOOL	ParseDiscovery(LPCTSTR pszURL, int nType);

	static void	SkipSlashes(LPCTSTR& pszURL, int nAdd = 0);
	static void	SafeString(CString& strInput);

// Registration Operations
public:
	static void	Register(BOOL bRegister = TRUE, BOOL bOnStartup = FALSE);

protected:
	static BOOL	RegisterShellType(LPCTSTR pszRoot, LPCTSTR pszProtocol, 
		LPCTSTR pszName, LPCTSTR pszType, LPCTSTR pszApplication, LPCTSTR pszTopic, UINT nIDIcon, BOOL bOverwrite = TRUE);
	static BOOL	RegisterMagnetHandler(LPCTSTR pszID, LPCTSTR pszName, LPCTSTR pszDescription, LPCTSTR pszApplication, UINT nIDIcon);
	static BOOL	UnregisterShellType(LPCTSTR pszRoot);
	static void DeleteKey(HKEY hParent, LPCTSTR pszKey);
};
