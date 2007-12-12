//
// PageSingle.h
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

#if !defined(AFX_PAGESINGLE_H__48CC1296_B18B_489B_B046_389F70A940B5__INCLUDED_)
#define AFX_PAGESINGLE_H__48CC1296_B18B_489B_B046_389F70A940B5__INCLUDED_

#pragma once

#include "WizardSheet.h"


class CSinglePage : public CWizardPage
{
// Construction
public:
	CSinglePage();
	virtual ~CSinglePage();

	DECLARE_DYNCREATE(CSinglePage)
	
// Dialog Data
public:
	//{{AFX_DATA(CSinglePage)
	enum { IDD = IDD_SINGLE_PAGE };
	CString	m_sFileName;
	CString	m_sFileSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSinglePage)
	public:
	virtual void OnReset();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSinglePage)
	afx_msg void OnBrowseFile();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGESINGLE_H__48CC1296_B18B_489B_B046_389F70A940B5__INCLUDED_)
