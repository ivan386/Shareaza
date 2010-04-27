//
// CtrlWeb.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "CtrlWeb.h"

#include "CoolMenu.h"
#include "Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CWebCtrl, CWnd)

BEGIN_INTERFACE_MAP(CWebCtrl::DocSite, COleControlSite)
	INTERFACE_PART(CWebCtrl::DocSite, IID_IDocHostUIHandler, DocHostUIHandler)
	INTERFACE_PART(CWebCtrl::DocSite, IID_IDocHostShowUI, DocHostShowUI)
	INTERFACE_PART(CWebCtrl::DocSite, IID_IServiceProvider, ServiceProvider)
	INTERFACE_PART(CWebCtrl::DocSite, IID_IInternetSecurityManager, InternetSecurityManager)
END_INTERFACE_MAP()

BEGIN_EVENTSINK_MAP(CWebCtrl, CWnd)
	ON_EVENT(CWebCtrl, AFX_IDW_PANE_FIRST, DISPID_BEFORENAVIGATE2, BeforeNavigate2, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CWebCtrl, AFX_IDW_PANE_FIRST, DISPID_NEWWINDOW2, OnNewWindow2, VTS_PDISPATCH VTS_PBOOL)
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CWebCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWebCtrl construction

CWebCtrl::CWebCtrl()
{
	m_bSandbox	= FALSE;
	m_tFrame	= 0;
	m_pMenu		= NULL;
}

CWebCtrl::~CWebCtrl()
{
	if ( m_pMenu != NULL )
	{
		EnterMenu( NULL );
		delete m_pMenu;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWebCtrl operations

BOOL CWebCtrl::Create(DWORD dwStyle, CWnd* pParentWnd, UINT nID)
{
	CRect rect( 0, 0, 0, 0 );
	return CWnd::Create( NULL, NULL, WS_CHILD|WS_CLIPCHILDREN|dwStyle,
		rect, pParentWnd, nID, NULL );
}

void CWebCtrl::EnableCoolMenu(BOOL bEnable)
{
	if ( m_pMenu != NULL )
	{
		if ( bEnable ) return;
		EnterMenu( NULL );
		delete m_pMenu;
		m_pMenu = NULL;
	}
	else
	{
		if ( ! bEnable ) return;
		m_pMenu = new CCoolMenu();
		m_pMenu->SetWatermark( Skin.GetWatermark( _T("CCoolMenu") ) );
	}
}

void CWebCtrl::EnableSandbox(BOOL bSandbox)
{
	m_bSandbox = bSandbox;
	if ( m_pBrowser ) m_pBrowser->put_Offline( m_bSandbox ? VARIANT_TRUE : VARIANT_FALSE );
}

void CWebCtrl::SetExternal(IDispatch* pDispatch)
{
	m_pExternal = pDispatch;
}

HRESULT CWebCtrl::Navigate(LPCTSTR lpszURL, DWORD dwFlags, LPCTSTR lpszTargetFrameName, LPCTSTR lpszHeaders, LPVOID lpvPostData, DWORD dwPostDataLen)
{
	if ( m_pBrowser == NULL ) return E_UNEXPECTED;

	CComBSTR bstrURL( lpszURL );

	COleSafeArray vPostData;
	if ( lpvPostData != NULL )
	{
		if ( dwPostDataLen == 0 ) dwPostDataLen = lstrlen( (LPCTSTR)lpvPostData );
		vPostData.CreateOneDim( VT_UI1, dwPostDataLen, lpvPostData );
	}

	return m_pBrowser->Navigate( bstrURL, COleVariant( (long) dwFlags, VT_I4 ),
        COleVariant( lpszTargetFrameName, VT_BSTR ), vPostData,
		COleVariant( lpszHeaders, VT_BSTR ) );
}

/////////////////////////////////////////////////////////////////////////////
// CWebCtrl message handlers

int CWebCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	AfxEnableControlContainer();

	CRect rect;
	GetClientRect( rect );

	if ( m_wndBrowser.CreateControl( CLSID_WebBrowser, NULL,
		 WS_VISIBLE|WS_CHILD, rect, this, AFX_IDW_PANE_FIRST ) )
	{
		IUnknown* pUnknown = m_wndBrowser.GetControlUnknown();

		if ( pUnknown != NULL &&
			 SUCCEEDED( pUnknown->QueryInterface( IID_IWebBrowser2, (void**)&m_pBrowser ) ) &&
			 m_pBrowser )
		{
			if ( m_bSandbox ) m_pBrowser->put_Offline( VARIANT_TRUE );
		}
		else
		{
			m_pBrowser = NULL;
			m_wndBrowser.DestroyWindow();
		}
	}

	return 0;
}

void CWebCtrl::OnDestroy()
{
	EnterMenu( NULL );

	m_pBrowser = NULL;
	if ( m_wndBrowser.m_hWnd != NULL ) m_wndBrowser.DestroyWindow();

	CWnd::OnDestroy();
}

BOOL CWebCtrl::PreTranslateMessage(MSG* pMsg)
{
	if ( m_pBrowser )
	{
		CComQIPtr<IOleInPlaceActiveObject> pInPlace( m_pBrowser );
		if ( pInPlace )
			return pInPlace->TranslateAccelerator( pMsg ) == S_OK;
	}

	return CWnd::PreTranslateMessage( pMsg );
}

void CWebCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );

	if ( ::IsWindow( m_wndBrowser.m_hWnd ) )
	{
		CRect rect;
		GetClientRect( rect );
		::AdjustWindowRectEx( rect, (DWORD)::GetWindowLongPtr( m_wndBrowser, GWL_STYLE ), FALSE, WS_EX_CLIENTEDGE );
		m_wndBrowser.SetWindowPos( NULL, rect.left, rect.top,
			rect.Width(), rect.Height(), SWP_NOACTIVATE|SWP_NOZORDER );
	}
}

