//
// CtrlSchema.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
// This file is part of SHAREAZA (www.shareaza.com)
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

#if !defined(AFX_CTRLSCHEMA_H__98487F51_E425_4B0D_BC82_BCA8A7F5D952__INCLUDED_)
#define AFX_CTRLSCHEMA_H__98487F51_E425_4B0D_BC82_BCA8A7F5D952__INCLUDED_

#pragma once

class CSchema;
class CXMLElement;


class CSchemaCtrl : public CWnd
{
// Construction
public:
	CSchemaCtrl();
	virtual ~CSchemaCtrl();

// Attributes
public:
	int				m_nCaptionWidth;
	int				m_nItemHeight;
	BOOL			m_bShowBorder;
protected:
	CSchema*		m_pSchema;
	CObArray		m_pControls;
	CStringArray	m_pCaptions;
	int				m_nScroll;
	CString			strMultipleString;

// Operations
public:
	void		SetSchema(CSchema* pSchema, BOOL bPromptOnly = FALSE);
	BOOL		UpdateData(CXMLElement* pBase, BOOL bSaveAndValidate);
	void		Disable();
	void		Enable();
	BOOL		OnTab();
	void		ScrollBy(int nDelta);
protected:
	void		Layout();
	void		SetFocusTo(CWnd* pCtrl);

// Overrides
public:
	//{{AFX_VIRTUAL(CSchemaCtrl)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CSchemaCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	afx_msg void OnControlEdit();

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#define IDC_METADATA_CONTROL	99

#endif // !defined(AFX_CTRLSCHEMA_H__98487F51_E425_4B0D_BC82_BCA8A7F5D952__INCLUDED_)
