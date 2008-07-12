//
// CtrlLibraryMetaPanel.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
#include "SchemaCache.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "ThumbCache.h"
#include "ImageServices.h"
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

IMPLEMENT_DYNCREATE(CLibraryMetaPanel, CLibraryPanel)

BEGIN_MESSAGE_MAP(CLibraryMetaPanel, CLibraryPanel)
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel construction

CLibraryMetaPanel::CLibraryMetaPanel()
: m_nThumbSize( Settings.Library.ThumbSize )
, m_crLight( CCoolInterface::CalculateColour( CoolInterface.m_crTipBack, RGB( 255, 255, 255 ), 128 ) )
, m_bNewFile( TRUE )
, m_pMetadata( new CMetaPanel() )
, m_pServiceData( NULL )
, m_bExternalData( FALSE )
, m_bDownloadingImage( FALSE )
, m_bForceUpdate( FALSE )
{
	m_rcFolder.SetRectEmpty();

	// Try to get the number of lines to scroll when the mouse wheel is rotated
	if( !SystemParametersInfo ( SPI_GETWHEELSCROLLLINES, 0, &m_nScrollWheelLines, 0) )
	{
		m_nScrollWheelLines = 3;
	}
}

CLibraryMetaPanel::~CLibraryMetaPanel()
{
	if ( m_pMetadata != NULL )
		delete m_pMetadata;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel operations

BOOL CLibraryMetaPanel::CheckAvailable(CLibraryTreeItem* pFolders, CLibraryList* /*pObjects*/)
{
	m_bAvailable = FALSE;
	
	if ( pFolders != NULL )
	{
		ASSERT_VALID( pFolders );

		m_bAvailable = TRUE;
		
		if ( pFolders->m_pSelNext == NULL && pFolders->m_pVirtual != NULL )
		{
			// Do not display meta panel for the collection folder
			if ( pFolders->m_pVirtual->m_oCollSHA1 &&
				 pFolders->m_pVirtual->GetBestView().Find( _T("Collection") ) > 0 ||
				 CheckURI( pFolders->m_pVirtual->m_sSchemaURI, CSchema::uriCollectionsFolder ) )
			{
				 m_bAvailable = FALSE;
				 return FALSE;
			}

			INT_PTR nFileCount = pFolders->m_pVirtual->GetFileCount();
			m_bAvailable = ( nFileCount != 0 );
		}
	}
	
	return m_bAvailable;
}

void CLibraryMetaPanel::Update()
{
	CSingleLock pLock1( &Library.m_pSection, TRUE );
	CSingleLock pLock2( &m_pSection, TRUE );
	
	CLibraryList* pSel = GetViewSelection();
	m_nSelected = static_cast< int >( pSel->GetCount() );
	
	CLibraryFile* pFirst = m_nSelected ? Library.LookupFile( pSel->GetHead() ) : NULL;
	if ( pFirst == NULL ) m_nSelected = 0;
	
	m_nIcon32 = m_nIcon48 = -1;

	if ( m_nSelected == 1 )
	{
		m_nIndex	= pFirst->m_nIndex;
		m_sName		= pFirst->m_sName;
		CString strNewFile( pFirst->GetPath() );
		m_bNewFile = ( m_sPath != strNewFile || pFirst->IsGhost() );
		if ( m_bNewFile ) m_sPath = strNewFile;
		if ( pFirst->m_pFolder != NULL )
			m_sFolder = pFirst->m_pFolder->m_sPath;
		else
			m_sFolder.Empty();
		m_sSize		= Settings.SmartVolume( pFirst->GetSize() );
		m_sType		= ShellIcons.GetTypeString( m_sName );
		m_nIcon32	= ShellIcons.Get( m_sName, 32 );
		m_nIcon48	= ShellIcons.Get( m_sName, 48 );
		m_nRating	= pFirst->m_nRating;
	}
	else if ( m_nSelected > 1 )
	{
		CString strFormat;
		m_bNewFile = TRUE;
		LoadString( strFormat, IDS_LIBPANEL_MULTIPLE_FILES );
		m_sName.Format( strFormat, m_nSelected );
		QWORD nSize = 0;
		
		m_sFolder	= ( pFirst->m_pFolder != NULL ) ? pFirst->m_pFolder->m_sPath : _T("");
		m_nIcon32	= ShellIcons.Get( pFirst->m_sName, 32 );
		m_nIcon48	= ShellIcons.Get( pFirst->m_sName, 48 );
		m_nRating	= 0;
		
		for ( POSITION pos = pSel->GetHeadPosition() ; pos ; )
		{
			CLibraryFile* pFile = Library.LookupFile( pSel->GetNext( pos ) );
			if ( pFile == NULL ) continue;
			
			nSize += pFile->GetSize() / 1024;
			
			if ( pFile->m_pFolder != NULL && pFile->m_pFolder->m_sPath != m_sFolder )
			{
				LoadString( m_sFolder, IDS_LIBPANEL_MULTIPLE_FOLDERS );
			}
			
			int nIcon = ShellIcons.Get( pFile->m_sName, 48 );
			if ( nIcon != m_nIcon48 ) m_nIcon48 = -1;
			nIcon = ShellIcons.Get( pFile->m_sName, 32 );
			if ( nIcon != m_nIcon32 ) m_nIcon32 = -1;
		}
		
		m_sSize = Settings.SmartVolume( nSize );
		m_sPath.Empty();
		m_sType.Empty();
	}
	
	m_pSchema = NULL;
	
	for ( POSITION pos = pSel->GetHeadPosition() ; pos ; )
	{
		CLibraryFile* pFile = Library.LookupFile( pSel->GetNext( pos ) );
		if ( pFile == NULL ) continue;
		m_pSchema = pFile->m_pSchema;
		if ( m_pSchema ) break;
	}
	
	if ( m_bExternalData )
	{
		ASSERT( m_pServiceData );
		m_pMetadata->Setup( m_pServiceData );
	}
	else
	{
		m_pMetadata->Setup( m_pSchema );
	
		if ( m_pSchema != NULL )
		{
			for ( POSITION pos = pSel->GetHeadPosition() ; pos ; )
			{
				if ( CLibraryFile* pFile = Library.LookupFile( pSel->GetNext( pos ) ) )
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
	if ( Settings.General.LanguageRTL ) theApp.m_pfnSetLayout( dc.m_hDC, LAYOUT_BITMAPORIENTATIONPRESERVED );
	SCROLLINFO pInfo;
	CRect rc;
	
	GetClientRect( &rc );
	
	int nThumbSize = rc.Height() - 16;
	nThumbSize = max( nThumbSize, 64 );
	nThumbSize = min( nThumbSize, 128 );
	
	int nHeight = 54 + m_pMetadata->Layout( &dc, rc.Width() - 24 - nThumbSize );
	
	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;
	pInfo.nMin		= 0;
	pInfo.nMax		= nHeight;
	pInfo.nPage		= rc.Height();
	pInfo.nPos		= GetScrollPos( SB_VERT );
	pInfo.nPos		= max( 0, min( pInfo.nPos, pInfo.nMax - (int)pInfo.nPage + 1 ) );
	
	SetScrollInfo( SB_VERT, &pInfo, TRUE );
	
	if ( m_bmThumb.m_hObject != NULL )
	{
		if ( m_sThumb != m_sPath || m_bForceUpdate ) 
			m_bmThumb.DeleteObject();
	}

	pLock2.Unlock();
	pLock1.Unlock();
	
	if ( m_sPath.GetLength() && m_bmThumb.m_hObject == NULL )
	{
		BeginThread( "CtrlLibraryMetaPanel" );
		
		Wakeup();
	}

	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel create and destroy

int CLibraryMetaPanel::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CLibraryPanel::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	return 0;
}

void CLibraryMetaPanel::OnDestroy() 
{
	CloseThread();
	
	CLibraryPanel::OnDestroy();
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
	
	CRect rcWork;
	DrawThumbnail( &dc, rcClient, rcWork );
	
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
		nWidth = min( sz.cx, nMaxWidth );
	}

	CRect rc( nX - 2, nY - 2, nX + nWidth + 2, nY + sz.cy + 2 );
	
	pDC->ExtTextOut( nX, nY, ETO_CLIPPED|ETO_OPAQUE|dwFlags, &rc, pszText, static_cast< UINT >( _tcslen( pszText ) ), NULL );
	pDC->ExcludeClipRect( &rc );
	
	if ( pRect != NULL ) CopyMemory( pRect, &rc, sizeof(RECT) );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel thumbnail painting

void CLibraryMetaPanel::DrawThumbnail(CDC* pDC, CRect& rcClient, CRect& rcWork)
{
	int nThumbSize = rcClient.Height() - 16;
	nThumbSize = max( nThumbSize, 64 );
	nThumbSize = min( nThumbSize, 128 );
	
	CRect rcThumb( rcClient.left + 8, rcClient.top + 8,
		rcClient.left + 8 + nThumbSize, rcClient.top + 8 + nThumbSize );
	
	rcWork.CopyRect( &rcThumb );
	pDC->Draw3dRect( &rcWork, CoolInterface.m_crMargin, CoolInterface.m_crMargin );
	rcWork.DeflateRect( 1, 1 );
	m_nThumbSize = rcWork.Width();
	
	DrawThumbnail( pDC, rcWork );
	pDC->ExcludeClipRect( &rcThumb );
	
	rcWork.SetRect( rcThumb.right + 8, rcThumb.top, rcClient.right - 8, rcClient.bottom );
}

void CLibraryMetaPanel::DrawThumbnail(CDC* pDC, CRect& rcThumb)
{
	if ( m_bmThumb.m_hObject &&
		 ( m_szThumb.cx == m_nThumbSize || m_szThumb.cy == m_nThumbSize ) )
	{
		CDC dcMem;
		dcMem.CreateCompatibleDC( pDC );
		
		CBitmap* pOld = (CBitmap*)dcMem.SelectObject( &m_bmThumb );
		
		CPoint ptImage(	( rcThumb.left + rcThumb.right ) / 2 - m_szThumb.cx / 2,
						( rcThumb.top + rcThumb.bottom ) / 2 - m_szThumb.cy / 2 );
		
		pDC->BitBlt( ptImage.x, ptImage.y, m_szThumb.cx, m_szThumb.cy,
			&dcMem, 0, 0, SRCCOPY );
		pDC->ExcludeClipRect( ptImage.x, ptImage.y,
			ptImage.x + m_szThumb.cx, ptImage.y + m_szThumb.cy );
		
		dcMem.SelectObject( pOld );
		
		pDC->FillSolidRect( &rcThumb, m_crLight );
	}
	else
	{
		CPoint pt(	( rcThumb.left + rcThumb.right ) / 2 - 24,
					( rcThumb.top + rcThumb.bottom ) / 2 - 24 );
		
		if ( m_nIcon48 >= 0 )
		{
			ImageList_DrawEx( ShellIcons.GetHandle( 48 ), m_nIcon48, pDC->GetSafeHdc(),
				pt.x, pt.y, 48, 48, m_crLight, CLR_NONE, ILD_NORMAL );
			pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 48, pt.y + 48 );
		}
		else if ( m_nIcon32 >= 0 )
		{
			pt.x += 8; pt.y += 8;
			ImageList_DrawEx( ShellIcons.GetHandle( 32 ), m_nIcon32, pDC->GetSafeHdc(),
				pt.x, pt.y, 32, 32, m_crLight, CLR_NONE, ILD_NORMAL );
			pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 32, pt.y + 32 );
		}
		
		pDC->FillSolidRect( &rcThumb, m_crLight );
	}
}

BOOL CLibraryMetaPanel::SetServicePanel(CMetaPanel* pPanel)
{
	m_pSection.Lock();

	m_pServiceData = pPanel;
	if ( pPanel == NULL )
	{
		// If it's NULL, first assign the flag and do updates
		// Otherwise, first update and then assign the flag
		m_bExternalData = FALSE;
		m_bForceUpdate = !m_bDownloadingImage;
	}
	else
	{
		m_bForceUpdate = !m_bDownloadingImage && pPanel->m_sThumbnailURL.GetLength() > 0;
	}

	m_pSection.Unlock();

	Update();

	m_pSection.Lock();
	m_bExternalData = pPanel != NULL;

	if ( m_bExternalData )
		m_sThumb = pPanel->m_sThumbnailURL;

	m_pSection.Unlock();

	return TRUE;
}

CMetaPanel* CLibraryMetaPanel::GetServicePanel()
{
	if ( m_pServiceData != NULL && m_bExternalData )
		return m_pServiceData;
	else
		return m_pMetadata;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel scrolling

void CLibraryMetaPanel::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/) 
{
	SCROLLINFO pScroll = {};
	
	pScroll.cbSize	= sizeof(pScroll);
	pScroll.fMask	= SIF_ALL;
	
	GetScrollInfo( SB_VERT, &pScroll );
	
	switch ( nSBCode )
	{
	case SB_TOP:
		pScroll.nPos = 0;
		break;
	case SB_BOTTOM:
		pScroll.nPos = pScroll.nMax - 1;
		break;
	case SB_LINEUP:
		pScroll.nPos -= 8;
		break;
	case SB_LINEDOWN:
		pScroll.nPos += 8;
		break;
	case SB_PAGEUP:
		pScroll.nPos -= pScroll.nPage;
		break;
	case SB_PAGEDOWN:
		pScroll.nPos += pScroll.nPage;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		pScroll.nPos = nPos;
		break;
	}
	
	pScroll.fMask	= SIF_POS;
	pScroll.nPos	= max( 0, min( pScroll.nPos, pScroll.nMax ) );
	
	SetScrollInfo( SB_VERT, &pScroll );
	Invalidate();
}

void CLibraryMetaPanel::OnSize(UINT nType, int cx, int cy) 
{
	CLibraryPanel::OnSize( nType, cx, cy );
	Update();
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
	
	return CLibraryPanel::OnSetCursor( pWnd, nHitTest, message );
}

void CLibraryMetaPanel::OnLButtonUp(UINT nFlags, CPoint point) 
{
	point.y += GetScrollPos( SB_VERT );
	
	if ( m_nSelected > 0 && m_rcFolder.PtInRect( point ) )
	{
		CLibraryFolder* pFolder = LibraryFolders.GetFolder( m_sFolder );
		if ( pFolder )
		{
			if ( Settings.Library.ShowVirtual )
			{
				CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
				ASSERT_KINDOF(CLibraryFrame, pFrame );
				pFrame->Display( pFolder );
			}
			else if ( LibraryFolders.CheckFolder( pFolder, TRUE ) )
			{
				CFileExecutor::Execute( m_sFolder, TRUE );
			}
		}
	}
	else if ( m_nSelected > 0 && m_rcRating.PtInRect( point ) )
	{
		CLibraryList* pList = GetViewSelection();
		
		if ( pList != NULL && pList->GetCount() > 0 )
		{
			CFilePropertiesSheet dlg;
			dlg.Add( pList );
			dlg.DoModal( 2 );
		}
	}
	else if ( CMetaItem* pItem = m_pMetadata->HitTest( point, TRUE ) )
	{
		if ( CAlbumFolder* pFolder = pItem->GetLinkTarget() )
		{
			CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
			ASSERT_KINDOF(CLibraryFrame, pFrame );
			pFrame->Display( pFolder );
		}
	}
	
	CLibraryPanel::OnLButtonUp( nFlags, point );
}

void CLibraryMetaPanel::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	SetFocus();
	CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
	pFrame->HideDynamicBar();
}

BOOL CLibraryMetaPanel::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	OnVScroll( SB_THUMBPOSITION, (int)( GetScrollPos( SB_VERT ) - zDelta / WHEEL_DELTA * m_nScrollWheelLines * 8 ), NULL );
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel thread run

void CLibraryMetaPanel::OnRun()
{
	while ( IsThreadEnabled() )
	{
		Doze();
		if ( ! IsThreadEnabled() ) break;
		if ( ! m_bNewFile && ! m_bForceUpdate || m_bDownloadingImage )
		{
			Exit();
			break;
		}

		m_pSection.Lock();
		CString strPath = m_sPath;
		CString strThumbRes = m_sThumb;
		m_bForceUpdate = FALSE;
		m_pSection.Unlock();

		CImageFile pFile;
		BOOL bSuccess = FALSE;

		if ( m_bExternalData )
		{
			m_bDownloadingImage = TRUE;
			bSuccess = pFile.LoadFromURL( strThumbRes ) && pFile.EnsureRGB();
			m_bDownloadingImage = FALSE;
		}

		if ( ! bSuccess )
			bSuccess = CThumbCache::Cache( strPath, &pFile );

		if ( bSuccess )
		{
			// Resample now to display dimensions
			pFile.FitTo( m_nThumbSize, m_nThumbSize );

			m_pSection.Lock();

			if ( m_sPath == strPath )
			{
				if ( m_bmThumb.m_hObject ) 
					m_bmThumb.DeleteObject();
				m_sThumb = m_sPath;
				m_bmThumb.Attach( pFile.CreateBitmap() );
				m_szThumb.cx = pFile.m_nWidth;
				m_szThumb.cy = pFile.m_nHeight;
				Invalidate();
			}

			m_pSection.Unlock();
		}
	}
}
