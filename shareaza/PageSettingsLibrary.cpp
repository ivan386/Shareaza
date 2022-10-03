//
// PageSettingsLibrary.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "Library.h"
#include "LibraryHistory.h"
#include "LibraryFolders.h"
#include "LibraryBuilder.h"
#include "PageSettingsLibrary.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLibrarySettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CLibrarySettingsPage, CSettingsPage)
	ON_CBN_SELCHANGE(IDC_SAFE_TYPES, OnSelChangeSafeTypes)
	ON_CBN_EDITCHANGE(IDC_SAFE_TYPES, OnEditChangeSafeTypes)
	ON_BN_CLICKED(IDC_SAFE_ADD, OnSafeAdd)
	ON_BN_CLICKED(IDC_SAFE_REMOVE, OnSafeRemove)
	ON_CBN_SELCHANGE(IDC_PRIVATE_TYPES, OnSelChangePrivateTypes)
	ON_CBN_EDITCHANGE(IDC_PRIVATE_TYPES, OnEditChangePrivateTypes)
	ON_BN_CLICKED(IDC_PRIVATE_ADD, OnPrivateAdd)
	ON_BN_CLICKED(IDC_PRIVATE_REMOVE, OnPrivateRemove)
	ON_BN_CLICKED(IDC_RECENT_CLEAR, OnRecentClear)
	ON_BN_CLICKED(IDC_COLLECTIONS_BROWSE, OnCollectionsBrowse)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibrarySettingsPage property page

CLibrarySettingsPage::CLibrarySettingsPage()
	: CSettingsPage			( CLibrarySettingsPage::IDD )
	, m_bMakeGhosts			( FALSE )
	, m_bWatchFolders		( FALSE )
	, m_nRecentDays			( 0 )
	, m_nRecentTotal		( 0 )
	, m_bStoreViews			( FALSE )
	, m_bBrowseFiles		( FALSE )
	, m_bHighPriorityHash	( FALSE )
	, m_bSmartSeries		( FALSE )
{
}

void CLibrarySettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_RECENT_TOTAL_SPIN, m_wndRecentTotal);
	DDX_Control(pDX, IDC_RECENT_DAYS_SPIN, m_wndRecentDays);
	DDX_Control(pDX, IDC_SAFE_REMOVE, m_wndSafeRemove);
	DDX_Control(pDX, IDC_SAFE_ADD, m_wndSafeAdd);
	DDX_Control(pDX, IDC_SAFE_TYPES, m_wndSafeList);
	DDX_Control(pDX, IDC_PRIVATE_REMOVE, m_wndPrivateRemove);
	DDX_Control(pDX, IDC_PRIVATE_ADD, m_wndPrivateAdd);
	DDX_Control(pDX, IDC_PRIVATE_TYPES, m_wndPrivateList);
	DDX_Check(pDX, IDC_WATCH_FOLDERS, m_bWatchFolders);
	DDX_Text(pDX, IDC_RECENT_DAYS, m_nRecentDays);
	DDX_Text(pDX, IDC_RECENT_TOTAL, m_nRecentTotal);
	DDX_Check(pDX, IDC_STORE_VIEWS, m_bStoreViews);
	DDX_Check(pDX, IDC_BROWSE_FILES, m_bBrowseFiles);
	DDX_Check(pDX, IDC_HIGH_HASH, m_bHighPriorityHash);
	DDX_Control(pDX, IDC_COLLECTIONS_BROWSE, m_wndCollectionPath);
	DDX_Control(pDX, IDC_COLLECTIONS_FOLDER, m_wndCollectionFolder);
	DDX_Check(pDX, IDC_MAKE_GHOSTS, m_bMakeGhosts);
	DDX_Check(pDX, IDC_SMART_SERIES_DETECTION, m_bSmartSeries);
	DDX_Control(pDX, IDC_RECENT_CLEAR, m_wndRecentClear);
}

/////////////////////////////////////////////////////////////////////////////
// CLibrarySettingsPage message handlers

BOOL CLibrarySettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	CQuickLock oLock( Library.m_pSection );

	AddAndSelect( m_wndCollectionFolder, Settings.General.UserPath + _T("\\Collections") );
	AddAndSelect( m_wndCollectionFolder, Settings.Downloads.CollectionPath );

	m_bStoreViews		= Settings.Library.StoreViews;
	m_bWatchFolders		= Settings.Library.WatchFolders;
	m_bBrowseFiles		= Settings.Community.ServeFiles;
	m_bHighPriorityHash = Settings.Library.HighPriorityHash;
	m_bMakeGhosts		= Settings.Library.CreateGhosts;
	m_bSmartSeries		= Settings.Library.SmartSeriesDetection;
	m_nRecentTotal		= Settings.Library.HistoryTotal;
	m_nRecentDays		= Settings.Library.HistoryDays;

	for ( string_set::const_iterator i = Settings.Library.SafeExecute.begin() ;
		i != Settings.Library.SafeExecute.end(); ++i )
	{
		m_wndSafeList.AddString( *i );
	}

	for ( string_set::const_iterator i = Settings.Library.PrivateTypes.begin() ;
		i != Settings.Library.PrivateTypes.end(); ++i )
	{
		m_wndPrivateList.AddString( *i );
	}

	m_wndCollectionPath.SetIcon( IDI_BROWSE );

	UpdateData( FALSE );

	Settings.SetRange( &Settings.Library.HistoryTotal, m_wndRecentTotal );
	Settings.SetRange( &Settings.Library.HistoryDays, m_wndRecentDays );

	m_wndSafeAdd.EnableWindow( m_wndSafeList.GetWindowTextLength() > 0 );
	m_wndSafeRemove.EnableWindow( m_wndSafeList.GetCurSel() >= 0 );
	m_wndPrivateAdd.EnableWindow( m_wndPrivateList.GetWindowTextLength() > 0 );
	m_wndPrivateRemove.EnableWindow( m_wndPrivateList.GetCurSel() >= 0 );

	m_wndRecentClear.EnableWindow( TRUE );

	return TRUE;
}

