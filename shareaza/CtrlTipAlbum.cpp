//
// CtrlTipAlbum.cpp
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
#include "Library.h"
#include "LibraryFolders.h"
#include "AlbumFolder.h"
#include "Schema.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "CtrlTipAlbum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CAlbumTipCtrl, CCoolTipCtrl)

BEGIN_MESSAGE_MAP(CAlbumTipCtrl, CCoolTipCtrl)
	//{{AFX_MSG_MAP(CAlbumTipCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAlbumTipCtrl construction

CAlbumTipCtrl::CAlbumTipCtrl()
{
	m_crLight = CCoolInterface::CalculateColour( CoolInterface.m_crTipBack, RGB( 255, 255, 255 ), 128 );
}

CAlbumTipCtrl::~CAlbumTipCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CAlbumTipCtrl prepare

BOOL CAlbumTipCtrl::OnPrepare()
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	CAlbumFolder* pFolder = (CAlbumFolder*)m_pContext;
	if ( ! LibraryFolders.CheckAlbum( pFolder ) ) return FALSE;
	
	// Basic data
	
	m_sName	= pFolder->m_sName;
	m_sType	= _T("Virtual Folder");
	
	m_nIcon32 = SHI_FOLDER_OPEN;
	m_nIcon48 = SHI_FOLDER_OPEN;
	m_bCollection = pFolder->m_bCollSHA1;
	
	// Metadata
	
	m_pMetadata.Clear();
	
	if ( pFolder->m_pSchema != NULL )
	{
		CString strText;
		LPCTSTR pszColon = _tcschr( pFolder->m_pSchema->m_sTitle, ':' );
		m_sType = pszColon ? pszColon + 1 : pFolder->m_pSchema->m_sTitle;
		LoadString( strText, IDS_TIP_FOLDER );
		m_sType += " " + strText;
		
		m_nIcon48	= pFolder->m_pSchema->m_nIcon48;
		m_nIcon32	= pFolder->m_pSchema->m_nIcon32;
		
		if ( pFolder->m_pXML != NULL )
		{
			m_pMetadata.Setup( pFolder->m_pSchema, FALSE );
			m_pMetadata.Combine( pFolder->m_pXML );
			m_pMetadata.Clean();
		}
	}
	
	CalcSizeHelper();
	
	return m_sz.cx > 0;
}

/////////////////////////////////////////////////////////////////////////////
// CAlbumTipCtrl compute size

void CAlbumTipCtrl::OnCalcSize(CDC* pDC)
{
	AddSize( pDC, m_sName );
	m_sz.cy += TIP_TEXTHEIGHT;
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	AddSize( pDC, m_sType );
	m_sz.cy += TIP_TEXTHEIGHT;

	m_sz.cy += TIP_RULE;

	int nMetaHeight = m_pMetadata.GetCount() * TIP_TEXTHEIGHT;
	int nValueWidth = 0;
	m_nKeyWidth = 0;

	m_pMetadata.ComputeWidth( pDC, m_nKeyWidth, nValueWidth );

	if ( m_nKeyWidth ) m_nKeyWidth += TIP_GAP;
	m_sz.cx = max( m_sz.cx, m_nKeyWidth + nValueWidth + 102 );
	m_sz.cy += max( nMetaHeight, 96 );
}

/////////////////////////////////////////////////////////////////////////////
// CAlbumTipCtrl painting

void CAlbumTipCtrl::OnPaint(CDC* pDC)
{
	CPoint pt( 0, 0 );

	DrawText( pDC, &pt, m_sName );
	pt.y += TIP_TEXTHEIGHT;
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	DrawText( pDC, &pt, m_sType );
	pt.y += TIP_TEXTHEIGHT;

	DrawRule( pDC, &pt );

	CRect rcThumb( pt.x, pt.y, pt.x + 96, pt.y + 96 );
	CRect rcWork( &rcThumb );
	DrawThumb( pDC, rcWork );
	pDC->ExcludeClipRect( &rcThumb );

	int nCount = 0;

	for ( POSITION pos = m_pMetadata.GetIterator() ; pos ; )
	{
		CMetaItem* pItem = m_pMetadata.GetNext( pos );

		DrawText( pDC, &pt, pItem->m_sKey + ':', 100 );
		DrawText( pDC, &pt, pItem->m_sValue, 100 + m_nKeyWidth );
		pt.y += TIP_TEXTHEIGHT;

		if ( ++nCount == 5 )
		{
			pt.x += 98; pt.y -= 2;
			DrawRule( pDC, &pt, TRUE );
			pt.x -= 98; pt.y -= 2;
		}
	}
}

void CAlbumTipCtrl::DrawThumb(CDC* pDC, CRect& rcThumb)
{
	pDC->Draw3dRect( &rcThumb, CoolInterface.m_crTipBorder, CoolInterface.m_crTipBorder );
	rcThumb.DeflateRect( 1, 1 );
	
	CPoint pt(	( rcThumb.left + rcThumb.right ) / 2 - 24,
				( rcThumb.top + rcThumb.bottom ) / 2 - 24 );
	
	UINT nStyle = ( m_bCollection ? INDEXTOOVERLAYMASK(SHI_O_COLLECTION) : 0 );
	
	if ( m_nIcon48 >= 0 )
	{
		ImageList_DrawEx( ShellIcons.GetHandle( 48 ), m_nIcon48, *pDC,
			pt.x, pt.y, 48, 48, m_crLight, CLR_DEFAULT, nStyle );
		pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 48, pt.y + 48 );
	}
	else if ( m_nIcon32 >= 0 )
	{
		pt.x += 8; pt.y += 8;
		ImageList_DrawEx( ShellIcons.GetHandle( 32 ), m_nIcon32, *pDC,
			pt.x, pt.y, 32, 32, m_crLight, CLR_DEFAULT, nStyle );
		pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 32, pt.y + 32 );
	}
	
	pDC->FillSolidRect( &rcThumb, m_crLight );
}

