//
// CtrlUploadTip.cpp
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
#include "CoolInterface.h"
#include "Transfers.h"
#include "UploadFile.h"
#include "UploadFiles.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "UploadTransfer.h"
#include "GraphLine.h"
#include "GraphItem.h"
#include "Flags.h"
#include "FragmentedFile.h"
#include "FragmentBar.h"
#include "CtrlUploadTip.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CUploadTipCtrl, CCoolTipCtrl)

BEGIN_MESSAGE_MAP(CUploadTipCtrl, CCoolTipCtrl)
	ON_WM_TIMER()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CUploadTipCtrl construction

CUploadTipCtrl::CUploadTipCtrl()
	: m_pUploadFile	( NULL )
	, m_pGraph		( NULL )
	, m_pItem		( NULL )
	, m_nHeaderWidth( 0 )
	, m_nValueWidth	( 0 )
	, m_nHeaders	( 0 )
{
}

CUploadTipCtrl::~CUploadTipCtrl()
{
	delete m_pGraph;
}

/////////////////////////////////////////////////////////////////////////////
// CUploadTipCtrl events

BOOL CUploadTipCtrl::OnPrepare()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return FALSE;

	CalcSizeHelper();

	return ( m_sz.cx > 0 );
}

void CUploadTipCtrl::OnCalcSize(CDC* pDC)
{
	if ( m_pUploadFile && UploadFiles.Check( m_pUploadFile ) )
	{
		OnCalcSize( pDC, m_pUploadFile->GetActive() );
	}
	m_sz.cx = min( max( m_sz.cx, 400l ), (LONG)GetSystemMetrics( SM_CXSCREEN ) / 2 );
}

void CUploadTipCtrl::OnShow()
{
	delete m_pGraph;

	m_pGraph	= CreateLineGraph();
	m_pItem		= new CGraphItem( 0, 1.0f, RGB( 0xFF, 0, 0 ) );

	m_pGraph->AddItem( m_pItem );
}

void CUploadTipCtrl::OnHide()
{
	delete m_pGraph;
	m_pGraph = NULL;
	m_pItem = NULL;
}

