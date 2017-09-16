//
// WindowManager.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "CoolInterface.h"
#include "CtrlWndTabBar.h"
#include "SearchManager.h"
#include "Skin.h"
#include "WindowManager.h"
#include "WndBrowseHost.h"
#include "WndDownloads.h"
#include "WndHome.h"
#include "WndMain.h"
#include "WndNeighbours.h"
#include "WndSearch.h"
#include "WndSystem.h"
#include "WndTraffic.h"
#include "WndUploads.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CWindowManager, CWnd)

BEGIN_MESSAGE_MAP(CWindowManager, CWnd)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////
// CWindowManager construction

CWindowManager::CWindowManager()
	: m_pParent			( NULL )
	, m_rcSize			( 0, 0, 0, 0 )
	, m_bIgnoreActivate	( FALSE )
{
}

CWindowManager::~CWindowManager()
{
}

//////////////////////////////////////////////////////////////////////
// CWindowManager owner

void CWindowManager::SetOwner(CMainWnd* pParent)
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

	if ( pChild && bFocus )
	{
		Sleep( 100 );
		pChild->BringWindowToTop();
	}

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
		SaveWindowStates();
	}

	Close();

	Settings.General.GUIMode = nMode;

	if ( bSaveState )
		Settings.Save();

	LoadSearchWindows();
	LoadBrowseHostWindows();
	LoadWindowStates();

	AutoResize();
	ShowWindow( SW_SHOW );
}

//////////////////////////////////////////////////////////////////////
// CWindowManager load complex window states

void CWindowManager::LoadWindowStates()
{
	CString strWindows = theApp.GetProfileString( _T("Windows"), _T("State") );

	CChildWnd* pDownloads = NULL;
	CChildWnd* pUploads = NULL;
	CChildWnd* pNeighbours = NULL;
	CChildWnd* pSystem = NULL;

	switch ( Settings.General.GUIMode )
	{
	case GUI_BASIC:
		pDownloads = Open( RUNTIME_CLASS( CDownloadsWnd ) );
		break;

	case GUI_TABBED:
		pDownloads = Open( RUNTIME_CLASS( CDownloadsWnd ) );
		pUploads = Open( RUNTIME_CLASS( CUploadsWnd ) );
		if ( pUploads ) pUploads->m_pGroupParent = pDownloads;
		pNeighbours = Open( RUNTIME_CLASS( CNeighboursWnd ) );
		pSystem = Open( RUNTIME_CLASS( CSystemWnd ) );
		if ( pSystem ) pSystem->m_pGroupParent = pNeighbours;
		break;

	case GUI_WINDOWED:
		break;
	}

	for ( strWindows += '|' ; strWindows.GetLength() > 1 ; )
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
		else if ( strClass.GetLength() &&
			strClass != _T("CMediaWnd") &&		// Never
			strClass != _T("CBrowseHostWnd") &&	// Open by LoadBrowseHostWindows()
			strClass != _T("CSearchWnd") &&		// Open by LoadSearchWindows()
			( ! pDownloads  || strClass != _T("CDownloadsWnd") ) &&
			( ! pUploads    || strClass != _T("CUploadsWnd") ) &&
			( ! pNeighbours || strClass != _T("CNeighboursWnd") ) &&
			( ! pSystem     || strClass != _T("CSystemWnd") ) )
		{
			CRuntimeClass* pClass = AfxClassForName( strClass );
			if ( pClass && pClass->IsDerivedFrom( RUNTIME_CLASS(CChildWnd) ) )
			{
				Open( pClass, FALSE, FALSE );
			}
		}
	}

	if ( Settings.General.GUIMode != GUI_WINDOWED )
		Open( RUNTIME_CLASS( CHomeWnd ), FALSE, TRUE );
}

//////////////////////////////////////////////////////////////////////
// CWindowManager save complex window states

