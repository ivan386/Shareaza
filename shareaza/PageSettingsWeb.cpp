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
	m_bUriDC = FALSE;
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
	DDX_Check(pDX, IDC_URI_DC, m_bUriDC);
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
	m_bUriDC		= Settings.Web.DC;

	m_bWebHook		= Settings.Downloads.WebHookEnable;

	for ( string_set::const_iterator i = Settings.Downloads.WebHookExtensions.begin() ;
		i != Settings.Downloads.WebHookExtensions.end(); ++i )
	{
		m_wndExtensions.AddString( (*i) );
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
	Settings.Web.DC			= m_bUriDC != FALSE;

	Settings.Downloads.WebHookEnable = m_bWebHook != FALSE;

	Settings.Downloads.WebHookExtensions.clear();
	for ( int nItem = 0 ; nItem < m_wndExtensions.GetCount() ; nItem++ )
	{
		CString str;
		m_wndExtensions.GetLBText( nItem, str );
		if ( str.GetLength() )
		{
			Settings.Downloads.WebHookExtensions.insert( str );
		}
	}
	
	CShareazaURL::Register();

	CSettingsPage::OnOK();
}
