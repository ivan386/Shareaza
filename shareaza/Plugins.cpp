//
// Plugins.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

CPlugins::CPlugins()
{
	m_nCommandID = ID_PLUGIN_FIRST;
}

CPlugins::~CPlugins()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CPlugins enumerate

void CPlugins::Enumerate()
{
	HKEY hKey;
	
	if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
		_T("Software\\Shareaza\\Shareaza\\Plugins\\General"),
		NULL, KEY_READ, &hKey ) != ERROR_SUCCESS ) return;
	
	for ( DWORD nKey = 0 ; ; nKey++ )
	{
		TCHAR szName[128], szCLSID[64];
		DWORD dwType, dwName = 128, dwCLSID = 64 * sizeof(TCHAR);
		
		if ( RegEnumValue( hKey, nKey, szName, &dwName, NULL, &dwType, (LPBYTE)szCLSID, &dwCLSID )
			 != ERROR_SUCCESS ) break;
		
		if ( dwType != REG_SZ ) continue;
		szCLSID[ 38 ] = 0;
		
		CLSID pCLSID;
		if ( ! GUIDX::Decode( szCLSID, &pCLSID ) ) continue;
		
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
	
	RegCloseKey( hKey );
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

BOOL CPlugins::LookupCLSID(LPCTSTR pszGroup, LPCTSTR pszKey, CLSID& pCLSID, BOOL bEnableDefault)
{
	DWORD dwType, dwCLSID;
	TCHAR szCLSID[64];
	CString strKey;
	HKEY hKey;
	
	strKey.Format( _T("Software\\Shareaza\\Shareaza\\Plugins\\%s"), pszGroup );
	
	if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, strKey,
		NULL, KEY_READ, &hKey ) == ERROR_SUCCESS )
	{
		dwType = REG_SZ;
		dwCLSID = 64 * sizeof(TCHAR);
		if ( ERROR_SUCCESS != RegQueryValueEx( hKey, pszKey, NULL, &dwType,
			(LPBYTE)szCLSID, &dwCLSID ) ) dwType = 0;
		RegCloseKey( hKey );
		
		if ( dwType == REG_SZ )
		{
			szCLSID[ 38 ] = 0;
			
			return	GUIDX::Decode( szCLSID, &pCLSID ) &&
					LookupEnable( pCLSID, bEnableDefault );
		}
	}
	
	return FALSE;
}

BOOL CPlugins::LookupEnable(REFCLSID pCLSID, BOOL bDefault)
{
	CString strCLSID = GUIDX::Encode( &pCLSID );
	return theApp.GetProfileInt( _T("Plugins"), strCLSID, bDefault );
}

//////////////////////////////////////////////////////////////////////
// CPlugins skin changed event

void CPlugins::OnSkinChanged()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );
		
		if ( pPlugin->m_pCommand ) pPlugin->m_pCommand->InsertCommands();
		if ( pPlugin->m_pPlugin ) pPlugin->m_pPlugin->OnSkinChanged();
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
	STRISTATE bVisible	= TSTRUE;
	STRISTATE bEnabled	= TSTRUE;
	STRISTATE bChecked	= TSUNKNOWN;
	
	CCoolBarItem* pCoolUI = CCoolBarItem::FromCmdUI( pCmdUI );
	
	if ( pActiveWnd != NULL && pActiveWnd->IsKindOf( RUNTIME_CLASS(CPluginWnd) ) )
	{
		CPluginWnd* pPluginWnd = (CPluginWnd*)pActiveWnd;
		
		if ( pPluginWnd->m_pOwner )
		{
			if ( pPluginWnd->m_pOwner->OnUpdate( nCommandID, &bVisible, &bEnabled, &bChecked ) == S_OK )
			{
				if ( bVisible != TSUNKNOWN && pCoolUI != NULL )
					pCoolUI->Show( bVisible == TSTRUE );
				if ( bEnabled != TSUNKNOWN )
					pCmdUI->Enable( bEnabled == TSTRUE );
				if ( bChecked != TSUNKNOWN )
					pCmdUI->SetCheck( bChecked == TSTRUE );
				
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
				if ( bVisible != TSUNKNOWN && pCoolUI != NULL )
					pCoolUI->Show( bVisible == TSTRUE );
				if ( bEnabled != TSUNKNOWN )
					pCmdUI->Enable( bEnabled == TSTRUE );
				if ( bChecked != TSUNKNOWN )
					pCmdUI->SetCheck( bChecked == TSTRUE );
				
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

BOOL CPlugins::OnExecuteFile(LPCTSTR pszFile)
{
	COleVariant vFile( pszFile );
	vFile.ChangeType( VT_BSTR );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = GetNext( pos );
		
		if ( pPlugin->m_pExecute )
		{
			if ( pPlugin->m_pExecute->OnExecute( vFile.bstrVal ) == S_OK )
				return TRUE;
		}
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


//////////////////////////////////////////////////////////////////////
// CPlugin construction

CPlugin::CPlugin(REFCLSID pCLSID, LPCTSTR pszName)
{
	m_pCLSID	= pCLSID;
	m_sName		= pszName;
	m_hIcon		= LookupIcon();
	
	m_pPlugin	= NULL;
	m_pCommand	= NULL;
	m_pExecute	= NULL;
}

CPlugin::~CPlugin()
{
	Stop();
	if ( m_hIcon != NULL ) DestroyIcon( m_hIcon );
}

//////////////////////////////////////////////////////////////////////
// CPlugin start / stop operations

BOOL CPlugin::Start()
{
	if ( m_pPlugin != NULL ) return FALSE;
	
	HRESULT hResult = CoCreateInstance( m_pCLSID, NULL, CLSCTX_INPROC_SERVER,
		IID_IGeneralPlugin, (void**)&m_pPlugin );
	
	if ( FAILED( hResult ) || m_pPlugin == NULL )
	{
		m_pPlugin = NULL;
		return FALSE;
	}
	
	m_pPlugin->SetApplication(
		(IApplication*)Application.GetInterface( IID_IApplication, FALSE ) );
	
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
	if ( Plugins.LookupEnable( m_pCLSID, TRUE ) )
		return Start();
	else
		return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CPlugin CLSID helper

CString CPlugin::GetStringCLSID() const
{
	return GUIDX::Encode( &m_pCLSID );
}

//////////////////////////////////////////////////////////////////////
// CPlugin icon helper

HICON CPlugin::LookupIcon()
{
	CString strName;
	HKEY hKey;
	
	strName.Format( _T("CLSID\\%s\\InprocServer32"), (LPCTSTR)GetStringCLSID() );
	
	if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, strName, 0, KEY_QUERY_VALUE, &hKey ) )
		return NULL;
	
	DWORD dwType = REG_SZ, dwSize = 256 * sizeof(TCHAR);
	LONG lResult = RegQueryValueEx( hKey, _T(""), NULL, &dwType, (LPBYTE)strName.GetBuffer( 256 ), &dwSize );
	strName.ReleaseBuffer( dwSize / sizeof(TCHAR) );
	RegCloseKey( hKey );
	
	if ( lResult != ERROR_SUCCESS ) return NULL;
	
	HICON hIcon = NULL;
	ExtractIconEx( strName, 0, NULL, &hIcon, 1 );
	return hIcon;
}
