//
// DlgFilePropertiesSheet.cpp
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
#include "DlgFilePropertiesSheet.h"
#include "PageFileGeneral.h"
#include "PageFileMetadata.h"
#include "PageFileSources.h"
#include "PageFileComments.h"
#include "PageFileSharing.h"

#include "Skin.h"
#include "SkinWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CFilePropertiesSheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CFilePropertiesSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CFilePropertiesSheet)
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCACTIVATE()
	ON_WM_NCPAINT()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_NCMOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesSheet

CFilePropertiesSheet::CFilePropertiesSheet(UINT nIndex) : CPropertySheet( _T("") )
{
	if ( nIndex > 0 ) m_pList.AddTail( nIndex );
	
	m_pSkin = NULL;
	m_psh.dwFlags &= ~PSP_HASHELP;
}

CFilePropertiesSheet::~CFilePropertiesSheet()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesSheet operations

void CFilePropertiesSheet::Add(UINT nIndex)
{
	m_pList.CheckAndAdd( nIndex );
}

void CFilePropertiesSheet::Add(CLibraryList* pList)
{
	m_pList.Merge( pList );
}

int CFilePropertiesSheet::DoModal(int nPage) 
{
	//TODO: Get page title from defined caption in resources (Rolandas)
	CFileGeneralPage	pGeneral;
	CFileMetadataPage	pMetadata;
	CFileCommentsPage	pComments;
	CFileSharingPage	pSharing;
	CFileSourcesPage	pSources;
		
	CString strTabLabel;
		
	switch ( m_pList.GetCount() )
	{
	case 0:
		return IDCANCEL;
	case 1:
		strTabLabel = Skin.GetDialogCaption( _T("CFileGeneralPage") );
		pGeneral.m_psp.dwFlags |= PSP_USETITLE;
		pGeneral.m_psp.pszTitle = strTabLabel.IsEmpty() ? _T("General") : strTabLabel;
		AddPage( &pGeneral );
		strTabLabel = Skin.GetDialogCaption( _T("CFileMetadataPage") );
		pMetadata.m_psp.dwFlags |= PSP_USETITLE;
		pMetadata.m_psp.pszTitle = strTabLabel.IsEmpty() ? _T("Metadata") : strTabLabel;
		AddPage( &pMetadata );
		strTabLabel = Skin.GetDialogCaption( _T("CFileCommentsPage") );
		pComments.m_psp.dwFlags |= PSP_USETITLE;
		pComments.m_psp.pszTitle = strTabLabel.IsEmpty() ? _T("My Review") : strTabLabel;
		AddPage( &pComments );
		strTabLabel = Skin.GetDialogCaption( _T("CFileSharingPage") );
		pSharing.m_psp.dwFlags |= PSP_USETITLE;
		pSharing.m_psp.pszTitle = strTabLabel.IsEmpty() ? _T("Sharing") : strTabLabel;
		AddPage( &pSharing );
		strTabLabel = Skin.GetDialogCaption( _T("CFileSourcesPage") );
		pSources.m_psp.dwFlags |= PSP_USETITLE;
		pSources.m_psp.pszTitle = strTabLabel.IsEmpty() ? _T("Sources") : strTabLabel;
		AddPage( &pSources );
		break;
	default:
		strTabLabel = Skin.GetDialogCaption( _T("CFileMetadataPage") );
		pMetadata.m_psp.dwFlags |= PSP_USETITLE;
		pMetadata.m_psp.pszTitle = strTabLabel.IsEmpty() ? _T("Metadata") : strTabLabel;
		AddPage( &pMetadata );
		strTabLabel = Skin.GetDialogCaption( _T("CFileCommentsPage") );
		pComments.m_psp.dwFlags |= PSP_USETITLE;
		pComments.m_psp.pszTitle = strTabLabel.IsEmpty() ? _T("My Review") : strTabLabel;
		AddPage( &pComments );
		strTabLabel = Skin.GetDialogCaption( _T("CFileSharingPage") );
		pSharing.m_psp.dwFlags |= PSP_USETITLE;
		pSharing.m_psp.pszTitle = strTabLabel.IsEmpty() ? _T("Sharing") : strTabLabel;
		AddPage( &pSharing );
		if ( nPage == 1 ) nPage = 0;
		else if ( nPage == 2 ) nPage = 1;
		break;
	}
	
	m_psh.nStartPage = nPage;
	return CPropertySheet::DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesSheet message handlers

BOOL CFilePropertiesSheet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
	SetFont( &theApp.m_gdiFont );
	SetIcon( theApp.LoadIcon( IDI_PROPERTIES ), TRUE );

	CString strCaption;
	LoadString( strCaption, IDS_FILE_PROPERTIES );
	SetWindowText( strCaption );

	m_pSkin = Skin.GetWindowSkin( _T("CFilePropertiesSheet") );
	if ( m_pSkin == NULL ) m_pSkin = Skin.GetWindowSkin( this );
	if ( m_pSkin == NULL ) m_pSkin = Skin.GetWindowSkin( _T("CDialog") );
	
	if ( m_pSkin != NULL )
	{
		CRect rc;
		GetClientRect( &rc );
		m_pSkin->CalcWindowRect( &rc );
		m_brDialog.CreateSolidBrush( Skin.m_crDialog );
		SetWindowPos( NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED );
		OnSize( 1982, 0, 0 );
	}

	if ( GetDlgItem( IDOK ) )
	{
		CRect rc;
		GetDlgItem( IDOK )->GetWindowRect( &rc );
		ScreenToClient( &rc );
		GetDlgItem( IDOK )->SetWindowPos( NULL, 6, rc.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE );
		GetDlgItem( IDCANCEL )->SetWindowPos( NULL, 11 + rc.Width(), rc.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE );
	}

	if ( GetDlgItem( 0x3021 ) ) GetDlgItem( 0x3021 )->ShowWindow( SW_HIDE );
	if ( GetDlgItem( 0x0009 ) ) GetDlgItem( 0x0009 )->ShowWindow( SW_HIDE );
	
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesSheet skin support

void CFilePropertiesSheet::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	if ( m_pSkin )
		m_pSkin->OnNcCalcSize( this, bCalcValidRects, lpncsp );
	else
		CPropertySheet::OnNcCalcSize( bCalcValidRects, lpncsp );
}

