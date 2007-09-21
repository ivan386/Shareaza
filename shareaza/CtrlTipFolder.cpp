//
// CtrlTipFolder.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
	//{{AFX_MSG_MAP(CFolderTipCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFolderTipCtrl construction

CFolderTipCtrl::CFolderTipCtrl()
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

	CLibraryFolder* pFolder = (CLibraryFolder*)m_pContext;
	if ( ! LibraryFolders.CheckFolder( pFolder, TRUE ) ) return FALSE;

	m_sName		= pFolder->m_sName;
	m_sPath		= pFolder->m_sPath;

	m_sFiles.Format( _T("%lu"), pFolder->m_nFiles );
	m_sVolume = Settings.SmartVolume( pFolder->m_nVolume );

	QWORD nTotal;
	CString strText;
	LibraryMaps.GetStatistics( NULL, &nTotal );

	LoadString( strText, IDS_TIP_LIBRARY_PERCENT );
	m_sPercentage.Format( _T("%.2f%% %s"),
		100.0 * ( pFolder->m_nVolume >> 10 ) / nTotal, strText );

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

	m_sz.cy += TIP_RULE;

	AddSize( pDC, m_sFiles, 120 );
	AddSize( pDC, m_sVolume, 120 );
	AddSize( pDC, m_sPercentage, 40 );

	m_sz.cy += TIP_TEXTHEIGHT * 4;
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

	ShellIcons.Draw( pDC, SHI_FOLDER_OPEN, 32, pt.x, pt.y, CoolInterface.m_crTipBack );

	CString strText;
	LoadString( strText, IDS_TIP_TOTAL_FILES );
	DrawText( pDC, &pt, strText, 40 );
	DrawText( pDC, &pt, m_sFiles, 120 );
	pt.y += TIP_TEXTHEIGHT;
	LoadString( strText, IDS_TIP_TOTAL_VOLUME );
	DrawText( pDC, &pt, strText, 40 );
	DrawText( pDC, &pt, m_sVolume, 120 );
	pt.y += TIP_TEXTHEIGHT;
	DrawText( pDC, &pt, m_sPercentage, 40 );
	pt.y += TIP_TEXTHEIGHT;
}

