//
// CtrlSearchDetailPanel.cpp
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
#include "MatchObjects.h"
#include "QueryHit.h"
#include "Buffer.h"
#include "XML.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "Library.h"
#include "CoolInterface.h"
#include "ImageFile.h"
#include "RichElement.h"
#include "ShellIcons.h"
#include "Emoticons.h"
#include "Skin.h"
#include "CtrlSearchDetailPanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSearchDetailPanel, CPanelCtrl)

BEGIN_MESSAGE_MAP(CSearchDetailPanel, CPanelCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_VSCROLL()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY(RVN_CLICK, IDC_REVIEW_VIEW, &CSearchDetailPanel::OnClickReview)
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchDetailPanel construction

CSearchDetailPanel::CSearchDetailPanel()
	: m_pMatches		( NULL )
	, m_bValid			( FALSE )
	, m_pFile			( NULL )
	, m_nIcon48			( 0 )
	, m_nIcon32			( 0 )
	, m_nRating			( 0 )
	, m_pSchema			( NULL )
	, m_bCanPreview		( FALSE )
	, m_bRunPreview		( FALSE )
	, m_bIsPreviewing	( FALSE )
	, m_bRedraw			( TRUE )
{
	m_rcStatus.SetRectEmpty();
	m_rcThumb.SetRectEmpty();
}

CSearchDetailPanel::~CSearchDetailPanel()
{
	ClearReviews();
}

/////////////////////////////////////////////////////////////////////////////
// CSearchDetailPanel operations

void CSearchDetailPanel::SetFile(CMatchFile* pFile)
{
	CSingleLock pLock( &m_pSection, TRUE );

	CancelPreview();
	ClearReviews();
	m_pMetadata.Clear();

	if ( pFile == NULL || ( ! pFile->IsValid() ) )
	{
		if ( m_bValid )
		{
			m_bValid = FALSE;
			Update();
		}
		return;
	}

	m_pFile		= pFile;
	m_pMatches	= pFile->m_pList;
	m_bValid	= TRUE;
	m_oSHA1		= pFile->m_oSHA1;
	m_sName		= pFile->m_sName;
	m_sSize		= pFile->m_sSize;
	m_nIcon32	= ShellIcons.Get( pFile->m_sName, 32 );
	m_nIcon48	= ShellIcons.Get( pFile->m_sName, 48 );
	m_nRating	= pFile->m_nRated ? pFile->m_nRating / pFile->m_nRated : 0;

	m_pSchema		= pFile->AddHitsToMetadata( m_pMetadata );
	DWORD nSpeed	= pFile->GetTotalHitsSpeed();
	m_bCanPreview	= pFile->AddHitsToPreviewURLs( m_pPreviewURLs );
	pFile->AddHitsToReviews( m_pReviews );

	m_pMetadata.Vote();
	m_pMetadata.CreateLinks();
	m_pMetadata.Clean( 4096 );

	CString strPart;
	m_sStatus.Empty();

	if ( pFile->m_nSources == 1 )
	{
		strPart.Format( LoadString( IDS_SEARCH_DETAILS_SOURCES_ONE ), (LPCTSTR)Settings.SmartVolume( nSpeed, KiloBytes ) );
		m_sStatus += strPart;
	}
	else
	{
		if ( pFile->m_nSources == 0 )
			nSpeed = 0;
		strPart.Format( LoadString( IDS_SEARCH_DETAILS_SOURCES_MANY ), pFile->m_nSources, (LPCTSTR)Settings.SmartVolume( nSpeed, KiloBytes ) );
		m_sStatus += strPart;
	}

	if ( m_pReviews.GetCount() > 1 )
	{
		strPart.Format( LoadString( IDS_SEARCH_DETAILS_REVIEWS_MANY ),
			m_pReviews.GetCount() );
		m_sStatus += strPart;
	}
	else if ( m_pReviews.GetCount() == 1 )
	{
		LoadString( strPart, IDS_SEARCH_DETAILS_REVIEWS_ONE );
		m_sStatus += strPart;
	}

	if ( pFile->m_pPreview != NULL && pFile->m_nPreview > 0 )
	{
		CImageFile pImage;

		if ( pImage.LoadFromMemory( _T(".jpg"), (LPCVOID)pFile->m_pPreview, pFile->m_nPreview, FALSE, TRUE ) )
		{
			pLock.Unlock();
			OnPreviewLoaded( m_oSHA1, &pImage );
		}
	}

	Update();

	if ( m_bCanPreview && ! m_bIsPreviewing && Settings.Search.AutoPreview )
	{
		RequestPreview();
	}
}

void CSearchDetailPanel::Update()
{
	CRect rc;

	GetWindowRect( &rc );
	rc.OffsetRect( -rc.left, -rc.top );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );

	int nThumbSize = min( max( rc.Height() - 16, 64 ), (int)Settings.Library.ThumbSize );
	rc.left += nThumbSize + 16;
	rc.right -= 8;

	CClientDC dc( this );
	int nHeight = 54 + m_pMetadata.Layout( &dc, rc.Width() );

	for ( POSITION pos = m_pReviews.GetHeadPosition() ; pos ; )
	{
		Review* pReview = m_pReviews.GetNext( pos );
		CRect rcReview( rc.left, nHeight, rc.right, nHeight );
		pReview->Layout( this, &rcReview );
		nHeight += rcReview.Height();
	}

	if ( ! m_bValid ) nHeight = 0;

	SCROLLINFO pInfo = {};
	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;
	pInfo.nMin		= 0;
	pInfo.nMax		= nHeight;
	pInfo.nPage		= rc.Height();
	pInfo.nPos		= 0;
	pInfo.nPos		= max( 0, min( pInfo.nPos, pInfo.nMax - (int)pInfo.nPage + 1 ) );

	SetScrollInfo( SB_VERT, &pInfo, TRUE );

	m_bRedraw = TRUE;
}

