//
// DlgDownloadGroup.cpp
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
	ON_BN_CLICKED(IDC_FILTER_ADD, &CDownloadGroupDlg::OnFilterAdd)
	ON_BN_CLICKED(IDC_FILTER_REMOVE, &CDownloadGroupDlg::OnFilterRemove)
	ON_CBN_EDITCHANGE(IDC_FILTER_LIST, &CDownloadGroupDlg::OnEditChangeFilterList)
	ON_CBN_SELCHANGE(IDC_FILTER_LIST, &CDownloadGroupDlg::OnSelChangeFilterList)
	ON_BN_CLICKED(IDC_DOWNLOADS_BROWSE, &CDownloadGroupDlg::OnBrowse)
	ON_CBN_CLOSEUP(IDC_SCHEMAS, &CDownloadGroupDlg::OnCbnCloseupSchemas)
	ON_BN_CLICKED(IDC_DOWNLOADS_DEFAULT, &CDownloadGroupDlg::OnBnClickedDownloadDefault)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadGroupDlg dialog

CDownloadGroupDlg::CDownloadGroupDlg(CDownloadGroup* pGroup, CWnd* pParent) :
	CSkinDialog( CDownloadGroupDlg::IDD, pParent ),
	m_pGroup( pGroup ),
	m_bTorrent( FALSE )
{
}

void CDownloadGroupDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_DOWNLOADS_BROWSE, m_wndBrowse);
	DDX_Control(pDX, IDC_DOWNLOADS_DEFAULT, m_wndCancel);
	DDX_Control(pDX, IDC_FOLDER, m_wndFolder);
	DDX_Control(pDX, IDC_FILTER_ADD, m_wndFilterAdd);
	DDX_Control(pDX, IDC_FILTER_REMOVE, m_wndFilterRemove);
	DDX_Control(pDX, IDC_FILTER_LIST, m_wndFilterList);
	DDX_Control(pDX, IDC_SCHEMAS, m_wndSchemas);
	DDX_Text(pDX, IDC_NAME, m_sName);
	DDX_Text(pDX, IDC_FOLDER, m_sFolder);
	DDX_Check(pDX, IDC_DOWNLOADS_TORRENT, m_bTorrent);
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadGroupDlg message handlers

BOOL CDownloadGroupDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CDownloadGroupDlg") );

	LoadString( m_wndSchemas.m_sNoSchemaText, IDS_SEARCH_PANEL_AFT );
	m_wndSchemas.Load( m_pGroup->m_sSchemaURI );
	m_sOldSchemaURI = m_pGroup->m_sSchemaURI;
	
	m_wndBrowse.SetCoolIcon( IDI_BROWSE, Settings.General.LanguageRTL );
	m_wndCancel.SetCoolIcon( IDI_FAKE, Settings.General.LanguageRTL );

	CSingleLock pLock( &DownloadGroups.m_pSection, TRUE );

	if ( ! DownloadGroups.Check( m_pGroup ) )
	{
		EndDialog( IDCANCEL );
		return TRUE;
	}

	m_sName		= m_pGroup->m_sName;
	m_sFolder	= m_pGroup->m_sFolder;
	m_bTorrent	= m_pGroup->m_bTorrent;

	for ( POSITION pos = m_pGroup->m_pFilters.GetHeadPosition() ; pos ; )
	{
		m_wndFilterList.AddString( m_pGroup->m_pFilters.GetNext( pos ) );
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

	int nNewSelected = CB_ERR;
	if ( nItem > CB_ERR )
	{
		nNewSelected = m_wndFilterList.DeleteString( nItem );
		if ( nItem == 0 && nNewSelected > 0 )
			nNewSelected = 0; // first one
		else
			nNewSelected = nItem - 1;
	}

	m_wndFilterList.SetCurSel( nNewSelected );
	m_wndFilterRemove.EnableWindow( nNewSelected != CB_ERR );
}

