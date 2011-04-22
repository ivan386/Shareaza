//
// CtrlTipFolder.cpp
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
#include "SharedFolder.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "CtrlTipFolder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CFolderTipCtrl, CCoolTipCtrl)

BEGIN_MESSAGE_MAP(CFolderTipCtrl, CCoolTipCtrl)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFolderTipCtrl construction

CFolderTipCtrl::CFolderTipCtrl()
	: m_pLibraryFolder( NULL )
	, m_nKeyWidth( 0 )
{
}

CFolderTipCtrl::~CFolderTipCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFolderTipCtrl prepare

BOOL CFolderTipCtrl::OnPrepare()
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;

	if ( ! m_pLibraryFolder || ! LibraryFolders.CheckFolder( m_pLibraryFolder, TRUE ) ) return FALSE;

	m_sName		= m_pLibraryFolder->m_sName;
	m_sPath		= m_pLibraryFolder->m_sPath;

	LoadString( m_sFilesTitle, IDS_TIP_TOTAL_FILES );
	m_sFiles.Format( _T("%lu"), m_pLibraryFolder->m_nFiles );

	LoadString( m_sVolumeTitle, IDS_TIP_TOTAL_VOLUME );
	m_sVolume = Settings.SmartVolume( m_pLibraryFolder->m_nVolume );

	LoadString( m_sFreeTitle, IDS_TIP_FREE_SPACE );
	ULARGE_INTEGER nFree = {}, nNull = {};
	GetDiskFreeSpaceEx( m_sPath, &nFree, &nNull, &nNull );
	m_sFree = Settings.SmartVolume( (QWORD)nFree.QuadPart );

	QWORD nTotal;
	LibraryMaps.GetStatistics( NULL, &nTotal );

	if ( nTotal )
		m_sPercentage.Format( _T("%.2f%% %s"),
			100.0 * ( m_pLibraryFolder->m_nVolume >> 10 ) / nTotal,
			(LPCTSTR)LoadString( IDS_TIP_LIBRARY_PERCENT ) );
	else
		m_sPercentage.Empty();

	CalcSizeHelper();

	return m_sz.cx > 0;
}

/////////////////////////////////////////////////////////////////////////////
// CFolderTipCtrl compute size

void CFolderTipCtrl::OnCalcSize(CDC* pDC)
{
	AddSize( pDC, m_sName );
	m_sz.cy += TIP_TEXTHEIGHT;

	pDC->SelectObject( &CoolInterface.m_fntNormal );
	AddSize( pDC, m_sPath );
	m_sz.cy += TIP_TEXTHEIGHT;

	m_sz.cy += TIP_RULE;

	AddSize( pDC, m_sFiles, 40 );
	m_sz.cy += TIP_TEXTHEIGHT;

	AddSize( pDC, m_sVolume, 40 );
	m_sz.cy += TIP_TEXTHEIGHT;

	AddSize( pDC, m_sPercentage, 40 );
	m_sz.cy += TIP_TEXTHEIGHT;

	m_sz.cy += TIP_RULE;

	AddSize( pDC, m_sFree, 40 );
	m_sz.cy += TIP_TEXTHEIGHT;

	m_nKeyWidth = 0;

	m_nKeyWidth = max( m_nKeyWidth, GetSize( pDC, m_sFilesTitle ) );
	m_nKeyWidth = max( m_nKeyWidth, GetSize( pDC, m_sVolumeTitle ) );
	m_nKeyWidth = max( m_nKeyWidth, GetSize( pDC, m_sFreeTitle ) );

	if ( m_nKeyWidth ) m_nKeyWidth += TIP_GAP;
	if ( m_nKeyWidth ) m_sz.cx += m_nKeyWidth;
}

/////////////////////////////////////////////////////////////////////////////
// CFolderTipCtrl painting

void CFolderTipCtrl::OnPaint(CDC* pDC)
{
	CPoint pt( 0, 0 );

	DrawText( pDC, &pt, m_sName );
	pt.y += TIP_TEXTHEIGHT;

	pDC->SelectObject( &CoolInterface.m_fntNormal );
	DrawText( pDC, &pt, m_sPath );
	pt.y += TIP_TEXTHEIGHT;

	DrawRule( pDC, &pt );

	CoolInterface.Draw( pDC, IDI_FOLDER_OPEN, 32, pt.x, pt.y, CoolInterface.m_crTipBack );
	pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 32, pt.y + 32 );

	DrawText( pDC, &pt, m_sFilesTitle, 40 );
	DrawText( pDC, &pt, m_sFiles, 40 + m_nKeyWidth );
	pt.y += TIP_TEXTHEIGHT;

	DrawText( pDC, &pt, m_sVolumeTitle, 40 );
	DrawText( pDC, &pt, m_sVolume, 40 + m_nKeyWidth );
	pt.y += TIP_TEXTHEIGHT;

	DrawText( pDC, &pt, m_sPercentage, 40 + m_nKeyWidth );
	pt.y += TIP_TEXTHEIGHT;

	pt.x += 40;
	DrawRule( pDC, &pt, TRUE );
	pt.x -= 40;

	DrawText( pDC, &pt, m_sFreeTitle, 40 );
	DrawText( pDC, &pt, m_sFree, 40 + m_nKeyWidth );
	pt.y += TIP_TEXTHEIGHT;
}

