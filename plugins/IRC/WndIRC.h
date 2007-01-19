//
// WndIRC.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#pragma once
#include "IRC.h"

class CIRCPlugin;
// CIRCWnd

class ATL_NO_VTABLE CIRCWnd : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CWindow, public CMDIChildWinTraits, public IPluginWindowOwner
{
public:
	// Construction
public:
	CIRCWnd();
	virtual ~CIRCWnd();

	// Attributes
public:
	CIRCPlugin*				m_pPlugin;
	CComPtr<IApplication>	m_pApplication;
	CComPtr<IPluginWindow>	m_pWindow;

	BOOL Create(CIRCPlugin* pPlugin);
	BOOL Refresh();
	BOOL ResizeWindow();

	DECLARE_NOT_AGGREGATABLE(CIRCWnd)

	BEGIN_COM_MAP(CIRCWnd)
		COM_INTERFACE_ENTRY(IPluginWindowOwner)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
	HRESULT FinalConstruct() { return S_OK; }
	void FinalRelease() {}

	// IPluginWindowOwner Methods
public:
	STDMETHOD(OnTranslate)(MSG* pMessage);
	STDMETHOD(OnMessage)(UINT nMessage, WPARAM wParam, LPARAM  Param, LRESULT* plResult);
	STDMETHOD(OnUpdate)(UINT nCommandID, STRISTATE* pbVisible, STRISTATE* pbEnabled, STRISTATE* pbChecked);
	STDMETHOD(OnCommand)(UINT nCommandID);

	// Message Map
public:
	BEGIN_MSG_MAP(CIRCWindow)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
	END_MSG_MAP()

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};
