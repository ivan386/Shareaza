//
// PageSettingsMedia.cpp
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
#include "WndSettingsPage.h"
#include "WndSettingsSheet.h"
#include "PageSettingsMedia.h"
#include "PageSettingsPlugins.h"
#include "DlgMediaVis.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CMediaSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CMediaSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CMediaSettingsPage)
	ON_BN_CLICKED(IDC_MEDIA_PLAY, OnMediaPlay)
	ON_BN_CLICKED(IDC_MEDIA_ENQUEUE, OnMediaEnqueue)
	ON_CBN_SELCHANGE(IDC_MEDIA_TYPES, OnSelChangeMediaTypes)
	ON_CBN_EDITCHANGE(IDC_MEDIA_TYPES, OnEditChangeMediaTypes)
	ON_CBN_SELCHANGE(IDC_MEDIA_SERVICE, OnSelChangeMediaService)
	ON_BN_CLICKED(IDC_MEDIA_ADD, OnMediaAdd)
	ON_BN_CLICKED(IDC_MEDIA_REMOVE, OnMediaRemove)
	ON_BN_CLICKED(IDC_MEDIA_VIS, OnMediaVis)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMediaSettingsPage property page

CMediaSettingsPage::CMediaSettingsPage() : CSettingsPage(CMediaSettingsPage::IDD)
{
	//{{AFX_DATA_INIT(CMediaSettingsPage)
	m_sType = _T("");
	m_bEnablePlay = FALSE;
	m_bEnableEnqueue = FALSE;
	//}}AFX_DATA_INIT
}

CMediaSettingsPage::~CMediaSettingsPage()
{
}

void CMediaSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMediaSettingsPage)
	DDX_Control(pDX, IDC_MEDIA_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_MEDIA_ADD, m_wndAdd);
	DDX_Control(pDX, IDC_MEDIA_TYPES, m_wndList);
	DDX_CBString(pDX, IDC_MEDIA_TYPES, m_sType);
	DDX_Check(pDX, IDC_MEDIA_PLAY, m_bEnablePlay);
	DDX_Check(pDX, IDC_MEDIA_ENQUEUE, m_bEnableEnqueue);
	DDX_Control(pDX, IDC_MEDIA_SERVICE, m_wndServices);
	//}}AFX_DATA_MAP

}

/////////////////////////////////////////////////////////////////////////////
// CMediaSettingsPage message handlers

BOOL CMediaSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_bEnablePlay		= Settings.MediaPlayer.EnablePlay;
	m_bEnableEnqueue	= Settings.MediaPlayer.EnableEnqueue;

	for ( CString strList = Settings.MediaPlayer.FileTypes + '|' ; strList.GetLength() ; )
	{
		CString strType = strList.SpanExcluding( _T(" |") );
		strList = strList.Mid( strType.GetLength() + 1 );
		strType.TrimLeft();
		strType.TrimRight();
		if ( strType.GetLength() ) m_wndList.AddString( strType );
	}
	
	CString str;
	LoadString( str, IDS_MEDIA_SMPLAYER );
	m_wndServices.AddString( str );
	LoadString( str, IDS_GENERAL_CUSTOM );
	str.Insert( 0, '(' );
	str.Append( _T("\x2026)") );
	m_wndServices.AddString( str );

	if ( Settings.MediaPlayer.ServicePath.IsEmpty() )
		m_wndServices.SetCurSel( 0 );
	else
	{
		m_sServicePath = Settings.MediaPlayer.ServicePath;
		int nBackSlash = m_sServicePath.ReverseFind( '\\' );
		str = m_sServicePath.Mid( nBackSlash + 1 );
		m_wndServices.InsertString( 0, str );
		m_wndServices.SetCurSel( 0 );
		GetDlgItem( IDC_MEDIA_PLAY )->EnableWindow( FALSE );
		GetDlgItem( IDC_MEDIA_ENQUEUE )->EnableWindow( FALSE );
		GetDlgItem( IDC_MEDIA_VIS )->EnableWindow( FALSE );
	}

	UpdateData( FALSE );

	m_wndAdd.EnableWindow( m_wndList.GetWindowTextLength() > 0 );
	m_wndRemove.EnableWindow( m_wndList.GetCurSel() >= 0 );

	return TRUE;
}

void CMediaSettingsPage::OnMediaPlay()
{
	UpdateData();
	m_wndList.EnableWindow( m_bEnablePlay || m_bEnableEnqueue );
	m_wndAdd.EnableWindow( ( m_bEnablePlay || m_bEnableEnqueue ) && m_wndList.GetWindowTextLength() > 0 );
	m_wndRemove.EnableWindow( ( m_bEnablePlay || m_bEnableEnqueue ) && m_wndList.GetCurSel() >= 0 );
}

void CMediaSettingsPage::OnMediaEnqueue()
{
	UpdateData();
	m_wndList.EnableWindow( m_bEnablePlay || m_bEnableEnqueue );
	m_wndAdd.EnableWindow( ( m_bEnablePlay || m_bEnableEnqueue ) && m_wndList.GetWindowTextLength() > 0 );
	m_wndRemove.EnableWindow( ( m_bEnablePlay || m_bEnableEnqueue ) && m_wndList.GetCurSel() >= 0 );
}

void CMediaSettingsPage::OnSelChangeMediaTypes()
{
	m_wndRemove.EnableWindow( m_wndList.GetCurSel() >= 0 );
}

void CMediaSettingsPage::OnEditChangeMediaTypes()
{
	m_wndAdd.EnableWindow( m_wndList.GetWindowTextLength() > 0 );
}