void CUploadTipCtrl::OnPaint(CDC* pDC)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	if ( m_pUploadFile && UploadFiles.Check( m_pUploadFile ) )
	{
		OnPaint( pDC, m_pUploadFile->GetActive() );
	}
	else
	{
		Hide();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl upload transfer case

void CUploadTipCtrl::OnCalcSize(CDC* pDC, CUploadTransfer* pUpload)
{
	if ( pUpload->m_sRemoteNick.GetLength() > 0 )
		m_sAddress = pUpload->m_sRemoteNick + _T(" (") +
			CString( inet_ntoa( pUpload->m_pHost.sin_addr ) ) + _T(")");
	else
		m_sAddress = inet_ntoa( pUpload->m_pHost.sin_addr );

	AddSize( pDC, m_pUploadFile->m_sName );
	AddSize( pDC, m_sAddress );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	AddSize( pDC, pUpload->m_sCountryName );

	m_sz.cy += TIP_TEXTHEIGHT * 3 + 4;
	m_sz.cy += TIP_RULE;
	m_sz.cy += TIP_TEXTHEIGHT * 3;
	m_sz.cy += TIP_GAP;
	m_sz.cy += 16;
	m_sz.cy += TIP_GAP;
	m_sz.cy += 40;
	m_sz.cy += TIP_GAP;

	m_nValueWidth = 0;
	m_nHeaderWidth = 0;
	m_nHeaders = 0;
	if ( Settings.General.GUIMode != GUI_BASIC )
	{
		m_nHeaders = (int)pUpload->m_pHeaderName.GetSize();
		for ( int nHeader = 0 ; nHeader < m_nHeaders ; nHeader++ )
		{
			CString strName		= pUpload->m_pHeaderName.GetAt( nHeader ) + _T(':');
			CString strValue	= pUpload->m_pHeaderValue.GetAt( nHeader );
			CSize szKey			= pDC->GetTextExtent( strName );
			CSize szValue		= pDC->GetTextExtent( strValue );
			m_nHeaderWidth		= max( m_nHeaderWidth, (int)szKey.cx );
			m_nValueWidth		= max( m_nValueWidth, (int)szValue.cx );
			m_sz.cy				+= TIP_TEXTHEIGHT;
		}
	}

	if ( m_nHeaderWidth ) m_nHeaderWidth += TIP_GAP;
	m_sz.cx = max( m_sz.cx, (LONG)m_nHeaderWidth + m_nValueWidth );
}

void CUploadTipCtrl::OnPaint(CDC* pDC, CUploadTransfer* pUpload)
{
	CPoint pt( 0, 0 );
	CSize sz( m_sz.cx, TIP_TEXTHEIGHT );

	DrawText( pDC, &pt, m_pUploadFile->m_sName );
	pt.y += TIP_TEXTHEIGHT;
	DrawText( pDC, &pt, m_sAddress );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	pt.y += TIP_TEXTHEIGHT;

	int nFlagIndex = Flags.GetFlagIndex( pUpload->m_sCountry );
	if ( nFlagIndex >= 0 )
	{
		Flags.Draw( nFlagIndex, pDC->GetSafeHdc(), pt.x, pt.y, CoolInterface.m_crTipBack );
		pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 16, pt.y + 16 );
	}
	pt.x += 16 + 4;
	DrawText( pDC, &pt, pUpload->m_sCountryName );
	pt.x -= 16 + 4;
	pt.y += 16;

	DrawRule( pDC, &pt );

	CString strStatus, strSpeed, strText;
	CString strOf;
	LoadString( strOf, IDS_GENERAL_OF );

	strSpeed.Format( _T("%s %s %s (%s)"),
		(LPCTSTR)Settings.SmartSpeed( pUpload->GetMeasuredSpeed() ),
		(LPCTSTR)strOf,
		(LPCTSTR)Settings.SmartSpeed( pUpload->m_nBandwidth ),
		(LPCTSTR)Settings.SmartSpeed( pUpload->GetMaxSpeed() ) );

	int nQueue = UploadQueues.GetPosition( pUpload, FALSE );
	if ( m_pUploadFile != pUpload->m_pBaseFile || pUpload->m_nState == upsNull )
	{
		LoadString( strStatus, IDS_TIP_INACTIVE );
	}
	else if ( nQueue == 0 )
	{
		if ( pUpload->m_nState == upsQueued )
		{
			LoadString( strText, IDS_TIP_NEXT );
			strStatus.Format( _T("%s: %s"),
				(LPCTSTR)pUpload->m_pQueue->m_sName, (LPCTSTR)strText );
		}
		else
		{
			LoadString( strText, IDS_TIP_ACTIVE );
			strStatus.Format( _T("%s: %s"),
				(LPCTSTR)pUpload->m_pQueue->m_sName, (LPCTSTR)strText );
		}
	}
	else if ( nQueue > 0 )
	{
		strStatus.Format( _T("%s: %i %s %u"),
			(LPCTSTR)pUpload->m_pQueue->m_sName,
			nQueue, (LPCTSTR)strOf, pUpload->m_pQueue->GetQueuedCount() );
	}
	else
	{
		LoadString( strStatus, IDS_TIP_ACTIVE );
	}

	LoadString( strText, IDS_TIP_STATUS );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, strStatus, 80 );
	pt.y += TIP_TEXTHEIGHT;

	LoadString( strText, IDS_TIP_SPEED );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, strSpeed, 80 );
	pt.y += TIP_TEXTHEIGHT;

	LoadString( strText, IDS_TIP_USERAGENT );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, pUpload->m_sUserAgent, 80 );
	pt.y += TIP_TEXTHEIGHT;

	pt.y += TIP_GAP;

	DrawProgressBar( pDC, &pt, m_pUploadFile );
	pt.y += TIP_GAP;

	CRect rc( pt.x, pt.y, m_sz.cx, pt.y + 40 );
	pDC->Draw3dRect( &rc, CoolInterface.m_crTipBorder, CoolInterface.m_crTipBorder );
	rc.DeflateRect( 1, 1 );
	if ( m_pGraph)
		m_pGraph->BufferedPaint( pDC, &rc );
	rc.InflateRect( 1, 1 );
	pDC->ExcludeClipRect( &rc );
	pt.y += 40;
	pt.y += TIP_GAP;

	if ( Settings.General.GUIMode != GUI_BASIC )
	{
		if ( m_nHeaders != pUpload->m_pHeaderName.GetSize() )
		{
			ShowImpl( true );
			return;
		}
		for ( int nHeader = 0 ; nHeader < m_nHeaders ; nHeader++ )
		{
			CString strName = pUpload->m_pHeaderName.GetAt( nHeader ) + _T(':');
			CString strValue = pUpload->m_pHeaderValue.GetAt( nHeader );
			DrawText( pDC, &pt, strName );
			pt.x += m_nHeaderWidth;
			sz.cx -= m_nHeaderWidth;
			DrawText( pDC, &pt, strValue, &sz );
			pt.x -= m_nHeaderWidth;
			sz.cx += m_nHeaderWidth;
			pt.y += TIP_TEXTHEIGHT;
		}
	}
}

void CUploadTipCtrl::DrawProgressBar(CDC* pDC, CPoint* pPoint, CUploadFile* pFile)
{
	CRect rcCell( pPoint->x, pPoint->y, m_sz.cx, pPoint->y + TIP_TEXTHEIGHT );
	pPoint->y += TIP_TEXTHEIGHT;

	pDC->Draw3dRect( &rcCell, CoolInterface.m_crTipBorder, CoolInterface.m_crTipBorder );
	rcCell.DeflateRect( 1, 1 );

	CFragmentBar::DrawUpload( pDC, &rcCell, pFile, CoolInterface.m_crTipBack );

	rcCell.InflateRect( 1, 1 );
	pDC->ExcludeClipRect( &rcCell );
}

/////////////////////////////////////////////////////////////////////////////
// CUploadTipCtrl message handlers

void CUploadTipCtrl::OnTimer(UINT_PTR nIDEvent)
{
	CCoolTipCtrl::OnTimer( nIDEvent );

	if ( m_pGraph == NULL ) return;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 10 ) ) return;

	if ( ! m_pUploadFile || ! UploadFiles.Check( m_pUploadFile ) )
	{
		Hide();
		return;
	}

	if ( CUploadTransfer* pUpload = m_pUploadFile->GetActive() )
	{
		DWORD nSpeed = pUpload->GetMeasuredSpeed();
		m_pItem->Add( nSpeed );
		m_pGraph->m_nUpdates++;
		m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nSpeed );
		Invalidate( FALSE );
	}
}