void CSearchDetailPanel::ClearReviews()
{
	for ( POSITION pos = m_pReviews.GetHeadPosition() ; pos ; )
	{
		delete m_pReviews.GetNext( pos );
	}

	m_pReviews.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CSearchDetailPanel message handlers

int CSearchDetailPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelCtrl::OnCreate( lpCreateStruct ) == -1 ) return -1;

	SetTimer( 1, 500, NULL );

	return 0;
}

void CSearchDetailPanel::OnDestroy()
{
	KillTimer( 1 );

	ClearReviews();

	CancelPreview();

	CloseThread();

	CPanelCtrl::OnDestroy();
}

void CSearchDetailPanel::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CPanelCtrl::OnVScroll( nSBCode, nPos, pScrollBar );

	SCROLLINFO pScroll = {};
	pScroll.cbSize	= sizeof(pScroll);
	pScroll.fMask	= SIF_ALL;
	GetScrollInfo( SB_VERT, &pScroll );

	for ( POSITION pos = m_pReviews.GetHeadPosition() ; pos ; )
	{
		Review* pReview = m_pReviews.GetNext( pos );
		pReview->Reposition( pScroll.nPos );
	}

	Invalidate();
}

void CSearchDetailPanel::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	SetFocus();
}

void CSearchDetailPanel::OnPaint()
{
	CSingleLock pLock( &m_pSection, TRUE );
	CPaintDC dc( this );
	CRect rcClient;
	CString str;

	GetClientRect( &rcClient );

	CFont* pOldFont = dc.GetCurrentFont();
	dc.SetBkColor( CoolInterface.m_crWindow );
	dc.SetBkMode( OPAQUE );
	dc.SetTextColor( CoolInterface.m_crText );

	if ( ! m_bValid )
	{
		dc.SelectObject( &CoolInterface.m_fntNormal );
		LoadString( str, IDS_SEARCH_DETAILS_EMPTY );
		CSize sz = dc.GetTextExtent( str );
		CPoint pt = rcClient.CenterPoint();
		pt.x -= sz.cx / 2; pt.y -= sz.cy / 2;
		dc.ExtTextOut( pt.x, pt.y, ETO_OPAQUE, &rcClient, str, NULL );
		dc.SelectObject( pOldFont );
		return;
	}

	// Draw thumbnail or icon
	int nThumbSize = min( max( rcClient.Height() - 16, 64 ), (int)Settings.Library.ThumbSize );
	CRect rcWork( rcClient.left + 8, rcClient.top + 8,
		rcClient.left + 8 + nThumbSize, rcClient.top + 8 + nThumbSize );

	m_rcThumb = rcWork;

	CString strLabel;
	if ( m_bCanPreview )
		LoadString( strLabel,
			m_bIsPreviewing ? IDS_SEARCH_DETAILS_PREVIEWING : IDS_SEARCH_DETAILS_PREVIEW );

	CoolInterface.DrawThumbnail( &dc, rcWork, m_bIsPreviewing, FALSE,
		m_bmThumb, m_nIcon48, m_nIcon32, strLabel );

	rcWork.SetRect( rcWork.right + 8, rcWork.top, rcClient.right - 8, rcClient.bottom );

	dc.SetViewportOrg( 0, -GetScrollPos( SB_VERT ) );

	dc.SetBkColor( CoolInterface.m_crWindow );
	dc.SetTextColor( CoolInterface.m_crText );

	CPoint ptStar( rcWork.right - 3, rcWork.top - 2 );

	if ( m_nRating > 1 )
	{
		for ( int nRating = m_nRating - 1 ; nRating ; nRating-- )
		{
			ptStar.x -= 16;
			CoolInterface.Draw( &dc, IDI_STAR, 16, ptStar.x, ptStar.y, CoolInterface.m_crWindow );
			dc.ExcludeClipRect( ptStar.x, ptStar.y, ptStar.x + 16, ptStar.y + 16 );
		}
	}
	else if ( m_nRating == 1 )
	{
		ptStar.x -= 16;
		CoolInterface.Draw( &dc, IDI_FAKE, 16, ptStar.x, ptStar.y, CoolInterface.m_crWindow );
		dc.ExcludeClipRect( ptStar.x, ptStar.y, ptStar.x + 16, ptStar.y + 16 );
	}

	dc.SelectObject( &CoolInterface.m_fntCaption );
	DrawText( &dc, rcWork.left, rcWork.top, m_sName, NULL,
		m_nRating > 0 ? rcWork.Width() - m_nRating * 16 - 3 : rcWork.Width() - 4 );

	rcWork.top += 20;

	dc.FillSolidRect( rcWork.left, rcWork.top, rcWork.Width(), 1, CoolInterface.m_crMargin );
	dc.ExcludeClipRect( rcWork.left, rcWork.top, rcWork.right, rcWork.top + 1 );
	dc.SetBkColor( CoolInterface.m_crWindow );
	rcWork.top += 4;

	dc.SelectObject( &CoolInterface.m_fntBold );
	LoadString( str, IDS_TIP_SIZE );
	DrawText( &dc, rcWork.right - 125, rcWork.top, str + ':' );
	dc.SelectObject( &CoolInterface.m_fntNormal );
	DrawText( &dc, rcWork.right - 60, rcWork.top, m_sSize );
	if ( m_pReviews.GetCount() )
	{
		dc.SelectObject( &CoolInterface.m_fntUnder );
		dc.SetTextColor( CoolInterface.m_crTextLink );
	}
	DrawText( &dc, rcWork.left, rcWork.top, m_sStatus, &m_rcStatus );
	rcWork.top += 18;

	m_pMetadata.Paint( &dc, &rcWork );

	dc.SetViewportOrg( 0, 0 );
	dc.SelectObject( &CoolInterface.m_fntCaption );
	dc.SetBkColor( CoolInterface.m_crWindow );
	dc.SetTextColor( 0 );

	for ( POSITION pos = m_pReviews.GetHeadPosition() ; pos ; )
	{
		Review* pReview = m_pReviews.GetNext( pos );
		pReview->Paint( &dc, GetScrollPos( SB_VERT ) );
	}

	dc.SelectObject( pOldFont );
	dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );
}

