//
// DlgMediaVis.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "CoolInterface.h"
#include "DlgMediaVis.h"
#include "CtrlMediaFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CMediaVisDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CMediaVisDlg, CSkinDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_PLUGINS, OnDblClkPlugins)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PLUGINS, OnItemChangedPlugins)
	ON_BN_CLICKED(IDC_VIS_SETUP, OnSetup)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMediaVisDlg dialog

CMediaVisDlg::CMediaVisDlg(CMediaFrame* pFrame) : CSkinDialog( CMediaVisDlg::IDD, NULL )
{
	//{{AFX_DATA_INIT(CMediaVisDlg)
	m_nSize = -1;
	//}}AFX_DATA_INIT
	m_pFrame	= pFrame;
	m_hIcon		= NULL;
}

CMediaVisDlg::~CMediaVisDlg()
{
	if ( m_hIcon ) DestroyIcon( m_hIcon );
}

void CMediaVisDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VIS_SETUP, m_wndSetup);
	DDX_Control(pDX, IDC_PLUGINS, m_wndList);
	DDX_CBIndex(pDX, IDC_VIS_SIZE, m_nSize);
}

/////////////////////////////////////////////////////////////////////////////
// CMediaVisDlg message handlers

BOOL CMediaVisDlg::OnInitDialog()
{
	CString strMessage;

	CSkinDialog::OnInitDialog();

	SkinMe( NULL, ID_MEDIA_VIS );

	CRect rc;
	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + 1;

	CoolInterface.SetImageListTo( m_wndList, LVSIL_SMALL );
	m_wndList.InsertColumn( 0, _T("Description"), LVCFMT_LEFT, rc.right, -1 );
	m_wndList.InsertColumn( 1, _T("CLSID"), LVCFMT_LEFT, 0, 0 );
	m_wndList.InsertColumn( 2, _T("Subpath"), LVCFMT_LEFT, 0, 1 );

	m_wndList.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT );

	m_nIcon = CoolInterface.ImageForID( ID_MEDIA_VIS );
	m_hIcon = CoolInterface.ExtractIcon( ID_MEDIA_VIS, FALSE );
	if ( m_hIcon )
		SetIcon( m_hIcon, FALSE );

	LoadString( strMessage, IDS_MEDIAVIS_NOVIS );
	AddPlugin( strMessage, NULL, NULL );
	Enumerate();

	m_nSize = Settings.MediaPlayer.VisSize + 1;
	UpdateData( FALSE );

	m_wndSetup.EnableWindow( m_wndList.GetSelectedCount() == 1 );

	return TRUE;
}

void CMediaVisDlg::Enumerate()
{
	CWaitCursor pCursor;
	HKEY hKey;

	if ( RegOpenKeyEx( HKEY_CURRENT_USER,
		REGISTRY_KEY _T("\\Plugins\\AudioVis"),
		NULL, KEY_READ, &hKey ) != ERROR_SUCCESS ) return;

	for ( DWORD nKey = 0 ; ; nKey++ )
	{
		DWORD dwType, dwName = 256, dwCLSID = 64 * sizeof(TCHAR);
		TCHAR szName[256], szCLSID[64];

		if ( RegEnumValue( hKey, nKey, szName, &dwName, NULL, &dwType, (LPBYTE)szCLSID, &dwCLSID )
			 != ERROR_SUCCESS ) break;

		if ( dwType != REG_SZ || dwCLSID / sizeof(TCHAR) != 39 || szCLSID[0] != '{' || szName[0] == '{' ) continue;
		szCLSID[ 38 ] = 0;

		CLSID pCLSID;
		if ( ! Hashes::fromGuid( szCLSID, &pCLSID ) ) continue;
		if ( ! Plugins.LookupEnable( pCLSID ) ) continue;

		if ( _tcsistr( szName, _T("wrap") ) )
		{
			if ( ! EnumerateWrapped( szName, pCLSID, szCLSID ) )
			{
				AddPlugin( szName, szCLSID, NULL );
			}
		}
		else
		{
			AddPlugin( szName, szCLSID, NULL );
		}
	}

	RegCloseKey( hKey );
}

