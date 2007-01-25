//
// IRCPlugin.h
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

class CIRCWnd;

class ATL_NO_VTABLE CIRCPlugin : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CIRCPlugin, &CLSID_IRCPlugin>,
	public ICommandPlugin,
	public IGeneralPlugin
{
public:
	CIRCPlugin();

	DECLARE_REGISTRY_RESOURCEID(IDR_IRCPLUGIN)
	DECLARE_NOT_AGGREGATABLE(CIRCPlugin)

	BEGIN_COM_MAP(CIRCPlugin)
		COM_INTERFACE_ENTRY(ICommandPlugin)
		COM_INTERFACE_ENTRY(IGeneralPlugin)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct(){ return S_OK; }
	void FinalRelease() {}

	// Attributes
public:
	CComPtr<IApplication>		m_pApplication;
	CComPtr<IUserInterface>		m_pInterface;
	CIRCWnd*					m_pWindow;

public:
	INT	m_nCmdWindow;
	INT	m_nCmdWindow2;
	INT	m_nCmdCloseTab;
	INT	m_nCmdWhois;
	INT	m_nCmdQuery;
	INT	m_nCmdTime;
	INT	m_nCmdVersion;
	INT	m_nCmdIgnore;
	INT	m_nCmdUnIgnore;
	INT	m_nCmdOp;
	INT	m_nCmdDeOp;
	INT	m_nCmdVoice;
	INT	m_nCmdDeVoice;
	INT	m_nCmdKick;
	INT	m_nCmdKickWhy;
	INT	m_nCmdBan;
	INT	m_nCmdUnBan;
	INT	m_nCmdBanKick;
	INT	m_nCmdBanKickWhy;
	INT	m_nCmdConnect;
	INT	m_nCmdDisconnect;
	INT	m_nCmdSettings;
	INT	m_nCmdAdd;
	INT	m_nCmdRemove;

	// ICommandPlugin Methods
public:
	STDMETHOD(RegisterCommands)();
	STDMETHOD(InsertCommands)();
	STDMETHOD(OnUpdate)(INT nCommandID, STRISTATE* pbVisible, 
						STRISTATE* pbEnabled, STRISTATE* pbChecked);
	STDMETHOD(OnCommand)(INT nCommandID);

	// IGeneralPlugin Methods
public:
	STDMETHOD(SetApplication)(IApplication* pApplication);
	STDMETHOD(QueryCapabilities)(LONG* pnCaps);
	STDMETHOD(Configure)();
	STDMETHOD(OnSkinChanged)();
};

OBJECT_ENTRY_AUTO(__uuidof(IRCPlugin), CIRCPlugin)