void CMediaSettingsPage::OnMediaAdd()
{
	UpdateData();

	ToLower( m_sType );
	m_sType.Trim();
	if ( m_sType.IsEmpty() ) return;

	if ( m_wndList.FindStringExact( -1, m_sType ) >= 0 ) return;

	m_wndList.AddString( m_sType );
	m_sType.Empty();
	UpdateData( FALSE );
}

void CMediaSettingsPage::OnMediaRemove()
{
	int nItem = m_wndList.GetCurSel();
	if ( nItem >= 0 ) m_wndList.DeleteString( nItem );
	m_wndRemove.EnableWindow( FALSE );
}

void CMediaSettingsPage::OnMediaVis()
{
	CMediaVisDlg dlg( NULL );
	dlg.DoModal();
}

void CMediaSettingsPage::OnOK()
{
	UpdateData();

	Settings.MediaPlayer.EnablePlay		= m_bEnablePlay != FALSE;
	Settings.MediaPlayer.EnableEnqueue	= m_bEnableEnqueue != FALSE;
	Settings.MediaPlayer.ServicePath	= m_sServicePath;

	CString strRegData;

	if ( m_sServicePath.IsEmpty() )
		Settings.MediaPlayer.ShortPaths = FALSE;
	else
	{	
		strRegData = _T("-");
		/*
		// Starting from v.0.8.5 VLC player reads unicode paths
		CString strExecutable;
		m_wndServices.GetWindowText( strExecutable );
		Settings.MediaPlayer.ShortPaths = ToLower( strExecutable ) == _T("vlc.exe");
		*/
	}

	theApp.WriteProfileString( _T("Plugins"), Settings.MediaPlayer.AviPreviewCLSID, strRegData );
	theApp.WriteProfileString( _T("Plugins"), Settings.MediaPlayer.MediaServicesCLSID, strRegData );
	theApp.WriteProfileString( _T("Plugins"), Settings.MediaPlayer.Mp3PreviewCLSID, strRegData );
	theApp.WriteProfileString( _T("Plugins"), Settings.MediaPlayer.Mpeg1PreviewCLSID, strRegData );
	theApp.WriteProfileString( _T("Plugins"), Settings.MediaPlayer.VisCLSID, strRegData );
	theApp.WriteProfileString( _T("Plugins"), Settings.MediaPlayer.VisSoniqueCLSID, strRegData );
	theApp.WriteProfileString( _T("Plugins"), Settings.MediaPlayer.VisWrapperCLSID, strRegData );

	CSettingsSheet* pSheet = GetSheet();
	for ( INT_PTR nPage = 0 ; nPage < pSheet->GetPageCount() ; nPage++ )
	{
		CSettingsPage* pPage = pSheet->GetPage( nPage );
		if ( pPage )
		{
			CString strClass = pPage->GetRuntimeClass()->m_lpszClassName;
			if ( strClass == _T("CPluginsSettingsPage") )
			{
				CPluginsSettingsPage* pPluginPage = static_cast< CPluginsSettingsPage* >( pPage );
				pPluginPage->UpdateList();
				break;
			}
		}
	}

	Settings.MediaPlayer.FileTypes.Empty();

	for ( int nItem = 0 ; nItem < m_wndList.GetCount() ; nItem++ )
	{
		CString str;
		m_wndList.GetLBText( nItem, str );

		if ( str.GetLength() )
		{
			if ( Settings.MediaPlayer.FileTypes.IsEmpty() )
				Settings.MediaPlayer.FileTypes += '|';
			Settings.MediaPlayer.FileTypes += str;
			Settings.MediaPlayer.FileTypes += '|';
		}
	}

	CSettingsPage::OnOK();
}

void CMediaSettingsPage::OnSelChangeMediaService()
{
	int nCustomIndex = ( m_wndServices.GetCount() == 2 ) ? 1 : 2;
	int nSelected = m_wndServices.GetCurSel();

	if ( nSelected == nCustomIndex )
	{
		CFileDialog dlg( TRUE, _T("exe"), _T("") , OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
			_T("Executable Files|*.exe;*.com|All Files|*.*||"), this );
		
		if ( dlg.DoModal() != IDOK )
		{
			m_wndServices.SetCurSel( 0 );
			return;
		}
		
		// Delete old file name first
		if ( nCustomIndex == 2 ) m_wndServices.DeleteString( 0 );
		m_wndServices.InsertString( 0, dlg.GetFileName() );
		m_wndServices.SetCurSel( 0 );
		m_sServicePath = dlg.GetPathName();

		m_bEnablePlay = m_bEnableEnqueue = FALSE;
		UpdateData( FALSE );

		GetDlgItem( IDC_MEDIA_PLAY )->EnableWindow( FALSE );
		GetDlgItem( IDC_MEDIA_ENQUEUE )->EnableWindow( FALSE );
		GetDlgItem( IDC_MEDIA_VIS )->EnableWindow( FALSE );
	}
	else if ( nSelected == 1 )
	{
		if ( nCustomIndex == 2 ) m_wndServices.DeleteString( 0 );
		m_sServicePath.Empty();

		m_bEnablePlay = m_bEnableEnqueue = TRUE;
		UpdateData( FALSE );

		GetDlgItem( IDC_MEDIA_PLAY )->EnableWindow( TRUE );
		GetDlgItem( IDC_MEDIA_ENQUEUE )->EnableWindow( TRUE );
		GetDlgItem( IDC_MEDIA_VIS )->EnableWindow( TRUE );
	}
}
