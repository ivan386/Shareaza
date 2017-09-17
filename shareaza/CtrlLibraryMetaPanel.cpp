//
// CtrlLibraryMetaPanel.cpp
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
#include "Library.h"
#include "LibraryFolders.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "AlbumFolder.h"
#include "Schema.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "ThumbCache.h"
#include "ImageFile.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryMetaPanel.h"
#include "FileExecutor.h"
#include "DlgFilePropertiesSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLibraryMetaPanel, CPanelCtrl)

BEGIN_MESSAGE_MAP(CLibraryMetaPanel, CPanelCtrl)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel construction

CLibraryMetaPanel::CLibraryMetaPanel()
	: m_pMetadata	( new CMetaList() )
	, m_pServiceData( NULL )
	, m_bForceUpdate( FALSE )
	, m_bRedraw		( TRUE )
{
	m_rcFolder.SetRectEmpty();
}

CLibraryMetaPanel::~CLibraryMetaPanel()
{
	delete m_pMetadata;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel operations

CLibraryList* CLibraryMetaPanel::GetViewSelection() const
{
	if ( ! m_hWnd )
		return NULL;

	CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
	if ( ! pFrame )
		return NULL;

	ASSERT_KINDOF(CLibraryFrame, pFrame);
	return pFrame->GetViewSelection();
}

void CLibraryMetaPanel::Update()
{
	CSingleLock pLock1( &Library.m_pSection, TRUE );
	CSingleLock pLock2( &m_pSection, TRUE );

	CLibraryListPtr pSel( GetViewSelection() );
	m_nSelected = pSel ? static_cast< int >( pSel->GetCount() ) : 0;

	// Show info for library files only
	CLibraryFile* pFirst = NULL;
	if ( m_nSelected )
	{
		const CLibraryListItem& pItem = pSel->GetHead();
		if ( pItem.Type == CLibraryListItem::LibraryFile )
			pFirst = Library.LookupFile( pItem );
		if ( pFirst == NULL ) m_nSelected = 0;
	}

	m_nIcon32 = m_nIcon48 = -1;

	if ( m_nSelected == 1 )
	{
		m_nIndex	= pFirst->m_nIndex;
		m_sName		= pFirst->m_sName;
		m_sPath		= pFirst->GetPath();
		m_sFolder	= pFirst->GetFolder();
		m_sSize		= Settings.SmartVolume( pFirst->GetSize() );
		m_sType		= ShellIcons.GetTypeString( m_sName );
		m_nIcon32	= ShellIcons.Get( pFirst->GetPath(), 32 );
		m_nIcon48	= ShellIcons.Get( pFirst->GetPath(), 48 );
		m_nRating	= pFirst->m_nRating;
	}
	else if ( m_nSelected > 1 )
	{
		CString strFormat;
		LoadString( strFormat, IDS_LIBPANEL_MULTIPLE_FILES );
		m_sName.Format( strFormat, m_nSelected );
		QWORD nSize = 0;

		m_sFolder	= pFirst->GetFolder();
		m_nIcon32	= ShellIcons.Get( pFirst->GetPath(), 32 );
		m_nIcon48	= ShellIcons.Get( pFirst->GetPath(), 48 );
		m_nRating	= 0;

		for ( POSITION pos = pSel->GetHeadPosition() ; pos ; )
		{
			CLibraryFile* pFile = Library.LookupFile( pSel->GetNext( pos ) );
			if ( pFile == NULL ) continue;

			nSize += pFile->GetSize() / 1024;

			if ( pFile->IsAvailable() && pFile->GetFolder().CompareNoCase( m_sFolder ) )
			{
				LoadString( m_sFolder, IDS_LIBPANEL_MULTIPLE_FOLDERS );
			}

			int nIcon = ShellIcons.Get( pFile->GetPath(), 48 );
			if ( nIcon != m_nIcon48 ) m_nIcon48 = -1;
			nIcon = ShellIcons.Get( pFile->GetPath(), 32 );
			if ( nIcon != m_nIcon32 ) m_nIcon32 = -1;
		}

		m_sSize = Settings.SmartVolume( nSize );
		m_sPath.Empty();
		m_sType.Empty();
	}

	m_pSchema = NULL;

	if ( pSel )
	{
		for ( POSITION pos = pSel->GetHeadPosition() ; pos ; )
		{
			const CLibraryListItem& pItem = pSel->GetNext( pos );
			if ( pItem.Type != CLibraryListItem::LibraryFile ) continue;
			CLibraryFile* pFile = Library.LookupFile( pItem );
			if ( pFile == NULL ) continue;
			m_pSchema = pFile->m_pSchema;
			if ( m_pSchema ) break;
		}
	}

	if ( m_pServiceData )
	{
		m_pMetadata->Setup( m_pServiceData );
	}
	else
	{
		m_pMetadata->Setup( m_pSchema );

		if ( m_pSchema && pSel )
		{
			for ( POSITION pos = pSel->GetHeadPosition() ; pos ; )
			{
				const CLibraryListItem& pItem = pSel->GetNext( pos );
				if ( pItem.Type != CLibraryListItem::LibraryFile ) continue;
				if ( CLibraryFile* pFile = Library.LookupFile( pItem ) )
				{
					if ( pFile->m_pMetadata != NULL &&
						m_pSchema->Equals( pFile->m_pSchema ) )
					{
						m_pMetadata->Combine( pFile->m_pMetadata );
					}
				}
			}
		}
	}

	m_pMetadata->CreateLinks();
	m_pMetadata->Clean( 4096 );

	CClientDC dc( this );
	if ( Settings.General.LanguageRTL )
		SetLayout( dc.m_hDC, LAYOUT_BITMAPORIENTATIONPRESERVED );
	SCROLLINFO pInfo;
	CRect rc;

	GetClientRect( &rc );

	int nThumbSize = min( max( rc.Height() - 16, 64 ), (int)Settings.Library.ThumbSize );

	int nHeight = 54 + m_pMetadata->Layout( &dc, rc.Width() - 24 - nThumbSize );

	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;
	pInfo.nMin		= 0;
	pInfo.nMax		= nHeight;
	pInfo.nPage		= rc.Height();
	pInfo.nPos		= GetScrollPos( SB_VERT );
	pInfo.nPos		= max( 0, min( pInfo.nPos, pInfo.nMax - (int)pInfo.nPage + 1 ) );

	SetScrollInfo( SB_VERT, &pInfo, TRUE );

	if ( m_bForceUpdate || ( m_sThumb != m_sPath ) )
	{
		m_bForceUpdate = FALSE;

		if ( m_bmThumb.m_hObject )
			m_bmThumb.DeleteObject();

		if ( ! IsThreadAlive() )
		{
			BeginThread( "CtrlLibraryMetaPanel" );
		}
	}

	m_bRedraw = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel create and destroy

int CLibraryMetaPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelCtrl::OnCreate( lpCreateStruct ) == -1 ) return -1;

	SetTimer( 1, 500, NULL );

	return 0;
}