void CSearchDetailPanel::DrawText(CDC* pDC, int nX, int nY, LPCTSTR pszText, RECT* pRect, int nMaxWidth)
{
	CSize sz = pDC->GetTextExtent( pszText, static_cast< int >( _tcslen( pszText ) ) );

	int nWidth = sz.cx;
	if ( nMaxWidth > 0 )
	{
		nWidth = min( (int)sz.cx, nMaxWidth );
	}
	CRect rc( nX - 2, nY - 2, nX + nWidth + 2, nY + sz.cy + 2 );

	pDC->ExtTextOut( nX, nY, ETO_CLIPPED|ETO_OPAQUE, &rc, pszText, static_cast< UINT >( _tcslen( pszText ) ), NULL );
	pDC->ExcludeClipRect( &rc );

	if ( pRect != NULL ) CopyMemory( pRect, &rc, sizeof(RECT) );
}

BOOL CSearchDetailPanel::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;

	GetCursorPos( &point );
	ScreenToClient( &point );

	if ( m_bValid && m_bCanPreview && ! m_bIsPreviewing && m_rcThumb.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}

	point.y += GetScrollPos( SB_VERT );

	if ( m_bValid && m_pReviews.GetCount() > 0 && m_rcStatus.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}

	if ( m_bValid && m_pMetadata.HitTest( point, TRUE ) != NULL )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}

	return CPanelCtrl::OnSetCursor( pWnd, nHitTest, message );
}

