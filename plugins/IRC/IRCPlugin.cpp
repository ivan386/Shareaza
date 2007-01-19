//
// IRCPlugin.cpp
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

#include "StdAfx.h"
#include "IRCPlugin.h"
#include "WndIRC.h"

CIRCPlugin::CIRCPlugin() : m_pWindow(NULL)
{
}

// ICommandPlugin Methods
STDMETHODIMP CIRCPlugin::RegisterCommands()
{
	m_pInterface->RegisterCommand( L"IRC_CloseTab", NULL, &m_nCmdCloseTab );
	m_pInterface->RegisterCommand( L"IRC_Whois", NULL, &m_nCmdWhois );
	m_pInterface->RegisterCommand( L"IRC_Query", NULL, &m_nCmdQuery );
	m_pInterface->RegisterCommand( L"IRC_Time", NULL, &m_nCmdTime );
	m_pInterface->RegisterCommand( L"IRC_Version", NULL, &m_nCmdVersion );
	m_pInterface->RegisterCommand( L"IRC_Ignore", NULL, &m_nCmdIgnore );
	m_pInterface->RegisterCommand( L"IRC_UnIgnore", NULL, &m_nCmdUnIgnore );
	m_pInterface->RegisterCommand( L"IRC_Op", NULL, &m_nCmdOp );
	m_pInterface->RegisterCommand( L"IRC_DeOp", NULL, &m_nCmdDeOp );
	m_pInterface->RegisterCommand( L"IRC_Voice", NULL, &m_nCmdVoice );
	m_pInterface->RegisterCommand( L"IRC_DeVoice", NULL, &m_nCmdDeVoice );
	m_pInterface->RegisterCommand( L"IRC_Kick", NULL, &m_nCmdKick );
	m_pInterface->RegisterCommand( L"IRC_KickWhy", NULL, &m_nCmdKickWhy );
	m_pInterface->RegisterCommand( L"IRC_Ban", NULL, &m_nCmdBan );
	m_pInterface->RegisterCommand( L"IRC_UnBan", NULL, &m_nCmdUnBan );
	m_pInterface->RegisterCommand( L"IRC_BanKick", NULL, &m_nCmdBanKick );
	m_pInterface->RegisterCommand( L"IRC_BanKickWhy", NULL, &m_nCmdBanKickWhy );
	m_pInterface->RegisterCommand( L"IRC_Connect", NULL, &m_nCmdConnect );
	m_pInterface->RegisterCommand( L"IRC_Disconnect", NULL, &m_nCmdDisconnect );
	m_pInterface->RegisterCommand( L"IRC_Settings", NULL, &m_nCmdSettings );
	m_pInterface->RegisterCommand( L"ID_TAB_IRC", NULL, &m_nCmdOpen );
	m_pInterface->RegisterCommand( L"IRC_Add", NULL, &m_nCmdAdd );
	m_pInterface->RegisterCommand( L"IRC_Remove", NULL, &m_nCmdRemove );

	return S_OK;
}

STDMETHODIMP CIRCPlugin::InsertCommands()
{
	m_pInterface->AddFromResource( v_hResources, IDR_SKIN );
	return S_OK;
}

STDMETHODIMP CIRCPlugin::OnUpdate(unsigned int nCommandID, STRISTATE* pbVisible, 
								  STRISTATE* pbEnabled, STRISTATE* pbChecked)
{
	if ( pbEnabled && nCommandID == m_nCmdOpen )
	{
		*pbEnabled = TSTRUE;
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CIRCPlugin::OnCommand(unsigned int nCommandID)
{
	if ( nCommandID == m_nCmdOpen )
	{
		if ( m_pWindow == NULL )
		{
			m_pWindow = new CComObject< CIRCWnd >;
			m_pWindow->Create( this );
			m_pWindow->ShowWindow( SW_SHOWNORMAL );
		}

		m_pWindow->ResizeWindow();
		m_pWindow->BringWindowToTop();
		m_pWindow->Invalidate();

		return S_OK;
	}
	return S_FALSE;
}

// IGeneralPlugin Methods
STDMETHODIMP CIRCPlugin::SetApplication(IApplication* pApplication)
{
	m_pApplication = pApplication;
	m_pApplication->get_UserInterface( &m_pInterface );

	return S_OK;
}

STDMETHODIMP CIRCPlugin::QueryCapabilities(unsigned long* pnCaps)
{
	return S_OK;
}

STDMETHODIMP CIRCPlugin::Configure()
{
	return E_NOTIMPL;
}

STDMETHODIMP CIRCPlugin::OnSkinChanged()
{
	return S_OK;
}