void CLibraryMetaPanel::OnDestroy()
{
	KillTimer( 1 );

	CloseThread();

	CPanelCtrl::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel painting

void CLibraryMetaPanel::OnPaint()
{
	CSingleLock pLock( &m_pSection, TRUE );
	CPaintDC dc( this );
	CRect rcClient;
	CString str;
	DWORD dwFlags = ( Settings.General.LanguageRTL ? ETO_RTLREADING : 0 );

	GetClientRect( &rcClient );

	CFont* pOldFont = dc.GetCurrentFont();
	dc.SetTextColor( CoolInterface.m_crText );
	dc.SetBkColor( CoolInterface.m_crWindow );
	dc.SetBkMode( OPAQUE );

	if ( m_nSelected == 0 )
	{
		dc.SelectObject( &CoolInterface.m_fntNormal );
		LoadString( str, IDS_LIBPANEL_NO_SELECTION );
		CSize sz = dc.GetTextExtent( str );
		CPoint pt = rcClient.CenterPoint();
		pt.x -= sz.cx / 2; pt.y -= sz.cy / 2;
		dc.ExtTextOut( pt.x, pt.y, ETO_OPAQUE|dwFlags, &rcClient, str, NULL );
		dc.SelectObject( pOldFont );
		return;
	}

	// Draw thumbnail or icon
	int nThumbSize = min( max( rcClient.Height() - 16, 64 ), (int)Settings.Library.ThumbSize );
	CRect rcWork( rcClient.left + 8, rcClient.top + 8,
		rcClient.left + 8 + nThumbSize, rcClient.top + 8 + nThumbSize );
	CoolInterface.DrawThumbnail( &dc, rcWork, TRUE, FALSE, m_bmThumb, m_nIcon48, m_nIcon32,
		IsThreadAlive() ? LoadString( IDS_SEARCH_DETAILS_PREVIEWING ) : CString() );
	rcWork.SetRect( rcWork.right + 8, rcWork.top, rcClient.right - 8, rcClient.bottom );

	dc.SetViewportOrg( 0, -GetScrollPos( SB_VERT ) );

	dc.SetBkColor( CoolInterface.m_crWindow );

	if ( m_nRating > 1 )
	{
		CPoint ptStar( rcWork.right - 3, rcWork.top - 2 );
		m_rcRating.SetRectEmpty();

		for ( int nRating = m_nRating - 1 ; nRating ; nRating-- )
		{
			ptStar.x -= 16;
			CoolInterface.Draw( &dc, IDI_STAR, 16, ptStar.x, ptStar.y, CoolInterface.m_crWindow );
			dc.ExcludeClipRect( ptStar.x, ptStar.y, ptStar.x + 16, ptStar.y + 16 );
			m_rcRating.UnionRect( &m_rcRating, CRect( ptStar.x, ptStar.y, ptStar.x + 16, ptStar.y + 16 ) );
		}
	}
	else if ( m_nRating == 1 )
	{
		CPoint ptStar( rcWork.right - 3, rcWork.top - 2 );
		m_rcRating.SetRectEmpty();
		ptStar.x -= 16;
		CoolInterface.Draw( &dc, IDI_FAKE, 16, ptStar.x, ptStar.y, CoolInterface.m_crWindow );
		dc.ExcludeClipRect( ptStar.x, ptStar.y, ptStar.x + 16, ptStar.y + 16 );
		m_rcRating.UnionRect( &m_rcRating, CRect( ptStar.x, ptStar.y, ptStar.x + 16, ptStar.y + 16 ) );
	}
	else
	{
		m_rcRating.SetRectEmpty();
		dc.SelectObject( &CoolInterface.m_fntUnder );
		dc.SetTextColor( CoolInterface.m_crTextLink );
		LoadString( str, IDS_LIBPANEL_RATE_FILE );
		CSize szText = dc.GetTextExtent( str );
		DrawText( &dc, rcWork.right - szText.cx, rcWork.top, str, &m_rcRating );
	}

	dc.SelectObject( &CoolInterface.m_fntCaption );
	dc.SetTextColor( CoolInterface.m_crText );
	DrawText( &dc, rcWork.left, rcWork.top, m_sName, NULL, rcWork.Width() - m_rcRating.Width() - 4 );

	rcWork.top += 20;
	dc.FillSolidRect( rcWork.left, rcWork.top, rcWork.Width(), 1, CoolInterface.m_crMargin );
	dc.ExcludeClipRect( rcWork.left, rcWork.top, rcWork.right, rcWork.top + 1 );
	dc.SetBkColor( CoolInterface.m_crWindow );
	dc.SetTextColor( CoolInterface.m_crText );
	rcWork.top += 4;

	dc.SelectObject( &CoolInterface.m_fntBold );
	LoadString( str, IDS_TIP_LOCATION );
	if ( Settings.General.LanguageRTL )
		DrawText( &dc, rcWork.left, rcWork.top, ':' + str );
	else
		DrawText( &dc, rcWork.left, rcWork.top, str + ':' );
	LoadString( str, IDS_TIP_SIZE );
	DrawText( &dc, rcWork.right - 125, rcWork.top, str + ':' );
	dc.SelectObject( &CoolInterface.m_fntNormal );
	DrawText( &dc, rcWork.right - 60, rcWork.top, m_sSize );

	if ( m_sFolder.Find( '\\' ) >= 0 )
	{
		dc.SelectObject( &CoolInterface.m_fntUnder );
		dc.SetTextColor( CoolInterface.m_crTextLink );
		str = m_sFolder;
		long nTextLength = dc.GetTextExtent( str + _T('\x2026') ).cx;
		const long nLimit = rcWork.Width() - 125 - 68 - 10;
		if ( nTextLength > nLimit && nLimit > 0 )
		{
			while ( nTextLength > nLimit )
			{
				if ( str.IsEmpty() ) break;
				str = str.Left( str.GetLength() - 1 );
				nTextLength = dc.GetTextExtent( str + _T('\x2026') ).cx;
			}
			str += _T('\x2026');
		}
		else if ( nLimit <= 0 ) str.Empty();
	}
	else str.Empty();

	DrawText( &dc, rcWork.left + 68, rcWork.top, str, &m_rcFolder );
	if ( m_sFolder.Find( '\\' ) < 0 ) m_rcFolder.SetRectEmpty();
	rcWork.top += 18;

	m_pMetadata->Paint( &dc, &rcWork );

	dc.SetViewportOrg( 0, 0 );

	dc.SelectObject( pOldFont );
	dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );
}

