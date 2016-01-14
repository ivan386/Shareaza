//
// WndHitMonitor.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "QuerySearch.h"
#include "QueryHit.h"
#include "MatchObjects.h"
#include "Network.h"
#include "Packet.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "Skin.h"

#include "WndHitMonitor.h"
#include "WndSearch.h"
#include "DlgHitColumns.h"
#include "DlgNewSearch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CHitMonitorWnd, CBaseMatchWnd, 0)

BEGIN_MESSAGE_MAP(CHitMonitorWnd, CBaseMatchWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_HITMONITOR_CLEAR, OnHitMonitorClear)
	ON_UPDATE_COMMAND_UI(ID_HITMONITOR_PAUSE, OnUpdateHitMonitorPause)
	ON_COMMAND(ID_HITMONITOR_PAUSE, OnHitMonitorPause)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CHitMonitorWnd construction

CHitMonitorWnd::CHitMonitorWnd()
{
	Create( IDR_HITMONITORFRAME );
	m_bPaused = FALSE;
}

CHitMonitorWnd::~CHitMonitorWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHitMonitorWnd message handlers

int CHitMonitorWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CBaseMatchWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_pMatches->m_sFilter = Settings.Search.MonitorFilter;
	m_pMatches->Filter();

	if ( CSchemaPtr pSchema = SchemaCache.Get( Settings.Search.MonitorSchemaURI ) )
	{
		CSchemaMemberList pColumns;
		CSchemaColumnsDlg::LoadColumns( pSchema, &pColumns );
		m_wndList.SelectSchema( pSchema, &pColumns );
	}

	LoadState( _T("CHitMonitorWnd"), TRUE );

	return 0;
}

void CHitMonitorWnd::OnDestroy()
{
	Settings.Search.MonitorFilter = m_pMatches->m_sFilter;

	if ( m_wndList.m_pSchema )
	{
		Settings.Search.MonitorSchemaURI = m_wndList.m_pSchema->GetURI();
	}
	else
	{
		Settings.Search.MonitorSchemaURI.Empty();
	}

	SaveState( _T("CHitMonitorWnd") );

	CBaseMatchWnd::OnDestroy();
}

void CHitMonitorWnd::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if ( m_bContextMenu )
	{
		Skin.TrackPopupMenu( _T("CHitMonitorWnd"), point, ID_SEARCH_DOWNLOAD );
	}
	else
	{
		CBaseMatchWnd::OnContextMenu( pWnd, point );
	}
}

void CHitMonitorWnd::OnUpdateHitMonitorPause(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bPaused );
}

void CHitMonitorWnd::OnHitMonitorPause()
{
	m_bPaused = ! m_bPaused;
}

void CHitMonitorWnd::OnHitMonitorClear()
{
	m_wndList.DestructiveUpdate();
	m_pMatches->Clear();
	m_bUpdate = TRUE;
	PostMessage( WM_TIMER, 2 );
}

/////////////////////////////////////////////////////////////////////////////
// CHitMonitorWnd event handlers

void CHitMonitorWnd::OnSkinChange()
{
	CBaseMatchWnd::OnSkinChange();
	Skin.Translate( _T("CMatchCtrl"), &m_wndList.m_wndHeader );
	Skin.CreateToolBar( _T("CHitMonitorWnd"), &m_wndToolBar );
}

BOOL CHitMonitorWnd::OnQueryHits(const CQueryHit* pHits)
{
	if ( m_bPaused || m_hWnd == NULL ) return FALSE;

	CSingleLock pLock( &m_pMatches->m_pSection );

	if ( pLock.Lock( 100 ) && ! m_bPaused )
	{
		m_pMatches->AddHits( pHits );
		m_bUpdate = TRUE;
		return TRUE;
	}

	return FALSE;
}
