//
// CtrlDownloadTip.cpp
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
#include "ShellIcons.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadTransferBT.h"
#include "FragmentedFile.h"
#include "FragmentBar.h"
#include "Skin.h"
#include "SHA.h"
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
	//{{AFX_MSG_MAP(CDownloadTipCtrl)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl construction

CDownloadTipCtrl::CDownloadTipCtrl()
{
	m_pGraph	= NULL;
	m_nIcon		= 0;
}

CDownloadTipCtrl::~CDownloadTipCtrl()
{
	if ( m_pGraph ) delete m_pGraph;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl events

BOOL CDownloadTipCtrl::OnPrepare()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return FALSE;

	CalcSizeHelper();
	
	return m_sz.cx > 0;
}

void CDownloadTipCtrl::OnCalcSize(CDC* pDC)
{
	if ( Downloads.Check( (CDownload*)m_pContext ) )
	{
		OnCalcSize( pDC, (CDownload*)m_pContext );
		m_sz.cx = max( m_sz.cx, 400 );
	}
	else if ( Downloads.Check( (CDownloadSource*)m_pContext ) )
	{
		OnCalcSize( pDC, (CDownloadSource*)m_pContext );
		m_sz.cx = max( m_sz.cx, 400 );
	}
}

void CDownloadTipCtrl::OnShow()
{
	if ( m_pGraph ) delete m_pGraph;

	m_pGraph	= CreateLineGraph();
	m_pItem		= new CGraphItem( 0, 0, RGB( 0xFF, 0, 0 ) );
	m_pGraph->AddItem( m_pItem );
}

void CDownloadTipCtrl::OnHide()
{
	if ( m_pGraph ) delete m_pGraph;
	m_pGraph = NULL;
	m_pItem = NULL;
}

