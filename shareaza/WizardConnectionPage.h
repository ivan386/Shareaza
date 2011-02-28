//
// WizardConnectionPage.h
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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

#pragma once

#include "ThreadImpl.h"
#include "WizardSheet.h"


class CWizardConnectionPage :
	public CWizardPage,
	public CThreadImpl
{
	DECLARE_DYNCREATE(CWizardConnectionPage)

public:
	CWizardConnectionPage();
	virtual ~CWizardConnectionPage();

	enum { IDD = IDD_WIZARD_CONNECTION };

protected:
	CComboBox		m_wndType;
	CComboBox		m_wndDownloadSpeed;
	CComboBox		m_wndUploadSpeed;
	CComboBox		m_wndUPnP;
	CProgressCtrl	m_wndProgress;
	CStatic			m_wndStatus;
	bool			m_bQueryDiscoveries;
	bool			m_bUpdateDonkeyServers;
	short			m_nProgressSteps;

	void OnRun();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnQueryCancel();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnSelChangeConnectionType();
	afx_msg void OnChangeConnectionSpeed();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
