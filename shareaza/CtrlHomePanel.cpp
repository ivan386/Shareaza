//
// CtrlHomePanel.cpp
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
#include "CtrlHomePanel.h"

#include "Statistics.h"
#include "Network.h"
#include "Neighbours.h"
#include "GraphItem.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "LibraryHistory.h"
#include "SharedFile.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"

#include "CoolInterface.h"
#include "ShellIcons.h"
#include "RichDocument.h"
#include "RichElement.h"
#include "FragmentBar.h"
#include "FileExecutor.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CHomePanel, CTaskPanel)
BEGIN_MESSAGE_MAP(CHomePanel, CTaskPanel)
	//{{AFX_MSG_MAP(CHomePanel)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CHomeDownloadsBox, CRichTaskBox)
BEGIN_MESSAGE_MAP(CHomeDownloadsBox, CRichTaskBox)
	//{{AFX_MSG_MAP(CHomeDownloadsBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CHomeLibraryBox, CRichTaskBox)
BEGIN_MESSAGE_MAP(CHomeLibraryBox, CRichTaskBox)
	//{{AFX_MSG_MAP(CHomeLibraryBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CHomeUploadsBox, CRichTaskBox)
BEGIN_MESSAGE_MAP(CHomeUploadsBox, CRichTaskBox)
	//{{AFX_MSG_MAP(CHomeUploadsBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CHomeConnectionBox, CRichTaskBox)
BEGIN_MESSAGE_MAP(CHomeConnectionBox, CRichTaskBox)
	//{{AFX_MSG_MAP(CHomeConnectionBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CHomeTorrentsBox, CRichTaskBox)
BEGIN_MESSAGE_MAP(CHomeTorrentsBox, CRichTaskBox)
	//{{AFX_MSG_MAP(CHomeTorrentsBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CHomePanel construction

CHomePanel::CHomePanel()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHomePanel message handlers

BOOL CHomePanel::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 200, 0 );
	return CTaskPanel::Create( _T("CHomePanel"), WS_VISIBLE, rect, pParentWnd, IDC_HOME_PANEL );
}

int CHomePanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CTaskPanel::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_boxDownloads.Create( this, _T("Downloads"), IDR_DOWNLOADSFRAME );
	m_boxUploads.Create( this, _T("Uploads"), IDR_UPLOADSFRAME );
	m_boxConnection.Create( this, _T("Connection"), IDR_NEIGHBOURSFRAME );
	m_boxLibrary.Create( this, _T("Library"), IDR_LIBRARYFRAME );
#ifndef LAN_MODE
	m_boxTorrents.Create( this, _T("Torrents"), ID_NETWORK_BT );
#endif // LAN_MODE

	AddBox( &m_boxDownloads );
	AddBox( &m_boxLibrary );
	AddBox( &m_boxConnection );
	if ( ! Settings.Interface.LowResMode )
	{
		AddBox( &m_boxUploads );
#ifndef LAN_MODE
		AddBox( &m_boxTorrents );
#endif // LAN_MODE
	}

	// SetStretchBox( &m_boxLibrary );

	return 0;
}

void CHomePanel::OnSkinChange()
{
	SetWatermark( _T("CHomePanel") );
	SetFooter( _T("CHomePanel.Footer") );

	m_boxDownloads.OnSkinChange();
	m_boxUploads.OnSkinChange();
	m_boxConnection.OnSkinChange();
	m_boxLibrary.OnSkinChange();
#ifndef LAN_MODE
	m_boxTorrents.OnSkinChange();
#endif // LAN_MODE

	Update();
	Invalidate();
}

void CHomePanel::Update()
{
	m_boxDownloads.Update();
	m_boxUploads.Update();
	m_boxConnection.Update();
	m_boxLibrary.Update();
#ifndef LAN_MODE
	m_boxTorrents.Update();
#endif // LAN_MODE
}


/////////////////////////////////////////////////////////////////////////////
// CHomeDownloadsBox construction

CHomeDownloadsBox::CHomeDownloadsBox() :
	m_pdDownloadsNone( NULL ),
	m_pdDownloadsOne( NULL ),
	m_pdDownloadsMany( NULL ),
	m_hHand( NULL ),
	m_pHover( NULL )
{
	SetPrimary();
}

CHomeDownloadsBox::~CHomeDownloadsBox()
{
	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		delete m_pList.GetAt( nItem );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHomeDownloadsBox message handlers

int CHomeDownloadsBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CRichTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_pFont.CreateFont( -(int)(Settings.Fonts.FontSize - 1), 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );

	m_hHand = theApp.LoadCursor( IDC_HAND );

	m_wndTip.Create( this, &Settings.Interface.TipDownloads );

	return 0;
}

void CHomeDownloadsBox::OnSkinChange()
{
	if ( m_pDocument ) delete m_pDocument;
	m_pDocument = NULL;
	m_pdDownloadsNone = m_pdDownloadsOne = m_pdDownloadsMany = NULL;

	SetCaptionmark( _T("CHomeDownloadsBox.Caption") );

	CXMLElement* pXML = Skin.GetDocument( _T("CHomeDownloadsBox") );
	if ( pXML == NULL ) return;

	SetCaption( pXML->GetAttributeValue( _T("title"), _T("Downloads") ) );
	HICON hIcon = CoolInterface.ExtractIcon( IDR_DOWNLOADSFRAME, Settings.General.LanguageRTL );
	if ( hIcon )
		SetIcon( hIcon );

	m_pDocument = new CRichDocument();

	CElementMap pMap;
	if ( ! m_pDocument->LoadXML( pXML, &pMap ) ) return;

	pMap.Lookup( _T("DownloadsNone"), m_pdDownloadsNone );
	pMap.Lookup( _T("DownloadsOne"), m_pdDownloadsOne );
	pMap.Lookup( _T("DownloadsMany"), m_pdDownloadsMany );

	if ( m_pdDownloadsMany ) m_sDownloadsMany = m_pdDownloadsMany->m_sText;

	GetView().SetDocument( m_pDocument );
	Update();
}

