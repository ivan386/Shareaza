//
// PageSettingsGeneral.cpp
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
#include "PageSettingsGeneral.h"
#include "DlgHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CGeneralSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CGeneralSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CGeneralSettingsPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGeneralSettingsPage property page

CGeneralSettingsPage::CGeneralSettingsPage() : CSettingsPage(CGeneralSettingsPage::IDD)
{
	//{{AFX_DATA_INIT(CGeneralSettingsPage)
	m_bRatesInBytes = -1;
	m_bExpandMatches = FALSE;
	m_bAutoConnect = FALSE;
	m_nCloseMode = -1;
	m_bTrayMinimise = -1;
	m_bSwitchToTransfers = FALSE;
	m_bExpandDownloads = FALSE;
	m_bStartup = FALSE;
	m_bPromptURLs = FALSE;
	m_bHideSearch = FALSE;
	m_bAdultFilter = FALSE;
	m_nTipDelay = 0;
	m_bHighlightNew = FALSE;
	//}}AFX_DATA_INIT
}

CGeneralSettingsPage::~CGeneralSettingsPage()
{
}

void CGeneralSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGeneralSettingsPage)
	DDX_CBIndex(pDX, IDC_RATES_IN_BYTES, m_bRatesInBytes);
	DDX_Check(pDX, IDC_EXPAND_MATCHES, m_bExpandMatches);
	DDX_Check(pDX, IDC_AUTO_CONNECT, m_bAutoConnect);
	DDX_CBIndex(pDX, IDC_CLOSE_MODE, m_nCloseMode);
	DDX_CBIndex(pDX, IDC_TRAY_MINIMISE, m_bTrayMinimise);
	DDX_Check(pDX, IDC_SWITCH_TO_TRANSFERS, m_bSwitchToTransfers);
	DDX_Check(pDX, IDC_EXPAND_DOWNLOAD, m_bExpandDownloads);
	DDX_Check(pDX, IDC_AUTO_START, m_bStartup);
	DDX_Check(pDX, IDC_PROMPT_URLS, m_bPromptURLs);
	DDX_Check(pDX, IDC_HIDE_SEARCH, m_bHideSearch);
	DDX_Check(pDX, IDC_ADULT_FILTER, m_bAdultFilter);
	DDX_Control(pDX, IDC_TIP_DELAY_SPIN, m_wndTipSpin);
	DDX_Control(pDX, IDC_TIP_DISPLAY, m_wndTips);
	DDX_Control(pDX, IDC_TIP_ALPHA, m_wndTipAlpha);
	DDX_Text(pDX, IDC_TIP_DELAY, m_nTipDelay);
	DDX_Check(pDX, IDC_HIGHLIGHT_NEW, m_bHighlightNew);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CGeneralSettingsPage message handlers

BOOL CGeneralSettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();
	
	m_bStartup				= Settings.CheckStartup();
	m_bAutoConnect			= Settings.Connection.AutoConnect;
	m_nCloseMode			= Settings.General.CloseMode;
	m_bTrayMinimise			= Settings.General.TrayMinimise;
	m_bHighlightNew			= Settings.Search.HighlightNew;
	m_bExpandMatches		= Settings.Search.ExpandMatches;
	m_bSwitchToTransfers	= Settings.Search.SwitchToTransfers;
	m_bExpandDownloads		= Settings.Downloads.AutoExpand;
	m_bPromptURLs			= ! Settings.General.AlwaysOpenURLs;
	m_bHideSearch			= Settings.Search.HideSearchPanel;
	m_bAdultFilter			= Settings.Search.AdultFilter;
	
	m_bRatesInBytes			= Settings.General.RatesInBytes
							+ Settings.General.RatesUnit * 2;
	
	CRect rc;
	m_wndTips.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + 1;
	
	m_wndTips.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, rc.right, 0 );
	m_wndTips.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES );
	
	Add( _T("Search Results"), Settings.Interface.TipSearch );
	Add( _T("Library"), Settings.Interface.TipLibrary );
	Add( _T("Downloads"), Settings.Interface.TipDownloads );
	Add( _T("Uploads"), Settings.Interface.TipUploads );
	Add( _T("Neighbours"), Settings.Interface.TipNeighbours );
	Add( _T("Media Player"), Settings.Interface.TipMedia );
	
	m_wndTipSpin.SetRange( 100, 5000 );
	m_nTipDelay	= Settings.Interface.TipDelay;
	
	m_wndTipAlpha.SetRange( 0, 255 );
	m_wndTipAlpha.SetPos( Settings.Interface.TipAlpha );
	
	UpdateData( FALSE );
	
	return TRUE;
}

void CGeneralSettingsPage::Add(LPCTSTR pszName, BOOL bState)
{
	int nItem = m_wndTips.InsertItem( LVIF_TEXT, m_wndTips.GetItemCount(),
		pszName, 0, 0, 0, 0 );
	
	if ( bState )
		m_wndTips.SetItemState( nItem, 2 << 12, LVIS_STATEIMAGEMASK );
}

void CGeneralSettingsPage::OnOK() 
{
	UpdateData();

	if ( ( Settings.Search.AdultFilter == FALSE ) && ( m_bAdultFilter == TRUE ) 
		&& ( Settings.Live.AdultWarning == FALSE ) )
	{
		Settings.Live.AdultWarning = TRUE;
		CHelpDlg::Show( _T("GeneralHelp.AdultFilter") );
	}
	
	Settings.SetStartup( m_bStartup );
	Settings.Connection.AutoConnect		= m_bAutoConnect;
	Settings.General.CloseMode			= m_nCloseMode;
	Settings.General.TrayMinimise		= m_bTrayMinimise;
	Settings.Search.HighlightNew		= m_bHighlightNew;
	Settings.Search.ExpandMatches		= m_bExpandMatches;
	Settings.Search.SwitchToTransfers	= m_bSwitchToTransfers;
	Settings.Downloads.AutoExpand		= m_bExpandDownloads;
	Settings.General.AlwaysOpenURLs		= ! m_bPromptURLs;
	Settings.Search.HideSearchPanel		= m_bHideSearch;
	Settings.Search.AdultFilter			= m_bAdultFilter;
	
	Settings.General.RatesInBytes		= m_bRatesInBytes % 2;
	Settings.General.RatesUnit			= m_bRatesInBytes / 2;
	
	Settings.Interface.TipSearch		= m_wndTips.GetItemState( 0, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );
	Settings.Interface.TipLibrary		= m_wndTips.GetItemState( 1, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );
	Settings.Interface.TipDownloads		= m_wndTips.GetItemState( 2, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );
	Settings.Interface.TipUploads		= m_wndTips.GetItemState( 3, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );
	Settings.Interface.TipNeighbours	= m_wndTips.GetItemState( 4, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );
	Settings.Interface.TipMedia			= m_wndTips.GetItemState( 5, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );
	
	Settings.Interface.TipDelay	= m_nTipDelay;
	Settings.Interface.TipAlpha	= m_wndTipAlpha.GetPos();
	
	CSettingsPage::OnOK();
}

