//
// Scheduler.cpp
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
#include "atltime.h"
#include "Shareaza.h"
#include "Settings.h"
#include "DlgHelp.h"
#include "Scheduler.h"
#include "Network.h"

CScheduler Schedule;

//////////////////////////////////////////////////////////////////////
// CScheduler construction

CScheduler::CScheduler()
{
	FillMemory( m_pSchedule , 7 * 24, SCHEDULE_FULL_SPEED );
	m_nCurrentHour = 0xFF;
	m_tLastCheck = 0;
}

CScheduler::~CScheduler()
{
}

//////////////////////////////////////////////////////////////////////
// CScheduler load and save

BOOL CScheduler::Load()
{
	CFile pFile;
	CString strFile = Settings.General.Path + _T("\\Data\\Schedule.dat");

	if ( ! pFile.Open( strFile, CFile::modeRead ) ) 
	{
		theApp.Message( MSG_ERROR, _T("Failed to open Schedule.dat") );
		return FALSE;
	}
	
	try
	{
		CArchive ar( &pFile, CArchive::load );
		Serialize( ar );
		ar.Close();
	}
	catch ( CException* pException )
	{
		pException->Delete();
	}

	pFile.Close();

	m_nCurrentHour = 0xFF;	// Reset the current hour so the scheduler updates now.
	return TRUE;
}

void CScheduler::Save()
{
	CFile pFile;
	CString strFile = Settings.General.UserPath + _T("\\Data\\Schedule.dat");

	if ( pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) )
	{
		CArchive ar( &pFile, CArchive::store );
		Serialize( ar );
		ar.Close();
	}
	
	m_nCurrentHour = 0xFF;	// Reset the current hour so the scheduler updates now.
}

//////////////////////////////////////////////////////////////////////
// CScheduler serialize

void CScheduler::Serialize(CArchive& ar)
{
	int nDay, nHour;

	if ( ar.IsStoring() )
	{
		for ( nDay = 0 ; nDay < 7 ; nDay++ )
		{
			for ( nHour = 0 ; nHour < 24 ; nHour++ )
			{
				ar << m_pSchedule[nDay][nHour];
			}
		}
	}
	else
	{
		for ( nDay = 0 ; nDay < 7 ; nDay++ )
		{
			for ( nHour = 0 ; nHour < 24 ; nHour++ )
			{
				ar >> m_pSchedule[nDay][nHour];
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CScheduler Update
void CScheduler::Update()
{
	int nDay, nHour;

	if ( Settings.Scheduler.Enable )
	{
		DWORD tTicks = GetTickCount();

		if ( tTicks - m_tLastCheck < 20000 ) return; // Only run this once every 20 seconds since the CTime functions can be slow
		m_tLastCheck = tTicks;

		// Get the current time
		CTime tTime = CTime::GetCurrentTime();

		// Get current Day/Hour and check they are valid
		nDay = tTime.GetDayOfWeek() - 1;
		nHour = tTime.GetHour();
		if ( ( nDay >= 7 ) || ( nDay < 0 ) || ( nHour >= 24 ) || ( nHour < 0 ) )
		{
			// Really Really Strange Error That Should Never Happen. 
			theApp.Message( MSG_ERROR, _T("Scheduler received invalid time") );
			return;
		}

		// If the hour has changed, set the current status
		if ( m_nCurrentHour != nHour )
		{
			SetVariables( m_pSchedule[nDay][nHour] );
			m_nCurrentHour = nHour;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CScheduler set variables
void CScheduler::SetVariables(BYTE nCurrentSettings)
{
	switch ( nCurrentSettings )
	{
	case SCHEDULE_OFF:
		theApp.Message( MSG_DEBUG, _T("Scheduler: Shutting down") );
		Settings.Live.BandwidthScale = 0;
		Settings.Gnutella2.EnableToday	= FALSE;
		Settings.Gnutella1.EnableToday	= FALSE;
		Settings.eDonkey.EnableToday	= FALSE;
		if ( Network.IsConnected() ) Network.Disconnect();
		break;
	case SCHEDULE_LIMITED_SPEED:
		theApp.Message( MSG_DEBUG, _T("Scheduler: Limited speed") );
		Settings.Live.BandwidthScale = Settings.Scheduler.LimitedBandwidth;
		Settings.Gnutella2.EnableToday	= TRUE;
		Settings.Gnutella1.EnableToday	= Settings.Scheduler.LimitedNetworks ? FALSE :Settings.Gnutella1.EnableAlways;
		Settings.eDonkey.EnableToday	= Settings.Scheduler.LimitedNetworks ? FALSE :Settings.eDonkey.EnableAlways;
		if ( ! Network.IsConnected() ) Network.Connect( TRUE );
		break;
	case SCHEDULE_FULL_SPEED:
		theApp.Message( MSG_DEBUG, _T("Scheduler: Full Speed") );
		Settings.Live.BandwidthScale = 100;
		Settings.Gnutella2.EnableToday	= TRUE;
		Settings.Gnutella1.EnableToday	= Settings.Gnutella1.EnableAlways;
		Settings.eDonkey.EnableToday	= Settings.eDonkey.EnableAlways;
		if ( ! Network.IsConnected() ) Network.Connect( TRUE );
		break;
	default: //Error
		theApp.Message( MSG_ERROR, _T("Invalid value in schedule") );
		break;
	}
}
