//
// HttpURL.h
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

#if !defined(AFX_HTTPURL_H__E247E342_B4B7_4E3C_84E4_B69BBFF0BC2E__INCLUDED_)
#define AFX_HTTPURL_H__E247E342_B4B7_4E3C_84E4_B69BBFF0BC2E__INCLUDED_

#pragma once


class CSourceURL  
{
// Construction
public:
	CSourceURL(LPCTSTR pszURL = NULL);
	
// Attributes
public:
	CString		m_sURL;
	PROTOCOLID	m_nProtocol;
public:
	CString		m_sAddress;
	IN_ADDR		m_pAddress;
	WORD		m_nPort;
	IN_ADDR		m_pServerAddress;
	WORD		m_nServerPort;
	CString		m_sPath;
public:
	BOOL		m_bSHA1;
	SHA1		m_pSHA1;
	BOOL		m_bED2K;
	MD4			m_pED2K;
	BOOL		m_bBTH;
	SHA1		m_pBTH;
	BOOL		m_bBTC;
	SHA1		m_pBTC;
	BOOL		m_bSize;
	QWORD		m_nSize;
	
// Operations
public:
	void	Clear();
	BOOL	Parse(LPCTSTR pszURL, BOOL bResolve = TRUE);
	BOOL	ParseHTTP(LPCTSTR pszURL, BOOL bResolve = TRUE);
	BOOL	ParseFTP(LPCTSTR pszURL, BOOL bResolve = TRUE);
	BOOL	ParseED2KFTP(LPCTSTR pszURL, BOOL bResolve = TRUE);
	BOOL	ParseBTC(LPCTSTR pszURL, BOOL bResolve = TRUE);

};

#endif // !defined(AFX_HTTPURL_H__E247E342_B4B7_4E3C_84E4_B69BBFF0BC2E__INCLUDED_)
