//
// CtrlUploadTip.cpp
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
#include "CoolInterface.h"
#include "Transfers.h"
#include "UploadFile.h"
#include "UploadFiles.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "UploadTransfer.h"
#include "GraphLine.h"
#include "GraphItem.h"
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
	//{{AFX_MSG_MAP(CUploadTipCtrl)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CUploadTipCtrl construction

CUploadTipCtrl::CUploadTipCtrl()
{
	m_pGraph = NULL;
}

CUploadTipCtrl::~CUploadTipCtrl()
{
	if ( m_pGraph ) delete m_pGraph;
}

/////////////////////////////////////////////////////////////////////////////
// CUploadTipCtrl events

BOOL CUploadTipCtrl::OnPrepare()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return FALSE;

	CalcSizeHelper();
	
	return m_sz.cx > 0;
}

void CUploadTipCtrl::OnShow()
{
	if ( m_pGraph ) delete m_pGraph;

	m_pGraph	= CreateLineGraph();
	m_pItem		= new CGraphItem( 0, 0, RGB( 0xFF, 0, 0 ) );

	m_pGraph->AddItem( m_pItem );
}

void CUploadTipCtrl::OnHide()
{
	if ( m_pGraph ) delete m_pGraph;
	m_pGraph = NULL;
	m_pItem = NULL;
}

