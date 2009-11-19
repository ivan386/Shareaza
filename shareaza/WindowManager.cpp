//
// WindowManager.cpp
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
#include "WindowManager.h"
#include "Skin.h"
#include "CtrlWndTabBar.h"

#include "WndHome.h"
#include "WndSystem.h"
#include "WndNeighbours.h"
#include "WndDownloads.h"
#include "WndUploads.h"

#include "WndTraffic.h"
#include "WndSearch.h"
#include "WndBrowseHost.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CWindowManager, CWnd)

BEGIN_MESSAGE_MAP(CWindowManager, CWnd)
	//{{AFX_MSG_MAP(CWindowManager)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////
// CWindowManager construction

CWindowManager::CWindowManager(CMDIFrameWnd* pParent)
{
	m_bIgnoreActivate	= FALSE;

	if ( pParent ) SetOwner( pParent );
}

CWindowManager::~CWindowManager()
{
}

//////////////////////////////////////////////////////////////////////
// CWindowManager owner

void CWindowManager::SetOwner(CMDIFrameWnd* pParent)
{
	m_pParent = pParent;
	SubclassWindow( m_pParent->m_hWndMDIClient );
}

//////////////////////////////////////////////////////////////////////
// CWindowManager add and remove

void CWindowManager::Add(CChildWnd* pChild)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );
	if ( m_pWindows.Find( pChild ) == NULL ) m_pWindows.AddTail( pChild );
}

void CWindowManager::Remove(CChildWnd* pChild)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );
	POSITION pos = m_pWindows.Find( pChild );
	if ( pos ) m_pWindows.RemoveAt( pos );
}

//////////////////////////////////////////////////////////////////////
// CWindowManager get active

CChildWnd* CWindowManager::GetActive() const
{
	if ( m_hWnd == NULL ) return NULL;

	CWnd* pActive = m_pParent->MDIGetActive();

	if ( pActive && pActive->IsKindOf( RUNTIME_CLASS(CChildWnd) ) )
		return (CChildWnd*)pActive;
	else
		return NULL;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager iteration

POSITION CWindowManager::GetIterator() const
{
	return m_pWindows.GetHeadPosition();
}

CChildWnd* CWindowManager::GetNext(POSITION& pos) const
{
	return m_pWindows.GetNext( pos );
}

BOOL CWindowManager::Check(CChildWnd* pChild) const
{
	return m_pWindows.Find( pChild ) != NULL;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager find

CChildWnd* CWindowManager::Find(CRuntimeClass* pClass, CChildWnd* pAfter, CChildWnd* pExcept)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );
	BOOL bFound = ( pAfter == NULL );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChildWnd* pChild = GetNext( pos );

		if ( pChild == pExcept ) continue;
		else if ( bFound && ( !pClass || pChild->IsKindOf( pClass ) ) ) return pChild;
		else if ( pChild == pAfter ) bFound = TRUE;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager open / toggle

CChildWnd* CWindowManager::Open(CRuntimeClass* pClass, BOOL bToggle, BOOL bFocus)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	CChildWnd* pChild = Find( pClass );

	if ( pChild && pChild->IsIconic() )
	{
		pChild->ShowWindow( SW_SHOWNORMAL );
		bToggle = FALSE;
	}

	if ( pChild && pChild->m_bTabMode )
		bToggle = FALSE;

	if ( pChild && bToggle && GetActive() == pChild )
	{
		pChild->DestroyWindow();
		return NULL;
	}

	pLock.Unlock();

	if ( ! pChild )
		pChild = static_cast< CChildWnd* >( pClass->CreateObject() );

	if ( pChild && bFocus  )
		pChild->BringWindowToTop();

	return pChild;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager find by point

CChildWnd* CWindowManager::FindFromPoint(const CPoint& point) const
{
	CPoint pt( point );

	ScreenToClient( &pt );
	CChildWnd* pWnd = (CChildWnd*)ChildWindowFromPoint( pt );

	return ( pWnd != NULL && pWnd->IsKindOf( RUNTIME_CLASS(CChildWnd) ) ) ? pWnd : NULL;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager close

void CWindowManager::Close()
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );
	CList< CChildWnd* > pClose;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChildWnd* pChild = GetNext( pos );
		pClose.AddTail( pChild );

		pChild->RemoveSkin();
	}

	for ( POSITION pos = pClose.GetHeadPosition() ; pos ; )
	{
		CChildWnd* pChild = pClose.GetNext( pos );
		pChild->DestroyWindow();
	}
}