void CLibraryMetaPanel::DrawText(CDC* pDC, int nX, int nY, LPCTSTR pszText, RECT* pRect, int nMaxWidth)
{
	DWORD dwFlags = ( Settings.General.LanguageRTL ? ETO_RTLREADING : 0 );
	CSize sz = pDC->GetTextExtent( pszText, static_cast< int >( _tcslen( pszText ) ) );

	int nWidth = sz.cx;
	if ( nMaxWidth > 0 )
	{
		nWidth = min( sz.cx, (LONG)nMaxWidth );
	}

	CRect rc( nX - 2, nY - 2, nX + nWidth + 2, nY + sz.cy + 2 );

	pDC->ExtTextOut( nX, nY, ETO_CLIPPED|ETO_OPAQUE|dwFlags, &rc, pszText, static_cast< UINT >( _tcslen( pszText ) ), NULL );
	pDC->ExcludeClipRect( &rc );

	if ( pRect != NULL ) CopyMemory( pRect, &rc, sizeof(RECT) );
}

BOOL CLibraryMetaPanel::SetServicePanel(CMetaList* pPanel)
{
	m_pSection.Lock();

	m_pServiceData = pPanel;

	m_pSection.Unlock();

	Update();

	return TRUE;
}

CMetaList* CLibraryMetaPanel::GetServicePanel()
{
	if ( m_pServiceData )
		return m_pServiceData;
	else
		return m_pMetadata;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel linking

BOOL CLibraryMetaPanel::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;

	GetCursorPos( &point );
	ScreenToClient( &point );
	point.y += GetScrollPos( SB_VERT );

	if ( m_nSelected > 0 && m_rcFolder.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}
	else if ( m_nSelected > 0 && m_rcRating.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}
	else if ( m_pMetadata->HitTest( point, TRUE ) )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}

	return CPanelCtrl::OnSetCursor( pWnd, nHitTest, message );
}

