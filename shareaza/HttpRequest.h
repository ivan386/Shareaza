//
// HttpRequest.h
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

class CBuffer;


class CHttpRequest
{
// Constructions
public:
	CHttpRequest();
	virtual ~CHttpRequest();
	
// Operations
public:
	void		Clear();
public:
	CString		GetURL();
	BOOL		SetURL(LPCTSTR pszURL);
	void		SetUserAgent(LPCTSTR pszUserAgent);
	void		AddHeader(LPCTSTR pszKey, LPCTSTR pszValue);
	void		SetPostData(LPCVOID pBody, DWORD nBody);
	void		LimitContentLength(DWORD nLimit);
	void		SetNotify(HWND hWnd, UINT nMsg, WPARAM wParam = 0);
public:
	int			GetStatusCode();
	BOOL		GetStatusSuccess();
	CString		GetStatusString();
	CString		GetHeader(LPCTSTR pszName);
	CString		GetResponseString(UINT nCodePage = CP_UTF8);
	CBuffer*	GetResponseBuffer();
	BOOL		InflateResponse();
public:
	BOOL		Execute(BOOL bBackground);
	BOOL		IsPending();
	BOOL		IsFinished();
	void		Cancel();

// Data
protected:
	CCriticalSection	m_pSection;
	HANDLE				m_hThread;
	HINTERNET			m_hInternet;
	BOOL				m_bCancel;
protected:
	CString				m_sURL;
	CString				m_sUserAgent;
	CString				m_sRequestHeaders;
	DWORD				m_nLimit;
	int					m_nStatusCode;
	CString				m_sStatusString;
	CBuffer*			m_pPost;
	CBuffer*			m_pResponse;
	CMapStringToString m_pResponseHeaders;
protected:
	HWND				m_hNotifyWnd;
	UINT				m_nNotifyMsg;
	WPARAM				m_nNotifyParam;

// Implementation
protected:
	static UINT ThreadStart(LPVOID lpParameter);
protected:
	int		Run();
	void	RunRequest();
	void	RunResponse(HINTERNET hURL);

// Utilities
public:
	static void CloseThread(HANDLE* phThread, LPCTSTR pszName);
};
