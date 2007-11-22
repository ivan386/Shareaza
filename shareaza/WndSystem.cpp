//
// WndSystem.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "WndSystem.h"
#include "Neighbours.h"
#include "Skin.h"
#include "CrawlSession.h"
#include "WindowManager.h"
#include "WndNeighbours.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CSystemWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CSystemWnd, CPanelWnd)
	//{{AFX_MSG_MAP(CSystemWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SYSTEM_CLEAR, OnSystemClear)
	ON_COMMAND(ID_SYSTEM_COPY, OnSystemCopy)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_SYSTEM_VERBOSE, OnUpdateSystemVerbose)
	ON_COMMAND(ID_SYSTEM_VERBOSE, OnSystemVerbose)
	ON_UPDATE_COMMAND_UI(ID_SYSTEM_TIMESTAMP, OnUpdateSystemTimestamp)
	ON_COMMAND(ID_SYSTEM_TIMESTAMP, OnSystemTimestamp)
	ON_COMMAND(ID_SYSTEM_TEST, OnSystemTest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSystemWnd construction

CSystemWnd::CSystemWnd() : CPanelWnd( TRUE, TRUE )
{
	Create( IDR_SYSTEMFRAME );
}

/////////////////////////////////////////////////////////////////////////////
// CSystemWnd operations

void CSystemWnd::Add(int nType, LPCTSTR pszText)
{
	if ( nType == MSG_DEBUG && ! Settings.General.Debug ) return;
	if ( nType != MSG_SYSTEM && nType != MSG_DISPLAYED_ERROR && !Settings.General.VerboseMode ) return;

	if ( Settings.General.ShowTimestamp )
	{
		CTime pNow = CTime::GetCurrentTime();
		CString strLine;
		strLine.Format( _T("[%.2i:%.2i:%.2i] %s"),
			pNow.GetHour(), pNow.GetMinute(), pNow.GetSecond(), pszText );
		m_wndText.AddLine( nType, strLine );
	}
	else
	{
		m_wndText.AddLine( nType, pszText );
	}
}

void CSystemWnd::Clear()
{
	m_wndText.Clear();
}

void CSystemWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();
	m_wndText.Clear( FALSE );
	if ( Settings.Live.LoadWindowState == FALSE )
	{
		ShowStartupText();
	}
}

void CSystemWnd::ShowStartupText()
{
	CString strBody;
	Skin.LoadString( strBody, IDS_SYSTEM_MESSAGE );

	strBody.Replace( _T("(version)"), (LPCTSTR)(theApp.m_sVersion + _T(" (") + theApp.m_sBuildDate + _T(")")) );

	for ( strBody += '\n' ; strBody.GetLength() ; )
	{
		CString strLine = strBody.SpanExcluding( _T("\r\n") );
		strBody = strBody.Mid( strLine.GetLength() + 1 );

		strLine.TrimLeft();
		strLine.TrimRight();
		if ( strLine.IsEmpty() ) continue;

		if ( strLine == _T(".") ) strLine.Empty();

		if ( _tcsnicmp( strLine, _T("!"), 1 ) == 0 )
			m_wndText.AddLine( MSG_SYSTEM, (LPCTSTR)strLine + 1 );
		else
			m_wndText.AddLine( MSG_DEFAULT, strLine );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSystemWnd message handlers

int CSystemWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CRect rc;
	m_wndText.Create( WS_VISIBLE, rc, this, 100 );

	LoadState( _T("CSystemWnd"), FALSE );
	theApp.Message( MSG_DEBUG, _T("IsG2HubCapable() = %i"), Neighbours.IsG2HubCapable() );
	theApp.Message( MSG_DEBUG, _T("IsG1UltrapeerCapable() = %i"), Neighbours.IsG1UltrapeerCapable() );

	return 0;
}

void CSystemWnd::OnDestroy()
{
	SaveState( _T("CSystemWnd") );
	CPanelWnd::OnDestroy();
}

void CSystemWnd::OnSize(UINT nType, int cx, int cy)
{
	CPanelWnd::OnSize( nType, cx, cy );
	m_wndText.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
}

void CSystemWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	TrackPopupMenu( _T("CSystemWnd"), point );
}

BOOL CSystemWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_PRIOR )
		{
			m_wndText.PostMessage( WM_VSCROLL, MAKELONG( SB_PAGEUP, 0 ), NULL );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_NEXT )
		{
			m_wndText.PostMessage( WM_VSCROLL, MAKELONG( SB_PAGEDOWN, 0 ), NULL );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_TAB )
		{
			GetManager()->Open( RUNTIME_CLASS(CNeighboursWnd) );
			return TRUE;
		}
	}

	return CPanelWnd::PreTranslateMessage(pMsg);
}

void CSystemWnd::OnUpdateSystemVerbose(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.General.VerboseMode );
}

void CSystemWnd::OnSystemVerbose()
{
	Settings.General.VerboseMode = ! Settings.General.VerboseMode;
}

void CSystemWnd::OnUpdateSystemTimestamp(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.General.ShowTimestamp );
}

void CSystemWnd::OnSystemTimestamp()
{
	Settings.General.ShowTimestamp = ! Settings.General.ShowTimestamp;
}

void CSystemWnd::OnSystemClear()
{
	Clear();
}

void CSystemWnd::OnSystemCopy()
{
	m_wndText.CopyText();
}

void CSystemWnd::OnSystemTest()
{
	CrawlSession.m_bActive = ! CrawlSession.m_bActive;

	if ( CrawlSession.m_bActive )
	{
		CrawlSession.Bootstrap();
	}
	else
	{
		theApp.Message( MSG_SYSTEM, _T("CCrawlSession: %i hubs, %i leaves"),
			CrawlSession.GetHubCount(), CrawlSession.GetLeafCount() );
	}
}
