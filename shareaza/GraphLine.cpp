//
// GraphLine.cpp
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
#include "GraphLine.h"
#include "GraphItem.h"
#include "CoolInterface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const DWORD MIN_GRID_SIZE_HORZ = 16u;
#define TOP_MARGIN			10


//////////////////////////////////////////////////////////////////////
// CLineGraph construction

CLineGraph::CLineGraph()
{
	m_bShowAxis		= TRUE;
	m_bShowGrid		= TRUE;
	m_bShowLegend	= TRUE;
	m_crBack		= CoolInterface.m_crTrafficWindowBack;
	m_crGrid		= CoolInterface.m_crTrafficWindowGrid;
	m_nMinGridVert	= 32;

	m_nSpeed		= 100;
	m_nScale		= 2;
	m_nMaximum		= 0;
	m_nUpdates		= 0;
	m_tLastScale	= 0;
}

CLineGraph::~CLineGraph()
{
	ClearItems();
}

//////////////////////////////////////////////////////////////////////
// CLineGraph list access

void CLineGraph::AddItem(CGraphItem* pItem)
{
	m_pItems.AddTail( pItem );
}

POSITION CLineGraph::GetItemIterator() const
{
	return m_pItems.GetHeadPosition();
}

CGraphItem* CLineGraph::GetNextItem(POSITION& pos) const
{
	return m_pItems.GetNext( pos );
}

void CLineGraph::RemoveItem(CGraphItem* pItem)
{
	POSITION pos = m_pItems.Find( pItem );
	if ( pos != NULL ) m_pItems.RemoveAt( pos );
	delete pItem;
}