void CLibraryMetaPanel::OnLButtonUp(UINT nFlags, CPoint point)
{
	point.y += GetScrollPos( SB_VERT );

	if ( m_nSelected > 0 && m_rcFolder.PtInRect( point ) )
	{
		CQuickLock oLock( Library.m_pSection );

		if ( CLibraryFolder* pFolder = LibraryFolders.GetFolder( m_sFolder ) )
		{
			if ( Settings.Library.ShowVirtual )
			{
				CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
				ASSERT_KINDOF(CLibraryFrame, pFrame );
				pFrame->Display( pFolder );
			}
			else if ( LibraryFolders.CheckFolder( pFolder, TRUE ) )
			{
				ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), NULL,
					m_sFolder, NULL, NULL, SW_SHOWNORMAL );
			}
		}
	}
	else if ( m_nSelected > 0 && m_rcRating.PtInRect( point ) )
	{
		CLibraryListPtr pList( GetViewSelection() );

		if ( pList && pList->GetCount() > 0 )
		{
			CFilePropertiesSheet dlg;
			dlg.Add( pList );
			dlg.DoModal( 2 );
		}
	}
	else if ( CMetaItem* pItem = m_pMetadata->HitTest( point, TRUE ) )
	{
		CQuickLock oLock( Library.m_pSection );

		if ( CAlbumFolder* pFolder = pItem->GetLinkTarget() )
		{
			CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
			ASSERT_KINDOF(CLibraryFrame, pFrame );
			pFrame->Display( pFolder );
		}
	}

	CPanelCtrl::OnLButtonUp( nFlags, point );
}

void CLibraryMetaPanel::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	SetFocus();
	CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
	pFrame->HideDynamicBar();
}

void CLibraryMetaPanel::OnTimer(UINT_PTR nIDEvent)
{
	CPanelCtrl::OnTimer( nIDEvent );

	{
		CQuickLock pLock( m_pSection );
		if ( ! m_bRedraw )
			return;
		m_bRedraw = FALSE;
	}

	Invalidate();
	UpdateWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel thread run

void CLibraryMetaPanel::OnRun()
{
	m_pSection.Lock();

	while ( IsThreadEnabled() )
	{
		CString strPath = m_sPath;

		if ( strPath.IsEmpty() )
			break;

		m_pSection.Unlock();

		CImageFile pFile;
		BOOL bSuccess = CThumbCache::Cache( strPath, &pFile );

		m_pSection.Lock();

		// If nothing changes
		if ( m_sPath == strPath )
		{
			if ( bSuccess )
			{
				if ( m_bmThumb.m_hObject )
					m_bmThumb.DeleteObject();

				m_bmThumb.Attach( pFile.CreateBitmap() );
			}

			break;
		}
	}

	m_bRedraw = TRUE;

	m_pSection.Unlock();
}
