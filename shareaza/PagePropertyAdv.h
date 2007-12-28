//
// PagePropertyAdv.h
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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

// CPropertyPageAdv dialog

class CPropertyPageAdv : public CPropertyPage
{
	DECLARE_DYNAMIC(CPropertyPageAdv)

public:
	CPropertyPageAdv(UINT nIDD);
	virtual ~CPropertyPageAdv();

// Members
protected:
	CToolTipCtrl m_wndToolTip;

public:
	int	m_nIcon;

// Implementation
private:
	void	PaintStaticHeader(CDC* pDC, CRect* prc, LPCTSTR psz);

protected:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