void CLibrarySettingsPage::OnSelChangeSafeTypes()
{
	m_wndSafeRemove.EnableWindow( m_wndSafeList.GetCurSel() >= 0 );
}

void CLibrarySettingsPage::OnEditChangeSafeTypes()
{
	m_wndSafeAdd.EnableWindow( m_wndSafeList.GetWindowTextLength() > 0 );
}

void CLibrarySettingsPage::OnSafeAdd()
{
	CString strType;
	m_wndSafeList.GetWindowText( strType );

	ToLower( strType );

	strType.Trim();
	if ( strType.IsEmpty() ) return;

	if ( m_wndSafeList.FindStringExact( -1, strType ) >= 0 ) return;

	m_wndSafeList.AddString( strType );
	m_wndSafeList.SetWindowText( _T("") );
}

void CLibrarySettingsPage::OnSafeRemove()
{
	int nItem = m_wndSafeList.GetCurSel();
	if ( nItem >= 0 ) m_wndSafeList.DeleteString( nItem );
	m_wndSafeRemove.EnableWindow( FALSE );
}

void CLibrarySettingsPage::OnSelChangePrivateTypes()
{
	m_wndPrivateRemove.EnableWindow( m_wndPrivateList.GetCurSel() >= 0 );
}

void CLibrarySettingsPage::OnEditChangePrivateTypes()
{
	m_wndPrivateAdd.EnableWindow( m_wndPrivateList.GetWindowTextLength() > 0 );
}

void CLibrarySettingsPage::OnPrivateAdd()
{
	CString strType;
	m_wndPrivateList.GetWindowText( strType );

	ToLower( strType );

	strType.Trim();
	if ( strType.IsEmpty() ) return;

	if ( m_wndPrivateList.FindStringExact( -1, strType ) >= 0 ) return;

	m_wndPrivateList.AddString( strType );
	m_wndPrivateList.SetWindowText( _T("") );
}

void CLibrarySettingsPage::OnPrivateRemove()
{
	int nItem = m_wndPrivateList.GetCurSel();
	if ( nItem >= 0 ) m_wndPrivateList.DeleteString( nItem );
	m_wndPrivateRemove.EnableWindow( FALSE );
}

void CLibrarySettingsPage::OnRecentClear()
{
	CQuickLock oLock( Library.m_pSection );
	Settings.ClearSearches();
	LibraryHistory.Clear();
	LibraryFolders.ClearGhosts();
	Library.Update();

	PostMainWndMessage( WM_COMMAND, ID_LIBRARY_REFRESH );

	m_wndRecentClear.EnableWindow( FALSE );
}

void CLibrarySettingsPage::OnCollectionsBrowse()
{
	CString sCollectionPath;
	m_wndCollectionFolder.GetWindowText( sCollectionPath );
	sCollectionPath = BrowseForFolder( IDS_SELECT_FOLDER_COLLECTIONS, sCollectionPath );
	if ( sCollectionPath.IsEmpty() )
		return;

	AddAndSelect( m_wndCollectionFolder, sCollectionPath );
}

void CLibrarySettingsPage::OnOK()
{
	UpdateData();

	CString sCollectionPath;
	m_wndCollectionFolder.GetWindowText( sCollectionPath );

	Settings.Library.StoreViews			= m_bStoreViews != FALSE;
	Settings.Library.WatchFolders		= m_bWatchFolders != FALSE;
	Settings.Community.ServeFiles		= m_bBrowseFiles != FALSE;
	Settings.Library.HighPriorityHash	= m_bHighPriorityHash != FALSE;
	Settings.Library.CreateGhosts		= m_bMakeGhosts != FALSE;
	Settings.Library.SmartSeriesDetection = m_bSmartSeries != FALSE;
	Settings.Library.HistoryTotal		= m_nRecentTotal;
	Settings.Library.HistoryDays		= m_nRecentDays;
	Settings.Downloads.CollectionPath	= sCollectionPath;

	//Set current hashing speed to requested
	LibraryBuilder.BoostPriority( m_bHighPriorityHash != FALSE );

	Settings.Library.SafeExecute.clear();

	for ( int nItem = 0 ; nItem < m_wndSafeList.GetCount() ; nItem++ )
	{
		CString str;
		m_wndSafeList.GetLBText( nItem, str );
		if ( str.GetLength() )
		{
			Settings.Library.SafeExecute.insert( str );
		}
	}

	Settings.Library.PrivateTypes.clear();

	for ( int nItem = 0 ; nItem < m_wndPrivateList.GetCount() ; nItem++ )
	{
		CString str;
		m_wndPrivateList.GetLBText( nItem, str );
		if ( str.GetLength() )
		{
			Settings.Library.PrivateTypes.insert( str );
		}
	}

	CSettingsPage::OnOK();
}
