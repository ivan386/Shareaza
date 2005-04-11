//
// CtrlLibraryDetailView.cpp
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
#include "Library.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "AlbumFolder.h"
#include "LiveList.h"
#include "XML.h"
#include "SHA.h"
#include "Schema.h"
#include "SchemaCache.h"

#include "CoolInterface.h"
#include "ShellIcons.h"
#include "CoolMenu.h"
#include "Skin.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryDetailView.h"
#include "DlgHitColumns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLibraryDetailView, CLibraryFileView)
IMPLEMENT_DYNCREATE(CLibraryListView, CLibraryDetailView)
IMPLEMENT_DYNCREATE(CLibraryIconView, CLibraryDetailView)

BEGIN_MESSAGE_MAP(CLibraryDetailView, CLibraryFileView)
	//{{AFX_MSG_MAP(CLibraryDetailView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_RENAME, OnUpdateLibraryRename)
	ON_COMMAND(ID_LIBRARY_RENAME, OnLibraryRename)
	ON_COMMAND(ID_LIBRARY_COLUMNS, OnLibraryColumns)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_COLUMNS, OnUpdateLibraryColumns)
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(LVN_ODCACHEHINT, OnCacheHint)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFOW, OnGetDispInfoW)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFOA, OnGetDispInfoA)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDITW, OnBeginLabelEdit)
	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDITA, OnBeginLabelEdit)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDITW, OnEndLabelEditW)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDITA, OnEndLabelEditA)
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBeginDrag)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
	ON_NOTIFY_REFLECT(LVN_ODSTATECHANGED, OnItemRangeChanged)
	ON_NOTIFY_REFLECT(LVN_ODFINDITEMW, OnFindItemW)
	ON_NOTIFY_REFLECT(LVN_ODFINDITEMA, OnFindItemA)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblClk)
	ON_UPDATE_COMMAND_UI_RANGE(1000, 1100, OnUpdateBlocker)
END_MESSAGE_MAP()

#define GET_LIST()		CListCtrl* pList = (CListCtrl*)this
#define DETAIL_COLUMNS	7


/////////////////////////////////////////////////////////////////////////////
// CLibraryDetailView construction

CLibraryDetailView::CLibraryDetailView(UINT nCommandID)
{
	switch ( m_nCommandID = nCommandID )
	{
	case ID_LIBRARY_VIEW_DETAIL:
		m_nStyle = LVS_REPORT;
		break;
	case ID_LIBRARY_VIEW_LIST:
		m_nStyle = LVS_LIST;
		break;
	case ID_LIBRARY_VIEW_ICON:
		m_nStyle = LVS_ICON;
		break;
	}
	
	m_pCoolMenu	= NULL;
	m_pSchema	= NULL;
}

CLibraryDetailView::~CLibraryDetailView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryDetailView create and destroy

BOOL CLibraryDetailView::Create(CWnd* pParentWnd) 
{
	CRect rect( 0, 0, 0, 0 );
	SelClear( FALSE );
	DWORD nStyle = m_nStyle;
	return CWnd::Create( WC_LISTVIEW, _T("CLibraryDetailView"),
		WS_CHILD|LVS_ICON|LVS_AUTOARRANGE|LVS_SHOWSELALWAYS|nStyle|
		LVS_SHAREIMAGELISTS|LVS_EDITLABELS|LVS_OWNERDATA, rect, pParentWnd, IDC_LIBRARY_VIEW );
}

int CLibraryDetailView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CLibraryFileView::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP,
		LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP );
	
	GET_LIST();
	
	pList->InsertColumn( 0, _T("File"), LVCFMT_LEFT, 200, -1 );
	pList->SetCallbackMask( LVIS_SELECTED );
	
	if ( m_nStyle == LVS_REPORT )
	{
		pList->InsertColumn( 1, _T("Extension"), LVCFMT_CENTER, 40, 0 );
		pList->InsertColumn( 2, _T("Size"), LVCFMT_CENTER, 60, 1 );
		pList->InsertColumn( 3, _T("Folder"), LVCFMT_LEFT, 0, 2 );
		pList->InsertColumn( 4, _T("Hits"), LVCFMT_CENTER, 70, 3 );
		pList->InsertColumn( 5, _T("Uploads"), LVCFMT_CENTER, 70, 4 );
		pList->InsertColumn( 6, _T("Modified"), LVCFMT_CENTER, 0, 5 );
		pList->SetImageList( ShellIcons.GetObject( 16 ), LVSIL_SMALL );

		CHeaderCtrl* pHeader = (CHeaderCtrl*)GetWindow( GW_CHILD );
		if ( pHeader ) Skin.Translate( _T("CLibraryWnd"), pHeader );
	}
	else if ( m_nStyle != LVS_ICON )
	{
		pList->SetImageList( ShellIcons.GetObject( 16 ), LVSIL_SMALL );
	}
	else
	{
		pList->SetImageList( ShellIcons.GetObject( 32 ), LVSIL_NORMAL );
	}
	
	pList->SetCallbackMask( LVIS_SELECTED );
	
	m_pList	= NULL;
	m_nList	= m_nBuffer = 0;
	
	CString strSchemaURI = Settings.Library.FilterURI.GetLength() ?
		Settings.Library.FilterURI : Settings.Library.SchemaURI;
	
	if ( CSchema* pSchema = SchemaCache.Get( strSchemaURI ) )
	{
		CPtrList pColumns;
		CSchemaColumnsDlg::LoadColumns( pSchema, &pColumns );
		SetViewSchema( pSchema, &pColumns, FALSE, FALSE );
	}
	else
	{
		SetViewSchema( NULL, NULL, FALSE, FALSE );
	}
	
	m_bCreateDragImage	= FALSE;
	
	return 0;
}

