//
// PageSettingsScheduler.cpp
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


#define HEADING_HEIGHT 32


/////////////////////////////////////////////////////////////////////////////
// CSchedulerSettingsPage property page

CSchedulerSettingsPage::CSchedulerSettingsPage() : CSettingsPage(CSchedulerSettingsPage::IDD)
{
	//{{AFX_DATA_INIT(CSchedulerSettingsPage)
	m_bSchedulerEnable = FALSE;
	//}}AFX_DATA_INIT
	CopyMemory( m_pSchedule, Schedule.m_pSchedule, 7 * 24 );
}

CSchedulerSettingsPage::~CSchedulerSettingsPage()
{
}

void CSchedulerSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSchedulerSettingsPage)
	DDX_Check(pDX, IDC_SCHEDULER_ENABLE, m_bSchedulerEnable);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CSchedulerSettingsPage message handlers

BOOL CSchedulerSettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();

	if ( ! m_bmHeader.LoadBitmap( IDB_SCHEDULER_HEADER ) )
		AfxMessageBox(_T("Error"),MB_OK);



	CBitmap bmTimeSlices;
	bmTimeSlices.LoadBitmap( IDB_SCHEDULER_TIMESLICES );
	m_pTimeSlices.Create( 16, 16, ILC_COLOR24, 3, 0 );
	m_pTimeSlices.Add( &bmTimeSlices, RGB( 0, 255, 0 ) );
	
	
	m_bSchedulerEnable = Settings.Scheduler.Enable;
	
	UpdateData( FALSE );

	return TRUE;
}


void CSchedulerSettingsPage::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect rc;

	GetClientRect( &rc );

	rc.top += 10 + HEADING_HEIGHT;
	rc.left += 30;


	rc.bottom = rc.top + ( 7 * 16 );
	rc.right = rc.left + ( 24 * 16 );

	if ( rc.PtInRect( point ) )
	{
		int nHoverDay = ( point.y - rc.top ) / 16;
		int nHoverHour = ( point.x - rc.left ) / 16;

		if ( nHoverDay != m_nHoverDay )
		{
			m_nHoverDay = nHoverDay;
			Invalidate();
		}
		if ( nHoverHour != m_nHoverHour )
		{
			m_nHoverHour = nHoverHour;
			Invalidate();
		}
	}
	else 
	{
		if ( m_nHoverDay )
		{
			m_nHoverDay = 0;
			Invalidate();
		}
		if ( m_nHoverHour )
		{
			m_nHoverHour = 0;
			Invalidate();
		}
	}

	//m_bKeyMode = FALSE;
	
	//CSettingsPage::OnMouseMove( nFlags, point );
}


void CSchedulerSettingsPage::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_nDownDay = m_nHoverDay;
	m_nDownHour = m_nHoverHour;
	SetCapture();
	Invalidate();
}

void CSchedulerSettingsPage::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ( m_nDownDay != m_nHoverDay ) return;
	if ( m_nDownHour != m_nHoverHour ) return;

	m_pSchedule[m_nHoverDay][m_nHoverHour] ++;
	m_pSchedule[m_nHoverDay][m_nHoverHour] %= 3;

	m_nDownDay = m_nHoverDay = 0;
	m_nDownHour = m_nHoverHour = 0;

	ReleaseCapture();
	Invalidate();
	UpdateWindow();
}

BOOL CSchedulerSettingsPage::OnEraseBkgnd(CDC* pDC) 
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
	rc.left += 30;
	CDC mdc;
	mdc.CreateCompatibleDC( &dc );

	CBitmap* pOldBmp = (CBitmap*)mdc.SelectObject( &m_bmHeader );
	dc.BitBlt( rc.left, rc.top, 384, HEADING_HEIGHT, &mdc, 0, 0, SRCCOPY );
	mdc.SelectObject( pOldBmp );
	mdc.DeleteDC();
	rc.top += HEADING_HEIGHT;

	for ( nDay = 0 ; nDay < 7 ; nDay++ )
	{
		for ( nHour = 0 ; nHour < 24 ; nHour++ )
		{
			ImageList_DrawEx( m_pTimeSlices, m_pSchedule[nDay][nHour], dc.GetSafeHdc(),
					rc.left + ( nHour * 16 ) , rc.top + ( nDay * 16 ) , 16, 16, 
					CLR_DEFAULT, CLR_DEFAULT, /*pSource->m_bSelected ? ILD_SELECTED :*/ ILD_NORMAL );
			
		}
	}

	//Draw the border of the box
	dc.Draw3dRect( 29, 9, 385, 145, RGB( 0, 0, 0 ), RGB( 0, 0, 0 ) );

	//Draw the schedule time slices for the 'key'
	ImageList_DrawEx( m_pTimeSlices, SCHEDULE_OFF, dc.GetSafeHdc(),
					40 ,  200 , 16, 16, CLR_DEFAULT, CLR_DEFAULT, ILD_NORMAL );

	ImageList_DrawEx( m_pTimeSlices, SCHEDULE_LIMITED_SPEED, dc.GetSafeHdc(),
					40 , 220 , 16, 16, CLR_DEFAULT, CLR_DEFAULT, ILD_NORMAL );

	ImageList_DrawEx( m_pTimeSlices, SCHEDULE_FULL_SPEED  , dc.GetSafeHdc(),
					40 , 240 , 16, 16, CLR_DEFAULT, CLR_DEFAULT, ILD_NORMAL );

	//CSettingsPage::OnPaint();
	
}

void CSchedulerSettingsPage::OnOK() 
{
	UpdateData();

	Settings.Scheduler.Enable = m_bSchedulerEnable;
	CopyMemory( Schedule.m_pSchedule , m_pSchedule, 7 * 24 );
	Schedule.Save();
	
	CSettingsPage::OnOK();
}

