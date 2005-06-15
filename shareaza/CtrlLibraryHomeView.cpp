//
// CtrlLibraryHomeView.cpp
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
#include "CoolInterface.h"
#include "Library.h"
#include "AlbumFolder.h"
#include "ShellIcons.h"
#include "SchemaCache.h"
#include "Schema.h"
#include "Skin.h"

#include "CtrlLibraryFrame.h"
#include "CtrlLibraryHomeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLibraryHomeView, CLibraryView)

BEGIN_MESSAGE_MAP(CLibraryHomeView, CLibraryView)
	//{{AFX_MSG_MAP(CLibraryHomeView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryHomeView construction

CLibraryHomeView::CLibraryHomeView()
{
	m_nCommandID	= ID_LIBRARY_VIEW_HOME;
	m_pszToolBar	= _T("CLibraryHomeView");
}

CLibraryHomeView::~CLibraryHomeView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryHomeView message handlers

BOOL CLibraryHomeView::CheckAvailable(CLibraryTreeItem* pSel)
{
	m_bAvailable = ( pSel == NULL );
	return m_bAvailable;
}

int CLibraryHomeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CLibraryView::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndTile.Create( this );
	m_wndTile.SetOwner( GetOwner() );
	m_wndTile.ShowWindow( SW_SHOW );

	return 0;
}

void CLibraryHomeView::OnDestroy()
{
	CLibraryView::OnDestroy();
}

void CLibraryHomeView::OnSize(UINT nType, int cx, int cy)
{
	if ( nType != 1982 ) CLibraryView::OnSize( nType, cx, cy );

	CRect rc;
	GetClientRect( &rc );
	m_wndTile.MoveWindow( &rc );
}

void CLibraryHomeView::GetHeaderContent(int& nImage, CString& str)
{
	if ( CSchema* pSchema = SchemaCache.Get( CSchema::uriLibrary ) )
	{
		nImage = pSchema->m_nIcon16;
		LoadString( str, IDS_LIBHEAD_EXPLORE_FOLDER );
		LPCTSTR psz = _tcschr( pSchema->m_sTitle, ':' );
		if ( theApp.m_bRTL )
		{
			CString strCaption( psz ? psz + 1 : pSchema->m_sTitle );
			str = _T("\x202A") + strCaption + _T(" \x200E") + str;
		}
		else
			str += psz ? psz + 1 : pSchema->m_sTitle;
	}
	else
	{
		nImage = SHI_COMPUTER;
		LoadString( str, IDS_LIBHEAD_HOME );
	}
}

void CLibraryHomeView::Update()
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	m_wndTile.Update();
}

BOOL CLibraryHomeView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndTile.m_hWnd != NULL )
	{
		if ( m_wndTile.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}

	return CLibraryView::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

void CLibraryHomeView::OnSetFocus(CWnd* pOldWnd)
{
	CLibraryView::OnSetFocus( pOldWnd );
	if ( m_wndTile.m_hWnd ) m_wndTile.SetFocus();
}
