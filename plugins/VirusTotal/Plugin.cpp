//
// Plugin.cpp : Implementation of CPlugin
//
// Copyright (c) Nikolay Raspopov, 2009-2014.
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
#include "Plugin.h"

static const LPCWSTR VIRUSTOTAL_CHECK	= L"&VirusTotal Check";
static const LPCWSTR VIRUSTOTAL_HOME	= L"http://www.virustotal.com";
//static const LPCWSTR VIRUSTOTAL_URL	= L"http://www.virustotal.com/vt/en/consultamd5";
static const LPCWSTR VIRUSTOTAL_URL		= L"http://www.virustotal.com/latest-report.html?resource=";

void CPlugin::InsertCommand(ISMenu* pMenu, int nPos, UINT nID, LPCWSTR szItem)
{
	// Check for existing menu item
	CComPtr< ISMenu > pCheckMenu;
	if ( SUCCEEDED( pMenu->get_Item( CComVariant( nID ), &pCheckMenu ) ) && ! pCheckMenu )
	{
		// Insert new menu item
		pMenu->InsertCommand( nPos, nID, CComBSTR( szItem ), NULL );
	}
}

HRESULT CPlugin::Request(LPCWSTR szHash)
{
	ATLTRACE( "VirusTotal : Request( %s )\n", (LPCSTR)CW2A( szHash ) );

	ShellExecute( NULL, NULL, CString( VIRUSTOTAL_URL ) + szHash, NULL, NULL, SW_SHOWDEFAULT );

	return S_OK;
/*
	CComPtr< IWebBrowserApp > pWebBrowserApp;
	HRESULT hr = pWebBrowserApp.CoCreateInstance( CLSID_InternetExplorer );
	if ( SUCCEEDED( hr ) )
	{
		CComSafeArray< BYTE > pPost;
		pPost.Create();
		pPost.Add( 5, (LPBYTE)"hash=" );
		pPost.Add( lstrlenW( szHash ), (LPBYTE)(LPCSTR)CW2A( szHash ) );
		CComBSTR bstrURL( VIRUSTOTAL_URL );
		CComVariant vFlags( 0 );
		CComVariant vFrame( CComBSTR( L"" ) );
		VARIANT vPost;
		VariantInit( &vPost );
		vPost.vt = VT_ARRAY | VT_UI1;
		vPost.parray = pPost;
		CComVariant vHeaders( CComBSTR( L"Content-Type: application/x-www-form-urlencoded\r\n") );
		hr = pWebBrowserApp->Navigate( bstrURL, &vFlags, &vFrame, &vPost, &vHeaders );
		if ( SUCCEEDED( hr ) )
		{
			pWebBrowserApp->put_Visible( VARIANT_TRUE );
		}
		else
		{
			pWebBrowserApp->Quit();

			ATLTRACE( "VirusTotal : Request() : Internet Explorer navigate error: 0x%08x\n", hr );
		}
	}
	else
		ATLTRACE( "VirusTotal : Request() : Create Internet Explorer instance error: 0x%08x\n", hr );
	return hr;
*/
}

// IGeneralPlugin

STDMETHODIMP CPlugin::SetApplication( 
	/* [in] */ IApplication __RPC_FAR *pApplication)
{
	if ( ! pApplication )
		return E_POINTER;

	m_pApplication = pApplication;
	return m_pApplication->get_UserInterface( &m_pUserInterface );
}

STDMETHODIMP CPlugin::QueryCapabilities(
	/* [in] */ DWORD __RPC_FAR *pnCaps)
{
	if ( ! pnCaps )
		return E_POINTER;

	return S_OK;
}

STDMETHODIMP CPlugin::Configure()
{
	ShellExecute( NULL, NULL, VIRUSTOTAL_HOME, NULL, NULL, SW_SHOWDEFAULT );

	return S_OK;
}

STDMETHODIMP CPlugin::OnSkinChanged()
{
	// Recreate lost menu items
	return InsertCommands();
}

// ICommandPlugin

STDMETHODIMP CPlugin::RegisterCommands()
{
	if ( ! m_pUserInterface )
		return E_UNEXPECTED;

	HRESULT hr = m_pUserInterface->RegisterCommand( CComBSTR( L"VirusTotalPlugin_Check" ),
		LoadIcon( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_ICON ) ),
		&m_nCmdCheck );
	if ( SUCCEEDED( hr ) )
	{
		return S_OK;
	}

	return E_FAIL;
}

