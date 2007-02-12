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
#include "BasePluginWindow.h"
#include "CtrlIRCFrame.h"

class CIRCPlugin;

// CIRCWnd

class CIRCWnd : public CBasePluginWindow< CMDIChildWinTraits >
{
public:
	// Construction
public:
	CIRCWnd();
	virtual ~CIRCWnd();

	virtual BOOL Initialize(CIRCPlugin* pPlugin, LPCTSTR pszClassName);
	BOOL Refresh();
	void OnSkinChanged(void);

protected:
	CIRCFrame*	m_pFrame;

	BEGIN_COM_MAP(CIRCWnd)
		COM_INTERFACE_ENTRY(IPluginWindowOwner)
	END_COM_MAP()

	// IPluginWindowOwner Methods
public:
	STDMETHOD(OnTranslate)(MSG* pMessage);
	STDMETHOD(OnMessage)(INT nMessage, WPARAM wParam, LPARAM  Param, LRESULT* plResult);
	STDMETHOD(OnUpdate)(INT nCommandID, STRISTATE* pbVisible, STRISTATE* pbEnabled, STRISTATE* pbChecked);
	STDMETHOD(OnCommand)(INT nCommandID);
	STDMETHOD(GetWndClassName)(BSTR* pszClassName);
	
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
