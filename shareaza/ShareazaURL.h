//
// ShareazaURL.h
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

#if !defined(AFX_SHAREAZAURL_H__B39B7816_FE18_4843_A10A_C0DB32D96E52__INCLUDED_)
#define AFX_SHAREAZAURL_H__B39B7816_FE18_4843_A10A_C0DB32D96E52__INCLUDED_

#pragma once

class CQuerySearch;
class CBTInfo;


class CShareazaURL  
{
// Construction
public:
	CShareazaURL();
	CShareazaURL(CBTInfo* pTorrent);
	virtual ~CShareazaURL();
	
// Attributes
public:
	int			m_nAction;
public:
	BOOL		m_bSHA1;
	SHA1		m_pSHA1;
	BOOL		m_bTiger;
	TIGEROOT	m_pTiger;
	BOOL		m_bMD5;
	MD5			m_pMD5;
	BOOL		m_bED2K;
	MD4			m_pED2K;
	BOOL		m_bBTH;
	SHA1		m_pBTH;
public:
	CString		m_sName;
	BOOL		m_bSize;
	QWORD		m_nSize;
	int			m_nPort;
	CString		m_sURL;
	CBTInfo*	m_pTorrent;
	
	enum
	{
		uriNull, uriSource, uriDownload, uriSearch, uriHost, uriBrowse,
		uriDonkeyServer, uriDiscovery
	};

// Operations
public:
	void			Clear();
	BOOL			Parse(LPCTSTR pszURL);
	CQuerySearch*	ToQuery();
protected:
	BOOL	ParseMagnet(LPCTSTR pszURL);
	BOOL	ParseShareaza(LPCTSTR pszURL);
	BOOL	ParseShareazaHost(LPCTSTR pszURL, BOOL bBrowse = FALSE);
	BOOL	ParseShareazaFile(LPCTSTR pszURL);
	BOOL	ParseDonkey(LPCTSTR pszURL);
	BOOL	ParseDonkeyFile(LPCTSTR pszURL);
	BOOL	ParseDonkeyServer(LPCTSTR pszURL);
	BOOL	ParsePiolet(LPCTSTR pszURL);
	BOOL	ParsePioletFile(LPCTSTR pszURL);
	BOOL	ParseDiscovery(LPCTSTR pszURL, int nType);
protected:
	void	SkipSlashes(LPCTSTR& pszURL, int nAdd = 0);
	void	SafeString(CString& strInput);

// Registration Operations
public:
	static void	Register();
	static BOOL	RegisterShellType(LPCTSTR pszProtocol, LPCTSTR pszName, LPCTSTR pszType, LPCTSTR pszApplication, LPCTSTR pszTopic, UINT nIDIcon, BOOL bOverwrite = TRUE);
	static BOOL	RegisterMagnetHandler(LPCTSTR pszID, LPCTSTR pszName, LPCTSTR pszDescription, LPCTSTR pszApplication, UINT nIDIcon);
	static BOOL	IsRegistered(LPCTSTR pszProtocol);
	static BOOL	UnregisterShellType(LPCTSTR pszProtocol);
	static void DeleteKey(HKEY hParent, LPCTSTR pszKey);

};

#endif // !defined(AFX_SHAREAZAURL_H__B39B7816_FE18_4843_A10A_C0DB32D96E52__INCLUDED_)
