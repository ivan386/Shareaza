//
// CtrlLibraryCollectionView.cpp
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
#include "AlbumFolder.h"
#include "Application.h"
#include "CollectionFile.h"
#include "Download.h"
#include "Downloads.h"
#include "FileExecutor.h"
#include "IEProtocol.h"
#include "Library.h"
#include "Network.h"
#include "Settings.h"
#include "ShareazaURL.h"
#include "SharedFile.h"
#include "Skin.h"
#include "Transfers.h"

#include "CtrlLibraryCollectionView.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryTip.h"
#include "CtrlWeb.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CLibraryCollectionView, CLibraryFileView)

IMPLEMENT_DYNAMIC(CHtmlCollection, CComObject)

BEGIN_INTERFACE_MAP(CHtmlCollection, CComObject)
	INTERFACE_PART(CHtmlCollection, IID_ICollectionHtmlView, View)
END_INTERFACE_MAP()

BEGIN_MESSAGE_MAP(CLibraryCollectionView, CLibraryFileView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_NOTIFY(WVN_CONTEXTMENU, AFX_IDW_PANE_FIRST, &CLibraryCollectionView::OnWebContextMenu)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_DOWNLOAD, &CLibraryCollectionView::OnUpdateLibraryFolderDownload)
	ON_COMMAND(ID_LIBRARY_FOLDER_DOWNLOAD, &CLibraryCollectionView::OnLibraryFolderDownload)
	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryCollectionView construction

CLibraryCollectionView::CLibraryCollectionView()
	: m_pWebCtrl	( NULL )
	, m_nWebIndex	( 0 )
	, m_pCollection	( new CCollectionFile() )
	, m_bTrusted	( TRI_UNKNOWN )
	, m_bLockdown	( FALSE )
{
	m_nCommandID = ID_LIBRARY_VIEW_COLLECTION;
	m_xExternal.m_pView = this;
	m_pszToolBar = _T("CLibraryCollectionView");
}

