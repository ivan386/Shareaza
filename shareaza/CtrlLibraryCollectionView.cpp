//
// CtrlLibraryCollectionView.cpp
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

#include "Network.h"
#include "Library.h"
#include "AlbumFolder.h"
#include "SharedFile.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "FileExecutor.h"
#include "ShareazaURL.h"
#include "IEProtocol.h"
#include "CollectionFile.h"
#include "SHA.h"

#include "Skin.h"
#include "CtrlWeb.h"
#include "CtrlLibraryTip.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryTree.h"
#include "CtrlLibraryCollectionView.h"

IMPLEMENT_DYNCREATE(CLibraryCollectionView, CLibraryFileView)

BEGIN_INTERFACE_MAP(CLibraryCollectionView::External, CComObject)
	INTERFACE_PART(CLibraryCollectionView::External, IID_ICollectionHtmlView, View)
END_INTERFACE_MAP()

BEGIN_MESSAGE_MAP(CLibraryCollectionView, CLibraryFileView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_NOTIFY(WVN_CONTEXTMENU, AFX_IDW_PANE_FIRST, OnWebContextMenu)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_DOWNLOAD, OnUpdateLibraryFolderDownload)
	ON_COMMAND(ID_LIBRARY_FOLDER_DOWNLOAD, OnLibraryFolderDownload)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryCollectionView construction

CLibraryCollectionView::CLibraryCollectionView()
{
	m_nCommandID = ID_LIBRARY_VIEW_COLLECTION;
	
	m_pWebCtrl		= NULL;
	m_nWebIndex		= 0;
	m_pCollection	= new CCollectionFile();
	
	m_xExternal.m_pView = this;
	m_pszToolBar = _T("CLibraryCollectionView");
}

CLibraryCollectionView::~CLibraryCollectionView()
{
	delete m_pCollection;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryCollectionView view operations

BOOL CLibraryCollectionView::CheckAvailable(CLibraryTreeItem* pSel)
{
	BOOL bAvailable = FALSE;
	
	if ( CAlbumFolder* pFolder = GetSelectedAlbum( pSel ) )
	{
		if ( pFolder->m_bCollSHA1 )
		{
			if ( LibraryMaps.LookupFileBySHA1( &pFolder->m_pCollSHA1, FALSE, FALSE, TRUE ) )
			{
				bAvailable = TRUE;
			}
			else
			{
				Library.Lock();
				pFolder->m_bCollSHA1 = FALSE;
				Library.Unlock( TRUE );
			}
		}
	}
	
	if ( bAvailable != m_bAvailable )
	{
		m_bAvailable = bAvailable;
		m_xExternal.m_bLockdown = FALSE;
	}
	
	return m_bAvailable;
}

void CLibraryCollectionView::Update()
{
	if ( CAlbumFolder* pFolder = GetSelectedAlbum() )
	{
		if ( pFolder->m_bCollSHA1 && m_pWebCtrl != NULL )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileBySHA1( &pFolder->m_pCollSHA1, FALSE, FALSE, TRUE ) )
			{
				ShowCollection( pFile );
				return;
			}
		}
	}
	
	ShowCollection( NULL );
}

BOOL CLibraryCollectionView::ShowCollection(CLibraryFile* pFile)
{
	if ( pFile != NULL )
	{
		if ( m_pCollection->IsOpen() && m_pSHA1 == pFile->m_pSHA1 ) return TRUE;
		
		if ( m_pCollection->Open( pFile->GetPath() ) )
		{
			CString strIndex, strURL;
			IEProtocol.SetCollection( &pFile->m_pSHA1, pFile->GetPath(), &strIndex );
			
			strURL.Format( _T("p2p-col://%s/%s"),
				(LPCTSTR)CSHA::HashToString( &pFile->m_pSHA1 ),
				(LPCTSTR)strIndex );
			m_pWebCtrl->Navigate( strURL );
			
			m_pSHA1 = pFile->m_pSHA1;
			return TRUE;
		}
	}
	
	if ( m_pCollection->IsOpen() )
	{
		m_pCollection->Close();
		if ( m_pWebCtrl != NULL ) m_pWebCtrl->Navigate( _T("about:blank") );
	}
	
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryCollectionView message handlers

int CLibraryCollectionView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CLibraryFileView::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	CWaitCursor pCursor;
	m_pWebCtrl = new CWebCtrl();
	
	if ( m_pWebCtrl->Create( 0, this ) != -1 )
	{
		m_pWebCtrl->EnableCoolMenu();
		m_pWebCtrl->EnableSandbox();
		m_pWebCtrl->SetExternal( m_xExternal.GetDispatch() );
		m_pWebCtrl->Navigate( _T("about:blank") );
	}
	else
	{
		delete m_pWebCtrl;
		m_pWebCtrl = NULL;
	}
	
	return 0;
}

