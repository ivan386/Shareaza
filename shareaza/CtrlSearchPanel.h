//
// CtrlSearchPanel.h
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

#if !defined(AFX_CTRLSEARCHPANEL_H__EAFFA7F3_526D_45C3_8C17_17A265ED3240__INCLUDED_)
#define AFX_CTRLSEARCHPANEL_H__EAFFA7F3_526D_45C3_8C17_17A265ED3240__INCLUDED_

#pragma once

#include "CtrlTaskPanel.h"
#include "CtrlSchemaCombo.h"
#include "CtrlNetworkCombo.h"
#include "CtrlSchema.h"
#include "CtrlIconButton.h"

class CManagedSearch;
class CQuerySearch;


class CSearchInputBox : public CTaskBox
{
// Construction
public:
	CSearchInputBox();
	virtual ~CSearchInputBox();
	DECLARE_DYNAMIC(CSearchInputBox)

// Attributes
public:
	CEdit			m_wndSearch;
	CSchemaCombo	m_wndSchemas;
	CIconButtonCtrl	m_wndStart;
	CIconButtonCtrl	m_wndStop;
	
// Operations
public:
	void	OnSkinChange();

// Overrides
public:
	//{{AFX_VIRTUAL(CSearchInputBox)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CSearchInputBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSelChangeSchemas();
	afx_msg void OnCloseUpSchemas();
	afx_msg void OnSearchStart();
	afx_msg void OnSearchStop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CSearchAdvancedBox : public CTaskBox
{
// Construction
public:
	CSearchAdvancedBox();
	virtual ~CSearchAdvancedBox();
	DECLARE_DYNAMIC(CSearchAdvancedBox)

// Attributes
public:

	CComboBox		m_wndSizeMin;
	CComboBox		m_wndSizeMax;
	CNetworkCombo	m_wndNetworks;
	
// Operations
public:
	void	OnSkinChange();

// Overrides
public:
	//{{AFX_VIRTUAL(CSearchAdvancedBox)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CSearchAdvancedBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CSearchSchemaBox : public CTaskBox
{
// Construction
public:
	CSearchSchemaBox();
	virtual ~CSearchSchemaBox();
	DECLARE_DYNAMIC(CSearchSchemaBox)
	
// Attributes
public:
	CSchemaCtrl		m_wndSchema;

// Operations
public:

// Overrides
public:
	//{{AFX_VIRTUAL(CSearchSchemaBox)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CSearchSchemaBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


class CSearchResultsBox : public CTaskBox
{
// Construction
public:
	CSearchResultsBox();
	virtual ~CSearchResultsBox();
	DECLARE_DYNAMIC(CSearchResultsBox)

// Attributes
public:
	BOOL	m_bActive;
	DWORD	m_nFiles;
	DWORD	m_nHits;
	DWORD	m_nHubs;
	DWORD	m_nLeaves;

// Operations
public:
	void	Update(BOOL bSearching, DWORD nFiles, DWORD nHits, DWORD nHubs, DWORD nLeaves);
protected:
	static void DrawText(CDC* pDC, int nX, int nY, UINT nFlags, LPCTSTR pszText);

	virtual void OnExpanded(BOOL bOpen);

// Overrides
public:
	//{{AFX_VIRTUAL(CSearchResultsBox)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CSearchResultsBox)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


class CSearchPanel : public CTaskPanel
{
// Construction
public:
	CSearchPanel();
	virtual ~CSearchPanel();
	DECLARE_DYNAMIC(CSearchPanel)

// Attributes
public:
	BOOL				m_bSendSearch;
protected:
	CSearchInputBox		m_boxSearch;
	CSearchAdvancedBox	m_boxAdvanced;
	CSearchSchemaBox	m_boxSchema;
	CSearchResultsBox	m_boxResults;
	BOOL				m_bAdvanced;

// Operations
public:
	void			SetSearchFocus();
	void			ShowSearch(CManagedSearch* pSearch);
	void			ShowStatus(BOOL bStarted, BOOL bSearching, DWORD nFiles, DWORD nHits, DWORD nHubs, DWORD nLeaves);
	void			OnSchemaChange();
	void			ExecuteSearch();
	CManagedSearch*	GetSearch();
	void			OnSkinChange();
	void			Disable();
	void			Enable();
	
// Overrides
public:
	//{{AFX_VIRTUAL(CSearchPanel)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CSearchPanel)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#define IDC_SEARCH_PANEL	111
#define IDC_SEARCH_START	105
#define IDC_SEARCH_STOP		106
#define IDC_SEARCH_NETWORKS	107
#define IDC_SEARCH_SIZEMIN	108
#define IDC_SEARCH_SIZEMAX	109

#endif // !defined(AFX_CTRLSEARCHPANEL_H__EAFFA7F3_526D_45C3_8C17_17A265ED3240__INCLUDED_)