STDMETHODIMP CPlugin::InsertCommands()
{
	if ( ! m_pUserInterface )
		return E_UNEXPECTED;

	// Insert new items at the top of "Web Services" submenu
	UINT nMenuID = 0;
	if ( FAILED( m_pUserInterface->NameToID( CComBSTR( L"ID_WEBSERVICES_LIST" ), &nMenuID ) ) || ! nMenuID )
	{
		return E_UNEXPECTED;
	}
	// but preferably after "MusicBrainz" item
	UINT nAfterItemID = 0;
	m_pUserInterface->NameToID( CComBSTR( L"ID_WEBSERVICES_MUSICBRAINZ" ), &nAfterItemID );

	CComPtr< ISMenu > pSearchMenu;
	if ( SUCCEEDED( m_pUserInterface->GetMenu( CComBSTR( L"CSearchWnd" ), VARIANT_FALSE, &pSearchMenu ) ) && pSearchMenu )
	{
		CComPtr< ISMenu > pWebMenu;
		if ( SUCCEEDED( pSearchMenu->get_Item( CComVariant( nMenuID ), &pWebMenu ) ) && pWebMenu )
		{
			InsertCommand( pWebMenu, 0, m_nCmdCheck, VIRUSTOTAL_CHECK );
		}
	}

	CComPtr< ISMenu > pBrowseHostMenu;
	if ( SUCCEEDED( m_pUserInterface->GetMenu( CComBSTR( L"CBrowseHostWnd" ), VARIANT_FALSE, &pBrowseHostMenu ) ) && pBrowseHostMenu )
	{
		CComPtr< ISMenu > pWebMenu;
		if ( SUCCEEDED( pBrowseHostMenu->get_Item( CComVariant( nMenuID ), &pWebMenu ) ) && pWebMenu )
		{
			InsertCommand( pWebMenu, 0, m_nCmdCheck, VIRUSTOTAL_CHECK );
		}
	}

	CComPtr< ISMenu > pHitMonitorMenu;
	if ( SUCCEEDED( m_pUserInterface->GetMenu( CComBSTR( L"CHitMonitorWnd" ), VARIANT_FALSE, &pHitMonitorMenu ) ) && pHitMonitorMenu )
	{
		CComPtr< ISMenu > pWebMenu;
		if ( SUCCEEDED( pHitMonitorMenu->get_Item( CComVariant( nMenuID ), &pWebMenu ) ) && pWebMenu )
		{
			InsertCommand( pWebMenu, 0, m_nCmdCheck, VIRUSTOTAL_CHECK );
		}
	}

	CComPtr< ISMenu > pFileMenu;
	if ( SUCCEEDED( m_pUserInterface->GetMenu( CComBSTR( L"CLibraryFileView.Physical" ), VARIANT_FALSE, &pFileMenu ) ) && pFileMenu )
	{
		CComPtr< ISMenu > pWebMenu;
		if ( SUCCEEDED( pFileMenu->get_Item( CComVariant( nMenuID ), &pWebMenu ) ) && pWebMenu )
		{
			LONG nPos = -1;
			CComPtr< ISMenu > pAfterMenu;
			if ( nAfterItemID && SUCCEEDED( pWebMenu->get_Item( CComVariant( nAfterItemID ), &pAfterMenu ) ) && pAfterMenu )
			{
				pAfterMenu->get_Position( &nPos );
			}
			InsertCommand( pWebMenu, nPos + 1, m_nCmdCheck, VIRUSTOTAL_CHECK );
		}
	}

	CComPtr< ISMenu > pVirtualMenu;
	if ( SUCCEEDED( m_pUserInterface->GetMenu( CComBSTR( L"CLibraryFileView.Virtual" ), VARIANT_FALSE, &pVirtualMenu ) ) && pVirtualMenu )
	{
		CComPtr< ISMenu > pWebMenu;
		if ( SUCCEEDED( pVirtualMenu->get_Item( CComVariant( nMenuID ), &pWebMenu ) ) && pWebMenu )
		{
			InsertCommand( pWebMenu, 0, m_nCmdCheck, VIRUSTOTAL_CHECK );
		}
	}

	CComPtr< ISMenu > pListMenu;
	if ( SUCCEEDED( m_pUserInterface->GetMenu( CComBSTR( L"WebServices.List.Menu" ), VARIANT_FALSE, &pListMenu ) ) && pListMenu )
	{
		LONG nPos = -1;
		CComPtr< ISMenu > pAfterMenu;
		if ( nAfterItemID && SUCCEEDED( pListMenu->get_Item( CComVariant( nAfterItemID ), &pAfterMenu ) ) && pAfterMenu )
		{
			pAfterMenu->get_Position( &nPos );
		}
		InsertCommand( pListMenu, nPos + 1, m_nCmdCheck, VIRUSTOTAL_CHECK );
	}

	return S_OK;
}

