//
// WndPlugin.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2011.
// This file is part of SHAREAZA (shareaza.sourceforge.net)
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
#include "Shareaza.h"
#include "Settings.h"
#include "WndPlugin.h"
#include "WindowManager.h"
#include "CoolInterface.h"
#include "Skin.h"
#include "SkinWindow.h"
#include "ComToolbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPluginWnd, CPanelWnd)

BEGIN_MESSAGE_MAP(CPluginWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_INTERFACE_MAP(CPluginWnd, CPanelWnd)
	INTERFACE_PART(CPluginWnd, IID_IPluginWindow, PluginWindow)
END_INTERFACE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPluginWnd construction

CPluginWnd::CPluginWnd(LPCTSTR pszName, IPluginWindowOwner* pOwner)
	: m_pOwner	( pOwner )
	, m_sName	( pszName )
	, m_pHandled( NULL )
	, m_nHandled( 0 )
	, m_pToolbar( NULL )
	, m_bAccel	( TRUE )
{
	InternalAddRef();
}

CPluginWnd::~CPluginWnd()
{
	delete [] m_pHandled;
	delete m_pToolbar;
}

/////////////////////////////////////////////////////////////////////////////
// CPluginWnd message handlers

BOOL CPluginWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( m_bAccel && pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST && m_pOwner )
	{
		HRESULT hr = m_pOwner->OnTranslate( pMsg );
		if ( S_OK == hr ) return TRUE;
		if ( E_NOTIMPL == hr ) m_bAccel = FALSE;
	}

	return CPanelWnd::PreTranslateMessage( pMsg );
}

LRESULT CPluginWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT* pHandled = m_pHandled;
	DWORD nHandled = m_nHandled;

	while ( nHandled-- )
	{
		if ( *pHandled++ == message )
		{
			LRESULT lResult;
			if ( m_pOwner && S_OK == m_pOwner->OnMessage( message, wParam, lParam, &lResult ) )
				return lResult;
			break;
		}
	}

	return CPanelWnd::WindowProc( message, wParam, lParam );
}

int CPluginWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_bAlert = 1982;
	OnSkinChange();
	m_bAlert = FALSE;

	GetManager()->Add( this );

	return 0;
}

void CPluginWnd::OnSize(UINT nType, int cx, int cy)
{
	if ( nType != 1982 ) CPanelWnd::OnSize( nType, cx, cy );

	if ( m_pToolbar != NULL )
	{
		CRect rc;
		GetClientRect( &rc );

		if ( m_nToolbar == 1 )
		{
			m_pToolbar->SetWindowPos( NULL, 0, 0, rc.right, 28, SWP_NOZORDER );
		}
		else if ( m_nToolbar == 2 )
		{
			m_pToolbar->SetWindowPos( NULL, 0, rc.bottom - 28, rc.Width(), 28, SWP_NOZORDER );
		}
	}
}

void CPluginWnd::OnSkinChange()
{
	m_pSkin = Skin.GetWindowSkin( m_sName );
	if ( m_pSkin == NULL ) m_pSkin = Skin.GetWindowSkin( this );

	if ( m_nResID )
	{
		CoolInterface.SetIcon( m_nResID, Settings.General.LanguageRTL, FALSE, this );

		CString strCaption;
		Skin.LoadString( strCaption, m_nResID );

		SetWindowText( _T("") );
		SetWindowText( strCaption );
	}

	if ( m_bAlert != 1982 )
	{
		SetWindowRgn( NULL, FALSE );
		SetWindowPos( NULL, 0, 0, 0, 0,
			SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_FRAMECHANGED );

		if ( m_pSkin ) m_pSkin->OnSize( this );

		LRESULT lResult = 0;
		if ( m_pOwner ) m_pOwner->OnMessage( WM_SKINCHANGED, 0, 0, &lResult );
	}
}

