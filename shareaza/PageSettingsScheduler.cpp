//
// PageSettingsScheduler.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "Scheduler.h"
#include "PageSettingsScheduler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CSchedulerSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CSchedulerSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CSchedulerSettingsPage)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


#define HEADING_HEIGHT 32


/////////////////////////////////////////////////////////////////////////////
// CSchedulerSettingsPage property page

CSchedulerSettingsPage::CSchedulerSettingsPage() : CSettingsPage(CSchedulerSettingsPage::IDD)
{
	//{{AFX_DATA_INIT(CSchedulerSettingsPage)
	m_bSchedulerEnable = FALSE;
	m_nLimited = 0;
	m_bLimitedNetworks = TRUE;
	//}}AFX_DATA_INIT
}

CSchedulerSettingsPage::~CSchedulerSettingsPage()
{
}

void CSchedulerSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSchedulerSettingsPage)
	DDX_Check(pDX, IDC_SCHEDULER_ENABLE, m_bSchedulerEnable);
	DDX_Text(pDX, IDC_SCHEDULER_LIMITED, m_nLimited);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_SPIN, m_wndLimitedSpin);
	DDX_Check(pDX, IDC_SCHEDULER_LIMITED_NETWORKS, m_bLimitedNetworks);
	DDX_Control(pDX, IDC_SCHEDULER_DISPLAY, m_wndDisplay);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CSchedulerSettingsPage message handlers

BOOL CSchedulerSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_bmHeader.LoadBitmap( IDB_SCHEDULER_HEADER );

	CBitmap bmTimeSlices;
	bmTimeSlices.LoadBitmap( IDB_SCHEDULER_TIMESLICES );
	m_pTimeSlices.Create( 16, 16, ILC_COLOR32|ILC_MASK, 3, 0 ) ||
	m_pTimeSlices.Create( 16, 16, ILC_COLOR24|ILC_MASK, 3, 0 ) ||
	m_pTimeSlices.Create( 16, 16, ILC_COLOR16|ILC_MASK, 3, 0 );
	m_pTimeSlices.Add( &bmTimeSlices, RGB( 0, 255, 0 ) );

	CopyMemory( m_pSchedule, Schedule.m_pSchedule, sizeof( m_pSchedule ) );

	m_bSchedulerEnable	= Settings.Scheduler.Enable;
	m_nLimited			= Settings.Scheduler.LimitedBandwidth;
	m_bLimitedNetworks	= Settings.Scheduler.LimitedNetworks;

	m_nDownDay			= m_nHoverDay = 0xFF;
	m_nDownHour			= m_nHoverHour = 0xFF;

	m_wndLimitedSpin.SetRange( 5, 95 );

	LoadString (m_sDayName[0], IDS_DAY_SUNDAY);
	LoadString (m_sDayName[1], IDS_DAY_MONDAY);
	LoadString (m_sDayName[2], IDS_DAY_TUESDAY);
	LoadString (m_sDayName[3], IDS_DAY_WEDNESDAY);
	LoadString (m_sDayName[4], IDS_DAY_THURSDAY);
	LoadString (m_sDayName[5], IDS_DAY_FRIDAY);
	LoadString (m_sDayName[6], IDS_DAY_SATURDAY);
	LoadString (m_sDayName[7], IDS_DAY_EVERYDAY);

	UpdateData( FALSE );

	return TRUE;
}