void CHomeDownloadsBox::Update()
{
	if ( m_pDocument == NULL ) return;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;

	BOOL bChanged = FALSE;
	CString str;

	for ( INT_PTR nItem = m_pList.GetSize() - 1 ; nItem >= 0 ; nItem-- )
	{
		Item* pItem = m_pList.GetAt( nItem );

		if ( ! Downloads.CheckActive( pItem->m_pDownload, 6 ) )
		{
			delete pItem;
			m_pList.RemoveAt( nItem );
			bChanged = TRUE;
			m_pHover = NULL;
			KillTimer( 2 );
		}
	}

	int nCount = 0;
	int nInsert = 0;

	for ( POSITION pos = Downloads.GetReverseIterator() ; pos && nCount < 6 ; )
	{
		CDownload* pDownload = Downloads.GetPrevious( pos );
		if ( pDownload->IsPaused() ) continue;
		if ( pDownload->IsCompleted() ) continue;

		Item* pItem = NULL;

		for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
		{
			pItem = m_pList.GetAt( nItem );
			if ( pItem->m_pDownload == pDownload ) break;
			pItem = NULL;
		}

		if ( pItem == NULL )
		{
			pItem				= new Item();
			pItem->m_pDownload	= pDownload;
			pItem->m_sText		= pDownload->GetDisplayName();
			pItem->m_nIcon16	= ShellIcons.Get( pItem->m_sText, 16 );
			m_pList.InsertAt( nInsert++, pItem );
			bChanged = TRUE;
			m_pHover = NULL;
			KillTimer( 2 );
		}

		QWORD nComplete	= pDownload->GetVolumeComplete();
		BOOL bPaused	= pDownload->GetFirstTransfer() == NULL;

		if ( pItem->m_nComplete != nComplete ||
			 pItem->m_bPaused != bPaused ||
			 pItem->m_nSize != pDownload->m_nSize )
		{
			pItem->m_nSize		= pDownload->m_nSize;
			pItem->m_nComplete	= nComplete;
			pItem->m_bPaused	= bPaused;
			bChanged = TRUE;
		}

		nCount++;
	}

	nCount = static_cast< int >( m_pList.GetSize() * 18 );
	if ( nCount ) nCount += 6;

	m_pDocument->ShowGroup( 1, m_pList.GetSize() == 0 );

	int nActive = (int)CGraphItem::GetValue( GRC_DOWNLOADS_FILES );

	if ( nActive > 1 )
	{
		if ( m_pdDownloadsMany )
		{
			str.Format( m_sDownloadsMany, nActive );
			m_pdDownloadsMany->SetText( str );
			m_pdDownloadsMany->Show( TRUE );
		}
		if ( m_pdDownloadsOne ) m_pdDownloadsOne->Show( FALSE );
		if ( m_pdDownloadsNone ) m_pdDownloadsNone->Show( FALSE );
	}
	else if ( nActive == 1 )
	{
		if ( m_pdDownloadsMany ) m_pdDownloadsMany->Show( FALSE );
		if ( m_pdDownloadsOne ) m_pdDownloadsOne->Show( TRUE );
		if ( m_pdDownloadsNone ) m_pdDownloadsNone->Show( FALSE );
	}
	else
	{
		if ( m_pdDownloadsMany ) m_pdDownloadsMany->Show( FALSE );
		if ( m_pdDownloadsOne ) m_pdDownloadsOne->Show( FALSE );
		if ( m_pdDownloadsNone ) m_pdDownloadsNone->Show( TRUE );
	}

	m_pDocument->ShowGroup( 1, nActive == 0 );

	if ( GetView().IsModified() )
	{
		CRect rc;
		GetClientRect( &rc );
		m_nWidth = rc.Width();
		nCount += m_wndView.FullHeightMove( 0, nCount, m_nWidth );
	}
	else
	{
		CRect rc;
		m_wndView.GetWindowRect( &rc );
		ScreenToClient( &rc );

		if ( rc.top != nCount )
		{
			m_nWidth = rc.Width();
			nCount += m_wndView.FullHeightMove( 0, nCount, m_nWidth );
		}
		else
		{
			m_wndView.GetClientRect( &rc );
			nCount += rc.Height();
		}
	}

	SetSize( nCount );

	if ( bChanged ) Invalidate();
}

void CHomeDownloadsBox::OnSize(UINT nType, int cx, int cy)
{
	CTaskBox::OnSize( nType, cx, cy );

	if ( m_nWidth != cx )
	{
		m_nWidth = cx;
		int nCount = static_cast< int >( m_pList.GetSize() * 18 );
		if ( nCount ) nCount += 6;
		nCount += m_wndView.FullHeightMove( 0, nCount, m_nWidth );
		SetSize( nCount );
	}
}

