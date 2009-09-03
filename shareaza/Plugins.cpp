//
// Plugins.cpp
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
#include "Settings.h"
#include "Plugins.h"
#include "Application.h"
#include "CtrlCoolBar.h"
#include "WndPlugin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CPlugins Plugins;


//////////////////////////////////////////////////////////////////////
// CPlugins construction

CPlugins::CPlugins() :
	m_nCommandID( ID_PLUGIN_FIRST )
{
}

CPlugins::~CPlugins()
{
	Clear();
}

void CPlugins::Register()
{
	CList< HINSTANCE > oModules; // Cache

	CFileFind finder;
	BOOL bWorking = finder.FindFile( Settings.General.Path + _T("\\*.dll") );
	while ( bWorking )
	{
		bWorking = finder.FindNextFile();
		CString sDllPath = finder.GetFilePath().MakeLower();

		if ( sDllPath.Find( _T("razawebhook.dll") ) != -1 )
			// Skip RazaWebHook.dll
			continue;

		if ( HINSTANCE hDll = LoadLibrary( sDllPath ) )
		{
			HRESULT hr = S_FALSE;

			HRESULT (WINAPI *pfnDllInstall)(BOOL bInstall, LPCWSTR pszCmdLine);
			(FARPROC&)pfnDllInstall = GetProcAddress( hDll, "DllInstall" );
			if ( pfnDllInstall )
			{
				hr = pfnDllInstall( TRUE, L"user" );
			}
			else
			{
				HRESULT (WINAPI *pfnDllRegisterServer)(void);
				(FARPROC&)pfnDllRegisterServer = GetProcAddress( hDll, "DllRegisterServer" );
				if ( pfnDllRegisterServer )
				{
					hr = pfnDllRegisterServer();
				}
			}

			if ( hr == S_OK )
				theApp.Message( MSG_NOTICE, _T("Registered plugin: %s"), sDllPath );
			else if ( FAILED( hr ) )
				theApp.Message( MSG_ERROR, _T("Failed to register plugin: %s : 0x%08x"), sDllPath, hr );

			oModules.AddTail( hDll );
		}
	}

	for ( POSITION pos = oModules.GetHeadPosition(); pos; )
		FreeLibrary( oModules.GetNext( pos ) );
}

//////////////////////////////////////////////////////////////////////
// CPlugins enumerate

void CPlugins::Enumerate()
{
	HUSKEY hKey;

	if ( SHRegOpenUSKey( _T("Software\\Shareaza\\Shareaza\\Plugins\\General"),
		KEY_READ, NULL, &hKey, FALSE ) != ERROR_SUCCESS ) return;

	for ( DWORD nKey = 0 ; ; nKey++ )
	{
		TCHAR szName[128], szCLSID[64];
		DWORD dwType, dwName = 128, dwCLSID = 64 * sizeof(TCHAR);

		if ( SHRegEnumUSValue( hKey, nKey, szName, &dwName, &dwType,
			(LPBYTE)szCLSID, &dwCLSID, SHREGENUM_DEFAULT ) != ERROR_SUCCESS ) break;

		if ( dwType != REG_SZ ) continue;
		szCLSID[ 38 ] = 0;

		CLSID pCLSID;
		if ( ! Hashes::fromGuid( szCLSID, &pCLSID ) ) continue;

		for ( POSITION pos = GetIterator() ; pos ; )
		{
			if ( GetNext( pos )->m_pCLSID == pCLSID )
			{
				pCLSID = GUID_NULL;
				break;
			}
		}

		if ( pCLSID == GUID_NULL ) continue;

		CPlugin* pPlugin = new CPlugin( pCLSID, szName );
		m_pList.AddTail( pPlugin );

		pPlugin->StartIfEnabled();
	}

	SHRegCloseUSKey( hKey );
}

//////////////////////////////////////////////////////////////////////
// CPlugins clear

void CPlugins::Clear()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		delete GetNext( pos );
	}

	m_pList.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CPlugins CLSID helpers

BOOL CPlugins::LookupCLSID(LPCTSTR pszGroup, LPCTSTR pszKey, CLSID& pCLSID) const
{
	CString strCLSID = theApp.GetProfileString(
		CString( _T("Plugins\\") ) + pszGroup, pszKey, _T("") );
	return ! strCLSID.IsEmpty() &&
		Hashes::fromGuid( strCLSID, &pCLSID ) &&
		LookupEnable( pCLSID, pszKey );
}

