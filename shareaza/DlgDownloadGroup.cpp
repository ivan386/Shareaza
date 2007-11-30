//
// DlgDownloadGroup.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "LibraryFolders.h"
#include "SharedFolder.h"
#include "DownloadGroup.h"
#include "DownloadGroups.h"
#include "DlgDownloadGroup.h"
#include "DlgHelp.h"

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
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_ICON_LIST, OnLvnItemchangingIconList)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadGroupDlg dialog

CDownloadGroupDlg::CDownloadGroupDlg(CDownloadGroup* pGroup, CWnd* pParent) :
	CSkinDialog( CDownloadGroupDlg::IDD, pParent ),
	m_pGroup( pGroup )
{
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
	m_wndImages.SetColumnWidth( 0, 36 );
	m_wndImages.SetExtendedStyle( LVS_EX_CHECKBOXES );
	m_wndBrowse.SetCoolIcon( IDI_BROWSE, theApp.m_bRTL );

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

	CList<CString>	pUsedIcons;
	int nSelectedIndex = 0;
	for ( POSITION pos = SchemaCache.GetIterator() ; pos ; )
	{
		CSchema* pSchema = SchemaCache.GetNext( pos );

		if ( pUsedIcons.Find( pSchema->m_sIcon ) == NULL ||
			 pSchema->m_sURI == m_pGroup->m_sSchemaURI )
		{
			pUsedIcons.AddTail( pSchema->m_sIcon );

			int nIndex = m_wndImages.InsertItem( LVIF_IMAGE|LVIF_PARAM,
				m_wndImages.GetItemCount(), NULL, 0, 0, pSchema->m_nIcon16,
				(LPARAM)pSchema );

			if ( pSchema->m_sURI == m_pGroup->m_sSchemaURI )
			{
				m_wndImages.SetItemState( nIndex, LVIS_SELECTED, LVIS_SELECTED );
				m_wndImages.SetCheck( nIndex );
				nSelectedIndex = nIndex;
			}
		}
	}
	m_wndImages.EnsureVisible( nSelectedIndex, FALSE );

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

		BOOL bAdd = ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES );
		if ( bAdd )
		{
			if ( LibraryFolders.IsSubFolderShared( m_sFolder ) )
			{
				CString strFormat, strMessage;
				LoadString( strFormat, IDS_LIBRARY_SUBFOLDER_IN_LIBRARY );
				strMessage.Format( strFormat, (LPCTSTR)m_sFolder );

				bAdd = ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES );
				if ( bAdd )
				{
					CLibraryFolder* pFolder;
					while ( ( pFolder = LibraryFolders.IsSubFolderShared( m_sFolder ) ) != NULL )
					{
						LibraryFolders.RemoveFolder( pFolder );
					}
				}
			}
			if ( bAdd )
			{
				if ( !LibraryFolders.IsShareable( m_sFolder ) )
				{
					pLock.Unlock();
					CHelpDlg::Show( _T("ShareHelp.BadShare") );
					bAdd = FALSE;
				}
			}
			if ( bAdd )
			{
				if ( CLibraryFolder* pFolder = LibraryFolders.AddFolder( m_sFolder ) )
				{
					LoadString( strMessage, IDS_LIBRARY_DOWNLOADS_SHARE );

					BOOL bShare = AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES;

					CQuickLock oLock( Library.m_pSection );
					if ( LibraryFolders.CheckFolder( pFolder, TRUE ) )
						pFolder->SetShared( bShare ? TRI_TRUE : TRI_FALSE );
					Library.Update();
				}
			}
		}
	}

	CSkinDialog::OnOK();
}

void CDownloadGroupDlg::OnBrowse()
{
	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	CComPtr< IMalloc > pMalloc;

	BROWSEINFO pBI = {};
	pBI.hwndOwner		= AfxGetMainWnd()->GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= _T("Select folder for downloads:");
	pBI.ulFlags			= BIF_RETURNONLYFSDIRS;

	pPath = SHBrowseForFolder( &pBI );

	if ( pPath == NULL ) return;

	SHGetPathFromIDList( pPath, szPath );
	if ( SUCCEEDED( SHGetMalloc( &pMalloc ) ) )
		pMalloc->Free( pPath );

	// Make sure download/incomplete folders aren't the same
	if ( _tcsicmp( szPath, Settings.Downloads.IncompletePath ) == 0 )
	{
		CString strMessage;
		LoadString( strMessage, IDS_SETTINGS_FILEPATH_NOT_SAME );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return;
	}

	// If the group folder and download folders are the same, use the default download folder
	if ( _tcsicmp( szPath, Settings.Downloads.CompletePath ) == 0 )
	{
		UpdateData( TRUE );
		m_sFolder.Empty();
		UpdateData( FALSE );
		return;
	}

	UpdateData( TRUE );
	m_sFolder = szPath;
	UpdateData( FALSE );
}

void CDownloadGroupDlg::OnLvnItemchangingIconList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;
	
	int nLength = m_wndImages.GetItemCount();

	// Set check and selection states on first mouse click
	if ( ( pNMLV->uOldState == 0x1000 && pNMLV->uNewState == 0x2000 ) ||
		 ( pNMLV->uOldState == 0x0000 && pNMLV->uNewState == 0x0003 ) )
	{
		int nOldIndex = -1;
		for ( int nIndex = 0; nIndex < nLength; ++nIndex )
		{
			if ( pNMLV->iItem != nIndex && m_wndImages.GetCheck( nIndex ) )
			{
				nOldIndex = nIndex;
				break;
			}
		}
		m_wndImages.SetCheck( pNMLV->iItem );
		if ( nOldIndex != -1 )
			m_wndImages.SetCheck( nOldIndex, FALSE );
		m_wndImages.SetItemState( pNMLV->iItem, LVIS_SELECTED, LVIS_SELECTED );
	}
	// Disable removing check on other mouse clicks
	else if ( pNMLV->uOldState == 0x2000 && pNMLV->uNewState == 0x1000 )
	{
		int nCount = 0;
		for ( int nIndex = 0; nIndex < nLength; ++nIndex )
		{
			if ( m_wndImages.GetCheck( nIndex ) )
			{
				nCount++;
			}
		}
		if ( nCount == 1 )
			*pResult = 1;
	}
}
