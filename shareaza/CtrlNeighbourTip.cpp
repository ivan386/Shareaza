//
// CtrlNeighbourTip.cpp : implementation file
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

#include "Stdafx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "Network.h"
#include "GProfile.h"
#include "CoolInterface.h"
#include "CtrlNeighbourTip.h"
#include "GraphLine.h"
#include "GraphItem.h"

#include "EDPacket.h"
#include "EDNeighbour.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CNeighbourTipCtrl, CCoolTipCtrl)
	//{{AFX_MSG_MAP(CNeighbourTipCtrl)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNeighbourTipCtrl construction

CNeighbourTipCtrl::CNeighbourTipCtrl()
{
	m_pGraph = NULL;
}

CNeighbourTipCtrl::~CNeighbourTipCtrl()
{
	if ( m_pGraph ) delete m_pGraph;
}

/////////////////////////////////////////////////////////////////////////////
// CNeighbourTipCtrl prepare

BOOL CNeighbourTipCtrl::OnPrepare()
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return FALSE;

	CNeighbour* pNeighbour = Neighbours.Get( (DWORD)m_pContext );
	if ( pNeighbour == NULL ) return FALSE;
	
	CalcSizeHelper();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CNeighbourTipCtrl show and hide

void CNeighbourTipCtrl::OnShow()
{
	if ( m_pGraph ) delete m_pGraph;

	m_pGraph	= CreateLineGraph();
	m_pItemIn	= new CGraphItem( 0, 0, RGB( 0, 0, 0xFF ) );
	m_pItemOut	= new CGraphItem( 0, 0, RGB( 0xFF, 0, 0 ) );

	m_pGraph->AddItem( m_pItemIn );
	m_pGraph->AddItem( m_pItemOut );
}

void CNeighbourTipCtrl::OnHide()
{
	if ( m_pGraph ) delete m_pGraph;
	m_pGraph = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CNeighbourTipCtrl size

void CNeighbourTipCtrl::OnCalcSize(CDC* pDC)
{
	CNeighbour* pNeighbour = Neighbours.Get( (DWORD)m_pContext );
	CString str;

	if ( pNeighbour->m_pProfile && pNeighbour->m_pProfile->IsValid() )
	{
		str = pNeighbour->m_pProfile->GetNick();
		if ( str.GetLength() )
		{
			pDC->SelectObject( &CoolInterface.m_fntBold );
			AddSize( pDC, str );
			m_sz.cy += TIP_TEXTHEIGHT;
		}

		str = pNeighbour->m_pProfile->GetLocation();
		if ( str.GetLength() )
		{
			pDC->SelectObject( &CoolInterface.m_fntNormal );
			AddSize( pDC, str );
			m_sz.cy += TIP_TEXTHEIGHT;
		}

		m_sz.cy += TIP_RULE;
	}
	else if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K )
	{
		str = ((CEDNeighbour*)pNeighbour)->m_sServerName;
		
		if ( str.GetLength() )
		{
			pDC->SelectObject( &CoolInterface.m_fntNormal );
			AddSize( pDC, str );
			m_sz.cy += TIP_TEXTHEIGHT;
			m_sz.cy += TIP_RULE;
		}
	}
	
	pDC->SelectObject( &CoolInterface.m_fntBold );
	AddSize( pDC, pNeighbour->m_sAddress );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	m_sz.cy += TIP_TEXTHEIGHT;
	
	if ( pNeighbour->m_sUserAgent.GetLength() )
	{
		AddSize( pDC, pNeighbour->m_sUserAgent );
		m_sz.cy += TIP_TEXTHEIGHT;
	}

	m_sz.cy += TIP_TEXTHEIGHT;
	m_sz.cy += TIP_RULE;
	
	m_sz.cy += TIP_TEXTHEIGHT * 6;
	
	m_sz.cx = max( m_sz.cx, 128 + 160 );
	m_sz.cy += 40;
}

/////////////////////////////////////////////////////////////////////////////
// CNeighbourTipCtrl paint

