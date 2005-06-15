//
// WndMedia.cpp
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
#include "ImageServices.h"
#include "Skin.h"
#include "WndMedia.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CMediaWnd, CPanelWnd, 1)

BEGIN_MESSAGE_MAP(CMediaWnd, CPanelWnd)
	//{{AFX_MSG_MAP(CMediaWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_NCLBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_MESSAGE(0x0319, OnMediaKey)
	ON_MESSAGE(WM_DEVMODECHANGE, OnDevModeChange)
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMediaWnd construction

CMediaWnd::CMediaWnd() : CPanelWnd( TRUE, FALSE )
{
	m_bPanelClose = TRUE;
	Create( IDR_MEDIAFRAME );
}

CMediaWnd::~CMediaWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMediaWnd operations

BOOL CMediaWnd::PlayFile(LPCTSTR pszFile)
{
	return m_wndFrame.PlayFile( pszFile );
}

BOOL CMediaWnd::EnqueueFile(LPCTSTR pszFile)
{
	return m_wndFrame.EnqueueFile( pszFile );
}

BOOL CMediaWnd::IsPlaying()
{
	return m_wndFrame.IsPlaying();
}

void CMediaWnd::OnFileDelete(LPCTSTR pszFile)
{
	m_wndFrame.OnFileDelete( pszFile );
}

/////////////////////////////////////////////////////////////////////////////
// CMediaWnd message handlers

int CMediaWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndFrame.Create( this );
	LoadState();

	DragAcceptFiles();

	return 0;
}

void CMediaWnd::OnDestroy()
{
	SaveState();
	CPanelWnd::OnDestroy();
}

void CMediaWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();
	m_wndFrame.OnSkinChange();
}

BOOL CMediaWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndFrame.m_hWnd != NULL )
	{
		if ( m_wndFrame.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}

	return CPanelWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

LONG CMediaWnd::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	if ( m_wndFrame.m_hWnd != NULL && m_wndFrame.GetParent() != this )
	{
		m_wndFrame.OnUpdateCmdUI();
	}

	CPanelWnd::OnIdleUpdateCmdUI();

	return 0;
}

BOOL CMediaWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN )
	{
		if ( m_wndFrame.m_hWnd && m_wndFrame.PreTranslateMessage( pMsg ) ) return TRUE;
	}

	return CPanelWnd::PreTranslateMessage(pMsg);
}

void CMediaWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ( ( nID & 0xFFF0 ) == SC_CLOSE )
	{
		PostMessage( WM_CLOSE );
		return;
	}

	CPanelWnd::OnSysCommand( nID, lParam );
}

void CMediaWnd::OnSize(UINT nType, int cx, int cy)
{
	CPanelWnd::OnSize( nType, cx, cy );

	if ( m_wndFrame.m_hWnd != NULL && m_wndFrame.GetParent() == this )
	{
		m_wndFrame.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
	}
}

void CMediaWnd::OnPaint()
{
	CPaintDC dc( this );

	if ( m_wndFrame.m_hWnd == NULL || m_wndFrame.GetParent() != this )
	{
		CRect rc;
		GetClientRect( &rc );
		dc.FillSolidRect( &rc, 0 );
	}
}

LONG CMediaWnd::OnMediaKey(WPARAM wParam, LPARAM lParam)
{
	return m_wndFrame.SendMessage( 0x0319, wParam, lParam );
}

LONG CMediaWnd::OnDevModeChange(WPARAM wParam, LPARAM lParam)
{
	return m_wndFrame.SendMessage( WM_DEVMODECHANGE, wParam, lParam );
}

LONG CMediaWnd::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	return m_wndFrame.SendMessage( WM_DISPLAYCHANGE, wParam, lParam );
}

BOOL CMediaWnd::OnDropFiles(CStringList& pFiles, const CPoint& ptScreen, BOOL bDrop)
{
	if ( bDrop == FALSE ) return TRUE;

	CPoint pt( ptScreen );

	m_wndFrame.ScreenToClient( &pt );
	CWnd* pDropped = m_wndFrame.ChildWindowFromPoint( pt );

	BOOL bEnqueue = ( pDropped->IsKindOf( RUNTIME_CLASS(CMediaListCtrl) ) );

	for ( POSITION pos = pFiles.GetHeadPosition() ; pos ; )
	{
		CString strFile = pFiles.GetNext( pos );

		if ( bEnqueue )
			EnqueueFile( strFile );
		else
			PlayFile( strFile );
	}

	return TRUE;
}

void CMediaWnd::OnDropFiles(HDROP hDropInfo)
{
	if ( hDropInfo != NULL )
	{
		CStringList oFileList;
		TCHAR szFileName[MAX_PATH + 1];
		UINT nFiles = DragQueryFile( hDropInfo, (UINT)-1, NULL, 0 );
		for( UINT nNames = 0; nNames < nFiles; nNames++ )
		{
			ZeroMemory(szFileName, MAX_PATH + 1);
			DragQueryFile( hDropInfo, nNames, (LPTSTR)szFileName, MAX_PATH + 1 );
	        oFileList.AddTail( szFileName ); 
		}
		CPoint oPoint;
		POINT ppt;
		DragQueryPoint( hDropInfo, &ppt );
		oPoint.SetPoint( ppt.x, ppt.y );

		OnDropFiles( oFileList, oPoint, TRUE);
	}
}