HRESULT CPluginWnd::GetGenericView(IGenericView** ppView)
{
	return ( m_pOwner && SUCCEEDED( m_pOwner->QueryInterface( IID_IGenericView, (void**)ppView ) ) ) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CPluginWnd IPluginWindow

IMPLEMENT_UNKNOWN(CPluginWnd, PluginWindow)

STDMETHODIMP CPluginWnd::XPluginWindow::ListenForSingleMessage(UINT nMessage)
{
	METHOD_PROLOGUE( CPluginWnd, PluginWindow )

	UINT* pHandled = new UINT[ pThis->m_nHandled + 1 ];
	if ( ! pHandled ) return E_OUTOFMEMORY;

	if ( pThis->m_pHandled )
	{
		CopyMemory( pHandled, pThis->m_pHandled, sizeof(UINT) * pThis->m_nHandled );
		delete [] pThis->m_pHandled;
	}
	pThis->m_pHandled = pHandled;
	pThis->m_pHandled[ pThis->m_nHandled++ ] = nMessage;

	return S_OK;
}

STDMETHODIMP CPluginWnd::XPluginWindow::ListenForMultipleMessages(SAFEARRAY FAR* pMessages)
{
	METHOD_PROLOGUE( CPluginWnd, PluginWindow )

	if ( SafeArrayGetDim( pMessages ) != 1 ) return E_INVALIDARG;
	if ( SafeArrayGetElemsize( pMessages ) != sizeof(UINT) ) return E_INVALIDARG;

	DWORD nCount;
	SafeArrayGetUBound( pMessages, 1, (LONG*)&nCount );
	nCount++;

	UINT* pHandled = new UINT[ pThis->m_nHandled + nCount ];
	if ( ! pHandled ) return E_OUTOFMEMORY;

	if ( pThis->m_pHandled )
	{
		CopyMemory( pHandled, pThis->m_pHandled, sizeof(UINT) * pThis->m_nHandled );
		delete [] pThis->m_pHandled;
	}
	pThis->m_pHandled = pHandled;

	UINT* pSource;
	SafeArrayAccessData( pMessages, (void**)&pSource );
	while ( nCount-- ) pThis->m_pHandled[ pThis->m_nHandled++ ] = *pSource++;
	SafeArrayUnaccessData( pMessages );

	return S_OK;
}

STDMETHODIMP CPluginWnd::XPluginWindow::Create1(BSTR bsCaption, HICON hIcon, VARIANT_BOOL bPanel, VARIANT_BOOL bTabbed)
{
	METHOD_PROLOGUE( CPluginWnd, PluginWindow )

	pThis->m_bPanelMode	= ( Settings.General.GUIMode != GUI_WINDOWED && ( bPanel == VARIANT_TRUE ) );
	pThis->m_bTabMode	= ( pThis->m_bPanelMode && ( bTabbed == VARIANT_TRUE ) );

	if ( ! pThis->Create( 0, FALSE ) ) return E_FAIL;

	if ( hIcon != NULL ) pThis->SetIcon( hIcon, FALSE );
	pThis->SetWindowText( CString( bsCaption ) );

	if ( pThis->m_bPanelMode )
		LoadState( VARIANT_TRUE );

	return S_OK;
}

STDMETHODIMP CPluginWnd::XPluginWindow::Create2(UINT nCommandID, VARIANT_BOOL bPanel, VARIANT_BOOL bTabbed)
{
	METHOD_PROLOGUE( CPluginWnd, PluginWindow )

	pThis->m_bPanelMode	= ( Settings.General.GUIMode != GUI_WINDOWED && ( bPanel == VARIANT_TRUE ) );
	pThis->m_bTabMode	= ( pThis->m_bPanelMode && ( bTabbed == VARIANT_TRUE ) );

	if ( ! pThis->Create( nCommandID, FALSE ) ) return E_FAIL;

	if ( pThis->m_bPanelMode )
		LoadState( VARIANT_TRUE );

	return S_OK;
}

STDMETHODIMP CPluginWnd::XPluginWindow::GetHwnd(HWND FAR* phWnd)
{
	METHOD_PROLOGUE( CPluginWnd, PluginWindow )
	*phWnd = pThis->GetSafeHwnd();
	return S_OK;
}

STDMETHODIMP CPluginWnd::XPluginWindow::HandleMessage(LRESULT* plResult)
{
	METHOD_PROLOGUE( CPluginWnd, PluginWindow )

	if ( MSG const * pMsg = pThis->GetCurrentMessage() )
	{
		*plResult = pThis->CPanelWnd::WindowProc( pMsg->message, pMsg->wParam, pMsg->lParam );
		return S_OK;
	}
	else
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CPluginWnd::XPluginWindow::LoadState(VARIANT_BOOL bMaximise)
{
	METHOD_PROLOGUE( CPluginWnd, PluginWindow )
	pThis->LoadState( pThis->m_sName, bMaximise == VARIANT_TRUE );
	return S_OK;
}

STDMETHODIMP CPluginWnd::XPluginWindow::SaveState()
{
	METHOD_PROLOGUE( CPluginWnd, PluginWindow )
	pThis->SaveState( pThis->m_sName );
	return S_OK;
}

STDMETHODIMP CPluginWnd::XPluginWindow::ThrowMenu(BSTR sName, LONG nDefaultID, POINT FAR* pPoint)
{
	METHOD_PROLOGUE( CPluginWnd, PluginWindow )
	CPoint pt;
	if ( pPoint ) pt = *pPoint;
	else GetCursorPos( &pt );
	Skin.TrackPopupMenu( CString( sName ), pt, (UINT)nDefaultID );
	return S_OK;
}

STDMETHODIMP CPluginWnd::XPluginWindow::AddToolbar(BSTR sName, LONG nPosition, HWND FAR* phWnd, ISToolbar FAR* FAR* ppToolbar)
{
	METHOD_PROLOGUE( CPluginWnd, PluginWindow )

	if ( pThis->m_pToolbar != NULL ) delete pThis->m_pToolbar;

	pThis->m_nToolbar = nPosition ? nPosition : 2;

	pThis->m_pToolbar = new CCoolBarCtrl();
	pThis->m_pToolbar->Create( pThis, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR );
	pThis->m_pToolbar->SetBarStyle( pThis->m_pToolbar->GetBarStyle() | CBRS_TOOLTIPS );
	pThis->m_pToolbar->SetOwner( AfxGetMainWnd() );

	if ( pThis->m_nToolbar & 1 )
	{
		pThis->m_pToolbar->SetBarStyle( pThis->m_pToolbar->GetBarStyle() | CBRS_BORDER_BOTTOM );
	}

	if ( pThis->m_nToolbar & 2 )
	{
		pThis->m_pToolbar->SetBarStyle( pThis->m_pToolbar->GetBarStyle() | CBRS_BORDER_TOP );
	}

	Skin.CreateToolBar( CString( sName ), pThis->m_pToolbar );

	if ( phWnd ) *phWnd = pThis->m_pToolbar->GetSafeHwnd();
	if ( ppToolbar ) *ppToolbar = CComToolbar::Wrap( pThis->m_pToolbar );

	pThis->SendMessage( WM_SIZE, 1982, 0 );

	return S_OK;
}

STDMETHODIMP CPluginWnd::XPluginWindow::AdjustWindowRect(RECT FAR* pRect, VARIANT_BOOL bClientToWindow)
{
	METHOD_PROLOGUE( CPluginWnd, PluginWindow )

	if ( pThis->m_pSkin != NULL )
	{
		pThis->m_pSkin->CalcWindowRect( pRect, bClientToWindow ? FALSE : TRUE );
	}
	else
	{
		pThis->CalcWindowRect( pRect );
	}

	return NOERROR;
}

