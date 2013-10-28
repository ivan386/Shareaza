//
// DlgQueueProperties.h
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

#if !defined(AFX_DLGQUEUEPROPERTIES_H__930ED48C_23D4_4BA3_8F4C_66B115DE5083__INCLUDED_)
#define AFX_DLGQUEUEPROPERTIES_H__930ED48C_23D4_4BA3_8F4C_66B115DE5083__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"

class CUploadQueue;


class CQueuePropertiesDlg : public CSkinDialog
{
// Construction
public:
	CQueuePropertiesDlg(CUploadQueue* pQueue, BOOL bEnable, CWnd* pParent = NULL);

// Dialog Data
public:
	//{{AFX_DATA(CQueuePropertiesDlg)
	enum { IDD = IDD_QUEUE_PROPERTIES };
	CButton m_wndPartialOnly;
	CButton m_wndLibraryOnly;
	CButton m_wndBoth;
	CEdit	m_wndMatch;
	CEdit	m_wndBandwidthPoints;
	CEdit	m_wndBandwidthValue;
	CSpinButtonCtrl	m_wndTransfersMin;
	CSpinButtonCtrl	m_wndTransfersMax;
	CSpinButtonCtrl	m_wndRotateTimeSpin;
	CEdit	m_wndRotateTime;
	CListCtrl	m_wndProtocols;
	CEdit	m_wndMinSize;
	CEdit	m_wndMaxSize;
	CComboBox	m_wndMarked;
	CSpinButtonCtrl	m_wndCapacity;
	CSliderCtrl	m_wndBandwidthSlider;
	INT_PTR	m_nCapacity;
	BOOL	m_bMaxSize;
	CString	m_sMaxSize;
	BOOL	m_bMinSize;
	CString	m_sMinSize;
	BOOL	m_bMarked;
	CString	m_sName;
	DWORD	m_nFileStatusFlag;
	BOOL	m_bProtocols;
	BOOL	m_bRotate;
	BOOL	m_bReward;
	int		m_nRotateTime;
	INT_PTR	m_nTransfersMax;
	INT_PTR	m_nTransfersMin;
	BOOL	m_bMatch;
	CString	m_sMatch;
	BOOL	m_bEnable;
	CString	m_sMarked;
	//}}AFX_DATA

	CImageList		m_gdiProtocols;
	CUploadQueue*	m_pQueue;
	BOOL			m_bEnableOverride;

// Overrides
public:
	//{{AFX_VIRTUAL(CQueuePropertiesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CQueuePropertiesDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnMinimumCheck();
	afx_msg void OnMaximumCheck();
	afx_msg void OnProtocolsCheck();
	afx_msg void OnMarkedCheck();
	afx_msg void OnRotateEnable();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChangeTransfersMax();
	afx_msg void OnMatchCheck();
	afx_msg void OnPartialClicked();
	afx_msg void OnLibraryClicked();
	afx_msg void OnBothClicked();
	//}}AFX_MSG

	// Get the protcol checkbox
	CButton* GetProtocolCheckbox()
	{
		return ((CButton*)GetDlgItem(IDC_PROTOCOLS_CHECK));
	}

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGQUEUEPROPERTIES_H__930ED48C_23D4_4BA3_8F4C_66B115DE5083__INCLUDED_)