void CHomeDownloadsBox::OnPaint()
{
	CRect rcClient, rcIcon, rcText;
	CPaintDC dc( this );

	GetClientRect( &rcClient );
	m_wndView.GetClientRect( &rcIcon );
	rcClient.bottom -= rcIcon.Height();
	rcClient.top += 6;

	rcIcon.SetRect( 4, rcClient.top, 4 + 20, rcClient.top + 16 );
	rcText.SetRect( rcIcon.right, rcIcon.top, rcClient.right - 4, rcIcon.bottom );
	rcIcon.DeflateRect( 0, 2 );

	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( CoolInterface.m_crRichdocBack );
	dc.SetTextColor( CoolInterface.m_crTextLink );

	CFont* pOldFont = (CFont*)dc.SelectObject( &m_pFont );

	COLORREF crAlt[3] = { RGB( 0, 153, 255 ), RGB( 190, 0, 0 ), RGB( 0, 153, 0 ) };

	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		Item* pItem = m_pList.GetAt( nItem );

		if ( pItem->m_nComplete == 0 || pItem->m_nSize == SIZE_UNKNOWN )
		{
			CRect rc( rcIcon.left, rcIcon.top, rcIcon.left + 16, rcIcon.top + 16 );
			ShellIcons.Draw( &dc, pItem->m_nIcon16, 16, rc.left, rc.top,
				CoolInterface.m_crWindow );
			dc.ExcludeClipRect( &rc );
		}
		else
		{
			COLORREF cr = pItem->m_bPaused ? CoolInterface.m_crNetworkNull : crAlt[ nItem % 3 ];
			dc.Draw3dRect( &rcIcon, CoolInterface.m_crFragmentBorder, CoolInterface.m_crFragmentBorder );
			rcIcon.DeflateRect( 1, 1 );
			CFragmentBar::DrawFragment( &dc, &rcIcon, pItem->m_nSize, 0, pItem->m_nComplete, cr, true );
			dc.FillSolidRect( &rcIcon, CoolInterface.m_crWindow );
			rcIcon.InflateRect( 1, 1 );
			dc.ExcludeClipRect( &rcIcon );
		}

		CString str = pItem->m_sText;

		if ( dc.GetTextExtent( str ).cx > rcText.Width() - 8 )
		{
			while ( str.GetLength() && dc.GetTextExtent( str + _T('\x2026') ).cx > rcText.Width() - 8 )
			{
				str = str.Left( str.GetLength() - 1 );
			}
			str += _T('\x2026');
		}

		dc.SetTextColor( m_pHover == pItem ? CoolInterface.m_crTextLinkHot : CoolInterface.m_crTextLink );
		dc.ExtTextOut( rcText.left + 4, rcText.top + 2, ETO_CLIPPED|ETO_OPAQUE,
			&rcText, str, NULL );

		dc.ExcludeClipRect( &rcText );

		rcIcon.OffsetRect( 0, 18 );
		rcText.OffsetRect( 0, 18 );
	}

	rcClient.top = 0;
	dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );
	dc.SelectObject( pOldFont );
}

CHomeDownloadsBox::Item* CHomeDownloadsBox::HitTest(const CPoint& point) const
{
	CRect rcClient, rcIcon, rcText;

	GetClientRect( &rcClient );
	rcClient.top += 6;

	rcIcon.SetRect( 4, rcClient.top, 4 + 20, rcClient.top + 16 );
	rcText.SetRect( rcIcon.right, rcIcon.top, rcClient.right - 4, rcIcon.bottom );

	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		Item* pItem = m_pList.GetAt( nItem );

		if ( rcIcon.PtInRect( point ) || rcText.PtInRect( point ) ) return pItem;

		rcIcon.OffsetRect( 0, 18 );
		rcText.OffsetRect( 0, 18 );
	}

	return NULL;
}

BOOL CHomeDownloadsBox::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	GetCursorPos( &point );
	ScreenToClient( &point );

	Item* pItem = HitTest( point );

	if ( pItem != NULL )
		m_wndTip.Show( pItem->m_pDownload );
	else
		m_wndTip.Hide();

	if ( pItem != m_pHover )
	{
		if ( pItem != NULL && m_pHover == NULL )
			SetTimer( 2, 200, NULL );
		else if ( pItem == NULL && m_pHover != NULL )
			KillTimer( 2 );

		m_pHover = pItem;
		Invalidate();
	}

	if ( m_pHover != NULL )
	{
		::SetCursor( m_hHand );
		return TRUE;
	}

	return CTaskBox::OnSetCursor( pWnd, nHitTest, message );
}

void CHomeDownloadsBox::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_wndTip.Hide();

	if ( Item* pItem = HitTest( point ) )
	{
		m_pHover = NULL;
		KillTimer( 2 );
		Invalidate();

		ExecuteDownload( pItem->m_pDownload );
	}

	CTaskBox::OnLButtonUp( nFlags, point );
}

void CHomeDownloadsBox::OnTimer(UINT_PTR nIDEvent)
{
	CTaskBox::OnTimer( nIDEvent );

	CPoint point;
	CRect rect;

	GetCursorPos( &point );
	GetWindowRect( &rect );

	if ( rect.PtInRect( point ) ) return;

	KillTimer( 2 );

	if ( m_pHover != NULL )
	{
		m_pHover = NULL;
		Invalidate();
	}
}

BOOL CHomeDownloadsBox::ExecuteDownload(CDownload* pDownload)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 1000 ) ) return FALSE;
	if ( ! Downloads.Check( pDownload ) ) return FALSE;

	if ( ! pDownload->Launch( -1, &pLock, FALSE ) )
		PostMainWndMessage( WM_COMMAND, ID_VIEW_DOWNLOADS );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CHomeLibraryBox

CHomeLibraryBox::CHomeLibraryBox() :
	m_pdLibraryFiles( NULL ),
	m_pdLibraryVolume( NULL ),
	m_pdLibraryHashRemaining( NULL ),
	m_hHand( NULL ),
	m_pHover( NULL )
{
	SetPrimary();
}

CHomeLibraryBox::~CHomeLibraryBox()
{
	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		delete m_pList.GetAt( nItem );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHomeLibraryBox message handlers

int CHomeLibraryBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CRichTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_pFont.CreateFont( -(int)(Settings.Fonts.FontSize - 1), 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );

	m_hHand = theApp.LoadCursor( IDC_HAND );

	m_wndTip.Create( this, &Settings.Interface.TipLibrary );

	return 0;
}