void CDownloadTipCtrl::OnPaint(CDC* pDC)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	if ( Downloads.Check( (CDownload*)m_pContext ) )
	{
		OnPaint( pDC, (CDownload*)m_pContext );
	}
	else if ( Downloads.Check( (CDownloadSource*)m_pContext ) )
	{
		OnPaint( pDC, (CDownloadSource*)m_pContext );
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
	PrepareFileInfo( pDownload );
	
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

	m_sz.cy += TIP_RULE;
	AddSize( pDC, m_sSize, 80 );
	AddSize( pDC, m_sType, 80 );
	m_sz.cy += 36;
	m_sz.cy += TIP_RULE;
	
	if ( pDownload->m_bTorrentTrackerError && ( pDownload->m_sTorrentTrackerError ) )
	{
		m_bDrawError = TRUE;
		m_sz.cy += TIP_TEXTHEIGHT;
		m_sz.cy += TIP_RULE;
	}
	else
		m_bDrawError = FALSE;

	if ( pDownload->IsSeeding() )
	{
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	else if ( pDownload->m_bBTH )
	{
		m_sz.cy += TIP_TEXTHEIGHT * 5;
	}
	else
	{
		m_sz.cy += TIP_TEXTHEIGHT * 4;
	}
	
	if ( m_sURL.GetLength() )
	{
		m_sz.cy += TIP_RULE;
		AddSize( pDC, m_sURL );
		m_sz.cy += TIP_TEXTHEIGHT;
	}

	if ( ! pDownload->IsSeeding() )
	{
		m_sz.cy += 2;
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	
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
	CString str;
	
	DrawText( pDC, &pt, m_sName );
	pt.y += TIP_TEXTHEIGHT;
	
	if ( m_sSHA1.GetLength() )
	{
		if( pDownload->m_bSHA1Trusted ) pDC->SelectObject( &CoolInterface.m_fntNormal );
		else pDC->SelectObject( &CoolInterface.m_fntItalic );
		DrawText( pDC, &pt, m_sSHA1 );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( m_sED2K.GetLength() )
	{
		if( pDownload->m_bED2KTrusted ) pDC->SelectObject( &CoolInterface.m_fntNormal );
		else pDC->SelectObject( &CoolInterface.m_fntItalic );
		DrawText( pDC, &pt, m_sED2K );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( m_sBTH.GetLength() )
	{
		if( pDownload->m_bBTHTrusted ) pDC->SelectObject( &CoolInterface.m_fntNormal );
		else pDC->SelectObject( &CoolInterface.m_fntItalic );
		DrawText( pDC, &pt, m_sBTH );
		pt.y += TIP_TEXTHEIGHT;
	}

	pDC->SelectObject( &CoolInterface.m_fntNormal );
	
	DrawRule( pDC, &pt );
	
	ImageList_DrawEx( ShellIcons.GetHandle( 32 ), m_nIcon, pDC->GetSafeHdc(),
		pt.x, pt.y, 32, 32, CoolInterface.m_crTipBack, CLR_NONE, ILD_NORMAL );
	pDC->ExcludeClipRect( pt.x, pt.y, pt.x + 32, pt.y + 32 );
	
	pt.y += 2;
	pt.x += 40;
	LoadString( str, IDS_TIP_SIZE );
	DrawText( pDC, &pt, str + ':' );
	DrawText( pDC, &pt, m_sSize, 40 );
	pt.y += TIP_TEXTHEIGHT;
	LoadString( str, IDS_TIP_TYPE );
	DrawText( pDC, &pt, str + ':' );
	DrawText( pDC, &pt, m_sType, 40 );
	pt.x -= 40;
	pt.y -= TIP_TEXTHEIGHT + 2;
	pt.y += 34;
	
	DrawRule( pDC, &pt );
	
	CString strFormat, strETA, strSpeed, strVolume, strSources, strTorrentUpload;
	
	int nSourceCount	= pDownload->GetSourceCount();
	int nTransferCount	= pDownload->GetTransferCount();
	
	LoadString( strFormat, IDS_TIP_NA );
	
	if ( pDownload->IsMoving() )
	{
		strETA		= strFormat;
		strSpeed	= strFormat;
		LoadString( strSources, IDS_DLM_COMPLETED_WORD );
	}
	else if ( pDownload->IsPaused() )
	{
		strETA		= strFormat;
		strSpeed	= strFormat;
		strSources.Format( _T("%i"), nSourceCount );
	}
	else if ( nTransferCount )
	{
		DWORD nTime = pDownload->GetTimeRemaining();
		
		if ( nTime != 0xFFFFFFFF )
		{
			if ( nTime > 3600 )
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
		
		strSpeed = Settings.SmartVolume( pDownload->GetAverageSpeed() * 8, FALSE, TRUE );
		
		LoadString( strFormat, IDS_TIP_NOFN );
		strSources.Format( strFormat, nTransferCount, nSourceCount );
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
	
	if ( pDownload->IsStarted() )
	{
		LoadString( strFormat, IDS_DLM_VOLUME );
		strVolume.Format( strFormat,
			(LPCTSTR)Settings.SmartVolume( pDownload->GetVolumeComplete(), FALSE ),
			(LPCTSTR)Settings.SmartVolume( pDownload->m_nSize, FALSE ),
			pDownload->GetProgress() * 100.0f );
	}
	else
	{
		LoadString( strVolume, IDS_TIP_NA );
	}

	if ( pDownload->m_nTorrentUploaded )
	{
		LoadString( strFormat, IDS_DLM_UPLOADED );
		strTorrentUpload.Format( strFormat,
			(LPCTSTR)Settings.SmartVolume( pDownload->m_nTorrentUploaded, FALSE ),
			(LPCTSTR)Settings.SmartVolume( pDownload->m_nTorrentDownloaded, FALSE ),
			pDownload->GetRatio() * 100.0f );
	}
	else
	{
		LoadString( strFormat, IDS_DLM_UPLOADED );
		strTorrentUpload.Format( strFormat, _T("0"),
			(LPCTSTR)Settings.SmartVolume( pDownload->m_nTorrentDownloaded, FALSE ), 0.0f );
	}
	
	//Draw the pop-up box
	if ( m_bDrawError )
	{	//Tracker error
		DrawText( pDC, &pt, pDownload->m_sTorrentTrackerError, 3 );
		pt.y += TIP_TEXTHEIGHT;
		DrawRule( pDC, &pt );
	}
	if ( ! pDownload->IsSeeding() )
	{	//Not applicable for seeding torrents.
		LoadString( strFormat, IDS_DLM_TOTAL_SPEED );
		DrawText( pDC, &pt, strFormat, 3 );
		DrawText( pDC, &pt, strSpeed, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
		LoadString( strFormat, IDS_DLM_ESTIMATED_TIME );
		DrawText( pDC, &pt, strFormat, 3 );
		DrawText( pDC, &pt, strETA, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
		LoadString( strFormat, IDS_DLM_VOLUME_DOWNLOADED );
		DrawText( pDC, &pt, strFormat, 3 );
		DrawText( pDC, &pt, strVolume, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( pDownload->m_bBTH )
	{	//Only torrents have the uploaded count
		LoadString( strFormat, IDS_DLM_VOLUME_UPLOADED );
		DrawText( pDC, &pt, strFormat, 3 );
		DrawText( pDC, &pt, strTorrentUpload, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( ! pDownload->IsSeeding() )
	{	//Not applicable for seeding torrents.
		LoadString( strFormat, IDS_DLM_NUMBER_OF_SOURCES );
		DrawText( pDC, &pt, strFormat, 3 );
		DrawText( pDC, &pt, strSources, m_nStatWidth );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( m_sURL.GetLength() )
	{	//Draw URL if present
		DrawRule( pDC, &pt );
		DrawText( pDC, &pt, m_sURL );
		pt.y += TIP_TEXTHEIGHT;
	}

	if ( ! pDownload->IsSeeding() )
	{	//Not applicable for seeding torrents.
		pt.y += 2;
		DrawProgressBar( pDC, &pt, pDownload );
		pt.y += TIP_GAP;
	}
	
	if ( m_bDrawGraph )
	{	//Don't draw empty graph.
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

void CDownloadTipCtrl::PrepareFileInfo(CDownload* pDownload)
{
	m_sName = pDownload->m_sRemoteName;
	m_sSize = Settings.SmartVolume( pDownload->m_nSize, FALSE );
	if ( pDownload->m_nSize == SIZE_UNKNOWN ) m_sSize = _T("?");
	
	m_sSHA1.Empty();
	m_sED2K.Empty();
	m_sBTH.Empty();
	m_sURL.Empty();
	
	if ( pDownload->m_bSHA1 )
	{
		m_sSHA1 = _T("sha1:") + CSHA::HashToString( &pDownload->m_pSHA1 );
	}
	if ( pDownload->m_bED2K )
	{
		m_sED2K = _T("ed2k:") + CED2K::HashToString( &pDownload->m_pED2K );
	}
	if ( pDownload->m_bBTH )
	{
		m_sBTH = _T("btih:") + CSHA::HashToString( &pDownload->m_pBTH );
		m_sURL = pDownload->m_pTorrent.m_sTracker;
	}
	
	int nPeriod = m_sName.ReverseFind( '.' );
	
	m_sType.Empty();
	m_nIcon = 0;
	
	if ( nPeriod > 0 )
	{
		CString strType = m_sName.Mid( nPeriod );
		CString strName, strMime;
		
		ShellIcons.Lookup( strType, NULL, NULL, &strName, &strMime );
		m_nIcon = ShellIcons.Get( strType, 32 );
		
		if ( strName.GetLength() )
		{
			m_sType = strName;
			if ( strMime.GetLength() ) m_sType += _T(" (") + strMime + _T(")");
		}
		else
		{
			m_sType = strType.Mid( 1 );
		}
	}
	
	if ( m_sType.IsEmpty() ) m_sType = _T("Unknown");
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl source case

void CDownloadTipCtrl::OnCalcSize(CDC* pDC, CDownloadSource* pSource)
{
	CDownload* pDownload = pSource->m_pDownload;
	
	if ( pSource->m_sNick.GetLength() > 0 )
		m_sName = pSource->m_sNick + _T(" (") + inet_ntoa( pSource->m_pAddress ) + ')';
	else
		m_sName = inet_ntoa( pSource->m_pAddress );

	if( pSource->m_bPushOnly )
	{
		m_sName += _T(" (push)");
	}

	
	m_sURL = pSource->m_sURL;

	if ( m_sURL.GetLength() > 128 )
	{
		if ( LPCTSTR pszSlash = _tcschr( (LPCTSTR)m_sURL + 7, '/' ) )
		{
			int nFirst = pszSlash - (LPCTSTR)m_sSize;
			m_sURL = m_sURL.Left( nFirst + 1 ) + _T("...") + m_sURL.Right( 10 );
		}
	}
	
	m_pHeaderName.RemoveAll();
	m_pHeaderValue.RemoveAll();
	
	if ( pSource->m_pTransfer != NULL && Settings.General.GUIMode != GUI_BASIC )
	{
		for ( int nHeader = 0 ; nHeader < pSource->m_pTransfer->m_pHeaderName.GetSize() ; nHeader++ )
		{
			CString strName		= pSource->m_pTransfer->m_pHeaderName.GetAt( nHeader );
			CString strValue	= pSource->m_pTransfer->m_pHeaderValue.GetAt( nHeader );
			
			if ( strValue.GetLength() > 64 ) strValue = strValue.Left( 64 ) + _T("...");
			
			m_pHeaderName.Add( strName );
			m_pHeaderValue.Add( strValue );
		}
	}
	
	AddSize( pDC, m_sName );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	m_sz.cy += TIP_TEXTHEIGHT + TIP_RULE;

	AddSize( pDC, m_sURL, 80 );
	m_sz.cy += TIP_TEXTHEIGHT * 4;
	
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
}

void CDownloadTipCtrl::OnPaint(CDC* pDC, CDownloadSource* pSource)
{
	CDownload* pDownload = pSource->m_pDownload;
	CPoint pt( 0, 0 );

	DrawText( pDC, &pt, m_sName );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	pt.y += TIP_TEXTHEIGHT;

	DrawRule( pDC, &pt );

	CString strStatus, strSpeed, strText;

	if ( pSource->m_pTransfer != NULL )
	{
		DWORD nLimit = pSource->m_pTransfer->m_mInput.pLimit != NULL ?
			*pSource->m_pTransfer->m_mInput.pLimit : 0;
		
		strStatus = pSource->m_pTransfer->GetStateText( TRUE );
		
		if ( nLimit > 0 )
		{
			strSpeed.Format( _T("%s of %s"),
				(LPCTSTR)Settings.SmartVolume( pSource->m_pTransfer->GetMeasuredSpeed() * 8, FALSE, TRUE ),
				(LPCTSTR)Settings.SmartVolume( nLimit * 8, FALSE, TRUE ) );
		}
		else
		{
			strSpeed = Settings.SmartVolume( pSource->m_pTransfer->GetMeasuredSpeed() * 8, FALSE, TRUE );
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
	DrawText( pDC, &pt, m_sURL, 80 );
	pt.y += TIP_TEXTHEIGHT;
	
	LoadString( strText, IDS_TIP_USERAGENT );
	DrawText( pDC, &pt, strText );
	DrawText( pDC, &pt, pSource->m_sServer, 80 );
	pt.y += TIP_TEXTHEIGHT;

	pt.y += TIP_GAP;

	DrawProgressBar( pDC, &pt, pSource );
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

	CFragmentBar::DrawSource( pDC, &rcCell, pSource, CoolInterface.m_crTipBack );

	rcCell.InflateRect( 1, 1 );
	pDC->ExcludeClipRect( &rcCell );
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadTipCtrl timer

void CDownloadTipCtrl::OnTimer(UINT nIDEvent) 
{
	CCoolTipCtrl::OnTimer( nIDEvent );

	if ( m_pGraph == NULL ) return;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 10 ) ) return;

	if ( Downloads.Check( (CDownload*)m_pContext ) )
	{
		CDownload* pDownload = (CDownload*)m_pContext;
		DWORD nSpeed = pDownload->GetMeasuredSpeed() * 8;
		m_pItem->Add( nSpeed );
		m_pGraph->m_nUpdates++;
		m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nSpeed );
		Invalidate();
	}
	else if ( Downloads.Check( (CDownloadSource*)m_pContext ) )
	{
		CDownloadSource* pSource = (CDownloadSource*)m_pContext;

		if ( pSource->m_pTransfer )
		{
			DWORD nSpeed = pSource->m_pTransfer->GetMeasuredSpeed() * 8;
			m_pItem->Add( nSpeed );
			m_pGraph->m_nUpdates++;
			m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nSpeed );
			Invalidate();
		}
	}
}
