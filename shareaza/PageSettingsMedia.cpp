//
// PageSettingsMedia.cpp
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
#include "PageSettingsMedia.h"
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

	UpdateData( FALSE );

	m_wndList.EnableWindow( m_bEnablePlay || m_bEnableEnqueue );
	m_wndAdd.EnableWindow( ( m_bEnablePlay || m_bEnableEnqueue ) && m_wndList.GetWindowTextLength() > 0 );
	m_wndRemove.EnableWindow( ( m_bEnablePlay || m_bEnableEnqueue ) && m_wndList.GetCurSel() >= 0 );
	
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
	m_wndRemove.EnableWindow( ( m_bEnablePlay || m_bEnableEnqueue ) && m_wndList.GetCurSel() >= 0 );
}

void CMediaSettingsPage::OnEditChangeMediaTypes() 
{
	m_wndAdd.EnableWindow( ( m_bEnablePlay || m_bEnableEnqueue ) && m_wndList.GetWindowTextLength() > 0 );
}

void CMediaSettingsPage::OnMediaAdd() 
{
	UpdateData();

	m_sType.MakeLower();
	m_sType.TrimLeft();
	m_sType.TrimRight();
	if ( m_sType.IsEmpty() ) return;

	if ( m_wndList.FindString( -1, m_sType ) >= 0 ) return;

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

	Settings.MediaPlayer.EnablePlay		= m_bEnablePlay;
	Settings.MediaPlayer.EnableEnqueue	= m_bEnableEnqueue;
	
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
