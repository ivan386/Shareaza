//
// Remote.h
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

#pragma once

#include "Buffer.h"
#include "Transfer.h"

class CMatchFile;


class CRemote : public CTransfer
{
// Construction
public:
	CRemote(CConnection* pConnection);
	~CRemote();
	
// Attributes
protected:
	CString				m_sHandshake;
	CString				m_sRedirect;
	CString				m_sHeader;
	CString				m_sResponse;
	CBuffer				m_pResponse;
	CMapStringToString	m_pKeys;
	static CList<int>	m_pCookies;

// Operations
public:
	virtual BOOL	OnRun();
	virtual void	OnDropped(BOOL bError);
	virtual BOOL	OnRead();
	virtual BOOL	OnHeadersComplete();
protected:
	CString			GetKey(LPCTSTR pszName);
	BOOL			CheckCookie();
	void			Prepare(LPCTSTR pszPrefix = NULL);
	void			Add(LPCTSTR pszKey, LPCTSTR pszValue);
	void			Output(LPCTSTR pszName);
	
// Page Handlers
protected:
	void	PageSwitch(CString& strPath);
	void	PageLogin();
	void	PageHome();
	void	PageSearch();
	void	PageNewSearch();
	void	PageDownloads();
	void	PageNewDownload();
	void	PageUploads();
	void	PageNetwork();
	void	PageBanner(CString& strPath);
	void	PageImage(CString& strPath);

// Utilities
protected:
	void	PageSearchHeaderColumn(int nColumnID, LPCTSTR pszCaption, LPCTSTR pszAlign);
	void	PageSearchRowColumn(int nColumnID, CMatchFile* pFile, LPCTSTR pszValue, LPCTSTR pszAlign = _T("center"));
	void	PageNetworkNetwork(int nID, BOOL* pbConnect, LPCTSTR pszName);
};
