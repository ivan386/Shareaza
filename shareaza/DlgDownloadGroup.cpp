//
// DlgDownloadGroup.cpp
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
#include "Library.h"
#include "LibraryFolders.h"
#include "SharedFolder.h"
#include "DownloadGroup.h"
#include "DownloadGroups.h"
#include "DlgDownloadGroup.h"

#include "Schema.h"
#include "SchemaCache.h"
#include "ShellIcons.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CDownloadGroupDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CDownloadGroupDlg)
	ON_BN_CLICKED(IDC_FILTER_ADD, OnFilterAdd)
	ON_BN_CLICKED(IDC_FILTER_REMOVE, OnFilterRemove)
	ON_CBN_EDITCHANGE(IDC_FILTER_LIST, OnEditChangeFilterList)
	ON_CBN_SELCHANGE(IDC_FILTER_LIST, OnSelChangeFilterList)
	ON_BN_CLICKED(IDC_DOWNLOADS_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadGroupDlg dialog

CDownloadGroupDlg::CDownloadGroupDlg(CDownloadGroup* pGroup, CWnd* pParent) : CSkinDialog(CDownloadGroupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDownloadGroupDlg)
	m_sName = _T("");
	m_sFolder = _T("");
	//}}AFX_DATA_INIT
	m_pGroup = pGroup;
}

void CDownloadGroupDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDownloadGroupDlg)
	DDX_Control(pDX, IDC_DOWNLOADS_BROWSE, m_wndBrowse);
	DDX_Control(pDX, IDC_ICON_LIST, m_wndImages);
	DDX_Control(pDX, IDC_FOLDER, m_wndFolder);
	DDX_Control(pDX, IDC_FILTER_ADD, m_wndFilterAdd);
	DDX_Control(pDX, IDC_FILTER_REMOVE, m_wndFilterRemove);
	DDX_Control(pDX, IDC_FILTER_LIST, m_wndFilterList);
	DDX_Text(pDX, IDC_NAME, m_sName);
	DDX_Text(pDX, IDC_FOLDER, m_sFolder);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadGroupDlg message handlers