void CHomeLibraryBox::OnSkinChange()
{
	if ( m_pDocument ) delete m_pDocument;
	m_pDocument = NULL;
	m_pdLibraryFiles = m_pdLibraryVolume = m_pdLibraryHashRemaining = NULL;

	SetCaptionmark( _T("CHomeLibraryBox.Caption") );

	CXMLElement* pXML = Skin.GetDocument( _T("CHomeLibraryBox") );
	if ( pXML == NULL ) return;

	SetCaption( pXML->GetAttributeValue( _T("title"), _T("Library") ) );
	HICON hIcon = CoolInterface.ExtractIcon( IDR_LIBRARYFRAME, Settings.General.LanguageRTL );
	if ( hIcon )
		SetIcon( hIcon );

	m_pDocument = new CRichDocument();

	CElementMap pMap;
	if ( ! m_pDocument->LoadXML( pXML, &pMap ) ) return;

	pMap.Lookup( _T("LibraryFiles"), m_pdLibraryFiles );
	pMap.Lookup( _T("LibraryVolume"), m_pdLibraryVolume );
	pMap.Lookup( _T("LibraryHashRemaining"), m_pdLibraryHashRemaining );

	SetDocument( m_pDocument );

	Update();
}

void CHomeLibraryBox::Update()
{
	if ( m_pDocument == NULL ) return;

	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;

	BOOL bChanged = FALSE;
	int nCount = 0;
	CString str;

	for ( INT_PTR nItem = m_pList.GetSize() - 1 ; nItem >= 0 ; nItem-- )
	{
		Item* pItem = m_pList.GetAt( nItem );

		if ( ! LibraryHistory.Check( pItem->m_pRecent, 6 ) ||
			NULL == pItem->m_pRecent->m_pFile )
		{
			delete pItem;
			m_pList.RemoveAt( nItem );
			bChanged = TRUE;
		}
	}

	for ( POSITION pos = LibraryHistory.GetIterator() ; pos && nCount < 6 ; )
	{
		CLibraryRecent* pRecent = LibraryHistory.GetNext( pos );
		if ( pRecent->m_pFile == NULL ) continue;

        INT_PTR nItem = m_pList.GetSize() - 1;
		for ( ; nItem >= 0 ; nItem-- )
		{
			Item* pItem = m_pList.GetAt( nItem );
			if ( pItem->m_pRecent == pRecent ) break;
		}

		if ( nItem < 0 )
		{
			Item* pItem			= new Item();
			pItem->m_pRecent	= pRecent;
			pItem->m_nIndex		= pRecent->m_pFile->m_nIndex;
			pItem->m_sText		= pRecent->m_pFile->m_sName;
			pItem->m_nIcon16	= ShellIcons.Get( pRecent->m_pFile->GetPath(), 16 );

			m_pList.InsertAt( nCount, pItem );
			bChanged = TRUE;
		}

		nCount++;
	}

	nCount = static_cast< int >( m_pList.GetSize() * 18 );
	if ( nCount ) nCount += 6;

	m_pDocument->ShowGroup( 2, m_pList.GetSize() == 0 );

	QWORD nVolume;
	DWORD nFiles;

	LibraryMaps.GetStatistics( &nFiles, &nVolume );

	if ( m_pdLibraryFiles )
	{
		str.Format( _T("%lu "), nFiles );
		if ( m_pdLibraryFiles->m_sText.Compare( str ) != 0 )
		{
			m_pdLibraryFiles->SetText( str );
			bChanged = TRUE;
		}
	}

	if ( m_pdLibraryVolume )
	{
		str = Settings.SmartVolume( nVolume, KiloBytes ) + ' ';
		if ( m_pdLibraryVolume->m_sText.Compare( str ) != 0 )
		{
			m_pdLibraryVolume->SetText( str );
			bChanged = TRUE;
		}
	}

	DWORD nHashing = (DWORD)LibraryBuilder.GetRemaining();

	if ( nHashing > 0 )
	{
		str.Format( _T("%lu "), nHashing );
		if ( m_pdLibraryHashRemaining )
		{
			if ( m_pdLibraryHashRemaining->m_sText.Compare( str ) != 0 )
			{
				m_pdLibraryHashRemaining->SetText( str );
				bChanged = TRUE;
			}
		}
		m_pDocument->ShowGroup( 1, TRUE );

		BOOL bPriority = LibraryBuilder.GetBoostPriority();
		m_pDocument->ShowGroup( 3, bPriority == FALSE );
		m_pDocument->ShowGroup( 4, bPriority == TRUE );
	}
	else
	{
		m_pDocument->ShowGroup( 1, FALSE );
		m_pDocument->ShowGroup( 3, FALSE );
		m_pDocument->ShowGroup( 4, FALSE );
	}

	if ( GetView().IsModified() )
	{
		CRect rc;
		GetClientRect( &rc );
		m_nWidth = rc.Width();
		nCount += m_wndView.FullHeightMove( 0, nCount, m_nWidth );
	}
	else
	{
		CRect rc;
		m_wndView.GetWindowRect( &rc );
		ScreenToClient( &rc );

		if ( rc.top != nCount )
		{
			m_nWidth = rc.Width();
			nCount += m_wndView.FullHeightMove( 0, nCount, m_nWidth );
		}
		else
		{
			m_wndView.GetClientRect( &rc );
			nCount += rc.Height();
		}
	}

	SetSize( nCount );

	if ( bChanged )
	{
		m_pHover = NULL;
		KillTimer( 2 );
		m_wndView.Invalidate();
	}
}

void CHomeLibraryBox::OnSize(UINT nType, int cx, int cy)
{
	CTaskBox::OnSize( nType, cx, cy );

	if ( m_nWidth != cx )
	{
		m_nWidth = cx;
		int nCount = static_cast< int >( m_pList.GetSize() * 18 );
		if ( nCount ) nCount += 6;
		nCount += m_wndView.FullHeightMove( 0, nCount, m_nWidth );
		SetSize( nCount );
	}
}

