//
// PageCommandline.h
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

#if !defined(AFX_PAGECOMMANDLINE__INCLUDED_)
#define AFX_PAGECOMMANDLINE__INCLUDED_

#pragma once

#include "WizardSheet.h"

class CTorrentBuilder;


class CCommandlinePage : public CWizardPage
{
// Construction
public:
	CCommandlinePage();
	virtual ~CCommandlinePage();

	DECLARE_DYNCREATE(CCommandlinePage)

// Dialog Data
public:
	//{{AFX_DATA(CCommandlinePage)
	enum { IDD = IDD_COMMANDLINE_PAGE };
	CButton	m_wndAbort;
	CEdit	m_wndTorrentName;
	CStatic	m_wndSpeedMessage;
	CSliderCtrl	m_wndSpeed;
	CStatic	m_wndSpeedSlow;
	CStatic	m_wndSpeedFast;
	CProgressCtrl	m_wndProgress;
	CStatic	m_wndFileName;
	CStatic	m_wndDone2;
	CStatic	m_wndDone1;
	//}}AFX_DATA

// Attributes
protected:
	CTorrentBuilder*	m_pBuilder;
	CString				m_sDestinationFile;

// Operations
protected:
	void		Start();

// Overrides
public:
	//{{AFX_VIRTUAL(CCommandlinePage)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CCommandlinePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnAbort();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnStnClickedDone2();
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGECOMMANDLINE__INCLUDED_)
