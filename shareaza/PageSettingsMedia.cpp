//
// PageSettingsMedia.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
#include "WndSettingsPage.h"
#include "WndSettingsSheet.h"
#include "PageSettingsMedia.h"
#include "PageSettingsPlugins.h"
#include "DlgMediaVis.h"
#include "SchemaCache.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CMediaSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CMediaSettingsPage, CSettingsPage)
	ON_BN_CLICKED(IDC_MEDIA_PLAY, OnMediaPlay)
	ON_BN_CLICKED(IDC_MEDIA_ENQUEUE, OnMediaEnqueue)
	ON_CBN_SELCHANGE(IDC_MEDIA_TYPES, OnSelChangeMediaTypes)
	ON_CBN_EDITCHANGE(IDC_MEDIA_TYPES, OnEditChangeMediaTypes)
	ON_CBN_SELCHANGE(IDC_MEDIA_SERVICE, OnSelChangeMediaService)
	ON_BN_CLICKED(IDC_MEDIA_ADD, OnMediaAdd)
	ON_BN_CLICKED(IDC_MEDIA_REMOVE, OnMediaRemove)
	ON_BN_CLICKED(IDC_MEDIA_VIS, OnMediaVis)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

const int nCustomIndex = 0;
const int nSnareazaIndex = 1;

/////////////////////////////////////////////////////////////////////////////
// CMediaSettingsPage property page

CMediaSettingsPage::CMediaSettingsPage()
	: CSettingsPage(CMediaSettingsPage::IDD)
	, m_bEnablePlay( FALSE )
	, m_bEnableEnqueue( FALSE )
{
}

CMediaSettingsPage::~CMediaSettingsPage()
{
}

void CMediaSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MEDIA_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_MEDIA_ADD, m_wndAdd);
	DDX_Control(pDX, IDC_MEDIA_TYPES, m_wndList);
	DDX_CBString(pDX, IDC_MEDIA_TYPES, m_sType);
	DDX_Check(pDX, IDC_MEDIA_PLAY, m_bEnablePlay);
	DDX_Check(pDX, IDC_MEDIA_ENQUEUE, m_bEnableEnqueue);
	DDX_Control(pDX, IDC_MEDIA_SERVICE, m_wndServices);
}