void CDownloadGroupDlg::OnOK()
{
	UpdateData();

	CSingleLock pLock( &DownloadGroups.m_pSection, TRUE );

	if ( DownloadGroups.Check( m_pGroup ) )
	{
		m_pGroup->m_sName	= m_sName;
		m_pGroup->m_sFolder	= m_sFolder;
		m_pGroup->m_bTorrent = m_bTorrent;

		m_pGroup->m_pFilters.RemoveAll();
		for ( int nItem = 0 ; nItem < m_wndFilterList.GetCount() ; nItem++ )
		{
			CString str;
			m_wndFilterList.GetLBText( nItem, str );
			m_pGroup->AddFilter( str );
		}

		// Change schema and remove old schema filters (preserve custom ones)
		m_pGroup->SetSchema( m_wndSchemas.GetSelectedURI(), TRUE );

		// Why should we force users to have groups named after the schema?
		// Because we add new schema related types without asking?
		if ( m_sName.GetLength() && m_pGroup->m_sName != m_sName )
			m_pGroup->m_sName = m_sName;
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
	UpdateData();

	CString strPath( BrowseForFolder( _T("Select folder for downloads:"), m_sFolder ) );
	if ( strPath.IsEmpty() )
		return;

	// Make sure download/incomplete folders aren't the same
	if ( _tcsicmp( strPath, Settings.Downloads.IncompletePath ) == 0 )
	{
		CString strMessage;
		LoadString( strMessage, IDS_SETTINGS_FILEPATH_NOT_SAME );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return;
	}

	// If the group folder and schema default download folders are the same,
	// use the default download folder
	CString sSchema = m_wndSchemas.GetSelectedURI();
	if ( sSchema != CSchema::uriBitTorrent &&
		 sSchema != CSchema::uriCollection && 
		! strPath.CompareNoCase( Settings.Downloads.CompletePath ) )
		m_sFolder.Empty();
	else
		m_sFolder = strPath;

	UpdateData( FALSE );
}

void CDownloadGroupDlg::OnCbnCloseupSchemas()
{
	// Get filters
	CList< CString > oList;
	for ( int nItem = 0 ; nItem < m_wndFilterList.GetCount() ; nItem++ )
	{
		CString strFilter;
		m_wndFilterList.GetLBText( nItem, strFilter );
		if ( oList.Find( strFilter ) == NULL )
			oList.AddTail( strFilter );
	}

	// Remove old schema filters (preserve custom ones)
	if ( CSchemaPtr pOldSchema = SchemaCache.Get( m_sOldSchemaURI ) )
	{
		for ( POSITION pos1 = pOldSchema->GetFilterIterator(); pos1 ; )
		{
			CString strFilter;
			BOOL bResult;
			pOldSchema->GetNextFilter( pos1, strFilter, bResult );
			if ( bResult )
			{
				strFilter.Insert( 0, _T('.') );
				while ( POSITION pos2 = oList.Find( strFilter ) )
					oList.RemoveAt( pos2 );
			}
		}
	}

	// Add new schema filters
	if ( CSchemaPtr pNewSchema = SchemaCache.Get( m_wndSchemas.GetSelectedURI() ) )
	{
		for ( POSITION pos = pNewSchema->GetFilterIterator(); pos ; )
		{
			CString strFilter;
			BOOL bResult;
			pNewSchema->GetNextFilter( pos, strFilter, bResult );
			if ( bResult )
			{
				strFilter.Insert( 0, _T('.') );
				oList.AddTail( strFilter );
			}
		}
	}

	// Refill interface filters list
	m_wndFilterList.ResetContent();
	for ( POSITION pos = oList.GetHeadPosition() ; pos ; )
		m_wndFilterList.AddString( oList.GetNext( pos ) );

	m_sOldSchemaURI = m_wndSchemas.GetSelectedURI();
}

void CDownloadGroupDlg::OnBnClickedDownloadDefault()
{
	UpdateData();

	CString sSchema = m_wndSchemas.GetSelectedURI();
	if ( sSchema == CSchema::uriBitTorrent )
		m_sFolder = Settings.Downloads.TorrentPath;
	else if ( sSchema == CSchema::uriCollection )
		m_sFolder = Settings.Downloads.CollectionPath;
	else
		m_sFolder.Empty();

	UpdateData( FALSE );
}
