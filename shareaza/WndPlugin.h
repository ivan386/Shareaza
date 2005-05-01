//
// WndPlugin.h
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

#if !defined(AFX_WNDPLUGIN_H__ED4EF23F_A534_4A1C_B657_CE776DE55F93__INCLUDED_)
#define AFX_WNDPLUGIN_H__ED4EF23F_A534_4A1C_B657_CE776DE55F93__INCLUDED_

#pragma once

#include "WndPanel.h"


class CPluginWnd : public CPanelWnd
{
// Construction
public:
	CPluginWnd(LPCTSTR pszName = NULL, IPluginWindowOwner* pOwner = NULL);
	virtual ~CPluginWnd();

	DECLARE_DYNAMIC(CPluginWnd)

// Attributes
public:
	IPluginWindowOwner*	m_pOwner;
	CString				m_sName;
	UINT*				m_pHandled;
	DWORD				m_nHandled;
	CCoolBarCtrl*		m_pToolbar;
	int					m_nToolbar;
	BOOL				m_bAccel;

// Operations
public:
	virtual void	OnSkinChange();
	virtual HRESULT	GetGenericView(IGenericView** ppView);

// Overrides
public:
	//{{AFX_VIRTUAL(CPluginWnd)
	public:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CPluginWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// IPluginWindow
protected:
	BEGIN_INTERFACE_PART(PluginWindow, IPluginWindow)
		STDMETHOD(ListenForSingleMessage)(UINT nMessage);
		STDMETHOD(ListenForMultipleMessages)(SAFEARRAY FAR* pMessages);
		STDMETHOD(Create1)(BSTR bsCaption, HICON hIcon, VARIANT_BOOL bPanel, VARIANT_BOOL bTabbed);
		STDMETHOD(Create2)(UINT nCommandID, VARIANT_BOOL bPanel, VARIANT_BOOL bTabbed);
		STDMETHOD(GetHwnd)(HWND FAR* phWnd);
		STDMETHOD(HandleMessage)(LRESULT* plResult);
		STDMETHOD(LoadState)(VARIANT_BOOL bMaximise);
		STDMETHOD(SaveState)();
		STDMETHOD(ThrowMenu)(BSTR sName, LONG nDefaultID, POINT FAR* pPoint);
		STDMETHOD(AddToolbar)(BSTR sName, LONG nPosition, HWND FAR* phWnd, ISToolbar FAR* FAR* ppToolbar);
		STDMETHOD(AdjustWindowRect)(RECT FAR* pRect, VARIANT_BOOL bClientToWindow);
	END_INTERFACE_PART(PluginWindow)

	DECLARE_INTERFACE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WNDPLUGIN_H__ED4EF23F_A534_4A1C_B657_CE776DE55F93__INCLUDED_)