void CHomeLibraryBox::OnPaint()
{
	CRect rcClient, rcIcon, rcText;
	CPaintDC dc( this );

	GetClientRect( &rcClient );
	m_wndView.GetClientRect( &rcIcon );
	rcClient.bottom -= rcIcon.Height();
	rcClient.top += 6;

	rcIcon.SetRect( 4, rcClient.top, 4 + 16, rcClient.top + 16 );
	rcText.SetRect( rcIcon.right, rcIcon.top, rcClient.right - 4, rcIcon.bottom );

	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( CoolInterface.m_crWindow );
	dc.SetTextColor( CoolInterface.m_crTextLink );

	CFont* pOldFont = (CFont*)dc.SelectObject( &m_pFont );

	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		Item* pItem = m_pList.GetAt( nItem );

		ShellIcons.Draw( &dc, pItem->m_nIcon16, 16, rcIcon.left, rcIcon.top,
			CoolInterface.m_crWindow );

		CString str = pItem->m_sText;

		if ( dc.GetTextExtent( str ).cx > rcText.Width() - 8 )
		{
			while ( str.GetLength() && dc.GetTextExtent( str + _T('\x2026') ).cx > rcText.Width() - 8 )
			{
				str = str.Left( str.GetLength() - 1 );
			}
			str += _T('\x2026');
		}

		dc.SetTextColor( m_pHover == pItem ? CoolInterface.m_crTextLinkHot : CoolInterface.m_crTextLink );
		dc.ExtTextOut( rcText.left + 4, rcText.top + 2, ETO_CLIPPED|ETO_OPAQUE,
			&rcText, str, NULL );

		dc.ExcludeClipRect( &rcIcon );
		dc.ExcludeClipRect( &rcText );

		rcIcon.OffsetRect( 0, 18 );
		rcText.OffsetRect( 0, 18 );
	}

	rcClient.top = 0;
	dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );
	dc.SelectObject( pOldFont );
}

CHomeLibraryBox::Item* CHomeLibraryBox::HitTest(const CPoint& point) const
{
	CRect rcClient, rcIcon, rcText;

	GetClientRect( &rcClient );
	rcClient.top += 6;

	rcIcon.SetRect( 4, rcClient.top, 4 + 16, rcClient.top + 16 );
	rcText.SetRect( rcIcon.right, rcIcon.top, rcClient.right - 4, rcIcon.bottom );

	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		Item* pItem = m_pList.GetAt( nItem );

		if ( rcIcon.PtInRect( point ) || rcText.PtInRect( point ) ) return pItem;

		rcIcon.OffsetRect( 0, 18 );
		rcText.OffsetRect( 0, 18 );
	}

	return NULL;
}

BOOL CHomeLibraryBox::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	GetCursorPos( &point );
	ScreenToClient( &point );

	Item* pItem = HitTest( point );

	if ( pItem != NULL )
		m_wndTip.Show( pItem->m_nIndex );
	else
		m_wndTip.Hide();

	if ( pItem != m_pHover )
	{
		if ( pItem != NULL && m_pHover == NULL )
			SetTimer( 2, 200, NULL );
		else if ( pItem == NULL && m_pHover != NULL )
			KillTimer( 2 );

		m_pHover = pItem;
		Invalidate();
	}

	if ( m_pHover != NULL )
	{
		::SetCursor( m_hHand );
		return TRUE;
	}

	return CTaskBox::OnSetCursor( pWnd, nHitTest, message );
}

void CHomeLibraryBox::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_wndTip.Hide();

	if ( Item* pItem = HitTest( point ) )
	{
		CSingleLock oLock( &Library.m_pSection, TRUE );
		if ( CLibraryFile* pFile = Library.LookupFile( pItem->m_nIndex ) )
		{
			if ( pFile->IsAvailable() )
			{
				CString strPath = pFile->GetPath();
				oLock.Unlock();
				CFileExecutor::Execute( strPath );
			}
			else
			{
				oLock.Unlock();
			}
			m_pHover = NULL;
			KillTimer( 2 );
			Invalidate();
		}
	}

	CTaskBox::OnLButtonUp( nFlags, point );
}

