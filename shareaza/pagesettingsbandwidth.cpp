//
// PageSettingsBandwidth.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "PageSettingsBandwidth.h"
#include "PageSettingsGnutella.h"
#include "PageSettingsDownloads.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CBandwidthSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CBandwidthSettingsPage, CSettingsPage)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBandwidthSettingsPage property page

CBandwidthSettingsPage::CBandwidthSettingsPage() : CSettingsPage(CBandwidthSettingsPage::IDD)
{
	m_sUPInLimit = _T("");
	m_sUPInMax = _T("");
	m_sUPInTotal = _T("");
	m_sUPOutLimit = _T("");
	m_sUPOutMax = _T("");
	m_sUPOutTotal = _T("");
	m_sDLTotal = _T("");
	m_sInTotal = _T("");
	m_sOutTotal = _T("");
	m_sPInTotal = _T("");
	m_sPOutLimit = _T("");
	m_sPOutMax = _T("");
	m_sPOutTotal = _T("");
	m_sULTotal = _T("");
	m_sPInLimit = _T("");
	m_sPInMax = _T("");
	m_sLInLimit = _T("");
	m_sLInMax = _T("");
	m_sLInTotal = _T("");
	m_sLOutLimit = _T("");
	m_sLOutMax = _T("");
	m_sLOutTotal = _T("");
	m_sUDPTotal = _T("");
	m_bActive = FALSE;
}

CBandwidthSettingsPage::~CBandwidthSettingsPage()
{
}

void CBandwidthSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange( pDX );
	DDX_Control(pDX, IDC_HEAD_TRANSMIT, m_wndHeadTransmit);
	DDX_Control(pDX, IDC_HEAD_RECEIVE, m_wndHeadReceive);
	DDX_Text(pDX, IDC_UPIN_LIMIT, m_sUPInLimit);
	DDX_Text(pDX, IDC_UPIN_MAX, m_sUPInMax);
	DDX_Text(pDX, IDC_UPIN_TOTAL, m_sUPInTotal);
	DDX_Text(pDX, IDC_UPOUT_LIMIT, m_sUPOutLimit);
	DDX_Text(pDX, IDC_UPOUT_MAX, m_sUPOutMax);
	DDX_Text(pDX, IDC_UPOUT_TOTAL, m_sUPOutTotal);
	DDX_Text(pDX, IDC_DL_TOTAL, m_sDLTotal);
	DDX_Text(pDX, IDC_IN_TOTAL, m_sInTotal);
	DDX_Text(pDX, IDC_OUT_TOTAL, m_sOutTotal);
	DDX_Text(pDX, IDC_PIN_TOTAL, m_sPInTotal);
	DDX_Text(pDX, IDC_POUT_LIMIT, m_sPOutLimit);
	DDX_Text(pDX, IDC_POUT_MAX, m_sPOutMax);
	DDX_Text(pDX, IDC_POUT_TOTAL, m_sPOutTotal);
	DDX_Text(pDX, IDC_UL_TOTAL, m_sULTotal);
	DDX_Text(pDX, IDC_PIN_LIMIT, m_sPInLimit);
	DDX_Text(pDX, IDC_PIN_MAX, m_sPInMax);
	DDX_Text(pDX, IDC_LIN_LIMIT, m_sLInLimit);
	DDX_Text(pDX, IDC_LIN_MAX, m_sLInMax);
	DDX_Text(pDX, IDC_LIN_TOTAL, m_sLInTotal);
	DDX_Text(pDX, IDC_LOUT_LIMIT, m_sLOutLimit);
	DDX_Text(pDX, IDC_LOUT_MAX, m_sLOutMax);
	DDX_Text(pDX, IDC_LOUT_TOTAL, m_sLOutTotal);
	DDX_Text(pDX, IDC_UDP_TOTAL, m_sUDPTotal);
}

/////////////////////////////////////////////////////////////////////////////
// CBandwidthSettingsPage message handlers

