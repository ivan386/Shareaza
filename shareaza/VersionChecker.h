//
// VersionChecker.h
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

#include "HttpRequest.h"

class CLibraryFile;


class CVersionChecker : public CThreadImpl
{
// Construction
public:
	CVersionChecker();
	virtual ~CVersionChecker();
	
// Attributes
public:
	CString		m_sMessage;
	CString		m_sUpgradePath;

protected:
	bool			m_bVerbose;
	CHttpRequest	m_pRequest;
	CStringIMap		m_pResponse;

// Operations
public:
	BOOL		Start();
	void		Stop();
	static void ClearVersionCheck();
	void		ForceCheck();
	void		SetNextCheck(int nDays);
    BOOL		CheckUpgradeHash(const CLibraryFile* pFile = NULL);

	// Test if available version is newer than current
	static BOOL	IsVersionNewer();

	inline bool	IsUpgradeAvailable() const throw()
	{
		return ! Settings.VersionCheck.UpgradePrompt.IsEmpty();
	}

	inline bool IsVerbose() const throw()
	{
		return m_bVerbose;
	}

protected:
	BOOL		NeedToCheck();
	void		OnRun();
	BOOL		ExecuteRequest();
	void		ProcessResponse();
};

extern CVersionChecker VersionChecker;

enum VERSION_CHECK	// WM_VERSIONCHECK message wParam argument
{
	VC_MESSAGE_AND_CONFIRM = 0, // Show message and then ask to download new version
	VC_CONFIRM = 1,				// Ask to download new version
	VC_UPGRADE = 2				// Ask then start upgrading of already downloaded installer
};
