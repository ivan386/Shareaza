//
// WndMonitor.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "GraphItem.h"
#include "CoolInterface.h"
#include "SkinWindow.h"
#include "Skin.h"

#include "WndMonitor.h"
#include "CtrlMonitorBar.h"
#include "CtrlMediaFrame.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CRemoteWnd, CWnd)

BEGIN_MESSAGE_MAP(CRemoteWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_WINDOWPOSCHANGING()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

LPCTSTR CRemoteWnd::m_hClass = NULL;

#define SNAP_PIXELS 6


/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd construction

CRemoteWnd::CRemoteWnd()
{
	if ( m_hClass == NULL )
		m_hClass = AfxRegisterWndClass( 0 );

	m_pMonitor		= NULL;
	m_pSkin			= NULL;
	m_pCmdHover		= NULL;
	m_pCmdDown		= NULL;
	m_nTimer		= 0;

	m_bScaler		= FALSE;
	m_bMediaSeek	= FALSE;
	m_nMediaSeek	= -1;
	m_bMediaVol		= FALSE;
	m_nMediaVol		= -1;
}

CRemoteWnd::~CRemoteWnd()
{
	for ( POSITION pos = m_pButtons.GetHeadPosition() ; pos ; ) delete m_pButtons.GetNext( pos );
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd system message handlers

BOOL CRemoteWnd::Create(CMonitorBarCtrl* pMonitor)
{
	ASSERT( m_hWnd == NULL );

	m_pMonitor	= pMonitor;
	m_pSkin		= NULL;

	return CreateEx(	WS_EX_TOOLWINDOW, m_hClass, CLIENT_NAME_T _T(" Remote"),
						WS_OVERLAPPED|WS_CLIPCHILDREN, 0, 0, 0, 0, NULL, 0 );
}

BOOL CRemoteWnd::IsVisible()
{
	return ( m_hWnd != NULL );
}

int CRemoteWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	if ( lpCreateStruct->dwExStyle & WS_EX_LAYOUTRTL )
	{
		lpCreateStruct->dwExStyle ^= WS_EX_LAYOUTRTL;
		SetWindowLongPtr( this->m_hWnd, GWL_EXSTYLE, lpCreateStruct->dwExStyle );
	}

	OnSkinChange();
	EnableToolTips();

	int nWindowX = AfxGetApp()->GetProfileInt( _T("Windows"), _T("CRemoteWnd.Left"), 0 );
	int nWindowY = AfxGetApp()->GetProfileInt( _T("Windows"), _T("CRemoteWnd.Top"), 0 );

	SetWindowPos( &wndTopMost, nWindowX, nWindowY, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW );

	SetTimer( 1, 50, NULL );

	return 0;
}

void CRemoteWnd::OnDestroy()
{
	KillTimer( 1 );
	Settings.SaveWindow( NULL, this );

	CWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd window movement message handlers

void CRemoteWnd::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos)
{
	CWnd::OnWindowPosChanging( lpwndpos );

	CRect rcWork;
	SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWork, 0 );

	if ( abs( lpwndpos->x ) <= ( rcWork.left + SNAP_PIXELS ) )
	{
		lpwndpos->x = rcWork.left;
	}
	else if (	( lpwndpos->x + lpwndpos->cx ) >= ( rcWork.right - SNAP_PIXELS ) &&
				( lpwndpos->x + lpwndpos->cx ) <= ( rcWork.right + SNAP_PIXELS ) )
	{
		lpwndpos->x = rcWork.right - lpwndpos->cx;
	}

	if ( abs( lpwndpos->y ) <= ( rcWork.top + SNAP_PIXELS ) )
	{
		lpwndpos->y = rcWork.top;
	}
	else if (	( lpwndpos->y + lpwndpos->cy ) >= ( rcWork.bottom - SNAP_PIXELS) &&
				( lpwndpos->y + lpwndpos->cy ) <= ( rcWork.bottom + SNAP_PIXELS ) )
	{
		lpwndpos->y = rcWork.bottom - lpwndpos->cy;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd command UI

LRESULT CRemoteWnd::OnIdleUpdateCmdUI(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	UpdateCmdButtons();
	return 0;
}

void CRemoteWnd::OnTimer(UINT_PTR /*nIDEvent*/)
{
	if ( m_nTimer++ >= 10 )
	{
		m_nTimer = 0;
		UpdateCmdButtons();

		if ( m_pCmdHover != NULL )
		{
			CPoint point;
			GetCursorPos( &point );
			ScreenToClient( &point );
			if ( HitTestButtons( point ) != m_pCmdHover ) m_pCmdHover = NULL;
		}

		m_sStatus.Format( LoadString( IDS_TRAY_TIP ),
			CGraphItem::GetValue( GRC_GNUTELLA_CONNECTIONS ),
			(LPCTSTR)Settings.SmartSpeed( CGraphItem::GetValue( GRC_TOTAL_BANDWIDTH_IN ), bits ),
			(LPCTSTR)Settings.SmartSpeed( CGraphItem::GetValue( GRC_TOTAL_BANDWIDTH_OUT ), bits ),
			CGraphItem::GetValue( GRC_DOWNLOADS_TRANSFERS ),
			CGraphItem::GetValue( GRC_UPLOADS_TRANSFERS ) );
	}

	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd command buttons

CRemoteWnd::CmdButton* CRemoteWnd::HitTestButtons(const CPoint& ptIn, BOOL bAll) const
{
	if ( m_pSkin == NULL ) return NULL;

	for ( POSITION pos = m_pButtons.GetHeadPosition() ; pos ; )
	{
		CmdButton* pButton = m_pButtons.GetNext( pos );
		if ( pButton->HitTest( ptIn, bAll ) ) return pButton;
	}

	return NULL;
}

void CRemoteWnd::UpdateCmdButtons()
{
	if ( m_hWnd == NULL ) return;

	BOOL bChanged = FALSE;

	for ( POSITION pos = m_pButtons.GetHeadPosition() ; pos ; )
	{
		CmdButton* pButton = m_pButtons.GetNext( pos );
		pButton->m_bChanged = FALSE;
		pButton->DoUpdate( (CCmdTarget*)AfxGetMainWnd(), TRUE );
		bChanged |= pButton->m_bChanged;
	}

	if ( bChanged ) Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd skin change handler

void CRemoteWnd::OnSkinChange()
{
	for ( POSITION pos = m_pButtons.GetHeadPosition() ; pos ; ) delete m_pButtons.GetNext( pos );
	m_pCmdHover = m_pCmdDown = NULL;
	m_pButtons.RemoveAll();

	m_pSkin = NULL;

	if ( m_hWnd == NULL ) return;

	m_pSkin = Skin.GetWindowSkin( _T("CRemoteWnd") );

	if ( m_pSkin == NULL )
	{
		ShowWindow( SW_HIDE );
		return;
	}

	if ( ! m_pSkin->GetPart( _T("Background"), m_rcBackground ) )
	{
		m_pSkin = NULL;
		ShowWindow( SW_HIDE );
		return;
	}

	for ( POSITION pos = m_pSkin->m_pAnchorList.GetStartPosition() ; pos ; )
	{
		CRect* prcAnchor;
		CString strAnchor;
		m_pSkin->m_pAnchorList.GetNextAssoc( pos, strAnchor, prcAnchor );

		if ( strAnchor.Find( '_' ) == 0 )
		{
			m_pButtons.AddTail( new CmdButton( strAnchor ) );
		}
	}

	m_bsStatusText		= m_pSkin->GetAnchor( _T("StatusText"), m_rcsStatusText );

	m_bsHistoryDest		= m_pSkin->GetAnchor( _T("History"), m_rcsHistoryDest );
	m_bsHistoryTx[0]	= m_pSkin->GetPart( _T("HistoryTx1"), m_rcsHistoryTx[0] );
	m_bsHistoryTx[1]	= m_pSkin->GetPart( _T("HistoryTx2"), m_rcsHistoryTx[1] );
	m_bsHistoryRx[0]	= m_pSkin->GetPart( _T("HistoryRx1"), m_rcsHistoryRx[0] );
	m_bsHistoryRx[1]	= m_pSkin->GetPart( _T("HistoryRx2"), m_rcsHistoryRx[1] );
	m_bsFlowTxDest		= m_pSkin->GetAnchor( _T("FlowTx"), m_rcsFlowTxDest );
	m_bsFlowTxSrc[0]	= m_pSkin->GetPart( _T("FlowTx1"), m_rcsFlowTxSrc[0] );
	m_bsFlowTxSrc[1]	= m_pSkin->GetPart( _T("FlowTx2"), m_rcsFlowTxSrc[1] );
	m_bsFlowRxDest		= m_pSkin->GetAnchor( _T("FlowRx"), m_rcsFlowRxDest );
	m_bsFlowRxSrc[0]	= m_pSkin->GetPart( _T("FlowRx1"), m_rcsFlowRxSrc[0] );
	m_bsFlowRxSrc[1]	= m_pSkin->GetPart( _T("FlowRx2"), m_rcsFlowRxSrc[1] );
	m_bsScalerTrack		= m_pSkin->GetAnchor( _T("ScalerTrack"), m_rcsScalerTrack );
	m_bsScalerTab		= m_pSkin->GetPart( _T("ScalerTab"), m_rcsScalerTab );
	m_bsScalerBar		= m_pSkin->GetPart( _T("ScalerBar"), m_rcsScalerBar );

	m_bsMediaSeekTrack	= m_pSkin->GetAnchor( _T("MediaSeekTrack"), m_rcsMediaSeekTrack );
	m_bsMediaSeekTab	= m_pSkin->GetPart( _T("MediaSeekTab"), m_rcsMediaSeekTab );
	m_bsMediaSeekBar	= m_pSkin->GetPart( _T("MediaSeekBar"), m_rcsMediaSeekBar );
	m_bsMediaVolTrack	= m_pSkin->GetAnchor( _T("MediaVolTrack"), m_rcsMediaVolTrack );
	m_bsMediaVolTab		= m_pSkin->GetPart( _T("MediaVolTab"), m_rcsMediaVolTab );
	m_bsMediaVolBar		= m_pSkin->GetPart( _T("MediaVolBar"), m_rcsMediaVolBar );

	m_bsMediaStates[0][0]	= m_pSkin->GetAnchor( _T("MediaStateStopped"), m_rcsMediaStates[0][0] );
	m_bsMediaStates[0][1]	= m_pSkin->GetAnchor( _T("MediaStatePlaying"), m_rcsMediaStates[0][1] );
	m_bsMediaStates[0][2]	= m_pSkin->GetAnchor( _T("MediaStatePaused"), m_rcsMediaStates[0][2] );
	m_bsMediaStates[1][0]	= m_pSkin->GetPart( _T("MediaStateStopped"), m_rcsMediaStates[1][0] );
	m_bsMediaStates[1][1]	= m_pSkin->GetPart( _T("MediaStatePlaying"), m_rcsMediaStates[1][1] );
	m_bsMediaStates[1][2]	= m_pSkin->GetPart( _T("MediaStatePaused"), m_rcsMediaStates[1][2] );

	SetWindowPos( NULL, 0, 0, m_rcBackground.Width(), m_rcBackground.Height(), SWP_NOMOVE );
	m_pSkin->OnSize( this );
	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd display message handlers

void CRemoteWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );
	if ( m_pSkin != NULL ) m_pSkin->OnSize( this );
	Invalidate();
}

void CRemoteWnd::OnPaint()
{
	CPaintDC dc( this );
	CRect rcClient;

	GetClientRect( &rcClient );

	if ( m_pSkin == NULL )
	{
		dc.FillSolidRect( &rcClient, CoolInterface.m_crSysBtnFace );
		return;
	}

	CSize size = rcClient.Size();
	CDC* pDC = CoolInterface.GetBuffer( dc, size );
	m_pSkin->Prepare( &dc );

	pDC->BitBlt( 0, 0, m_rcBackground.Width(), m_rcBackground.Height(),
		&m_pSkin->m_dcSkin, m_rcBackground.left, m_rcBackground.top, SRCCOPY );

	CFont* pfOld = NULL;

	if ( m_pSkin->m_fnCaption.m_hObject != NULL )
	{
		pfOld = (CFont*)pDC->SelectObject( &m_pSkin->m_fnCaption );
		pDC->SetTextColor( m_pSkin->m_crCaptionText );
	}
	else
	{
		pfOld = (CFont*)pDC->SelectObject( &CoolInterface.m_fntNormal );
		pDC->SetTextColor( RGB( 255, 255, 255 ) );
	}

	pDC->SetBkMode( TRANSPARENT );
	m_bStatus = FALSE;

	PaintHistory( pDC, m_pMonitor->m_pTxItem, m_pMonitor->m_pRxItem, m_pMonitor->m_nMaximum );
	PaintFlow( pDC, &m_bsFlowTxDest, &m_rcsFlowTxDest, m_bsFlowTxSrc, m_rcsFlowTxSrc, m_pMonitor->m_pTxItem, m_pMonitor->m_nMaximum );
	PaintFlow( pDC, &m_bsFlowRxDest, &m_rcsFlowRxDest, m_bsFlowRxSrc, m_rcsFlowRxSrc, m_pMonitor->m_pRxItem, m_pMonitor->m_nMaximum );
	PaintScaler( pDC );
	PaintMedia( pDC );
	PaintStatus( pDC );

	for ( POSITION pos = m_pButtons.GetHeadPosition() ; pos ; )
	{
		m_pButtons.GetNext( pos )->Paint( pDC, rcClient, m_pSkin, m_pCmdHover, m_pCmdDown );
	}

	pDC->SelectObject( pfOld );
	pDC->SelectClipRgn( NULL );
	dc.BitBlt( 0, 0, rcClient.Width(), rcClient.Height(), pDC, 0, 0, SRCCOPY );
}

void CRemoteWnd::OnNcPaint()
{
}

BOOL CRemoteWnd::OnNcActivate(BOOL /*bActive*/)
{
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd paint monitor functionality

void CRemoteWnd::PaintHistory(CDC* pDC, CGraphItem* pTxItem, CGraphItem* pRxItem, DWORD nMaximum)
{
	if ( nMaximum == 0 ) return;
	if ( m_bsHistoryDest == FALSE ) return;

	pTxItem->SetHistory( m_rcsHistoryDest.Width(), TRUE );
	pRxItem->SetHistory( m_rcsHistoryDest.Width(), TRUE );

	DWORD nMax = min( pTxItem->m_nLength, (DWORD)m_rcsHistoryDest.Width() );
	int nX = m_rcsHistoryDest.right - 1;

	for ( DWORD nPos = 0 ; nPos < nMax ; nPos++, nX-- )
	{
		DWORD nTxValue = pTxItem->GetValueAt( nPos );
		DWORD nRxValue = pRxItem->GetValueAt( nPos );
		nTxValue = m_rcsHistoryDest.bottom - nTxValue * m_rcsHistoryDest.Height() / nMaximum;
		nRxValue = m_rcsHistoryDest.bottom - nRxValue * m_rcsHistoryDest.Height() / nMaximum;

		if ( nTxValue < nRxValue )
		{
			if ( nTxValue < (DWORD)m_rcsHistoryDest.bottom && m_bsHistoryTx[0] )
			{
				pDC->BitBlt( nX, nTxValue, 1, m_rcsHistoryDest.bottom - nTxValue,
					&m_pSkin->m_dcSkin, m_rcsHistoryTx[0].left + ( nX - m_rcsHistoryDest.left ),
					m_rcsHistoryTx[0].top + nTxValue - m_rcsHistoryDest.top, SRCCOPY );
			}
			else if ( nTxValue < (DWORD)m_rcsHistoryDest.bottom && m_bsHistoryTx[1] )
			{
				pDC->BitBlt( nX, nTxValue, 1, m_rcsHistoryDest.bottom - nTxValue,
					&m_pSkin->m_dcSkin, m_rcsHistoryTx[1].left + ( nX - m_rcsHistoryDest.left ),
					m_rcsHistoryTx[1].top, SRCCOPY );
			}
			if ( nRxValue < (DWORD)m_rcsHistoryDest.bottom && m_bsHistoryRx[0] )
			{
				pDC->BitBlt( nX, nRxValue, 1, m_rcsHistoryDest.bottom - nRxValue,
					&m_pSkin->m_dcSkin, m_rcsHistoryRx[0].left + ( nX - m_rcsHistoryDest.left ),
					m_rcsHistoryRx[0].top + nRxValue - m_rcsHistoryDest.top, SRCCOPY );
			}
			else if ( nRxValue < (DWORD)m_rcsHistoryDest.bottom && m_bsHistoryRx[1] )
			{
				pDC->BitBlt( nX, nRxValue, 1, m_rcsHistoryDest.bottom - nRxValue,
					&m_pSkin->m_dcSkin, m_rcsHistoryRx[1].left + ( nX - m_rcsHistoryDest.left ),
					m_rcsHistoryRx[1].top, SRCCOPY );
			}
		}
		else
		{
			if ( nRxValue < (DWORD)m_rcsHistoryDest.bottom && m_bsHistoryRx[0] )
			{
				pDC->BitBlt( nX, nRxValue, 1, m_rcsHistoryDest.bottom - nRxValue,
					&m_pSkin->m_dcSkin, m_rcsHistoryRx[0].left + ( nX - m_rcsHistoryDest.left ),
					m_rcsHistoryRx[0].top + nRxValue - m_rcsHistoryDest.top, SRCCOPY );
			}
			else if ( nRxValue < (DWORD)m_rcsHistoryDest.bottom && m_bsHistoryRx[1] )
			{
				pDC->BitBlt( nX, nRxValue, 1, m_rcsHistoryDest.bottom - nRxValue,
					&m_pSkin->m_dcSkin, m_rcsHistoryRx[1].left + ( nX - m_rcsHistoryDest.left ),
					m_rcsHistoryRx[1].top, SRCCOPY );
			}
			if ( nTxValue < (DWORD)m_rcsHistoryDest.bottom && m_bsHistoryTx[0] )
			{
				pDC->BitBlt( nX, nTxValue, 1, m_rcsHistoryDest.bottom - nTxValue,
					&m_pSkin->m_dcSkin, m_rcsHistoryTx[0].left + ( nX - m_rcsHistoryDest.left ),
					m_rcsHistoryTx[0].top + nTxValue - m_rcsHistoryDest.top, SRCCOPY );
			}
			else if ( nTxValue < (DWORD)m_rcsHistoryDest.bottom && m_bsHistoryTx[1] )
			{
				pDC->BitBlt( nX, nTxValue, 1, m_rcsHistoryDest.bottom - nTxValue,
					&m_pSkin->m_dcSkin, m_rcsHistoryTx[1].left + ( nX - m_rcsHistoryDest.left ),
					m_rcsHistoryTx[1].top, SRCCOPY );
			}
		}
	}
}

void CRemoteWnd::PaintFlow(CDC* pDC, BOOL* pbDest, CRect* prcDest, BOOL* pbSrc, CRect* prcSrc, CGraphItem* pItem, DWORD nMaximum)
{
	if ( *pbDest == FALSE ) return;
	if ( nMaximum == 0 || pItem->m_nLength < 1 ) return;

	DWORD nValue = (DWORD)pItem->GetValue( pItem->m_nCode );
	nValue = nValue * prcDest->Height() / nMaximum;

	if ( pbSrc[0] )
	{
		pDC->BitBlt( prcDest->left, prcDest->bottom - nValue,
			prcDest->Width(), nValue, &m_pSkin->m_dcSkin,
			prcSrc[0].left, prcSrc[0].bottom - nValue, SRCCOPY );
	}
	else if ( pbSrc[1] )
	{
		pDC->BitBlt( prcDest->left, prcDest->bottom - nValue,
			prcDest->Width(), nValue, &m_pSkin->m_dcSkin,
			prcSrc[1].left, prcSrc[1].top, SRCCOPY );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd paint bandwidth scaler

void CRemoteWnd::PaintScaler(CDC* pDC)
{
	if ( m_bsScalerTrack )
	{
		CRect rcTrack( &m_rcsScalerTrack ), rcPart;
		float nPosition = 0;

		if ( Settings.Live.BandwidthScale > 100 )
			nPosition = 1.0f;
		else
			nPosition = (float)Settings.Live.BandwidthScale / 105.0f;

		if ( m_bsScalerTab )
		{
			rcPart.CopyRect( &m_rcsScalerTab );

			if ( m_bScaler ) rcPart.OffsetRect( rcPart.Width(), 0 );

			m_rcScalerTab.left		= rcTrack.left + (int)( nPosition * ( rcTrack.Width() - rcPart.Width() ) );
			m_rcScalerTab.right		= m_rcScalerTab.left + rcPart.Width();
			m_rcScalerTab.top		= rcTrack.top;
			m_rcScalerTab.bottom	= rcTrack.top + rcPart.Height();

			pDC->BitBlt( m_rcScalerTab.left, m_rcScalerTab.top, m_rcScalerTab.Width(),
				m_rcScalerTab.Height(), &m_pSkin->m_dcSkin, rcPart.left, rcPart.top, SRCCOPY );
			pDC->ExcludeClipRect( &m_rcScalerTab );

			rcTrack.right = m_rcScalerTab.left;
		}

		if ( m_bsScalerBar )
		{
			rcPart.CopyRect( &m_rcsScalerBar );
			CRect rcBar( &rcTrack );

			while ( rcBar.left < rcBar.right )
			{
				pDC->BitBlt( rcBar.left, rcBar.top, min( rcBar.Width(), rcPart.Width() ),
					rcPart.Height(), &m_pSkin->m_dcSkin, rcPart.left, rcPart.top, SRCCOPY );
				rcBar.left += rcPart.Width();
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd paint media functionality

void CRemoteWnd::PaintMedia(CDC* pDC)
{
	if ( m_bsMediaSeekTrack )
	{
		float nPosition = m_nMediaSeek >= 0 ? m_nMediaSeek : ( CMediaFrame::GetMediaFrame() != NULL ? CMediaFrame::GetMediaFrame()->GetPosition() : 0 );
		CRect rcTrack( &m_rcsMediaSeekTrack ), rcPart;

		if ( m_bsMediaSeekTab )
		{
			rcPart.CopyRect( &m_rcsMediaSeekTab );

			if ( m_bMediaSeek || m_nMediaSeek >= 0 )
				rcPart.OffsetRect( rcPart.Width(), 0 );

			m_rcMediaSeekTab.left	= rcTrack.left + (int)( nPosition * ( rcTrack.Width() - rcPart.Width() ) );
			m_rcMediaSeekTab.right	= m_rcMediaSeekTab.left + rcPart.Width();
			m_rcMediaSeekTab.top	= rcTrack.top;
			m_rcMediaSeekTab.bottom	= rcTrack.top + rcPart.Height();

			pDC->BitBlt( m_rcMediaSeekTab.left, m_rcMediaSeekTab.top, m_rcMediaSeekTab.Width(),
				m_rcMediaSeekTab.Height(), &m_pSkin->m_dcSkin, rcPart.left, rcPart.top, SRCCOPY );
			pDC->ExcludeClipRect( &m_rcMediaSeekTab );

			rcTrack.right = m_rcMediaSeekTab.left;
		}

		if ( m_bsMediaSeekBar )
		{
			rcPart.CopyRect( &m_rcsMediaSeekBar );
			CRect rcBar( &rcTrack );

			while ( rcBar.left < rcBar.right )
			{
				pDC->BitBlt( rcBar.left, rcBar.top, min( rcBar.Width(), rcPart.Width() ),
					rcPart.Height(), &m_pSkin->m_dcSkin, rcPart.left, rcPart.top, SRCCOPY );
				rcBar.left += rcPart.Width();
			}
		}
	}

	if ( m_bsMediaVolTrack )
	{
		float nPosition = (float)Settings.MediaPlayer.Volume;
		CRect rcTrack( &m_rcsMediaVolTrack ), rcPart;

		if ( m_bsMediaVolTab )
		{
			rcPart.CopyRect( &m_rcsMediaVolTab );

			if ( m_bMediaVol || m_nMediaVol >= 0 )
				rcPart.OffsetRect( rcPart.Width(), 0 );

			m_rcMediaVolTab.left	= rcTrack.left + (int)( nPosition * ( rcTrack.Width() - rcPart.Width() ) );
			m_rcMediaVolTab.right	= m_rcMediaVolTab.left + rcPart.Width();
			m_rcMediaVolTab.top		= rcTrack.top;
			m_rcMediaVolTab.bottom	= rcTrack.top + rcPart.Height();

			pDC->BitBlt( m_rcMediaVolTab.left, m_rcMediaVolTab.top, m_rcMediaVolTab.Width(),
				m_rcMediaVolTab.Height(), &m_pSkin->m_dcSkin, rcPart.left, rcPart.top, SRCCOPY );
			pDC->ExcludeClipRect( &m_rcMediaVolTab );

			rcTrack.right = m_rcMediaVolTab.left;
		}

		if ( m_bsMediaVolBar )
		{
			rcPart.CopyRect( &m_rcsMediaVolBar );
			rcTrack.right = min( rcTrack.right, rcTrack.left + rcPart.Width() );
			pDC->BitBlt( rcTrack.left, rcTrack.top, rcTrack.Width(),
				rcPart.Height(), &m_pSkin->m_dcSkin, rcPart.left, rcPart.top, SRCCOPY );
		}
	}

	if ( m_bsStatusText && CMediaFrame::GetMediaFrame() != NULL && ! m_bStatus )
	{
		m_bStatus |= CMediaFrame::GetMediaFrame()->PaintStatusMicro( *pDC, m_rcsStatusText );
	}

	if ( CMediaFrame::GetMediaFrame() != NULL )
	{
		MediaState nState = CMediaFrame::GetMediaFrame()->GetState();
		int nImage = 0;

		if ( nState >= smsPlaying )
			nImage = 1;
		else if ( nState >= smsPaused )
			nImage = 2;

		if ( m_bsMediaStates[0][nImage] && m_bsMediaStates[1][nImage] )
		{
			CRect* prcAnchor = &m_rcsMediaStates[0][nImage];
			CRect* prcPart = &m_rcsMediaStates[1][nImage];

			pDC->BitBlt( prcAnchor->left, prcAnchor->top,
				prcAnchor->Width(), prcAnchor->Height(), &m_pSkin->m_dcSkin,
				prcPart->left, prcPart->top, SRCCOPY );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd paint status functionality

void CRemoteWnd::PaintStatus(CDC* pDC)
{
	if ( m_bsStatusText && ! m_bStatus )
	{
		if ( m_pSkin->m_crCaptionOutline != CLR_NONE )
		{
			pDC->SetTextColor( m_pSkin->m_crCaptionOutline );
			m_rcsStatusText.OffsetRect( 1 , 0 );
			pDC->DrawText( m_sStatus, m_rcsStatusText,
			DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS );
			m_rcsStatusText.OffsetRect( 0 , 1 );
			pDC->DrawText( m_sStatus, m_rcsStatusText,
			DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS );
			m_rcsStatusText.OffsetRect( -1 , 0 );
			pDC->DrawText( m_sStatus, m_rcsStatusText,
			DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS );
			m_rcsStatusText.OffsetRect( -1 , 0 );
			pDC->DrawText( m_sStatus, m_rcsStatusText,
			DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS );
			m_rcsStatusText.OffsetRect( 0 , -1 );
			pDC->DrawText( m_sStatus, m_rcsStatusText,
			DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS );
			m_rcsStatusText.OffsetRect( 0 , -1 );
			pDC->DrawText( m_sStatus, m_rcsStatusText,
			DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS );
			m_rcsStatusText.OffsetRect( 1 , 0 );
			pDC->DrawText( m_sStatus, m_rcsStatusText,
			DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS );
			m_rcsStatusText.OffsetRect( 1 , 0 );
			pDC->DrawText( m_sStatus, m_rcsStatusText,
			DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS );
			m_rcsStatusText.OffsetRect( -1 , 1 );
			pDC->SetTextColor( m_pSkin->m_crCaptionText );
		}

		if ( m_pSkin->m_crCaptionShadow != CLR_NONE )
		{
			pDC->SetTextColor( m_pSkin->m_crCaptionShadow );
			m_rcsStatusText.OffsetRect( 1 , 1 );
			pDC->DrawText( m_sStatus, &m_rcsStatusText,
			DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS );
			m_rcsStatusText.OffsetRect( -1 , -1 );
			pDC->SetTextColor( m_pSkin->m_crCaptionText );
		}

		pDC->DrawText( m_sStatus, &m_rcsStatusText,
			DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS );

		if ( Settings.General.LanguageRTL )
		{
			CRect rcSrc;
			rcSrc.top = 0; rcSrc.left = 0;
			rcSrc.right = m_rcsStatusText.Width();
			rcSrc.bottom = m_rcsStatusText.Height();

			pDC->StretchBlt( rcSrc.Width() + m_rcsStatusText.left,
				m_rcsStatusText.top, -rcSrc.Width(), rcSrc.Height(),
				pDC, m_rcsStatusText.left, m_rcsStatusText.top,
				rcSrc.Width(), rcSrc.Height(), SRCCOPY );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd mouse interaction

void CRemoteWnd::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS* /*lpncsp*/)
{
}

LRESULT CRemoteWnd::OnNcHitTest(CPoint /*point*/)
{
	return HTCLIENT;
}

BOOL CRemoteWnd::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	CPoint point;
	GetCursorPos( &point );
	ScreenToClient( &point );

	BOOL bAvailable = CMediaFrame::GetMediaFrame() != NULL;

	if ( m_bsScalerTrack && m_rcScalerTab.PtInRect( point ) )
	{
		if ( ! m_bScaler ) Invalidate();
		m_bScaler = TRUE;
	}
	else if ( bAvailable && m_bsMediaSeekTrack && m_rcMediaSeekTab.PtInRect( point ) )
	{
		if ( ! m_bMediaSeek ) Invalidate();
		m_bMediaSeek = TRUE;
	}
	else if ( bAvailable && m_bsMediaVolTrack && m_rcMediaVolTab.PtInRect( point ) )
	{
		if ( ! m_bMediaVol ) Invalidate();
		m_bMediaVol = TRUE;
	}
	else if ( m_bScaler || m_bMediaSeek || m_bMediaVol )
	{
		m_bScaler = m_bMediaSeek = m_bMediaVol = FALSE;
		Invalidate();
	}

	if ( HitTestButtons( point ) != NULL )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}
	else if ( m_bsScalerTrack && m_rcsScalerTrack.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}
	else if ( bAvailable && m_bsMediaSeekTrack && m_rcsMediaSeekTrack.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}
	else if ( bAvailable && m_bsMediaVolTrack && m_rcsMediaVolTrack.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}
    else
	{
		SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
		return TRUE;
	}
}

void CRemoteWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	CmdButton* pButton = HitTestButtons( point );

	if ( m_pCmdHover != pButton )
	{
		m_pCmdHover = pButton;
		Invalidate();
	}

	if ( m_pCmdDown == NULL ) CWnd::OnMouseMove(nFlags, point);
}

void CRemoteWnd::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetActiveWindow();
	SetFocus();

	if ( CmdButton* pButton = HitTestButtons( point ) )
	{
		m_pCmdDown = m_pCmdHover = pButton;
		Invalidate();
		SetCapture();
	}
	else if ( m_bsScalerTrack && m_rcsScalerTrack.PtInRect( point ) )
	{
		TrackScaler();
	}
	else if ( m_bsMediaSeekTrack && m_rcsMediaSeekTrack.PtInRect( point ) )
	{
		TrackSeek();
	}
	else if ( m_bsMediaVolTrack && m_rcsMediaVolTrack.PtInRect( point ) )
	{
		TrackVol();
	}
	else
	{
		ClientToScreen( &point );
		SendMessage( WM_NCLBUTTONDOWN, HTCAPTION, MAKELONG( point.x, point.y ) );
	}
}

void CRemoteWnd::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	if ( m_pCmdDown != NULL )
	{
		BOOL bOK = ( m_pCmdDown == m_pCmdHover );

		m_pCmdDown = NULL;
		Invalidate();
		ReleaseCapture();

		if ( bOK )
		{
			UpdateWindow();
			m_pCmdHover->Execute( (CFrameWnd*)AfxGetMainWnd() );
			UpdateCmdButtons();
		}
	}
	else
	{
		ClientToScreen( &point );
		SendMessage( WM_NCLBUTTONUP, HTCAPTION, MAKELONG( point.x, point.y ) );
	}
}

void CRemoteWnd::OnNcLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	if ( m_bsHistoryDest && m_rcsHistoryDest.PtInRect( point ) )
	{
		PostMainWndMessage( WM_COMMAND, ID_TAB_HOME );
	}
	else if ( m_bsStatusText && m_rcsStatusText.PtInRect( point ) )
	{
		PostMainWndMessage( WM_COMMAND, ID_TAB_MEDIA );
	}
	else
	{
		PostMainWndMessage( WM_COMMAND, ID_TRAY_OPEN );
	}
}

void CRemoteWnd::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	CMenu* pMenu = Skin.GetMenu( _T("CRemoteWnd") );
	if ( pMenu == NULL ) pMenu = Skin.GetMenu( _T("CMainWnd.Tray") );
	if ( pMenu == NULL ) return;

	MENUITEMINFO pInfo;
	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= MIIM_STATE;
	GetMenuItemInfo( pMenu->GetSafeHmenu(), ID_TRAY_OPEN, FALSE, &pInfo );
	pInfo.fState	|= MFS_DEFAULT;
	SetMenuItemInfo( pMenu->GetSafeHmenu(), ID_TRAY_OPEN, FALSE, &pInfo );

	ClientToScreen( &point );
	pMenu->TrackPopupMenu( TPM_CENTERALIGN|TPM_TOPALIGN|TPM_RIGHTBUTTON,
		point.x, point.y, AfxGetMainWnd(), NULL );
}

INT_PTR CRemoteWnd::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	if ( CmdButton* pButton = HitTestButtons( point ) )
	{
		CString strTip;

		if ( LoadString( strTip, pButton->m_nID ) )
		{
			if ( LPCTSTR pszBreak = _tcschr( strTip, '\n' ) )
			{
				pTI->lpszText = _tcsdup( pszBreak + 1 );
			}
			else
			{
				strTip = strTip.SpanExcluding( _T(".") );
				pTI->lpszText = _tcsdup( strTip );
			}
		}

		pTI->hwnd		= GetSafeHwnd();
		pTI->uId		= pButton->m_nID;
		pTI->uFlags		= pTI->uFlags & ~TTF_IDISHWND;
		pTI->rect		= pButton->m_rc;

		return pTI->uId;
	}

	return CWnd::OnToolHitTest( point, pTI );
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd trackers

void CRemoteWnd::TrackScaler()
{
	MSG* pMsg = &AfxGetThreadState()->m_msgCur;
	CRect rcTrack( &m_rcsScalerTrack );
	CPoint point;

	ClientToScreen( &rcTrack );
	ClipCursor( &rcTrack );
	ScreenToClient( &rcTrack );
	SetCapture();

	rcTrack.DeflateRect( m_rcScalerTab.Width() / 2, 0 );

	while ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
	{
		while ( ::PeekMessage( pMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) );

		if ( ! AfxGetThread()->PumpMessage() )
		{
			AfxPostQuitMessage( 0 );
			break;
		}

		GetCursorPos( &point );
		ScreenToClient( &point );

		int nPosition = (int)( 105.0f * (float)( point.x - rcTrack.left ) / (float)rcTrack.Width() );
		if ( nPosition < 0 ) nPosition = 0;
		else if ( nPosition >= 102 ) nPosition = 101;
		else if ( nPosition >= 100 ) nPosition = 100;

		if ( nPosition != (int)Settings.Live.BandwidthScale )
		{
			Settings.Live.BandwidthScale = (DWORD)nPosition;
			Invalidate();
		}
	}

	ReleaseCapture();
	ClipCursor( NULL );
	Invalidate();
}

void CRemoteWnd::TrackSeek()
{
	MSG* pMsg = &AfxGetThreadState()->m_msgCur;
	CRect rcTrack( &m_rcsMediaSeekTrack );
	CPoint point;

	if ( CMediaFrame::GetMediaFrame() == NULL ) return;

	ClientToScreen( &rcTrack );
	ClipCursor( &rcTrack );
	ScreenToClient( &rcTrack );
	SetCapture();

	rcTrack.DeflateRect( m_rcMediaSeekTab.Width() / 2, 0 );

	while ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
	{
		while ( ::PeekMessage( pMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) );

		if ( ! AfxGetThread()->PumpMessage() )
		{
			AfxPostQuitMessage( 0 );
			break;
		}

		GetCursorPos( &point );
		ScreenToClient( &point );

		float nPosition = (float)( point.x - rcTrack.left ) / (float)rcTrack.Width();
		if ( nPosition < 0.0f ) nPosition = 0.0f;
		if ( nPosition > 1.0f ) nPosition = 1.0f;

		if ( nPosition != m_nMediaSeek )
		{
			m_nMediaSeek = nPosition;
			CMediaFrame::GetMediaFrame()->SeekTo( m_nMediaSeek );
			Invalidate();
		}
	}

	ReleaseCapture();
	ClipCursor( NULL );

	m_nMediaSeek = -1;
	Invalidate();
}

void CRemoteWnd::TrackVol()
{
	MSG* pMsg = &AfxGetThreadState()->m_msgCur;
	CRect rcTrack( &m_rcsMediaVolTrack );
	CPoint point;

	if ( CMediaFrame::GetMediaFrame() == NULL ) return;

	ClientToScreen( &rcTrack );
	ClipCursor( &rcTrack );
	ScreenToClient( &rcTrack );
	SetCapture();

	rcTrack.DeflateRect( m_rcMediaVolTab.Width() / 2, 0 );

	while ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
	{
		while ( ::PeekMessage( pMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) );

		if ( ! AfxGetThread()->PumpMessage() )
		{
			AfxPostQuitMessage( 0 );
			break;
		}

		GetCursorPos( &point );
		ScreenToClient( &point );

		float nPosition = (float)( point.x - rcTrack.left ) / (float)rcTrack.Width();
		if ( nPosition < 0.0f ) nPosition = 0.0f;
		if ( nPosition > 1.0f ) nPosition = 1.0f;

		if ( nPosition != m_nMediaVol )
		{
			m_nMediaVol = nPosition;
			CMediaFrame::GetMediaFrame()->SetVolume( m_nMediaVol );
			Invalidate();
		}
	}

	ReleaseCapture();
	ClipCursor( NULL );

	m_nMediaVol = -1;
	Invalidate();
}


/////////////////////////////////////////////////////////////////////////////
// CRemoteWnd::CmdButton construction

CRemoteWnd::CmdButton::CmdButton(LPCTSTR pszName) : m_rc( 0, 0, 0, 0 )
{
	m_nID		= CoolInterface.NameToID( pszName + 1 );
	m_sName		= pszName;
	m_bVisible	= TRUE;
	m_bEnabled	= TRUE;
	m_bChecked	= FALSE;
	m_bChanged	= FALSE;
	if ( m_nID == 0 ) m_nID = 1;
}

BOOL CRemoteWnd::CmdButton::HitTest(const CPoint& point, BOOL bAll) const
{
	return ( bAll || m_bEnabled ) && m_rc.PtInRect( point );
}

void CRemoteWnd::CmdButton::Show(BOOL bShow)
{
	if ( m_bVisible == bShow ) return;
	m_bVisible = bShow;
	m_bChanged = TRUE;
}

void CRemoteWnd::CmdButton::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	if ( m_bEnabled == bOn ) return;
	m_bEnabled = bOn;
	m_bChanged = TRUE;
}

void CRemoteWnd::CmdButton::SetCheck(int nCheck)
{
	if ( m_bChecked == ( nCheck != 0 ) ) return;
	m_bChecked = ( nCheck != 0 );
	m_bChanged = TRUE;
}

void CRemoteWnd::CmdButton::Execute(CFrameWnd* pTarget)
{
	if ( m_bVisible && m_bEnabled )
	{
		pTarget->SendMessage( WM_COMMAND, m_nID );
	}
}

void CRemoteWnd::CmdButton::Paint(CDC* pdcWindow, CRect& rcWindow, CSkinWindow* pSkin, CmdButton* pHover, CmdButton* pDown)
{
	pSkin->GetAnchor( m_sName, m_rc );

	if ( ! m_bVisible )
	{
		return;
	}
	else if ( m_bChecked )
	{
		if ( ! pSkin->PaintPartOnAnchor( pdcWindow, rcWindow, m_sName + _T(".Checked"), m_sName ) )
			pSkin->PaintPartOnAnchor( pdcWindow, rcWindow, m_sName + _T(".Down"), m_sName );
	}
	else if ( ! m_bEnabled )
	{
		pSkin->PaintPartOnAnchor( pdcWindow, rcWindow, m_sName + _T(".Disabled"), m_sName );
	}
	else if ( pHover == this && pDown == this )
	{
		pSkin->PaintPartOnAnchor( pdcWindow, rcWindow, m_sName + _T(".Down"), m_sName );
	}
	else if ( ( pHover == this && pDown == NULL ) || ( pDown == this ) )
	{
		pSkin->PaintPartOnAnchor( pdcWindow, rcWindow, m_sName + _T(".Hover"), m_sName );
	}
	else
	{
		pSkin->PaintPartOnAnchor( pdcWindow, rcWindow, m_sName + _T(".Up"), m_sName );
	}
}

BOOL CRemoteWnd::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	return FALSE;
}
