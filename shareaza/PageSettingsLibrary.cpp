//
// PageSettingsLibrary.cpp
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
#include "Library.h"
#include "LibraryHistory.h"
#include "LibraryBuilder.h"
#include "PageSettingsLibrary.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLibrarySettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CLibrarySettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CLibrarySettingsPage)
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibrarySettingsPage property page

CLibrarySettingsPage::CLibrarySettingsPage() : CSettingsPage(CLibrarySettingsPage::IDD)
{
	//{{AFX_DATA_INIT(CLibrarySettingsPage)
	//m_bSourceMesh = FALSE;
	m_bWatchFolders = FALSE;
	m_nRecentDays = 0;
	m_nRecentTotal = 0;
	m_bStoreViews = FALSE;
	m_bBrowseFiles = FALSE;
	m_bHighPriorityHash = FALSE;
	m_sCollectionPath = _T("");
	//}}AFX_DATA_INIT
}

CLibrarySettingsPage::~CLibrarySettingsPage()
{
}

void CLibrarySettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLibrarySettingsPage)
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
	DDX_Text(pDX, IDC_COLLECTIONS_FOLDER, m_sCollectionPath);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CLibrarySettingsPage message handlers

BOOL CLibrarySettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();
	
	m_bStoreViews		= Settings.Library.StoreViews;
	m_bWatchFolders		= Settings.Library.WatchFolders;
	m_bBrowseFiles		= Settings.Community.ServeFiles;
	m_bHighPriorityHash = Settings.Library.HighPriorityHash;

	m_nRecentTotal		= Settings.Library.HistoryTotal;
	m_nRecentDays		= Settings.Library.HistoryDays;

	m_sCollectionPath	= Settings.Downloads.CollectionPath;

	

	CString strList;
	
	for ( strList = Settings.Library.SafeExecute + '|' ; strList.GetLength() ; )
	{
		CString strType = strList.SpanExcluding( _T(" |") );
		strList = strList.Mid( strType.GetLength() + 1 );
		strType.TrimLeft();
		strType.TrimRight();
		if ( strType.GetLength() ) m_wndSafeList.AddString( strType );
	}
	
	for ( strList = Settings.Library.PrivateTypes + '|' ; strList.GetLength() ; )
	{
		CString strType = strList.SpanExcluding( _T(" |") );
		strList = strList.Mid( strType.GetLength() + 1 );
		strType.TrimLeft();
		strType.TrimRight();
		if ( strType.GetLength() ) m_wndPrivateList.AddString( strType );
	}

	m_wndCollectionPath.SetIcon( IDI_BROWSE );

	UpdateData( FALSE );

	m_wndRecentTotal.SetRange( 0, 100 );
	m_wndRecentDays.SetRange( 0, 365 );

	m_wndSafeAdd.EnableWindow( m_wndSafeList.GetWindowTextLength() > 0 );
	m_wndSafeRemove.EnableWindow( m_wndSafeList.GetCurSel() >= 0 );
	m_wndPrivateAdd.EnableWindow( m_wndPrivateList.GetWindowTextLength() > 0 );
	m_wndPrivateRemove.EnableWindow( m_wndPrivateList.GetCurSel() >= 0 );

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

	CharLower( strType.GetBuffer() );
	strType.ReleaseBuffer();
	strType.TrimLeft(); strType.TrimRight();
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

	CharLower( strType.GetBuffer() );
	strType.ReleaseBuffer();
	strType.TrimLeft(); strType.TrimRight();
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
	LibraryHistory.Clear();
	Library.Update();
}

void CLibrarySettingsPage::OnCollectionsBrowse() 
{
	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	LPMALLOC pMalloc;
	BROWSEINFO pBI;
		
	ZeroMemory( &pBI, sizeof(pBI) );
	pBI.hwndOwner		= AfxGetMainWnd()->GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= _T("Select folder for collections:");
	pBI.ulFlags			= BIF_RETURNONLYFSDIRS;
	
	pPath = SHBrowseForFolder( &pBI );

	if ( pPath == NULL ) return;

	SHGetPathFromIDList( pPath, szPath );
	SHGetMalloc( &pMalloc );
	pMalloc->Free( pPath );
	pMalloc->Release();
	
	UpdateData( TRUE );
	m_sCollectionPath = szPath;
	//m_bCollectionsChanged = TRUE;
	UpdateData( FALSE );
}

void CLibrarySettingsPage::OnOK() 
{
	UpdateData();

	Settings.Library.StoreViews		= m_bStoreViews;
	Settings.Library.WatchFolders	= m_bWatchFolders;
	Settings.Community.ServeFiles	= m_bBrowseFiles;
	Settings.Library.HighPriorityHash=m_bHighPriorityHash;

	Settings.Library.HistoryTotal	= m_nRecentTotal;
	Settings.Library.HistoryDays	= m_nRecentDays;

	Settings.Downloads.CollectionPath = m_sCollectionPath;

	//Set current hashing speed to requested
	LibraryBuilder.BoostPriority( m_bHighPriorityHash );

	Settings.Library.SafeExecute.Empty();

	for ( int nItem = 0 ; nItem < m_wndSafeList.GetCount() ; nItem++ )
	{
		CString str;
		m_wndSafeList.GetLBText( nItem, str );

		if ( str.GetLength() )
		{
			if ( Settings.Library.SafeExecute.IsEmpty() )
				Settings.Library.SafeExecute += '|';
			Settings.Library.SafeExecute += str;
			Settings.Library.SafeExecute += '|';
		}
	}
	
	Settings.Library.PrivateTypes.Empty();

	for ( int nItem = 0 ; nItem < m_wndPrivateList.GetCount() ; nItem++ )
	{
		CString str;
		m_wndPrivateList.GetLBText( nItem, str );

		if ( str.GetLength() )
		{
			if ( Settings.Library.PrivateTypes.IsEmpty() )
				Settings.Library.PrivateTypes += '|';
			Settings.Library.PrivateTypes += str;
			Settings.Library.PrivateTypes += '|';
		}
	}

	CSettingsPage::OnOK();
}