void CHomeLibraryBox::OnTimer(UINT_PTR nIDEvent)
{
	CTaskBox::OnTimer( nIDEvent );

	CPoint point;
	CRect rect;

	GetCursorPos( &point );
	GetWindowRect( &rect );

	if ( rect.PtInRect( point ) ) return;

	KillTimer( 2 );

	if ( m_pHover != NULL )
	{
		m_pHover = NULL;
		Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHomeUploadsBox construction

CHomeUploadsBox::CHomeUploadsBox() :
	m_pdUploadsNone( NULL ),
	m_pdUploadsOne( NULL ),
	m_pdUploadsMany( NULL ),
	m_pdUploadedNone( NULL ),
	m_pdUploadedOne( NULL ),
	m_pdUploadedMany( NULL )
{
	SetPrimary();
}

CHomeUploadsBox::~CHomeUploadsBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHomeUploadsBox message handlers

void CHomeUploadsBox::OnSkinChange()
{
	if ( m_pDocument ) delete m_pDocument;
	m_pDocument = NULL;
	m_pdUploadsNone = m_pdUploadsOne = m_pdUploadsMany = NULL;
	m_pdUploadedNone = m_pdUploadedOne = m_pdUploadedMany = NULL;

	SetCaptionmark( _T("CHomeUploadsBox.Caption") );

	CXMLElement* pXML = Skin.GetDocument( _T("CHomeUploadsBox") );
	if ( pXML == NULL ) return;

	SetCaption( pXML->GetAttributeValue( _T("title"), _T("Uploads") ) );
	HICON hIcon = CoolInterface.ExtractIcon( IDR_UPLOADSFRAME, Settings.General.LanguageRTL );
	if ( hIcon )
		SetIcon( hIcon );

	m_pDocument = new CRichDocument();

	CElementMap pMap;
	if ( ! m_pDocument->LoadXML( pXML, &pMap ) ) return;

	pMap.Lookup( _T("UploadsNone"), m_pdUploadsNone );
	pMap.Lookup( _T("UploadsOne"), m_pdUploadsOne );
	pMap.Lookup( _T("UploadsMany"), m_pdUploadsMany );
	pMap.Lookup( _T("UploadedNone"), m_pdUploadedNone );
	pMap.Lookup( _T("UploadedOne"), m_pdUploadedOne );
	pMap.Lookup( _T("UploadedMany"), m_pdUploadedMany );

	if ( m_pdUploadedOne ) m_sUploadedOne = m_pdUploadedOne->m_sText;
	if ( m_pdUploadedMany ) m_sUploadedMany = m_pdUploadedMany->m_sText;

	if ( m_pdUploadsMany ) m_sUploadsMany = m_pdUploadsMany->m_sText;

	SetDocument( m_pDocument );

	Update();
}

void CHomeUploadsBox::Update()
{
	if ( m_pDocument == NULL ) return;

	CString str;

	int nCount = (int)CGraphItem::GetValue( GRC_UPLOADS_TRANSFERS );

	if ( nCount > 1 )
	{
		if ( m_pdUploadsMany )
		{
			str.Format( m_sUploadsMany, nCount );
			m_pdUploadsMany->SetText( str );
			m_pdUploadsMany->Show( TRUE );
		}
		if ( m_pdUploadsOne ) m_pdUploadsOne->Show( FALSE );
		if ( m_pdUploadsNone ) m_pdUploadsNone->Show( FALSE );
	}
	else if ( nCount == 1 )
	{
		if ( m_pdUploadsMany ) m_pdUploadsMany->Show( FALSE );
		if ( m_pdUploadsOne ) m_pdUploadsOne->Show( TRUE );
		if ( m_pdUploadsNone ) m_pdUploadsNone->Show( FALSE );
	}
	else
	{
		if ( m_pdUploadsMany ) m_pdUploadsMany->Show( FALSE );
		if ( m_pdUploadsOne ) m_pdUploadsOne->Show( FALSE );
		if ( m_pdUploadsNone ) m_pdUploadsNone->Show( TRUE );
	}

	if ( Statistics.Today.Uploads.Files == 0 )
	{
		if ( m_pdUploadedNone ) m_pdUploadedNone->Show( TRUE );
		if ( m_pdUploadedOne )  m_pdUploadedOne->Show( FALSE );
		if ( m_pdUploadedMany ) m_pdUploadedMany->Show( FALSE );
	}
	else if ( Statistics.Today.Uploads.Files == 1 )
	{
		str.Format( m_sUploadedOne, (LPCTSTR)Settings.SmartVolume( Statistics.Today.Uploads.Volume, KiloBytes ) );

		if ( m_pdUploadedNone ) m_pdUploadedNone->Show( FALSE );
		if ( m_pdUploadedMany ) m_pdUploadedMany->Show( FALSE );

		if ( m_pdUploadedOne )
		{
			m_pdUploadedOne->SetText( str );
			m_pdUploadedOne->Show( TRUE );
		}
	}
	else
	{
		str.Format( m_sUploadedMany, (int)Statistics.Today.Uploads.Files, (LPCTSTR)Settings.SmartVolume( Statistics.Today.Uploads.Volume, KiloBytes ) );

		if ( m_pdUploadedNone ) m_pdUploadedNone->Show( FALSE );
		if ( m_pdUploadedOne )  m_pdUploadedOne->Show( FALSE );

		if ( m_pdUploadedMany )
		{
			m_pdUploadedMany->SetText( str );
			m_pdUploadedMany->Show( TRUE );
		}
	}

	CRichTaskBox::Update();
}

/////////////////////////////////////////////////////////////////////////////
// CHomeConnectionBox construction

CHomeConnectionBox::CHomeConnectionBox() :
	m_pdConnectedHours( NULL ),
	m_pdConnectedMinutes( NULL )
{
	ZeroMemory( m_pdCount, sizeof( m_pdCount ) );
	SetPrimary();
}

CHomeConnectionBox::~CHomeConnectionBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHomeConnectionBox message handlers

void CHomeConnectionBox::OnSkinChange()
{
	if ( m_pDocument ) delete m_pDocument;
	m_pDocument = NULL;
	m_pdConnectedHours = m_pdConnectedMinutes = NULL;

	for ( int nP = PROTOCOL_NULL ; nP < PROTOCOL_LAST ; ++nP )
	{
		for ( int nT = ntNode ; nT <= ntLeaf ; nT++ )
		{
			m_pdCount[ nP ][ nT ] = NULL;
			m_sCount[ nP ][ nT ].Empty();
		}
	}

	SetCaptionmark( _T("CHomeConnectionBox.Caption") );

	CXMLElement* pXML = Skin.GetDocument( _T("CHomeConnectionBox") );
	if ( pXML == NULL ) return;

	SetCaption( pXML->GetAttributeValue( _T("title"), _T("Connection") ) );
	HICON hIcon = CoolInterface.ExtractIcon( IDR_NEIGHBOURSFRAME, Settings.General.LanguageRTL );
	if ( hIcon )
		SetIcon( hIcon );

	m_pDocument = new CRichDocument();

	CElementMap pMap;
	if ( ! m_pDocument->LoadXML( pXML, &pMap ) ) return;

	pMap.Lookup( _T("ConnectedHours"), m_pdConnectedHours );
	pMap.Lookup( _T("ConnectedMinutes"), m_pdConnectedMinutes );

	pMap.Lookup( _T("G1Peers"), m_pdCount[PROTOCOL_G1][ntNode] );
	pMap.Lookup( _T("G1Hubs"), m_pdCount[PROTOCOL_G1][ntHub] );
	pMap.Lookup( _T("G1Leaves"), m_pdCount[PROTOCOL_G1][ntLeaf] );

	pMap.Lookup( _T("G2Peers"), m_pdCount[PROTOCOL_G2][ntNode] );
	pMap.Lookup( _T("G2Hubs"), m_pdCount[PROTOCOL_G2][ntHub] );
	pMap.Lookup( _T("G2Leaves"), m_pdCount[PROTOCOL_G2][ntLeaf] );

	pMap.Lookup( _T("EDServers"), m_pdCount[PROTOCOL_ED2K][ntHub] );

	pMap.Lookup( _T("DCHubs"), m_pdCount[PROTOCOL_DC][ntHub] );

	for ( int nP = PROTOCOL_NULL ; nP < PROTOCOL_LAST ; ++nP )
	{
		for ( int nT = ntNode ; nT <= ntLeaf ; nT++ )
		{
			if ( m_pdCount[ nP ][ nT ] != NULL )
			{
				m_sCount[ nP ][ nT ] = m_pdCount[ nP ][ nT ]->m_sText;
			}
		}
	}

	SetDocument( m_pDocument );

	Update();
}

void CHomeConnectionBox::Update()
{
	if ( m_pDocument == NULL ) return;

	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;

	int nCount[ PROTOCOL_LAST ][ ntLeaf + 2 ] = {};
	int nTotal = 0;
	CString str;

	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );

		if ( pNeighbour->m_nState >= nrsConnected )
		{
			nCount[ pNeighbour->m_nProtocol ][ pNeighbour->m_nNodeType+1 ] ++;
			nTotal ++;
		}
		else
		{
			nCount[ pNeighbour->m_nProtocol ][ ntNode ] ++;
		}
	}

	nCount[ PROTOCOL_G1 ][ ntNode ] += nCount[ PROTOCOL_NULL ][ ntNode ];
	nCount[ PROTOCOL_G2 ][ ntNode ] += nCount[ PROTOCOL_NULL ][ ntNode ];

	BOOL bConnected = Network.IsConnected();
	m_pDocument->ShowGroup( 1, ! bConnected );
	m_pDocument->ShowGroup( 2, bConnected );

	static const bool* pEnable[ PROTOCOL_LAST ] =
	{
		NULL,								// PROTOCOL_NULL
		&Settings.Gnutella1.EnableToday,	// PROTOCOL_G1
		&Settings.Gnutella2.EnableToday,	// PROTOCOL_G2
		&Settings.eDonkey.EnableToday,		// PROTOCOL_ED2K
		NULL,								// PROTOCOL_HTTP
		NULL,								// PROTOCOL_FTP
		&Settings.BitTorrent.EnableToday,	// PROTOCOL_BT
		NULL,								// PROTOCOL_KAD
		&Settings.DC.EnableToday			// PROTOCOL_DC
	};

	BOOL bDetail = Settings.General.GUIMode != GUI_BASIC;

	for ( int nProtocol = PROTOCOL_NULL ; nProtocol < PROTOCOL_LAST ; ++nProtocol )
	{
		int nBase = nProtocol * 10;

		if ( ! pEnable[ nProtocol ] )
			continue;

		if ( bConnected && *pEnable[ nProtocol ] )
		{
			m_pDocument->ShowGroup( nBase, TRUE );

			if ( nCount[ nProtocol ][ ntLeaf + 1 ] )
			{
				if ( m_pdCount[ nProtocol ][ ntLeaf ] && bDetail )
				{
					str.Format( m_sCount[ nProtocol ][ ntLeaf ], nCount[ nProtocol ][ ntLeaf+1 ] );
					m_pdCount[ nProtocol ][ ntLeaf ]->SetText( str );

					m_pDocument->ShowGroup( nBase + 3, FALSE );
					m_pDocument->ShowGroup( nBase + 4, FALSE );
					m_pDocument->ShowGroup( nBase + 5, TRUE );
				}
				else
				{
					m_pDocument->ShowGroup( nBase + 3, TRUE );
					m_pDocument->ShowGroup( nBase + 4, FALSE );
					m_pDocument->ShowGroup( nBase + 5, FALSE );
				}
			}
			else if ( nCount[ nProtocol ][ ntHub+1 ] )
			{
				if ( m_pdCount[ nProtocol ][ ntHub ] && bDetail )
				{
					str.Format( m_sCount[ nProtocol ][ ntHub ], nCount[ nProtocol ][ ntHub+1 ] );
					m_pdCount[ nProtocol ][ ntHub ]->SetText( str );

					m_pDocument->ShowGroup( nBase + 3, FALSE );
					m_pDocument->ShowGroup( nBase + 4, TRUE );
					m_pDocument->ShowGroup( nBase + 5, FALSE );
				}
				else
				{
					m_pDocument->ShowGroup( nBase + 3, TRUE );
					m_pDocument->ShowGroup( nBase + 4, FALSE );
					m_pDocument->ShowGroup( nBase + 5, FALSE );
				}
			}
			else if ( nCount[ nProtocol ][ ntNode+1 ] )
			{
				m_pDocument->ShowGroup( nBase + 3, TRUE );
				m_pDocument->ShowGroup( nBase + 4, FALSE );
				m_pDocument->ShowGroup( nBase + 5, FALSE );
			}
			else
			{
				m_pDocument->ShowGroup( nBase + 3, FALSE );
				m_pDocument->ShowGroup( nBase + 4, FALSE );
				m_pDocument->ShowGroup( nBase + 5, FALSE );
			}

			if ( nCount[ nProtocol ][ ntNode+1 ] ||
				 nCount[ nProtocol ][ ntHub+1 ] ||
				 nCount[ nProtocol ][ ntLeaf+1 ] )
			{
				m_pDocument->ShowGroup( nBase + 1, FALSE );
				m_pDocument->ShowGroup( nBase + 2, FALSE );
			}
			else
			{
				m_pDocument->ShowGroup( nBase + 1, nCount[ nProtocol ][ ntNode ] == 0 );
				m_pDocument->ShowGroup( nBase + 2, nCount[ nProtocol ][ ntNode ] != 0 );
			}
		}
		else
		{
			m_pDocument->ShowGroupRange( nBase, nBase + 9, FALSE );
		}
	}

	str.Format( _T("%I64u "), Statistics.Today.Timer.Connected / ( 60 * 60 ) );
	if ( m_pdConnectedHours ) m_pdConnectedHours->SetText( str );
	str.Format( _T("%I64u "), ( Statistics.Today.Timer.Connected / 60 ) % 60 );
	if ( m_pdConnectedMinutes ) m_pdConnectedMinutes->SetText( str );

	CRichTaskBox::Update();
}

