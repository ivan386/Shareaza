//
// RichElement.cpp
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
#include "RichDocument.h"
#include "RichElement.h"
#include "RichFragment.h"

#include "CoolInterface.h"
#include "ImageFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CRichElement construction

CRichElement::CRichElement(int nType, LPCTSTR pszText, LPCTSTR pszLink, DWORD nFlags, int nGroup)
	: m_pDocument	( NULL )
	, m_nType		( nType )
	, m_nFlags		( nFlags )
	, m_nGroup		( nGroup )
	, m_hImage		( NULL )
	, m_nImageIndex ( NULL )
{
	if ( m_nType == retHeading )
	{
		m_nType = retText;
		m_nFlags |= retfHeading;
	}

	if ( pszText )
	{
		if ( ( m_nType == retBitmap || m_nType == retIcon ) && HIWORD( pszText ) == 0 )
		{
			m_sText.Format( _T("%Iu"), (size_t)pszText );
		}
		else
		{
			m_sText = pszText;
		}
	}

	if ( pszLink ) m_sLink = pszLink;
}

CRichElement::CRichElement(HBITMAP hBitmap, LPCTSTR pszLink, DWORD nFlags, int nGroup)
	: m_pDocument	( NULL )
	, m_nType		( retBitmap )
	, m_nFlags		( nFlags )
	, m_nGroup		( nGroup )
	, m_hImage		( (HANDLE)hBitmap )
	, m_nImageIndex ( NULL )
{
	if ( pszLink ) m_sLink = pszLink;
}

CRichElement::CRichElement(HICON hIcon, LPCTSTR pszLink, DWORD nFlags, int nGroup)
	: m_pDocument	( NULL )
	, m_nType		( retIcon )
	, m_nFlags		( nFlags )
	, m_nGroup		( nGroup )
	, m_hImage		( (HANDLE)hIcon )
	, m_nImageIndex ( NULL )
{
	if ( pszLink ) m_sLink = pszLink;
}

