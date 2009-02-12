//
// PageComment.h : header file
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#if !defined(AFX_PAGECOMMENT_H__F1FF05AD_7932_427B_BBF3_6809860BEF0A__INCLUDED_)
#define AFX_PAGECOMMENT_H__F1FF05AD_7932_427B_BBF3_6809860BEF0A__INCLUDED_

#pragma once

#include "WizardSheet.h"


class CCommentPage : public CWizardPage
{
// Construction
public:
	CCommentPage();
	virtual ~CCommentPage();

	DECLARE_DYNCREATE(CCommentPage)

// Dialog Data
public:
	//{{AFX_DATA(CCommentPage)
	enum { IDD = IDD_COMMENT_PAGE };
	CString	m_sComment;
	//}}AFX_DATA


// Overrides
public:
	//{{AFX_VIRTUAL(CCommentPage)
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
	//{{AFX_MSG(CCommentPage)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGECOMMENT_H__F1FF05AD_7932_427B_BBF3_6809860BEF0A__INCLUDED_)
