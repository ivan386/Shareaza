//
// WndLibrary.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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
#include "LibraryBuilder.h"
#include "CollectionFile.h"
#include "SharedFile.h"
#include "WndLibrary.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CLibraryWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CLibraryWnd, CPanelWnd)
	//{{AFX_MSG_MAP(CLibraryWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_MDIACTIVATE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CLibraryWnd construction

CLibraryWnd::CLibraryWnd() : CPanelWnd( TRUE, FALSE )
{
	Create( IDR_LIBRARYFRAME );
}

CLibraryWnd::~CLibraryWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryWnd operations

BOOL CLibraryWnd::Display(CLibraryFile* pFile)
{
	return m_wndFrame.Display( pFile );
}

BOOL CLibraryWnd::Display(CAlbumFolder* pFolder)
{
	return m_wndFrame.Display( pFolder );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryWnd message handlers

int CLibraryWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	m_tLast = 0;

	m_wndFrame.Create( this );

	LoadState();

	return 0;
}

void CLibraryWnd::OnDestroy() 
{
	SaveState();
	CPanelWnd::OnDestroy();
}

BOOL CLibraryWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if ( m_wndFrame.m_hWnd )
	{
		if ( m_wndFrame.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}
	
	return CPanelWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

void CLibraryWnd::OnSize(UINT nType, int cx, int cy) 
{
	CPanelWnd::OnSize( nType, cx, cy );
	if ( m_wndFrame.m_hWnd ) m_wndFrame.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
}

void CLibraryWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) 
{
	CPanelWnd::OnMDIActivate( bActivate, pActivateWnd, pDeactivateWnd );

	if ( bActivate )
	{
		m_wndFrame.Update();
		m_wndFrame.SetFocus();
	}
}

void CLibraryWnd::OnTimer(UINT nIDEvent) 
{
	DWORD tNow = GetTickCount();

	if ( IsPartiallyVisible() )
	{
		CWaitCursor pCursor;
		m_wndFrame.Update( FALSE );
		m_tLast = tNow;
	}
	else if ( tNow - m_tLast > 30000 )
	{
		m_wndFrame.Update( FALSE );
		m_tLast = tNow;
	}
}

void CLibraryWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();
	m_wndFrame.OnSkinChange();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryWnd events

HRESULT CLibraryWnd::GetGenericView(IGenericView** ppView)
{
	if ( m_wndFrame.m_hWnd == NULL ) return S_FALSE;
	CLibraryList* pList = m_wndFrame.GetViewSelection();
	*ppView = (IGenericView*)pList->GetInterface( IID_IGenericView, TRUE );
	return S_OK;
}

BOOL CLibraryWnd::OnCollection(LPCTSTR pszPath)
{
	CAlbumFolder* pFolder = NULL;
	
	if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( pszPath, TRUE, FALSE, TRUE ) )
	{
		pFolder = LibraryFolders.GetCollection( &pFile->m_pSHA1 );
		Library.Unlock();
	}
	else
	{
		CString strFormat, strMessage;
		CCollectionFile pCollection;
		
		if ( pCollection.Open( pszPath ) )
		{
			CString strSource( pszPath ), strTarget;
			
			int nName = strSource.ReverseFind( '\\' );
			if ( nName >= 0 )
			{
				strTarget = Settings.Downloads.CompletePath + strSource.Mid( nName );
				LibraryBuilder.RequestPriority( strTarget );
			}
			
			if ( strTarget.GetLength() > 0 && CopyFile( strSource, strTarget, TRUE ) )
			{
				LoadString( strFormat, IDS_LIBRARY_COLLECTION_INSTALLED );
				strMessage.Format( strFormat, (LPCTSTR)pCollection.GetTitle() );
				AfxMessageBox( strMessage, MB_ICONINFORMATION );
				
				if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( strTarget, TRUE, FALSE, TRUE ) )
				{
					pFolder = LibraryFolders.GetCollection( &pFile->m_pSHA1 );
					Library.Unlock();
				}
			}
			else
			{
				LoadString( strFormat, IDS_LIBRARY_COLLECTION_CANT_INSTALL );
				strMessage.Format( strFormat, (LPCTSTR)pCollection.GetTitle(), (LPCTSTR)Settings.Downloads.CompletePath );
				AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
			}
		}
		else
		{
			LoadString( strFormat, IDS_LIBRARY_COLLECTION_INVALID );
			strMessage.Format( strFormat, pszPath );
			AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		}
	}
	
	if ( pFolder != NULL ) Display( pFolder );
	return ( pFolder != NULL );
}