/////////////////////////////////////////////////////////////////////////////
// CHomeTorrentsBox construction

CHomeTorrentsBox::CHomeTorrentsBox() :
	m_pdTorrentsNone( NULL ),
	m_pdTorrentsOne( NULL ),
	m_pdTorrentsMany( NULL ),
	m_pdReseedTorrent( NULL )
{
	SetPrimary();
}

CHomeTorrentsBox::~CHomeTorrentsBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHomeTorrentsBox message handlers

void CHomeTorrentsBox::OnSkinChange()
{
	if ( m_pDocument ) delete m_pDocument;
	m_pDocument = NULL;
	m_pdTorrentsNone = m_pdTorrentsOne = m_pdTorrentsMany = m_pdReseedTorrent = NULL;

	SetCaptionmark( _T("CHomeTorrentsBox.Caption") );

	CXMLElement* pXML = Skin.GetDocument( _T("CHomeTorrentsBox") );
	if ( pXML == NULL ) return;

	SetCaption( pXML->GetAttributeValue( _T("title"), _T("Torrents") ) );
	SetIcon( CoolInterface.ExtractIcon( ID_NETWORK_BT ) );

	m_pDocument = new CRichDocument();

	CElementMap pMap;
	if ( ! m_pDocument->LoadXML( pXML, &pMap ) ) return;

	pMap.Lookup( _T("TorrentsNone"), m_pdTorrentsNone );
	pMap.Lookup( _T("TorrentsOne"), m_pdTorrentsOne );
	pMap.Lookup( _T("TorrentsMany"), m_pdTorrentsMany );

	if ( m_pdTorrentsMany ) m_sTorrentsMany = m_pdTorrentsMany->m_sText;

	pMap.Lookup( _T("ReseedTorrent"), m_pdReseedTorrent );
	if ( m_pdReseedTorrent ) m_sReseedTorrent = m_pdReseedTorrent->m_sText;

	GetView().SetDocument( m_pDocument );
	Update();
}

