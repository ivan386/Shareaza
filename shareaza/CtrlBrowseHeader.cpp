//
// CtrlBrowseHeader.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "GProfile.h"
#include "Hostbrowser.h"
#include "SchemaCache.h"
#include "Schema.h"
#include "Skin.h"
#include "XML.h"
#include "ShellIcons.h"
#include "CoolInterface.h"
#include "CtrlBrowseHeader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CBrowseHeaderCtrl, CCoolBarCtrl)
	//{{AFX_MSG_MAP(CBrowseHeaderCtrl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBrowseHeaderCtrl construction

CBrowseHeaderCtrl::CBrowseHeaderCtrl()
{
	m_bBuffered = TRUE;
}

CBrowseHeaderCtrl::~CBrowseHeaderCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseHeaderCtrl operations

BOOL CBrowseHeaderCtrl::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	return CWnd::Create( NULL, NULL, WS_CHILD|WS_VISIBLE,
		rect, pParentWnd, IDC_BROWSE_HEADER, NULL );
}

void CBrowseHeaderCtrl::Update(CHostBrowser* pBrowser)
{
	CGProfile* pProfile = pBrowser->m_pProfile;
	CString strValue, strFormat, strNick;

	bool bNickIsPresent = false;

	if ( pProfile != NULL )
	{
		strNick = pProfile->GetNick();
		bNickIsPresent = !strNick.IsEmpty();
	}

	if ( bNickIsPresent )
	{
		LoadString( strFormat, IDS_BROWSE_TITLE_FORMAT );
		strValue.Format( strFormat, (LPCTSTR)strNick );
	}
	else
	{
		LoadString( strFormat, IDS_BROWSE_TITLE_FORMAT );
		strValue.Format( strFormat, (LPCTSTR)CString( inet_ntoa( pBrowser->m_pAddress ) ) );
	}

	float nProgress = pBrowser->GetProgress();

	if ( nProgress > 0.0f && nProgress < 1.0f )
	{
		CString strItem;
		nProgress *= 100.0f;
		strItem.Format( _T(" (%.1f%%)"), double( nProgress ) );
		strValue += strItem;
	}

	if ( strValue != m_sTitle )
	{
		m_sTitle = strValue;
		Invalidate();
	}

	strValue.Empty();

	if ( pBrowser->m_nHits > 0 )
	{
		LoadString( strFormat, IDS_BROWSE_INTRO_FORMAT );
		strValue.Format( strFormat, pBrowser->m_nHits, bNickIsPresent
			? (LPCTSTR)strNick : (LPCTSTR)CString( inet_ntoa( pBrowser->m_pAddress ) ) );
	}

	if ( m_sIntro != strValue )
	{
		m_sIntro = strValue;
		Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseHeaderCtrl message handlers

int CBrowseHeaderCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	if ( CSchemaPtr pSchema = SchemaCache.Get( CSchema::uriLibrary ) )
	{
		m_nIcon32 = pSchema->m_nIcon32;
		m_nIcon48 = pSchema->m_nIcon48;
	}

	OnSkinChange();

	return 0;
}

void CBrowseHeaderCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );
	Invalidate();
}

void CBrowseHeaderCtrl::OnSkinChange()
{
	Skin.CreateToolBar( _T("CBrowseHeaderCtrl"), this );

	if ( m_bmImage.m_hObject == NULL &&
		 Skin.m_crBannerBack == RGB( 122, 161, 230 ) )
	{
		m_bmImage.LoadBitmap( IDB_BANNER_MARK );
	}
}

void CBrowseHeaderCtrl::PrepareRect(CRect* pRect) const
{
	CCoolBarCtrl::PrepareRect( pRect );
	pRect->left -= 5;
	if ( m_czLast.cx < pRect->Width() ) pRect->left = pRect->right - m_czLast.cx;
	pRect->left += 5;

	pRect->top		= ( pRect->top + pRect->bottom ) / 2;
	pRect->bottom	= pRect->top + 14;
	pRect->top		= pRect->top - 14;
}

void CBrowseHeaderCtrl::DoPaint(CDC* pDC, CRect& rcClient, BOOL bTransparent)
{
	if ( ! CoolInterface.DrawWatermark( pDC, &rcClient, &m_bmImage ) )
	{
		pDC->FillSolidRect( rcClient.left, rcClient.top,
			rcClient.Width(), rcClient.Height(), Skin.m_crBannerBack );
	}

	CRect rcBar;

	rcBar.top		= ( rcClient.top + rcClient.bottom ) / 2;
	rcBar.bottom	= rcBar.top + 14;
	rcBar.top		= rcBar.top - 14;
	rcBar.right		= rcClient.right;
	rcBar.left		= rcBar.right - m_czLast.cx;

	CCoolBarCtrl::DoPaint( pDC, rcBar, bTransparent );

	CFont* pOldFont = pDC->GetCurrentFont();

	CPoint ptIcon( 8, ( rcClient.top + rcClient.bottom ) / 2 - 24 );

	if ( m_nIcon48 >= 0 )
	{
		ShellIcons.Draw( pDC, m_nIcon48, 48, ptIcon.x, ptIcon.y );
	}
	else if ( m_nIcon32 >= 0 )
	{
		ptIcon.x += 8; ptIcon.y += 8;
		ShellIcons.Draw( pDC, m_nIcon32, 32, ptIcon.x, ptIcon.y );
	}

	pDC->SetTextColor( Skin.m_crBannerText );
	pDC->SetBkMode( TRANSPARENT );

	CRect rcWork( &rcClient );
	rcWork.right -= m_czLast.cx;
	rcWork.DeflateRect( 8, 4 );
	rcWork.left += 48 + 16;
	rcWork.DeflateRect( 0, 4 );

	pDC->SelectObject( &CoolInterface.m_fntCaption );
	Skin.DrawWrappedText( pDC, &rcWork, m_sTitle, NULL, FALSE );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	Skin.DrawWrappedText( pDC, &rcWork, m_sIntro, NULL, FALSE );

	pDC->SelectObject( pOldFont );
}

