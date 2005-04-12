//
// CtrlLibraryMetaPanel.cpp
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
#include "LibraryFolders.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "AlbumFolder.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "ImageServices.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryMetaPanel.h"
#include "CtrlLibraryTree.h"
#include "DlgFilePropertiesSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLibraryMetaPanel, CLibraryPanel)

BEGIN_MESSAGE_MAP(CLibraryMetaPanel, CLibraryPanel)
	//{{AFX_MSG_MAP(CLibraryMetaPanel)
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel construction

CLibraryMetaPanel::CLibraryMetaPanel()
{
	m_nThumbSize	= 96;
	m_crLight		=	CCoolInterface::CalculateColour(
						CoolInterface.m_crTipBack, RGB( 255, 255, 255 ), 128 );
}

CLibraryMetaPanel::~CLibraryMetaPanel()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel operations

BOOL CLibraryMetaPanel::CheckAvailable(CLibraryTreeItem* pFolders, CLibraryList* pObjects)
{
	m_bAvailable = FALSE;
	
	if ( pFolders != NULL )
	{
		m_bAvailable = TRUE;
		
		if ( pFolders->m_pSelNext == NULL && pFolders->m_pVirtual != NULL )
		{
			if ( pFolders->m_pVirtual->m_bCollSHA1 &&
				 pFolders->m_pVirtual->GetBestView().Find( _T("Collection") ) > 0 )
				 m_bAvailable = FALSE;
			if ( pFolders->m_pVirtual->GetFolderCount() > 0 ) m_bAvailable = FALSE;
		}
	}
	
	return m_bAvailable;
}