CLibraryCollectionView::~CLibraryCollectionView()
{
	delete m_pCollection;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryCollectionView create and destroy

BOOL CLibraryCollectionView::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	SelClear( FALSE );
	// Do not add WS_VSCROLL here. The IE frame that gets loaded will have
	// its own scrollbar and will handle its own scrolling.
	return CWnd::CreateEx( 0, NULL, _T("CLibraryCollectionView"), WS_CHILD |
		WS_TABSTOP | WS_GROUP, rect, pParentWnd, IDC_LIBRARY_VIEW );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryCollectionView view operations

BOOL CLibraryCollectionView::CheckAvailable(CLibraryTreeItem* pSel)
{
	CQuickLock oLock( Library.m_pSection );

	BOOL bAvailable = FALSE;

	if ( CAlbumFolder* pFolder = GetSelectedAlbum( pSel ) )
	{
		if ( pFolder->m_oCollSHA1 )
		{
			if ( LibraryMaps.LookupFileBySHA1( pFolder->m_oCollSHA1, FALSE, TRUE ) )
			{
				bAvailable = TRUE;
			}
			else
			{
				pFolder->m_oCollSHA1.clear();
				Library.Update();
			}
		}
	}

	if ( bAvailable != m_bAvailable )
	{
		m_bAvailable = bAvailable;
		m_bLockdown = FALSE;
	}

	return m_bAvailable;
}

void CLibraryCollectionView::Update()
{
	if ( CAlbumFolder* pFolder = GetSelectedAlbum() )
	{
		if ( pFolder->m_oCollSHA1 && m_pWebCtrl != NULL )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileBySHA1( pFolder->m_oCollSHA1, FALSE, TRUE ) )
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
		if ( m_pCollection->IsOpen() && validAndEqual( m_oSHA1, pFile->m_oSHA1 ) )
			// Already opened
			return TRUE;

		m_bTrusted = TRI_UNKNOWN;

		if ( m_pCollection->Open( pFile->GetPath() ) )
		{
			if ( m_pWebCtrl && SUCCEEDED( m_pWebCtrl->Navigate( _T("p2p-col://") + pFile->m_oSHA1.toString() + _T("/") ) ) )
			{
				m_oSHA1 = pFile->m_oSHA1;
				return TRUE;
			}
		}
	}

	m_bTrusted = TRI_UNKNOWN;

	if ( m_pCollection->IsOpen() )
	{
		m_pCollection->Close();
		if ( m_pWebCtrl ) m_pWebCtrl->Navigate( _T("about:blank") );
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
		// Disable cool menu because in RTL mode the text is drawn mirrored
		// It worked before, but somehow was broken and nothing helps.
		// TODO: fix it
		if ( !Settings.General.LanguageRTL )
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

	m_pCollection->Close();

	m_bTrusted = TRI_UNKNOWN;

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

	*pResult = TRUE;

	if ( m_nWebIndex != 0 )
	{
		GetToolTip()->Hide();

		SelClear( FALSE );
		SelAdd( m_nWebIndex );

		CPoint point( pNotify->ptMouse );

		CStringList oFiles;
		{
			CQuickLock pLock( Library.m_pSection );
			POSITION posSel = StartSelectedFileLoop();
			while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
			{
				oFiles.AddTail( pFile->GetPath() );
			}
		}

		CString strName = _T("CLibraryFileView");
		strName += Settings.Library.ShowVirtual ? _T(".Virtual") : _T(".Physical");
		Skin.TrackPopupMenu( strName, point, ID_LIBRARY_LAUNCH, oFiles );
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

UINT CLibraryCollectionView::OnGetDlgCode()
{
	return DLGC_WANTARROWS;
}

/////////////////////////////////////////////////////////////////////////////
// CCollectionHtmlView construction

CHtmlCollection::CHtmlCollection()
	: m_pView	( NULL )
{
	EnableDispatch( IID_ICollectionHtmlView );
}

CHtmlCollection::~CHtmlCollection()
{
}

IMPLEMENT_DISPATCH(CHtmlCollection, View)

STDMETHODIMP CHtmlCollection::XView::get_Application(IApplication **ppApplication)
{
	METHOD_PROLOGUE(CHtmlCollection, View)
	CLibraryCollectionView* pView = pThis->m_pView;

	if ( pView->m_bTrusted == TRI_UNKNOWN )
	{
		pView->m_bTrusted = ( AfxMessageBox( _T("The collection has requested access to the application. Allow access?"), MB_ICONEXCLAMATION | MB_YESNO ) == IDYES ) ? TRI_TRUE : TRI_FALSE;
	}

	return ( pView->m_bTrusted == TRI_TRUE ) ? CApplication::GetApp( ppApplication ) : E_ACCESSDENIED;
}

STDMETHODIMP CHtmlCollection::XView::Detect(BSTR sURN, BSTR *psState)
{
	METHOD_PROLOGUE(CHtmlCollection, View)
	CQuickLock oLock( Library.m_pSection );
	CLibraryCollectionView* pView = pThis->m_pView;

	CString str;
	if ( sURN && *sURN )
	{
		CString strURN( sURN );

		if ( pView->m_bLockdown )
		{
			str = _T("Lockdown");
		}
		else if ( pView->m_pCollection->FindByURN( strURN ) == NULL )
		{
			str = _T("NotInCollection");
		}
		else
		{
			CSingleLock pLock( &Transfers.m_pSection );
			if ( pLock.Lock( 250 ) )
			{
				if ( CDownload* pDownload = Downloads.FindByURN( strURN ) )
				{
					str.Format( _T("%.2f%%"), pDownload->GetProgress() );
				}
			}
			pLock.Unlock();

			if ( str.IsEmpty() )
			{
				if ( CLibraryFile* pFile = LibraryMaps.LookupFileByURN( strURN ) )
				{
					str = pFile->IsAvailable() ? _T("Complete") : _T("Ghost");
				}
			}
		}
	}
	*psState = CComBSTR( str ).Detach();
	return S_OK;
}

STDMETHODIMP CHtmlCollection::XView::Hover(BSTR sURN)
{
	METHOD_PROLOGUE(CHtmlCollection, View)
	CQuickLock oLock( Library.m_pSection );
	CLibraryCollectionView* pView = pThis->m_pView;

	if ( pView->m_bLockdown )
		return S_OK;

	if ( pView->m_pWebCtrl == NULL )
		return S_OK;

	if ( pView->GetFrame() == NULL )
		return S_OK;

	pView->m_nWebIndex = 0;

	if ( sURN && *sURN )
	{
		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByURN( CString( sURN ) ) )
		{
			pView->m_nWebIndex = pFile->m_nIndex;
		}
	}

	if ( pView->m_nWebIndex != 0 )
		pView->GetToolTip()->Show( pView->m_nWebIndex, pView->m_pWebCtrl->GetSafeHwnd() );
	else
		pView->GetToolTip()->Hide();

	return S_OK;
}

STDMETHODIMP CHtmlCollection::XView::Open(BSTR sURN, VARIANT_BOOL *pbResult)
{
	METHOD_PROLOGUE(CHtmlCollection, View)
	*pbResult = VARIANT_FALSE;
	CSingleLock oLock( &Library.m_pSection, TRUE );
	CLibraryCollectionView* pView = pThis->m_pView;

	if ( pView->m_bLockdown )
		return S_OK;

	if ( sURN && *sURN )
	{
		if ( pView->m_pCollection->FindByURN( CString( sURN ) ) != NULL )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByURN( CString( sURN ), FALSE, TRUE ) )
			{
				if ( pFile->IsAvailable() )
				{
					CString strPath = pFile->GetPath();
					oLock.Unlock();
					*pbResult = CFileExecutor::Execute( strPath ) ? VARIANT_TRUE : VARIANT_FALSE;
				}
			}
		}
	}
	return S_OK;
}

