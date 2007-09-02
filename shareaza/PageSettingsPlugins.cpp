//
// PageSettingsPlugins.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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
	//{{AFX_MSG_MAP(CPluginsSettingsPage)
	ON_WM_TIMER()
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_PLUGINS, OnItemChangingPlugins)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PLUGINS, OnItemChangedPlugins)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PLUGINS, OnCustomDrawPlugins)
	ON_BN_CLICKED(IDC_PLUGINS_SETUP, OnPluginsSetup)
	ON_BN_CLICKED(IDC_PLUGINS_WEB, OnPluginsWeb)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPluginsSettingsPage property page

CPluginsSettingsPage::CPluginsSettingsPage() : CSettingsPage( CPluginsSettingsPage::IDD )
{
	//{{AFX_DATA_INIT(CPluginsSettingsPage)
	//}}AFX_DATA_INIT
}

CPluginsSettingsPage::~CPluginsSettingsPage()
{
}

void CPluginsSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginsSettingsPage)
	DDX_Control(pDX, IDC_PLUGINS_SETUP, m_wndSetup);
	DDX_Control(pDX, IDC_SKIN_DESC, m_wndDesc);
	DDX_Control(pDX, IDC_SKIN_NAME, m_wndName);
	DDX_Control(pDX, IDC_PLUGINS, m_wndList);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CPluginsSettingsPage message handlers

BOOL CPluginsSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 2, 1 );
	AddIcon( IDI_FILE, m_gdiImageList );
	AddIcon( IDI_EXECUTABLE, m_gdiImageList );

	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
	m_wndList.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, 382, 0 );
	m_wndList.InsertColumn( 1, _T("CLSID"), LVCFMT_LEFT, 0, 1 );
	m_wndList.InsertColumn( 2, _T("Extensions"), LVCFMT_LEFT, 0, 2 );

	m_wndList.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES );

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

void CPluginsSettingsPage::OnCustomDrawPlugins(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;
	*pResult = CDRF_DODEFAULT;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		if ( pDraw->nmcd.lItemlParam != 0 )
		{
			pDraw->clrText = CoolInterface.m_crText ;			//Interface Elements (Temp Color, was ACTIVECAPTION)
		}
		else
		{
			pDraw->clrText = CoolInterface.m_crNetworkNull ;	//Hidden Plugin (Temp Color, was 3DDKSHADOW)
		}
	}
}

void CPluginsSettingsPage::OnPluginsSetup()
{
	CString strExt;
	if ( m_wndList.GetSelectedCount() != 1 ) return;

	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	CPlugin* pPlugin = (CPlugin*)m_wndList.GetItemData( nItem );
	strExt = m_wndList.GetItemText( nItem, 2 );

	if ( pPlugin != NULL && pPlugin->m_pPlugin != NULL ) 
		pPlugin->m_pPlugin->Configure();
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
	ShellExecute( GetSafeHwnd(), _T("open"),
		_T("http://www.shareaza.com/plugins/?Version=") + theApp.m_sVersion,
		NULL, NULL, SW_SHOWNORMAL );
}