void CLibraryDetailView::OnDestroy() 
{
	if ( m_pList )
	{
		for ( DWORD nItem = 0 ; nItem < m_nBuffer ; nItem++ )
		{
			if ( m_pList[ nItem ].pText ) delete m_pList[ nItem ].pText;
		}

		delete [] m_pList;
	}
	
	m_pList = NULL;
	m_nList = m_nBuffer = 0;
	
	if ( m_nStyle == LVS_REPORT )
	{
		if ( m_pSchema != NULL )
		{
			Settings.SaveList( _T("CLibraryDetailView.") + m_pSchema->m_sSingular, (CListCtrl*)this );
		}
		else
		{
			Settings.SaveList( _T("CLibraryDetailView"), (CListCtrl*)this );
		}
	}

	CLibraryFileView::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryDetailView schema setup

void CLibraryDetailView::SetViewSchema(CSchema* pSchema, CPtrList* pColumns, BOOL bSave, BOOL bUpdate)
{
	GET_LIST();
	
	pList->DeleteAllItems();
	
	if ( m_nStyle != LVS_REPORT )
	{
		if ( bUpdate ) PostUpdate();
		return;
	}

	if ( bSave )
	{
		if ( m_pSchema )
		{
			Settings.SaveList( _T("CLibraryDetailView.") + m_pSchema->m_sSingular, pList );
		}
		else
		{
			Settings.SaveList( _T("CLibraryDetailView"), pList );
		}
	}
	
	m_pColumns.RemoveAll();
	if ( m_pSchema = pSchema ) m_pColumns.AddTail( pColumns );
	
	int nColumn = DETAIL_COLUMNS;
	while ( pList->DeleteColumn( nColumn ) );
	
	for ( POSITION pos = m_pColumns.GetHeadPosition() ; pos ; nColumn++ )
	{
		CSchemaMember* pMember = (CSchemaMember*)m_pColumns.GetNext( pos );
		pList->InsertColumn( nColumn, pMember->m_sTitle, pMember->m_nColumnAlign, pMember->m_nColumnWidth, nColumn - 1 );
	}

	if ( m_pSchema )
		Settings.LoadList( _T("CLibraryDetailView.") + m_pSchema->m_sSingular, pList, 1 );
	else
		Settings.LoadList( _T("CLibraryDetailView"), pList, 1 );

	if ( bUpdate ) PostUpdate();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryDetailView update

void CLibraryDetailView::Update()
{
	GET_LIST();
	
	CSchema* pSchema	= SchemaCache.Get( Settings.Library.FilterURI );
	DWORD nCookie		= GetFolderCookie();
	
	if ( Settings.Library.ShowVirtual ) pSchema = NULL;
	
	if ( m_nStyle == LVS_REPORT )
	{
		CLibraryTreeItem* pTree = GetFolderSelection();
		
		if ( pTree != NULL && pTree->m_pVirtual != NULL && pTree->m_pSelNext == NULL &&
			 pTree->m_pVirtual->m_pSchema != NULL )
		{
			CString strURI = pTree->m_pVirtual->m_pSchema->GetContainedURI( CSchema::stFile );
			
			if ( strURI.GetLength() && ( m_pSchema == NULL || m_pSchema->m_sURI != strURI ) )
			{
				if ( CSchema* pSchema = SchemaCache.Get( strURI ) )
				{
					CPtrList pColumns;
					CSchemaColumnsDlg::LoadColumns( pSchema, &pColumns );
					SetViewSchema( pSchema, &pColumns, TRUE, FALSE );
				}
			}
		}
	}

	LDVITEM* pItem = m_pList + m_nList - 1;

	for ( DWORD nCount = m_nList ; nCount ; nCount--, pItem-- )
	{
		CLibraryFile* pFile = Library.LookupFile( pItem->nIndex );
		
		if ( pFile && pFile->m_nSelectCookie == nCookie && pFile->IsAvailable() &&
			 ( ! pSchema || pSchema->Equals( pFile->m_pSchema ) ||
			 ( ! pFile->m_pMetadata && pSchema->FilterType( pFile->m_sName ) ) ) )
		{
			pFile->m_nListCookie = nCookie;
		}
		else
		{
			SelRemove( pItem->nIndex );
			CStringArray* pSaved = pItem->pText;
			CopyMemory( pItem, pItem + 1, sizeof(LDVITEM) * ( m_nList - nCount ) );
			pItem[ m_nList - nCount ].pText = pSaved;
			m_nList--;
		}
	}

	for ( POSITION pos = LibraryMaps.GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = LibraryMaps.GetNextFile( pos );

		if ( pFile->m_nSelectCookie == nCookie &&
 			 pFile->m_nListCookie != nCookie &&
			 pFile->IsAvailable() &&
			 ( ! pSchema || pSchema->Equals( pFile->m_pSchema ) ||
			 ( ! pFile->m_pMetadata && pSchema->FilterType( pFile->m_sName ) ) ) )
		{
			if ( m_nList == m_nBuffer )	// MUST EQUAL
			{
				m_nBuffer += 64;
				LDVITEM* pList = new LDVITEM[ m_nBuffer ];
				if ( m_nList ) CopyMemory( pList, m_pList, m_nList * sizeof(LDVITEM) );
				if ( m_pList ) delete [] m_pList;
				ZeroMemory( pList + m_nList, ( m_nBuffer - m_nList ) * sizeof(LDVITEM) );
				m_pList = pList;
			}
			
			m_pList[ m_nList ].nIndex		= pFile->m_nIndex;
			m_pList[ m_nList ].nState		&= ~LDVI_SELECTED;
			m_nList++;
			
			pFile->m_nListCookie = nCookie;
		}
	}
	
	m_nListCookie++;
	
	SortItems();
}

BOOL CLibraryDetailView::Select(DWORD nObject)
{
	GET_LIST();
	
	for ( int nDeselect = -1 ; ; )
	{
		nDeselect = pList->GetNextItem( nDeselect, LVNI_SELECTED );
		if ( nDeselect < 0 ) break;
		pList->SetItemState( nDeselect, 0, LVIS_SELECTED );
	}
	
	LDVITEM* pItem = m_pList;
	
	for ( DWORD nCount = 0 ; nCount < m_nList ; nCount++, pItem++ )
	{
		if ( pItem->nIndex == nObject )
		{
			LV_ITEM pItem;
			
			pItem.mask = LVIF_STATE; pItem.state = 0xFFFFFFFF; pItem.stateMask = LVIS_SELECTED|LVIS_FOCUSED;
			SendMessage( LVM_SETITEMSTATE, nCount, (LPARAM)&pItem );
			PostMessage( LVM_ENSUREVISIBLE, nCount, FALSE );
			
			return TRUE;
		}
	}
	
	return FALSE;
}

void CLibraryDetailView::CacheItem(int nItem)
{
	CLibraryFile* pFile = Library.LookupFile( m_pList[ nItem ].nIndex );
	if ( ! pFile ) return;
	
	LDVITEM* pItem = &m_pList[ nItem ];
	pItem->nCookie = m_nListCookie;
	
	if ( pItem->pText == NULL ) pItem->pText = new CStringArray();
	
	CStringArray* pText = pItem->pText;
	pText->SetSize( m_nStyle == LVS_REPORT ? DETAIL_COLUMNS + m_pColumns.GetCount() : 1 );
	
	CString strName( pFile->m_sName );
	int nDot = strName.ReverseFind( '.' );
	if ( nDot >= 0 ) strName.SetAt( nDot, 0 );
	pText->SetAt( 0, strName );
	
	if ( m_nStyle == LVS_ICON )
	{
		pItem->nIcon = ShellIcons.Get( pFile->m_sName, 32 );
	}
	else
	{
		if ( pFile->m_nIcon16 >= 0 )
			pItem->nIcon = pFile->m_nIcon16;
		else
			pItem->nIcon = pFile->m_nIcon16 = ShellIcons.Get( pFile->m_sName, 16 );
	}
	
	pItem->nState &= LDVI_SELECTED;
	if ( ! pFile->IsShared() ) pItem->nState |= LDVI_PRIVATE;
	if ( ! pFile->m_bSHA1 ) pItem->nState |= LDVI_UNSCANNED;
	if ( pFile->m_bVerify == TS_FALSE ) pItem->nState |= LDVI_UNSAFE;
	
	if ( m_nStyle != LVS_REPORT ) return;
	
	if ( LPCTSTR pszType = _tcsrchr( pFile->m_sName, '.' ) )
		pText->SetAt( 1, pszType + 1 );
	else
		pText->SetAt( 1, _T("") );
	
	pText->SetAt( 2, Settings.SmartVolume( pFile->GetSize(), FALSE ) );
	if ( pFile->m_pFolder != NULL ) pText->SetAt( 3, pFile->m_pFolder->m_sPath );
	
	CString str;
	str.Format( _T("%lu (%lu)"), pFile->m_nHitsToday, pFile->m_nHitsTotal );
	pText->SetAt( 4, str );
	str.Format( _T("%lu (%lu)"), pFile->m_nUploadsToday, pFile->m_nUploadsTotal );
	pText->SetAt( 5, str );
	
	TCHAR szModified[ 64 ];
	SYSTEMTIME pTime;
	
	FileTimeToSystemTime( &pFile->m_pTime, &pTime );
	SystemTimeToTzSpecificLocalTime( NULL, &pTime, &pTime );
	
	GetDateFormat( LOCALE_USER_DEFAULT, 0, &pTime, _T("yyyy-MM-dd"), szModified, 64 );
	_tcscat( szModified, _T(" ") );
	GetTimeFormat( LOCALE_USER_DEFAULT, 0, &pTime, _T("hh:mm tt"), szModified + _tcslen( szModified ), 64 - _tcslen( szModified ) );
	
	pText->SetAt( 6, szModified );
	
	if ( m_pSchema == NULL ) return;
	
	int nColumn = DETAIL_COLUMNS;
	
	BOOL bSource =	pFile->m_pMetadata && m_pSchema->Equals( pFile->m_pSchema ) &&
					m_pSchema->m_sSingular.CompareNoCase( pFile->m_pMetadata->GetName() ) == 0;
	
	for ( POSITION pos = m_pColumns.GetHeadPosition() ; pos ; nColumn++ )
	{
		CSchemaMember* pMember = (CSchemaMember*)m_pColumns.GetNext( pos );
		
		if ( pMember->m_sName.CompareNoCase( _T("SHA1") ) == 0 )
		{
			if ( pFile->m_bSHA1 )
			{
				pText->SetAt( nColumn, CSHA::HashToString( &pFile->m_pSHA1 ) );
			}
			else pText->SetAt( nColumn, _T("") );
		}
		else if ( bSource )
		{
			pText->SetAt( nColumn, pMember->GetValueFrom( pFile->m_pMetadata, NULL, TRUE ) );
		}
		else
		{
			pText->SetAt( nColumn, _T("") );
		}
	}
}

void CLibraryDetailView::OnCacheHint(NMLVCACHEHINT* pNotify, LRESULT* pResult)
{
	CSingleLock oLock( &Library.m_pSection );
	if ( !oLock.Lock( 100 ) ) return;
	
	for ( int nItem = pNotify->iFrom ; nItem <= pNotify->iTo ; nItem++ )
	{
		CacheItem( nItem );
	}
}

void CLibraryDetailView::OnGetDispInfoW(NMLVDISPINFO* pNotify, LRESULT* pResult)
{
	*pResult = 0;
	
	LDVITEM* pItem = &m_pList[ pNotify->item.iItem ];
	
	if ( pNotify->item.mask & LVIF_STATE )
	{
		pNotify->item.state &= ~LVIS_SELECTED;
		if ( pItem->nState & LDVI_SELECTED ) pNotify->item.state |= LVIS_SELECTED;
	}
	
	if ( pItem->nCookie != m_nListCookie )
	{
		{
			CSingleLock oLock( &Library.m_pSection );
			if ( !oLock.Lock( 100 ) ) return;
			CacheItem( pNotify->item.iItem );
		}
		if ( pItem->nCookie != m_nListCookie ) return;
	}
	
	if ( pNotify->item.mask & LVIF_TEXT )
	{
		if ( pNotify->item.iSubItem < pItem->pText->GetSize() )
		{
			wcsncpy( (LPWSTR)pNotify->item.pszText, pItem->pText->GetAt( pNotify->item.iSubItem ), pNotify->item.cchTextMax );
		}
	}
	
	if ( pNotify->item.mask & LVIF_IMAGE )
	{
		pNotify->item.iImage = pItem->nIcon;
	}
}

void CLibraryDetailView::OnGetDispInfoA(NMLVDISPINFO* pNotify, LRESULT* pResult)
{
	*pResult = 0;
	
	LDVITEM* pItem = &m_pList[ pNotify->item.iItem ];
	
	if ( pNotify->item.mask & LVIF_STATE )
	{
		pNotify->item.state &= ~LVIS_SELECTED;
		if ( pItem->nState & LDVI_SELECTED ) pNotify->item.state |= LVIS_SELECTED;
	}
	
	if ( pItem->nCookie != m_nListCookie )
	{
		{
			CSingleLock oLock( &Library.m_pSection );
			if ( !oLock.Lock( 100 ) ) return;
			CacheItem( pNotify->item.iItem );
		}
		if ( pItem->nCookie != m_nListCookie ) return;
	}
	
	if ( pNotify->item.mask & LVIF_TEXT )
	{
		if ( pNotify->item.iSubItem < pItem->pText->GetSize() )
		{
			WideCharToMultiByte( CP_ACP, 0, pItem->pText->GetAt( pNotify->item.iSubItem ), -1, (LPSTR)pNotify->item.pszText, pNotify->item.cchTextMax, NULL, NULL );
		}
	}
	
	if ( pNotify->item.mask & LVIF_IMAGE )
	{
		pNotify->item.iImage = pItem->nIcon;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryDetailView sorting

CLibraryDetailView* CLibraryDetailView::m_pThis;

void CLibraryDetailView::SortItems(int nColumn)
{
	GET_LIST();
	
	CLiveList::Sort( pList, nColumn );
	nColumn = ( m_nStyle == LVS_REPORT ) ? GetWindowLong( GetSafeHwnd(), GWL_USERDATA ) : -1;

	if ( nColumn != 0 ) 
	{
		m_nSortColumn	= abs( nColumn ) - 1;
		m_bSortFlip		= nColumn < 0;
		m_pThis			= this;
		qsort( m_pList, m_nList, sizeof(m_pList[0]), ListCompare );
	}

	pList->SetItemCount( m_nList );
}

int CLibraryDetailView::ListCompare(LPCVOID pA, LPCVOID pB)
{
	LDVITEM* ppA	= (LDVITEM*)pA;
	LDVITEM* ppB	= (LDVITEM*)pB;
	int nTest		= 0;

	CLibraryFile* pfA = Library.LookupFile( ppA->nIndex );
	CLibraryFile* pfB = Library.LookupFile( ppB->nIndex );

	if ( ! pfA || ! pfB ) return 0;

	switch ( m_pThis->m_nSortColumn )
	{
	case 0:
		nTest = _tcsicoll( pfA->m_sName, pfB->m_sName );
		break;
	case 1:
		{
			LPCTSTR pszA = _tcsrchr( pfA->m_sName, '.' );
			LPCTSTR pszB = _tcsrchr( pfB->m_sName, '.' );
			if ( ! pszA || ! pszB ) return 0;
			nTest = _tcsicoll( pszA, pszB );
			break;
		}
	case 2:
		if ( pfA->GetSize() == pfB->GetSize() )
			nTest = 0;
		else if ( pfA->GetSize() < pfB->GetSize() )
			nTest = -1;
		else
			nTest = 1;
		break;
	case 3:
		if ( pfA->m_pFolder == NULL || pfB->m_pFolder == NULL ) return 0;
		nTest = _tcsicoll( pfA->m_pFolder->m_sPath, pfB->m_pFolder->m_sPath );
		break;
	case 4:
		if ( pfA->m_nHitsTotal == pfB->m_nHitsTotal )
			nTest = 0;
		else if ( pfA->m_nHitsTotal < pfB->m_nHitsTotal )
			nTest = -1;
		else
			nTest = 1;
		break;
	case 5:
		if ( pfA->m_nUploadsTotal == pfB->m_nUploadsTotal )
			nTest = 0;
		else if ( pfA->m_nUploadsTotal < pfB->m_nUploadsTotal )
			nTest = -1;
		else
			nTest = 1;
		break;
	case 6:
		nTest = CompareFileTime( &pfA->m_pTime, &pfB->m_pTime );
		break;
	default:
		{
			int nColumn = m_pThis->m_nSortColumn - DETAIL_COLUMNS;
			if ( nColumn >= m_pThis->m_pColumns.GetCount() ) return 0;
			POSITION pos = m_pThis->m_pColumns.FindIndex( nColumn );
			if ( pos == NULL ) return 0;
			CSchemaMember* pMember = (CSchemaMember*)m_pThis->m_pColumns.GetAt( pos );

			CString strA, strB;
			if ( pfA->m_pMetadata ) strA = pMember->GetValueFrom( pfA->m_pMetadata, NULL, TRUE );
			if ( pfB->m_pMetadata ) strB = pMember->GetValueFrom( pfB->m_pMetadata, NULL, TRUE );

			if ( *(LPCTSTR)strA && *(LPCTSTR)strB &&
				( ((LPCTSTR)strA)[ _tcslen( strA ) - 1 ] == 'k' || ((LPCTSTR)strA)[ _tcslen( strA ) - 1 ] == '~' )
				&&
				( ((LPCTSTR)strB)[ _tcslen( strB ) - 1 ] == 'k' || ((LPCTSTR)strB)[ _tcslen( strB ) - 1 ] == '~' ) )
			{
				nTest = CLiveList::SortProc( strA, strB, TRUE );
			}
			else
			{
				nTest = _tcsicoll( strA, strB );
			}
		}
	}

	if ( ! m_pThis->m_bSortFlip ) nTest = -nTest;

	return nTest;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryDetailView selection operations

void CLibraryDetailView::CacheSelection()
{
	GET_LIST();
	SelClear( FALSE );
	
	for ( int nItem = -1 ; ( nItem = pList->GetNextItem( nItem, LVNI_SELECTED ) ) >= 0 ; )
	{
		SelAdd( pList->GetItemData( nItem ), FALSE );
	}
}

DWORD CLibraryDetailView::HitTestIndex(const CPoint& point) const
{
	GET_LIST();
	int nItem = pList->HitTest( point );
	if ( nItem < 0 ) return 0;
	return m_pList[ nItem ].nIndex;
}

void CLibraryDetailView::OnItemChanged(NM_LISTVIEW* pNotify, LRESULT* pResult)
{
	*pResult = 0;

	if ( pNotify->iItem >= 0 )
	{
		if ( ( pNotify->uOldState & LVIS_SELECTED ) != ( pNotify->uNewState & LVIS_SELECTED ) )
		{
			if ( pNotify->uNewState & LVIS_SELECTED )
			{
				SelAdd( m_pList[ pNotify->iItem ].nIndex );
				m_pList[ pNotify->iItem ].nState |= LDVI_SELECTED;
			}
			else
			{
				SelRemove( m_pList[ pNotify->iItem ].nIndex );
				m_pList[ pNotify->iItem ].nState &= ~LDVI_SELECTED;
			}
		}
	}
	else
	{
		SelClear();

		LDVITEM* pItem = m_pList;
		for ( DWORD nCount = m_nList ; nCount ; nCount--, pItem++ ) pItem->nState &= ~LDVI_SELECTED;
	}
}

void CLibraryDetailView::OnItemRangeChanged(NMLVODSTATECHANGE* pNotify, LRESULT* pResult)
{
	*pResult = 0;

	for ( int nItem = pNotify->iFrom ; nItem <= pNotify->iTo ; nItem++ )
	{
		if ( pNotify->uNewState & LVIS_SELECTED )
		{
			SelAdd( m_pList[ nItem ].nIndex );
			m_pList[ nItem ].nState |= LDVI_SELECTED;
		}
		else
		{
			SelRemove( m_pList[ nItem ].nIndex );
			m_pList[ nItem ].nState &= ~LDVI_SELECTED;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryDetailView list coupling

void CLibraryDetailView::OnColumnClick(NM_LISTVIEW* pNotify, LRESULT* pResult)
{
	*pResult = 0;
	SortItems( pNotify->iSubItem );
}

void CLibraryDetailView::OnBeginLabelEdit(LV_DISPINFO* pNotify, LRESULT* pResult)
{
	m_bEditing = TRUE;
	*pResult = 0;
}

void CLibraryDetailView::OnEndLabelEditW(LV_DISPINFO* pNotify, LRESULT* pResult)
{
	*pResult = 0;
	
	if ( pNotify->item.pszText && *pNotify->item.pszText )
	{
		CSingleLock oLock( &Library.m_pSection, TRUE );
		if ( CLibraryFile* pFile = Library.LookupFile( m_pList[ pNotify->item.iItem ].nIndex ) )
		{
			m_pList[ pNotify->item.iItem ].nState &= ~LDVI_SELECTED;
			CString strName = (LPCWSTR)pNotify->item.pszText;
			LPCTSTR pszType = _tcsrchr( pFile->m_sName, '.' );
			if ( pszType ) strName += pszType;
			*pResult = pFile->Rename( strName );
			Library.Update();
			oLock.Unlock();
			
			if ( *pResult == FALSE )
			{
				CString strFormat, strMessage, strError = theApp.GetErrorString();
				LoadString( strFormat, IDS_LIBRARY_RENAME_FAIL );
				strMessage.Format( strFormat, (LPCTSTR)pFile->m_sName, (LPCTSTR)strName );
				strMessage += _T("\r\n\r\n") + strError;
				AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
			}
		}
	}
	
	m_bEditing = FALSE;
}

void CLibraryDetailView::OnEndLabelEditA(LV_DISPINFO* pNotify, LRESULT* pResult)
{
	*pResult = 0;
	
	if ( pNotify->item.pszText && *pNotify->item.pszText )
	{
		CSingleLock oLock( &Library.m_pSection, TRUE );
		if ( CLibraryFile* pFile = Library.LookupFile( m_pList[ pNotify->item.iItem ].nIndex ) )
		{
			m_pList[ pNotify->item.iItem ].nState &= ~LDVI_SELECTED;
			CString strName = (LPCSTR)pNotify->item.pszText;
			LPCTSTR pszType = _tcsrchr( pFile->m_sName, '.' );
			if ( pszType ) strName += pszType;
			*pResult = pFile->Rename( strName );
			Library.Update();
			oLock.Unlock();
			
			if ( *pResult == FALSE )
			{
				CString strFormat, strMessage, strError = theApp.GetErrorString();
				LoadString( strFormat, IDS_LIBRARY_RENAME_FAIL );
				strMessage.Format( strFormat, (LPCTSTR)pFile->m_sName, (LPCTSTR)strName );
				strMessage += _T("\r\n\r\n") + strError;
				AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
			}
		}
	}
	
	m_bEditing = FALSE;
}

void CLibraryDetailView::OnFindItemW(NMLVFINDITEM* pNotify, LRESULT* pResult)
{
	USES_CONVERSION;
	LPCTSTR pszFind = W2CT( (LPCWSTR)pNotify->lvfi.psz );
	
	GET_LIST();
	CQuickLock oLock( Library.m_pSection );

	for ( int nLoop = 0 ; nLoop < 2 ; nLoop++ )
	{
		for ( int nItem = pNotify->iStart ; nItem < pList->GetItemCount() ; nItem++ )
		{
			if ( CLibraryFile* pFile = Library.LookupFile( m_pList[ nItem ].nIndex ) )
			{
				if ( pNotify->lvfi.flags & LVFI_STRING )
				{
					if ( _tcsnicmp( pszFind, pFile->m_sName, _tcslen( pszFind ) ) == 0 )
					{
						*pResult = nItem;
						return;
					}
				}
			}
		}
		pNotify->iStart = 0;
	}

	*pResult = -1;
}

void CLibraryDetailView::OnFindItemA(NMLVFINDITEM* pNotify, LRESULT* pResult)
{
	USES_CONVERSION;
	LPCTSTR pszFind = A2CT( (LPCSTR)pNotify->lvfi.psz );
	
	GET_LIST();
	CQuickLock oLock( Library.m_pSection );

	for ( int nLoop = 0 ; nLoop < 2 ; nLoop++ )
	{
		for ( int nItem = pNotify->iStart ; nItem < pList->GetItemCount() ; nItem++ )
		{
			if ( CLibraryFile* pFile = Library.LookupFile( m_pList[ nItem ].nIndex ) )
			{
				if ( pNotify->lvfi.flags & LVFI_STRING )
				{
					if ( _tcsnicmp( pszFind, pFile->m_sName, _tcslen( pszFind ) ) == 0 )
					{
						*pResult = nItem;
						return;
					}
				}
			}
		}
		pNotify->iStart = 0;
	}

	*pResult = -1;
}

void CLibraryDetailView::OnCustomDraw(NMLVCUSTOMDRAW* pNotify, LRESULT* pResult)
{
	if ( pNotify->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (	pNotify->nmcd.dwDrawStage == CDDS_ITEMPREPAINT &&
				pNotify->nmcd.dwItemSpec < m_nList )
	{
		LDVITEM* pItem = m_pList + pNotify->nmcd.dwItemSpec;

		if ( pItem->nState & LDVI_SELECTED )
		{
			pNotify->clrText = CoolInterface.m_crHiText;
			pNotify->clrTextBk = CoolInterface.m_crHighlight;
		}
		else if ( pItem->nState & LDVI_UNSAFE )		pNotify->clrText = RGB( 255, 0, 0 );
		else if ( pItem->nState & LDVI_UNSCANNED )	pNotify->clrText = CoolInterface.m_crDisabled;
		else if ( pItem->nState & LDVI_PRIVATE )	pNotify->clrText = CoolInterface.m_crHighlight;

		if ( m_bCreateDragImage )
		{
			pNotify->clrTextBk = RGB( 250, 255, 250 );
		}

		*pResult = CDRF_DODEFAULT;
	}
	else
	{
		*pResult = 0;
	}
}

void CLibraryDetailView::OnDblClk(NMHDR* pNotify, LRESULT* pResult)
{
	*pResult = 0;
	SendMessage( WM_COMMAND, ID_LIBRARY_LAUNCH );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryDetailView column context menu

void CLibraryDetailView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CRect rcHeader;

	if ( CWnd* pHeader = GetWindow( GW_CHILD ) )
	{
		pHeader->GetWindowRect( &rcHeader );
	}

	if ( m_pSchema == NULL || rcHeader.PtInRect( point ) == FALSE || m_nStyle != LVS_REPORT )
	{
		CLibraryFileView::OnContextMenu( pWnd, point );
		return;
	}

	CMenu* pMenu = CSchemaColumnsDlg::BuildColumnMenu( m_pSchema, &m_pColumns );
	
	pMenu->AppendMenu( MF_SEPARATOR, ID_SEPARATOR, (LPCTSTR)NULL );
	CString strSchemas;
	LoadString( strSchemas, IDS_SCHEMAS );
	pMenu->AppendMenu( MF_STRING, ID_LIBRARY_COLUMNS, strSchemas + _T("...") );

	m_pCoolMenu = new CCoolMenu();
	m_pCoolMenu->AddMenu( pMenu, TRUE );
	m_pCoolMenu->SetWatermark( Skin.GetWatermark( _T("CCoolMenu") ) );
	
	UINT nCmd = pMenu->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|
		TPM_RETURNCMD, point.x, point.y, this );

	delete pMenu;
	delete m_pCoolMenu;
	m_pCoolMenu = NULL;

	if ( nCmd == ID_LIBRARY_COLUMNS )
	{
		OnLibraryColumns();
	}
	else if ( nCmd )
	{
		CPtrList pColumns;
		CSchemaColumnsDlg::ToggleColumnHelper( m_pSchema, &m_pColumns, &pColumns, nCmd, TRUE );
		SetViewSchema( m_pSchema, &pColumns, TRUE, TRUE );
	}
}

void CLibraryDetailView::OnUpdateBlocker(CCmdUI* pCmdUI)
{
	if ( m_pCoolMenu ) pCmdUI->Enable( TRUE );
	else pCmdUI->ContinueRouting();
}

void CLibraryDetailView::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if ( m_pCoolMenu ) m_pCoolMenu->OnMeasureItem( lpMeasureItemStruct );
}

void CLibraryDetailView::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( m_pCoolMenu ) m_pCoolMenu->OnDrawItem( lpDrawItemStruct );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryDetailView drag / drop

void CLibraryDetailView::OnBeginDrag(NM_LISTVIEW* pNotify, LRESULT* pResult)
{
	GET_LIST();

	CPoint ptAction( pNotify->ptAction );

	m_bCreateDragImage = TRUE;
	CImageList* pImage = CLiveList::CreateDragImage( pList, ptAction );
	m_bCreateDragImage = FALSE;

	if ( pImage == NULL ) return;

	UpdateWindow();

	ClientToScreen( &ptAction );
	DragObjects( pImage, ptAction );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryDetailView command handlers

void CLibraryDetailView::OnUpdateLibraryRename(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetSelectedCount() == 1 );
}

void CLibraryDetailView::OnLibraryRename() 
{
	int nItem = ListView_GetNextItem( GetSafeHwnd(), -1, LVNI_SELECTED );
	if ( nItem >= 0 ) ListView_EditLabel( GetSafeHwnd(), nItem );
}

void CLibraryDetailView::OnUpdateLibraryColumns(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_nStyle == LVS_REPORT );
}

void CLibraryDetailView::OnLibraryColumns() 
{
	CSchemaColumnsDlg dlg;

	dlg.m_pSchema = m_pSchema;
	dlg.m_pColumns.AddTail( &m_pColumns );

	if ( dlg.DoModal() != IDOK ) return;
	
	if ( Settings.Library.FilterURI.IsEmpty() )
	{
		Settings.Library.SchemaURI.Empty();
		if ( dlg.m_pSchema ) Settings.Library.SchemaURI = dlg.m_pSchema->m_sURI;
	}

	SetViewSchema( dlg.m_pSchema, &dlg.m_pColumns, TRUE, TRUE );
}

