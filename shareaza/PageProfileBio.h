//
// PageProfileBio.h
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

#if !defined(AFX_PAGEPROFILEBIO_H__98D89C38_9799_42D2_969E_F78FCDC7A167__INCLUDED_)
#define AFX_PAGEPROFILEBIO_H__98D89C38_9799_42D2_969E_F78FCDC7A167__INCLUDED_

#pragma once

#include "WndSettingsPage.h"


class CBioProfilePage : public CSettingsPage
{
// Construction
public:
	CBioProfilePage();
	virtual ~CBioProfilePage();

	DECLARE_DYNCREATE(CBioProfilePage)

// Dialog Data
public:
	//{{AFX_DATA(CBioProfilePage)
	enum { IDD = IDD_PROFILE_BIO };
	CEdit	m_wndText;
	//}}AFX_DATA

// Overrides
public:
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CBioProfilePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CBioProfilePage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGEPROFILEBIO_H__98D89C38_9799_42D2_969E_F78FCDC7A167__INCLUDED_)