UINT CFilePropertiesSheet::OnNcHitTest(CPoint point) 
{
	if ( m_pSkin )
		return m_pSkin->OnNcHitTest( this, point, ( GetStyle() & WS_THICKFRAME ) ? TRUE : FALSE );
	else
		return CPropertySheet::OnNcHitTest( point );
}

BOOL CFilePropertiesSheet::OnNcActivate(BOOL bActive) 
{
	if ( m_pSkin )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible ) ModifyStyle( WS_VISIBLE, 0 );
		BOOL bResult = CPropertySheet::OnNcActivate( bActive );
		if ( bVisible ) ModifyStyle( 0, WS_VISIBLE );
		m_pSkin->OnNcActivate( this, bActive || ( m_nFlags & WF_STAYACTIVE ) );
		return bResult;
	}
	else
	{
		return CPropertySheet::OnNcActivate( bActive );
	}
}

void CFilePropertiesSheet::OnNcPaint() 
{
	if ( m_pSkin )
		m_pSkin->OnNcPaint( this );
	else
		CPropertySheet::OnNcPaint();
}

void CFilePropertiesSheet::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDown( this, nHitTest, point ) ) return;
	CPropertySheet::OnNcLButtonDown(nHitTest, point);
}

void CFilePropertiesSheet::OnNcLButtonUp(UINT nHitTest, CPoint point) 
{
	if ( m_pSkin && m_pSkin->OnNcLButtonUp( this, nHitTest, point ) ) return;
	CPropertySheet::OnNcLButtonUp( nHitTest, point );
}

void CFilePropertiesSheet::OnNcLButtonDblClk(UINT nHitTest, CPoint point) 
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDblClk( this, nHitTest, point ) ) return;
	CPropertySheet::OnNcLButtonDblClk( nHitTest, point );
}

void CFilePropertiesSheet::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	if ( m_pSkin ) m_pSkin->OnNcMouseMove( this, nHitTest, point );
	CPropertySheet::OnNcMouseMove( nHitTest, point );
}

void CFilePropertiesSheet::OnSize(UINT nType, int cx, int cy) 
{
	if ( m_pSkin ) m_pSkin->OnSize( this );

	if ( nType != 1982 ) CPropertySheet::OnSize( nType, cx, cy );
}

LONG CFilePropertiesSheet::OnSetText(WPARAM wParam, LPARAM lParam)
{
	if ( m_pSkin )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible ) ModifyStyle( WS_VISIBLE, 0 );
		LONG lResult = Default();
		if ( bVisible ) ModifyStyle( 0, WS_VISIBLE );
		if ( m_pSkin ) m_pSkin->OnSetText( this );
		return lResult;
	}
	else
	{
		return Default();
	}
}

BOOL CFilePropertiesSheet::OnEraseBkgnd(CDC* pDC)
{
	if ( m_pSkin )
	{
		if ( m_pSkin->OnEraseBkgnd( this, pDC ) ) return TRUE;
	}

	return CPropertySheet::OnEraseBkgnd( pDC );
}

HBRUSH CFilePropertiesSheet::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// if ( m_brDialog.m_hObject ) return m_brDialog;
	return CPropertySheet::OnCtlColor( pDC, pWnd, nCtlColor );
}
