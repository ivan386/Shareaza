//
// PageSettingsPlugins.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPluginsSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CPluginsSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CPluginsSettingsPage)
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
	m_gdiImageList.Add( theApp.m_bRTL ? CreateMirroredIcon( theApp.LoadIcon( IDI_FILE ) ) : 
		theApp.LoadIcon( IDI_FILE ) );
	m_gdiImageList.Add( theApp.m_bRTL ? CreateMirroredIcon( theApp.LoadIcon( IDI_EXECUTABLE ) ) :
		theApp.LoadIcon( IDI_EXECUTABLE ) );

	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
	m_wndList.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, 382, 0 );
	m_wndList.InsertColumn( 1, _T("CLSID"), LVCFMT_LEFT, 0, 1 );

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

	m_bRunning = FALSE;

	EnumerateGenericPlugins();
	EnumerateMiscPlugins();

	m_wndSetup.EnableWindow( FALSE );
	m_bRunning = TRUE;

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

	if ( m_wndList.GetSelectedCount() == 1 )
	{
		int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
		CPlugin* pPlugin = (CPlugin*)m_wndList.GetItemData( nItem );
		m_wndName.SetWindowText( m_wndList.GetItemText( nItem, 0 ).Trim() );
		m_wndSetup.EnableWindow( pPlugin != NULL && pPlugin->m_pPlugin != NULL );
	}
	else
	{
		m_wndName.SetWindowText( _T("...") );
		m_wndSetup.EnableWindow( FALSE );
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
			pDraw->clrText = GetSysColor( COLOR_ACTIVECAPTION );
		}
		else
		{
			pDraw->clrText = GetSysColor( COLOR_3DDKSHADOW );
		}
	}
}

void CPluginsSettingsPage::OnPluginsSetup()
{
	if ( m_wndList.GetSelectedCount() != 1 ) return;

	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	CPlugin* pPlugin = (CPlugin*)m_wndList.GetItemData( nItem );

	if ( pPlugin != NULL && pPlugin->m_pPlugin != NULL ) pPlugin->m_pPlugin->Configure();
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

		if ( bEnabled != TS_UNKNOWN )
		{
			theApp.WriteProfileInt( _T("Plugins"), strCLSID, bEnabled == TS_TRUE );
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

void CPluginsSettingsPage::InsertPlugin(LPCTSTR pszCLSID, LPCTSTR pszName, int nImage, TRISTATE bEnabled, LPVOID pPlugin)
{
    int nItem = 0;
	for ( ; nItem < m_wndList.GetItemCount() ; nItem++ )
	{
		LPVOID pExisting = (LPVOID)m_wndList.GetItemData( nItem );
		CString strExisting = m_wndList.GetItemText( nItem, 0 );

		if ( pPlugin != NULL && pExisting == NULL ) break;
		if ( pPlugin == NULL && pExisting != NULL ) continue;
		if ( strExisting.Compare( pszName ) > 0 ) break;
	}

	nItem = m_wndList.InsertItem( LVIF_IMAGE|LVIF_TEXT|LVIF_PARAM, nItem,
		pszName, 0, 0, nImage, (LPARAM)pPlugin );

	m_wndList.SetItemText( nItem, 1, pszCLSID );

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
		int nImage = 0;

		if ( pPlugin->m_hIcon != NULL ) 
			nImage = m_gdiImageList.Add( theApp.m_bRTL ? CreateMirroredIcon ( pPlugin->m_hIcon ) : 
				pPlugin->m_hIcon );

		InsertPlugin( pPlugin->GetStringCLSID(), pPlugin->m_sName, nImage,
			pPlugin->m_pPlugin != NULL ? TS_TRUE : TS_FALSE, pPlugin );
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
	CStringList pCLSIDs;

	for ( DWORD nIndex = 0 ; ; nIndex++ )
	{
		DWORD nName = 128, nValue = 128, nType = REG_SZ;
		TCHAR szName[128], szValue[128];

		if ( ERROR_SUCCESS != RegEnumValue( hRoot, nIndex, szName, &nName,
			NULL, &nType, (LPBYTE)szValue, &nValue ) ) break;

		if ( nType == REG_SZ && szValue[0] == '{' )
		{
			if ( pCLSIDs.Find( szValue ) == NULL )
			{
				pCLSIDs.AddTail( szValue );
				AddMiscPlugin( pszType, szValue );
			}
		}
	}
}

void CPluginsSettingsPage::AddMiscPlugin(LPCTSTR pszType, LPCTSTR pszCLSID)
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
			if ( GUIDX::Decode( pszCLSID, &pCLSID ) )
			{
				TRISTATE bEnabled = TS_UNKNOWN;
				bEnabled = Plugins.LookupEnable( pCLSID, TRUE ) ? TS_TRUE : TS_FALSE;
				InsertPlugin( pszCLSID, szValue, 1, bEnabled );
			}
		}

		RegCloseKey( hClass );
	}
}
