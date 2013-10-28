//
// DlgConnectTo.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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


class CConnectToDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CConnectToDlg)

public:
	enum Type { Connect = 0, Browse = 1, Chat = 2 };

	CConnectToDlg(CWnd* pParent = NULL, Type nType = Connect);

	enum { IDD = IDD_CONNECT_TO };
	CButton		m_wndAdvanced;
	CComboBox	m_wndProtocol;
	CButton		m_wndUltrapeer;
	CButton		m_wndPrompt;
	CEdit		m_wndPort;
	CComboBox	m_wndHost;
	CString		m_sHost;
	BOOL		m_bNoUltraPeer;
	int			m_nPort;
	PROTOCOLID	m_nProtocol;

protected:
	CImageList	m_gdiProtocols;
	Type		m_nType;

	void		LoadItem(int nItem);
	BOOL		UpdateItems();
	void		SaveItems();
	BOOL		UpdateData(BOOL bSaveAndValidate = TRUE);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnCbnSelchangeConnectHost();
	afx_msg void OnCbnSelchangeConnectProtocol();
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};
