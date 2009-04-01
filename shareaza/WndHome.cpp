//
// WndHome.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "Network.h"
#include "Neighbours.h"
#include "Library.h"
#include "LibraryHistory.h"
#include "SharedFile.h"
#include "VersionChecker.h"
#include "GraphItem.h"
#include "Statistics.h"

#include "WndHome.h"
#include "WndSearch.h"
#include "WndBrowseHost.h"
#include "WindowManager.h"
#include "Skin.h"
#include "XML.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CHomeWnd, CPanelWnd)

BEGIN_MESSAGE_MAP(CHomeWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_TIMER()
	ON_WM_MDIACTIVATE()
	ON_NOTIFY(RVN_CLICK, IDC_HOME_VIEW, OnClickView)
	ON_NOTIFY(RVN_CLICK, 1, OnClickView)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHomeWnd construction

CHomeWnd::CHomeWnd() : CPanelWnd( TRUE )
{
	Create( IDR_HOMEFRAME );
}

/////////////////////////////////////////////////////////////////////////////
// CHomeWnd message handlers

int CHomeWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	if ( ! m_wndView.Create( rectDefault, this ) ) return -1;
	if ( ! m_wndPanel.Create( this ) ) return -1;

	LoadState();

	OnSkinChange();

	return 0;
}

void CHomeWnd::OnDestroy() 
{
	SaveState();
	CPanelWnd::OnDestroy();
}

void CHomeWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();

	m_wndView.OnSkinChange();
	m_wndPanel.OnSkinChange();
}

void CHomeWnd::OnSize(UINT nType, int cx, int cy)
{
	CPanelWnd::OnSize( nType, cx, cy );

	m_wndPanel.SetWindowPos( NULL, 0, 0, PANEL_WIDTH, cy, SWP_NOZORDER );
	m_wndView.SetWindowPos( NULL, PANEL_WIDTH, 0, cx - PANEL_WIDTH, cy, SWP_NOZORDER|SWP_SHOWWINDOW );
}

void CHomeWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	Skin.TrackPopupMenu( _T("CHomeWnd"), point );
}

void CHomeWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 2 || ( nIDEvent == 1 && IsActive() ) )
	{
		m_wndView.Update();
		m_wndPanel.Update();
	}
}

void CHomeWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	if ( bActivate )
	{
		m_wndView.Update();
		m_wndPanel.Update();

		m_wndView.Activate();
	}

	CPanelWnd::OnMDIActivate( bActivate, pActivateWnd, pDeactivateWnd );
}

void CHomeWnd::OnClickView(NMHDR* pNotify, LRESULT* /*pResult*/)
{
	if ( CRichElement* pElement = ((RVN_ELEMENTEVENT*)pNotify)->pElement )
	{
		theApp.InternalURI( pElement->m_sLink );
	}

	PostMessage( WM_TIMER, 2 );
}
