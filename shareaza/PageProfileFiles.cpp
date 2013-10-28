//
// PageProfileFiles.cpp
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
#include "Library.h"
#include "LibraryFolders.h"
#include "AlbumFolder.h"
#include "SharedFile.h"
#include "Schema.h"
#include "ShellIcons.h"
#include "PageProfileFiles.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CFilesProfilePage, CSettingsPage)

BEGIN_MESSAGE_MAP(CFilesProfilePage, CSettingsPage)
	//{{AFX_MSG_MAP(CFilesProfilePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFilesProfilePage property page

CFilesProfilePage::CFilesProfilePage() : CSettingsPage( CFilesProfilePage::IDD )
{
	//{{AFX_DATA_INIT(CFilesProfilePage)
	//}}AFX_DATA_INIT
}

CFilesProfilePage::~CFilesProfilePage()
{
}

void CFilesProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilesProfilePage)
	DDX_Control(pDX, IDC_FILE_LIST, m_wndList);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFilesProfilePage message handlers

BOOL CFilesProfilePage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	CRect rc;
	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + 1;
	m_wndList.InsertColumn( 0, _T("File"), LVCFMT_LEFT, rc.right, -1 );
	ShellIcons.AttachTo( &m_wndList, 16 );

	{
		CQuickLock oLock( Library.m_pSection );

		CAlbumFolder* pFolder = LibraryFolders.GetAlbumTarget( CSchema::uriFavouritesFolder, _T("Title"), NULL );

		if ( pFolder != NULL )
		{
			for ( POSITION pos = pFolder->GetFileIterator() ; pos ; )
			{
				CLibraryFile* pFile = pFolder->GetNextFile( pos );

				if ( pFile->IsShared() )
				{
					m_wndList.InsertItem( LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM, m_wndList.GetItemCount(),
						pFile->m_sName, 0, 0, ShellIcons.Get( pFile->GetPath(), 16 ), pFile->m_nIndex );
				}
			}
		}
	}

	UpdateData( FALSE );

	return TRUE;
}

void CFilesProfilePage::OnOK()
{
	CSettingsPage::OnOK();
}
