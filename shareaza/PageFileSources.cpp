//
// PageFileSources.cpp
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
#include "Library.h"
#include "SharedFile.h"
#include "PageFileSources.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CFileSourcesPage, CFilePropertiesPage)

BEGIN_MESSAGE_MAP(CFileSourcesPage, CFilePropertiesPage)
	//{{AFX_MSG_MAP(CFileSourcesPage)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FILE_SOURCES, OnItemChangedFileSources)
	ON_EN_CHANGE(IDC_FILE_SOURCE, OnChangeFileSource)
	ON_BN_CLICKED(IDC_SOURCE_REMOVE, OnSourceRemove)
	ON_BN_CLICKED(IDC_SOURCE_NEW, OnSourceNew)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileSourcesPage property page

CFileSourcesPage::CFileSourcesPage() : CFilePropertiesPage(CFileSourcesPage::IDD)
{
	//{{AFX_DATA_INIT(CFileSourcesPage)
	m_sSource = _T("");
	//}}AFX_DATA_INIT
}

CFileSourcesPage::~CFileSourcesPage()
{
}

void CFileSourcesPage::DoDataExchange(CDataExchange* pDX)
{
	CFilePropertiesPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileSourcesPage)
	DDX_Control(pDX, IDC_SOURCE_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_SOURCE_NEW, m_wndNew);
	DDX_Control(pDX, IDC_FILE_SOURCES, m_wndList);
	DDX_Text(pDX, IDC_FILE_SOURCE, m_sSource);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFileSourcesPage message handlers

BOOL CFileSourcesPage::OnInitDialog() 
{
	CFilePropertiesPage::OnInitDialog();

	CRect rc;
	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );
	m_wndList.InsertColumn( 0, _T("URL"), LVCFMT_LEFT, rc.right - 128, -1 );
	m_wndList.InsertColumn( 1, _T("Expires"), LVCFMT_RIGHT, 128, 0 );
	
	m_gdiImageList.Create( 16, 16, ILC_COLOR16|ILC_MASK, 1, 1 );
	m_gdiImageList.Add( theApp.LoadIcon( IDI_WEB_URL ) );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	m_wndList.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT );

	CLibraryFile* pFile = GetFile();
	if ( pFile == NULL ) return TRUE;

	for ( POSITION pos = pFile->m_pSources.GetHeadPosition() ; pos ; )
	{
		AddSource( (CSharedSource*)pFile->m_pSources.GetNext( pos ) );
	}

	Library.Unlock();

	Skin.Translate( _T("CFileSourcesPageList"), m_wndList.GetHeaderCtrl() );
	m_wndNew.EnableWindow( FALSE );
	m_wndRemove.EnableWindow( FALSE );
	
	return TRUE;
}

void CFileSourcesPage::AddSource(CSharedSource* pSource)
{
	LV_ITEM pItem;
	
	ZeroMemory( &pItem, sizeof(pItem) );
	pItem.mask		= LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
	pItem.pszText	= (LPTSTR)(LPCTSTR)pSource->m_sURL;
	pItem.iImage	= 0;
	pItem.lParam	= (LPARAM)pSource;
	pItem.iItem		= m_wndList.GetItemCount();
	pItem.iItem		= m_wndList.InsertItem( &pItem );
	
	FILETIME pFileTime = pSource->m_pTime;
	CString strDate, strTime;
	SYSTEMTIME pSystemTime;
	
	(LONGLONG&)pFileTime += (LONGLONG)Settings.Library.SourceExpire * 10000000;
	FileTimeToSystemTime( &pFileTime, &pSystemTime );
	SystemTimeToTzSpecificLocalTime( NULL, &pSystemTime, &pSystemTime );
	
	GetDateFormat( LOCALE_USER_DEFAULT, 0, &pSystemTime, _T("yyyy-MM-dd"), strDate.GetBuffer( 64 ), 64 );
	GetTimeFormat( LOCALE_USER_DEFAULT, 0, &pSystemTime, _T("HH:mm"), strTime.GetBuffer( 64 ), 64 );
	strDate.ReleaseBuffer(); strTime.ReleaseBuffer();
	
	strDate += ' ';
	strDate += strTime;

	m_wndList.SetItemText( pItem.iItem, 1, strDate );
}

void CFileSourcesPage::OnItemChangedFileSources(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	m_wndRemove.EnableWindow( m_wndList.GetSelectedCount() > 0 );
	*pResult = 0;
}

void CFileSourcesPage::OnChangeFileSource() 
{
	UpdateData();
	m_wndNew.EnableWindow( m_sSource.Find( _T("http://") ) == 0 );
}

void CFileSourcesPage::OnSourceRemove() 
{
	CLibraryFile* pFile = GetFile();
	if ( pFile == NULL ) return;

	for ( int nItem ; ( nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED ) ) >= 0 ; )
	{
		CSharedSource* pSource = (CSharedSource*)m_wndList.GetItemData( nItem );
		
		if ( POSITION pos = pFile->m_pSources.Find( pSource ) )
		{
			delete pSource;
			pFile->m_pSources.RemoveAt( pos );
		}
		
		m_wndList.DeleteItem( nItem );
	}

	Library.Unlock( TRUE );
	m_wndRemove.EnableWindow( FALSE );
}

void CFileSourcesPage::OnSourceNew() 
{
	UpdateData();

	if ( m_sSource.Find( _T("http://") ) == 0 )
	{
		if ( CLibraryFile* pFile = GetFile() )
		{
			CSharedSource* pSource = pFile->AddAlternateSources( m_sSource );
			m_sSource.Empty();
			Library.Unlock( TRUE );
			UpdateData( FALSE );
			if ( pSource ) AddSource( pSource );
		}
	}
}