void CMediaSettingsPage::Update()
{
	UpdateData();

	BOOL bShareaza = ( m_wndServices.GetCurSel() == nSnareazaIndex );

	GetDlgItem( IDC_MEDIA_PLAY )->EnableWindow( bShareaza );
	GetDlgItem( IDC_MEDIA_ENQUEUE )->EnableWindow( bShareaza );
	GetDlgItem( IDC_MEDIA_VIS )->EnableWindow( bShareaza );

	m_wndList.EnableWindow( m_bEnablePlay || m_bEnableEnqueue );
	m_wndAdd.EnableWindow( ( m_bEnablePlay || m_bEnableEnqueue ) && m_wndList.GetWindowTextLength() > 0 );
	m_wndRemove.EnableWindow( ( m_bEnablePlay || m_bEnableEnqueue ) && m_wndList.GetCurSel() >= 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CMediaSettingsPage message handlers

BOOL CMediaSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_bEnablePlay		= Settings.MediaPlayer.EnablePlay;
	m_bEnableEnqueue	= Settings.MediaPlayer.EnableEnqueue;

	for ( string_set::const_iterator i = Settings.MediaPlayer.FileTypes.begin() ;
		i != Settings.MediaPlayer.FileTypes.end(); ++i )
	{
		m_wndList.AddString( *i );
	}
	
	m_wndServices.AddString( _T("(") + LoadString( IDS_GENERAL_CUSTOM ) + _T("\x2026)") );
	m_wndServices.AddString( LoadString( IDS_MEDIA_SMPLAYER ) );
	int nSelected = nSnareazaIndex;
	for ( string_set::const_iterator i = Settings.MediaPlayer.ServicePath.begin() ;
		i != Settings.MediaPlayer.ServicePath.end(); ++i )
	{
		CString sPlayer = *i;
		int nAstrix = sPlayer.ReverseFind( _T('*') );
		sPlayer.Remove( _T('*') );
		int nIndex = m_wndServices.AddString( PathFindFileName( sPlayer ) );
		if ( nAstrix != -1 )	// Selected player
			nSelected = nIndex;
		m_wndServices.SetItemDataPtr( nIndex, new CString( sPlayer ) );
	}
	m_wndServices.SetCurSel( nSelected );

	UpdateData( FALSE );

	Update();

	return TRUE;
}

void CMediaSettingsPage::OnMediaPlay()
{
	Update();
}

void CMediaSettingsPage::OnMediaEnqueue()
{
	Update();
}

void CMediaSettingsPage::OnSelChangeMediaTypes()
{
	Update();
}

void CMediaSettingsPage::OnEditChangeMediaTypes()
{
	Update();
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
	UpdateData();

	int nItem = m_wndList.GetCurSel();
	if ( nItem >= 0 ) m_wndList.DeleteString( nItem );
	m_wndRemove.EnableWindow( FALSE );
}

void CMediaSettingsPage::OnMediaVis()
{
	UpdateData();

	CMediaVisDlg dlg( NULL );
	dlg.DoModal();
}

void CMediaSettingsPage::OnOK()
{
	UpdateData();

	Settings.MediaPlayer.EnablePlay		= ( m_bEnablePlay != FALSE );
	Settings.MediaPlayer.EnableEnqueue	= ( m_bEnableEnqueue != FALSE );

	int nSelected = m_wndServices.GetCurSel();
	if ( nSelected == nCustomIndex )
		nSelected = nSnareazaIndex;

	Settings.MediaPlayer.ServicePath.clear();
	int nCount = m_wndServices.GetCount();
	for( int i = 0; i < nCount; ++i )
	{
		CString* psPlayer = (CString*)m_wndServices.GetItemDataPtr( i );
		if ( ! psPlayer )
			continue;
		if ( i == nSelected )
			*psPlayer += _T("*");
		Settings.MediaPlayer.ServicePath.insert( *psPlayer );
	}

	if ( nSelected == nSnareazaIndex )	// Shareaza Media Player is selected
		Settings.MediaPlayer.ShortPaths = FALSE;

	CSettingsSheet* pSheet = GetSheet();
	for ( INT_PTR nPage = 0 ; nPage < pSheet->GetPageCount() ; nPage++ )
	{
		CSettingsPage* pPage = pSheet->GetPage( nPage );
		if ( pPage )
		{
			CString strClass( pPage->GetRuntimeClass()->m_lpszClassName );
			if ( strClass == _T("CPluginsSettingsPage") )
			{
				CPluginsSettingsPage* pPluginPage = static_cast< CPluginsSettingsPage* >( pPage );
				pPluginPage->UpdateList();
				break;
			}
		}
	}

	Settings.MediaPlayer.FileTypes.clear();
	for ( int nItem = 0 ; nItem < m_wndList.GetCount() ; nItem++ )
	{
		CString str;
		m_wndList.GetLBText( nItem, str );
		if ( str.GetLength() )
		{
			Settings.MediaPlayer.FileTypes.insert( str );
		}
	}

	CSettingsPage::OnOK();
}

void CMediaSettingsPage::OnSelChangeMediaService()
{
	UpdateData();

	int nSelected = m_wndServices.GetCurSel();
	if ( nSelected == nCustomIndex ) // Custom Media Player selected
	{
		CFileDialog dlg( TRUE, _T("exe"), NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
			SchemaCache.GetFilter( CSchema::uriApplicationAll ) +
			SchemaCache.GetFilter( CSchema::uriAllFiles ) +
			_T("|"), this );
		if ( dlg.DoModal() != IDOK )
		{
			// User cancel selection
			m_wndServices.SetCurSel( nSnareazaIndex );
			Update();
			return;
		}

		CString sNewPlayer = dlg.GetPathName();

		int nCount = m_wndServices.GetCount();
		for ( int i = 0; i < nCount; ++i )
		{
			CString* psPlayer = (CString*)m_wndServices.GetItemDataPtr( i );
			if ( ! psPlayer )
				continue;
			if ( psPlayer->CompareNoCase( sNewPlayer ) == 0 )
			{
				// Duplicate found
				m_wndServices.SetCurSel( i );
				Update();
				return;
			}
		}

		int nIndex = m_wndServices.AddString( PathFindFileName( sNewPlayer ) );
		m_wndServices.SetItemDataPtr( nIndex, new CString( sNewPlayer ) );
		m_wndServices.SetCurSel( nIndex );

		m_bEnablePlay = m_bEnableEnqueue = FALSE;
	}
	else if ( nSelected == nSnareazaIndex ) // Shareaza Media Player selected
	{
		m_bEnablePlay = m_bEnableEnqueue = TRUE;
	}
	else	// Not Shareaza, not custom
	{
		m_bEnablePlay = m_bEnableEnqueue = FALSE;
	}

	UpdateData( FALSE );

	Update();
}

void CMediaSettingsPage::OnDestroy()
{
	int nCount = m_wndServices.GetCount();
	for ( int i = 0; i < nCount; ++i )
	{
		delete (CString*)m_wndServices.GetItemDataPtr( i );
	}

	CSettingsPage::OnDestroy();
}
