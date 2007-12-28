//
// LiveListSizer.cpp
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
#include "LiveListSizer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CLiveListSizer construction

CLiveListSizer::CLiveListSizer(CListCtrl* pCtrl)
{
	m_pCtrl		= NULL;
	m_nWidth	= 0;
	m_nColumns	= 0;
	m_pWidth	= NULL;
	m_pTake		= NULL;
	if ( pCtrl ) Attach( pCtrl );
}

CLiveListSizer::~CLiveListSizer()
{
	Detach();
}

//////////////////////////////////////////////////////////////////////
// CLiveListSizer attach and detach

void CLiveListSizer::Attach(CListCtrl* pCtrl, BOOL bScale)
{
	Detach();
	m_pCtrl = pCtrl;
	if ( pCtrl && bScale ) Resize( 0, TRUE );
}

void CLiveListSizer::Detach()
{
	m_pCtrl		= NULL;
	m_nWidth	= 0;
	m_nColumns	= 0;
	if ( m_pWidth ) delete [] m_pWidth;
	m_pWidth = NULL;
	if ( m_pTake ) delete [] m_pTake;
	m_pTake = NULL;
}

//////////////////////////////////////////////////////////////////////
// CLiveListSizer scaling resize

BOOL CLiveListSizer::Resize(int nWidth, BOOL bScale)
{
	if ( m_pCtrl == NULL ) return FALSE;
	if ( ! Settings.General.SizeLists ) return FALSE;

	if ( ! nWidth )
	{
		CRect rc;
		m_pCtrl->GetClientRect( &rc );
		nWidth = rc.right;
	}

	nWidth -= GetSystemMetrics( SM_CXVSCROLL ) - 1;

	if ( nWidth < 64 ) return FALSE;

	LV_COLUMN pColumn;
	pColumn.mask = LVCF_WIDTH;
    int nColumn = 0;
	for ( ; m_pCtrl->GetColumn( nColumn, &pColumn ) ; nColumn++ );

	if ( nColumn != m_nColumns )
	{
		if ( m_pWidth ) delete [] m_pWidth;
		if ( m_pTake ) delete [] m_pTake;

		m_pWidth	= new int[ m_nColumns = nColumn ];
		m_pTake		= new float[ m_nColumns ];
	}

	if ( ! m_nWidth ) m_nWidth = nWidth;

	float nTotal = 0;

	for ( nColumn = 0 ; nColumn < m_nColumns ; nColumn++ )
	{
		m_pCtrl->GetColumn( nColumn, &pColumn );

		if ( pColumn.cx != m_pWidth[ nColumn ] )
		{
			m_pWidth[ nColumn ]	= pColumn.cx;
			m_pTake[ nColumn ]	= (float)pColumn.cx / (float)m_nWidth;
		}

		nTotal += m_pTake[ nColumn ];
	}

	if ( nTotal > 1 || ( bScale && nTotal > 0 ) )
	{
		for ( nColumn = 0 ; nColumn < m_nColumns ; nColumn++ )
		{
			m_pTake[ nColumn ] /= nTotal;
		}
		m_nWidth = 0;
	}

	if ( m_nWidth == nWidth ) return FALSE;
	m_nWidth = nWidth;

	for ( nColumn = 0 ; nColumn < m_nColumns ; nColumn++ )
	{
		pColumn.cx = (int)( m_pTake[ nColumn ] * (float)m_nWidth );
		m_pWidth[ nColumn ] = pColumn.cx;
		m_pCtrl->SetColumn( nColumn, &pColumn );
	}

	return TRUE;
}