void CNeighbourTipCtrl::OnPaint(CDC* pDC)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	CNeighbour* pNeighbour = Neighbours.Get( (DWORD)m_pContext );
	if ( pNeighbour == NULL ) return;

	CPoint pt( 0, 0 );
	CString str;

	if ( pNeighbour->m_pProfile != NULL && pNeighbour->m_pProfile->IsValid() )
	{
		str = pNeighbour->m_pProfile->GetNick();
		if ( str.GetLength() )
		{
			pDC->SelectObject( &CoolInterface.m_fntBold );
			DrawText( pDC, &pt, str );
			pt.y += TIP_TEXTHEIGHT;
		}
		pDC->SelectObject( &CoolInterface.m_fntNormal );
		str = pNeighbour->m_pProfile->GetLocation();
		if ( str.GetLength() )
		{
			DrawText( pDC, &pt, str );
			pt.y += TIP_TEXTHEIGHT;
		}
		DrawRule( pDC, &pt );
	}
	else if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K )
	{
		str = ((CEDNeighbour*)pNeighbour)->m_sServerName;
		
		if ( str.GetLength() )
		{
			pDC->SelectObject( &CoolInterface.m_fntBold );
			DrawText( pDC, &pt, str );
			pt.y += TIP_TEXTHEIGHT;
			DrawRule( pDC, &pt );
		}
	}

	pDC->SelectObject( &CoolInterface.m_fntBold );
	DrawText( pDC, &pt, pNeighbour->m_sAddress );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	pt.y += TIP_TEXTHEIGHT;
	
	switch ( pNeighbour->m_nProtocol )
	{
	case PROTOCOL_G1:
		switch ( pNeighbour->m_nNodeType )
		{
		case ntNode:
			str = _T("Gnutella: Peer-to-Peer Connection");
			break;
		case ntHub:
			str = _T("Gnutella: Leaf-to-Ultrapeer Connection");
			break;
		case ntLeaf:
			str = _T("Gnutella: Ultrapeer-to-Leaf Connection");
			break;
		}
		break;
	case PROTOCOL_G2:
		switch ( pNeighbour->m_nNodeType )
		{
		case ntNode:
			str = _T("Gnutella II: Hub-to-Hub Connection");
			break;
		case ntHub:
			str = _T("Gnutella II: Leaf-to-Hub Connection");
			break;
		case ntLeaf:
			str = _T("Gnutella II: Hub-to-Leaf Connection");
			break;
		}
		break;
	case PROTOCOL_ED2K:
		if ( CEDNeighbour* pED = (CEDNeighbour*)pNeighbour )
		{
			if ( CEDPacket::IsLowID( pED->m_nClientID ) )
			{
				str.Format( _T("eDonkey Server Connection (LOWID: %lu)"), pED->m_nClientID );
			}
			else
			{
				str = _T("eDonkey2000 Server Connection");
			}
		}
		break;
	default:
		str = _T("Handshake");
		break;
	}
	
	if ( pNeighbour->m_sUserAgent.GetLength() )
	{
		DrawText( pDC, &pt, pNeighbour->m_sUserAgent );
		pt.y += TIP_TEXTHEIGHT;
	}

	DrawText( pDC, &pt, str );
	pt.y += TIP_TEXTHEIGHT;

	DrawRule( pDC, &pt );

	pDC->SelectObject( &CoolInterface.m_fntBold );
	pDC->SetTextColor( m_pItemIn->m_nColour );
	DrawText( pDC, &pt, _T("Inbound"), 128 );
	pDC->SetTextColor( m_pItemOut->m_nColour );
	DrawText( pDC, &pt, _T("Outbound"), 128 + 80 );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	pDC->SetTextColor( 0 );
	
	pt.y += TIP_TEXTHEIGHT;

	DrawText( pDC, &pt, _T("Current:") );
	str = Settings.SmartVolume( pNeighbour->m_mInput.nMeasure * 8, FALSE, TRUE );
	DrawText( pDC, &pt, str, 128 );
	str = Settings.SmartVolume( pNeighbour->m_mOutput.nMeasure * 8, FALSE, TRUE );
	DrawText( pDC, &pt, str, 128 + 80 );
	pt.y += TIP_TEXTHEIGHT;

	DrawText( pDC, &pt, _T("Total:") );
	str = Settings.SmartVolume( pNeighbour->m_mInput.nTotal, FALSE );
	DrawText( pDC, &pt, str, 128 );
	str = Settings.SmartVolume( pNeighbour->m_mOutput.nTotal, FALSE );
	DrawText( pDC, &pt, str, 128 + 80 );
	pt.y += TIP_TEXTHEIGHT;

	float nCompIn, nCompOut;
	pNeighbour->GetCompression( &nCompIn, &nCompOut );

	DrawText( pDC, &pt, _T("Compression:") );
	str = nCompIn >= 0 ? _T("Deflate") : _T("None");
	DrawText( pDC, &pt, str, 128 );
	str = nCompOut >= 0 ? _T("Deflate") : _T("None");
	DrawText( pDC, &pt, str, 128 + 80 );
	pt.y += TIP_TEXTHEIGHT;

	DrawText( pDC, &pt, _T("Ratio:"), 8 );
	if ( nCompIn >= 0 ) str.Format( _T("%.2f%%"), nCompIn * 100.0f ); else str.Empty();
	DrawText( pDC, &pt, str, 128 );
	if ( nCompOut >= 0 ) str.Format( _T("%.2f%%"), nCompOut * 100.0f ); else str.Empty();
	DrawText( pDC, &pt, str, 128 + 80 );
	pt.y += TIP_TEXTHEIGHT;

	pt.y += TIP_TEXTHEIGHT;

	CRect rc( pt.x, pt.y, m_sz.cx, pt.y + 40 );
	pDC->Draw3dRect( &rc, CoolInterface.m_crTipBorder, CoolInterface.m_crTipBorder );
	rc.DeflateRect( 1, 1 );
	m_pGraph->BufferedPaint( pDC, &rc );
	rc.InflateRect( 1, 1 );
	pDC->ExcludeClipRect( &rc );
	pt.y += 40;
}

/////////////////////////////////////////////////////////////////////////////
// CNeighbourTipCtrl message handlers

void CNeighbourTipCtrl::OnTimer(UINT nIDEvent) 
{
	CCoolTipCtrl::OnTimer( nIDEvent );
	
	if ( m_pGraph == NULL ) return;
	
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	CNeighbour* pNeighbour = Neighbours.Get( (DWORD)m_pContext );
	if ( pNeighbour == NULL ) return;
	
	pNeighbour->Measure();

	DWORD nIn	= pNeighbour->m_mInput.nMeasure * 8;
	DWORD nOut	= pNeighbour->m_mOutput.nMeasure * 8;

	m_pItemIn->Add( nIn );
	m_pItemOut->Add( nOut );

	m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nIn );
	m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nOut );
	m_pGraph->m_nUpdates++;

	Invalidate();
}
