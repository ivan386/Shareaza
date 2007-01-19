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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CIRCWnd::CIRCWnd() : m_pPlugin(NULL)
{
}

//////////////////////////////////////////////////////////////////////
// CIRCWnd destruction

CIRCWnd::~CIRCWnd()
{
	// Destroy the various objects we may have allocated
}

BOOL CIRCWnd::Create(CIRCPlugin* pPlugin)
{
	m_pPlugin		= pPlugin;
	m_pApplication	= pPlugin->m_pApplication;

	// Get an IUserInterface pointer from the IApplication
	CComPtr< IUserInterface > pUI;
	m_pApplication->get_UserInterface( &pUI );

	pUI->NewWindow( L"IRCWindow", this, &m_pWindow );
	m_pWindow->ListenForSingleMessage( WM_CREATE );
	m_pWindow->ListenForSingleMessage( WM_CLOSE );
	m_pWindow->ListenForSingleMessage( WM_DESTROY );
	m_pWindow->ListenForSingleMessage( WM_CONTEXTMENU );
	m_pWindow->ListenForSingleMessage( WM_SIZE );
	m_pWindow->ListenForSingleMessage( WM_PAINT );
	m_pWindow->ListenForSingleMessage( WM_NCLBUTTONUP );
	m_pWindow->ListenForSingleMessage( WM_SETCURSOR );
	m_pWindow->ListenForSingleMessage( WM_SYSCOMMAND );

	m_pWindow->Create2( m_pPlugin->m_nCmdOpen, VARIANT_TRUE, VARIANT_TRUE );
	m_pWindow->GetHwnd( &m_hWnd );

	return TRUE;
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

BOOL CIRCWnd::ResizeWindow()
{
	HWND hMDI = GetParent();
	RECT rcWnd, rcMDI;

	::GetClientRect( hMDI, &rcMDI );
	rcWnd.left = rcWnd.top = 0;
	m_pWindow->AdjustWindowRect( &rcWnd, VARIANT_TRUE );

	rcWnd.left = rcMDI.left;
	rcWnd.right = rcMDI.right;
	rcWnd.top = rcMDI.top;
	rcWnd.bottom = rcMDI.bottom;

	// Move
	MoveWindow( &rcWnd );

	return TRUE;
}

// IPluginWindowOwner Methods
STDMETHODIMP CIRCWnd::OnTranslate(MSG* pMessage)
{
	// Add your function implementation here.
	return E_NOTIMPL;
}

STDMETHODIMP CIRCWnd::OnMessage(UINT nMessage, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
{
	return ProcessWindowMessage( m_hWnd, nMessage, wParam, lParam, *plResult ) ? S_OK : S_FALSE;
}

STDMETHODIMP CIRCWnd::OnUpdate(UINT nCommandID, STRISTATE* pbVisible, 
							   STRISTATE* pbEnabled, STRISTATE* pbChecked)
{
	if ( nCommandID == m_pPlugin->m_nCmdOpen )
		return S_OK;

	return S_FALSE;
}

STDMETHODIMP CIRCWnd::OnCommand(UINT nCommandID)
{
	if ( nCommandID == WM_CLOSE || nCommandID == WM_DESTROY )
	{
		PostMessage( WM_CLOSE );
		return S_OK;
	}

	return S_FALSE;
}

LRESULT CIRCWnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ( uMsg == WM_DESTROY )
	{
		m_pPlugin->m_pWindow = NULL;
	}
	
	bHandled = FALSE;
	return 0;
}

LRESULT CIRCWnd::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = FALSE;
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