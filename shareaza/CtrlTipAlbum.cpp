//
// CtrlTipAlbum.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAlbumTipCtrl construction

CAlbumTipCtrl::CAlbumTipCtrl()
	: m_pAlbumFolder( NULL )
	, m_nIcon32( 0 )
	, m_bCollection( FALSE )
	, m_nKeyWidth( 0 )
	, m_crLight( CCoolInterface::CalculateColour( CoolInterface.m_crTipBack, RGB( 255, 255, 255 ), 128 ) )
{
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

	if ( ! m_pAlbumFolder || ! LibraryFolders.CheckAlbum( m_pAlbumFolder ) ) return FALSE;

	// Basic data

	m_sName	= m_pAlbumFolder->m_sName;
	m_sType	= _T("Virtual Folder");

	LoadString( m_sFilesTitle, IDS_TIP_TOTAL_FILES );
	m_sFiles.Format( _T("%lu"), m_pAlbumFolder->GetFileCount( TRUE ) );

	LoadString( m_sVolumeTitle, IDS_TIP_TOTAL_VOLUME );
	m_sVolume = Settings.SmartVolume( m_pAlbumFolder->GetFileVolume( TRUE ) );

	m_nIcon32 = 0;
	m_bCollection = bool( m_pAlbumFolder->m_oCollSHA1 );
	
	// Metadata

	m_pMetadata.Clear();

	if ( m_pAlbumFolder->m_pSchema != NULL )
	{
		CString strText;
		LPCTSTR pszColon = _tcschr( m_pAlbumFolder->m_pSchema->m_sTitle, ':' );
		m_sType = pszColon ? pszColon + 1 : m_pAlbumFolder->m_pSchema->m_sTitle;
		LoadString( strText, IDS_TIP_FOLDER );

		if ( Settings.General.LanguageRTL )
			m_sType = _T("\x202A") + m_sType + _T(" \x200E") + strText;
		else
			m_sType += _T(" ") + strText;

		m_nIcon32	= m_pAlbumFolder->m_pSchema->m_nIcon32;

		if ( m_pAlbumFolder->m_pXML != NULL )
		{
			m_pMetadata.Setup( m_pAlbumFolder->m_pSchema, FALSE );
			m_pMetadata.Combine( m_pAlbumFolder->m_pXML );
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

	m_sz.cy += TIP_TEXTHEIGHT;
	m_sz.cy += TIP_TEXTHEIGHT;

	m_sz.cy += TIP_RULE;

	m_sz.cy += max( (LONG)m_pMetadata.GetCount() * TIP_TEXTHEIGHT, (LONG)32 );

	int nValueWidth = 0;
	m_nKeyWidth = 0;
	m_pMetadata.ComputeWidth( pDC, m_nKeyWidth, nValueWidth );

	m_nKeyWidth = max( m_nKeyWidth, GetSize( pDC, m_sFilesTitle ) );
	nValueWidth = max( nValueWidth, GetSize( pDC, m_sFiles ) );

	m_nKeyWidth = max( m_nKeyWidth, GetSize( pDC, m_sVolumeTitle ) );
	nValueWidth = max( nValueWidth, GetSize( pDC, m_sVolume ) );

	if ( m_nKeyWidth ) m_nKeyWidth += TIP_GAP;
	m_sz.cx = max( m_sz.cx, (LONG)m_nKeyWidth + nValueWidth + 40 + 2 );
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

	if ( m_nIcon32 >= 0 )
	{
		ShellIcons.Draw( pDC, m_nIcon32, 32, pt.x, pt.y, CoolInterface.m_crTipBack );
		if ( m_bCollection )
			CoolInterface.Draw( pDC, IDI_COLLECTION_MASK, 16, pt.x, pt.y, CoolInterface.m_crTipBack );
		pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 32, pt.y + 32 );
	}

	for ( POSITION pos = m_pMetadata.GetIterator() ; pos ; )
	{
		CMetaItem* pItem = m_pMetadata.GetNext( pos );

		DrawText( pDC, &pt, pItem->m_sKey + ':', 40 );
		DrawText( pDC, &pt, pItem->m_sValue, 40 + m_nKeyWidth );
		pt.y += TIP_TEXTHEIGHT;
	}

	if ( m_pMetadata.GetIterator() )
	{
		pt.x += 40;
		DrawRule( pDC, &pt, TRUE );
		pt.x -= 40;
	}

	DrawText( pDC, &pt, m_sFilesTitle, 40 );
	DrawText( pDC, &pt, m_sFiles, 40 + m_nKeyWidth );
	pt.y += TIP_TEXTHEIGHT;

	DrawText( pDC, &pt, m_sVolumeTitle, 40 );
	DrawText( pDC, &pt, m_sVolume, 40 + m_nKeyWidth );
	pt.y += TIP_TEXTHEIGHT;
}
