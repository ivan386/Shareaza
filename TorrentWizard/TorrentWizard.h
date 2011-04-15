//
// TorrentWizard.h
//
// Copyright (c) Shareaza Development Team, 2007-2011.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#include "Resource.h"
#include "WizardSheet.h"


class CTorrentWizardApp : public CWinApp
{
public:
	CTorrentWizardApp();

	CString		m_sPath;
	CString		m_sVersion;
	WORD		m_nVersion[4];
	CFont		m_fntNormal;
	CFont		m_fntBold;
	CFont		m_fntLine;
	CFont		m_fntHeader;
	CFont		m_fntTiny;

	CString		m_sCommandLineSourceFile;
	CString		m_sCommandLineDestination;
	CString		m_sCommandLineTracker;
	CString		m_sCommandLineComment;
	
protected:
	CWizardSheet*	m_pSheet;

	void		InitEnvironment();
	void		InitResources();
	
	virtual BOOL InitInstance();

	afx_msg void OnHelp();

	DECLARE_MESSAGE_MAP()
};

extern CTorrentWizardApp theApp;

CString SmartSize(QWORD nVolume);