BOOL CDownloadGroupDlg::OnInitDialog() 
{
	CSkinDialog::OnInitDialog();
	
	SkinMe( _T("CDownloadGroupDlg") );
	
	m_wndImages.SetImageList( ShellIcons.GetObject( 16 ), LVSIL_SMALL );
	m_wndBrowse.SetIcon( IDI_BROWSE );
	
	CSingleLock pLock( &DownloadGroups.m_pSection, TRUE );
	
	if ( ! DownloadGroups.Check( m_pGroup ) )
	{
		EndDialog( IDCANCEL );
		return TRUE;
	}
	
	m_sName		= m_pGroup->m_sName;
	m_sFolder	= m_pGroup->m_sFolder;
	
	for ( POSITION pos = m_pGroup->m_pFilters.GetHeadPosition() ; pos ; )
	{
		m_wndFilterList.AddString( m_pGroup->m_pFilters.GetNext( pos ) );
	}
	
	for ( pos = SchemaCache.GetIterator() ; pos ; )
	{
		CSchema* pSchema = SchemaCache.GetNext( pos );
		
		int nIndex = m_wndImages.InsertItem( LVIF_IMAGE|LVIF_PARAM|LVIF_TEXT,
			m_wndImages.GetItemCount(), _T("Icon"), 0, 0, pSchema->m_nIcon16,
			(LPARAM)pSchema );
		
		if ( pSchema->m_sURI == m_pGroup->m_sSchemaURI )
		{
			m_wndImages.SetItemState( nIndex, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
	
	UpdateData( FALSE );
	
	BOOL bSuper = DownloadGroups.GetSuperGroup() == m_pGroup;
	
	m_wndFolder.EnableWindow( ! bSuper );
	m_wndFilterList.EnableWindow( ! bSuper );
	m_wndFilterAdd.EnableWindow( m_wndFilterList.GetWindowTextLength() > 0 );
	m_wndFilterRemove.EnableWindow( m_wndFilterList.GetCurSel() >= 0 );
	
	return TRUE;
}

void CDownloadGroupDlg::OnEditChangeFilterList() 
{
	m_wndFilterAdd.EnableWindow( m_wndFilterList.GetWindowTextLength() > 0 );
}

void CDownloadGroupDlg::OnSelChangeFilterList() 
{
	m_wndFilterRemove.EnableWindow( m_wndFilterList.GetCurSel() >= 0 );
}

void CDownloadGroupDlg::OnFilterAdd() 
{
	CString strType;
	m_wndFilterList.GetWindowText( strType );
	
	strType.TrimLeft(); strType.TrimRight();
	if ( strType.IsEmpty() ) return;
	
	if ( m_wndFilterList.FindString( -1, strType ) >= 0 ) return;
	
	m_wndFilterList.AddString( strType );
	m_wndFilterList.SetWindowText( _T("") );
}

void CDownloadGroupDlg::OnFilterRemove() 
{
	int nItem = m_wndFilterList.GetCurSel();
	if ( nItem >= 0 ) m_wndFilterList.DeleteString( nItem );
	m_wndFilterRemove.EnableWindow( FALSE );
}

void CDownloadGroupDlg::OnOK() 
{
	UpdateData();
	
	CSingleLock pLock( &DownloadGroups.m_pSection, TRUE );
	
	if ( DownloadGroups.Check( m_pGroup ) )
	{
		m_pGroup->m_sName	= m_sName;
		m_pGroup->m_sFolder	= m_sFolder;
		
		m_pGroup->m_pFilters.RemoveAll();
		
		for ( int nItem = 0 ; nItem < m_wndFilterList.GetCount() ; nItem++ )
		{
			CString str;
			m_wndFilterList.GetLBText( nItem, str );
			if ( str.GetLength() ) m_pGroup->m_pFilters.AddTail( str );
		}
		
		int nIndex = m_wndImages.GetNextItem( -1, LVNI_SELECTED );
		
		if ( nIndex >= 0 )
		{
			CSchema* pSchema = (CSchema*)m_wndImages.GetItemData( nIndex );
			m_pGroup->SetSchema( pSchema->m_sURI );
		}
		else
		{
			m_pGroup->SetSchema( _T("") );
		}
	}
	
	if ( m_sFolder.GetLength() && ! LibraryFolders.IsFolderShared( m_sFolder ) )
	{
		CString strFormat, strMessage;
		
		LoadString( strFormat, IDS_LIBRARY_DOWNLOADS_ADD );
		strMessage.Format( strFormat, (LPCTSTR)m_sFolder );
		
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
		{
			if ( CLibraryFolder* pFolder = LibraryFolders.AddFolder( m_sFolder ) )
			{
				LoadString( strMessage, IDS_LIBRARY_DOWNLOADS_SHARE );
				
				BOOL bShare = AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES;
				
				Library.Lock();
				if ( LibraryFolders.CheckFolder( pFolder, TRUE ) )
					pFolder->m_bShared = bShare ? TS_TRUE : TS_FALSE;
				Library.Unlock( TRUE );
			}
		}
	}
	
	CSkinDialog::OnOK();
}

void CDownloadGroupDlg::OnBrowse() 
{
	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	LPMALLOC pMalloc;
	BROWSEINFO pBI;
		
	ZeroMemory( &pBI, sizeof(pBI) );
	pBI.hwndOwner		= AfxGetMainWnd()->GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= _T("Select folder for downloads:");
	pBI.ulFlags			= BIF_RETURNONLYFSDIRS;
	
	pPath = SHBrowseForFolder( &pBI );
	
	if ( pPath == NULL ) return;
	
	SHGetPathFromIDList( pPath, szPath );
	SHGetMalloc( &pMalloc );
	pMalloc->Free( pPath );
	pMalloc->Release();
	
	UpdateData( TRUE );
	m_sFolder = szPath;
	UpdateData( FALSE );
}