BOOL CBandwidthSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	/*
	for ( CWnd* pWnd = GetWindow( GW_CHILD ) ; pWnd ; pWnd = pWnd->GetNextWindow() )
	{
		TCHAR szClass[64];
		GetClassName( *pWnd, szClass, 64 );

		if ( _tcsistr( szClass, _T("UPDOWN") ) )
		{
			theApp.Message( MSG_DEBUG, _T("Class : %s"), szClass );
			CSpinButtonCtrl* pSpin = reinterpret_cast<CSpinButtonCtrl*>(pWnd);
			pSpin->SetRange( 0, 0x7FFF );
		}
	}
	*/

	m_bBytes		= Settings.General.RatesInBytes;
	m_sUPInLimit	= ToString( Settings.Bandwidth.HubIn, TRUE, TRUE );
	m_sUPOutLimit	= ToString( Settings.Bandwidth.HubOut, TRUE, TRUE );
	m_sLInLimit		= ToString( Settings.Bandwidth.LeafIn, TRUE, TRUE );
	m_sLOutLimit	= ToString( Settings.Bandwidth.LeafOut, TRUE, TRUE );
	m_sPInLimit		= ToString( Settings.Bandwidth.PeerIn, TRUE, TRUE );
	m_sPOutLimit	= ToString( Settings.Bandwidth.PeerOut, TRUE, TRUE );
	m_sDLTotal		= ToString( Settings.Bandwidth.Downloads, TRUE, TRUE );
	m_sULTotal		= ToString( Settings.Bandwidth.Uploads, TRUE, TRUE );
	m_sUDPTotal		= ToString( Settings.Bandwidth.UdpOut, TRUE, TRUE );

	Calculate( TRUE );

	return TRUE;
}

BOOL CBandwidthSettingsPage::OnSetActive()
{
	Calculate( TRUE );
	return CSettingsPage::OnSetActive();
}

void CBandwidthSettingsPage::Calculate(BOOL bForeward)
{
	int nNumHubs	= Settings.Gnutella1.NumHubs  + Settings.Gnutella2.NumHubs;
	int nNumLeafs	= Settings.Gnutella1.NumLeafs + Settings.Gnutella2.NumLeafs;
	int nNumPeers	= Settings.Gnutella1.NumPeers + Settings.Gnutella2.NumPeers;

	CGnutellaSettingsPage* ppGnutella =
		(CGnutellaSettingsPage*)GetPage( RUNTIME_CLASS(CGnutellaSettingsPage) );

	if ( ppGnutella->GetSafeHwnd() )
	{
		ppGnutella->UpdateData();
		nNumHubs	= min( ppGnutella->m_nG2Hubs, 3 ) + min( ppGnutella->m_nG1Hubs, 10 );
		nNumLeafs	= ppGnutella->m_nG1Leafs + ppGnutella->m_nG2Leafs;
		nNumPeers	= ppGnutella->m_nG1Peers + ppGnutella->m_nG2Peers;
	}

	Calculate( m_sUPInLimit, nNumHubs, m_sUPInMax, m_sUPInTotal, bForeward );
	Calculate( m_sLInLimit, nNumLeafs, m_sLInMax, m_sLInTotal, bForeward );
	Calculate( m_sPInLimit, nNumPeers, m_sPInMax, m_sPInTotal, bForeward );
	// Calculate( m_sDLLimit, nDownloads, m_sDLMax, m_sDLTotal, bForeward );

	CString str = m_sUPInTotal + ';' + m_sLInTotal + ';' + m_sPInTotal + ';' + m_sDLTotal;
	m_sInTotal = ToString( AddString( str ), TRUE, TRUE );

	Calculate( m_sUPOutLimit, nNumHubs, m_sUPOutMax, m_sUPOutTotal, bForeward );
	Calculate( m_sLOutLimit, nNumLeafs, m_sLOutMax, m_sLOutTotal, bForeward );
	Calculate( m_sPOutLimit, nNumPeers, m_sPOutMax, m_sPOutTotal, bForeward );
	// Calculate( m_sULLimit, nUploads, m_sULMax, m_sULTotal, bForeward );

	// CString strDummy;
	// Calculate( m_sUDPLimit, 1, strDummy, m_sUDPTotal, bForeward );

	str = m_sUPOutTotal + ';' + m_sLOutTotal + ';' + m_sPOutTotal + ';' + m_sULTotal + ';' + m_sUDPTotal;
	m_sOutTotal = ToString( AddString( str ), TRUE, TRUE );

	m_bActive = FALSE;
	UpdateData( FALSE );
	m_bActive = TRUE;
}

void CBandwidthSettingsPage::Calculate(CString& strLimit, int nCount, CString& strCount, CString& strTotal, BOOL bForeward)
{
	strCount.Format( _T("%lu"), nCount );

	if ( bForeward )
	{
		DWORD nSpeed = ToSpeed( strLimit ) * nCount;
		strTotal = ToString( nSpeed, TRUE, TRUE );
	}
	else
	{
		DWORD nSpeed = nCount ? ( ToSpeed( strTotal ) / nCount ) : 0;
		strLimit = ToString( nSpeed, TRUE, TRUE );
	}
}

DWORD CBandwidthSettingsPage::ToSpeed(CString& str)
{
	float nSpeed = 0;
	_stscanf( str, _T("%f"), &nSpeed );
	nSpeed *= 1024.0f;
	if ( ! m_bBytes ) nSpeed /= 8.0f;
	return (DWORD)nSpeed;
}