void CLibraryMetaPanel::Update()
{
	CSingleLock pLock1( &Library.m_pSection, TRUE );
	CSingleLock pLock2( &m_pSection, TRUE );
	
	CLibraryList* pSel = GetViewSelection();
	m_nSelected = pSel->GetCount();
	
	CLibraryFile* pFirst = m_nSelected ? Library.LookupFile( pSel->GetHead() ) : NULL;
	if ( pFirst == NULL ) m_nSelected = 0;
	
	m_nIcon32 = m_nIcon48 = -1;
	
	if ( m_nSelected == 1 )
	{
		m_nIndex	= pFirst->m_nIndex;
		m_sName		= pFirst->m_sName;
		m_sPath		= pFirst->GetPath();
		if ( pFirst->m_pFolder != NULL ) m_sFolder = pFirst->m_pFolder->m_sPath;
		m_sSize		= Settings.SmartVolume( pFirst->GetSize(), FALSE );
		m_sType		= ShellIcons.GetTypeString( m_sName );
		m_nIcon32	= ShellIcons.Get( m_sName, 32 );
		m_nIcon48	= ShellIcons.Get( m_sName, 48 );
		m_nRating	= pFirst->m_nRating;
	}
	else if ( m_nSelected > 1 )
	{
		CString strFormat;
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
		
		m_sSize = Settings.SmartVolume( nSize, TRUE );
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
	
	m_pMetadata.Setup( m_pSchema );
	
	if ( m_pSchema != NULL )
	{
		for ( POSITION pos = pSel->GetHeadPosition() ; pos ; )
		{
			if ( CLibraryFile* pFile = Library.LookupFile( pSel->GetNext( pos ) ) )
			{
				if ( pFile->m_pMetadata != NULL &&
					 m_pSchema->Equals( pFile->m_pSchema ) )
				{
					m_pMetadata.Combine( pFile->m_pMetadata );
				}
			}
		}
	}
	
	m_pMetadata.CreateLinks();
	m_pMetadata.Clean( 4096 );
	
	CClientDC dc( this );
	SCROLLINFO pInfo;
	CRect rc;
	
	GetClientRect( &rc );
	
	int nThumbSize = rc.Height() - 16;
	nThumbSize = max( nThumbSize, 64 );
	nThumbSize = min( nThumbSize, 128 );
	
	int nHeight = 54 + m_pMetadata.Layout( &dc, rc.Width() - 24 - nThumbSize );
	
	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;
	pInfo.nMin		= 0;
	pInfo.nMax		= nHeight;
	pInfo.nPage		= rc.Height();
	pInfo.nPos		= GetScrollPos( SB_VERT );
	pInfo.nPos		= max( 0, min( pInfo.nPos, pInfo.nMax - (int)pInfo.nPage + 1 ) );
	
	SetScrollInfo( SB_VERT, &pInfo, TRUE );
	
	if ( m_bmThumb.m_hObject != NULL && m_sThumb != m_sPath ) m_bmThumb.DeleteObject();
	
	pLock2.Unlock();
	pLock1.Unlock();
	
	if ( m_sPath.GetLength() && m_bmThumb.m_hObject == NULL )
	{
		if ( m_bThread == FALSE )
		{
			m_bThread = TRUE;
			CWinThread* pThread = AfxBeginThread( ThreadStart, this, THREAD_PRIORITY_IDLE );
			m_hThread = pThread->m_hThread;
		}
		
		m_pWakeup.SetEvent();
	}
	
	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel create and destroy

int CLibraryMetaPanel::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CLibraryPanel::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	m_hThread = NULL;
	m_bThread = FALSE;
	
	return 0;
}

void CLibraryMetaPanel::OnDestroy() 
{
	m_bThread = FALSE;
	m_pWakeup.SetEvent();
	
    int nAttempt = 5;
	for ( ; nAttempt > 0 ; nAttempt-- )
	{
		DWORD nCode;
		
		if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
		if ( nCode != STILL_ACTIVE ) break;
		
		Sleep( 100 );
	}
	
	if ( nAttempt == 0 )
	{
		TerminateThread( m_hThread, 0 );
		theApp.Message( MSG_DEBUG, _T("WARNING: Terminating CLibraryMetaPanel thread.") );
		Sleep( 100 );
	}
	
	m_hThread = NULL;
	
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
		dc.ExtTextOut( pt.x, pt.y, ETO_OPAQUE, &rcClient, str, NULL );
		dc.SelectObject( pOldFont );
		return;
	}
	
	CRect rcWork;
	DrawThumbnail( &dc, rcClient, rcWork );
	
	dc.SetViewportOrg( 0, -GetScrollPos( SB_VERT ) );
	
	dc.SetBkColor( CoolInterface.m_crWindow );
	
	dc.SelectObject( &CoolInterface.m_fntCaption );
	DrawText( &dc, rcWork.left, rcWork.top, m_sName );
	
	if ( m_nRating > 1 )
	{
		CPoint ptStar( rcWork.right - 3, rcWork.top - 2 );
		m_rcRating.SetRectEmpty();
		
		for ( int nRating = m_nRating - 1 ; nRating ; nRating-- )
		{
			ptStar.x -= 16;
			ShellIcons.Draw( &dc, SHI_STAR, 16, ptStar.x, ptStar.y, CoolInterface.m_crWindow );
			m_rcRating.UnionRect( &m_rcRating, CRect( ptStar.x, ptStar.y, ptStar.x + 16, ptStar.y + 16 ) );
		}
	}
	else if ( m_nRating == 1 )
	{
		CPoint ptStar( rcWork.right - 3, rcWork.top - 2 );
		m_rcRating.SetRectEmpty();
		ptStar.x -= 16;
		ShellIcons.Draw( &dc, SHI_FAKE, 16, ptStar.x, ptStar.y, CoolInterface.m_crWindow );
		m_rcRating.UnionRect( &m_rcRating, CRect( ptStar.x, ptStar.y, ptStar.x + 16, ptStar.y + 16 ) );
	}
	else
	{
		dc.SelectObject( &CoolInterface.m_fntUnder );
		dc.SetTextColor( RGB( 0, 0, 255 ) );
		LoadString( str, IDS_LIBPANEL_RATE_FILE );
		CSize szText = dc.GetTextExtent( str );
		DrawText( &dc, rcWork.right - szText.cx, rcWork.top, str, &m_rcRating );
	}
	
	rcWork.top += 20;
	dc.FillSolidRect( rcWork.left, rcWork.top, rcWork.Width(), 1, CoolInterface.m_crMargin );
	dc.ExcludeClipRect( rcWork.left, rcWork.top, rcWork.right, rcWork.top + 1 );
	dc.SetBkColor( CoolInterface.m_crWindow );
	dc.SetTextColor( CoolInterface.m_crText );
	rcWork.top += 4;
	
	dc.SelectObject( &CoolInterface.m_fntBold );
	LoadString( str, IDS_TIP_LOCATION );
	DrawText( &dc, rcWork.left, rcWork.top, str + ':' );
	LoadString( str, IDS_TIP_SIZE );
	DrawText( &dc, rcWork.right - 125, rcWork.top, str + ':' );
	dc.SelectObject( &CoolInterface.m_fntNormal );
	DrawText( &dc, rcWork.right - 60, rcWork.top, m_sSize );
	
	if ( m_sFolder.Find( '\\' ) >= 0 )
	{
		dc.SelectObject( &CoolInterface.m_fntUnder );
		dc.SetTextColor( RGB( 0, 0, 255 ) );
		str = m_sFolder;
		long nTextLength = dc.GetTextExtent( str + _T('\x2026') ).cx;
		const long nLimit = rcWork.Width() - 125 - 68 - 10;
		if ( nTextLength > nLimit && nLimit > 0 )
		{
			while ( nTextLength > nLimit )
			{
				str = str.Left( str.GetLength() - 1 );
				nTextLength = dc.GetTextExtent( str + _T('\x2026') ).cx;
			}
			str += _T('\x2026');
		}
		else str.Empty();
	}
	else str.Empty();

	DrawText( &dc, rcWork.left + 68, rcWork.top, str, &m_rcFolder );
	if ( m_sFolder.Find( '\\' ) < 0 ) m_rcFolder.SetRect( 0, 0, 0, 0 );
	rcWork.top += 18;
	
	m_pMetadata.Paint( &dc, &rcWork );
	
	dc.SetViewportOrg( 0, 0 );
	
	dc.SelectObject( pOldFont );
	dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );
}

