//
// GraphBase.cpp
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
#include "GraphBase.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CGraphBase construction

CGraphBase::CGraphBase()
{
	m_hOldImage = NULL;
	m_szImage	= CSize( 0, 0 );
	m_hOldImage	= NULL;
}

CGraphBase::~CGraphBase()
{
	if ( m_hOldImage )
	{
		m_pDC.SelectObject( CBitmap::FromHandle( m_hOldImage ) );
		m_pDC.DeleteDC();
		m_pImage.DeleteObject();
		m_hOldImage = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CGraphBase defaults

void CGraphBase::CreateDefaults()
{
}

//////////////////////////////////////////////////////////////////////
// CGraphBase serialize

void CGraphBase::Serialize(CArchive& /*ar*/)
{
}

//////////////////////////////////////////////////////////////////////
// CGraphBase update

BOOL CGraphBase::Update()
{
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CGraphBase clear

void CGraphBase::Clear()
{
}

//////////////////////////////////////////////////////////////////////
// CGraphBase paint

void CGraphBase::Paint(CDC* /*pDC*/, CRect* /*pRect*/)
{
}

//////////////////////////////////////////////////////////////////////
// CGraphBase buffered paint

void CGraphBase::BufferedPaint(CDC* pDC, CRect* pRect)
{
	CSize sz = pRect->Size();

	if ( sz.cx < 0 || sz.cy < 0 ) return;

	if ( sz != m_szImage || m_hOldImage == NULL )
	{
		if ( m_hOldImage != NULL )
		{
			m_pDC.SelectObject( CBitmap::FromHandle( m_hOldImage ) );
			m_pDC.DeleteDC();
			m_pImage.DeleteObject();
		}

		m_szImage = sz;
		m_pImage.CreateCompatibleBitmap( pDC, sz.cx, sz.cy );
		m_pDC.CreateCompatibleDC( pDC );
		m_hOldImage = (HBITMAP)m_pDC.SelectObject( &m_pImage )->GetSafeHandle();
	}

	CRect rc( 0, 0, sz.cx, sz.cy );
	Paint( &m_pDC, &rc );

	pDC->BitBlt( pRect->left, pRect->top, sz.cx, sz.cy, &m_pDC, 0, 0, SRCCOPY );
}