CString CBandwidthSettingsPage::ToString(DWORD nSpeed, BOOL bUnlimited, BOOL bUnit)
{
	CString str;

	if ( nSpeed || ! bUnlimited )
	{
		float nValue = (float)nSpeed / 1024.0f;
		if ( ! m_bBytes ) nValue *= 8.0f;
		str.Format( _T("%.1f"), double( nValue ) );

		if ( bUnit ) str += ( m_bBytes ) ? _T(" KB/s") : _T(" Kb/s");
	}
	else
	{
		str = _T("U/L");
	}

	return str;
}

DWORD CBandwidthSettingsPage::AddString(CString& str)
{
	DWORD nTotal = 0;

	for ( str += ',' ; str.GetLength() ; )
	{
		CString num = str.SpanExcluding( _T(";") );
		str = str.Mid( num.GetLength() + 1 );

		if ( num.GetLength() )
		{
			DWORD nSpeed = ToSpeed( num );
			if ( ! nSpeed ) return 0;
			nTotal += nSpeed;
		}
	}

	return nTotal;
}

void CBandwidthSettingsPage::SwapBytesBits(CString& str)
{
	m_bBytes = ! m_bBytes;
	DWORD nSpeed = ToSpeed( str );
	m_bBytes = ! m_bBytes;
	str = ToString( nSpeed, TRUE );
}

BOOL CBandwidthSettingsPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if ( HIWORD( wParam ) == EN_CHANGE && m_bActive )
	{
		switch ( LOWORD( wParam ) )
		{
		case IDC_UPIN_LIMIT:
		case IDC_LIN_LIMIT:
		case IDC_PIN_LIMIT:
//		case IDC_DL_LIMIT:
		case IDC_UPOUT_LIMIT:
		case IDC_LOUT_LIMIT:
		case IDC_POUT_LIMIT:
			PostMessage( WM_TIMER, 1 );
			break;
		case IDC_UPIN_TOTAL:
		case IDC_LIN_TOTAL:
		case IDC_PIN_TOTAL:
		case IDC_DL_TOTAL:
		case IDC_UPOUT_TOTAL:
		case IDC_LOUT_TOTAL:
		case IDC_POUT_TOTAL:
		case IDC_UL_TOTAL:
		case IDC_UDP_TOTAL:
			PostMessage( WM_TIMER, 2 );
			break;
		}
	}

	return CSettingsPage::OnCommand( wParam, lParam );
}

BOOL CBandwidthSettingsPage::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NM_UPDOWN* pNotify = reinterpret_cast<NM_UPDOWN*>( lParam );

	if ( pNotify->hdr.code == UDN_DELTAPOS )
	{
		CWnd* pWnd		= CWnd::FromHandle( pNotify->hdr.hwndFrom );
		CEdit* pEdit	= static_cast<CEdit*>( pWnd->GetNextWindow( GW_HWNDPREV ) );
		CString str;

		pEdit->GetWindowText( str );
		DWORD nSpeed = ToSpeed( str );

		if ( m_bBytes )
			nSpeed -= pNotify->iDelta * 103;
		else
			nSpeed -= pNotify->iDelta * 128;

		if ( nSpeed > 0xF0000000 ) nSpeed = 0;

		str = ToString( nSpeed, TRUE, TRUE );
		pEdit->SetWindowText( str );

		return TRUE;
	}

	return CSettingsPage::OnNotify( wParam, lParam, pResult );
}

void CBandwidthSettingsPage::OnTimer(UINT_PTR nIDEvent)
{
	UpdateData( TRUE );
	Calculate( nIDEvent == 1 );
}

HBRUSH CBandwidthSettingsPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSettingsPage::OnCtlColor(pDC, pWnd, nCtlColor);

	if ( pWnd == &m_wndHeadReceive || pWnd == &m_wndHeadTransmit )
	{
		pDC->SelectObject( &theApp.m_gdiFontBold );
	}

	return hbr;
}

void CBandwidthSettingsPage::OnOK()
{
	UpdateData();

	Settings.Bandwidth.HubIn		= ToSpeed( m_sUPInLimit );
	Settings.Bandwidth.HubOut		= ToSpeed( m_sUPOutLimit );
	Settings.Bandwidth.LeafIn		= ToSpeed( m_sLInLimit );
	Settings.Bandwidth.LeafOut		= ToSpeed( m_sLOutLimit );
	Settings.Bandwidth.PeerIn		= ToSpeed( m_sPInLimit );
	Settings.Bandwidth.PeerOut		= ToSpeed( m_sPOutLimit );
	Settings.Bandwidth.Downloads	= ToSpeed( m_sDLTotal );
	Settings.Bandwidth.Uploads		= ToSpeed( m_sULTotal );
	Settings.Bandwidth.UdpOut		= ToSpeed( m_sUDPTotal );

	CSettingsPage::OnOK();
}