void CSearchDetailPanel::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ( m_bValid && m_bCanPreview && ! m_bIsPreviewing && m_rcThumb.PtInRect( point ) )
	{
		RequestPreview();
	}

	point.y += GetScrollPos( SB_VERT );

	if ( m_bValid && m_pReviews.GetCount() > 0 && m_rcStatus.PtInRect( point ) )
	{
		int nHeight = 54 + m_pMetadata.GetHeight();
		SetScrollPos( SB_VERT, nHeight );
		OnVScroll( SB_THUMBPOSITION, nHeight, NULL );
		Invalidate();
	}

	m_pMetadata.OnClick( point );

	CPanelCtrl::OnLButtonUp( nFlags, point );
}

/////////////////////////////////////////////////////////////////////////////
// Review construction

Review::Review(const Hashes::Guid& oGUID, IN_ADDR* pAddress, LPCTSTR pszNick, int nRating, LPCTSTR pszComments)
{
	m_oGUID		= oGUID;
	m_nRating	= nRating;

	if ( pszNick != NULL && *pszNick != 0 )
	{
		m_sNick.Format( _T("%s (%s)"), pszNick,
			(LPCTSTR)CString( inet_ntoa( *pAddress ) ) );
	}
	else
	{
		m_sNick = inet_ntoa( *pAddress );
	}

	if ( pszComments != NULL )
	{
		m_pComments.m_szMargin = CSize( 6, 0 );
		Emoticons.FormatText( &m_pComments, pszComments, TRUE );
	}
}

Review::~Review()
{
	if ( m_wndComments.m_hWnd != NULL ) m_wndComments.DestroyWindow();
}

void Review::Layout(CSearchDetailPanel* pParent, CRect* pRect)
{
	pRect->bottom += 22;

	if ( m_pComments.GetCount() )
	{
		if ( m_wndComments.m_hWnd == NULL )
		{
			m_wndComments.Create( WS_CHILD, *pRect, pParent, IDC_REVIEW_VIEW );
			m_wndComments.SetSelectable( TRUE );
			m_wndComments.SetDocument( &m_pComments );
		}

		pRect->bottom += m_wndComments.FullHeightMove( pRect->left, pRect->bottom, pRect->Width(), TRUE );
		pRect->bottom += 4;
	}
	else
	{
		pRect->bottom += 2;
	}

	m_rc.CopyRect( pRect );
}

void Review::Reposition(int nScroll)
{
	if ( m_wndComments.m_hWnd != NULL )
	{
		m_wndComments.SetWindowPos( NULL, m_rc.left, m_rc.top - nScroll + 22, 0, 0,
			SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE );
	}
}

