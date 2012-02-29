//
// DlgFilePropertiesPage.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "SharedFile.h"
#include "ShellIcons.h"
#include "DlgFilePropertiesSheet.h"
#include "DlgFilePropertiesPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CFilePropertiesPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CFilePropertiesPage, CPropertyPageAdv)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesPage property page

CFilePropertiesPage::CFilePropertiesPage(UINT nIDD) : CPropertyPageAdv( nIDD )
{
}

CFilePropertiesPage::~CFilePropertiesPage()
{
}

void CFilePropertiesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange(pDX);
}

/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesPage helper functions

CLibraryFile* CFilePropertiesPage::GetFile()
{
	CLibraryListPtr pList( GetList() );
	if ( pList->GetCount() != 1 ) return NULL;
	CQuickLock oLock( Library.m_pSection );
	CLibraryFile* pFile = Library.LookupFile( pList->GetHead() );
	if ( pFile != NULL ) return pFile;
	PostMessage( WM_CLOSE );
	return NULL;
}

CLibraryList* CFilePropertiesPage::GetList() const
{
	CFilePropertiesSheet* pSheet = (CFilePropertiesSheet*)GetParent();
	return pSheet->m_pList;
}

/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesPage message handlers

BOOL CFilePropertiesPage::OnInitDialog()
{
	CPropertyPageAdv::OnInitDialog();

	CSingleLock oLock( &Library.m_pSection, TRUE );
	if ( CLibraryFile* pFile = GetFile() )
	{
		if ( CWnd* pNameWnd = GetDlgItem( IDC_FILE_NAME ) )
		{
			if ( Settings.General.LanguageRTL )
			{
				CRect rc, rcPage;
				pNameWnd->GetWindowRect( &rc );
				GetWindowRect( &rcPage );
				pNameWnd->MoveWindow( rcPage.right - rc.right, 
					rc.top - rcPage.top, rc.Width(), rc.Height(), FALSE );
				pNameWnd->ModifyStyleEx( WS_EX_RTLREADING, WS_EX_LTRREADING, 0 );
			}
			pNameWnd->SetWindowText( pFile->m_sName );
		}
		m_nIcon = ShellIcons.Get( pFile->GetPath(), 48 );

		oLock.Unlock();
	}
	else
	{
		oLock.Unlock();
		CLibraryListPtr pList( GetList() );
		if ( pList )
		{
			if ( CWnd* pNameWnd = GetDlgItem( IDC_FILE_NAME ) )
			{
				if ( Settings.General.LanguageRTL )
				{
					CRect rc, rcPage;
					pNameWnd->GetWindowRect( &rc );
					GetWindowRect( &rcPage );
					pNameWnd->MoveWindow( rcPage.right - rc.right, 
						rc.top - rcPage.top, rc.Width(), rc.Height(), FALSE );
					pNameWnd->ModifyStyleEx( 0, WS_EX_RTLREADING, 0 );
				}
				CString strFormat, strMessage;
				LoadString( strFormat, IDS_LIBRARY_METADATA_EDIT );
				strMessage.Format( strFormat, pList->GetCount() );
				pNameWnd->SetWindowText( strMessage );

			}
		}
	}

	return TRUE;
}

