//
// DlgScheduleItem.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#if !defined(AFX_DLGSCHEDULEITEM_H__INCLUDED_)
#define AFX_DLGSCHEDULEITEM_H__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"
#include "afxdtctl.h"
#include "afxwin.h"

class CScheduleItem;

//////////////////////////////////////////////////////////////////////
// CScheduleItemDlg class: Schedule Item Dialog
//////////////////////////////////////////////////////////////////////

class CScheduleItemDlg : public CSkinDialog
{
// Construction
public:
	CScheduleItemDlg(CWnd* pParent = NULL, CScheduleItem* pSchItem = NULL);
	virtual ~CScheduleItemDlg();

// Dialog Data
public:
	//{{AFX_DATA(CScheduleItemDlg)
	enum { IDD = IDD_SCHEDULE_ITEM };

	//}}AFX_DATA

	CScheduleItem		*m_pScheduleItem;
	BOOL				m_bEveryDay;
	int					m_nAction;
	CString				m_sDescription;
	CTime				m_tDateAndTime;
	BOOL				m_bActive;
	BOOL				m_bNew;
	BOOL				m_bToggleBandwidth;
	BOOL				m_bLimitedNetworks;
	int					m_nLimited;
	int					m_nLimitedDown;
	int					m_nLimitedUp;

// Overrides
public:
	//{{AFX_VIRTUAL(CScheduleItemDlg)

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CScheduleItemDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedOnlyonce();
	afx_msg void OnBnClickedToggleBandwidth();
	afx_msg void OnDtnDatetimechangeDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeTime(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedEveryday();
	afx_msg void OnBnClickedActive();
	afx_msg void OnCbnSelchangeEventtype();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CEdit				m_pwndLimitedEdit;
	CComboBox			m_pwndTypeSel;
	CEdit				m_pwndLimitedEditDown;
	CEdit				m_pwndLimitedEditUp;
	CButton				m_pwndLimitedCheck;
	CButton				m_pwndLimitedCheckTgl;
	CStatic				m_pwndLimitedStatic;
	CStatic				m_pwndLimitedStaticDown;
	CStatic				m_pwndLimitedStaticUp;
	CButton				m_pwndActiveCheck;
	CDateTimeCtrl		m_wndDate;
	CDateTimeCtrl		m_wndTime;
	CSpinButtonCtrl		m_wndSpin;
	CSpinButtonCtrl		m_wndSpinDown;
	CSpinButtonCtrl		m_wndSpinUp;
	CButton				m_pwndRadioOnce;
	CButton				m_pwndRadioEveryDay;
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGSCHEDULEITEM_H__INCLUDED_)