BOOL CPlugins::LookupEnable(REFCLSID pCLSID, LPCTSTR pszExt) const
{
	HKEY hPlugins = NULL;

	CString strCLSID = Hashes::toGuid( pCLSID );

	if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER,
		_T("Software\\Shareaza\\Shareaza\\Plugins"), 0, KEY_ALL_ACCESS, &hPlugins ) )
	{
		DWORD nType = REG_SZ, nValue = 0;
		if ( ERROR_SUCCESS == RegQueryValueEx( hPlugins, strCLSID, NULL, &nType, NULL, &nValue ) )
		{
			// Upgrade here; Smart upgrade doesn't work
			if ( nType == REG_DWORD )
			{
				BOOL bEnabled = theApp.GetProfileInt( _T("Plugins"), strCLSID, TRUE );
				RegCloseKey( hPlugins );
				theApp.WriteProfileString( _T("Plugins"), strCLSID, bEnabled ? _T("") : _T("-") );
				return bEnabled;
			}
		}
		RegCloseKey( hPlugins );
	}

	CString strExtensions = theApp.GetProfileString( _T("Plugins"), strCLSID, _T("") );

	if ( strExtensions.IsEmpty() )
		return TRUE;
	else if ( strExtensions == _T("-") ) // for plugins without associations
		return FALSE;
	else if ( strExtensions.Left( 1 ) == _T("-") && strExtensions.GetLength() > 1 )
		strExtensions = strExtensions.Mid( 1 );

	if ( pszExt ) // Checking only a certain extension
	{
		CString strToFind;
		strToFind.Format( _T("|%s|"), pszExt );
		return strExtensions.Find( strToFind ) != -1;
	}

	// For Settings page
	CStringArray oTokens;
	Split( strExtensions, _T('|'), oTokens );
	INT_PTR nTotal = oTokens.GetCount();
	INT_PTR nChecked = 0;

	for ( INT_PTR nToken = 0 ; nToken < nTotal ; nToken++ )
	{
		CString strToken = oTokens.GetAt( nToken );
		if ( strToken.Left( 1 ) != _T("-") ) nChecked++;
	}

	if ( nChecked == 0 ) return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CPlugins skin changed event

void CPlugins::OnSkinChanged()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pPlugin ) pPlugin->m_pPlugin->OnSkinChanged();
	}
}

void CPlugins::InsertCommands()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pCommand ) pPlugin->m_pCommand->InsertCommands();
	}
}

//////////////////////////////////////////////////////////////////////
// CPlugins command ID registration

void CPlugins::RegisterCommands()
{
	m_nCommandID = ID_PLUGIN_FIRST;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );
		if ( pPlugin->m_pCommand ) pPlugin->m_pCommand->RegisterCommands();
	}
}

UINT CPlugins::GetCommandID()
{
	return m_nCommandID++;
}

//////////////////////////////////////////////////////////////////////
// CPlugins command handling