void CWebCtrl::OnPaint()
{
	CPaintDC dc( this );

	if ( ! ::IsWindow( m_wndBrowser.m_hWnd ) )
	{
		CString str = _T("Internet Explorer is required for this feature.");
		CRect rc;
		GetClientRect( &rc );
		dc.FillSolidRect( &rc, GetSysColor( COLOR_WINDOW ) );
		dc.SetTextColor( GetSysColor( COLOR_WINDOWTEXT ) );
		dc.DrawText( str, &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWebCtrl cool menu driver

CWebCtrl* CWebCtrl::m_pThis = NULL;

void CWebCtrl::EnterMenu(POINT* pPoint)
{
	if ( m_pThis != NULL )
	{
		SetWindowLongPtr( m_pThis->m_hWndThis, GWLP_WNDPROC, (LONG_PTR)m_pThis->m_pWndProc );
		m_pThis = NULL;
	}

	if ( pPoint == NULL || m_pMenu == NULL ) return;

	CPoint ptScreen( *pPoint );
	ptScreen.Offset( 2, 2 );

    CWnd* pChild = this;
	for ( ; ; )
	{
		CPoint ptClient( ptScreen );
		pChild->ScreenToClient( &ptClient );
		CWnd* pNext = pChild->ChildWindowFromPoint( ptClient, CWP_ALL );
		if ( pNext == NULL || pNext == pChild ) break;
		pChild = pNext;
	}

	TCHAR szClass[128];
    GetClassName( *pChild, szClass, 128 );
	if ( _tcsistr( szClass, _T("Internet Explorer") ) == NULL ) return;

	m_pThis = this;
	m_hWndThis = pChild->GetSafeHwnd();
	m_pWndProc = (WNDPROC)(LONG_PTR)GetWindowLongPtr( m_hWndThis, GWLP_WNDPROC );
	SetWindowLongPtr( m_hWndThis, GWLP_WNDPROC, (LONG_PTR)&WebWndProc );
}

LRESULT PASCAL CWebCtrl::WebWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC pWndProc = m_pThis->m_pWndProc;

	ASSERT( m_pThis->m_hWndThis == hWnd );

    switch ( nMsg )
	{
	case WM_DESTROY:
	case WM_EXITMENULOOP:
		SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG_PTR)pWndProc );
		m_pThis = NULL;
		break;
	case WM_INITMENUPOPUP:
		m_pThis->m_pMenu->AddMenu( CMenu::FromHandle( (HMENU)wParam ), TRUE );
		break;
	case WM_MEASUREITEM:
		m_pThis->m_pMenu->OnMeasureItem( (LPMEASUREITEMSTRUCT)lParam );
		return 0;
	case WM_DRAWITEM:
		m_pThis->m_pMenu->OnDrawItem( (LPDRAWITEMSTRUCT)lParam );
		return 0;
	}

	return CallWindowProc( pWndProc, hWnd, nMsg, wParam, lParam );
}

