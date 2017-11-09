//
// DlgProgressBar.h
//
// Copyright (c) Shareaza Development Team, 2009.
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

#include "DlgSkinDialog.h"


class CProgressBarDlg : public CSkinDialog
{
// Construction
public:
	CProgressBarDlg(CWnd* pParent = NULL);
	virtual ~CProgressBarDlg();

	DECLARE_DYNAMIC(CProgressBarDlg)
	enum { IDD = IDD_PROGRESSBAR };

// Attributes
private:
	CString			m_sAction;
	CString			m_sEvent;
	CProgressCtrl	m_oEventProgress;
	CString			m_sSubAction;
	CString			m_sSubEvent;
	CProgressCtrl	m_oSubEventProgress;

// Operations
public:
	void	SetActionText(const CString& strText);
	void	SetEventText(const CString& strText);
	void	SetEventRange(int nLower, int nUpper);
	void	SetEventPos(int nPos);
	void	SetEventStep(int nStep);
	void	SetSubActionText(const CString& strText);
	void	SetSubEventText(const CString& strText);
	void	SetSubEventRange(int nLower, int nUpper);
	void	SetSubEventPos(int nPos);
	void	SetSubEventStep(int nStep);
	void	StepEvent();
	void	StepEvent(int nPos);
	void	StepSubEvent();
	void	StepSubEvent(int nPos);

// Overrides
protected:
	virtual void	DoDataExchange(CDataExchange* pDX);		// DDX/DDV support
	virtual BOOL	OnInitDialog();
	virtual void	OnCancel();
	virtual void	OnOK();

	DECLARE_MESSAGE_MAP()
};