CRichElement::~CRichElement()
{
	if ( m_hImage != NULL )
	{
		if ( m_nType == retBitmap )
		{
			DeleteObject( (HBITMAP)m_hImage );
			m_hImage = NULL;
		}
		else if ( m_nType == retIcon )
		{
			DestroyIcon( (HICON)m_hImage );
			m_hImage = NULL;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CRichElement editing

void CRichElement::Show(BOOL bShow)
{
	if ( bShow == ( ( m_nFlags & retfHidden ) > 0 ) )
	{
		m_nFlags |= retfHidden;
		if ( bShow ) m_nFlags &= ~retfHidden;
		m_pDocument->m_nCookie++;
	}
}

void CRichElement::SetText(LPCTSTR pszText)
{
	if ( m_sText != pszText )
	{
		m_sText = pszText;
		m_pDocument->m_nCookie++;
	}
}

void CRichElement::SetFlags(DWORD nFlags, DWORD nMask)
{
	DWORD nNew = ( m_nFlags & ~nMask ) | ( nFlags & nMask );

	if ( nNew != m_nFlags )
	{
		m_nFlags = nNew;
		m_pDocument->m_nCookie++;
	}
}

//////////////////////////////////////////////////////////////////////
// CRichElement delete

void CRichElement::Delete()
{
	if ( m_pDocument ) m_pDocument->Remove( this );
	delete this;
}

//////////////////////////////////////////////////////////////////////
// CRichElement setup for paint

void CRichElement::PrePaint(CDC* pDC, BOOL bHover)
{
	ASSERT( m_pDocument->m_fntNormal.m_hObject != NULL );

	CFont* pFont = &m_pDocument->m_fntNormal;

	switch ( m_nType )
	{
	case retText:
		if ( m_nFlags & retfColour )
			pDC->SetTextColor( m_cColour );
		else
			pDC->SetTextColor( m_pDocument->m_crText );
		pFont = &m_pDocument->m_fntNormal;
		break;
	case retLink:
		if ( m_nFlags & retfColour )
			pDC->SetTextColor( m_cColour );
		else
			pDC->SetTextColor( bHover ? m_pDocument->m_crHover : m_pDocument->m_crLink );
		pFont = &m_pDocument->m_fntUnder;
		break;
	case retBitmap:
		PrePaintBitmap( pDC );
		pFont = NULL;
		break;
	case retIcon:
		PrePaintIcon( pDC );
		pFont = NULL;
		break;
	case retEmoticon:
		_stscanf( m_sText, _T("%i"), &m_nImageIndex );		// (TODO)
		m_hImage = NULL;
		pFont = NULL;
		break;
	case retCmdIcon:
		if ( UINT nID = CoolInterface.NameToID( m_sText ) )
		{
			m_nImageIndex = CoolInterface.ImageForID( nID );	// (TODO)
			m_hImage = NULL;
		}
		break;
	default:
		pFont = NULL;
		break;
	}

	if ( m_nFlags & retfBold )
	{
		if ( m_nFlags & retfUnderline ) pFont = &m_pDocument->m_fntBoldUnder;
		else pFont = &m_pDocument->m_fntBold;
	}
	else if ( m_nFlags & retfItalic )
	{
		pFont = &m_pDocument->m_fntItalic;
	}
	else if ( m_nFlags & retfUnderline )
	{
		pFont = &m_pDocument->m_fntUnder;
	}
	else if ( m_nFlags & retfHeading )
	{
		pFont = &m_pDocument->m_fntHeading;
		pDC->SetTextColor( m_pDocument->m_crHeading );
	}

	if ( pFont ) pDC->SelectObject( pFont );
}

void CRichElement::PrePaintBitmap(CDC* /*pDC*/)
{
	if ( m_hImage != NULL ) return;

	if ( _tcsnicmp( m_sText, _T("res:"), 4 ) == 0 )
	{
		UINT nID = 0;
		if ( _stscanf( (LPCTSTR)m_sText + 4, _T("%u"), &nID ) != 1 )
			return;

		m_hImage = CImageFile::LoadBitmapFromResource( nID );
	}
	else
	{
		m_hImage = CImageFile::LoadBitmapFromFile( Settings.General.Path + '\\' + m_sText );
	}
}

void CRichElement::PrePaintIcon(CDC* /*pDC*/)
{
	if ( m_hImage != NULL || m_sText.IsEmpty() ) return;

	UINT nID = 0, nWidth = 16, nHeight = 16;
	_stscanf( m_sText, _T("%u.%u.%u"), &nID, &nWidth, &nHeight );
	ASSERT( ( nWidth == 16 && nHeight == 16 ) || ( nWidth == 32 && nHeight == 32 ) );

	m_hImage = CoolInterface.ExtractIcon( nID, Settings.General.LanguageRTL,
		( nWidth == 16 ) ? LVSIL_SMALL : LVSIL_NORMAL );
	ASSERT( m_hImage != NULL );
}

//////////////////////////////////////////////////////////////////////
// CRichElement dimensions

CSize CRichElement::GetSize() const
{
	CSize sz( 0, 0 );

	if ( m_nType == retGap )
	{
		_stscanf( m_sText, _T("%ld"), &sz.cx );
	}
	else if ( m_nType == retBitmap && m_hImage != NULL )
	{
		BITMAP pInfo = {};
		GetObject( (HBITMAP)m_hImage, sizeof(pInfo), &pInfo );

		sz.cx = pInfo.bmWidth;
		sz.cy = pInfo.bmHeight;
	}
	else if ( m_nType == retIcon )
	{
		sz.cx = sz.cy = 16;
		UINT nID = 0;
		_stscanf( m_sText, _T("%u.%li.%li"), &nID, &sz.cx, &sz.cy );
	}
	else if ( m_nType == retEmoticon || m_nType == retCmdIcon )
	{
		sz.cx = sz.cy = 16;
	}
	else if ( m_nType == retAnchor )
	{
		sz.cx = sz.cy = 16;
		_stscanf( m_sText, _T("%li.%li"), &sz.cx, &sz.cy );
	}

	return sz;
}
