//
// CtrlMatchTip.cpp
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
#include "CtrlCoolTip.h"
#include "CtrlMatchTip.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Library.h"
#include "SharedFile.h"
#include "MatchObjects.h"
#include "QueryHit.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "VendorCache.h"
#include "TigerTree.h"
#include "SHA.h"
#include "ED2K.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CMatchTipCtrl, CWnd)
	//{{AFX_MSG_MAP(CMatchTipCtrl)
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define TIP_DELAY		500
#define TIP_OFFSET_X	0
#define TIP_OFFSET_Y	24
#define TIP_MARGIN		6
#define TIP_TEXTHEIGHT	14


/////////////////////////////////////////////////////////////////////////////
// CMatchTipCtrl construction

CMatchTipCtrl::CMatchTipCtrl()
{
	m_pOwner	= NULL;
	m_bVisible	= FALSE;
	m_pFile		= NULL;
	m_pHit		= NULL;
	m_tOpen		= 0;
	m_nIcon		= 0;
	
	if ( m_hUser32 = LoadLibrary( _T("User32.dll") ) )
	{
		(FARPROC&)m_pfnSetLayeredWindowAttributes = GetProcAddress(
			m_hUser32, "SetLayeredWindowAttributes" );
	}
	else
	{
		m_pfnSetLayeredWindowAttributes = NULL;
	}
	
	if ( ! m_hClass ) m_hClass = AfxRegisterWndClass( CS_SAVEBITS );
	
	m_crBack	= CoolInterface.m_crTipBack;
	m_crText	= CoolInterface.m_crTipText;
	m_crBorder	= CCoolInterface::CalculateColour( m_crBack, (COLORREF)0, 100 );
	
	if ( m_brBack.m_hObject ) m_brBack.DeleteObject();
	m_brBack.CreateSolidBrush( m_crBack );
}

CMatchTipCtrl::~CMatchTipCtrl()
{
	if ( m_hUser32 ) FreeLibrary( m_hUser32 );
}

LPCTSTR		CMatchTipCtrl::m_hClass = NULL;
CBrush		CMatchTipCtrl::m_brBack;
COLORREF	CMatchTipCtrl::m_crBack;
COLORREF	CMatchTipCtrl::m_crText;
COLORREF	CMatchTipCtrl::m_crBorder;

/////////////////////////////////////////////////////////////////////////////
// CMatchTipCtrl operations

BOOL CMatchTipCtrl::Create(CWnd* pParentWnd) 
{
	CRect rc( 0, 0, 0, 0 );
	m_pOwner = pParentWnd;
	return CWnd::CreateEx( WS_EX_TOPMOST, m_hClass, NULL, WS_POPUP|WS_DISABLED, rc, pParentWnd, 0, NULL );
}

void CMatchTipCtrl::Show(CMatchFile* pFile, CQueryHit* pHit)
{
	if ( AfxGetMainWnd() != GetForegroundWindow() ) return;
	if ( ! Settings.Interface.TipSearch ) return;
	
	CPoint point;
	GetCursorPos( &point );
	
	if ( m_bVisible )
	{
		if ( pFile == m_pFile && pHit == m_pHit ) return;
		
		Hide();
		
		m_pFile	= pFile;
		m_pHit	= pHit;
		
		ShowInternal();
	}
	else if ( point != m_pOpen )
	{
		m_pFile	= pFile;
		m_pHit	= pHit;
		m_pOpen	= point;
		m_tOpen	= GetTickCount() + TIP_DELAY;
	}
}