void CLibraryCollectionView::OnDestroy()
{
	if ( m_pWebCtrl != NULL )
	{
		m_pWebCtrl->DestroyWindow();
		delete m_pWebCtrl;
		m_pWebCtrl = NULL;
	}
	
	IEProtocol.SetCollection( NULL, NULL );
	m_pCollection->Close();
	
	CLibraryFileView::OnDestroy();
}

void CLibraryCollectionView::OnSize(UINT nType, int cx, int cy)
{
	CLibraryFileView::OnSize( nType, cx, cy );
	
	if ( m_pWebCtrl != NULL && m_pWebCtrl->GetSafeHwnd() != NULL )
	{
		m_pWebCtrl->SetWindowPos( NULL, 0, 0, cx, cy, SWP_SHOWWINDOW );
	}
}

void CLibraryCollectionView::OnWebContextMenu(NMHDR* pNMHDR, LPARAM* pResult)
{
	WVNCONTEXTMENU* pNotify = (WVNCONTEXTMENU*)pNMHDR;
	
	if ( m_nWebIndex != 0 )
	{
		*pResult = TRUE;
		GetToolTip()->Hide();
		
		SelClear( FALSE );
		SelAdd( m_nWebIndex );
		
		CPoint point( pNotify->ptMouse );
		
		CString strName = _T("CLibraryFileView");
		strName += Settings.Library.ShowVirtual ? _T(".Virtual") : _T(".Physical");
		UINT nCmdID = Skin.TrackPopupMenu( strName, point, ID_LIBRARY_LAUNCH, TPM_RETURNCMD );
		
		if ( nCmdID != 0 ) GetFrame()->SendMessage( WM_COMMAND, nCmdID );
		
		SelClear( TRUE );
	}
}

void CLibraryCollectionView::OnUpdateLibraryFolderDownload(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_pCollection->GetMissingCount() > 0 );
}

void CLibraryCollectionView::OnLibraryFolderDownload()
{
	for ( POSITION pos = m_pCollection->GetFileIterator() ; pos ; )
	{
		CCollectionFile::File* pFile = m_pCollection->GetNextFile( pos );
		pFile->Download();
	}
	
	if ( ! Network.IsWellConnected() ) Network.Connect( TRUE );
	PostUpdate();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryCollectionView::External construction

CLibraryCollectionView::External::External()
{
	EnableDispatch( IID_ICollectionHtmlView );
	m_bLockdown = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryCollectionView::External ICollectionHtmlView implementation

IMPLEMENT_DISPATCH(CLibraryCollectionView::External, View)

STDMETHODIMP CLibraryCollectionView::External::XView::get_Application(IApplication **ppApplication)
{
	METHOD_PROLOGUE(CLibraryCollectionView::External, View)
	*ppApplication = NULL;
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryCollectionView::External::XView::Detect(BSTR sURN, BSTR *psState)
{
	METHOD_PROLOGUE(CLibraryCollectionView::External, View)
	
	if ( pThis->m_bLockdown )
	{
		CString( _T("Lockdown") ).SetSysString( psState );
	}
	else if ( pThis->m_pView->m_pCollection->FindByURN( CString( sURN ) ) == NULL )
	{
		CString( _T("NotInCollection") ).SetSysString( psState );
	}
	else if ( LibraryMaps.LookupFileByURN( CString( sURN ), FALSE, FALSE, TRUE ) )
	{
		CString( _T("Complete") ).SetSysString( psState );
	}
	else
	{
		CSingleLock pLock( &Transfers.m_pSection, TRUE );
		
		if ( CDownload* pDownload = Downloads.FindByURN( CString( sURN ) ) )
		{
			CString str;
			str.Format( _T("%.2f%%"), pDownload->GetProgress() * 100 );
			str.SetSysString( psState );
		}
		else
		{
			CString().SetSysString( psState );
		}
	}
	
	return S_OK;
}

STDMETHODIMP CLibraryCollectionView::External::XView::Hover(BSTR sURN)
{
	METHOD_PROLOGUE(CLibraryCollectionView::External, View)
	if ( pThis->m_bLockdown ) return S_OK;
	
	CLibraryCollectionView* pView = pThis->m_pView;
	if ( pView->m_pWebCtrl == NULL ) return S_OK;
	if ( pView->GetFrame() == NULL ) return S_OK;
	
	pView->m_nWebIndex = 0;
	
	if ( sURN != NULL && wcslen( sURN ) != 0 )
	{
		if ( pThis->m_pView->m_pCollection->FindByURN( CString( sURN ) ) != NULL )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByURN( CString( sURN ), TRUE, FALSE, TRUE ) )
			{
				pView->m_nWebIndex = pFile->m_nIndex;
				Library.Unlock();
			}
		}
	}
	
	if ( pView->m_nWebIndex != 0 )
	{
		HWND hWnd = pView->m_pWebCtrl->GetSafeHwnd();
		pView->GetToolTip()->Show( (LPVOID)pView->m_nWebIndex, hWnd );
	}
	else
	{
		pView->GetToolTip()->Hide();
	}
	
	return S_OK;
}

STDMETHODIMP CLibraryCollectionView::External::XView::Open(BSTR sURN, VARIANT_BOOL *pbResult)
{
	METHOD_PROLOGUE(CLibraryCollectionView::External, View)
	*pbResult = VARIANT_FALSE;
	if ( pThis->m_bLockdown ) return S_OK;
	
	if ( pThis->m_pView->m_pCollection->FindByURN( CString( sURN ) ) != NULL )
	{
		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByURN( CString( sURN ), TRUE, FALSE, TRUE ) )
		{
			*pbResult = pFile->Execute() ? VARIANT_TRUE : VARIANT_FALSE;
			Library.Unlock();
		}
	}
	
	return S_OK;
}

