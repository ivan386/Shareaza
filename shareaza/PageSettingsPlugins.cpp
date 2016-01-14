//
// PageSettingsPlugins.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "PageSettingsPlugins.h"
#include "DlgPluginExtSetup.h"
#include "CoolInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPluginsSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CPluginsSettingsPage, CSettingsPage)
	ON_WM_TIMER()
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_PLUGINS, OnItemChangingPlugins)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PLUGINS, OnItemChangedPlugins)
	ON_NOTIFY(NM_DBLCLK, IDC_PLUGINS, OnNMDblclkPlugins)
	ON_BN_CLICKED(IDC_PLUGINS_SETUP, OnPluginsSetup)
	ON_BN_CLICKED(IDC_PLUGINS_WEB, OnPluginsWeb)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPluginsSettingsPage property page

CPluginsSettingsPage::CPluginsSettingsPage() : CSettingsPage( CPluginsSettingsPage::IDD )
{
}

void CPluginsSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PLUGINS_SETUP, m_wndSetup);
	DDX_Control(pDX, IDC_SKIN_DESC, m_wndDesc);
	DDX_Control(pDX, IDC_SKIN_NAME, m_wndName);
	DDX_Control(pDX, IDC_PLUGINS, m_wndList);
}

/////////////////////////////////////////////////////////////////////////////
// CPluginsSettingsPage message handlers

BOOL CPluginsSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 6, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 6, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR16|ILC_MASK, 6, 1 );

	AddIcon( IDI_EXECUTABLE, m_gdiImageList );

	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
	m_wndList.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, 382, 0 );
	m_wndList.InsertColumn( 1, _T("CLSID"), LVCFMT_LEFT, 0, 1 );
	m_wndList.InsertColumn( 2, _T("Extensions"), LVCFMT_LEFT, 0, 2 );

	m_wndList.SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES |
		LVS_EX_DOUBLEBUFFER );

	/*
	LVGROUP pGroup;
	pGroup.cbSize		= sizeof(pGroup);
	pGroup.mask			= LVGF_ALIGN|LVGF_GROUPID|LVGF_HEADER;
	pGroup.uAlign		= LVGA_HEADER_LEFT;
	pGroup.pszHeader	= _T("General Plugins");
	pGroup.cchHeader	= _tcslen( pGroup.pszHeader );
	m_wndList.InsertGroup( 0, &pGroup );
	*/

	SetTimer( 1, 100, NULL );
	m_wndSetup.EnableWindow( FALSE );

	return TRUE;
}

void CPluginsSettingsPage::OnItemChangingPlugins(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW* pNMListView = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
	*pResult = 0;

	if ( ( pNMListView->uOldState & LVIS_STATEIMAGEMASK ) == 0 &&
		 ( pNMListView->uNewState & LVIS_STATEIMAGEMASK ) != 0 )
	{
		if ( m_bRunning ) *pResult = 1;
	}
}

void CPluginsSettingsPage::OnItemChangedPlugins(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pNMListView = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
	*pResult = 0;
	if ( ! m_bRunning ) return;

	// Selected item handling
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	if ( m_wndList.GetSelectedCount() == 1 )
	{
		CString str = m_wndList.GetItemText( nItem, 2 );
		CPlugin* pPlugin = (CPlugin*)m_wndList.GetItemData( nItem );
		m_wndName.SetWindowText( m_wndList.GetItemText( nItem, 0 ).Trim() );
		m_wndDesc.SetWindowText( GetPluginComments( m_wndList.GetItemText( nItem, 1 ) ) );
		m_wndSetup.EnableWindow( pPlugin != NULL && pPlugin->m_pPlugin != NULL ||
			( ! str.IsEmpty() && str != _T("-") ) );
	}
	else
	{
		m_wndName.SetWindowText( _T("...") );
		m_wndSetup.EnableWindow( FALSE );
	}

	// Check box handling
	nItem = pNMListView->iItem;

	if ( ( ( pNMListView->uOldState >> 12 ) & LVIS_SELECTED ) == 0 &&
		 ( ( pNMListView->uNewState >> 12 ) & LVIS_SELECTED ) != 0 ) 
	{
		CString strExt = m_wndList.GetItemText( nItem, 2 );
		strExt.Replace( _T("-"), _T("") );
		m_wndList.SetItemText( nItem, 2, strExt );
	}
	else if ( ( ( pNMListView->uOldState >> 12 ) & LVIS_SELECTED ) != 0 &&
			( ( pNMListView->uNewState >> 12 ) & LVIS_SELECTED ) == 0 )
	{
		CString strExt = m_wndList.GetItemText( nItem, 2 );
		if ( ! strExt.IsEmpty() ) 
			strExt.Replace( _T("-"), _T("") );
		else
			strExt = _T("-");

		for ( int nDot = 0 ; nDot != -1 ; )
		{
			nDot = strExt.Find( '.', nDot );
			if ( nDot != -1 )
			{
				strExt.Insert( nDot, '-' );
				nDot += 2;
			}
		}

		m_wndList.SetItemText( nItem, 2, strExt );
	}
}

