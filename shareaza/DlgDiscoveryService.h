//
// DlgDiscoveryService.h
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

#if !defined(AFX_DLGDISCOVERYSERVICE_H__3039DCB4_8EAA_442D_B893_139D1D44E277__INCLUDED_)
#define AFX_DLGDISCOVERYSERVICE_H__3039DCB4_8EAA_442D_B893_139D1D44E277__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"

class CDiscoveryService;


class CDiscoveryServiceDlg : public CSkinDialog
{
// Construction
public:
	CDiscoveryServiceDlg(CWnd* pParent = NULL, CDiscoveryService* pService = NULL);
	virtual ~CDiscoveryServiceDlg();

// Dialog Data
public:
	//{{AFX_DATA(CDiscoveryServiceDlg)
	enum { IDD = IDD_DISCOVERY_SERVICE };
	CButton	m_wndOK;
	CString	m_sAddress;
	int		m_nType;
	//}}AFX_DATA

	CDiscoveryService* m_pService;
	BOOL m_bNew;

// Overrides
public:
	//{{AFX_VIRTUAL(CDiscoveryServiceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CDiscoveryServiceDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeAddress();
	afx_msg void OnSelChangeServiceType();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGDISCOVERYSERVICE_H__3039DCB4_8EAA_442D_B893_139D1D44E277__INCLUDED_)