void CLibraryMetaPanel::DrawText(CDC* pDC, int nX, int nY, LPCTSTR pszText, RECT* pRect)
{
	CSize sz = pDC->GetTextExtent( pszText, _tcslen( pszText ) );
	CRect rc( nX - 2, nY - 2, nX + sz.cx + 2, nY + sz.cy + 2 );
	
	pDC->ExtTextOut( nX, nY, ETO_CLIPPED|ETO_OPAQUE, &rc, pszText, _tcslen( pszText ), NULL );
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

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel scrolling

void CLibraryMetaPanel::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SCROLLINFO pScroll;
	
	ZeroMemory( &pScroll, sizeof(pScroll) );
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
		pScroll.nPos = pScroll.nTrackPos;
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
	else if ( m_pMetadata.HitTest( point, TRUE ) )
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
		if ( CLibraryFolder* pFolder = LibraryFolders.GetFolder( m_sFolder ) )
		{
			CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
			ASSERT_KINDOF(CLibraryFrame, pFrame );
			pFrame->Display( pFolder );
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
	else if ( CMetaItem* pItem = m_pMetadata.HitTest( point, TRUE ) )
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

/////////////////////////////////////////////////////////////////////////////
// CLibraryMetaPanel thread run

UINT CLibraryMetaPanel::ThreadStart(LPVOID pParam)
{
	CLibraryMetaPanel* pPanel = (CLibraryMetaPanel*)pParam;
	pPanel->OnRun();
	return 0;
}

void CLibraryMetaPanel::OnRun()
{
	CImageServices pServices;

	while ( m_bThread )
	{
		WaitForSingleObject( m_pWakeup, INFINITE );
		if ( ! m_bThread ) break;
		
		m_pSection.Lock();
		CString strPath = m_sPath;
		m_pSection.Unlock();

		CImageFile pFile( &pServices );

		if ( pFile.LoadFromFile( strPath, FALSE, TRUE ) && pFile.EnsureRGB() )
		{
			int nSize = m_nThumbSize * pFile.m_nWidth / pFile.m_nHeight;
			
			if ( nSize > m_nThumbSize )
			{
				nSize = m_nThumbSize * pFile.m_nHeight / pFile.m_nWidth;
				pFile.Resample( m_nThumbSize, nSize );
			}
			else
			{
				pFile.Resample( nSize, m_nThumbSize );
			}

			m_pSection.Lock();

			if ( m_bmThumb.m_hObject ) m_bmThumb.DeleteObject();

			if ( m_sPath == strPath )
			{
				m_sThumb = m_sPath;
				m_bmThumb.Attach( pFile.CreateBitmap() );
				m_szThumb.cx = pFile.m_nWidth;
				m_szThumb.cy = pFile.m_nHeight;
				Invalidate();
			}

			m_pSection.Unlock();
		}
	}

	m_bThread = FALSE;
}