STDMETHODIMP CLibraryCollectionView::External::XView::Enqueue(BSTR sURN, VARIANT_BOOL *pbResult)
{
	METHOD_PROLOGUE(CLibraryCollectionView::External, View)
	*pbResult = VARIANT_FALSE;
	if ( pThis->m_bLockdown ) return S_OK;
	
	if ( pThis->m_pView->m_pCollection->FindByURN( CString( sURN ) ) != NULL )
	{
		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByURN( CString( sURN ), TRUE, FALSE, TRUE ) )
		{
			CString strPath = pFile->GetPath();
			Library.Unlock();
			*pbResult = CFileExecutor::Enqueue( strPath ) ? VARIANT_TRUE : VARIANT_FALSE;
		}
	}
	
	return S_OK;
}

STDMETHODIMP CLibraryCollectionView::External::XView::Download(BSTR sURN, VARIANT_BOOL *pbResult)
{
	METHOD_PROLOGUE(CLibraryCollectionView::External, View)
	*pbResult = VARIANT_FALSE;
	if ( pThis->m_bLockdown ) return S_OK;
	
	if ( CCollectionFile::File* pFile = pThis->m_pView->m_pCollection->FindByURN( CString( sURN ) ) )
	{
		if ( ! pFile->IsComplete() )
		{
			CString strFormat, strMessage;
			LoadString( strFormat, IDS_LIBRARY_COLLECTION_DOWNLOAD_FILE );
			strMessage.Format( strFormat, (LPCTSTR)pFile->m_sName );
			
			UINT nResponse = AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNOCANCEL );
			
			if ( nResponse == IDYES )
			{
				*pbResult = pFile->Download() ? VARIANT_TRUE : VARIANT_FALSE;

				if ( *pbResult == VARIANT_TRUE )
				{
					if ( ! Network.IsWellConnected() ) Network.Connect( TRUE );
				}
			}
			else if ( nResponse == IDCANCEL )
			{
				pThis->CheckLockdown();
			}
		}
	}
	
	return S_OK;
}

STDMETHODIMP CLibraryCollectionView::External::XView::DownloadAll()
{
	METHOD_PROLOGUE(CLibraryCollectionView::External, View)
	if ( pThis->m_bLockdown ) return S_OK;
	
	CString strMessage;
	LoadString( strMessage, IDS_LIBRARY_COLLECTION_DOWNLOAD_ALL );
	UINT nResponse = AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNOCANCEL );
	
	if ( nResponse == IDYES )
		pThis->m_pView->PostMessage( WM_COMMAND, ID_LIBRARY_FOLDER_DOWNLOAD );
	else if ( nResponse == IDCANCEL )
		pThis->CheckLockdown();
	
	return S_OK;
}

STDMETHODIMP CLibraryCollectionView::External::XView::get_MissingCount(LONG *pnCount)
{
	METHOD_PROLOGUE(CLibraryCollectionView::External, View)
	if ( pThis->m_bLockdown ) return S_OK;
	
	*pnCount = pThis->m_pView->m_pCollection->GetMissingCount();
	
	return S_OK;
}

void CLibraryCollectionView::External::CheckLockdown()
{
	CString strMessage;
	LoadString( strMessage, IDS_LIBRARY_COLLECTION_LOCKDOWN );
	if ( AfxMessageBox( strMessage, MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION ) == IDYES ) m_bLockdown = TRUE;
}
