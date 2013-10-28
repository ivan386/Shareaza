//
// DlgFolderProperties.h
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

#if !defined(AFX_DLGFOLDERPROPERTIES_H__692DC944_E81F_46C3_80F2_6DF9256B4B6D__INCLUDED_)
#define AFX_DLGFOLDERPROPERTIES_H__692DC944_E81F_46C3_80F2_6DF9256B4B6D__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"
#include "CtrlSchema.h"
#include "CtrlSchemaCombo.h"

class CAlbumFolder;


class CFolderPropertiesDlg : public CSkinDialog
{
// Construction
public:
	CFolderPropertiesDlg(CWnd* pParent = NULL, CAlbumFolder* pFolder = NULL);

	DECLARE_DYNAMIC(CFolderPropertiesDlg)

// Dialog Data
public:
	//{{AFX_DATA(CFolderPropertiesDlg)
	enum { IDD = IDD_FOLDER_PROPERTIES };
	CStatic			m_wndApply;
	CButton			m_wndCancel;
	CButton			m_wndOK;
	CEdit			m_wndTitle;
	CSchemaCombo	m_wndSchemas;
	CStatic			m_wndTitleLabel;
	CStatic			m_wndTypeLabel;
	//}}AFX_DATA

// Attributes
protected:
	CAlbumFolder*	m_pFolder;
	CSchemaCtrl		m_wndData;
	int				m_nWidth;
	BOOL			m_bUpdating;

	void	DoApply(BOOL bMetaToFiles);

// Overrides
public:
	//{{AFX_VIRTUAL(CFolderPropertiesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CFolderPropertiesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelChangeSchemas();
	afx_msg void OnCloseUpSchemas();
	virtual void OnOK();
	afx_msg void OnPaint();
	afx_msg void OnChangeTitle();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnChangeData();
	afx_msg void OnCancel();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#define IDC_METADATA		100


#endif // !defined(AFX_DLGFOLDERPROPERTIES_H__692DC944_E81F_46C3_80F2_6DF9256B4B6D__INCLUDED_)