STDMETHODIMP CHtmlCollection::XView::Enqueue(BSTR sURN, VARIANT_BOOL *pbResult)
{
	METHOD_PROLOGUE(CHtmlCollection, View)
	*pbResult = VARIANT_FALSE;
	CSingleLock oLock( &Library.m_pSection, TRUE );
	CLibraryCollectionView* pView = pThis->m_pView;

	if ( pView->m_bLockdown )
		return S_OK;

	if ( sURN && *sURN )
	{
		if ( pView->m_pCollection->FindByURN( CString( sURN ) ) != NULL )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByURN( CString( sURN ), FALSE, TRUE ) )
			{
				CString strPath = pFile->GetPath();
				oLock.Unlock();
				*pbResult = CFileExecutor::Enqueue( strPath ) ? VARIANT_TRUE : VARIANT_FALSE;
			}
		}
	}
	return S_OK;
}

STDMETHODIMP CHtmlCollection::XView::Download(BSTR sURN, VARIANT_BOOL *pbResult)
{
	METHOD_PROLOGUE(CHtmlCollection, View)
	*pbResult = VARIANT_FALSE;
	CQuickLock oLock( Library.m_pSection );
	CLibraryCollectionView* pView = pThis->m_pView;

	if ( pView->m_bLockdown )
		return S_OK;

	if ( sURN && *sURN )
	{
		if ( CCollectionFile::File* pFile = pView->m_pCollection->FindByURN( CString( sURN ) ) )
		{
			*pbResult = pFile->Download() ? VARIANT_TRUE : VARIANT_FALSE;
		}
	}
	return S_OK;
}

STDMETHODIMP CHtmlCollection::XView::DownloadAll()
{
	METHOD_PROLOGUE(CHtmlCollection, View)
	CLibraryCollectionView* pView = pThis->m_pView;

	if ( pView->m_bLockdown )
		return S_OK;

	INT_PTR nResponse = AfxMessageBox( IDS_LIBRARY_COLLECTION_DOWNLOAD_ALL, MB_ICONQUESTION|MB_YESNOCANCEL );
	if ( nResponse == IDYES )
		pView->PostMessage( WM_COMMAND, ID_LIBRARY_FOLDER_DOWNLOAD );
	else if ( nResponse == IDCANCEL )
	{
		if ( AfxMessageBox( IDS_LIBRARY_COLLECTION_LOCKDOWN, MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION ) == IDYES )
			pView->m_bLockdown = TRUE;
	}
	return S_OK;
}

STDMETHODIMP CHtmlCollection::XView::get_MissingCount(LONG *pnCount)
{
	METHOD_PROLOGUE(CHtmlCollection, View)
	*pnCount = 0;
	CQuickLock oLock( Library.m_pSection );
	CLibraryCollectionView* pView = pThis->m_pView;

	if ( pView->m_bLockdown )
		return S_OK;

	*pnCount = pView->m_pCollection->GetMissingCount();
	return S_OK;
}