void CPluginsSettingsPage::OnPluginsSetup()
{
	if ( m_wndList.GetSelectedCount() != 1 )
		return;

	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	CPlugin* pPlugin = (CPlugin*)m_wndList.GetItemData( nItem );
	CString strExt = m_wndList.GetItemText( nItem, 2 );

	if ( pPlugin != NULL )
	{
		bool bStopped = ( pPlugin->m_pPlugin == NULL );

		if ( bStopped )
			pPlugin->Start();

		if ( pPlugin->m_pPlugin )
			pPlugin->m_pPlugin->Configure();

		if ( bStopped )
			pPlugin->Stop();
	}
	else if ( ! strExt.IsEmpty() )
	{
		CPluginExtSetupDlg dlg( &m_wndList, strExt );
		m_bRunning = FALSE;
		dlg.DoModal();
		m_bRunning = TRUE;
	}
}

void CPluginsSettingsPage::OnPluginsWeb()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"),
		strWebSite + _T("?id=addon&Version=") + theApp.m_sVersion,
		NULL, NULL, SW_SHOWNORMAL );
}

void CPluginsSettingsPage::OnOK()
{
	BOOL bChanged = FALSE;

	int nCount = m_wndList.GetItemCount();
	for ( int nItem = 0 ; nItem < nCount ; nItem++ )
	{
		CPlugin* pPlugin = (CPlugin*)m_wndList.GetItemData( nItem );
		CString strCLSID = m_wndList.GetItemText( nItem, 1 );

		TRISTATE bEnabled = static_cast< TRISTATE >(
			m_wndList.GetItemState( nItem, LVIS_STATEIMAGEMASK ) >> 12 );

		if ( bEnabled != TRI_UNKNOWN && IsWindowVisible() )
		{
			theApp.WriteProfileString( _T("Plugins"), strCLSID, m_wndList.GetItemText( nItem, 2 ) );
		}

		if ( pPlugin != NULL && ( bEnabled == TRI_TRUE ) != ( pPlugin->m_pPlugin != NULL ) )
		{
			bChanged = TRUE;

			if ( bEnabled == TRI_TRUE )
				pPlugin->Start();
			else
				pPlugin->Stop();
		}
	}

	if ( bChanged ) PostMainWndMessage( WM_SKINCHANGED );

	CSettingsPage::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CPluginsSettingsPage plugin enumeration

void CPluginsSettingsPage::InsertPlugin(LPCTSTR pszCLSID, LPCTSTR pszName, 
	CPlugin* pPlugin, LPCTSTR pszExtension)
{
    int nItem = 0;
	CString strAssocAdd;
	if ( pszExtension && *pszExtension )
	{
		strAssocAdd = _T("|");
		strAssocAdd += pszExtension;
		strAssocAdd += _T("|");
	}

	int nCount = m_wndList.GetItemCount();
	for ( ; nItem < nCount ; nItem++ )
	{
		CString strExisting = m_wndList.GetItemText( nItem, 0 );
		if ( strExisting.Compare( pszName ) == 0 )
		{
			if ( ! strAssocAdd.IsEmpty() )
			{
				CString strCurrAssoc = m_wndList.GetItemText( nItem, 2 );
				if ( strCurrAssoc.Find( strAssocAdd ) == -1 )
				{
					if ( strCurrAssoc.IsEmpty() || strCurrAssoc == _T("-") )
						m_wndList.SetItemText( nItem, 2, strAssocAdd );
					else
						m_wndList.SetItemText( nItem, 2, strCurrAssoc +
							( (LPCTSTR)strAssocAdd + 1 ) );
				}
			}
			return;
		}
		if ( strExisting.Compare( pszName ) > 0 )
			break;
	}

	CLSID pCLSID = {};
	TRISTATE bEnabled = ( pPlugin && pPlugin->m_pPlugin ) ? TRI_TRUE :
		( ( Hashes::fromGuid( pszCLSID, &pCLSID ) && Plugins.LookupEnable( pCLSID ) ) ?
		TRI_TRUE : TRI_FALSE );
	HICON hIcon = LoadCLSIDIcon( pszCLSID );
	int nImage = hIcon ? AddIcon( hIcon, m_gdiImageList ) : -1;

	nItem = m_wndList.InsertItem( LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM, nItem,
		pszName, 0, 0, ( ( nImage != -1 ) ? nImage : 0 ), (LPARAM)pPlugin );

	m_wndList.SetItemText( nItem, 1, pszCLSID );

	if ( pszExtension && *pszExtension )
		m_wndList.SetItemText( nItem, 2, strAssocAdd );
	else
		m_wndList.SetItemText( nItem, 2, bEnabled < TRI_TRUE ? _T("-") : _T("") );

	if ( bEnabled != TRI_UNKNOWN )
	{
		m_wndList.SetItemState( nItem, bEnabled << 12, LVIS_STATEIMAGEMASK );
	}

	m_wndList.UpdateWindow();
}

void CPluginsSettingsPage::EnumerateGenericPlugins()
{
	Plugins.Enumerate();

	for ( POSITION pos = Plugins.GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = Plugins.GetNext( pos );

		InsertPlugin( pPlugin->GetStringCLSID(), pPlugin->m_sName, pPlugin );
	}
}

void CPluginsSettingsPage::EnumerateMiscPlugins()
{
	HKEY hPlugins = NULL;

	if ( ERROR_SUCCESS != RegOpenKeyEx( HKEY_CURRENT_USER,
		REGISTRY_KEY _T("\\Plugins"), 0, KEY_READ, &hPlugins ) )
		return;

	for ( DWORD nIndex = 0 ; ; nIndex++ )
	{
		HKEY hCategory = NULL;
		TCHAR szName[128];
		DWORD nName = 128;

		if ( ERROR_SUCCESS != RegEnumKeyEx( hPlugins, nIndex, szName, &nName,
			NULL, NULL, NULL, NULL ) ) break;

		if ( _tcsicmp( szName, _T("General") ) != 0 )
		{
			if ( ERROR_SUCCESS == RegOpenKeyEx( hPlugins, szName, 0, KEY_READ,
				 &hCategory ) )
			{
				EnumerateMiscPlugins( szName, hCategory );
				RegCloseKey( hCategory );
			}
		}
	}

	RegCloseKey( hPlugins );
}

void CPluginsSettingsPage::EnumerateMiscPlugins(LPCTSTR pszType, HKEY hRoot)
{
	CStringIMap pCLSIDs;
	CString strPath = REGISTRY_KEY _T("\\Plugins");

	for ( DWORD nIndex = 0 ; ; nIndex++ )
	{
		CWaitCursor pCursor;

		DWORD nName = 128, nValue = 128, nType = REG_SZ;
		TCHAR szName[128], szValue[128];

		if ( ERROR_SUCCESS != RegEnumValue( hRoot, nIndex, szName, &nName,
			NULL, &nType, (LPBYTE)szValue, &nValue ) ) break;

		if ( nType == REG_SZ && szValue[0] == '{' )
		{
			CString strExts, strEnabledExt, strDisabledExt, strCurrExt;

			if ( *szName == '.' )
			{
				if ( pCLSIDs.Lookup( szValue, strExts ) == FALSE )
				{
					DWORD nLength = 0;
					HKEY hUserPlugins = NULL;

					if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER, strPath, 0, KEY_READ, &hUserPlugins ) )
					{
						if ( ERROR_SUCCESS == RegQueryValueEx( hUserPlugins, (LPCTSTR)szValue, 
												NULL, &nType, NULL, &nLength ) && nType == REG_SZ && nLength )
						{
							TCHAR* pszExtValue = new TCHAR[ nLength ];
							if ( ERROR_SUCCESS == RegQueryValueEx( hUserPlugins, (LPCTSTR)szValue, 
													NULL, &nType, (LPBYTE)pszExtValue, &nLength ) )
							{
								// Found under user options
								strExts.SetString( pszExtValue );
							}
							delete [] pszExtValue;
						}
						else if ( nType == REG_DWORD ) // Upgrade from REG_DWORD to REG_SZ
						{
							BOOL bEnabled = theApp.GetProfileInt( _T("Plugins"), szValue, TRUE );
							strExts = bEnabled ? _T("") : _T("-");
						}
						RegCloseKey( hUserPlugins );
					}
				}

				// Disabled extensions have '-' sign before their names
				strEnabledExt.Format( _T("|%s|"), szName );
				strDisabledExt.Format( _T("|-%s|"), szName );
				if ( strExts.Find( strEnabledExt ) == -1 )
				{
					if ( strExts.Find( strDisabledExt ) == -1 )
					{
						// Missing extension under user options; append to the list
						// Leave "-" if upgrading: it will be removed eventually when user applies settings
						strCurrExt = ( strExts.Left( 1 ) == '-' ) ? strDisabledExt : strEnabledExt;
						strExts.Append( strCurrExt );
					}
					else
						strCurrExt = strDisabledExt;
				}
				else strCurrExt = strEnabledExt;
			}

			strExts.Replace( _T("||"), _T("|") );
            pCLSIDs.SetAt( szValue, strExts );
			if ( ! strExts.IsEmpty() ) theApp.WriteProfileString( _T("Plugins"), szValue, strExts );
			strCurrExt.Replace( _T("|"), _T("") );
			AddMiscPlugin( pszType, szValue, strCurrExt );
		}
	}
}