/////////////////////////////////////////////////////////////////////////////
// CWebCtrl browser event handlers

void CWebCtrl::BeforeNavigate2(LPDISPATCH /*pDispatch*/, VARIANT* pvURL, VARIANT* pvFlags, VARIANT* pvTargetFrameName, VARIANT* pvPostData, VARIANT* pvHeaders, VARIANT_BOOL* pvCancel)
{
	ASSERT(V_VT(pvURL) == VT_BSTR);
	ASSERT(V_VT(pvTargetFrameName) == VT_BSTR);
	ASSERT(pvCancel != NULL);
	*pvCancel = VARIANT_FALSE;

	if ( SysStringLen( V_BSTR(pvTargetFrameName) ) == 0 )
	{
		CString strURL( V_BSTR(pvURL) );

		if ( _tcsncmp( strURL, _T("http"), 4 ) == 0 )
		{
			*pvCancel = VARIANT_TRUE;
			m_tFrame = GetTickCount();
			COleVariant vFrame(	 _T("_blank"), VT_BSTR );
			m_pBrowser->Navigate2( pvURL, pvFlags, &vFrame, pvPostData, pvHeaders );
		}
	}
}

void CWebCtrl::OnNewWindow2(LPDISPATCH* /*ppDisp*/, VARIANT_BOOL* pbCancel)
{
	*pbCancel = VARIANT_FALSE;

	if ( m_bSandbox )
	{
		if ( GetTickCount() - m_tFrame > 1500 ) *pbCancel = VARIANT_TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWebCtrl::DocSite control site implementation

BOOL CWebCtrl::CreateControlSite(COleControlContainer* pContainer, COleControlSite** ppSite, UINT /*nID*/, REFCLSID /*clsid*/)
{
	ASSERT( ppSite != NULL );
	*ppSite = new DocSite( this, pContainer );
	return TRUE;
}

CWebCtrl::DocSite::DocSite(CWebCtrl* pCtrl, COleControlContainer* pContainer) : COleControlSite( pContainer )
{
	m_pCtrl = pCtrl;
}

/////////////////////////////////////////////////////////////////////////////
// CWebCtrl::DocSite IDocHostUIHandler

IMPLEMENT_UNKNOWN(CWebCtrl::DocSite, DocHostUIHandler)

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::GetExternal(LPDISPATCH *lppDispatch)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	CWebCtrl* pCtrl = pThis->GetCtrl();

	*lppDispatch = pCtrl->m_pExternal;
	if ( *lppDispatch != NULL ) (*lppDispatch)->AddRef();

	return ( *lppDispatch != NULL ) ? S_OK : S_FALSE;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::ShowContextMenu(DWORD dwID, LPPOINT ppt, LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	CWebCtrl* pCtrl = pThis->GetCtrl();
	WVNCONTEXTMENU pNotify;

	pNotify.hdr.hwndFrom	= pCtrl->GetSafeHwnd();
	pNotify.hdr.idFrom		= pCtrl->GetDlgCtrlID();
	pNotify.hdr.code		= WVN_CONTEXTMENU;
	pNotify.dwMenuID		= dwID;
	pNotify.ptMouse			= *ppt;
	pNotify.pCmdTarget		= pcmdtReserved;
	pNotify.pContext		= pdispReserved;

	LRESULT lResult = pCtrl->GetOwner()->SendMessage(
		WM_NOTIFY, (WPARAM)pNotify.hdr.idFrom, (LPARAM)&pNotify );

	if ( lResult == 1 ) return S_OK;
	pCtrl->EnterMenu( ppt );

	return S_FALSE;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::GetHostInfo(DOCHOSTUIINFO* /*pInfo*/)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_OK;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::ShowUI(DWORD /*dwID*/, LPOLEINPLACEACTIVEOBJECT /*pActiveObject*/, LPOLECOMMANDTARGET /*pCommandTarget*/, LPOLEINPLACEFRAME /*pFrame*/, LPOLEINPLACEUIWINDOW /*pDoc*/)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_FALSE;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::HideUI()
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_OK;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::UpdateUI(void)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_OK;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::EnableModeless(BOOL /*fEnable*/)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_OK;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::OnDocWindowActivate(BOOL /*fActivate*/)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_OK;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::OnFrameWindowActivate(BOOL /*fActivate*/)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_OK;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::ResizeBorder(LPCRECT /*prcBorder*/, LPOLEINPLACEUIWINDOW /*pUIWindow*/, BOOL /*fFrameWindow*/)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_OK;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::TranslateAccelerator(LPMSG /*lpMsg*/, const GUID* /*pguidCmdGroup*/, DWORD /*nCmdID*/)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_FALSE;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::GetOptionKeyPath(LPOLESTR* /*pchKey*/, DWORD /*dwReserved*/)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_FALSE;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::GetDropTarget(LPDROPTARGET /*pDropTarget*/, LPDROPTARGET* /*ppDropTarget*/)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_FALSE;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::TranslateUrl(DWORD /*dwTranslate*/, OLECHAR* /*pchURLIn*/, OLECHAR** /*ppchURLOut*/)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_FALSE;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostUIHandler::FilterDataObject(LPDATAOBJECT /*pDataObject*/, LPDATAOBJECT* /*ppDataObject*/)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostUIHandler)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CWebCtrl::DocSite IDocHostShowUI

IMPLEMENT_UNKNOWN(CWebCtrl::DocSite, DocHostShowUI)

STDMETHODIMP CWebCtrl::DocSite::XDocHostShowUI::ShowHelp(HWND /*hwnd*/, LPOLESTR /*pszHelpFile*/, UINT /*uCommand*/, DWORD /*dwData*/, POINT /*ptMouse*/, IDispatch* /*pDispatchObjectHit*/)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostShowUI)
	/*CWebCtrl* pCtrl =*/ pThis->GetCtrl();
	return S_FALSE;
}

