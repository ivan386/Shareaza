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
	CComPtr<IApplication>	m_pApplication;
	CComPtr<IUserInterface>	m_pInterface;
	CIRCWnd*				m_pWindow;

public:
	UINT	m_nCmdCloseTab;
	UINT	m_nCmdWhois;
	UINT	m_nCmdQuery;
	UINT	m_nCmdTime;
	UINT	m_nCmdVersion;
	UINT	m_nCmdIgnore;
	UINT	m_nCmdUnIgnore;
	UINT	m_nCmdOp;
	UINT	m_nCmdDeOp;
	UINT	m_nCmdVoice;
	UINT	m_nCmdDeVoice;
	UINT	m_nCmdKick;
	UINT	m_nCmdKickWhy;
	UINT	m_nCmdBan;
	UINT	m_nCmdUnBan;
	UINT	m_nCmdBanKick;
	UINT	m_nCmdBanKickWhy;
	UINT	m_nCmdConnect;
	UINT	m_nCmdDisconnect;
	UINT	m_nCmdSettings;
	UINT	m_nCmdOpen;
	UINT	m_nCmdAdd;
	UINT	m_nCmdRemove;

	// ICommandPlugin Methods
public:
	STDMETHOD(RegisterCommands)();
	STDMETHOD(InsertCommands)();
	STDMETHOD(OnUpdate)(unsigned int nCommandID, STRISTATE* pbVisible, 
						STRISTATE* pbEnabled, STRISTATE* pbChecked);
	STDMETHOD(OnCommand)(unsigned int nCommandID);

	// IGeneralPlugin Methods
public:
	STDMETHOD(SetApplication)(IApplication* pApplication);
	STDMETHOD(QueryCapabilities)(unsigned long* pnCaps);
	STDMETHOD(Configure)();
	STDMETHOD(OnSkinChanged)();
};

OBJECT_ENTRY_AUTO(__uuidof(IRCPlugin), CIRCPlugin)
