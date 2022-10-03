//
// HttpRequest.h
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

#include "ThreadImpl.h"

class CBuffer;


class CHttpRequest : public CThreadImpl
{
// Constructions
public:
	CHttpRequest();
	virtual ~CHttpRequest();

// Operations
public:
	void		Clear();
	CString		GetURL() const;
	BOOL		SetURL(LPCTSTR pszURL);
	void		AddHeader(LPCTSTR pszKey, LPCTSTR pszValue);
	void		SetPostData(LPCVOID pBody, DWORD nBody);
	void		LimitContentLength(DWORD nLimit);
	void		SetNotify(HWND hWnd, UINT nMsg, WPARAM wParam = 0);
	int			GetStatusCode() const;
	bool		GetStatusSuccess() const;
	CString		GetStatusString() const;
	CString		GetHeader(LPCTSTR pszName) const;
	CString		GetResponseString(UINT nCodePage = CP_UTF8) const;
	CBuffer*	GetResponseBuffer() const;
	BOOL		InflateResponse();
	bool		Execute(bool bBackground);
	BOOL		IsPending() const;
	BOOL		IsFinished() const;
	void		Cancel();
	void		EnableCookie(bool bEnable);

// Data
protected:
	HINTERNET	m_hInternet;
	CString		m_sURL;
	CString		m_sRequestHeaders;
	DWORD		m_nLimit;
	int			m_nStatusCode;
	CString		m_sStatusString;
//	CBuffer*	m_pPost;
	CBuffer*	m_pResponse;
	CStringIMap	m_pResponseHeaders;
	HWND		m_hNotifyWnd;
	UINT		m_nNotifyMsg;
	WPARAM		m_nNotifyParam;
	bool		m_bUseCookie;

	void		OnRun();

private:
	CHttpRequest(const CHttpRequest&);
	CHttpRequest& operator=(const CHttpRequest&);
};
