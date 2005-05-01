//
// DlgFilePropertiesPage.cpp
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
#include "Library.h"
#include "SharedFile.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "DlgFilePropertiesSheet.h"
#include "DlgFilePropertiesPage.h"
#include ".\dlgfilepropertiespage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CFilePropertiesPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CFilePropertiesPage, CPropertyPage)
	//{{AFX_MSG_MAP(CFilePropertiesPage)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesPage property page

CFilePropertiesPage::CFilePropertiesPage(UINT nIDD) : CPropertyPage( nIDD )
{
	//{{AFX_DATA_INIT(CFilePropertiesPage)
	//}}AFX_DATA_INIT
	m_nIcon = -1;
}

CFilePropertiesPage::~CFilePropertiesPage()
{
}

void CFilePropertiesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilePropertiesPage)
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesPage helper functions

CLibraryFile* CFilePropertiesPage::GetFile()
{
	CLibraryList* pList = GetList();
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
	return &pSheet->m_pList;
}

/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesPage message handlers

BOOL CFilePropertiesPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	Skin.Apply( NULL, this );

	CSingleLock oLock( &Library.m_pSection, TRUE );
	if ( CLibraryFile* pFile = GetFile() )
	{
		if ( CWnd* pNameWnd = GetDlgItem( IDC_FILE_NAME ) )
		{
			pNameWnd->SetWindowText( pFile->m_sName );
		}

		m_nIcon = ShellIcons.Get( pFile->m_sName, 48 );

		oLock.Unlock();
	}
	else
	{
		oLock.Unlock();
		if ( CLibraryList* pList = GetList() )
		{
			if ( CWnd* pNameWnd = GetDlgItem( IDC_FILE_NAME ) )
			{
				CString strFormat, strMessage;
				LoadString( strFormat, IDS_LIBRARY_METADATA_EDIT );
				strMessage.Format( strFormat, pList->GetCount() );
				pNameWnd->SetWindowText( strMessage );
			}

			m_nIcon = SHI_EXECUTABLE;
		}
	}

	return TRUE;
}

void CFilePropertiesPage::OnPaint()
{
	CPaintDC dc( this );

	if ( m_nIcon >= 0 )
	{
		ShellIcons.Draw( &dc, m_nIcon, 48, 4, 4 );
	}

	for ( CWnd* pWnd = GetWindow( GW_CHILD ) ; pWnd ; pWnd = pWnd->GetNextWindow() )
	{
		if ( pWnd->GetStyle() & WS_VISIBLE ) continue;

		TCHAR szClass[16];
		GetClassName( pWnd->GetSafeHwnd(), szClass, 16 );
		if ( _tcsicmp( szClass, _T("STATIC") ) ) continue;

		CString str;
		CRect rc;

		pWnd->GetWindowText( str );
		pWnd->GetWindowRect( &rc );
		ScreenToClient( &rc );

		if ( str.IsEmpty() || str.GetAt( 0 ) != '-' )
			PaintStaticHeader( &dc, &rc, str );
	}

	dc.SetBkColor( CCoolInterface::GetDialogBkColor() );
}

void CFilePropertiesPage::PaintStaticHeader(CDC* pDC, CRect* prc, LPCTSTR psz)
{
	CFont* pOldFont = (CFont*)pDC->SelectObject( GetFont() );
	CSize sz = pDC->GetTextExtent( psz );

	pDC->SetBkMode( OPAQUE );
	pDC->SetBkColor( Skin.m_crBannerBack );
	pDC->SetTextColor( Skin.m_crBannerText );

	CRect rc( prc );
	rc.bottom	= rc.top + min( rc.Height(), 16 );
	rc.right	= rc.left + sz.cx + 10;

	pDC->ExtTextOut( rc.left + 4, rc.top + 1, ETO_CLIPPED|ETO_OPAQUE,
		&rc, psz, _tcslen( psz ), NULL );

	rc.SetRect( rc.right, rc.top, prc->right, rc.top + 1 );
	pDC->ExtTextOut( rc.left, rc.top, ETO_OPAQUE, &rc, NULL, 0, NULL );

	pDC->SelectObject( pOldFont );
}

HBRUSH CFilePropertiesPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPage::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC )
	{
		pDC->SetBkColor( Skin.m_crDialog );
		hbr = Skin.m_brDialog;
	}

	return hbr;
}
