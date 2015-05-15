//
// CtrlDownloadTip.cpp
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
#include "ShellIcons.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadTransferBT.h"
#include "DownloadTransferED2K.h"
#include "EDClient.h"
#include "Flags.h"
#include "FragmentedFile.h"
#include "FragmentBar.h"
#include "Skin.h"
#include "GraphLine.h"
#include "GraphItem.h"
#include "CtrlDownloadTip.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CDownloadTipCtrl, CCoolTipCtrl)

BEGIN_MESSAGE_MAP(CDownloadTipCtrl, CCoolTipCtrl)
	ON_WM_TIMER()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl construction

CDownloadTipCtrl::CDownloadTipCtrl()
	: m_pDownload	( NULL )
	, m_pSource		( NULL )
	, m_pGraph		( NULL )
	, m_nIcon		( 0 )
	, m_nHeaderWidth( 0 )
	, m_nValueWidth	( 0 )
	, m_nHeaders	( 0 )
	, m_nStatWidth	( 0 )
	, m_bDrawGraph	( FALSE )
{
}

CDownloadTipCtrl::~CDownloadTipCtrl()
{
	delete m_pGraph;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl events

BOOL CDownloadTipCtrl::OnPrepare()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return FALSE;

	CalcSizeHelper();

	return ( m_sz.cx > 0 );
}

void CDownloadTipCtrl::OnCalcSize(CDC* pDC)
{
	if ( m_pDownload && Downloads.Check( m_pDownload ) )
	{
		OnCalcSize( pDC, m_pDownload );
	}
	else if ( m_pSource && Downloads.Check( m_pSource ) )
	{
		OnCalcSize( pDC, m_pSource );
	}
	m_sz.cx = min( max( m_sz.cx, 400l ), (LONG)GetSystemMetrics( SM_CXSCREEN ) / 2 );
}

void CDownloadTipCtrl::OnShow()
{
	delete m_pGraph;

	m_pGraph	= CreateLineGraph();
	m_pItem		= new CGraphItem( 0, 1.0f, RGB( 0, 0, 0xFF ) );

	m_pGraph->AddItem( m_pItem );
}

void CDownloadTipCtrl::OnHide()
{
	delete m_pGraph;
	m_pGraph = NULL;
	m_pItem = NULL;
}