void Review::Paint(CDC* pDC, int nScroll)
{
	CRect rc( &m_rc );
	rc.OffsetRect( 0, -nScroll );

	pDC->FillSolidRect( rc.left, rc.top, rc.Width(), 1, CoolInterface.m_crMargin );
	pDC->ExcludeClipRect( rc.left, rc.top, rc.right, rc.top + 1 );
	rc.top += 4;

	CString strFormat, strCaption;

	LoadString( strFormat, m_pComments.GetCount() > 0 ? IDS_SEARCH_DETAILS_WRITES : IDS_SEARCH_DETAILS_RATES );
	strCaption.Format( strFormat, (LPCTSTR)m_sNick );

	pDC->SetBkColor( CoolInterface.m_crWindow );
	CSearchDetailPanel::DrawText( pDC, rc.left, rc.top, strCaption );

	CPoint ptStar( rc.right - 3, rc.top );

	if ( m_nRating > 1 )
	{
		for ( int nRating = m_nRating - 1 ; nRating ; nRating-- )
		{
			ptStar.x -= 16;
			CoolInterface.Draw( pDC, IDI_STAR, 16, ptStar.x, ptStar.y, CoolInterface.m_crWindow );
			pDC->ExcludeClipRect( ptStar.x, ptStar.y, ptStar.x + 16, ptStar.y + 16 );
		}
	}
	else if ( m_nRating == 1 )
	{
		ptStar.x -= 16;
		CoolInterface.Draw( pDC, IDI_FAKE, 16, ptStar.x, ptStar.y, CoolInterface.m_crWindow );
		pDC->ExcludeClipRect( ptStar.x, ptStar.y, ptStar.x + 16, ptStar.y + 16 );
	}

	rc.top += 20;
}

void CSearchDetailPanel::OnClickReview(NMHDR* pNotify, LRESULT* /*pResult*/)
{
	if ( CRichElement* pElement = ((RVN_ELEMENTEVENT*) pNotify)->pElement )
	{
		theApp.InternalURI( pElement->m_sLink );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSearchDetailPanel previewing functionality

BOOL CSearchDetailPanel::RequestPreview()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( ! m_bValid || ! m_bCanPreview || m_pPreviewURLs.IsEmpty() ) return FALSE;

	BeginThread( "CtrlSearchDetailPanel" );

	m_bRunPreview = TRUE;

	pLock.Unlock();

	Wakeup();

	return TRUE;
}

void CSearchDetailPanel::CancelPreview()
{
	CSingleLock pLock( &m_pSection, TRUE );

	m_bRunPreview = FALSE;
	m_pPreviewURLs.RemoveAll();

	if ( m_bmThumb.m_hObject != NULL )
	{
		m_bmThumb.DeleteObject();
		m_bRedraw = TRUE;
	}

	if ( m_bIsPreviewing )
	{
		m_bIsPreviewing = FALSE;
		m_bRedraw = TRUE;
	}

	if ( m_pRequest.IsPending() && ! m_pRequest.IsThreadEnabled() )
		// Already asked for stop
		return;

	m_pRequest.Cancel();
}

void CSearchDetailPanel::OnRun()
{
	CSingleLock pLock( &m_pSection );

	while ( IsThreadEnabled() )
	{
		pLock.Lock();

		if ( ! m_bValid || ! m_bRunPreview || m_pPreviewURLs.IsEmpty() )
		{
			if ( m_bIsPreviewing )
			{
				m_bIsPreviewing = FALSE;
				m_bCanPreview = ! m_pPreviewURLs.IsEmpty();
				m_bRedraw = TRUE;
			}

			pLock.Unlock();

			Doze( 1000 );

			continue;
		}

		CString strURL	= m_pPreviewURLs.RemoveHead();
        Hashes::Sha1Hash oSHA1( m_oSHA1 );

		if ( ! m_bIsPreviewing )
		{
			m_bIsPreviewing = TRUE;
			m_bRedraw = TRUE;
		}

		pLock.Unlock();

		BYTE* pBuffer;
		DWORD nBuffer;

		if ( ExecuteRequest( strURL, &pBuffer, &nBuffer ) )
		{
			CImageFile pImage;

			if ( pImage.LoadFromMemory( _T(".jpg"), (LPCVOID)pBuffer, nBuffer, FALSE, TRUE ) )
			{
				OnPreviewLoaded( oSHA1, &pImage );
				CachePreviewImage( oSHA1, pBuffer, nBuffer );
			}
			else
			{
				theApp.Message( MSG_ERROR, IDS_SEARCH_DETAILS_PREVIEW_FAILED, (LPCTSTR)strURL );
			}

			delete[] pBuffer;
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_SEARCH_DETAILS_PREVIEW_FAILED, (LPCTSTR)strURL );
		}
	}
}

