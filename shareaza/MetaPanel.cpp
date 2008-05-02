//
// MetaPanel.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
#include "MetaPanel.h"
#include "CoolInterface.h"
#include "SchemaMember.h"
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

#define METAPANEL_KEY_WIDTH 120
#define MUSICBRAINZ_HEIGHT 31
#define MUSICBRAINZ_WIDTH 200

//////////////////////////////////////////////////////////////////////
// CMetaPanel construction

CMetaPanel::CMetaPanel()
: m_nHeight(0)
, m_pChild( NULL )
{
	m_bmMusicBrainz.LoadBitmap( IDB_MUSICBRAINZ_LOGO );
}

CMetaPanel::~CMetaPanel()
{
}

//////////////////////////////////////////////////////////////////////
// CMetaPanel layout

int CMetaPanel::Layout(CDC* pDC, int nWidth)
{
	int nSmall	= ( nWidth >= 400 ) ? nWidth / 2 - METAPANEL_KEY_WIDTH - 3 : 0;
	int nLarge	= nWidth - METAPANEL_KEY_WIDTH - 3;
	
	m_nHeight = 0;
	
	CFont* pOld = (CFont*)pDC->SelectObject( &CoolInterface.m_fntNormal );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CMetaItem* pItem = GetNext( pos );
		if ( pItem->m_pMember && pItem->m_pMember->m_bHidden ) continue;

		CSize sz = pDC->GetTextExtent( pItem->GetDisplayValue() );
		
		if ( sz.cx <= nSmall )
		{
			pItem->m_bFullWidth	= FALSE;
			pItem->m_nHeight	= 18;
			
			if ( CMetaItem* pNext = GetNext( pos ) )
			{	
				while ( pNext && pNext->m_pMember && pNext->m_pMember->m_bHidden )
					pNext = GetNext( pos );

				if ( pNext == NULL )
				{
					pItem->m_bFullWidth = TRUE;
					m_nHeight += 20;
					break;
				}

				sz = pDC->GetTextExtent( pNext->GetDisplayValue() );
				
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
				Skin.DrawWrappedText( pDC, &rcText, pItem->GetDisplayValue(), NULL, FALSE );
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
	CRect rcWork( prcArea );

	if ( m_bMusicBrainz )
	{
		pDC->FillSolidRect( rcWork.left, rcWork.top, rcWork.Width(), MUSICBRAINZ_HEIGHT + 4,
			CoolInterface.m_crWindow );

		CDC dcMem;
		dcMem.CreateCompatibleDC( pDC );

		CBitmap* pOld = (CBitmap*)dcMem.SelectObject( &m_bmMusicBrainz );
		pDC->BitBlt( rcWork.left, rcWork.top, MUSICBRAINZ_WIDTH, MUSICBRAINZ_HEIGHT,
			&dcMem, 0, 0, SRCCOPY );
		pDC->ExcludeClipRect( rcWork.left, rcWork.top, rcWork.right, rcWork.top + MUSICBRAINZ_HEIGHT + 4 );

		dcMem.SelectObject( pOld );

		rcWork.top += MUSICBRAINZ_HEIGHT + 4;
	}

	POSITION pos = GetIterator();
	DWORD dwFlags = ( Settings.General.LanguageRTL ? ETO_RTLREADING : 0 );
	
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
			
			CRect rcKey( rcValue.left, rcValue.top, rcValue.left + METAPANEL_KEY_WIDTH, rcValue.bottom );
			rcValue.left = rcKey.right;
			
			pDC->SetTextColor( CoolInterface.m_crText );
			pDC->SelectObject( &CoolInterface.m_fntBold );
			
			CString strKey( pItem->m_sKey );
			strKey.TrimRight( L" \x00A0" );
			pDC->ExtTextOut( rcKey.left + 3, rcKey.top + 2, ETO_CLIPPED|ETO_OPAQUE, &rcKey, strKey + ':', NULL );
			
			if ( pItem->m_bLink )
			{
				pDC->SetTextColor( CoolInterface.m_crTextLink );
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
				Skin.DrawWrappedText( pDC, &rcText, pItem->GetDisplayValue(), NULL, TRUE );
				pDC->ExtTextOut( rcValue.left, rcValue.top, ETO_OPAQUE|dwFlags, &rcValue, NULL, 0, NULL );
				pItem->m_rect.CopyRect( &rcValue );
			}
			else
			{
				pDC->ExtTextOut( rcValue.left + 3, rcValue.top + 2, ETO_CLIPPED|ETO_OPAQUE|dwFlags,
					&rcValue, pItem->GetDisplayValue(), NULL );
				
				pItem->m_rect.CopyRect( &rcValue );
				
				pItem->m_rect.right = pItem->m_rect.left + 6 +
					pDC->GetTextExtent( pItem->GetDisplayValue() ).cx;
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
