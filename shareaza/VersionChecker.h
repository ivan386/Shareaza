//
// VersionChecker.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
	BOOL		m_bUpgrade;
	CString		m_sUpgradePath;

protected:
	HANDLE				m_hThread;
	CHttpRequest		m_pRequest;
	CMap< CString, const CString&, CString, CString& >	m_pResponse;

// Operations
public:
	BOOL		Start();
	void		Stop();
	void		SetNextCheck(int nDays);
    BOOL		CheckUpgradeHash(const Hashes::Sha1Hash& oHash, LPCTSTR pszPath);
	BOOL		CheckUpgradeHash();
protected:
	BOOL		NeedToCheck();
	static UINT	ThreadStart(LPVOID pParam);
	void		OnRun();
	BOOL		ExecuteRequest();
	void		ProcessResponse();
};

extern CVersionChecker VersionChecker;
