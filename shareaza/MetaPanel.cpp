//
// MetaPanel.cpp
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
#include "MetaPanel.h"
#include "CoolInterface.h"
#include "Skin.h"

#include "Library.h"
#include "WndMain.h"
#include "WndLibrary.h"
#include "CtrlLibraryFrame.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CMetaPanel construction

CMetaPanel::CMetaPanel()
{
	m_nHeight = 0;
}

CMetaPanel::~CMetaPanel()
{
}

//////////////////////////////////////////////////////////////////////
// CMetaPanel layout

int CMetaPanel::Layout(CDC* pDC, int nWidth)
{
	int nSmall	= ( nWidth >= 400 ) ? nWidth / 2 - 100 - 3 : 0;
	int nLarge	= nWidth - 100 - 3;
	
	m_nHeight = 0;
	
	CFont* pOld = (CFont*)pDC->SelectObject( &CoolInterface.m_fntNormal );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CMetaItem* pItem = GetNext( pos );
		CSize sz = pDC->GetTextExtent( pItem->m_sValue );
		
		if ( sz.cx <= nSmall )
		{
			pItem->m_bFullWidth	= FALSE;
			pItem->m_nHeight	= 18;
			
			if ( CMetaItem* pNext = GetNext( pos ) )
			{
				sz = pDC->GetTextExtent( pNext->m_sValue );
				
				if ( sz.cx <= nSmall )
				{
					pNext->m_bFullWidth	= FALSE;
					pNext->m_nHeight	= 18;
				}
				else if ( pos )
				{
					pItem->m_bFullWidth = TRUE;
					m_pItems.GetPrev( pos );
				}
				else
				{
					pItem->m_bFullWidth = TRUE;
					pos = m_pItems.GetTailPosition();
				}
			}
			else
			{
				pItem->m_bFullWidth = TRUE;
			}
			
			m_nHeight += 20;
		}
		else
		{
			if ( sz.cx > nLarge )
			{
				CRect rcText( 0, 0, nLarge, 0xFFFF );
				WrappedText( pDC, &rcText, pItem->m_sValue, FALSE );
				pItem->m_bFullWidth	= TRUE+TRUE;
				pItem->m_nHeight	= rcText.top + 4;
				m_nHeight += pItem->m_nHeight + 2;
			}
			else
			{
				pItem->m_bFullWidth = TRUE;
				pItem->m_nHeight	= 18;
				m_nHeight += 20;
			}
		}
	}
	
	pDC->SelectObject( pOld );
	
	return m_nHeight;
}

//////////////////////////////////////////////////////////////////////
// CMetaPanel paint