void CMediaVisDlg::AddPlugin(LPCTSTR pszName, LPCTSTR pszCLSID, LPCTSTR pszPath)
{
	int nItem = m_wndList.InsertItem( LVIF_TEXT|LVIF_IMAGE, -1, pszName, 0, 0, m_nIcon, 0 );

	if ( pszCLSID ) m_wndList.SetItemText( nItem, 1, pszCLSID );
	if ( pszPath ) m_wndList.SetItemText( nItem, 2, pszPath );

	if ( ( pszCLSID == NULL && Settings.MediaPlayer.VisCLSID.IsEmpty() ) ||
		 ( pszCLSID != NULL && Settings.MediaPlayer.VisCLSID.CompareNoCase( pszCLSID ) == 0 ) )
	{
		if ( ( pszPath == NULL && Settings.MediaPlayer.VisPath.IsEmpty() ) ||
			 ( pszPath != NULL && Settings.MediaPlayer.VisPath.CompareNoCase( pszPath ) == 0 ) )
		{
			m_wndList.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
}

BOOL CMediaVisDlg::EnumerateWrapped(LPCTSTR pszName, REFCLSID pCLSID, LPCTSTR pszCLSID)
{
	IWrappedPluginControl* pPlugin = NULL;

	HINSTANCE hRes = AfxGetResourceHandle();

	HRESULT hr = CoCreateInstance( pCLSID, NULL, CLSCTX_ALL,
		IID_IWrappedPluginControl, (void**)&pPlugin );

	AfxSetResourceHandle( hRes );

	if ( FAILED(hr) || pPlugin == NULL ) return FALSE;

	LPSAFEARRAY pArray = NULL;
	hr = pPlugin->Enumerate( &pArray );

	pPlugin->Release();

	if ( FAILED(hr) || pArray == NULL ) return FALSE;

	LONG pIndex[2] = { 0, 0 };
	SafeArrayGetUBound( pArray, 2, &pIndex[1] );

	if ( pIndex[1] < 0 )
	{
		SafeArrayDestroy( pArray );
		return TRUE;
	}

	for ( ; pIndex[1] >= 0 ; pIndex[1] -- )
	{
		CString strName, strPath;
		BSTR bsValue = NULL;

		strName = pszName;
		strName += _T(": ");

		pIndex[0] = 0;
		SafeArrayGetElement( pArray, pIndex, &bsValue );
		strName += bsValue;
		SysFreeString( bsValue );
		bsValue = NULL;

		pIndex[0] = 1;
		SafeArrayGetElement( pArray, pIndex, &bsValue );
		strPath = bsValue;
		SysFreeString( bsValue );
		bsValue = NULL;

		AddPlugin( strName, pszCLSID, strPath );
	}

	SafeArrayDestroy( pArray );

	return TRUE;
}

void CMediaVisDlg::OnDblClkPlugins(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	PostMessage( WM_COMMAND, IDOK );
	*pResult = 0;
}

void CMediaVisDlg::OnItemChangedPlugins(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	m_wndSetup.EnableWindow( m_wndList.GetSelectedCount() == 1 );
	*pResult = 0;
}

void CMediaVisDlg::OnSetup()
{
	int nItem = m_wndList.GetNextItem( -1, LVIS_SELECTED );
	if ( nItem < 0 ) return;

	CString strCLSID	= m_wndList.GetItemText( nItem, 1 );
	CString strPath		= m_wndList.GetItemText( nItem, 2 );

	CLSID pCLSID;
	if ( ! Hashes::fromGuid( strCLSID, &pCLSID ) ) return;
	if ( ! Plugins.LookupEnable( pCLSID ) ) return;

	IAudioVisPlugin* pPlugin = NULL;

	if ( Settings.MediaPlayer.VisCLSID == strCLSID &&
		 Settings.MediaPlayer.VisPath == strPath )
	{
		IMediaPlayer* pPlayer = ( m_pFrame != NULL ) ? m_pFrame->GetPlayer() : NULL;

		if ( pPlayer != NULL )
		{
			pPlayer->GetPlugin( &pPlugin );

			if ( pPlugin != NULL )
			{
				pPlugin->Configure();
				pPlugin->Release();
				return;
			}
		}
	}

	HINSTANCE hRes = AfxGetResourceHandle();

	HRESULT hr = CoCreateInstance( pCLSID, NULL, CLSCTX_ALL,
		IID_IAudioVisPlugin, (void**)&pPlugin );

	AfxSetResourceHandle( hRes );

	if ( FAILED(hr) || pPlugin == NULL ) return;

	if ( strPath.GetLength() )
	{
		IWrappedPluginControl* pWrap = NULL;
		hr = pPlugin->QueryInterface( IID_IWrappedPluginControl, (void**)&pWrap );
		if ( SUCCEEDED(hr) && pWrap != NULL )
		{
			pWrap->Load( CComBSTR( strPath ), 0 );
			pWrap->Release();
		}
	}

	pPlugin->Configure();
	pPlugin->Release();
}

void CMediaVisDlg::OnOK()
{
	int nItem = m_wndList.GetNextItem( -1, LVIS_SELECTED );
	CString strCLSID, strPath;

	UpdateData( TRUE );

	if ( nItem >= 0 )
	{
		strCLSID	= m_wndList.GetItemText( nItem, 1 );
		strPath		= m_wndList.GetItemText( nItem, 2 );
	}

	if ( Settings.MediaPlayer.VisCLSID != strCLSID ||
		 Settings.MediaPlayer.VisPath != strPath ||
		 Settings.MediaPlayer.VisSize != (DWORD)( m_nSize - 1 ) )
	{
		Settings.MediaPlayer.VisCLSID	= strCLSID;
		Settings.MediaPlayer.VisPath	= strPath;
		Settings.MediaPlayer.VisSize	= m_nSize - 1;

		Settings.Save();
		CSkinDialog::OnOK();
	}
	else
	{
		CSkinDialog::OnCancel();
	}
}