void CLineGraph::ClearItems()
{
	for ( POSITION pos = GetItemIterator() ; pos ; )
	{
		delete GetNextItem( pos );
	}
	m_pItems.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CLineGraph create default

void CLineGraph::CreateDefaults()
{
	AddItem( new CGraphItem( GRC_TOTAL_BANDWIDTH_IN, 1.0f, RGB( 0, 255, 0 ) ) );
	AddItem( new CGraphItem( GRC_TOTAL_BANDWIDTH_OUT, 1.0f, RGB( 255, 0, 0 ) ) );
}

//////////////////////////////////////////////////////////////////////
// CLineGraph update

BOOL CLineGraph::Update()
{
	DWORD tNow = GetTickCount();

	if ( tNow - m_tLastScale > 10000 )
	{
		m_tLastScale = tNow;
		ResetMaximum();
	}

	for ( POSITION pos = GetItemIterator() ; pos ; )
	{
		CGraphItem* pItem = GetNextItem( pos );
		DWORD nValue = pItem->Update();
		m_nMaximum = max( m_nMaximum, nValue );
	}

	m_nUpdates++;
	return !m_pItems.IsEmpty();
}

//////////////////////////////////////////////////////////////////////
// CLineGraph clear

void CLineGraph::Clear()
{
	for ( POSITION pos = GetItemIterator() ; pos ; )
	{
		GetNextItem( pos )->Clear();
	}

	m_nMaximum = 0;
}

//////////////////////////////////////////////////////////////////////
// CLineGraph serialize

void CLineGraph::Serialize(CArchive& ar)
{
	if ( ar.IsStoring() )
	{
		ar << m_bShowAxis;
		ar << m_bShowGrid;
		ar << m_bShowLegend;
		ar << m_nSpeed;
		ar << m_nScale;

		ar.WriteCount( GetItemCount() );

		for ( POSITION pos = GetItemIterator() ; pos ; )
		{
			GetNextItem( pos )->Serialize( ar );
		}
	}
	else
	{
		ar >> m_bShowAxis;
		ar >> m_bShowGrid;
		ar >> m_bShowLegend;
		ar >> m_nSpeed;
		ar >> m_nScale;
		if ( m_nScale == 0 )
			m_nScale = 2;

		for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CGraphItem* pItem = new CGraphItem();
			pItem->Serialize( ar );
			m_pItems.AddTail( pItem );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CLineGraph maximum rescale

void CLineGraph::ResetMaximum(BOOL bForce)
{
	DWORD nMaximum = 0;

	for ( POSITION pos = GetItemIterator() ; pos ; )
	{
		CGraphItem* pItem = GetNextItem( pos );
		DWORD nValue = pItem->GetMaximum();
		nMaximum = max( nMaximum, nValue );
	}

	if ( nMaximum || bForce ) m_nMaximum = nMaximum;
}

//////////////////////////////////////////////////////////////////////
// CLineGraph paint

void CLineGraph::Paint(CDC* pDC, CRect* pRect)
{
	if ( m_pGridPen.m_hObject == NULL ) m_pGridPen.CreatePen( PS_SOLID, 1, m_crGrid );
	ASSERT( m_nScale != 0 );

	DWORD nWidth = (DWORD)pRect->Width() / m_nScale + 2;

	if ( pRect->Width() > 64 )
	{
		for ( POSITION pos = GetItemIterator() ; pos ; )
		{
			CGraphItem* pItem = GetNextItem( pos );
			pItem->SetHistory( nWidth );
		}
	}

	pDC->FillSolidRect( pRect, m_crBack );

	if ( m_pItems.IsEmpty() || m_nMaximum == 0 ) return;

	CFont* pOldFont = (CFont*)pDC->SelectObject( &theApp.m_gdiFont );
	pDC->SetBkMode( TRANSPARENT );

	if ( m_bShowGrid ) PaintGrid( pDC, pRect );

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		CGraphItem* pItem = m_pItems.GetNext( pos );

		DWORD nPoints	= min( pItem->m_nLength, nWidth );
		POINT* pPoints	= new POINT[ nPoints ];

		for ( DWORD nPos = 0 ; nPos < nPoints ; nPos++ )
		{
			DWORD nValue = pItem->GetValueAt( nPos );

			nValue = pRect->bottom - nValue * ( pRect->Height() - TOP_MARGIN ) / m_nMaximum;

			pPoints[ nPos ].x = pRect->right - nPos * m_nScale - 1;
			pPoints[ nPos ].y = nValue + 4;
		}

		pItem->MakeGradient( m_crBack );

		CPen* pOldPen = (CPen*)pDC->SelectObject( &pItem->m_pPen[3] );

		for ( int nLayer = 4 ; nLayer ; nLayer-- )
		{
			if ( nPoints == 1 )
				pDC->SetPixel( *pPoints, pItem->m_cPen[3] );
			else
				pDC->Polyline( pPoints, nPoints );

			if ( nLayer > 1 )
			{
				for ( DWORD nPos = 0 ; nPos < nPoints ; nPos++ ) pPoints[ nPos ].y --;
				pDC->SelectObject( &pItem->m_pPen[ nLayer - 2 ] );
			}
		}

		pDC->SelectObject( pOldPen );

		delete [] pPoints;
	}

	if ( m_bShowLegend ) PaintLegend( pDC, pRect );

	pDC->SelectObject( pOldFont );
}

void CLineGraph::PaintGrid(CDC* pDC, CRect* pRect)
{
	CPen* pOldPen = (CPen*)pDC->SelectObject( &m_pGridPen );

	if ( pRect->Height() <= TOP_MARGIN ) return;

	DWORD nScale = max( m_nScale, MIN_GRID_SIZE_HORZ );
	DWORD nCount = pRect->Width() / nScale + 1;
	DWORD nTimeB = nScale / m_nScale;

	int nX = pRect->right + m_nScale - ( m_nUpdates % nTimeB ) * m_nScale;

	for ( DWORD nPos = 0 ; nPos < nCount ; nPos++, nX -= nScale )
	{
		pDC->MoveTo( nX, pRect->top );
		pDC->LineTo( nX, pRect->bottom );
	}

	BOOL bVolume = FALSE;

	if ( m_bShowAxis )
	{
		for ( POSITION pos = GetItemIterator() ; pos && ! bVolume ; )
		{
			const CGraphItem* pItem	= GetNextItem( pos );
			const GRAPHITEM* pDesc	= pItem->GetItemDesc( pItem->m_nCode );
			if ( pDesc && pDesc->m_nUnits == 1 ) bVolume = TRUE;
		}
		pDC->SetTextColor( CoolInterface.m_crTrafficWindowText );
	}

	nScale = m_nMinGridVert * m_nMaximum / ( pRect->Height() - TOP_MARGIN );
	if ( ! nScale  ) nScale = 1;

	int nOldY = pRect->bottom;

	for ( DWORD nPos = 1 ; ; nPos++ )
	{
		int nY = pRect->bottom - nScale * nPos * ( pRect->Height() - TOP_MARGIN ) / m_nMaximum;
		if ( nY < 0 || nY >= nOldY - 4 ) break;
		nOldY = nY;

		pDC->MoveTo( pRect->left, nY );
		pDC->LineTo( pRect->right, nY );

		if ( m_bShowAxis )
		{
			CString strValue;
			if ( bVolume )
				strValue = Settings.SmartSpeed( nScale * nPos, bits );
			else
				strValue.Format( _T("%lu"), nScale * nPos );
			pDC->ExtTextOut( pRect->left + 4, nY + 1, 0, NULL, strValue, NULL );
			// Add the scale to the right side also
			pDC->ExtTextOut( pRect->right - 4 - pDC->GetTextExtent( strValue ).cx, nY + 1, 0, NULL, strValue, NULL );
		}
	}

	pDC->SelectObject( pOldPen );
}

void CLineGraph::PaintLegend(CDC* pDC, CRect* pRect)
{
	int nHeight	= pDC->GetTextExtent( _T("Cy") ).cy;
	int nLeft	= pRect->left + ( ( pRect->Width() > 128 ) ? 64 : 0 );
	int nTop	= pRect->top + 1;

	for ( POSITION pos = GetItemIterator() ; pos ; nTop += nHeight )
	{
		CGraphItem* pItem = GetNextItem( pos );
		pDC->SetTextColor( pItem->m_nColour );

		CString strText;
		if ( pItem->m_nMultiplier != 1.0f )
			strText.Format( L"%s (\x00D7%f)", (LPCTSTR)pItem->m_sName, pItem->m_nMultiplier );
		else
			strText = pItem->m_sName;

		pDC->ExtTextOut( nLeft, nTop, 0, NULL, _T("\x2022 ") + strText, NULL );
	}
}
