//
// PageTracker.h
//
// Copyright (c) Shareaza Pty. Ltd., 2003.
// This file is part of TorrentAid Torrent Wizard (www.torrentaid.com).
//
// TorrentAid Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// TorrentAid is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TorrentAid; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#if !defined(AFX_PAGETRACKER_H__98B08251_6D7C_4028_943D_5E483220F28D__INCLUDED_)
#define AFX_PAGETRACKER_H__98B08251_6D7C_4028_943D_5E483220F28D__INCLUDED_

#pragma once

#include "WizardSheet.h"


class CTrackerPage : public CWizardPage
{
// Construction
public:
	CTrackerPage();
	virtual ~CTrackerPage();

	DECLARE_DYNCREATE(CTrackerPage)

// Dialog Data
public:
	//{{AFX_DATA(CTrackerPage)
	enum { IDD = IDD_TRACKER_PAGE };
	CComboBox	m_wndTracker;
	CString	m_sTracker;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CTrackerPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CTrackerPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnClearTrackers();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGETRACKER_H__98B08251_6D7C_4028_943D_5E483220F28D__INCLUDED_)