void CPluginsSettingsPage::AddMiscPlugin(LPCTSTR /*pszType*/, LPCTSTR pszCLSID, LPCTSTR pszExtension)
{
	HKEY hClass = NULL;
	CString strClass;

	strClass.Format( _T("CLSID\\%s"), pszCLSID );

	if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CLASSES_ROOT, strClass, 0, KEY_READ, &hClass ) )
	{
		DWORD nValue = MAX_PATH * sizeof(TCHAR), nType = REG_SZ;
		TCHAR szValue[ MAX_PATH ];

		if ( ERROR_SUCCESS == RegQueryValueEx( hClass, NULL, NULL, &nType,
			(LPBYTE)szValue, &nValue ) )
		{
			InsertPlugin( pszCLSID, szValue, NULL, pszExtension );
		}

		RegCloseKey( hClass );
	}
}

// Plugin description have to be put to Comments in its code resources.
// Authors can put any information here.
CString CPluginsSettingsPage::GetPluginComments(LPCTSTR pszCLSID) const
{
	CString strPath;
	HKEY hKey;
	strPath.Format( _T("CLSID\\%s\\InProcServer32"), pszCLSID );
	if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, strPath, 0, KEY_READ, &hKey ) != ERROR_SUCCESS )
	{
		strPath.Format( _T("CLSID\\%s\\LocalServer32"), pszCLSID );
		if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, strPath, 0, KEY_READ, &hKey ) != ERROR_SUCCESS )
			return CString();
	}

	DWORD dwType = REG_SZ, dwSize = MAX_PATH * sizeof( TCHAR );
	LONG lResult = RegQueryValueEx( hKey, NULL, NULL, &dwType,
		(LPBYTE)strPath.GetBuffer( MAX_PATH ), &dwSize );
	strPath.ReleaseBuffer( dwSize / sizeof( TCHAR ) );
	RegCloseKey( hKey );

	if ( lResult != ERROR_SUCCESS )
		return CString();

	strPath.Trim( _T(" \"") );

	CString strValue;
	dwSize = GetFileVersionInfoSize( strPath, &dwSize );
	if ( dwSize )
	{
		if ( BYTE* pBuffer = new BYTE[ dwSize ] )
		{
			if ( GetFileVersionInfo( strPath, NULL, dwSize, pBuffer ) )
			{
				WCHAR* pLanguage = (WCHAR*)pBuffer + 20 + 26 + 18 + 3;
				if ( wcslen( pLanguage ) == 8 )
				{
					CString strKey = _T("\\StringFileInfo\\");
					strKey.Append( pLanguage );
					strKey.Append( _T("\\Comments") );
					BYTE* pValue = NULL;
					dwSize = 0;
					if ( VerQueryValue( pBuffer, (LPTSTR)(LPCTSTR)strKey,
						(void**)&pValue, (UINT*)&dwSize ) )
					{
						if ( pValue[1] )
							strValue = (LPCSTR)pValue;
						else
							strValue = (LPCTSTR)pValue;
					}
				}
			}
			delete [] pBuffer;
		}
	}
	return strValue;
}

void CPluginsSettingsPage::UpdateList()
{
	if ( m_hWnd == NULL ) return;

	if ( ! IsWindowVisible() || m_wndList.GetItemCount() == 0 )
	{
		m_bRunning = FALSE;

		CWaitCursor wc;

		m_wndList.DeleteAllItems();
		EnumerateGenericPlugins();
		EnumerateMiscPlugins();

		m_bRunning = TRUE;
	}
}

void CPluginsSettingsPage::OnTimer(UINT_PTR /*nIDEvent*/)
{
	if ( IsWindowVisible() )
	{
		KillTimer( 1 );
		UpdateList();
	}
}

void CPluginsSettingsPage::OnNMDblclkPlugins(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnPluginsSetup();
	*pResult = 0;
}
