//
// PageFinished.h
//
// Copyright (c) Shareaza Development Team, 2007.
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

#if !defined(AFX_PAGEFINISHED_H__1EDBB13B_5444_4D4B_8EC1_6971C13070A8__INCLUDED_)
#define AFX_PAGEFINISHED_H__1EDBB13B_5444_4D4B_8EC1_6971C13070A8__INCLUDED_

#pragma once

#include "WizardSheet.h"

class CTorrentBuilder;


class CFinishedPage : public CWizardPage
{
// Construction
public:
	CFinishedPage();
	virtual ~CFinishedPage();

	DECLARE_DYNCREATE(CFinishedPage)

// Dialog Data
public:
	//{{AFX_DATA(CFinishedPage)
	enum { IDD = IDD_FINISHED_PAGE };
	CButton	m_wndAbort;
	CEdit	m_wndTorrentName;
	CButton	m_wndTorrentCopy;
	CButton	m_wndTorrentOpen;
	CButton	m_wndTorrentSeed;
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

// Operations
protected:
	void		Start();

// Overrides
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnAbort();
	afx_msg void OnTorrentCopy();
	afx_msg void OnTorrentOpen();
	afx_msg void OnTorrentSeed();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGEFINISHED_H__1EDBB13B_5444_4D4B_8EC1_6971C13070A8__INCLUDED_)