BOOL CSearchDetailPanel::ExecuteRequest(const CString& strURL, BYTE** ppBuffer, DWORD* pnBuffer)
{
	m_pRequest.Clear();
	m_pRequest.SetURL( strURL );
	m_pRequest.AddHeader( _T("Accept"), _T("image/jpeg") );
	m_pRequest.LimitContentLength( Settings.Search.MaxPreviewLength );

	if ( ! m_pRequest.Execute( FALSE ) )
	{
		theApp.Message( MSG_DEBUG, _T("Preview failed: unable to execute request.") );
		return FALSE;
	}

	/*int nCode =*/ m_pRequest.GetStatusCode();

	if ( m_pRequest.GetStatusSuccess() == FALSE )
	{
		theApp.Message( MSG_DEBUG, _T("Preview failed: HTTP status code %i"),
			m_pRequest.GetStatusCode() );
		return FALSE;
	}

	CString strURN = m_pRequest.GetHeader( _T("X-Previewed-URN") );

	if ( strURN.GetLength() )
	{
		Hashes::Sha1Hash oSHA1;

		if ( oSHA1.fromUrn( strURN ) && validAndUnequal( oSHA1, m_oSHA1 ) )
		{
			theApp.Message( MSG_DEBUG, _T("Preview failed: wrong URN.") );
			return FALSE;
		}
	}

	CString strMIME = m_pRequest.GetHeader( _T("Content-Type") );

	if ( strMIME.CompareNoCase( _T("image/jpeg") ) != 0 )
	{
		theApp.Message( MSG_DEBUG, _T("Preview failed: unacceptable content type.") );
		return FALSE;
	}

	CBuffer* pBuffer = m_pRequest.GetResponseBuffer();
	if ( pBuffer == NULL ) return FALSE;

	*pnBuffer = pBuffer->m_nLength;
	*ppBuffer = new BYTE[ *pnBuffer ];
	if ( ! *ppBuffer )
	{
		*pnBuffer = 0;
		return FALSE;
	}

	CopyMemory( *ppBuffer, pBuffer->m_pBuffer, *pnBuffer );

	return TRUE;
}

void CSearchDetailPanel::OnPreviewLoaded(const Hashes::Sha1Hash& oSHA1, CImageFile* pImage)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( validAndUnequal( m_oSHA1, oSHA1 ) ) return;

	m_bCanPreview = m_bRunPreview = m_bIsPreviewing = FALSE;

	if ( m_bmThumb.m_hObject ) m_bmThumb.DeleteObject();

	m_bmThumb.Attach( pImage->CreateBitmap() );

	m_bRedraw = TRUE;
}

BOOL CSearchDetailPanel::CachePreviewImage(const Hashes::Sha1Hash& /*oSHA1*/, LPBYTE pBuffer, DWORD nBuffer)
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );

	if ( m_pMatches->FileToItem( m_pFile ) != 0xFFFFFFFF )
	{
		if ( m_pFile->m_pPreview != NULL ) delete [] m_pFile->m_pPreview;

		m_pFile->m_nPreview = nBuffer;
		m_pFile->m_pPreview = new BYTE[ nBuffer ];
		CopyMemory( m_pFile->m_pPreview, pBuffer, nBuffer );

		return TRUE;
	}

	return FALSE;
}

void CSearchDetailPanel::OnTimer(UINT_PTR nIDEvent)
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