STDMETHODIMP CWebCtrl::DocSite::XDocHostShowUI::ShowMessage(HWND /*hwnd*/, LPOLESTR /*lpstrText*/, LPOLESTR /*lpstrCaption*/, DWORD /*dwType*/, LPOLESTR /*lpstrHelpFile*/, DWORD /*dwHelpContext*/, LRESULT* /*plResult*/)
{
	METHOD_PROLOGUE_EX_(CWebCtrl::DocSite, DocHostShowUI)
	CWebCtrl* pCtrl = pThis->GetCtrl();
	return pCtrl->m_bSandbox ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CWebCtrl::DocSite IServiceProvider

IMPLEMENT_UNKNOWN(CWebCtrl::DocSite, ServiceProvider)

STDMETHODIMP CWebCtrl::DocSite::XServiceProvider::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
	METHOD_PROLOGUE(CWebCtrl::DocSite, ServiceProvider)
	CWebCtrl* pCtrl = pThis->GetCtrl();

	if ( guidService == SID_SInternetSecurityManager && pCtrl->m_bSandbox )
	{
		if ( riid == IID_IInternetSecurityManager )
		{
			return pThis->ExternalQueryInterface( &riid, ppv );
		}
		else
		{
			return E_NOINTERFACE;
		}
	}
	else
	{
		// return SVC_E_UNKNOWNSERVICE;
		return E_UNEXPECTED;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWebCtrl::DocSite IInternetSecurityManager

IMPLEMENT_UNKNOWN(CWebCtrl::DocSite, InternetSecurityManager)

STDMETHODIMP CWebCtrl::DocSite::XInternetSecurityManager::GetSecurityId(LPCWSTR /*pwszUrl*/, BYTE* /*pbSecurityId*/, DWORD* /*pcbSecurityId*/, DWORD_PTR /*dwReserved*/)
{
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CWebCtrl::DocSite::XInternetSecurityManager::GetSecuritySite(IInternetSecurityMgrSite** /*ppSite*/)
{
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CWebCtrl::DocSite::XInternetSecurityManager::GetZoneMappings(DWORD /*dwZone*/, IEnumString** /*ppenumString*/, DWORD /*dwFlags*/)
{
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CWebCtrl::DocSite::XInternetSecurityManager::MapUrlToZone(LPCWSTR /*pwszUrl*/, DWORD* /*pdwZone*/, DWORD /*dwFlags*/)
{
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CWebCtrl::DocSite::XInternetSecurityManager::ProcessUrlAction(LPCWSTR pwszUrl, DWORD dwAction, BYTE *pPolicy, DWORD cbPolicy, BYTE* /*pContext*/, DWORD /*cbContext*/, DWORD /*dwFlags*/, DWORD /*dwReserved*/)
{
	if ( cbPolicy != 4 ) return INET_E_DEFAULT_ACTION;
	PBOOL pBool = (PBOOL)pPolicy;

	if ( wcsncmp( pwszUrl, L"p2p-col://", 10 ) == 0 )
	{
		if (	( dwAction >= URLACTION_ACTIVEX_MIN && dwAction <= URLACTION_ACTIVEX_MAX )
			||	( dwAction >= URLACTION_SHELL_MIN && dwAction <= URLACTION_SHELL_MAX )
			||	( dwAction >= URLACTION_INFODELIVERY_MIN && dwAction <= URLACTION_INFODELIVERY_MAX )
			||	( dwAction >= URLACTION_HTML_MIN && dwAction <= URLACTION_HTML_MAX )
			||	( dwAction >= URLACTION_DOWNLOAD_MIN && dwAction <= URLACTION_DOWNLOAD_MAX ) )
		{
			*pBool = URLPOLICY_DISALLOW;
			return S_OK;
		}
		else if ( dwAction >= URLACTION_SCRIPT_MIN && dwAction <= URLACTION_SCRIPT_MAX )
		{
			*pBool = ( dwAction == URLACTION_SCRIPT_RUN ) ? URLPOLICY_ALLOW : URLPOLICY_DISALLOW;
			return S_OK;
		}
	}
	else
	{
		*pBool = URLPOLICY_DISALLOW;
		return S_OK;
	}

	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CWebCtrl::DocSite::XInternetSecurityManager::QueryCustomPolicy(LPCWSTR /*pwszUrl*/, REFGUID /*guidKey*/, BYTE** /*ppPolicy*/, DWORD* /*pcbPolicy*/, BYTE* /*pContext*/, DWORD /*cbContext*/, DWORD /*dwReserved*/)
{
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CWebCtrl::DocSite::XInternetSecurityManager::SetSecuritySite(IInternetSecurityMgrSite* /*pSite*/)
{
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CWebCtrl::DocSite::XInternetSecurityManager::SetZoneMapping(DWORD /*dwZone*/, LPCWSTR /*lpszPattern*/, DWORD /*dwFlags*/)
{
	return INET_E_DEFAULT_ACTION;
}
