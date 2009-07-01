//
// CtrlNeighbourTip.cpp : implementation file
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "Flags.h"

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

	CNeighbour* pNeighbour = Neighbours.Get( reinterpret_cast< DWORD_PTR >( m_pContext ) );
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
	m_pItemIn	= new CGraphItem( 0, 1.0f, RGB( 0, 0, 0xFF ) );
	m_pItemOut	= new CGraphItem( 0, 1.0f, RGB( 0xFF, 0, 0 ) );

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
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	CNeighbour* pNeighbour = Neighbours.Get( reinterpret_cast< DWORD_PTR>( m_pContext ) );
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

	if ( pNeighbour->m_sCountryName.GetLength() )
	{
		AddSize( pDC, pNeighbour->m_sCountryName );
		m_sz.cy += TIP_TEXTHEIGHT + 4;
	}

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

	CNeighbour* pNeighbour = Neighbours.Get( reinterpret_cast< DWORD_PTR>( m_pContext ) );
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

	if ( pNeighbour->m_sCountryName.GetLength() )
	{
		int nFlagIndex = Flags.GetFlagIndex( pNeighbour->m_sCountry );
		if ( nFlagIndex >= 0 )
		{
			ImageList_DrawEx( Flags.m_pImage, nFlagIndex, pDC->GetSafeHdc(),
				pt.x, pt.y, 18, 18, CoolInterface.m_crTipBack, CLR_NONE, ILD_NORMAL );
			pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 18, pt.y + 18 );

			pt.x += 25;
			pt.y += 2;
			DrawText( pDC, &pt, pNeighbour->m_sCountryName );
			pt.x -= 25;
		}
		else
		{
			DrawText( pDC, &pt, pNeighbour->m_sCountryName );
		}
		pt.y += TIP_TEXTHEIGHT + 2;
	}

	if ( pNeighbour->m_nState < nrsConnected )
	{
		LoadString( str, IDS_NEIGHBOUR_HANDSHAKE );
	}
	else
	{
		switch ( pNeighbour->m_nProtocol )
		{
		case PROTOCOL_G1:
			switch ( pNeighbour->m_nNodeType )
			{
			case ntNode:
				LoadString( str, IDS_NEIGHBOUR_G1PP );
				break;
			case ntHub:
				LoadString( str, IDS_NEIGHBOUR_G1LU );
				break;
			case ntLeaf:
				LoadString( str, IDS_NEIGHBOUR_G1UL );
				break;
			}
			break;
		case PROTOCOL_G2:
			switch ( pNeighbour->m_nNodeType )
			{
			case ntNode:
				LoadString( str, IDS_NEIGHBOUR_G2HH );
				break;
			case ntHub:
				LoadString( str, IDS_NEIGHBOUR_G2LH );
				break;
			case ntLeaf:
				LoadString( str, IDS_NEIGHBOUR_G2HL );
				break;
			}
			break;
		case PROTOCOL_ED2K:
			if ( CEDNeighbour* pED = (CEDNeighbour*)pNeighbour )
			{
				if ( CEDPacket::IsLowID( pED->m_nClientID ) )
				{
					CString sFormat;
					LoadString( sFormat, IDS_NEIGHBOUR_ED2K_LOW );
					str.Format( sFormat, pED->m_nClientID );
				}
				else
				{
					LoadString( str, IDS_NEIGHBOUR_ED2K_HIGH );
				}
			}
			break;
		default:
			LoadString( str, IDS_NEIGHBOUR_HANDSHAKE );
			break;
		}
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
	LoadString( str, IDS_NEIGHBOUR_INBOUND );
	DrawText( pDC, &pt, str, 128 );
	pDC->SetTextColor( m_pItemOut->m_nColour );
	LoadString( str, IDS_NEIGHBOUR_OUTBOUND );
	DrawText( pDC, &pt, str, 128 + 80 );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	pDC->SetTextColor( 0 );

	pt.y += TIP_TEXTHEIGHT;

	LoadString( str, IDS_NEIGHBOUR_CURRENT );
	DrawText( pDC, &pt, str );
	str = Settings.SmartSpeed( pNeighbour->m_mInput.nMeasure );
	DrawText( pDC, &pt, str, 128 );
	str = Settings.SmartSpeed( pNeighbour->m_mOutput.nMeasure );
	DrawText( pDC, &pt, str, 128 + 80 );
	pt.y += TIP_TEXTHEIGHT;

	LoadString( str, IDS_NEIGHBOUR_TOTAL );
	DrawText( pDC, &pt, str );
	str = Settings.SmartVolume( pNeighbour->m_mInput.nTotal );
	DrawText( pDC, &pt, str, 128 );
	str = Settings.SmartVolume( pNeighbour->m_mOutput.nTotal );
	DrawText( pDC, &pt, str, 128 + 80 );
	pt.y += TIP_TEXTHEIGHT;

	float nCompIn, nCompOut;
	pNeighbour->GetCompression( &nCompIn, &nCompOut );

	LoadString( str, IDS_NEIGHBOUR_COMPRESSION );
	DrawText( pDC, &pt, str );
	LoadString( str, nCompIn > 0 ? IDS_NEIGHBOUR_COMPRESSION_DF : IDS_NEIGHBOUR_COMPRESSION_NONE );
	DrawText( pDC, &pt, str, 128 );
	LoadString( str, nCompOut > 0 ? IDS_NEIGHBOUR_COMPRESSION_DF : IDS_NEIGHBOUR_COMPRESSION_NONE );
	DrawText( pDC, &pt, str, 128 + 80 );
	pt.y += TIP_TEXTHEIGHT;

	if ( nCompIn > 0 || nCompOut > 0 )
	{
		LoadString( str, IDS_NEIGHBOUR_RATIO );
		DrawText( pDC, &pt, str );
		if ( nCompIn > 0 ) str.Format( _T("%.2f%%"), nCompIn * 100.0 ); else str.Empty();
		DrawText( pDC, &pt, str, 128 );
		if ( nCompOut > 0 ) str.Format( _T("%.2f%%"), nCompOut * 100.0 ); else str.Empty();
		DrawText( pDC, &pt, str, 128 + 80 );
		pt.y += TIP_TEXTHEIGHT;
	}

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

void CNeighbourTipCtrl::OnTimer(UINT_PTR nIDEvent)
{
	CCoolTipCtrl::OnTimer( nIDEvent );

	if ( m_pGraph == NULL ) return;

	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	CNeighbour* pNeighbour = Neighbours.Get( reinterpret_cast< DWORD_PTR>( m_pContext ) );
	if ( pNeighbour == NULL ) return;

	pNeighbour->Measure();

	DWORD nIn	= pNeighbour->m_mInput.nMeasure;
	DWORD nOut	= pNeighbour->m_mOutput.nMeasure;

	m_pItemIn->Add( nIn );
	m_pItemOut->Add( nOut );

	m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nIn );
	m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nOut );
	m_pGraph->m_nUpdates++;

	Invalidate();
}
