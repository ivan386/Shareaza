//
// PageSettingsGeneral.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "PageSettingsGeneral.h"
#include "DlgHelp.h"
#include "Schema.h"
#include "SchemaCache.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CGeneralSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CGeneralSettingsPage, CSettingsPage)
	ON_CBN_DROPDOWN(IDC_CLOSE_MODE, &CGeneralSettingsPage::OnDropdownCloseMode)
	ON_CBN_DROPDOWN(IDC_TRAY_MINIMISE, &CGeneralSettingsPage::OnDropdownTrayMinimise)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGeneralSettingsPage property page

CGeneralSettingsPage::CGeneralSettingsPage()
	: CSettingsPage			( CGeneralSettingsPage::IDD )
	, m_bRatesInBytes		( -1 )
	, m_bExpandMatches		( FALSE )
	, m_bAutoConnect		( FALSE )
	, m_nCloseMode			( -1 )
	, m_bTrayMinimise		( -1 )
	, m_bSwitchToTransfers	( FALSE )
	, m_bExpandDownloads	( FALSE )
	, m_bNewWindow			( FALSE )
	, m_bStartup			( FALSE )
	, m_bPromptURLs			( FALSE )
	, m_bHideSearch			( FALSE )
	, m_bAdultFilter		( FALSE )
	, m_nTipDelay			( 0 )
{
}

CGeneralSettingsPage::~CGeneralSettingsPage()
{
}

void CGeneralSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);

	DDX_CBIndex(pDX, IDC_RATES_IN_BYTES, m_bRatesInBytes);
	DDX_Check(pDX, IDC_EXPAND_MATCHES, m_bExpandMatches);
	DDX_Check(pDX, IDC_AUTO_CONNECT, m_bAutoConnect);
	DDX_CBIndex(pDX, IDC_CLOSE_MODE, m_nCloseMode);
	DDX_CBIndex(pDX, IDC_TRAY_MINIMISE, m_bTrayMinimise);
	DDX_Check(pDX, IDC_SWITCH_TO_TRANSFERS, m_bSwitchToTransfers);
	DDX_Check(pDX, IDC_EXPAND_DOWNLOAD, m_bExpandDownloads);
	DDX_Check(pDX, IDC_NEW_WINDOW, m_bNewWindow);
	DDX_Check(pDX, IDC_AUTO_START, m_bStartup);
	DDX_Check(pDX, IDC_PROMPT_URLS, m_bPromptURLs);
	DDX_Check(pDX, IDC_HIDE_SEARCH, m_bHideSearch);
	DDX_Check(pDX, IDC_ADULT_FILTER, m_bAdultFilter);
	DDX_Control(pDX, IDC_TIP_DELAY_SPIN, m_wndTipSpin);
	DDX_Control(pDX, IDC_TIP_DISPLAY, m_wndTips);
	DDX_Control(pDX, IDC_TIP_ALPHA, m_wndTipAlpha);
	DDX_Text(pDX, IDC_TIP_DELAY, m_nTipDelay);
	DDX_Control(pDX, IDC_CLOSE_MODE, m_wndCloseMode);
	DDX_Control(pDX, IDC_TRAY_MINIMISE, m_wndTrayMinimise);
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
	m_bExpandMatches		= Settings.Search.ExpandMatches;
	m_bSwitchToTransfers	= Settings.Search.SwitchToTransfers;
	m_bExpandDownloads		= Settings.Downloads.AutoExpand;
	m_bNewWindow			= Settings.Downloads.ShowMonitorURLs;
	m_bPromptURLs			= ! Settings.General.AlwaysOpenURLs;
	m_bHideSearch			= Settings.Search.HideSearchPanel;
	m_bAdultFilter			= Settings.Search.AdultFilter;
	
	m_bRatesInBytes			= Settings.General.RatesInBytes
							+ Settings.General.RatesUnit * 2;
	
	CRect rc;
	CString strTitle( _T("Search Results") );

	m_wndTips.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + 1;
	
	m_wndTips.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, rc.right, 0 );
	m_wndTips.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES|LVS_EX_LABELTIP, 
		LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES|LVS_EX_LABELTIP );
	
	if ( CSchemaPtr pSchema = SchemaCache.Get( CSchema::uriSearchFolder ) )
	{
		strTitle = pSchema->m_sTitle;
		int nColon = strTitle.Find( ':' );
		if ( nColon >= 0 ) 
			strTitle = strTitle.Mid( nColon + 1 ).Trim();
	}

	Add( strTitle, Settings.Interface.TipSearch );
	LoadString( strTitle, IDR_LIBRARYFRAME );
	Add( strTitle, Settings.Interface.TipLibrary );
	LoadString( strTitle, IDR_DOWNLOADSFRAME );
	Add( strTitle, Settings.Interface.TipDownloads );
	LoadString( strTitle, IDR_UPLOADSFRAME );
	Add( strTitle, Settings.Interface.TipUploads );
	LoadString( strTitle, IDR_NEIGHBOURSFRAME );
	Add( strTitle, Settings.Interface.TipNeighbours );
	LoadString( strTitle, IDR_MEDIAFRAME );
	Add( strTitle, Settings.Interface.TipMedia );
	
	Settings.SetRange( &Settings.Interface.TipDelay, m_wndTipSpin );
	m_nTipDelay	= Settings.Interface.TipDelay;
	
	Settings.SetRange( &Settings.Interface.TipAlpha, m_wndTipAlpha );
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
	Settings.Connection.AutoConnect		= m_bAutoConnect != FALSE;
	Settings.General.CloseMode			= m_nCloseMode;
	Settings.General.TrayMinimise		= m_bTrayMinimise != FALSE;
	Settings.Search.ExpandMatches		= m_bExpandMatches != FALSE;
	Settings.Search.SwitchToTransfers	= m_bSwitchToTransfers != FALSE;
	Settings.Downloads.AutoExpand		= m_bExpandDownloads != FALSE;
	Settings.Downloads.ShowMonitorURLs	= m_bNewWindow != FALSE;
	Settings.General.AlwaysOpenURLs		= ! m_bPromptURLs;
	Settings.Search.HideSearchPanel		= m_bHideSearch != FALSE;
	Settings.Search.AdultFilter			= m_bAdultFilter != FALSE;
	
	Settings.General.RatesInBytes		= m_bRatesInBytes % 2 == 1;
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

void CGeneralSettingsPage::OnDropdownCloseMode()
{
	RecalcDropWidth( &m_wndCloseMode );
}

void CGeneralSettingsPage::OnDropdownTrayMinimise()
{
	RecalcDropWidth( &m_wndTrayMinimise );
}