void CWindowManager::SaveWindowStates() const
{
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

BOOL CWindowManager::LoadSearchWindows()
{
	const CString strFile = Settings.General.UserPath + _T("\\Data\\Searches.dat");

	CFile pFile;
	if ( ! pFile.Open( strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
		return FALSE;

	try
	{
		CArchive ar( &pFile, CArchive::load, 262144 );	// 256 KB buffer
		try
		{
			while ( ar.ReadCount() == 1 )
			{
				CSearchWnd* pWnd = new CSearchWnd();
				pWnd->Serialize( ar );
			}
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			theApp.Message( MSG_ERROR, _T("Failed to load search windows: %s"), (LPCTSTR)strFile );
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		theApp.Message( MSG_ERROR, _T("Failed to load search windows: %s"), (LPCTSTR)strFile );
		return FALSE;
	}

	return TRUE;
}

BOOL CWindowManager::SaveSearchWindows() const
{
	const CString strTemp = Settings.General.UserPath + _T("\\Data\\Searches.tmp");
	const CString strFile = Settings.General.UserPath + _T("\\Data\\Searches.dat");
	DWORD nCount = 0;

	CFile pFile;
	if ( ! pFile.Open( strTemp, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive | CFile::osSequentialScan ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, _T("Failed to save search windows: %s"), (LPCTSTR)strTemp );
		return FALSE;
	}

	try
	{
		CArchive ar( &pFile, CArchive::store, 262144 );	// 256 KB buffer
		try
		{
			DWORD nTotal = 0;
			for ( POSITION pos = GetIterator() ; pos ; )
			{
				CSearchWnd* pWnd = (CSearchWnd*)GetNext( pos );
				if ( pWnd->IsKindOf( RUNTIME_CLASS(CSearchWnd) ) &&
					 pWnd->GetLastSearch() )
				{
					++nTotal;
				}
			}
			DWORD nSkip = ( nTotal > Settings.Interface.SearchWindowsLimit ) ? ( nTotal - Settings.Interface.SearchWindowsLimit ) : 0;

			for ( POSITION pos = GetIterator() ; pos ; )
			{
				CSearchWnd* pWnd = (CSearchWnd*)GetNext( pos );
				if ( pWnd->IsKindOf( RUNTIME_CLASS(CSearchWnd) ) &&
					 pWnd->GetLastSearch() )
				{
					if ( nSkip )
					{
						--nSkip;
					}
					else
					{
						ar.WriteCount( 1 );
						pWnd->Serialize( ar );
						++nCount;
					}
				}
			}
			ar.WriteCount( 0 );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			DeleteFile( strTemp );
			theApp.Message( MSG_ERROR, _T("Failed to save search windows: %s"), (LPCTSTR)strTemp );
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, _T("Failed to save search windows: %s"), (LPCTSTR)strTemp );
		return FALSE;
	}

	if ( ! nCount )
	{
		DeleteFile( strTemp );
		DeleteFile( strFile );
	}
	else
	{
		if ( ! MoveFileEx( strTemp, strFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING ) )
		{
			DeleteFile( strTemp );
			theApp.Message( MSG_ERROR, _T("Failed to save search windows: %s"), (LPCTSTR)strFile );
			return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CWindowManager browse host load and save

BOOL CWindowManager::LoadBrowseHostWindows()
{
	const CString strFile = Settings.General.UserPath + _T("\\Data\\BrowseHosts.dat");

	CFile pFile;
	if ( ! pFile.Open( strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
		return FALSE;

	try
	{
		CArchive ar( &pFile, CArchive::load, 262144 );	// 256 KB buffer
		try
		{
			while ( ar.ReadCount() == 1 )
			{
				CBrowseHostWnd* pWnd = new CBrowseHostWnd();
				pWnd->Serialize( ar );
			}
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			theApp.Message( MSG_ERROR, _T("Failed to load browse host windows: %s"), (LPCTSTR)strFile );
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		theApp.Message( MSG_ERROR, _T("Failed to load browse host windows: %s"), (LPCTSTR)strFile );
		return FALSE;
	}

	return TRUE;
}

BOOL CWindowManager::SaveBrowseHostWindows() const
{
	const CString strTemp = Settings.General.UserPath + _T("\\Data\\BrowseHosts.tmp");
	const CString strFile = Settings.General.UserPath + _T("\\Data\\BrowseHosts.dat");
	DWORD nCount = 0;

	CFile pFile;
	if ( ! pFile.Open( strTemp, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive | CFile::osSequentialScan ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, _T("Failed to save browse host windows: %s"), (LPCTSTR)strTemp );
		return FALSE;
	}

	try
	{
		CArchive ar( &pFile, CArchive::store, 262144 );	// 256 KB buffer
		try
		{
			DWORD nTotal = 0;
			for ( POSITION pos = GetIterator() ; pos ; )
			{
				CBrowseHostWnd* pWnd = (CBrowseHostWnd*) GetNext( pos );
				if ( pWnd->IsKindOf( RUNTIME_CLASS(CBrowseHostWnd) ) )
				{
					++nTotal;
				}
			}
			DWORD nSkip = ( nTotal > Settings.Interface.BrowseWindowsLimit ) ? ( nTotal - Settings.Interface.BrowseWindowsLimit ) : 0;

			for ( POSITION pos = GetIterator() ; pos ; )
			{
				CBrowseHostWnd* pWnd = (CBrowseHostWnd*) GetNext( pos );
				if ( pWnd->IsKindOf( RUNTIME_CLASS(CBrowseHostWnd) ) )
				{
					if ( nSkip )
					{
						--nSkip;
					}
					else
					{
						ar.WriteCount( 1 );
						pWnd->Serialize( ar );
						++nCount;
					}
				}
			}
			ar.WriteCount( 0 );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			DeleteFile( strTemp );
			theApp.Message( MSG_ERROR, _T("Failed to save browse host windows: %s"), (LPCTSTR)strTemp );
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, _T("Failed to save browse host windows: %s"), (LPCTSTR)strTemp );
		return FALSE;
	}

	if ( ! nCount )
	{
		DeleteFile( strTemp );
		DeleteFile( strFile );
	}
	else
	{
		if ( ! MoveFileEx( strTemp, strFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING ) )
		{
			DeleteFile( strTemp );
			theApp.Message( MSG_ERROR, _T("Failed to save browse host windows: %s"), (LPCTSTR)strFile );
			return FALSE;
		}
	}

	return TRUE;
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
	CWnd::OnSize( nType, cx, cy );

	AutoResize();

	Invalidate();
}

BOOL CWindowManager::OnEraseBkgnd(CDC* /*pDC*/)
{
	return FALSE;
}

void CWindowManager::OnPaint()
{
	CPaintDC dc( this );

	CRect rc;
	GetClientRect( &rc );

	COLORREF crBackground = CoolInterface.m_crMediaWindow;

	if ( Settings.General.GUIMode == GUI_WINDOWED )
	{
		if ( HBITMAP hLogo = Skin.GetWatermark( _T("LargeLogo"), TRUE ) )
		{
			BITMAP pInfo = {};
			GetObject( hLogo, sizeof( BITMAP ), &pInfo );
			CPoint pt = rc.CenterPoint();
			pt.x -= pInfo.bmWidth / 2;
			pt.y -= pInfo.bmHeight / 2;
			CDC dcMem;
			dcMem.CreateCompatibleDC( &dc );
			HBITMAP pOldBmp = (HBITMAP)dcMem.SelectObject( hLogo );
			dc.BitBlt( pt.x, pt.y, pInfo.bmWidth, pInfo.bmHeight, &dcMem, 0, 0, SRCCOPY );
			crBackground = dc.GetPixel( pt.x, pt.y );
			dc.ExcludeClipRect( pt.x, pt.y, pt.x + pInfo.bmWidth, pt.y + pInfo.bmHeight );
			dcMem.SelectObject( pOldBmp );
		}
	}

	dc.FillSolidRect( &rc, crBackground );
}