BOOL CPlugins::OnUpdate(CChildWnd* pActiveWnd, CCmdUI* pCmdUI)
{
	UINT nCommandID		= pCmdUI->m_nID;
	TRISTATE bVisible	= TRI_TRUE;
	TRISTATE bEnabled	= TRI_TRUE;
	TRISTATE bChecked	= TRI_UNKNOWN;

	CCoolBarItem* pCoolUI = CCoolBarItem::FromCmdUI( pCmdUI );

	if ( pActiveWnd != NULL && pActiveWnd->IsKindOf( RUNTIME_CLASS(CPluginWnd) ) )
	{
		CPluginWnd* pPluginWnd = (CPluginWnd*)pActiveWnd;

		if ( pPluginWnd->m_pOwner )
		{
			if ( pPluginWnd->m_pOwner->OnUpdate( nCommandID, &bVisible, &bEnabled, &bChecked ) == S_OK )
			{
				if ( bVisible != TRI_UNKNOWN && pCoolUI != NULL )
					pCoolUI->Show( bVisible == TRI_TRUE );
				if ( bEnabled != TRI_UNKNOWN )
					pCmdUI->Enable( bEnabled == TRI_TRUE );
				if ( bChecked != TRI_UNKNOWN )
					pCmdUI->SetCheck( bChecked == TRI_TRUE );

				return TRUE;
			}
		}
	}

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pCommand )
		{
			if ( pPlugin->m_pCommand->OnUpdate( nCommandID, &bVisible, &bEnabled, &bChecked ) == S_OK )
			{
				if ( bVisible != TRI_UNKNOWN && pCoolUI != NULL )
					pCoolUI->Show( bVisible == TRI_TRUE );
				if ( bEnabled != TRI_UNKNOWN )
					pCmdUI->Enable( bEnabled == TRI_TRUE );
				if ( bChecked != TRI_UNKNOWN )
					pCmdUI->SetCheck( bChecked == TRI_TRUE );

				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CPlugins::OnCommand(CChildWnd* pActiveWnd, UINT nCommandID)
{
	if ( pActiveWnd != NULL && pActiveWnd->IsKindOf( RUNTIME_CLASS(CPluginWnd) ) )
	{
		CPluginWnd* pPluginWnd = (CPluginWnd*)pActiveWnd;

		if ( pPluginWnd->m_pOwner )
		{
			if ( pPluginWnd->m_pOwner->OnCommand( nCommandID ) == S_OK ) return TRUE;
		}
	}

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pCommand )
		{
			if ( pPlugin->m_pCommand->OnCommand( nCommandID ) == S_OK ) return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CPlugins file execution events

BOOL CPlugins::OnExecuteFile(LPCTSTR pszFile, BOOL bUseImageViewer)
{
	COleVariant vFile( pszFile );
	vFile.ChangeType( VT_BSTR );

	CPlugin* pImageViewer = NULL;
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pExecute )
		{
			if ( pPlugin->m_sName == _T("Shareaza Image Viewer") )
			{
				pImageViewer = pPlugin;
				continue;
			}
			if ( pPlugin->m_pExecute->OnExecute( vFile.bstrVal ) == S_OK )
				return TRUE;
		}
	}
	if ( bUseImageViewer && pImageViewer )
	{
		return ( pImageViewer->m_pExecute->OnExecute( vFile.bstrVal ) == S_OK );
	}

	return FALSE;
}

BOOL CPlugins::OnEnqueueFile(LPCTSTR pszFile)
{
	COleVariant vFile( pszFile );
	vFile.ChangeType( VT_BSTR );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );

		if ( pPlugin->m_pExecute )
		{
			if ( pPlugin->m_pExecute->OnEnqueue( vFile.bstrVal ) == S_OK )
				return TRUE;
		}
	}

	return FALSE;
}

CPlugin* CPlugins::Find(REFCLSID pCLSID) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );
		if ( pPlugin->m_pCLSID == pCLSID )
			return pPlugin;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CPlugin construction

CPlugin::CPlugin(REFCLSID pCLSID, LPCTSTR pszName) :
	m_pCLSID( pCLSID ),
	m_sName( pszName ),
	m_nCapabilities( 0 ),
	m_pPlugin( NULL ),
	m_pCommand( NULL ),
	m_pExecute( NULL )
{
}

CPlugin::~CPlugin()
{
	Stop();
}

//////////////////////////////////////////////////////////////////////
// CPlugin start / stop operations

BOOL CPlugin::Start()
{
	if ( m_pPlugin != NULL ) return FALSE;

	HRESULT hResult = CoCreateInstance( m_pCLSID, NULL, CLSCTX_ALL,
		IID_IGeneralPlugin, (void**)&m_pPlugin );

	if ( FAILED( hResult ) || m_pPlugin == NULL )
	{
		m_pPlugin = NULL;
		return FALSE;
	}

	CComPtr< IApplication > pApplication;
	if ( SUCCEEDED( CApplication::GetApp( &pApplication ) ) )
		m_pPlugin->SetApplication( pApplication );

	m_nCapabilities = 0;
	m_pPlugin->QueryCapabilities( &m_nCapabilities );

	m_pPlugin->QueryInterface( IID_ICommandPlugin, (void**)&m_pCommand );

	m_pPlugin->QueryInterface( IID_IExecutePlugin, (void**)&m_pExecute );

	return TRUE;
}

void CPlugin::Stop()
{
	if ( m_pExecute != NULL )
	{
		m_pExecute->Release();
		m_pExecute = NULL;
	}

	if ( m_pCommand != NULL )
	{
		m_pCommand->Release();
		m_pCommand = NULL;
	}

	if ( m_pPlugin != NULL )
	{
		m_pPlugin->Release();
		m_pPlugin = NULL;
	}
}

BOOL CPlugin::StartIfEnabled()
{
	if ( Plugins.LookupEnable( m_pCLSID ) )
		return Start();
	else
		return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CPlugin CLSID helper

CString CPlugin::GetStringCLSID() const
{
	return Hashes::toGuid( m_pCLSID );
}

//////////////////////////////////////////////////////////////////////
// CPlugin icon helper

HICON CPlugin::LookupIcon() const
{
	return LoadCLSIDIcon( GetStringCLSID() );
}