void CSchedulerSettingsPage::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	CRect rc;
	CString strSliceDisplay;

	GetClientRect( &rc );

	// Use part of the heading as a zone for highlighting all days of a particular hour.
	rc.top += HEADING_HEIGHT - 10;
	rc.left += 4;

	rc.bottom = rc.top + 20 + ( 7 * 16 );
	rc.right = rc.left + 30 + ( 24 * 16 );

	if ( rc.PtInRect( point ) )
	{
		BOOL bAllDays  = point.y - rc.top < 20;
		BOOL bAllHours = point.x - rc.left < 30;
		int nHoverDay  = bAllDays ? 7 : ( point.y - ( rc.top + 20 ) ) / 16;
		int nHoverHour = bAllHours ? 24  : ( point.x - ( rc.left + 30 ) ) / 16;

		if ( ( nHoverDay != m_nHoverDay ) || ( nHoverHour != m_nHoverHour ) )
		{
			if ( nHoverDay != m_nHoverDay )
			{
				m_nHoverDay = BYTE( nHoverDay );
			}
			if ( nHoverHour != m_nHoverHour )
			{
				m_nHoverHour = BYTE( nHoverHour );
			}

			if ( m_nHoverHour < 24 && m_nHoverDay == 7 )
			{
				strSliceDisplay.Format(_T("%s, %d:00 - %d:59"), (LPCTSTR)m_sDayName[m_nHoverDay], m_nHoverHour, m_nHoverHour );
				::SetCursor( AfxGetApp()->LoadCursor( IDC_DOWN ) );
			}
			else if ( m_nHoverHour == 24 && m_nHoverDay < 7 )
			{
				strSliceDisplay.Format(_T("%s, 0:00 - 23:59"), (LPCTSTR)m_sDayName[m_nHoverDay] );
				::SetCursor( AfxGetApp()->LoadCursor( theApp.m_bRTL ? IDC_LEFT : IDC_RIGHT ) );
			}
			else if ( m_nHoverHour < 24 && m_nHoverDay < 7 )
			{
				strSliceDisplay.Format(_T("%s, %d:00 - %d:59"), (LPCTSTR)m_sDayName[m_nHoverDay], m_nHoverHour, m_nHoverHour );
				::SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
			}
			else
			{
				strSliceDisplay = _T("");
				::SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
			}
			m_wndDisplay.SetWindowText( strSliceDisplay );

			InvalidateSchedulerRect();
		}
	}
	else
	{
		::SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
		m_wndDisplay.SetWindowText( _T("") );

		if ( ( m_nHoverDay != 0xFF ) || ( m_nHoverHour != 0xFF ) )
		{
			m_nHoverDay = m_nHoverHour = 0xFF;
			m_nDownHour = m_nHoverHour	= 0xFF;

			ReleaseCapture();
			Invalidate();
		}
	}

	//CSettingsPage::OnMouseMove( nFlags, point );
}


void CSchedulerSettingsPage::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	if ( ( m_nHoverDay == 0xFF ) || ( m_nHoverHour == 0xFF ) ) return;
	m_nDownDay  = m_nHoverDay;
	m_nDownHour = m_nHoverHour;
	
	SetCapture();
	InvalidateSchedulerRect();
}

void CSchedulerSettingsPage::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	ToggleTimeBlocks(1);
}

void CSchedulerSettingsPage::OnRButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	if ( ( m_nHoverDay == 0xFF ) || ( m_nHoverHour == 0xFF ) ) return;
	m_nDownDay		= m_nHoverDay;
	m_nDownHour		= m_nHoverHour;
	SetCapture();
	InvalidateSchedulerRect();
}

void CSchedulerSettingsPage::OnRButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	ToggleTimeBlocks(2);
}

