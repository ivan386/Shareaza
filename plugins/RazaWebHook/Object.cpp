//
// Object.cpp : Implementation of CRazaWebHook
//
// Copyright (c) Shareaza Development Team, 2008-2010.
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

#include "stdafx.h"
#include "Object.h"

#define INITGUID
#include "RazaWebHook_i.c"

// CRazaWebHook

CRazaWebHook::CRazaWebHook() :
	m_dwCookie( 0 )
{
}

HRESULT CRazaWebHook::FinalConstruct()
{
	return IsEnabled() ? S_OK : E_FAIL;
}

void CRazaWebHook::FinalRelease()
{
	Disconnect();
	m_spUnkSite.Release();
}

bool CRazaWebHook::IsEnabled() const
{
	DWORD dwWebHookEnable = FALSE;
	DWORD dwLength = sizeof( dwWebHookEnable );
	SHRegGetUSValue( _T("Software\\Shareaza\\Shareaza\\Downloads"),
		_T("WebHookEnable"), NULL, &dwWebHookEnable,
		&dwLength, FALSE, &dwWebHookEnable, sizeof( dwWebHookEnable ) );
	return ( dwWebHookEnable != FALSE );
}

bool CRazaWebHook::IsHooked(const CString& sExt) const
{
	CString sWebHookExtensions;
	DWORD dwLength = 1024;
	SHRegGetUSValue( _T("Software\\Shareaza\\Shareaza\\Downloads"),
		_T("WebHookExtensions"), NULL, sWebHookExtensions.GetBuffer( dwLength ),
		&dwLength, FALSE, _T(""), sizeof( TCHAR ) );
	sWebHookExtensions.ReleaseBuffer();
	sWebHookExtensions.MakeLower();
	return ( sWebHookExtensions.Find( CString( _T("|") ) + sExt + _T("|") ) != -1 );
}

void CRazaWebHook::Connect()
{
	if ( m_spUnkSite )
	{
		CComQIPtr< IConnectionPointContainer > pContainer( m_spUnkSite );
		if ( pContainer )
		{
			CComPtr< IConnectionPoint > pPoint;
			if ( SUCCEEDED( pContainer->FindConnectionPoint( DIID_DWebBrowserEvents2,
				&pPoint ) ) )
			{
				pPoint->Advise( static_cast< IDispatch* >( this ), &m_dwCookie );
			}
		}
	}
}

void CRazaWebHook::Disconnect()
{
	if ( m_spUnkSite )
	{
		CComQIPtr< IConnectionPointContainer > pContainer( m_spUnkSite );
		if ( pContainer )
		{
			CComPtr< IConnectionPoint > pPoint;
			if ( SUCCEEDED( pContainer->FindConnectionPoint( DIID_DWebBrowserEvents2,
				&pPoint ) ) )
			{
				pPoint->Unadvise( m_dwCookie );
				m_dwCookie = 0;
			}
		}
	}
}

void CRazaWebHook::AddLink(const CString& sURL)
{
	ShellExecute( NULL, NULL, CString( _T("shareaza://url:") ) + sURL,
		NULL, NULL, SW_SHOWDEFAULT );
}

STDMETHODIMP CRazaWebHook::Invoke(
	/* [in] */ DISPID dispIdMember,
	/* [in] */ REFIID riid,
	/* [in] */ LCID lcid,
	/* [in] */ WORD wFlags,
	/* [out][in] */ DISPPARAMS *pDispParams,
	/* [out] */ VARIANT* pVarResult,
	/* [out] */ EXCEPINFO* pExcepInfo,
	/* [out] */ UINT* puArgErr)
{
	if ( IsEnabled() )
	{
		switch ( dispIdMember )
		{
		case DISPID_BEFORENAVIGATE2:
			{
				ATLASSERT( pDispParams->cArgs == 7 );
				//ATLASSERT( pDispParams->rgvarg[ 6 ].vt == VT_DISPATCH );
				//CComQIPtr< IWebBrowser2 > pIWebBrowser2( pDispParams->rgvarg[ 6 ].pdispVal );
				ATLASSERT( pDispParams->rgvarg[ 5 ].vt == ( VT_BYREF | VT_VARIANT ) );
				ATLASSERT( pDispParams->rgvarg[ 5 ].pvarVal->vt == VT_BSTR );
				BSTR& bstrURL = pDispParams->rgvarg[ 5 ].pvarVal->bstrVal;

				ATLTRACE( _T("[Raza Web Hook] Before navigate: %s\n"), bstrURL );
				m_sURL.Empty();

				CString sURL( bstrURL );
				int nName = sURL.ReverseFind( _T('/') );
				CString sName = sURL.Mid( nName + 1 ).SpanExcluding( _T("?") );
				int nExt = sName.ReverseFind( _T('.') );
				CString sExt;
				if ( nExt != -1 )
					sExt = sName.Mid( nExt + 1 ).MakeLower();
				if ( ! sExt.IsEmpty() && IsHooked( sExt ) )
				{
					m_sURL = sURL;
				}
			}
			break;

		case DISPID_NAVIGATECOMPLETE2:
			m_sURL.Empty();
			break;

		case DISPID_FILEDOWNLOAD:
			{
				ATLASSERT( pDispParams->cArgs == 2 );
				ATLASSERT( pDispParams->rgvarg[ 0 ].vt == ( VT_BYREF | VT_BOOL ) );
				VARIANT_BOOL*& pCancel = pDispParams->rgvarg[ 0 ].pboolVal;

				if ( ! m_sURL.IsEmpty() )
				{
					ATLTRACE( "[Raza Web Hook] File download: %s\n", (LPCSTR)CT2A( m_sURL ) );
					AddLink( m_sURL );
					m_sURL.Empty();
					*pCancel = VARIANT_TRUE;
				}
			}
			break;
		}
	}

	return IRazaWebHookDispatchImpl::Invoke( dispIdMember, riid, lcid, wFlags,
		pDispParams, pVarResult, pExcepInfo, puArgErr );
}

STDMETHODIMP CRazaWebHook::SetSite(
	/* [in] */ IUnknown* pUnkSite)
{
	Disconnect();

	HRESULT hr = IObjectWithSiteImpl< CRazaWebHook >::SetSite( pUnkSite );

	Connect();

	return hr;
}

STDMETHODIMP CRazaWebHook::AddLink(
	/* [in] */ VARIANT oLink)
{
	if ( oLink.vt == VT_BSTR )
	{
		ATLTRACE( _T("[Raza Web Hook] Menu call: %s\n"), (LPCTSTR)oLink.bstrVal );
		AddLink( CString( oLink.bstrVal ) );
	}
	return S_OK;
}
