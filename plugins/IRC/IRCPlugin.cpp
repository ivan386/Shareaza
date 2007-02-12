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
	m_pInterface->RegisterCommand( L"ID_TAB_IRC", NULL, &m_nCmdWindow );
	m_pInterface->RegisterCommand( L"WINDOW_IRC", NULL, &m_nCmdWindow2 );
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
	m_pInterface->RegisterCommand( L"IRC_Add", NULL, &m_nCmdAdd );
	m_pInterface->RegisterCommand( L"IRC_Remove", NULL, &m_nCmdRemove );

	return S_OK;
}

STDMETHODIMP CIRCPlugin::InsertCommands()
{
	try
	{
		HRESULT hr = m_pInterface->AddFromResource( v_hResources, IDR_SKIN );
		THROW_HR( hr );

		// Add a button to all tool bars. We don't use the skin file because
		// each plugin could insert its own button.
		CComPtr< ISToolbar > pMainToolbar;
		hr = m_pInterface->GetToolbar( CComBSTR( L"CMainWnd.Tabbed" ), VARIANT_FALSE, &pMainToolbar );
		THROW_HR( hr );

		CComPtr< ISToolbarItem > pToolbarItem;
		hr = pMainToolbar->InsertButton( 4, m_nCmdWindow, CComBSTR( L"Chat" ), &pToolbarItem );
		THROW_HR( hr );
		pMainToolbar.Release();
		pToolbarItem.Release();

		hr = m_pInterface->GetToolbar( CComBSTR( L"CMainWnd.Windowed" ), VARIANT_FALSE, &pMainToolbar );
		THROW_HR( hr );
		hr = pMainToolbar->InsertButton( 10, m_nCmdWindow, CComBSTR( L"" ), &pToolbarItem );
		THROW_HR( hr );
		pMainToolbar.Release();
		pToolbarItem.Release();

		struct MenuDataEntry
		{
			LPCTSTR pszMenuName;
			long nMenuItemPosition;
			long nCommandPosition;
		};

		static const MenuDataEntry menuData[] =
		{
			{ L"CMainWnd.Basic", 1, 3 },
			{ L"CMainWnd.Tabbed", 1, 3 },
			{ L"CMainWnd.Windowed", 1, 6 },
			{ L"CMainWnd.View.Basic", 0, 3 },
			{ L"CMainWnd.View.Tabbed", 0, 3 },
			{ L"CMainWnd.View.Windowed", 0, 8 },
			{ L"CHomeWnd.Basic", 0, 3 },
			{ L"CHomeWnd.Tabbed", 0, 3 }
		};

		CComPtr< ISMenu > pMenu;
		CComPtr< ISMenu > pMenuItem;
		CComPtr< ISMenu > pNewItem;
		for ( u_short nConfig = 0 ; nConfig < 8 ; nConfig++ )
		{
			hr = m_pInterface->GetMenu( CComBSTR( menuData[ nConfig ].pszMenuName ), 
				VARIANT_FALSE, &pMenu );
			if ( FAILED(hr) ) continue;

			hr = pMenu->get_Item( CComVariant( menuData[ nConfig ].nMenuItemPosition ), &pMenuItem );
			if ( FAILED(hr) ) continue;

			hr = pMenuItem->InsertCommand( menuData[ nConfig ].nCommandPosition, 
				m_nCmdWindow, CComBSTR( L"&Chat" ), &pNewItem );

			if ( FAILED(hr) ) // right-click menu
				hr = pMenu->InsertCommand( menuData[ nConfig ].nCommandPosition,
				m_nCmdWindow, CComBSTR( L"&Chat" ), &pNewItem );
			pNewItem.Release();
			pMenuItem.Release();
			pMenu.Release();
		}
	}
	catch ( HRESULT hrErr )
	{
		return hrErr;
	}

	return S_OK;
}

STDMETHODIMP CIRCPlugin::OnUpdate(INT nCommandID, STRISTATE* pbVisible, 
								  STRISTATE* pbEnabled, STRISTATE* pbChecked)
{
	// Called when window is inactive. 
	// Button enabled but unchecked (no rectangular around button or menu entry)
	if ( nCommandID == m_nCmdWindow || nCommandID == m_nCmdWindow2 )
	{
		if ( pbEnabled )
			*pbEnabled = TSTRUE;
		if ( pbChecked )
		{
			if ( m_pWindow )
			{
				SGUIMode mode = GuiWindowed;
				m_pInterface->get_GUIMode( &mode );
				*pbChecked = mode == GuiWindowed ? TSTRUE : TSFALSE;
			}
			else
				*pbChecked = TSFALSE;
		}
		return S_OK;
	}

	return S_FALSE;
}

STDMETHODIMP CIRCPlugin::OnCommand(INT nCommandID)
{
	if ( nCommandID == m_nCmdWindow || nCommandID == m_nCmdWindow2 )
	{
		if ( m_pWindow == NULL )
		{
			m_pWindow = new CComObject< CIRCWnd >;
			m_pWindow->Initialize( this, L"CIRCWnd" );
			m_pWindow->ShowWindow( SW_SHOWNORMAL );
		}

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

STDMETHODIMP CIRCPlugin::QueryCapabilities(LONG* pnCaps)
{
	return S_OK;
}

STDMETHODIMP CIRCPlugin::Configure()
{
	return E_NOTIMPL;
}

STDMETHODIMP CIRCPlugin::OnSkinChanged()
{
	if ( m_pWindow )
		m_pWindow->OnSkinChanged();
	return S_OK;
}