BOOL CSchedulerSettingsPage::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CSchedulerSettingsPage::OnPaint()
{
	//Draw the schedule box
	int nDay, nHour;
	CPaintDC dc( this );
	CRect rc;
	GetClientRect( &rc );

	rc.top += 10;
	rc.left += 34;
	CDC mdc;
	mdc.CreateCompatibleDC( &dc );

	CBitmap* pOldBmp = (CBitmap*)mdc.SelectObject( &m_bmHeader );
	dc.BitBlt( rc.left, rc.top, 385, HEADING_HEIGHT, &mdc, 0, 0, SRCCOPY );
	mdc.SelectObject( pOldBmp );
	mdc.DeleteDC();
	rc.top += HEADING_HEIGHT;

	for ( nDay = 0 ; nDay < 7 ; nDay++ )
	{
		for ( nHour = 0 ; nHour < 24 ; nHour++ )
		{
			if ( ( ( nDay == m_nHoverDay ) && ( nHour == m_nHoverHour ) ) ||
				 ( ( nDay == m_nHoverDay ) && ( m_nHoverHour == 24 ) ) ||
				 ( ( m_nHoverDay == 7 ) && ( nHour == m_nHoverHour ) ) 
				)
				ImageList_DrawEx( m_pTimeSlices, m_pSchedule[nDay][nHour], dc.GetSafeHdc(), rc.left + ( nHour * 16 ),
					rc.top + ( nDay * 16 ) , 16, 16, CLR_DEFAULT, RGB( 180, 180, 180), ILD_SELECTED );
			else
				ImageList_DrawEx( m_pTimeSlices, m_pSchedule[nDay][nHour], dc.GetSafeHdc(), rc.left + ( nHour * 16 ),
					rc.top + ( nDay * 16 ) , 16, 16, CLR_DEFAULT, CLR_DEFAULT, ILD_NORMAL );
		}
	}

	//Draw the border of the box
	dc.Draw3dRect( 33, 9, 385, 145, RGB( 0, 0, 0 ), RGB( 0, 0, 0 ) );

	//Draw the schedule time slices for the 'key'
	LONG nYpos = ( rc.bottom / 2 ) + 7;

	ImageList_DrawEx( m_pTimeSlices, SCHEDULE_OFF, dc.GetSafeHdc(),
					30 , nYpos , 16, 16, CLR_DEFAULT, CLR_DEFAULT, ILD_NORMAL );

	ImageList_DrawEx( m_pTimeSlices, SCHEDULE_LIMITED_SPEED, dc.GetSafeHdc(),
					30 , nYpos + 20 , 16, 16, CLR_DEFAULT, CLR_DEFAULT, ILD_NORMAL );

	ImageList_DrawEx( m_pTimeSlices, SCHEDULE_FULL_SPEED  , dc.GetSafeHdc(),
					30 , nYpos + 40 , 16, 16, CLR_DEFAULT, CLR_DEFAULT, ILD_NORMAL );

	//CSettingsPage::OnPaint();
}

void CSchedulerSettingsPage::OnOK()
{
	UpdateData();

	Settings.Scheduler.Enable			= m_bSchedulerEnable;
	Settings.Scheduler.LimitedBandwidth = m_nLimited;
	Settings.Scheduler.LimitedNetworks	= m_bLimitedNetworks;

	CopyMemory( Schedule.m_pSchedule , m_pSchedule, sizeof( Schedule.m_pSchedule ) );
	Schedule.Save();

	CSettingsPage::OnOK();
}

void CSchedulerSettingsPage::ToggleTimeBlocks(BYTE nDirection)
{
	ASSERT( nDirection == 1 || nDirection == 2 );
	if ( ( m_nHoverDay == 0xFF ) || ( m_nHoverHour == 0xFF ) ) return;

	if ( m_nDownDay != m_nHoverDay ) return;
	if ( m_nDownHour != m_nHoverHour ) return;

	if ( m_nHoverHour < 24 && m_nHoverDay == 7 )
	{
		for ( int iDay = 0; iDay < 7; iDay++ )
		{
			m_pSchedule[iDay][m_nHoverHour] += nDirection;
			m_pSchedule[iDay][m_nHoverHour] %= 3;
		}
	}
	else if ( m_nHoverHour == 24 && m_nHoverDay < 7 )
	{
		for ( int iHour = 0; iHour < 24; iHour++ )
		{
			m_pSchedule[m_nHoverDay][iHour] += nDirection;
			m_pSchedule[m_nHoverDay][iHour] %= 3;
		}
	}
	else if ( m_nHoverHour < 24 && m_nHoverDay < 7 )
	{
		m_pSchedule[m_nHoverDay][m_nHoverHour] += nDirection;
		m_pSchedule[m_nHoverDay][m_nHoverHour] %= 3;
	}

	m_nDownDay	= 0xFF;
	m_nDownHour	= 0xFF;

	ReleaseCapture();
	InvalidateSchedulerRect();
	UpdateWindow();
}

void CSchedulerSettingsPage::InvalidateSchedulerRect()
{
	CRect rc;

	GetClientRect( &rc );

	rc.top += HEADING_HEIGHT - 10;
	rc.left += 4;

	rc.bottom = rc.top + 20 + ( 7 * 16 );
	rc.right = rc.left + 30 + ( 24 * 16 );
	
	InvalidateRect(rc);
}

BOOL CSchedulerSettingsPage::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	// Do not pass the event down, so cursor will be handled only here
	return FALSE;
}