void CHomeTorrentsBox::Update()
{
	if ( m_pDocument == NULL ) return;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;

	// Torrent Count
	int nCount = Downloads.GetSeedCount();

	if ( nCount > 1 )
	{
		if ( m_pdTorrentsMany )
		{
			CString str;
			str.Format( m_sTorrentsMany, nCount );
			m_pdTorrentsMany->SetText( str );
			m_pdTorrentsMany->Show( TRUE );
		}
		if ( m_pdTorrentsOne ) m_pdTorrentsOne->Show( FALSE );
		if ( m_pdTorrentsNone ) m_pdTorrentsNone->Show( FALSE );
	}
	else if ( nCount == 1 )
	{
		if ( m_pdTorrentsMany ) m_pdTorrentsMany->Show( FALSE );
		if ( m_pdTorrentsOne ) m_pdTorrentsOne->Show( TRUE );
		if ( m_pdTorrentsNone ) m_pdTorrentsNone->Show( FALSE );
	}
	else
	{
		if ( m_pdTorrentsMany ) m_pdTorrentsMany->Show( FALSE );
		if ( m_pdTorrentsOne ) m_pdTorrentsOne->Show( FALSE );
		if ( m_pdTorrentsNone ) m_pdTorrentsNone->Show( TRUE );
	}

	// Seed torrent
	m_pDocument->ShowGroup( 1, FALSE );
	m_pDocument->ShowGroup( 2, TRUE );

	// Re-seed last torrent option
	if ( m_pdReseedTorrent )
	{
		if ( ( LibraryHistory.LastSeededTorrent.m_sName.IsEmpty() ) ||
			 ( LibraryHistory.LastSeededTorrent.m_sPath.IsEmpty() ) ||
			 ( Downloads.FindByBTH( LibraryHistory.LastSeededTorrent.m_oBTH ) != NULL ) )
		{
			// No 'Last seeded' torrent, or it's already seeding
			m_pdReseedTorrent->Show( FALSE );
		}
		else
		{
			// We could re-seed this torrent...
			CString str;
			str.Format( m_sReseedTorrent, (LPCTSTR)LibraryHistory.LastSeededTorrent.m_sName );
			m_pdReseedTorrent->SetText( str );
			m_pdReseedTorrent->Show( TRUE );
			// Change 'seed' to 'seed another'
			m_pDocument->ShowGroup( 1, TRUE );
			m_pDocument->ShowGroup( 2, FALSE );
		}
	}

	CRichTaskBox::Update();
}



