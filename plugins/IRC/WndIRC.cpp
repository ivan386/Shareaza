//
// WndIRC.cpp
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
#include "WndIRC.h"
#include "IRC.h"
#include "IRCPlugin.h"

CIRCWnd::CIRCWnd() : m_pFrame(NULL)
{
}

//////////////////////////////////////////////////////////////////////
// CIRCWnd destruction

CIRCWnd::~CIRCWnd()
{
	if ( IsWindow() )
		Detach();
}

BOOL CIRCWnd::Initialize(CIRCPlugin* pPlugin, LPCTSTR pszClassName)
{
	__super::Initialize( pPlugin, pszClassName );

	// Don't listen to WM_CREATE. Otherwise no tab will be created in the Tab bar
	m_pWindow->ListenForSingleMessage( WM_CLOSE );
	m_pWindow->ListenForSingleMessage( WM_DESTROY );
	m_pWindow->ListenForSingleMessage( WM_CONTEXTMENU );
	m_pWindow->ListenForSingleMessage( WM_SIZE );
	m_pWindow->ListenForSingleMessage( WM_PAINT );
	m_pWindow->ListenForSingleMessage( WM_NCLBUTTONUP );
	m_pWindow->ListenForSingleMessage( WM_SETCURSOR );

	m_pWindow->Create2( m_pPlugin->m_nCmdWindow2, VARIANT_TRUE, VARIANT_FALSE );
	m_pWindow->GetHwnd( &m_hWnd );
	m_pFrame = new CComObject< CIRCFrame >;
	m_pFrame->Initialize( m_pPlugin, m_hWnd );

	// Load window position and size. The frame will be sized too
	m_pWindow->LoadState( VARIANT_FALSE );

	return TRUE;
}

void CIRCWnd::OnSkinChanged()
{
	m_pFrame->OnSkinChanged();
}

BOOL CIRCWnd::Refresh()
{
	// Go into wait cursor mode
	HCURSOR hCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
	KillTimer( 2 );
	{
		// do something
	}
	SetTimer( 2, 5000, NULL );

	// Restore the old cursor
	SetCursor( hCursor );

	return TRUE;
}

// IPluginWindowOwner Methods
STDMETHODIMP CIRCWnd::OnTranslate(MSG* pMessage)
{
	// Add your function implementation here.
	return __super::OnTranslate( pMessage );
}

STDMETHODIMP CIRCWnd::OnMessage(INT nMessage, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
{
	return ProcessWindowMessage( m_hWnd, nMessage, wParam, lParam, *plResult ) ? S_OK : S_FALSE;
}

STDMETHODIMP CIRCWnd::OnUpdate(INT nCommandID, STRISTATE* pbVisible, 
							   STRISTATE* pbEnabled, STRISTATE* pbChecked)
{
	// Called when window is active.
	// Check the button or menu entry (rectangular around them)
	if ( nCommandID == m_pPlugin->m_nCmdWindow || nCommandID == m_pPlugin->m_nCmdWindow2 )
	{
		if ( pbChecked )
			*pbChecked = TSTRUE;
		return S_OK;
	}

	return S_FALSE;
}

STDMETHODIMP CIRCWnd::OnCommand(INT nCommandID)
{
	if ( m_pFrame && m_pFrame->m_hWnd != NULL )
	{
		if ( m_pFrame->OnCommand( nCommandID ) == S_OK )
			return S_OK;
	}

	return S_FALSE;
}

STDMETHODIMP CIRCWnd::GetWndClassName(BSTR* pszClassName)
{
	return __super::GetWndClassName( pszClassName );
}

LRESULT CIRCWnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ( uMsg == WM_DESTROY )
	{
		// Save window position and size (for windowed mode)
		m_pWindow->SaveState();
		m_pFrame->DestroyWindow();
		m_pPlugin->m_pWindow = NULL;
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CIRCWnd::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	if ( m_pFrame && m_pFrame->m_hWnd != NULL )
		m_pFrame->SetWindowPos( NULL, 0, 0, (int)LOWORD(lParam), (int)HIWORD(lParam), SWP_NOZORDER );
	return 0;
}

LRESULT CIRCWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rcClient;
	PAINTSTRUCT ps;

	// Get the client rectangle, and begin painting
	GetClientRect( &rcClient );
	HDC hDC = BeginPaint( &ps );

	::SetBkMode( hDC, OPAQUE );
	::SetBkColor( hDC, RGB(200,200,200) );

	CComBSTR bsText( L"Empty area..." );
	::ExtTextOut( hDC, 6, 6, ETO_OPAQUE, &rcClient, bsText, bsText.Length(), NULL );

	// Finish painting
	EndPaint( &ps );

	return 0;
}

LRESULT CIRCWnd::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if ( wParam == 2 )
	{
		//Refresh();
	}

	bHandled = FALSE;
	return 0;
}