void CMetaPanel::Paint(CDC* pDC, const CRect* prcArea)
{
	POSITION pos = GetIterator();
	CRect rcWork( prcArea );
	
	for ( int nRow = 0 ; pos ; nRow++ )
	{
		pDC->SetBkColor( Skin.m_crSchemaRow[ nRow & 1 ] );
		int nHeight = 0;
		
		for ( int nColumn = 0 ; nColumn < 2 && pos ; nColumn++ )
		{
			CMetaItem* pItem = GetNext( pos );
						
			CRect rcValue( rcWork.left, rcWork.top, rcWork.left, rcWork.top + pItem->m_nHeight );
			
			if ( pItem->m_bFullWidth )
			{
				if ( nColumn > 0 )
				{
					if ( pos ) m_pItems.GetPrev( pos ); else pos = m_pItems.GetTailPosition();
					break;
				}
				
				rcValue.right	= rcWork.right;
			}
			else
			{
				rcValue.left	+= nColumn * rcWork.Width() / 2 + 1;
				rcValue.right	+= ( nColumn + 1 ) * rcWork.Width() / 2 - 1;
			}
			
			CRect rcKey( rcValue.left, rcValue.top, rcValue.left + 100, rcValue.bottom );
			rcValue.left = rcKey.right;
			
			pDC->SetTextColor( CoolInterface.m_crText );
			pDC->SelectObject( &CoolInterface.m_fntBold );
			
			pDC->ExtTextOut( rcKey.left + 3, rcKey.top + 2, ETO_CLIPPED|ETO_OPAQUE,
				&rcKey, pItem->m_sKey + ':', NULL );
			
			if ( pItem->m_bLink )
			{
				pDC->SetTextColor( RGB( 0, 0, 255 ) );
				pDC->SelectObject( &CoolInterface.m_fntUnder );
			}
			else
			{
				pDC->SelectObject( &CoolInterface.m_fntNormal );
			}
			
			if ( pItem->m_bFullWidth == 2 )
			{
				CRect rcText( &rcValue );
				rcText.DeflateRect( 3, 2 );
				WrappedText( pDC, &rcText, pItem->m_sValue, TRUE );
				pDC->ExtTextOut( rcValue.left, rcValue.top, ETO_OPAQUE, &rcValue, NULL, 0, NULL );
				pItem->m_rect.CopyRect( &rcValue );
			}
			else
			{
				pDC->ExtTextOut( rcValue.left + 3, rcValue.top + 2, ETO_CLIPPED|ETO_OPAQUE,
					&rcValue, pItem->m_sValue, NULL );
				
				pItem->m_rect.CopyRect( &rcValue );
				
				pItem->m_rect.right = pItem->m_rect.left + 6 +
					pDC->GetTextExtent( pItem->m_sValue ).cx;
			}
			
			pDC->ExcludeClipRect( &rcKey );
			pDC->ExcludeClipRect( &rcValue );
			
			nHeight = pItem->m_nHeight;
			if ( pItem->m_bFullWidth ) break;
		}
		
		rcWork.top += nHeight + 2;
	}
}

//////////////////////////////////////////////////////////////////////
// CMetaPanel wrapped text

void CMetaPanel::WrappedText(CDC* pDC, CRect* pBox, LPCTSTR pszText, BOOL bPaint)
{
	CPoint pt = pBox->TopLeft();
	
	LPCTSTR pszWord = pszText;
	LPCTSTR pszScan = pszText;
	
	for ( ; ; pszScan++ )
	{
		if ( *pszScan != NULL && (unsigned char)*pszScan > 32 ) continue;
		
		if ( pszWord < pszScan )
		{
			int nLen = pszScan - pszWord + ( *pszScan ? 1 : 0 );
			CSize sz = pDC->GetTextExtent( pszWord, nLen );
			
			if ( pt.x > pBox->left && pt.x + sz.cx > pBox->right )
			{
				pt.x = pBox->left;
				pt.y += sz.cy;
			}
			
			if ( bPaint )
			{
				CRect rc( pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy );
				
				pDC->ExtTextOut( pt.x, pt.y, ETO_CLIPPED|ETO_OPAQUE, &rc,
					pszWord, nLen, NULL );
				pDC->ExcludeClipRect( &rc );
			}
			
			pt.x += sz.cx;
			pBox->top = pt.y + sz.cy;
		}
		
		pszWord = pszScan + 1;
		if ( ! *pszScan ) break;
	}
}

//////////////////////////////////////////////////////////////////////
// CMetaPanel click handler

BOOL CMetaPanel::OnClick(const CPoint& point)
{
	if ( CMetaItem* pItem = HitTest( point, TRUE ) )
	{
		if ( CAlbumFolder* pFolder = pItem->GetLinkTarget() )
		{
			CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();
			if ( CLibraryWnd* pLibraryWnd = (CLibraryWnd*)pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CLibraryWnd) ) )
			{
				CLibraryFrame* pFrame = &pLibraryWnd->m_wndFrame;
				ASSERT_KINDOF(CLibraryFrame, pFrame );
				pFrame->Display( pFolder );
				return TRUE;
			}
		}
	}
	
	return FALSE;
}