void CUploadTipCtrl::OnCalcSize(CDC* pDC)
{
	CUploadFile* pFile = (CUploadFile*)m_pContext;
	if ( ! UploadFiles.Check( pFile ) ) return;
	CUploadTransfer* pUpload = pFile->GetActive();
	
	if ( pUpload->m_sNick.GetLength() > 0 )
		m_sAddress = pUpload->m_sNick + _T(" (") + inet_ntoa( pUpload->m_pHost.sin_addr ) + ')';
	else
		m_sAddress = inet_ntoa( pUpload->m_pHost.sin_addr );
	
	m_pHeaderName.RemoveAll();
	m_pHeaderValue.RemoveAll();
	
	if ( Settings.General.GUIMode != GUI_BASIC )
	{
		for ( int nHeader = 0 ; nHeader < pUpload->m_pHeaderName.GetSize() ; nHeader++ )
		{
			CString strName		= pUpload->m_pHeaderName.GetAt( nHeader );
			CString strValue	= pUpload->m_pHeaderValue.GetAt( nHeader );
			
			if ( strValue.GetLength() > 64 ) strValue = strValue.Left( 64 ) + _T("...");
			
			m_pHeaderName.Add( strName );
			m_pHeaderValue.Add( strValue );
		}
	}
	
	AddSize( pDC, pFile->m_sName );
	AddSize( pDC, m_sAddress );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	
	m_sz.cy += TIP_TEXTHEIGHT * 2;
	m_sz.cy += TIP_RULE;
	m_sz.cy += TIP_TEXTHEIGHT * 3;
	m_sz.cy += TIP_GAP;
	m_sz.cy += TIP_TEXTHEIGHT;
	m_sz.cy += TIP_GAP;
	m_sz.cy += 40;
	m_sz.cy += TIP_GAP;
	
	int nValueWidth = 0;
	m_nHeaderWidth = 0;
	
	for ( int nHeader = 0 ; nHeader < m_pHeaderName.GetSize() ; nHeader++ )
	{
		CString strName		= m_pHeaderName.GetAt( nHeader );
		CString strValue	= m_pHeaderValue.GetAt( nHeader );
		CSize szKey			= pDC->GetTextExtent( strName + ':' );
		CSize szValue		= pDC->GetTextExtent( strValue );
		
		m_nHeaderWidth		= max( m_nHeaderWidth, szKey.cx );
		nValueWidth			= max( nValueWidth, szValue.cx );
		
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	
	if ( m_nHeaderWidth ) m_nHeaderWidth += TIP_GAP;
	m_sz.cx = max( m_sz.cx, m_nHeaderWidth + nValueWidth );
	
	m_sz.cx = max( m_sz.cx, 320 );
}

void CUploadTipCtrl::OnPaint(CDC* pDC)
{
	CString strText, strOf;
	LoadString( strOf, IDS_GENERAL_OF );

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;
	
	CUploadFile* pFile = (CUploadFile*)m_pContext;
	
	if ( ! UploadFiles.Check( pFile ) )
	{
		Hide();
		return;
	}
	
	CUploadTransfer* pUpload = pFile->GetActive();
	
	CPoint pt( 0, 0 );
	
	DrawText( pDC, &pt, pFile->m_sName );
	pt.y += TIP_TEXTHEIGHT;
	DrawText( pDC, &pt, m_sAddress );
	pt.y += TIP_TEXTHEIGHT;
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	
	DrawRule( pDC, &pt );
	
	int nQueue = UploadQueues.GetPosition( pUpload, FALSE );
	CString strStatus, strSpeed;
	
	if ( pUpload->m_nProtocol != PROTOCOL_BT )
	{
		strSpeed.Format( _T("%s %s %s"),
			(LPCTSTR)Settings.SmartVolume( pUpload->GetMeasuredSpeed() * 8, FALSE, TRUE ),
			strOf, (LPCTSTR)Settings.SmartVolume( pUpload->m_nBandwidth * 8, FALSE, TRUE ) );
	}
	else
	{
		strSpeed = Settings.SmartVolume( pUpload->GetMeasuredSpeed() * 8, FALSE, TRUE );
	}
	
	if ( pFile != pUpload->m_pBaseFile || pUpload->m_nState == upsNull )
	{
		LoadString( strStatus, IDS_TIP_INACTIVE );
	}
	else if ( nQueue == 0 )
	{
		if ( pUpload->m_nState == upsQueued )
		{
			LoadString( strText, IDS_TIP_NEXT );
			strStatus.Format( _T("%s: %s"), 
				(LPCTSTR)pUpload->m_pQueue->m_sName, strText );
		}
		else
		{
			LoadString( strText, IDS_TIP_ACTIVE );
			strStatus.Format( _T("%s: %s"), 
				(LPCTSTR)pUpload->m_pQueue->m_sName, strText );
		}
	}
	else if ( nQueue > 0 )
	{
		strStatus.Format( _T("%s: %i %s %i"),
			(LPCTSTR)pUpload->m_pQueue->m_sName,
			nQueue, strOf, pUpload->m_pQueue->GetQueuedCount() );
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
	
	DrawProgressBar( pDC, &pt, pFile );
	pt.y += TIP_GAP;
	
	CRect rc( pt.x, pt.y, m_sz.cx, pt.y + 40 );
	pDC->Draw3dRect( &rc, CoolInterface.m_crTipBorder, CoolInterface.m_crTipBorder );
	rc.DeflateRect( 1, 1 );
	m_pGraph->BufferedPaint( pDC, &rc );
	rc.InflateRect( 1, 1 );
	pDC->ExcludeClipRect( &rc );
	pt.y += 40;
	pt.y += TIP_GAP;
	
	for ( int nHeader = 0 ; nHeader < m_pHeaderName.GetSize() ; nHeader++ )
	{
		CString strName		= m_pHeaderName.GetAt( nHeader );
		CString strValue	= m_pHeaderValue.GetAt( nHeader );
		
		DrawText( pDC, &pt, strName + ':' );
		DrawText( pDC, &pt, strValue, m_nHeaderWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
}

void CUploadTipCtrl::DrawProgressBar(CDC* pDC, CPoint* pPoint, CUploadFile* pFile)
{
	CUploadTransfer* pUpload = pFile->GetActive();

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

void CUploadTipCtrl::OnTimer(UINT nIDEvent) 
{
	CCoolTipCtrl::OnTimer( nIDEvent );
	
	if ( m_pGraph == NULL ) return;
	
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 10 ) ) return;
	
	CUploadFile* pFile = (CUploadFile*)m_pContext;
	
	if ( pFile == NULL || ! UploadFiles.Check( pFile ) )
	{
		Hide();
		return;
	}
	
	if ( CUploadTransfer* pUpload = pFile->GetActive() )
	{
		DWORD nSpeed = pUpload->GetMeasuredSpeed() * 8;
		m_pItem->Add( nSpeed );
		m_pGraph->m_nUpdates++;
		m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nSpeed );
		Invalidate();
	}	
}