STDMETHODIMP CPlugin::OnUpdate( 
    /* [in] */ UINT nCommandID,
    /* [out][in] */ TRISTATE __RPC_FAR *pbVisible,
    /* [out][in] */ TRISTATE __RPC_FAR *pbEnabled,
    /* [out][in] */ TRISTATE __RPC_FAR *pbChecked)
{
	if ( ! pbVisible || ! pbEnabled || ! pbChecked )
		return E_POINTER;

	if ( ! m_pUserInterface )
		return E_UNEXPECTED;

	if ( nCommandID == m_nCmdCheck )
	{
		*pbEnabled = TRI_FALSE;
		*pbVisible = TRI_TRUE;
		*pbChecked = TRI_UNKNOWN;

		CComPtr< IGenericView > pGenericView;
		HRESULT hr = m_pUserInterface->get_ActiveView( &pGenericView );
		if ( SUCCEEDED( hr ) && pGenericView )
		{
			LONG nCount = 0;
			hr = pGenericView->get_Count( &nCount );
			if ( SUCCEEDED( hr ) && nCount )
			{
				*pbEnabled = TRI_TRUE;
			}
		}
		return S_OK;
	}

	return S_FALSE;
}

STDMETHODIMP CPlugin::OnCommand( 
	/* [in] */ UINT nCommandID)
{
	ATLTRACE( "VirusTotal : OnCommand( %d )\n", nCommandID );

	if ( ! m_pUserInterface )
	{
		ATLTRACE( "VirusTotal : OnCommand : No user interface.\n" );
		return E_UNEXPECTED;
	}

	if ( nCommandID == m_nCmdCheck )
	{
		CComPtr< IGenericView > pGenericView;
		HRESULT hr = m_pUserInterface->get_ActiveView( &pGenericView );
		if ( SUCCEEDED( hr ) && pGenericView )
		{
			LONG nCount = 0;
			hr = pGenericView->get_Count( &nCount );
			if ( SUCCEEDED( hr ) && nCount )
			{
				for ( LONG i = 0; i < nCount; ++i )
				{
					CComVariant pItem;
					hr = pGenericView->get_Item( CComVariant( i ), &pItem );
					if ( FAILED( hr ) )
					{
						ATLTRACE( "VirusTotal : OnCommand() : Get item error: 0x%08x\n", hr );
						break;
					}

					CComBSTR pSHA1;
					CComBSTR pMD5;
					if ( pItem.vt == VT_I4 )
					{
						CComPtr< ILibrary > pLibrary;
						hr = m_pApplication->get_Library( &pLibrary );
						if ( SUCCEEDED( hr ) && pLibrary )
						{
							CComPtr< ILibraryFile > pLibraryFile;
							hr = pLibrary->FindByIndex( pItem.lVal, &pLibraryFile );
							if ( SUCCEEDED( hr ) && pLibraryFile )
							{
								pLibraryFile->get_Hash( URN_MD5,  ENCODING_BASE16, &pMD5 );
								pLibraryFile->get_Hash( URN_SHA1, ENCODING_BASE16, &pSHA1 );
							}
							else
								ATLTRACE( "VirusTotal : OnCommand() : Find file by index error: 0x%08x\n", hr );
						}
						else
							ATLTRACE( "VirusTotal : OnCommand() : Get Library error: 0x%08x\n", hr );

					}
					else if ( pItem.vt == VT_DISPATCH )
					{
						CComQIPtr< IShareazaFile > pShareazaFile( pItem.pdispVal );
						if ( pShareazaFile )
						{
							pShareazaFile->get_Hash( URN_MD5,  ENCODING_BASE16, &pMD5 );
							pShareazaFile->get_Hash( URN_SHA1, ENCODING_BASE16, &pSHA1 );
						}
					}
					else
						ATLTRACE( "VirusTotal : OnCommand() : Unknown item data.\n" );

					if ( pMD5.Length() )
					{
						Request( pMD5 );
					}
					else if ( pSHA1.Length() )
					{
						Request( pSHA1 );
					}
					else
						ATLTRACE( "VirusTotal : OnCommand() : No compatible hashes found.\n" );
				}
				return S_OK;
			}
			else
				ATLTRACE( "VirusTotal : OnCommand() : No files selected: 0x%08x\n", hr );
		}
		else
			ATLTRACE( "VirusTotal : OnCommand() : Active view get error: 0x%08x\n", hr );
	}

	return S_FALSE;
}
