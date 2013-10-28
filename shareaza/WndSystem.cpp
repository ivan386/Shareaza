//
// WndSystem.cpp
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
#include "WndSystem.h"
#include "Neighbours.h"
#include "Skin.h"
#include "CrawlSession.h"
#include "WindowManager.h"
#include "WndMain.h"
#include "WndNeighbours.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CSystemWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CSystemWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SYSTEM_CLEAR, OnSystemClear)
	ON_COMMAND(ID_SYSTEM_COPY, OnSystemCopy)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_SYSTEM_VERBOSE_ERROR, OnUpdateSystemVerboseError)
	ON_COMMAND(ID_SYSTEM_VERBOSE_ERROR, OnSystemVerboseError)
	ON_UPDATE_COMMAND_UI(ID_SYSTEM_VERBOSE_WARNING, OnUpdateSystemVerboseWarning)
	ON_COMMAND(ID_SYSTEM_VERBOSE_WARNING, OnSystemVerboseWarning)
	ON_UPDATE_COMMAND_UI(ID_SYSTEM_VERBOSE_NOTICE, OnUpdateSystemVerboseNotice)
	ON_COMMAND(ID_SYSTEM_VERBOSE_NOTICE, OnSystemVerboseNotice)
	ON_UPDATE_COMMAND_UI(ID_SYSTEM_VERBOSE_INFO, OnUpdateSystemVerboseInfo)
	ON_COMMAND(ID_SYSTEM_VERBOSE_INFO, OnSystemVerboseInfo)
	ON_UPDATE_COMMAND_UI(ID_SYSTEM_VERBOSE_DEBUG, OnUpdateSystemVerboseDebug)
	ON_COMMAND(ID_SYSTEM_VERBOSE_DEBUG, OnSystemVerboseDebug)
	ON_UPDATE_COMMAND_UI(ID_SYSTEM_TIMESTAMP, OnUpdateSystemTimestamp)
	ON_COMMAND(ID_SYSTEM_TIMESTAMP, OnSystemTimestamp)
	ON_COMMAND(ID_SYSTEM_TEST, OnSystemTest)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSystemWnd construction

CSystemWnd::CSystemWnd() : CPanelWnd( TRUE, TRUE )
{
	Create( IDR_SYSTEMFRAME );
}

/////////////////////////////////////////////////////////////////////////////
// CSystemWnd message handlers

int CSystemWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CRect rc;
	m_wndText.Create( WS_VISIBLE, rc, this, 100 );

	LoadState( _T("CSystemWnd"), FALSE );

	SetTimer( 1, 250, NULL );

	return 0;
}

void CSystemWnd::OnDestroy()
{
	KillTimer( 1 );

	SaveState( _T("CSystemWnd") );

	CPanelWnd::OnDestroy();
}

void CSystemWnd::OnTimer(UINT_PTR /*nIDEvent*/)
{
	CQuickLock pLock( theApp.m_csMessage );

	// Max 200 lines per second
	for ( int i = 0; i < 50 && ! theApp.m_oMessages.IsEmpty(); i++ )
	{
		CAutoPtr< CLogMessage > pMsg( theApp.m_oMessages.RemoveHead() );

		m_wndText.Add( pMsg );

		if ( ( pMsg->m_nType & MSG_TRAY ) == MSG_TRAY )
		{
			if ( CMainWnd* pWnd = theApp.CShareazaApp::SafeMainWnd() )
			{
				DWORD nType = NIIF_NONE;
				switch ( pMsg->m_nType & MSG_SEVERITY_MASK )
				{
				case MSG_ERROR:
					nType = NIIF_ERROR;
					break;

				case MSG_WARNING:
					nType = NIIF_WARNING;
					break;

				case MSG_NOTICE:
					nType = NIIF_INFO;
					break;

				case MSG_INFO:
					nType = NIIF_INFO;
					break;
				}
				pWnd->ShowTrayPopup( pMsg->m_strLog, CLIENT_NAME_T, nType );

				i = 50;
			}
		}
	}
}

void CSystemWnd::OnSize(UINT nType, int cx, int cy)
{
	CPanelWnd::OnSize( nType, cx, cy );
	m_wndText.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
}

void CSystemWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	Skin.TrackPopupMenu( _T("CSystemWnd"), point );
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
		else if ( pMsg->wParam == VK_HOME )
		{
			m_wndText.PostMessage( WM_VSCROLL, MAKELONG( SB_TOP, 0 ), NULL );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_END )
		{
			m_wndText.PostMessage( WM_VSCROLL, MAKELONG( SB_BOTTOM, 0 ), NULL );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_UP )
		{
			m_wndText.PostMessage( WM_VSCROLL, MAKELONG( SB_LINEUP, 0 ), NULL );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_DOWN )
		{
			m_wndText.PostMessage( WM_VSCROLL, MAKELONG( SB_LINEDOWN, 0 ), NULL );
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

void CSystemWnd::OnUpdateSystemVerboseError(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.General.LogLevel == MSG_ERROR );
}

void CSystemWnd::OnSystemVerboseError()
{
	Settings.General.LogLevel = MSG_ERROR;
}

void CSystemWnd::OnUpdateSystemVerboseWarning(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.General.LogLevel == MSG_WARNING );
}

void CSystemWnd::OnSystemVerboseWarning()
{
	Settings.General.LogLevel = MSG_WARNING;
}

void CSystemWnd::OnUpdateSystemVerboseNotice(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.General.LogLevel == MSG_NOTICE );
}

void CSystemWnd::OnSystemVerboseNotice()
{
	Settings.General.LogLevel = MSG_NOTICE;
}

void CSystemWnd::OnUpdateSystemVerboseInfo(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.General.LogLevel == MSG_INFO );
}

void CSystemWnd::OnSystemVerboseInfo()
{
	Settings.General.LogLevel = MSG_INFO;
}

void CSystemWnd::OnUpdateSystemVerboseDebug(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.General.LogLevel == MSG_DEBUG );
}

void CSystemWnd::OnSystemVerboseDebug()
{
	Settings.General.LogLevel = MSG_DEBUG;
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
	m_wndText.Clear();
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
		theApp.Message( MSG_NOTICE, _T("CCrawlSession: %i hubs, %i leaves"),
			CrawlSession.GetHubCount(), CrawlSession.GetLeafCount() );
	}
}
