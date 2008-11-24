// Object.cpp : Implementation of CRazaWebHook

#include "stdafx.h"
#include "Object.h"

// CRazaWebHook

CRazaWebHook::CRazaWebHook()
{
	m_pUnkMarshaler = NULL;
}

STDMETHODIMP CRazaWebHook::SetSite(
	/* [in] */ IUnknown* pUnkSite)
{
	HRESULT hr = IObjectWithSiteImpl< CRazaWebHook >::SetSite( pUnkSite );
	if ( SUCCEEDED( hr ) && m_spUnkSite )
	{
		/*HRESULT hr;
		CComQIPtr< IConnectionPointContainer > pContainer( m_spUnkSite );
		ATLASSERT( pContainer );
		if ( pContainer )
		{
			CComPtr< IConnectionPoint > pPoint;
			hr = pContainer->FindConnectionPoint( DIID_DWebBrowserEvents2, &pPoint );
			ATLASSERT( SUCCEEDED( hr ) );
			if ( SUCCEEDED( hr ) )
			{
				DWORD cookie = 0;
				hr = pPoint->Advise( GetControllingUnknown(), &cookie );
				ATLASSERT( SUCCEEDED( hr ) );
				if ( SUCCEEDED( hr ) )
				{
					// Ok
				}
			}
		}*/
	}
	return S_OK;
}

STDMETHODIMP CRazaWebHook::AddLink(
	/* [in] */ VARIANT oLink)
{
	if ( oLink.vt == VT_BSTR )
	{
		CString sURL( oLink.bstrVal );
		ATLTRACE( _T("[Raza Web Hook] Got: %s\n"), (LPCTSTR)sURL );

		if ( StrCmpNI( sURL, _T("magnet:"), 7 ) != 0 )
		{
			// Common link
			int nName = sURL.ReverseFind( _T('/') );
			CString sName = sURL.Mid( nName + 1 ).SpanExcluding( _T("?") );
			sURL.Replace( _T("?"), _T("%3f") ); // TODO: May be URLEncode?
			sURL.Replace( _T("&"), _T("%26") );
			sURL = CString( _T("magnet:?xs=") ) + sURL;
			if ( ! sName.IsEmpty() )
				sURL += CString( _T("&dn=") ) + sName;
			ShellExecute( NULL, NULL, sURL, NULL, NULL, SW_SHOWDEFAULT );
		}
		else
			// Magnet link
			ShellExecute( NULL, NULL, sURL, NULL, NULL, SW_SHOWDEFAULT );

/*		DWORD hInstance = 0;
		UINT uiResult = DdeInitialize( &hInstance, DDECallback, APPCLASS_STANDARD |
			APPCMD_CLIENTONLY | CBF_FAIL_ALLSVRXACTIONS, 0 );
		if ( uiResult == DMLERR_NO_ERROR )
		{
			HSZ hszService = DdeCreateStringHandle( hInstance,
				(LPCTSTR)_T("Shareaza"), CP_WINUNICODE );
			HSZ hszTopic = DdeCreateStringHandle( hInstance,
				(LPCTSTR)_T("URL"), CP_WINUNICODE );

			HINSTANCE hShareaza = NULL;
			for ( int i = 0; i < 20; ++i ) // ~20 sec
			{
				if ( HCONV hConnect = DdeConnect( hInstance, hszService, hszTopic, NULL ) )
				{
					if ( HDDEDATA pResult = DdeClientTransaction( (LPBYTE)oLink.bstrVal,
						( SysStringLen( oLink.bstrVal ) + 1 ) * sizeof( WCHAR ), hConnect,
						0, 0, XTYP_EXECUTE, 5000, NULL ) ) // ~5 sec
					{
						// Ok
						break;
					}
					else
						ATLTRACE( _T("[Raza Web Hook] DdeClientTransaction error: 0x%08x\n"), DdeGetLastError( hInstance ) );

					if ( ! DdeDisconnect( hConnect ) )
						ATLTRACE( _T("[Raza Web Hook] DdeDisconnect error: 0x%08x\n"), DdeGetLastError( hInstance ) );
				}
				else
				{
					UINT dwError = DdeGetLastError( hInstance );
					if ( dwError != DMLERR_NO_CONV_ESTABLISHED )
					{
						// Fatal error
						ATLTRACE( _T("[Raza Web Hook] DdeConnect error: 0x%08x\n"), dwError );
						break;
					}
				}

				if ( (int)hShareaza <= 32 )
				{
					TCHAR szPath[ MAX_PATH ] = {};
					GetModuleFileName( _AtlBaseModule.GetModuleInstance(), szPath, MAX_PATH );
					LPTSTR c = _tcsrchr( szPath, _T('\\') );
					if ( ! c )
						break;
					lstrcpy( c + 1, _T("Shareaza.exe") );
					hShareaza = ShellExecute( NULL, NULL, szPath, _T(""),
						NULL, SW_SHOWDEFAULT );
				}
				Sleep( 500 );
			}

			if ( ! DdeFreeStringHandle( hInstance, hszTopic ) )
				ATLTRACE( _T("[Raza Web Hook] DdeFreeStringHandle error: 0x%08x\n"), DdeGetLastError( hInstance ) );
			if ( ! DdeFreeStringHandle( hInstance, hszService ) )
				ATLTRACE( _T("[Raza Web Hook] DdeFreeStringHandle error: 0x%08x\n"), DdeGetLastError( hInstance ) );
			if ( ! DdeUninitialize( hInstance ) )
				ATLTRACE( _T("[Raza Web Hook] DdeUninitialize error: 0x%08x\n"), DdeGetLastError( hInstance ) );
		}
		else
			ATLTRACE( _T("[Raza Web Hook] DdeInitialize error: 0x%08x\n"), uiResult );*/
	}
	return S_OK;
}