//////////////////////////////////////////////////////////////////////
// CWindowManager automatic resize

void CWindowManager::AutoResize()
{
	CChildWnd* pChild;
	CRect rcSize;

	GetClientRect( &rcSize );
	if ( rcSize.right < 64 || rcSize.bottom < 64 ) return;

	for ( pChild = (CChildWnd*)GetWindow( GW_CHILD ) ; pChild ; pChild = (CChildWnd*)pChild->GetNextWindow() )
	{
		if ( ! pChild->IsKindOf( RUNTIME_CLASS(CChildWnd) ) ) continue;

		CRect rcChild;
		pChild->GetWindowRect( &rcChild );
		ScreenToClient( &rcChild );

		if ( rcChild.right == m_rcSize.right || rcChild.bottom == m_rcSize.bottom )
		{
			if ( rcChild.right == m_rcSize.right ) rcChild.right = rcSize.right;
			if ( rcChild.bottom == m_rcSize.bottom ) rcChild.bottom = rcSize.bottom;

			pChild->MoveWindow( &rcChild );
		}
	}

	m_rcSize = rcSize;

	if ( Settings.General.GUIMode != GUI_WINDOWED )
	{
		for ( pChild = (CChildWnd*)GetWindow( GW_CHILD ) ; pChild ; pChild = (CChildWnd*)pChild->GetNextWindow() )
		{
			if ( ! pChild->IsKindOf( RUNTIME_CLASS(CChildWnd) ) ) continue;

			if ( pChild->m_bGroupMode && pChild->m_pGroupParent )
			{
				CRect rcChild( &rcSize );
				rcChild.bottom = (int)( pChild->m_pGroupParent->m_nGroupSize * rcChild.bottom );
				pChild->m_pGroupParent->MoveWindow( &rcChild );
				rcChild.top = rcChild.bottom;
				rcChild.bottom = rcSize.bottom;
				pChild->MoveWindow( &rcChild );
			}
			else if ( pChild->m_bPanelMode && ! pChild->m_bGroupMode )
			{
				CRect rcChild;
				pChild->GetWindowRect( &rcChild );
				ScreenToClient( &rcChild );
				if ( rcChild != rcSize ) pChild->MoveWindow( &rcSize );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CWindowManager customized cascade

void CWindowManager::Cascade(BOOL bActiveOnly)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	CChildWnd* pActive = bActiveOnly ? GetActive() : NULL;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChildWnd* pChild = GetNext( pos );

		if ( ! pChild->m_bPanelMode && pChild->IsWindowVisible() &&
			 ! pChild->IsIconic() && ( pChild == pActive || ! pActive ) )
		{
			CRect rcClient;
			pChild->ShowWindow( SW_RESTORE );
			pChild->GetParent()->GetClientRect( &rcClient );
			pChild->MoveWindow( &rcClient );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CWindowManager activate grouped windows

void CWindowManager::ActivateGrouped(CChildWnd* pExcept)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	if ( m_bIgnoreActivate ) return;
	m_bIgnoreActivate = TRUE;

	CChildWnd* pParent = pExcept->m_pGroupParent ? pExcept->m_pGroupParent : pExcept;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChildWnd* pChild = GetNext( pos );

		if ( pChild != pExcept )
		{
			if ( pChild->m_bGroupMode && ( pChild == pParent || pChild->m_pGroupParent == pParent ) )
			{
				pChild->ShowWindow( SW_SHOWNORMAL );
				pChild->MDIActivate();
			}
		}
	}

	pExcept->MDIActivate();
	m_bIgnoreActivate = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager GUI mode

void CWindowManager::SetGUIMode(int nMode, BOOL bSaveState)
{
	ModifyStyle( WS_VISIBLE, 0 );

	if ( bSaveState )
	{
		SaveSearchWindows();
		SaveBrowseHostWindows();
		if ( Settings.General.GUIMode == GUI_WINDOWED )
			SaveWindowStates();
	}

	Close();

	Settings.General.GUIMode = nMode;
	Settings.Save();

	if ( Settings.General.GUIMode != GUI_WINDOWED )
	{
		CreateTabbedWindows();
	}
	else
	{
		LoadWindowStates();
	}

	LoadSearchWindows();
	LoadBrowseHostWindows();

	AutoResize();
	ShowWindow( SW_SHOW );
}

//////////////////////////////////////////////////////////////////////
// CWindowManager create tabbed windows

void CWindowManager::CreateTabbedWindows()
{
	CDownloadsWnd* pDownloads = new CDownloadsWnd();

	if ( Settings.General.GUIMode != GUI_BASIC )
	{
		CUploadsWnd* pUploads = new CUploadsWnd();
		pUploads->m_pGroupParent = pDownloads;

		CNeighboursWnd* pNeighbours = new CNeighboursWnd();
		CSystemWnd* pSystem = new CSystemWnd();
		pSystem->m_pGroupParent = pNeighbours;
	}

	/*CHomeWnd* pHome =*/ new CHomeWnd();
}

//////////////////////////////////////////////////////////////////////
// CWindowManager load complex window states

void CWindowManager::LoadWindowStates()
{
	if ( Settings.General.GUIMode != GUI_WINDOWED ) return;

	CString strWindows = theApp.GetProfileString( _T("Windows"), _T("State"),
		_T("CSystemWnd|CNeighboursWnd") );

	for ( strWindows += '|' ; strWindows.GetLength() ; )
	{
		CString strClass = strWindows.SpanExcluding( _T("| ,.\t") );
		strWindows = strWindows.Mid( strClass.GetLength() + 1 );

		if ( strClass.Find( _T("TG#") ) == 0 )
		{
			DWORD nUnique;

			if ( _stscanf( (LPCTSTR)strClass + 3, _T("%lu"), &nUnique ) == 1 )
			{
				new CTrafficWnd( nUnique );
			}
		}
		else if ( strClass.GetLength() )
		{
			CRuntimeClass* pClass = AfxClassForName( strClass );

			if ( pClass && pClass->IsDerivedFrom( RUNTIME_CLASS(CChildWnd) ) )
			{
				Open( pClass );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CWindowManager save complex window states

void CWindowManager::SaveWindowStates() const
{
//	if ( Settings.General.GUIMode != GUI_WINDOWED ) return;

	CString strWindows;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChildWnd* pChild = GetNext( pos );

		pChild->SaveState();

		if ( strWindows.GetLength() ) strWindows += '|';

		if ( pChild->IsKindOf( RUNTIME_CLASS(CTrafficWnd) ) )
		{
			CTrafficWnd* pTraffic = (CTrafficWnd*)pChild;
			CString strItem;
			strItem.Format( _T("TG#%.4x"), pTraffic->m_nUnique );
			strWindows += strItem;
		}
		else
		{
			strWindows += pChild->GetRuntimeClass()->m_lpszClassName;
		}
	}

	theApp.WriteProfileString( _T("Windows"), _T("State"), strWindows );
}

//////////////////////////////////////////////////////////////////////
// CWindowManager search load and save

void CWindowManager::LoadSearchWindows()
{
	CString strFile = Settings.General.UserPath + _T("\\Data\\Searches.dat");

	try
	{
		CFile pFile;
		if ( ! pFile.Open( strFile, CFile::modeRead ) )
			return;

		CArchive ar( &pFile, CArchive::load );
		while ( ar.ReadCount() == 1 )
		{
			CSearchWnd* pWnd = new CSearchWnd();
			pWnd->Serialize( ar );
		}
	}
	catch ( CException* pException )
	{
		pException->Delete();
	}

	if ( Settings.General.GUIMode != GUI_WINDOWED )
		Open( RUNTIME_CLASS(CHomeWnd) );
}

void CWindowManager::SaveSearchWindows() const
{
	CString strFile = Settings.General.UserPath + _T("\\Data\\Searches.dat");
	int nCount = 0;

	try
	{
		CFile pFile;
		if ( ! pFile.Open( strFile, CFile::modeWrite | CFile::modeCreate ) )
			return;

		CArchive ar( &pFile, CArchive::store );
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CSearchWnd* pWnd = (CSearchWnd*)GetNext( pos );

			if ( pWnd->IsKindOf( RUNTIME_CLASS(CSearchWnd) ) && pWnd->GetLastSearch() )
			{
				ar.WriteCount( 1 );
				pWnd->Serialize( ar );
				nCount++;
			}
		}

		ar.WriteCount( 0 );
	}
	catch ( CException* pException )
	{
		pException->Delete();
	}

	if ( ! nCount )
		DeleteFileEx( strFile, FALSE, FALSE, FALSE );
}

//////////////////////////////////////////////////////////////////////
// CWindowManager browse host load and save

void CWindowManager::LoadBrowseHostWindows()
{
	CString strFile = Settings.General.UserPath + _T("\\Data\\BrowseHosts.dat");

	try
	{
		CFile pFile;
		if ( ! pFile.Open( strFile, CFile::modeRead ) )
			return;

		CArchive ar( &pFile, CArchive::load );
		while ( ar.ReadCount() == 1 )
		{
			CBrowseHostWnd* pWnd = new CBrowseHostWnd();
			pWnd->Serialize( ar );
		}
	}
	catch ( CException* pException )
	{
		pException->Delete();
	}

	if ( Settings.General.GUIMode != GUI_WINDOWED )
		Open( RUNTIME_CLASS(CHomeWnd) );
}

void CWindowManager::SaveBrowseHostWindows() const
{
	CString strFile = Settings.General.UserPath + _T("\\Data\\BrowseHosts.dat");
	int nCount = 0;

	try
	{
		CFile pFile;
		if ( ! pFile.Open( strFile, CFile::modeWrite | CFile::modeCreate ) )
			return;

		CArchive ar( &pFile, CArchive::store );
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CBrowseHostWnd* pWnd = (CBrowseHostWnd*) GetNext( pos );

			if ( pWnd->IsKindOf( RUNTIME_CLASS(CBrowseHostWnd) ) )
			{
				ar.WriteCount( 1 );
				pWnd->Serialize( ar );
				nCount++;
			}
		}

		ar.WriteCount( 0 );
	}
	catch ( CException* pException )
	{
		pException->Delete();
	}

	if ( ! nCount )
		DeleteFileEx( strFile, FALSE, FALSE, FALSE );
}

//////////////////////////////////////////////////////////////////////
// CWindowManager new blank search window

void CWindowManager::OpenNewSearchWindow()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CSearchWnd* pChild = (CSearchWnd*)GetNext( pos );

		if ( pChild->IsKindOf( RUNTIME_CLASS(CSearchWnd) ) )
		{
			if ( ! pChild->GetLastSearch() )
			{
				if ( pChild->IsIconic() ) pChild->ShowWindow( SW_SHOWNORMAL );
				pChild->BringWindowToTop();
				return;
			}
		}
	}

	new CSearchWnd();
}

//////////////////////////////////////////////////////////////////////
// CWindowManager skin change

void CWindowManager::PostSkinChange()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChildWnd* pChildWnd = GetNext( pos );
		pChildWnd->OnSkinChange();
	}
}

void CWindowManager::PostSkinRemove()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->RemoveSkin();
	}
}

//////////////////////////////////////////////////////////////////////
// CWindowManager message handlers

void CWindowManager::OnSize(UINT nType, int cx, int cy)
{
	if ( nType != 1982 )
		CWnd::OnSize( nType, cx, cy );

	AutoResize();
}

BOOL CWindowManager::OnEraseBkgnd(CDC* pDC)
{
	CRect rc;
	GetClientRect( &rc );
	pDC->FillSolidRect( &rc, Skin.m_crPanelBack );

	return TRUE;
}

void CWindowManager::OnPaint()
{
	CPaintDC dc( this );
}