void CPluginsSettingsPage::OnOK()
{
	BOOL bChanged = FALSE;

	for ( int nItem = 0 ; nItem < m_wndList.GetItemCount() ; nItem++ )
	{
		CPlugin* pPlugin = (CPlugin*)m_wndList.GetItemData( nItem );
		CString strCLSID = m_wndList.GetItemText( nItem, 1 );

		TRISTATE bEnabled = m_wndList.GetItemState( nItem, LVIS_STATEIMAGEMASK ) >> 12;

		if ( bEnabled != TS_UNKNOWN && IsWindowVisible() )
		{
			theApp.WriteProfileString( _T("Plugins"), strCLSID, m_wndList.GetItemText( nItem, 2 ) );
		}

		if ( pPlugin != NULL && ( bEnabled == TS_TRUE ) != ( pPlugin->m_pPlugin != NULL ) )
		{
			bChanged = TRUE;

			if ( bEnabled == TS_TRUE )
				pPlugin->Start();
			else
				pPlugin->Stop();
		}
	}

	if ( bChanged ) AfxGetMainWnd()->PostMessage( WM_SKINCHANGED );

	CSettingsPage::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CPluginsSettingsPage plugin enumeration

void CPluginsSettingsPage::InsertPlugin(LPCTSTR pszCLSID, LPCTSTR pszName, int nImage, TRISTATE bEnabled, 
										LPVOID pPlugin, LPCTSTR pszExtension)
{
    int nItem = 0;
	CString strCurrAssoc, strAssocAdd;

	for ( ; nItem < m_wndList.GetItemCount() ; nItem++ )
	{
		LPVOID pExisting = (LPVOID)m_wndList.GetItemData( nItem );
		CString strExisting = m_wndList.GetItemText( nItem, 0 );

		if ( pPlugin != NULL && pExisting == NULL ) break;
		if ( pPlugin == NULL && pExisting != NULL ) continue;
		if ( strExisting.Compare( pszName ) == 0 )
		{
			if ( pszExtension && _tcslen( pszExtension ) )
			{
				strCurrAssoc = m_wndList.GetItemText( nItem, 2 );
				strAssocAdd.Format( _T("|%s|"), pszExtension );

				if ( strCurrAssoc.Find( strAssocAdd ) == -1 )
				{
					strCurrAssoc.Append( strAssocAdd );
					m_wndList.SetItemText( nItem, 2, strCurrAssoc );
				}
			}
			return;
		}
		if ( strExisting.Compare( pszName ) > 0 ) break;
	}

	nItem = m_wndList.InsertItem( LVIF_IMAGE|LVIF_TEXT|LVIF_PARAM, nItem,
		pszName, 0, 0, nImage, (LPARAM)pPlugin );

	m_wndList.SetItemText( nItem, 1, pszCLSID );

	if ( pszExtension && _tcslen( pszExtension ) )
	{
		strAssocAdd.Format( _T("|%s|"), pszExtension );
		m_wndList.SetItemText( nItem, 2, strAssocAdd );
	}
	else m_wndList.SetItemText( nItem, 2, bEnabled < TS_TRUE ? _T("-") : _T("") );

	if ( bEnabled != TS_UNKNOWN )
	{
		m_wndList.SetItemState( nItem, bEnabled << 12, LVIS_STATEIMAGEMASK );
	}
}

void CPluginsSettingsPage::EnumerateGenericPlugins()
{
	Plugins.Enumerate();

	for ( POSITION pos = Plugins.GetIterator() ; pos ; )
	{
		CPlugin* pPlugin = Plugins.GetNext( pos );

		if ( pPlugin->m_sName.GetLength() )
		{
			int nImage = AddIcon( pPlugin->LookupIcon(), m_gdiImageList );

			InsertPlugin( pPlugin->GetStringCLSID(), pPlugin->m_sName,
				( ( nImage == -1 ) ? 0 : nImage ),
				pPlugin->m_pPlugin != NULL ? TS_TRUE : TS_FALSE, pPlugin );
		}
	}
}

void CPluginsSettingsPage::EnumerateMiscPlugins()
{
	HKEY hPlugins = NULL;

	if ( ERROR_SUCCESS != RegOpenKeyEx( HKEY_LOCAL_MACHINE,
		_T("Software\\Shareaza\\Shareaza\\Plugins"), 0, KEY_READ, &hPlugins ) )
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
	CMap< CString, const CString&, CString, CString& >	pCLSIDs;
	CString strPath = _T("Software\\Shareaza\\Shareaza\\Plugins");

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
	CLSID pCLSID;

	strClass.Format( _T("CLSID\\%s"), pszCLSID );

	if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CLASSES_ROOT, strClass, 0, KEY_READ, &hClass ) )
	{
		DWORD nValue = 256 * sizeof(TCHAR), nType = REG_SZ;
		TCHAR szValue[256];

		if ( ERROR_SUCCESS == RegQueryValueEx( hClass, NULL, NULL, &nType,
			(LPBYTE)szValue, &nValue ) )
		{
			if ( Hashes::fromGuid( pszCLSID, &pCLSID ) )
			{
				TRISTATE bEnabled = TS_UNKNOWN;
				bEnabled = Plugins.LookupEnable( pCLSID, TRUE ) ? TS_TRUE : TS_FALSE;
				InsertPlugin( pszCLSID, szValue, 1, bEnabled, NULL, pszExtension );
			}
		}

		RegCloseKey( hClass );
	}
}

// Plugin description have to be put to Comments in its code resources.
// Authors can put any information here.
CString CPluginsSettingsPage::GetPluginComments(LPCTSTR pszCLSID) const
{
	CString strPath;
	HKEY hClassServer = NULL;

	strPath.Format( _T("CLSID\\%s\\InProcServer32"), pszCLSID );

	if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CLASSES_ROOT, strPath, 0, KEY_READ, &hClassServer ) )
	{
		DWORD nValue = MAX_PATH * sizeof(TCHAR), nType = REG_SZ;
		TCHAR szPluginPath[ MAX_PATH ];

		if ( ERROR_SUCCESS == RegQueryValueEx( hClassServer, NULL, NULL, &nType,
			(LPBYTE)szPluginPath, &nValue ) && nType == REG_SZ )
		{
			strPath.SetString( szPluginPath );
		}
		else return CString();
	}
	else return CString();

	DWORD nSize = GetFileVersionInfoSize( strPath, &nSize );
	BYTE* pBuffer = new BYTE[ nSize ];

	if ( ! GetFileVersionInfo( strPath, NULL, nSize, pBuffer ) )
	{
		delete [] pBuffer;
		return CString();
	}

	WCHAR* pLanguage = (WCHAR*)pBuffer + 20 + 26 + 18 + 3;
	
	if ( wcslen( pLanguage ) != 8 )
	{
		delete [] pBuffer;
		return CString();
	}

	CString strKey, strValue;

	strKey = _T("\\StringFileInfo\\");
	strKey.Append( pLanguage );
	strKey.Append( _T("\\Comments") );

	BYTE* pValue = NULL;
	nSize = 0;

	if ( VerQueryValue( pBuffer, (LPTSTR)(LPCTSTR)strKey, (void**)&pValue, (UINT*)&nSize ) )
	{
		if ( pValue[1] )
			strValue = (LPCSTR)pValue;
		else
			strValue = (LPCTSTR)pValue;
	}

	delete [] pBuffer;

	return strValue;
}

void CPluginsSettingsPage::UpdateList()
{
	if ( m_hWnd == NULL ) return;

	if ( ! IsWindowVisible() || m_wndList.GetItemCount() == 0 )
	{
		m_bRunning = FALSE;

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
