//
// VersionChecker.h
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

#if !defined(AFX_VERSIONCHECKER_H__EB5F6233_9917_44C3_BE67_CACB43D929CA__INCLUDED_)
#define AFX_VERSIONCHECKER_H__EB5F6233_9917_44C3_BE67_CACB43D929CA__INCLUDED_

#pragma once

#include "HttpRequest.h"


class CVersionChecker  
{
// Construction
public:
	CVersionChecker();
	virtual ~CVersionChecker();
	
// Attributes
public:
	CString		m_sMessage;
	CString		m_sQuote;
	BOOL		m_bUpgrade;
	CString		m_sUpgradePrompt;
	CString		m_sUpgradeFile;
	CString		m_sUpgradeHash;
	CString		m_sUpgradeSources;
	CString		m_sUpgradePath;
protected:
	HANDLE				m_hThread;
	CHttpRequest		m_pRequest;
	CMapStringToString	m_pResponse;
	HWND				m_hWndNotify;

// Operations
public:
	BOOL		NeedToCheck();
	BOOL		Start(HWND hWndNotify);
	void		Stop();
	void		SetNextCheck(int nDays);
	BOOL		CheckUpgradeHash(const SHA1* pHash, LPCTSTR pszPath);
protected:
	static UINT	ThreadStart(LPVOID pParam);
	void		OnRun();
	void		BuildRequest(CString& strRequest);
	BOOL		UndertakeRequest(CString& strPost);
	void		ProcessResponse();

};

extern CVersionChecker VersionChecker;

#endif // !defined(AFX_VERSIONCHECKER_H__EB5F6233_9917_44C3_BE67_CACB43D929CA__INCLUDED_)