void CMatchTipCtrl::Hide()
{
	m_pFile	= NULL;
	m_pHit	= NULL;
	m_tOpen	= 0;
	
	if ( m_bVisible )
	{
		ShowWindow( SW_HIDE );
		ModifyStyleEx( WS_EX_LAYERED, 0 );
		m_bVisible = FALSE;
		GetCursorPos( &m_pOpen );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMatchTipCtrl system message handlers

int CMatchTipCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	SetTimer( 1, 250, NULL );
	
	return 0;
}

void CMatchTipCtrl::OnDestroy() 
{
	KillTimer( 1 );
	CWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CMatchTipCtrl show logic message handlers

void CMatchTipCtrl::OnTimer(UINT nIDEvent) 
{
	CPoint point;
	GetCursorPos( &point );
	
	if ( WindowFromPoint( point ) != m_pOwner )
	{
		if ( m_bVisible ) Hide();
		return;
	}
	else
	{
		CWnd* pWnd = GetForegroundWindow();
		
		if ( pWnd != this && pWnd != AfxGetMainWnd() )
		{
			if ( m_bVisible ) Hide();
			return;
		}
	}
	
	if ( ! m_bVisible && m_tOpen && GetTickCount() >= m_tOpen )
	{
		m_tOpen = 0;
		if ( point == m_pOpen ) ShowInternal();
	}
}

void CMatchTipCtrl::ShowInternal()
{
	if ( m_bVisible ) return;
	
	if ( m_pHit != NULL )
	{
		LoadFromHit();
	}
	else if ( m_pFile != NULL )
	{
		LoadFromFile();
	}
	else
	{
		return;
	}
	
	if ( m_sName.GetLength() > 128 ) m_sName = m_sName.Left( 128 );
	
	m_bVisible = TRUE;
	
	CSize sz = ComputeSize();

	//** Multimonitor update
	HMONITOR hMonitor = NULL;
	MONITORINFO mi = {0};
	CRect rcMonitor( 0, 0, 0, 0 );

	CRect rc( m_pOpen.x + TIP_OFFSET_X, m_pOpen.y + TIP_OFFSET_Y, 0, 0 );
	rc.right = rc.left + sz.cx;
	rc.bottom = rc.top + sz.cy;
	

	if (GetSystemMetrics( SM_CMONITORS ) > 1)
	{
		mi.cbSize = sizeof(MONITORINFO);

		hMonitor = MonitorFromRect( rc, MONITOR_DEFAULTTONEAREST );
		if (NULL != hMonitor)
		{
		if ( GetMonitorInfoA(hMonitor, &mi) )
			rcMonitor = mi.rcWork;
		else
			hMonitor = NULL; // Fall back to GetSystemMetrics
		}

	}

	if ( NULL == hMonitor )
	{
		// Unimon system or something is wrong with multimon
		rcMonitor.right = GetSystemMetrics( SM_CXSCREEN );
		rcMonitor.bottom = GetSystemMetrics( SM_CYSCREEN );
	}

	if ( rc.right >= rcMonitor.right)
	{
		rc.OffsetRect( rcMonitor.right - rc.right - 4, 0 );
	}

	if ( rc.bottom >= rcMonitor.bottom )
	{
		rc.OffsetRect( 0, -sz.cy - TIP_OFFSET_Y - 4 );
	}

	/*
	CRect rc( m_pOpen.x + TIP_OFFSET_X, m_pOpen.y + TIP_OFFSET_Y, 0, 0 );
	rc.right = rc.left + sz.cx;
	rc.bottom = rc.top + sz.cy;
	
	if ( rc.right >= GetSystemMetrics( SM_CXSCREEN ) )
	{
		rc.OffsetRect( GetSystemMetrics( SM_CXSCREEN ) - rc.right - 4, 0 );
	}
	
	if ( rc.bottom >= GetSystemMetrics( SM_CYSCREEN ) )
	{
		rc.OffsetRect( 0, -sz.cy - TIP_OFFSET_Y - 4 );
	}*/
	
	if ( Settings.Interface.TipAlpha == 255 || m_pfnSetLayeredWindowAttributes == NULL )
	{
		ModifyStyleEx( WS_EX_LAYERED, 0 );
	}
	else
	{
		ModifyStyleEx( 0, WS_EX_LAYERED );
		(*m_pfnSetLayeredWindowAttributes)( GetSafeHwnd(),
			0, (BYTE)Settings.Interface.TipAlpha, LWA_ALPHA );
	}
	
	SetWindowPos( &wndTopMost, rc.left, rc.top, rc.Width(), rc.Height(),
		SWP_SHOWWINDOW|SWP_NOACTIVATE );
	UpdateWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CMatchTipCtrl load from content

void CMatchTipCtrl::LoadFromFile()
{
	m_sName = m_pFile->m_pBest->m_sName;
	m_sSize = m_pFile->m_sSize;
	LoadTypeInfo();
	
	if ( m_pFile->m_bSHA1 )
	{
		m_sSHA1 = _T("sha1:") + CSHA::HashToString( &m_pFile->m_pSHA1 );
	}
	else
	{
		m_sSHA1.Empty();
	}
	
	if ( m_pFile->m_bTiger )
	{
		m_sTiger = _T("tree:tiger/:") + CTigerNode::HashToString( &m_pFile->m_pTiger );
	}
	else
	{
		m_sTiger.Empty();
	}
	
	if ( m_pFile->m_bED2K )
	{
		m_sED2K = _T("ed2k:") + CED2K::HashToString( &m_pFile->m_pED2K );
	}
	else
	{
		m_sED2K.Empty();
	}

	if ( m_pFile->m_nFiltered == 1 && m_pFile->m_pBest->m_nPartial )
	{
		CString strFormat;
		LoadString( strFormat, IDS_TIP_PARTIAL );
		m_sPartial.Format( strFormat, 100.0f * (float)m_pFile->m_pBest->m_nPartial / (float)m_pFile->m_nSize );
	}
	else
	{
		m_sPartial.Empty();
	}
	
	if ( m_pFile->m_nFiltered == 1 && m_pFile->m_pBest->m_nUpSlots )
	{
		CString strFormat;
		LoadString( strFormat, IDS_TIP_QUEUE );
		m_sQueue.Format( strFormat, m_pFile->m_pBest->m_nUpSlots,
			max( 0, m_pFile->m_pBest->m_nUpQueue - m_pFile->m_pBest->m_nUpSlots ) );
	}
	else
	{
		m_sQueue.Empty();
	}
	
	m_pSchema = NULL;
	
	for ( CQueryHit* pHit = m_pFile->m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		m_pSchema = SchemaCache.Get( pHit->m_sSchemaURI );
		if ( m_pSchema ) break;
	}
	
	m_pMetadata.Setup( m_pSchema );
	
	if ( m_pSchema != NULL )
	{
		for ( pHit = m_pFile->m_pHits ; pHit ; pHit = pHit->m_pNext )
		{
			if ( pHit->m_pXML && m_pSchema->CheckURI( pHit->m_sSchemaURI ) )
			{
				m_pMetadata.Combine( pHit->m_pXML );
			}
		}
		
		m_pMetadata.Vote();
		m_pMetadata.Clean( 72 );
	}
	
	m_nRating = m_pFile->m_nRated ? m_pFile->m_nRating / m_pFile->m_nRated : 0;
	
	m_sStatus.Empty();
	
	if ( m_pFile->m_bExisting )
	{
		CLibraryFile* pExisting = NULL;
		
		if ( pExisting == NULL && m_pFile->m_bSHA1 == TRUE )
			pExisting = LibraryMaps.LookupFileBySHA1( &m_pFile->m_pSHA1, TRUE );
		if ( pExisting == NULL && m_pFile->m_bTiger == TRUE )
			pExisting = LibraryMaps.LookupFileByTiger( &m_pFile->m_pTiger, TRUE );
		if ( pExisting == NULL && m_pFile->m_bED2K == TRUE )
			pExisting = LibraryMaps.LookupFileByED2K( &m_pFile->m_pED2K, TRUE );
		
		if ( pExisting != NULL )
		{
			if ( pExisting->IsAvailable() )
			{
				LoadString( m_sStatus, IDS_TIP_EXISTS_LIBRARY );
				m_crStatus = RGB( 0, 128, 0 );
			}
			else
			{
				LoadString( m_sStatus, IDS_TIP_EXISTS_DELETED );
				m_crStatus = RGB( 255, 0, 0 );
				
				if ( pExisting->m_sComments.GetLength() )
				{
					LoadString( m_sStatus, IDS_TIP_EXISTS_BLACKLISTED );
					m_sStatus += pExisting->m_sComments;
				}
			}
			
			Library.Unlock();
		}
	}
	else if ( m_pFile->m_bDownload || m_pFile->m_pBest->m_bDownload )
	{
		LoadString( m_sStatus, IDS_TIP_EXISTS_DOWNLOAD );
		m_crStatus = RGB( 0, 0, 160 );
	}
	else if ( m_pFile->m_pBest->m_bBogus || ! m_pFile->m_bOneValid )
	{
		LoadString( m_sStatus, IDS_TIP_BOGUS );
		m_crStatus = RGB( 255, 0, 0 );
	}

	if ( m_pFile->m_nFiltered == 1 )
	{
		if ( m_pFile->m_pBest->m_sNick.GetLength() )
		{
			m_sUser.Format( _T("%s (%s - %s)"),
				(LPCTSTR)m_pFile->m_pBest->m_sNick,
				(LPCTSTR)CString( inet_ntoa( m_pFile->m_pBest->m_pAddress ) ),
				(LPCTSTR)m_pFile->m_pBest->m_pVendor->m_sName );
		}
		else
		{
			m_sUser.Format( _T("%s - %s"),
				(LPCTSTR)CString( inet_ntoa( m_pFile->m_pBest->m_pAddress ) ),
				(LPCTSTR)m_pFile->m_pBest->m_pVendor->m_sName );
		}
	}
	else
	{
		m_sUser.Empty();
	}
}

void CMatchTipCtrl::LoadFromHit()
{
	m_sName = m_pHit->m_sName;
	m_sSize = Settings.SmartVolume( m_pHit->m_nSize, FALSE );
	LoadTypeInfo();
	
	if ( m_pHit->m_bSHA1 )
	{
		m_sSHA1 = _T("sha1:") + CSHA::HashToString( &m_pHit->m_pSHA1 );
	}
	else
	{
		m_sSHA1.Empty();
	}
	
	if ( m_pHit->m_bTiger )
	{
		m_sTiger = _T("tree:tiger/:") + CTigerNode::HashToString( &m_pHit->m_pTiger );
	}
	else
	{
		m_sTiger.Empty();
	}
	
	if ( m_pHit->m_bED2K )
	{
		m_sED2K = _T("ed2k:") + CED2K::HashToString( &m_pHit->m_pED2K );
	}
	else
	{
		m_sED2K.Empty();
	}
	
	if ( m_pHit->m_nPartial )
	{
		CString strFormat;
		LoadString( strFormat, IDS_TIP_PARTIAL );
		m_sPartial.Format( strFormat, 100.0f * (float)m_pHit->m_nPartial / (float)m_pHit->m_nSize );
	}
	else
	{
		m_sPartial.Empty();
	}
	
	if ( m_pHit->m_nUpSlots )
	{
		CString strFormat;
		LoadString( strFormat, IDS_TIP_QUEUE );
		m_sQueue.Format( strFormat, m_pHit->m_nUpSlots,
			max( 0, m_pHit->m_nUpQueue - m_pHit->m_nUpSlots ) );
	}
	else
	{
		m_sQueue.Empty();
	}
	
	m_pSchema = SchemaCache.Get( m_pHit->m_sSchemaURI );
	
	m_pMetadata.Setup( m_pSchema );
	
	if ( m_pSchema != NULL )
	{
		if ( m_pHit->m_pXML && m_pSchema->CheckURI( m_pHit->m_sSchemaURI ) )
		{
			m_pMetadata.Combine( m_pHit->m_pXML );
		}
		m_pMetadata.Clean( 72 );
	}
	
	m_nRating = m_pHit->m_nRating;
	
	m_sStatus.Empty();
	
	if ( m_pFile->m_bExisting )
	{
		LoadString( m_sStatus, IDS_TIP_EXISTS_LIBRARY );
		m_crStatus = RGB( 0, 128, 0 );
	}
	else if ( m_pFile->m_bDownload || m_pHit->m_bDownload )
	{
		LoadString( m_sStatus, IDS_TIP_EXISTS_DOWNLOAD );
		m_crStatus = RGB( 0, 128, 0 );
	}
	else if ( m_pHit->m_bBogus )
	{
		LoadString( m_sStatus, IDS_TIP_BOGUS );
		m_crStatus = RGB( 255, 0, 0 );
	}
	
	if ( m_pHit->m_sNick.GetLength() )
	{
		m_sUser.Format( _T("%s (%s - %s)"),
			(LPCTSTR)m_pHit->m_sNick,
			(LPCTSTR)CString( inet_ntoa( m_pHit->m_pAddress ) ),
			(LPCTSTR)m_pHit->m_pVendor->m_sName );
	}
	else
	{
		m_sUser.Format( _T("%s - %s"),
			(LPCTSTR)CString( inet_ntoa( m_pHit->m_pAddress ) ),
			(LPCTSTR)m_pHit->m_pVendor->m_sName );
	}
}

BOOL CMatchTipCtrl::LoadTypeInfo()
{
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
	
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CMatchTipCtrl layout

CSize CMatchTipCtrl::ComputeSize()
{
	CClientDC dc( this );
	CSize sz( 0, 0 );
	
	CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntBold );
	
	ExpandSize( dc, sz, m_sName );
	sz.cy += TIP_TEXTHEIGHT;
	
	dc.SelectObject( &CoolInterface.m_fntNormal );

	if ( m_sUser.GetLength() )
	{
		ExpandSize( dc, sz, m_sUser );
		sz.cy += TIP_TEXTHEIGHT;
	}
	
	sz.cy += 5 + 6;
	
	if ( m_sStatus.GetLength() )
	{
		dc.SelectObject( &CoolInterface.m_fntBold );
		ExpandSize( dc, sz, m_sStatus );
		dc.SelectObject( &CoolInterface.m_fntNormal );
		sz.cy += TIP_TEXTHEIGHT;
		sz.cy += 5 + 6;
	}
	
	sz.cy += 32;
	ExpandSize( dc, sz, m_sSize, 80 );
	ExpandSize( dc, sz, m_sType, 80 );
	
	if ( m_sSHA1.GetLength() || m_sTiger.GetLength() || m_sED2K.GetLength() )
	{
		sz.cy += 5 + 6;
		
		if ( m_sSHA1.GetLength() )
		{
			ExpandSize( dc, sz, m_sSHA1 );
			sz.cy += TIP_TEXTHEIGHT;
		}
		
		if ( m_sTiger.GetLength() )
		{
			ExpandSize( dc, sz, m_sTiger );
			sz.cy += TIP_TEXTHEIGHT;
		}
		
		if ( m_sED2K.GetLength() )
		{
			ExpandSize( dc, sz, m_sED2K );
			sz.cy += TIP_TEXTHEIGHT;
		}
	}
	
	if ( m_sPartial.GetLength() || m_sQueue.GetLength() )
	{
		sz.cy += 5 + 6;
		
		if ( m_sPartial.GetLength() )
		{
			ExpandSize( dc, sz, m_sPartial );
			sz.cy += TIP_TEXTHEIGHT;
		}
		
		if ( m_sQueue.GetLength() )
		{
			ExpandSize( dc, sz, m_sQueue );
			sz.cy += TIP_TEXTHEIGHT;
		}
	}
	
	if ( m_pMetadata.GetCount() )
	{
		sz.cy += 5 + 6;
		
		int nValueWidth = 0;
		m_nKeyWidth = 0;
		
		m_pMetadata.ComputeWidth( &dc, m_nKeyWidth, nValueWidth );
		
		if ( m_nKeyWidth ) m_nKeyWidth += TIP_MARGIN;
		sz.cx = max( sz.cx, m_nKeyWidth + nValueWidth );
		sz.cy += TIP_TEXTHEIGHT * m_pMetadata.GetCount();
	}
	
	dc.SelectObject( pOldFont );
	
	sz.cx += TIP_MARGIN * 2;
	sz.cy += TIP_MARGIN * 2;
	
	return sz;
}

void CMatchTipCtrl::ExpandSize(CDC& dc, CSize& sz, LPCTSTR pszText, int nBase)
{
	CSize szText = dc.GetTextExtent( pszText, _tcslen( pszText ) );
	szText.cx += nBase;
	sz.cx = max( sz.cx, szText.cx );
}

/////////////////////////////////////////////////////////////////////////////
// CMatchTipCtrl painting

BOOL CMatchTipCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CMatchTipCtrl::OnPaint() 
{
	CPaintDC dc( this );
	CString str;
	CRect rc;
	
	GetClientRect( &rc );
	
	dc.Draw3dRect( &rc, m_crBorder, m_crBorder );
	rc.DeflateRect( 1, 1 );
	
	CFont* pOldFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntBold );
	CPoint pt( TIP_MARGIN, TIP_MARGIN );
	
	dc.SetTextColor( m_crText );
	
	DrawText( dc, pt, m_sName );
	pt.y += TIP_TEXTHEIGHT;
	
	dc.SelectObject( &CoolInterface.m_fntNormal );
	
	if ( m_sUser.GetLength() )
	{
		DrawText( dc, pt, m_sUser );
		pt.y += TIP_TEXTHEIGHT;
	}
	
	pt.y += 5;
	dc.Draw3dRect( rc.left + 2, pt.y, rc.Width() - 4, 1,
		m_crBorder, m_crBorder );
	dc.ExcludeClipRect( rc.left + 2, pt.y, rc.right - 2, pt.y + 1 );
	pt.y += 6;
	
	if ( m_sStatus.GetLength() )
	{
		dc.SetTextColor( m_crStatus );
		dc.SelectObject( &CoolInterface.m_fntBold );
		DrawText( dc, pt, m_sStatus );
		dc.SelectObject( &CoolInterface.m_fntNormal );
		dc.SetTextColor( m_crText );
		pt.y += TIP_TEXTHEIGHT;
		
		pt.y += 5;
		dc.Draw3dRect( rc.left + 2, pt.y, rc.Width() - 4, 1,
			m_crBorder, m_crBorder );
		dc.ExcludeClipRect( rc.left + 2, pt.y, rc.right - 2, pt.y + 1 );
		pt.y += 6;
	}
	
	ShellIcons.Draw( &dc, m_nIcon, 32, pt.x, pt.y, m_crBack );
	
	if ( m_nRating > 1 )
	{
		CPoint ptStar( rc.right - 3, pt.y - 2 );
		
		for ( int nRating = m_nRating - 1 ; nRating ; nRating-- )
		{
			ptStar.x -= 16;
			ShellIcons.Draw( &dc, SHI_STAR, 16, ptStar.x, ptStar.y, m_crBack );
		}
	}
	
	pt.x += 40;
	LoadString( str, IDS_TIP_SIZE );
	DrawText( dc, pt, str + ':' );
	pt.x += 40;
	DrawText( dc, pt, m_sSize );
	pt.x -= 40;
	pt.y += 16;
	LoadString( str, IDS_TIP_TYPE );
	DrawText( dc, pt, str + ':' );
	pt.x += 40;
	DrawText( dc, pt, m_sType );
	pt.x -= 40;
	pt.x -= 40;
	pt.y += 16;
	
	if ( m_sSHA1.GetLength() || m_sTiger.GetLength() || m_sED2K.GetLength() )
	{
		pt.y += 5;
		dc.Draw3dRect( rc.left + 2, pt.y, rc.Width() - 4, 1,
			m_crBorder, m_crBorder );
		dc.ExcludeClipRect( rc.left + 2, pt.y, rc.right - 2, pt.y + 1 );
		pt.y += 6;
		
		if ( m_sSHA1.GetLength() )
		{
			DrawText( dc, pt, m_sSHA1 );
			pt.y += TIP_TEXTHEIGHT;
		}
		
		if ( m_sTiger.GetLength() )
		{
			DrawText( dc, pt, m_sTiger );
			pt.y += TIP_TEXTHEIGHT;
		}
		
		if ( m_sED2K.GetLength() )
		{
			DrawText( dc, pt, m_sED2K );
			pt.y += TIP_TEXTHEIGHT;
		}
	}
	
	if ( m_sPartial.GetLength() || m_sQueue.GetLength() )
	{
		pt.y += 5;
		dc.Draw3dRect( rc.left + 2, pt.y, rc.Width() - 4, 1,
			m_crBorder, m_crBorder );
		dc.ExcludeClipRect( rc.left + 2, pt.y, rc.right - 2, pt.y + 1 );
		pt.y += 6;
		
		if ( m_sPartial.GetLength() )
		{
			DrawText( dc, pt, m_sPartial );
			pt.y += TIP_TEXTHEIGHT;
		}
		
		if ( m_sQueue.GetLength() )
		{
			DrawText( dc, pt, m_sQueue );
			pt.y += TIP_TEXTHEIGHT;
		}
	}
	
	if ( m_pMetadata.GetCount() )
	{
		pt.y += 5;
		dc.Draw3dRect( rc.left + 2, pt.y, rc.Width() - 4, 1,
			m_crBorder, m_crBorder );
		dc.ExcludeClipRect( rc.left + 2, pt.y, rc.right - 2, pt.y + 1 );
		pt.y += 6;
		
		for ( POSITION pos = m_pMetadata.GetIterator() ; pos ; )
		{
			CMetaItem* pItem = m_pMetadata.GetNext( pos );

			DrawText( dc, pt, pItem->m_sKey + ':' );
			pt.x += m_nKeyWidth;
			DrawText( dc, pt, pItem->m_sValue );
			pt.x -= m_nKeyWidth;
			pt.y += TIP_TEXTHEIGHT;
		}
	}
	
	dc.SelectObject( pOldFont );
	dc.FillSolidRect( &rc, m_crBack );
}

void CMatchTipCtrl::DrawText(CDC& dc, CPoint& pt, LPCTSTR pszText)
{
	CSize sz = dc.GetTextExtent( pszText, _tcslen( pszText ) );
	CRect rc( pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy );

	dc.SetBkColor( m_crBack );
	dc.ExtTextOut( pt.x, pt.y, ETO_CLIPPED|ETO_OPAQUE, &rc, pszText, _tcslen( pszText ), NULL );
	dc.ExcludeClipRect( &rc );
}

void CMatchTipCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	Hide();
}

void CMatchTipCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	Hide();
	CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}
