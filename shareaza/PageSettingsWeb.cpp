//
// PageSettingsWeb.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "ShareazaURL.h"
#include "PageSettingsWeb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CWebSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CWebSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CWebSettingsPage)
	ON_CBN_EDITCHANGE(IDC_EXT_LIST, OnEditChangeExtList)
	ON_CBN_SELCHANGE(IDC_EXT_LIST, OnSelChangeExtList)
	ON_BN_CLICKED(IDC_EXT_ADD, OnExtAdd)
	ON_BN_CLICKED(IDC_EXT_REMOVE, OnExtRemove)
	ON_BN_CLICKED(IDC_WEB_HOOK, OnWebHook)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWebSettingsPage property page

CWebSettingsPage::CWebSettingsPage() : CSettingsPage( CWebSettingsPage::IDD )
{
	//{{AFX_DATA_INIT(CWebSettingsPage)
	m_bUriMagnet = FALSE;
	m_bUriGnutella = FALSE;
	m_bUriED2K = FALSE;
	m_bWebHook = FALSE;
	m_bUriPiolet = FALSE;
	m_bUriTorrent = FALSE;
	//}}AFX_DATA_INIT
}

CWebSettingsPage::~CWebSettingsPage()
{
}

void CWebSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWebSettingsPage)
	DDX_Control(pDX, IDC_EXT_REMOVE, m_wndExtRemove);
	DDX_Control(pDX, IDC_EXT_ADD, m_wndExtAdd);
	DDX_Control(pDX, IDC_EXT_LIST, m_wndExtensions);
	DDX_Check(pDX, IDC_URI_MAGNET, m_bUriMagnet);
	DDX_Check(pDX, IDC_URI_GNUTELLA, m_bUriGnutella);
	DDX_Check(pDX, IDC_URI_ED2K, m_bUriED2K);
	DDX_Check(pDX, IDC_WEB_HOOK, m_bWebHook);
	DDX_Check(pDX, IDC_URI_PIOLET, m_bUriPiolet);
	DDX_Check(pDX, IDC_URI_TORRENT, m_bUriTorrent);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CWebSettingsPage message handlers

BOOL CWebSettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();
	
	m_bUriMagnet	= Settings.Web.Magnet;
	m_bUriGnutella	= Settings.Web.Gnutella;
	m_bUriED2K		= Settings.Web.ED2K;
	m_bUriPiolet	= Settings.Web.Piolet;
	m_bUriTorrent	= Settings.Web.Torrent;
	
	HKEY hKey;
	if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\{0EEDB912-C5FA-486F-8334-57288578C627}"), 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
	{
		RegCloseKey( hKey );
		m_bWebHook = TRUE;
	}
	
	CString strList = theApp.GetProfileString( _T("Downloads"), _T("WebHookExtensions"),
		_T("|zip|exe|bin|gz|z|tar|arj|lzh|sit|hqx|fml|tgz|grs|mp3|rar|r0|ace|iso|msi|") );
	
	for ( strList += '|' ; strList.GetLength() ; )
	{
		CString strType = strList.SpanExcluding( _T(" |") );
		strList = strList.Mid( strType.GetLength() + 1 );
		strType.TrimLeft();
		strType.TrimRight();
		if ( strType.GetLength() ) m_wndExtensions.AddString( strType );
	}
	
	UpdateData( FALSE );
	OnWebHook();
	
	return TRUE;
}

void CWebSettingsPage::OnWebHook() 
{
	UpdateData( TRUE );
	m_wndExtensions.EnableWindow( m_bWebHook );
	OnEditChangeExtList();
	OnSelChangeExtList();
}

void CWebSettingsPage::OnEditChangeExtList() 
{
	m_wndExtAdd.EnableWindow( m_bWebHook && m_wndExtensions.GetWindowTextLength() > 0 );
}

void CWebSettingsPage::OnSelChangeExtList() 
{
	m_wndExtRemove.EnableWindow( m_bWebHook && m_wndExtensions.GetCurSel() >= 0 );
}

void CWebSettingsPage::OnExtAdd() 
{
	CString strType;
	m_wndExtensions.GetWindowText( strType );

	ToLower( strType );

	strType.Trim();
	if ( strType.IsEmpty() ) return;

	if ( m_wndExtensions.FindString( -1, strType ) >= 0 ) return;

	m_wndExtensions.AddString( strType );
	m_wndExtensions.SetWindowText( _T("") );
}

void CWebSettingsPage::OnExtRemove() 
{
	int nItem = m_wndExtensions.GetCurSel();
	if ( nItem >= 0 ) m_wndExtensions.DeleteString( nItem );
	m_wndExtRemove.EnableWindow( FALSE );
}

void CWebSettingsPage::OnOK()
{
	UpdateData();
	
	Settings.Web.Magnet		= m_bUriMagnet != FALSE;
	Settings.Web.Gnutella	= m_bUriGnutella != FALSE;
	Settings.Web.ED2K		= m_bUriED2K != FALSE;
	Settings.Web.Piolet		= m_bUriPiolet != FALSE;
	Settings.Web.Torrent	= m_bUriTorrent != FALSE;
	
	CShareazaURL::Register();
	
	if ( HINSTANCE hInstance = LoadLibrary( Settings.General.Path + _T("\\RazaWebHook.dll") ) )
	{
		HRESULT (WINAPI *pfnRegister)();
		(FARPROC&)pfnRegister = GetProcAddress( hInstance, m_bWebHook ? "DllRegisterServer" : "DllUnregisterServer" );
		if ( pfnRegister != NULL ) (*pfnRegister)();
		FreeLibrary( hInstance );
	}
	
	HKEY hKey;
	
	if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects"), 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
	{
		if ( m_bWebHook )
		{
			DWORD dwDisposition;
			HKEY hCLSID = NULL;
			
			RegCreateKeyEx( hKey, _T("{0EEDB912-C5FA-486F-8334-57288578C627}"), 0, NULL,
				REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hCLSID, &dwDisposition );
			
			if ( hCLSID ) RegCloseKey( hCLSID );
		}
		else
		{
			RegDeleteKey( hKey, _T("{0EEDB912-C5FA-486F-8334-57288578C627}") );
		}
		
		RegCloseKey( hKey );
	}
	
	CString strExtensions;
	
	for ( int nItem = 0 ; nItem < m_wndExtensions.GetCount() ; nItem++ )
	{
		CString str;
		m_wndExtensions.GetLBText( nItem, str );
		
		if ( str.GetLength() )
		{
			if ( strExtensions.IsEmpty() ) strExtensions += '|';
			strExtensions += str;
			strExtensions += '|';
		}
	}
	
	theApp.WriteProfileInt( _T("Downloads"), _T("WebHookEnable"), m_bWebHook );
	theApp.WriteProfileString( _T("Downloads"), _T("WebHookExtensions"), strExtensions );
	
	CSettingsPage::OnOK();
}

