//
// PageTracker.h
//
// Copyright (c) Shareaza Development Team, 2007-2014.
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

#include "WizardSheet.h"


class CTrackerPage : public CWizardPage
{
	DECLARE_DYNCREATE(CTrackerPage)

public:
	CTrackerPage();

	enum { IDD = IDD_TRACKER_PAGE };

	CString		m_sTracker;
	BOOL		m_bPrivate;

protected:
	CComboBox	m_wndTracker;

	void SaveTrackers();

	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnClearTrackers();

	DECLARE_MESSAGE_MAP()
};