void CDownloadTipCtrl::OnPaint(CDC* pDC)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	if ( m_pDownload && Downloads.Check( m_pDownload ) )
	{
		OnPaint( pDC, m_pDownload );
	}
	else if ( m_pSource && Downloads.Check( m_pSource ) )
	{
		OnPaint( pDC, m_pSource );
	}
	else
	{
		Hide();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl download case

void CDownloadTipCtrl::OnCalcSize(CDC* pDC, CDownload* pDownload)
{
	PrepareDownloadInfo( pDownload );

	AddSize( pDC, m_sName );
	m_sz.cy += TIP_TEXTHEIGHT;
	pDC->SelectObject( &CoolInterface.m_fntNormal );

	if ( m_sSHA1.GetLength() )
	{
		AddSize( pDC, m_sSHA1 );
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	if ( m_sED2K.GetLength() )
	{
		AddSize( pDC, m_sED2K );
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	if ( m_sBTH.GetLength() )
	{
		AddSize( pDC, m_sBTH );
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	if ( m_sMD5.GetLength() )
	{
		AddSize( pDC, m_sMD5 );
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	if ( m_sTiger.GetLength() )
	{
		AddSize( pDC, m_sTiger );
		m_sz.cy += TIP_TEXTHEIGHT;
	}

	m_sz.cy += TIP_RULE;
	AddSize( pDC, m_sSize, 80 );
	AddSize( pDC, m_sType, 80 );
	m_sz.cy += 36;
	m_sz.cy += TIP_RULE;

	// Torrent Tracker error
	if ( pDownload->m_bTorrentTrackerError && pDownload->m_sTorrentTrackerError.GetLength() )
	{
		AddSize( pDC, pDownload->m_sTorrentTrackerError );
		m_sz.cy += TIP_TEXTHEIGHT;
		m_sz.cy += TIP_RULE;
	}

	// File error		
	if ( pDownload->GetFileError() != ERROR_SUCCESS )
	{
		if ( pDownload->GetFileErrorString().GetLength() )
		{
			AddSize( pDC, pDownload->GetFileErrorString() );
			m_sz.cy += TIP_TEXTHEIGHT;
		}
		AddSize( pDC, GetErrorString( pDownload->GetFileError() ) );
		m_sz.cy += TIP_TEXTHEIGHT;
		m_sz.cy += TIP_RULE;
	}

	if ( pDownload->IsTorrent() )
	{	//Torrent ratio
		m_sz.cy += TIP_TEXTHEIGHT;
	}

	if ( ! pDownload->IsSeeding() )
	{	// Seeding torrent display none of this
		if ( pDownload->IsCompleted() )
		{	// ETA and downloaded
			m_sz.cy += TIP_TEXTHEIGHT * 2;
		}
		else
		{	// Speed, ETA, Downloaded, No. Sources
			m_sz.cy += TIP_TEXTHEIGHT * 4;
		}
	}

	// Number of reviews
	if ( pDownload->GetReviewCount() > 0 )
		m_sz.cy += TIP_TEXTHEIGHT;

	// URL
	if ( m_sURL.GetLength() )
	{
		m_sz.cy += TIP_RULE;
		AddSize( pDC, m_sURL );
		m_sz.cy += TIP_TEXTHEIGHT;
	}

	// Progress bar (not applicable for seeding torrents)
	if ( ! pDownload->IsSeeding() )
	{
		m_sz.cy += 2;
		m_sz.cy += TIP_TEXTHEIGHT;
	}

	// Graph (Only for files in progress)
	if ( pDownload->IsCompleted() )
		m_bDrawGraph = FALSE;
	else
	{
		m_sz.cy += TIP_GAP;
		m_sz.cy += 40;
		m_bDrawGraph = TRUE;
	}

	CString str;
	LoadString( str, IDS_DLM_ESTIMATED_TIME );
	m_nStatWidth = pDC->GetTextExtent( str ).cx + 8;
}

void CDownloadTipCtrl::OnPaint(CDC* pDC, CDownload* pDownload)
{
	CPoint pt( 0, 0 );
	CSize sz( m_sz.cx, TIP_TEXTHEIGHT );

	CString str, strOf, strAnother;
	LoadString( strOf, IDS_GENERAL_OF );

	DrawText( pDC, &pt, m_sName, &sz );
	pt.y += TIP_TEXTHEIGHT;
	pDC->SelectObject( &CoolInterface.m_fntNormal );

	if ( !m_sSHA1.IsEmpty() )
	{
		DrawText( pDC, &pt, m_sSHA1 );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( !m_sTiger.IsEmpty() )
	{
		DrawText( pDC, &pt, m_sTiger );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( !m_sED2K.IsEmpty() )
	{
		DrawText( pDC, &pt, m_sED2K );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( !m_sBTH.IsEmpty() )
	{
		DrawText( pDC, &pt, m_sBTH );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( !m_sMD5.IsEmpty() )
	{
		DrawText( pDC, &pt, m_sMD5 );
		pt.y += TIP_TEXTHEIGHT;
	}

	DrawRule( pDC, &pt );

	ShellIcons.Draw( pDC, m_nIcon, 32, pt.x, pt.y, CoolInterface.m_crTipBack );

	pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 32, pt.y + 32 );

	pt.y += 2;
	LoadString( str, IDS_TIP_SIZE );
	str.Append( _T(": ") );
	pt.x += 40;
	DrawText( pDC, &pt, str );
	CSize sz1 = pDC->GetTextExtent( str );
	LoadString( strAnother, IDS_TIP_TYPE );
	strAnother.Append( _T(": ") );
	CSize sz2 = pDC->GetTextExtent( strAnother );

	sz1.cx = max( sz1.cx, sz2.cx );

	pt.x += sz1.cx + 2;
	DrawText( pDC, &pt, m_sSize );
	pt.y += TIP_TEXTHEIGHT;
	pt.x -= sz1.cx + 2;
	DrawText( pDC, &pt, strAnother );
	pt.x += sz1.cx + 2;
	DrawText( pDC, &pt, m_sType );
	pt.x -= 40 + sz1.cx + 2;
	pt.y -= TIP_TEXTHEIGHT + 2;
	pt.y += 34;

	DrawRule( pDC, &pt );

	CString strFormat, strETA, strSpeed, strVolume, strSources, strReviews, strTorrentUpload;

	int nSourceCount	= pDownload->GetSourceCount();
	int nTransferCount	= pDownload->GetTransferCount();
	int nReviewCount	= pDownload->GetReviewCount();

	LoadString( strFormat, IDS_TIP_NA );

	if ( pDownload->IsMoving() )
	{
		LoadString( strETA, IDS_DLM_COMPLETED_WORD );
		strSpeed = strFormat;
		LoadString( strSources, IDS_DLM_COMPLETED_WORD );
	}
	else if ( pDownload->IsPaused() )
	{
		strETA = strFormat;
		strSpeed = strFormat;
		strSources.Format( _T("%i"), nSourceCount );
	}
	else if ( nTransferCount )
	{
		DWORD nTime = pDownload->GetTimeRemaining();

		if ( nTime != 0xFFFFFFFF )
		{
			if ( nTime > 86400 )
			{
				LoadString( strFormat, IDS_DLM_TIME_DAH );
				strETA.Format( strFormat, nTime / 86400, ( nTime / 3600 ) % 24 );
			}
			else if ( nTime > 3600 )
			{
				LoadString( strFormat, IDS_DLM_TIME_HAM );
				strETA.Format( strFormat, nTime / 3600, ( nTime % 3600 ) / 60 );
			}
			else if ( nTime > 60 )
			{
				LoadString( strFormat, IDS_DLM_TIME_MAS );
				strETA.Format( strFormat, nTime / 60, nTime % 60 );
			}
			else
			{
				LoadString( strFormat, IDS_DLM_TIME_S );
				strETA.Format( strFormat, nTime % 60 );
			}
		}

		strSpeed = Settings.SmartSpeed( pDownload->GetAverageSpeed() );

		strSources.Format( _T("%i %s %i"), nTransferCount, (LPCTSTR)strOf, nSourceCount );
		if ( Settings.General.LanguageRTL ) strSources = _T("\x202B") + strSources;
	}
	else if ( nSourceCount )
	{
		strETA		= strFormat;
		strSpeed	= strFormat;
		strSources.Format( _T("%i"), nSourceCount );
	}
	else
	{
		strETA		= strFormat;
		strSpeed	= strFormat;
		LoadString( strSources, IDS_DLM_NO_SOURCES );
	}

	if ( nReviewCount > 0 )
	{
		strReviews.Format( _T("%i"), nReviewCount );
	}

	if ( pDownload->IsStarted() && pDownload->m_nSize < SIZE_UNKNOWN )
	{
		if ( Settings.General.LanguageRTL )
		{
			strVolume.Format( _T("(%.2f%%) %s %s %s"),
				pDownload->GetProgress(),
				(LPCTSTR)Settings.SmartVolume( pDownload->m_nSize ),
				(LPCTSTR)strOf,
				(LPCTSTR)Settings.SmartVolume( pDownload->GetVolumeComplete() ) );
		}
		else
		{
			strVolume.Format( _T("%s %s %s (%.2f%%)"),
				(LPCTSTR)Settings.SmartVolume( pDownload->GetVolumeComplete() ),
				(LPCTSTR)strOf,
				(LPCTSTR)Settings.SmartVolume( pDownload->m_nSize ),
				pDownload->GetProgress() );
		}
	}
	else
	{
		LoadString( strVolume, IDS_TIP_NA );
	}

	if ( pDownload->IsTorrent() )
	{
		if ( Settings.General.LanguageRTL )
		{
			strTorrentUpload.Format( _T("(%.2f%%) %s %s %s"),
				pDownload->GetRatio(),
				(LPCTSTR)Settings.SmartVolume( pDownload->m_pTorrent.m_nTotalDownload ),
				(LPCTSTR)strOf,
				(LPCTSTR)Settings.SmartVolume( pDownload->m_pTorrent.m_nTotalUpload ) );
		}
		else
		{
			strTorrentUpload.Format( _T("%s %s %s (%.2f%%)"),
				(LPCTSTR)Settings.SmartVolume( pDownload->m_pTorrent.m_nTotalUpload ),
				(LPCTSTR)strOf,
				(LPCTSTR)Settings.SmartVolume( pDownload->m_pTorrent.m_nTotalDownload ),
				pDownload->GetRatio() );
		}
	}

	// Draw the pop-up box
	if ( pDownload->m_bTorrentTrackerError && pDownload->m_sTorrentTrackerError.GetLength() )
	{	// Tracker error
		DrawText( pDC, &pt, pDownload->m_sTorrentTrackerError, 3 );
		pt.y += TIP_TEXTHEIGHT;
		DrawRule( pDC, &pt );
	}

	if ( pDownload->GetFileError() != ERROR_SUCCESS )
	{	// File error
		COLORREF crOld = pDC->SetTextColor( CoolInterface.m_crTextAlert );
		if ( pDownload->GetFileErrorString().GetLength() )
		{
			DrawText( pDC, &pt, pDownload->GetFileErrorString(), 3 );
			pt.y += TIP_TEXTHEIGHT;
		}
		DrawText( pDC, &pt, GetErrorString( pDownload->GetFileError() ), 3 );
		pDC->SetTextColor( crOld );
		pt.y += TIP_TEXTHEIGHT;
		DrawRule( pDC, &pt );
	}

	if ( ! pDownload->IsCompleted() )
	{	// Speed. Not for completed files
		LoadString( strFormat, IDS_DLM_TOTAL_SPEED );
		DrawText( pDC, &pt, strFormat, 3 );
		DrawText( pDC, &pt, strSpeed, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( ! pDownload->IsSeeding() )
	{	// ETA. Not applicable for seeding torrents.
		LoadString( strFormat, IDS_DLM_ESTIMATED_TIME );
		DrawText( pDC, &pt, strFormat, 3 );
		DrawText( pDC, &pt, strETA, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
		// Volume downloaded. Not for seeding torrents
		LoadString( strFormat, IDS_DLM_VOLUME_DOWNLOADED );
		DrawText( pDC, &pt, strFormat, 3 );
		DrawText( pDC, &pt, strVolume, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( pDownload->IsTorrent() )
	{	// Upload ratio- only for torrents
		LoadString( strFormat, IDS_DLM_VOLUME_UPLOADED );
		DrawText( pDC, &pt, strFormat, 3 );
		DrawText( pDC, &pt, strTorrentUpload, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( ! pDownload->IsCompleted() )
	{	// No. Sources- Not applicable for completed files.
		LoadString( strFormat, IDS_DLM_NUMBER_OF_SOURCES );
		DrawText( pDC, &pt, strFormat, 3 );
		DrawText( pDC, &pt, strSources, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( nReviewCount > 0 )
	{	// No. Reviews
		LoadString( strFormat, IDS_DLM_NUMBER_OF_REVIEWS );
		DrawText( pDC, &pt, strFormat, 3 );
		DrawText( pDC, &pt, strReviews, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( m_sURL.GetLength() )
	{	// Draw URL if present
		DrawRule( pDC, &pt );
		DrawText( pDC, &pt, m_sURL );
		pt.y += TIP_TEXTHEIGHT;
	}

	if ( ! pDownload->IsSeeding() )
	{	// Not applicable for seeding torrents.
		pt.y += 2;
		DrawProgressBar( pDC, &pt, pDownload );
		pt.y += TIP_GAP;
	}

	if ( m_bDrawGraph && m_pGraph )
	{	// Don't draw empty graph.
		CRect rc( pt.x, pt.y, m_sz.cx, pt.y + 40 );
		pDC->Draw3dRect( &rc, CoolInterface.m_crTipBorder, CoolInterface.m_crTipBorder );
		rc.DeflateRect( 1, 1 );
		m_pGraph->BufferedPaint( pDC, &rc );
		rc.InflateRect( 1, 1 );
		pDC->ExcludeClipRect( &rc );
		pt.y += 40;
	}
	pt.y += TIP_GAP;
}

void CDownloadTipCtrl::PrepareDownloadInfo(CDownload* pDownload)
{
	PrepareFileInfo( pDownload );
	
	if ( Settings.General.GUIMode == GUI_BASIC )
		return;

	// We also report on if we have a hashset, and if hash is trusted (Debug mode only)
	CString strNoHashset, strUntrusted;
	LoadString( strNoHashset, IDS_TIP_NOHASHSET );
	LoadString( strUntrusted, IDS_TIP_UNTRUSTED );

	m_sSHA1 = pDownload->m_oSHA1.toShortUrn();
	if ( m_sSHA1.GetLength() )
	{
		if ( ! pDownload->m_bSHA1Trusted )
		{
			m_sSHA1 += _T(" (") + strUntrusted + _T(")");
		}
	}

	m_sTiger = pDownload->m_oTiger.toShortUrn();
	if ( m_sTiger.GetLength() )
	{
		if ( ! pDownload->m_pTigerBlock )
		{
			if ( pDownload->m_bTigerTrusted )
			{
				m_sTiger += _T(" (") + strNoHashset + _T(")");
			}
			else
			{
				m_sTiger += _T(" (") + strNoHashset + _T(", ") + strUntrusted + _T(")");
			}
		}
		else if ( ! pDownload->m_bTigerTrusted )
		{
			m_sTiger += _T(" (") + strUntrusted + _T(")");
		}
	}

	m_sED2K = pDownload->m_oED2K.toShortUrn();
	if ( m_sED2K.GetLength() )
	{
		if ( ! pDownload->m_pHashsetBlock )
		{
			if ( pDownload->m_bED2KTrusted )
			{
				m_sED2K += _T(" (") + strNoHashset + _T(")");
			}
			else
			{
				m_sED2K += _T(" (") + strNoHashset + _T(", ") + strUntrusted + _T(")");
			}
		}
		else if ( ! pDownload->m_bED2KTrusted )
		{
			m_sED2K += _T(" (") + strUntrusted + _T(")");
		}
	}

	m_sBTH = pDownload->m_oBTH.toShortUrn();
	if ( m_sBTH.GetLength() )
	{
		if ( ! pDownload->m_pTorrentBlock )
		{
			if ( pDownload->m_bBTHTrusted )
			{
				m_sBTH += _T(" (") + strNoHashset + _T(")");
			}
			else
			{
				m_sBTH += _T(" (") + strNoHashset + _T(", ") + strUntrusted + _T(")");
			}
		}
		else if ( ! pDownload->m_bBTHTrusted )
		{
			m_sBTH += _T(" (") + strUntrusted + _T(")");
		}
	}

	if ( pDownload->IsTorrent() )
		m_sURL = pDownload->m_pTorrent.GetTrackerAddress();

	m_sMD5 = pDownload->m_oMD5.toShortUrn();
	if ( m_sMD5.GetLength() )
	{
		if ( ! pDownload->m_bMD5Trusted )
		{
			m_sMD5+= _T(" (") + strUntrusted + _T(")");
		}
	}
}

void CDownloadTipCtrl::PrepareFileInfo(CShareazaFile* pDownload)
{
	m_sName = pDownload->m_sName;
	m_sSize = Settings.SmartVolume( pDownload->m_nSize );
	if ( pDownload->m_nSize == SIZE_UNKNOWN )
		m_sSize = _T("?");

	m_sSHA1.Empty();
	m_sTiger.Empty();
	m_sED2K.Empty();
	m_sBTH.Empty();
	m_sURL.Empty();
	m_sMD5.Empty();

	m_nIcon = ShellIcons.Get( m_sName, 32 );
	m_sType = ShellIcons.GetTypeString( m_sName );
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl source case

void CDownloadTipCtrl::OnCalcSize(CDC* pDC, CDownloadSource* pSource)
{
	const CTransfer* pTransfer = pSource->GetTransfer();

	// Is this a firewalled eDonkey client
	if ( pSource->m_nProtocol == PROTOCOL_ED2K && pSource->m_bPushOnly == TRUE )
	{
		m_sName.Format( _T("%lu@%s:%u"),
			pSource->m_pAddress.S_un.S_addr,
			(LPCTSTR)CString( inet_ntoa( pSource->m_pServerAddress ) ),
			pSource->m_nServerPort );
	}

	// Or an active transfer
	else if ( ! pSource->IsIdle() )
	{
		m_sName.Format( _T("%s:%u"),
			(LPCTSTR)pSource->GetAddress(),
			ntohs( pSource->GetPort() ) );
	}

	// Or just queued
	else
	{
		m_sName.Format( _T("%s:%u"),
			(LPCTSTR)CString( inet_ntoa( pSource->m_pAddress ) ),
			pSource->m_nPort );
	}

	// Add the Nickname if there is one and they are being shown
	if ( Settings.Search.ShowNames && !pSource->m_sNick.IsEmpty() )
		m_sName = pSource->m_sNick + _T(" (") + m_sName + _T(")");

	// Indicate if this is a firewalled client
	if ( pSource->m_bPushOnly )
		m_sName += _T(" (push)");

	m_sCountryName = pSource->m_sCountryName;

	m_sURL = pSource->m_sURL;

	AddSize( pDC, m_sName );
	m_sz.cy += TIP_TEXTHEIGHT;

	pDC->SelectObject( &CoolInterface.m_fntNormal );
	AddSize( pDC, m_sCountryName );
	m_sz.cy += 16;

	m_sz.cy += TIP_RULE;

	AddSize( pDC, m_sURL, 80 );
	m_sz.cy += TIP_TEXTHEIGHT * 4;

	m_sz.cy += TIP_GAP;
	m_sz.cy += TIP_TEXTHEIGHT;
	m_sz.cy += TIP_GAP;
	m_sz.cy += 40;
	m_sz.cy += TIP_GAP;

	m_nValueWidth = 0;
	m_nHeaderWidth = 0;
	m_nHeaders = 0;
	if ( ! pSource->IsIdle() && Settings.General.GUIMode != GUI_BASIC )
	{
		m_nHeaders = (int)pTransfer->m_pHeaderName.GetSize();
		for ( int nHeader = 0 ; nHeader < m_nHeaders ; nHeader++ )
		{
			CString strName		= pTransfer->m_pHeaderName.GetAt( nHeader ) + _T(':');
			CString strValue	= pTransfer->m_pHeaderValue.GetAt( nHeader );
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

void CDownloadTipCtrl::OnPaint(CDC* pDC, CDownloadSource* pSource)
{
	CPoint pt( 0, 0 );
	CSize sz( m_sz.cx, TIP_TEXTHEIGHT );

	DrawText( pDC, &pt, m_sName );
	pt.y += TIP_TEXTHEIGHT;

	int nFlagIndex = Flags.GetFlagIndex( pSource->m_sCountry );
	if ( nFlagIndex >= 0 )
	{
		Flags.Draw( nFlagIndex, pDC->GetSafeHdc(), pt.x, pt.y, CoolInterface.m_crTipBack );
		pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 16, pt.y + 16 );
	}
	pt.x += 16 + 4;
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	DrawText( pDC, &pt, m_sCountryName );
	pt.y += 16;
	pt.x -= 16 + 4;

	DrawRule( pDC, &pt );

	CString strStatus, strSpeed, strText;
	CString strOf;
	LoadString( strOf, IDS_GENERAL_OF );

	if ( ! pSource->IsIdle() )
	{
		strStatus = pSource->GetState( TRUE );

		if ( DWORD nLimit = pSource->GetLimit() )
		{
			strSpeed.Format( _T("%s %s %s"),
				(LPCTSTR)Settings.SmartSpeed( pSource->GetMeasuredSpeed() ),
				(LPCTSTR)strOf,
				(LPCTSTR)Settings.SmartSpeed( nLimit ) );
		}
		else
		{
			strSpeed = Settings.SmartSpeed( pSource->GetMeasuredSpeed() );
		}
	}
	else
	{
		LoadString( strStatus, IDS_TIP_INACTIVE );
		LoadString( strSpeed, IDS_TIP_NA );
	}

	LoadString( strText, IDS_TIP_STATUS );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, strStatus, 80 );
	pt.y += TIP_TEXTHEIGHT;

	LoadString( strText, IDS_TIP_SPEED );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, strSpeed, 80 );
	pt.y += TIP_TEXTHEIGHT;

	LoadString( strText, IDS_TIP_URL );
	DrawText( pDC, &pt, strText );
	pt.x += 80;
	sz.cx -= 80;
	DrawText( pDC, &pt, m_sURL, &sz );
	pt.x -= 80;
	sz.cx += 80;
	pt.y += TIP_TEXTHEIGHT;

	LoadString( strText, IDS_TIP_USERAGENT );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, pSource->m_sServer, 80 );
	pt.y += TIP_TEXTHEIGHT;

	pt.y += TIP_GAP;

	DrawProgressBar( pDC, &pt, pSource );
	pt.y += TIP_GAP;

	if ( m_pGraph )
	{
		CRect rc( pt.x, pt.y, m_sz.cx, pt.y + 40 );
		pDC->Draw3dRect( &rc, CoolInterface.m_crTipBorder, CoolInterface.m_crTipBorder );
		rc.DeflateRect( 1, 1 );
		m_pGraph->BufferedPaint( pDC, &rc );
		rc.InflateRect( 1, 1 );
		pDC->ExcludeClipRect( &rc );
		pt.y += 40;
	}
	pt.y += TIP_GAP;

	if ( ! pSource->IsIdle() && Settings.General.GUIMode != GUI_BASIC )
	{
		const CTransfer* pTransfer = pSource->GetTransfer();
		if ( m_nHeaders != pTransfer->m_pHeaderName.GetSize() )
		{
			ShowImpl( true );
			return;
		}
		for ( int nHeader = 0 ; nHeader < m_nHeaders ; nHeader++ )
		{
			CString strName = pTransfer->m_pHeaderName.GetAt( nHeader ) + _T(':');
			CString strValue = pTransfer->m_pHeaderValue.GetAt( nHeader );
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

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl progress case

void CDownloadTipCtrl::DrawProgressBar(CDC* pDC, CPoint* pPoint, CDownload* pDownload)
{
	CRect rcCell( pPoint->x, pPoint->y, m_sz.cx, pPoint->y + TIP_TEXTHEIGHT );
	pPoint->y += TIP_TEXTHEIGHT;

	pDC->Draw3dRect( &rcCell, CoolInterface.m_crTipBorder, CoolInterface.m_crTipBorder );
	rcCell.DeflateRect( 1, 1 );

	CFragmentBar::DrawDownload( pDC, &rcCell, pDownload, CoolInterface.m_crTipBack );

	rcCell.InflateRect( 1, 1 );
	pDC->ExcludeClipRect( &rcCell );
}

void CDownloadTipCtrl::DrawProgressBar(CDC* pDC, CPoint* pPoint, CDownloadSource* pSource)
{
	CRect rcCell( pPoint->x, pPoint->y, m_sz.cx, pPoint->y + TIP_TEXTHEIGHT );
	pPoint->y += TIP_TEXTHEIGHT;

	pDC->Draw3dRect( &rcCell, CoolInterface.m_crTipBorder, CoolInterface.m_crTipBorder );
	rcCell.DeflateRect( 1, 1 );

	pSource->Draw( pDC, &rcCell, CoolInterface.m_crTransferRanges );

	rcCell.InflateRect( 1, 1 );
	pDC->ExcludeClipRect( &rcCell );
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl timer

void CDownloadTipCtrl::OnTimer(UINT_PTR nIDEvent)
{
	CCoolTipCtrl::OnTimer( nIDEvent );

	if ( m_pGraph == NULL ) return;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 10 ) ) return;

	if ( m_pDownload && Downloads.Check( m_pDownload ) )
	{
		DWORD nSpeed = m_pDownload->GetMeasuredSpeed();
		m_pItem->Add( nSpeed );
		m_pGraph->m_nUpdates++;
		m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nSpeed );
		Invalidate( FALSE );
	}
	else if ( m_pSource && Downloads.Check( m_pSource ) )
	{
		if ( ! m_pSource->IsIdle() )
		{
			DWORD nSpeed = m_pSource->GetMeasuredSpeed();
			m_pItem->Add( nSpeed );
			m_pGraph->m_nUpdates++;
			m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nSpeed );
			Invalidate( FALSE );
		}
	}
}
