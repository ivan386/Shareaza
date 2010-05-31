//
// CtrlLibraryHeaderBar.cpp
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
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "CtrlLibraryHeaderBar.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CLibraryHeaderBar, CCoolBarCtrl)

BEGIN_MESSAGE_MAP(CLibraryHeaderBar, CCoolBarCtrl)
	//{{AFX_MSG_MAP(CLibraryHeaderBar)
	ON_COMMAND(ID_LIBRARY_VIEW, OnLibraryView)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_MENUSELECT()
	ON_WM_ENTERIDLE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryHeaderBar construction

CLibraryHeaderBar::CLibraryHeaderBar()
{
	m_pLastView	= NULL;
	m_nImage	= NULL;
	m_pCoolMenu	= NULL;
}

CLibraryHeaderBar::~CLibraryHeaderBar()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryHeaderBar operations

void CLibraryHeaderBar::Update(CLibraryView* pView)
{
	CString strTitle;
	int nImage = SHI_FOLDER_CLOSED;

	if ( pView != NULL ) pView->GetHeaderContent( nImage, strTitle );

	if ( nImage != m_nImage || strTitle != m_sTitle )
	{
		m_nImage	= nImage;
		m_sTitle	= strTitle;

		if (m_hWnd) Invalidate();
	}

	if ( pView != m_pLastView && ( m_pLastView = pView ) != NULL )
	{
		if ( CCoolBarItem* pItem = GetID( ID_LIBRARY_VIEW ) )
		{
			CString strName;
			Skin.LoadString( strName, pView->m_nCommandID );
			LPCTSTR pszName = _tcschr( strName, '\n' );
			pszName = ( pszName ) ? ( pszName + 1 ) : (LPCTSTR)strName;
			pItem->SetImage( pView->m_nCommandID );
			pItem->SetText( pszName );
		}

		OnUpdated();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryHeaderBar message handlers

void CLibraryHeaderBar::PrepareRect(CRect* pRect) const
{
	CCoolBarCtrl::PrepareRect( pRect );
	pRect->left -= 10;
	if ( m_czLast.cx < pRect->Width() ) pRect->left = pRect->right - m_czLast.cx;
	pRect->left += 10;
	pRect->bottom --;
}

void CLibraryHeaderBar::DoPaint(CDC* pDC, CRect& rcBar, BOOL bTransparent)
{
	pDC->FillSolidRect( rcBar.left, rcBar.bottom - 1, rcBar.Width(), 1,
		CoolInterface.m_crSys3DShadow );
	rcBar.bottom --;

	if ( m_czLast.cx < rcBar.Width() - 22 )
	{
		CRect rcHeader( &rcBar );
		rcHeader.right = rcBar.left = rcBar.right - m_czLast.cx;
		PaintHeader( pDC, rcHeader, bTransparent );
	}

	CCoolBarCtrl::DoPaint( pDC, rcBar, bTransparent );
}

void CLibraryHeaderBar::PaintHeader(CDC* pDC, CRect& rcBar, BOOL bTransparent)
{
	CFont* pOldFont = (CFont*)pDC->SelectObject( &CoolInterface.m_fntBold );

	CSize szText = pDC->GetTextExtent( m_sTitle );

	pDC->SetTextColor( CoolInterface.m_crCmdText );
	pDC->SetBkColor( CoolInterface.m_crMidtone );

	int nMiddle = ( rcBar.top + rcBar.bottom ) / 2;

	CString strText = m_sTitle;

	if ( pDC->GetTextExtent( strText ).cx > rcBar.Width() - 22 )
	{
		while ( pDC->GetTextExtent( strText ).cx > rcBar.Width() - 30 && strText.GetLength() )
		{
			strText = strText.Left( strText.GetLength() - 1 );
		}

		strText += _T("...");
	}

	if ( bTransparent )
	{
		if ( m_nImage )
		{
			ShellIcons.Draw( pDC, m_nImage, 16, rcBar.left + 4, nMiddle - 8 );
		}

		pDC->SetBkMode( TRANSPARENT );
		pDC->ExtTextOut( rcBar.left + 22, nMiddle - szText.cy / 2,
			ETO_CLIPPED, &rcBar, strText, NULL );
	}
	else
	{
		pDC->SetBkMode( OPAQUE );

		if ( m_nImage )
		{
			ShellIcons.Draw( pDC, m_nImage, 16, rcBar.left + 4, nMiddle - 8, CoolInterface.m_crMidtone );

			pDC->ExcludeClipRect( rcBar.left + 4, nMiddle - 8, rcBar.left + 20, nMiddle + 8 );
		}

		pDC->FillSolidRect( rcBar.left, rcBar.top, 20, rcBar.Height(),
			CoolInterface.m_crMidtone );

		rcBar.left += 20;
		pDC->ExtTextOut( rcBar.left + 2, nMiddle - szText.cy / 2,
			ETO_CLIPPED|ETO_OPAQUE, &rcBar, strText, NULL );
		rcBar.left -= 20;
	}

	pDC->SelectObject( pOldFont );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryHeaderBar view menu

void CLibraryHeaderBar::OnLibraryView()
{
	CMenu pMenu;

	pMenu.CreatePopupMenu();

	CLibraryFrame* pFrame	= (CLibraryFrame*)GetParent();
	CList< CLibraryView* >* pViews = pFrame->GetViews();

	for ( POSITION pos = pViews->GetHeadPosition() ; pos ; )
	{
		CLibraryView* pView = (CLibraryView*)pViews->GetNext( pos );
		if ( ! pView->m_bAvailable ) continue;

		CString strName;
		Skin.LoadString( strName, pView->m_nCommandID );
		LPCTSTR pszName = _tcschr( strName, '\n' );
		pszName = ( pszName ) ? ( pszName + 1 ) : (LPCTSTR)strName;

		pMenu.AppendMenu( MF_STRING | ( pView == m_pLastView ? MF_CHECKED : 0 ),
			pView->m_nCommandID, pszName );
	}

	m_pCoolMenu = new CCoolMenu();
	m_pCoolMenu->AddMenu( &pMenu, TRUE );
	m_pCoolMenu->SetWatermark( Skin.GetWatermark( _T("CCoolMenu") ) );

	if ( UINT nCmd = ThrowMenu( ID_LIBRARY_VIEW, &pMenu, this, TRUE, TRUE ) )
	{
		for ( POSITION pos = pViews->GetHeadPosition() ; pos ; )
		{
			CLibraryView* pView = (CLibraryView*)pViews->GetNext( pos );

			if ( pView->m_nCommandID == nCmd )
			{
				pFrame->SetView( pView );
			}
		}
	}

	delete m_pCoolMenu;
	m_pCoolMenu = NULL;
}

void CLibraryHeaderBar::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if ( m_pCoolMenu ) m_pCoolMenu->OnMeasureItem( lpMeasureItemStruct );
}

void CLibraryHeaderBar::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( m_pCoolMenu ) m_pCoolMenu->OnDrawItem( lpDrawItemStruct );
}

void CLibraryHeaderBar::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	AfxGetMainWnd()->SendMessage( WM_MENUSELECT, MAKELONG( nItemID, nFlags ), (LPARAM)hSysMenu );
}

void CLibraryHeaderBar::OnEnterIdle(UINT nWhy, CWnd* pWho)
{
	AfxGetMainWnd()->SendMessage( WM_ENTERIDLE, (WPARAM)nWhy, (LPARAM)pWho->GetSafeHwnd() );
}
