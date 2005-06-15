//
// PageFileComments.cpp
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
#include "SharedFile.h"
#include "ShellIcons.h"
#include "PageFileComments.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CFileCommentsPage, CFilePropertiesPage)

BEGIN_MESSAGE_MAP(CFileCommentsPage, CFilePropertiesPage)
	//{{AFX_MSG_MAP(CFileCommentsPage)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileCommentsPage property page

CFileCommentsPage::CFileCommentsPage() : CFilePropertiesPage( CFileCommentsPage::IDD )
{
	//{{AFX_DATA_INIT(CFileCommentsPage)
	m_sComments = _T("");
	m_nRating = -1;
	//}}AFX_DATA_INIT
}

CFileCommentsPage::~CFileCommentsPage()
{
}

void CFileCommentsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileCommentsPage)
	DDX_Control(pDX, IDC_FILE_COMMENTS, m_wndComments);
	DDX_Control(pDX, IDC_FILE_RATING, m_wndRating);
	DDX_Text(pDX, IDC_FILE_COMMENTS, m_sComments);
	DDX_CBIndex(pDX, IDC_FILE_RATING, m_nRating);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFileCommentsPage message handlers

BOOL CFileCommentsPage::OnInitDialog()
{
	CFilePropertiesPage::OnInitDialog();

	CLibraryList* pFiles = GetList();
	if ( pFiles == NULL ) return TRUE;

	if ( pFiles->GetCount() == 1 )
	{
		CQuickLock oLock( Library.m_pSection );
		CLibraryFile* pFile = GetFile();
		if ( pFile == NULL ) return TRUE;

		m_nRating	= pFile->m_nRating;
		m_sComments	= pFile->m_sComments;
	}
	else
	{
		m_wndComments.EnableWindow( FALSE );

		CQuickLock oLock( Library.m_pSection );

		for ( POSITION pos = pFiles->GetIterator() ; pos ; )
		{
			if ( CLibraryFile* pFile = pFiles->GetNextFile( pos ) )
			{
				m_nRating = pFile->m_nRating;
			}
		}

	}

	UpdateData( FALSE );

	return TRUE;
}

void CFileCommentsPage::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemWidth	= 1024;
	lpMeasureItemStruct->itemHeight	= 18;
}

void CFileCommentsPage::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( lpDrawItemStruct->itemID == (UINT)-1 ) return;
	if ( ( lpDrawItemStruct->itemAction & ODA_SELECT ) == 0 &&
		 ( lpDrawItemStruct->itemAction & ODA_DRAWENTIRE ) == 0 ) return;

	CRect rcItem( &lpDrawItemStruct->rcItem );
	CPoint pt( rcItem.left + 1, rcItem.top + 1 );
	CDC dc;

	dc.Attach( lpDrawItemStruct->hDC );
	if ( theApp.m_bRTL ) dc.SetLayout ( LAYOUT_RTL );

	int nRating = lpDrawItemStruct->itemID;

	CFont* pOldFont = (CFont*)dc.SelectObject( nRating > 0 ? &theApp.m_gdiFontBold : &theApp.m_gdiFont );
	dc.SetTextColor( GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT ) );

	dc.FillSolidRect( &rcItem,
		GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? COLOR_HIGHLIGHT : COLOR_WINDOW ) );
	dc.SetBkMode( TRANSPARENT );

	rcItem.DeflateRect( 4, 1 );

	if ( nRating > 1 )
	{
		for ( int nStar = nRating - 1 ; nStar ; nStar-- )
		{
			rcItem.right -= 16;
			ShellIcons.Draw( &dc, SHI_STAR, 16, rcItem.right, rcItem.top, CLR_NONE,
				( lpDrawItemStruct->itemState & ODS_SELECTED ) );
			rcItem.right -= 2;
		}
	}
	else if ( nRating == 1 )
	{
		rcItem.right -= 16;
		ShellIcons.Draw( &dc, SHI_FAKE, 16, rcItem.right, rcItem.top, CLR_NONE,
			( lpDrawItemStruct->itemState & ODS_SELECTED ) );
	}

	if ( ( lpDrawItemStruct->itemState & ODS_SELECTED ) == 0 )
	{
		static COLORREF crRating[7] =
		{
			RGB( 0, 0, 0 ),			// Unrated
			RGB( 255, 0, 0 ),		// 0 - Fake
			RGB( 128, 128, 128 ),	// 1 - Poor
			RGB( 0, 0, 0 ),			// 2 - Average
			RGB( 128, 128, 128 ),	// 3 - Good
			RGB( 0, 128, 0 ),		// 4 - Very good
			RGB( 0, 0, 255 ),		// 5 - Excellent
		};

		dc.SetTextColor( crRating[ nRating ] );
	}

	CString str;
	m_wndRating.GetLBText( nRating, str );
	dc.DrawText( str, &rcItem, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_NOPREFIX );

	dc.SelectObject( pOldFont );
	dc.Detach();
}

void CFileCommentsPage::OnOK()
{
	UpdateData();
	m_sComments.TrimLeft();
	m_sComments.TrimRight();

	CLibraryList* pFiles = GetList();

	if ( pFiles == NULL || pFiles->GetCount() == 1 )
	{
		CQuickLock oLock( Library.m_pSection );
		if ( CLibraryFile* pFile = GetFile() )
		{
			pFile->m_nRating	= m_nRating;
			pFile->m_sComments	= m_sComments;

			pFile->SaveMetadata();
			Library.Update();
		}
	}
	else
	{
		CQuickLock oLock( Library.m_pSection );

		for ( POSITION pos = pFiles->GetIterator() ; pos ; )
		{
			if ( CLibraryFile* pFile = pFiles->GetNextFile( pos ) )
			{
				pFile->m_nRating = m_nRating;
			}
		}

		Library.Update();
	}

	CFilePropertiesPage::OnOK();
}
