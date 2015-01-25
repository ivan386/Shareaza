//
// PageFileComments.cpp
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
#include "CoolInterface.h"
#include "PageFileComments.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CFileCommentsPage, CFilePropertiesPage)

BEGIN_MESSAGE_MAP(CFileCommentsPage, CFilePropertiesPage)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileCommentsPage property page

CFileCommentsPage::CFileCommentsPage() :
	CFilePropertiesPage( CFileCommentsPage::IDD ),
	m_sComments(), m_nRating( -1 )
{
}

CFileCommentsPage::~CFileCommentsPage()
{
}

void CFileCommentsPage::DoDataExchange(CDataExchange* pDX)
{
	CFilePropertiesPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_FILE_COMMENTS, m_wndComments);
	DDX_Control(pDX, IDC_FILE_RATING, m_wndRating);
	DDX_Text(pDX, IDC_FILE_COMMENTS, m_sComments);
	DDX_CBIndex(pDX, IDC_FILE_RATING, m_nRating);
}

/////////////////////////////////////////////////////////////////////////////
// CFileCommentsPage message handlers

BOOL CFileCommentsPage::OnInitDialog()
{
	CFilePropertiesPage::OnInitDialog();

	CLibraryListPtr pFiles( GetList() );
	if ( ! pFiles ) return TRUE;

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

		for ( POSITION pos = pFiles->GetHeadPosition() ; pos ; )
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

void CFileCommentsPage::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemWidth	= 1024;
	lpMeasureItemStruct->itemHeight	= 18;
}

void CFileCommentsPage::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( lpDrawItemStruct->itemID == (UINT)-1 ) return;
	if ( ( lpDrawItemStruct->itemAction & ODA_SELECT ) == 0 &&
		 ( lpDrawItemStruct->itemAction & ODA_DRAWENTIRE ) == 0 ) return;

	CRect rcItem( &lpDrawItemStruct->rcItem );
	CDC dc;

	dc.Attach( lpDrawItemStruct->hDC );
	if ( Settings.General.LanguageRTL )
		SetLayout( dc.m_hDC, LAYOUT_RTL );

	int nRating = lpDrawItemStruct->itemID;

	CFont* pOldFont = (CFont*)dc.SelectObject( nRating > 0 ? &theApp.m_gdiFontBold : &theApp.m_gdiFont );
	dc.SetTextColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? CoolInterface.m_crHiText : CoolInterface.m_crText );
	dc.FillSolidRect( &rcItem, ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? CoolInterface.m_crHighlight : CoolInterface.m_crSysWindow );
	dc.SetBkMode( TRANSPARENT );

	rcItem.DeflateRect( 4, 1 );

	if ( nRating > 1 )
	{
		for ( int nStar = nRating - 1 ; nStar ; nStar-- )
		{
			rcItem.right -= 16;
			CoolInterface.Draw( &dc, IDI_STAR, 16, rcItem.right, rcItem.top, CLR_NONE,
				( lpDrawItemStruct->itemState & ODS_SELECTED ) );
			rcItem.right -= 2;
		}
	}
	else if ( nRating == 1 )
	{
		rcItem.right -= 16;
		CoolInterface.Draw( &dc, IDI_FAKE, 16, rcItem.right, rcItem.top, CLR_NONE,
			( lpDrawItemStruct->itemState & ODS_SELECTED ) );
	}

	if ( ( lpDrawItemStruct->itemState & ODS_SELECTED ) == 0 &&
		nRating >= 0 && nRating <= 6 )
	{
		static COLORREF crRating[7] =
		{
			CoolInterface.m_crRatingNull,	// Unrated
			CoolInterface.m_crRating0,		// Fake
			CoolInterface.m_crRating1,		// Poor
			CoolInterface.m_crRating2,		// Average
			CoolInterface.m_crRating3,		// Good
			CoolInterface.m_crRating4,		// Very good
			CoolInterface.m_crRating5,		// Excellent
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

	CLibraryListPtr pFiles( GetList() );

	if ( ! pFiles || pFiles->GetCount() == 1 )
	{
		CQuickLock oLock( Library.m_pSection );
		if ( CLibraryFile* pFile = GetFile() )
		{
			pFile->m_nRating	= m_nRating;
			pFile->m_sComments	= m_sComments;

			pFile->ModifyMetadata();
			Library.Update();
		}
	}
	else
	{
		CQuickLock oLock( Library.m_pSection );

		for ( POSITION pos = pFiles->GetHeadPosition() ; pos ; )
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
