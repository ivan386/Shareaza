//
// Scheduler.cpp
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
#include "Buffer.h"
#include "DlgHelp.h"
#include "Scheduler.h"
#include "XML.h"
#include "Network.h"

void CScheduler::Execute(const CString& sTaskData)
{
	int nAction, nLimitDown, nLimitUp, nDisabled, nEnabled;
	if ( _stscanf( sTaskData, _T("%d:%d:%d:%d:%d"),
		&nAction, &nLimitDown, &nLimitUp, &nDisabled, &nEnabled ) != 5 ||
		nAction < BANDWIDTH_FULLSPEED || nAction > SYSTEM_START ||
		nLimitDown < 0 || nLimitDown > 100 ||
		nLimitUp < 0 || nLimitUp > 100 ||
		nDisabled < 0 ||
		nEnabled < 0 )
		// Invalid argument
		return;

	if ( nAction == BANDWIDTH_FULLSPEED ||
		 nAction == BANDWIDTH_REDUCEDSPEED )
	{
		if ( nEnabled & 1 )
			Settings.Gnutella1.EnableToday = true;
		else if ( nDisabled & 1 )
			Settings.Gnutella1.EnableToday = false;
		
		if ( nEnabled & 2 )
			Settings.Gnutella2.EnableToday = true;
		else if ( nDisabled & 2 )
			Settings.Gnutella2.EnableToday = false;
		
		if ( nEnabled & 4 )
			Settings.eDonkey.EnableToday = true;
		else if ( nDisabled & 4 )
			Settings.eDonkey.EnableToday = false;
		
		if ( nEnabled & 8 )
			Settings.DC.EnableToday = true;
		else if ( nDisabled & 8 )
			Settings.DC.EnableToday = false;
		
		if ( nEnabled & 16 )
			Settings.BitTorrent.EnableToday = true;
		else if ( nDisabled & 16 )
			Settings.BitTorrent.EnableToday = false;
	}
	
	switch ( nAction )
	{
	case BANDWIDTH_FULLSPEED:
		Settings.Live.BandwidthScale	= 100;
		// No Limit
		Settings.Bandwidth.Downloads	= 0;
		// Reset upload limit to 90% of capacity, trimmed down to the nearest KB.
		Settings.Bandwidth.Uploads = ( ( ( Settings.Connection.OutSpeed *
			( 100 - Settings.Uploads.FreeBandwidthFactor ) ) / 100 ) / Bytes ) * Kilobits;
		if ( ! Network.IsConnected() ) Network.Connect( TRUE );
		break;

	case BANDWIDTH_REDUCEDSPEED:
		if ( nLimitDown == nLimitUp )
		{
			Settings.Live.BandwidthScale	= nLimitDown;
			// No Limit
			Settings.Bandwidth.Downloads	= 0;
			// Reset upload limit to 90% of capacity, trimmed down to the nearest KB.
			Settings.Bandwidth.Uploads = ( ( ( Settings.Connection.OutSpeed *
				( 100 - Settings.Uploads.FreeBandwidthFactor ) ) / 100 ) / Bytes ) * Kilobits;
		}
		else
		{
			Settings.Live.BandwidthScale	= 100;
			Settings.Bandwidth.Downloads	= ( ( Settings.Connection.InSpeed  * Kilobits / Bytes ) * nLimitDown ) / 100;
			Settings.Bandwidth.Uploads		= ( ( Settings.Connection.OutSpeed * Kilobits / Bytes ) * nLimitUp   ) / 100;
		}
		if ( ! Network.IsConnected() ) Network.Connect( TRUE );
		break;

	case BANDWIDTH_STOP:
		Settings.Live.BandwidthScale	= 0;
		Settings.Gnutella1.EnableToday	= false;
		Settings.Gnutella2.EnableToday	= false;
		Settings.eDonkey.EnableToday	= false;
		Settings.DC.EnableToday			= false;
		Settings.BitTorrent.EnableToday	= false;
		if ( Network.IsConnected() ) Network.Disconnect();
		break;

	case SYSTEM_DIALUP_DC:
		Settings.Live.BandwidthScale	= 0;
		Settings.Gnutella1.EnableToday	= false;
		Settings.Gnutella2.EnableToday	= false;
		Settings.eDonkey.EnableToday	= false;
		Settings.DC.EnableToday			= false;
		Settings.BitTorrent.EnableToday	= false;
		if ( Network.IsConnected() ) Network.Disconnect();
		HangUpConnection();
		break;

	case SYSTEM_EXIT:
		// Close self
		PostMainWndMessage( WM_CLOSE );
		break;

	case SYSTEM_SHUTDOWN:
		// Begin shutdown
		if ( SetShutdownRights()&& ShutDownComputer() )
		{
			// Close Shareaza if shutdown successfully started
			PostMainWndMessage( WM_CLOSE );
		}
		break;

	case SYSTEM_START:
		// Unexpected
		break;
	}
}

void CScheduler::Execute(HWND hWnd, const CString& sTaskData)
{
	DWORD_PTR dwResult;

	// Execute the selected scheduled task
	int nAction = _tstoi( sTaskData );
	switch ( nAction )
	{
	case SYSTEM_DIALUP_DC:
		if ( ! hWnd )
		{
			HangUpConnection();
		}
		return;

	case SYSTEM_EXIT:
		// Close remote Shareaza
		if ( hWnd )
		{
			SendMessageTimeout( hWnd, WM_CLOSE, 0, 0, SMTO_NORMAL, 250, &dwResult );
		}
		return;

	case SYSTEM_SHUTDOWN:
		// Begin shutdown
		if ( SetShutdownRights() && ShutDownComputer() )
		{
			// Close remote Shareaza
			if ( hWnd )
			{
				SendMessageTimeout( hWnd, WM_CLOSE, 0, 0, SMTO_NORMAL, 250, &dwResult );
			}
		}
		return;

	case SYSTEM_START:
		// Unexpected
		return;
	}

	// Just pass data to already running Shareaza
	if ( hWnd )
	{
		COPYDATASTRUCT cd =
		{
			COPYDATA_SCHEDULER,
			sTaskData.GetLength() * sizeof( TCHAR ),
			(PVOID)(LPCTSTR)sTaskData
		};
		SendMessageTimeout( hWnd, WM_COPYDATA, NULL, (WPARAM)&cd, SMTO_NORMAL, 250, &dwResult );
	}
}

void CScheduler::HangUpConnection()
{
	DWORD dwCb = sizeof( RASCONN );
	DWORD dwConnections = 0;
	RASCONN* lpRasConn = NULL;
	LPRASCONNSTATUS RasConStatus = 0;

	for (;;)
	{
		// Free the memory if necessary.
		if ( lpRasConn != NULL )
		{
			HeapFree( GetProcessHeap(), 0, lpRasConn );
			lpRasConn = NULL;
		}

		// Allocate the size needed for the RAS structure.
		lpRasConn = (RASCONN*)HeapAlloc( GetProcessHeap(), 0, dwCb );
		if ( ! lpRasConn )
			// Out of memory
			return;

		// Set the first structure size for version checking purposes.
		lpRasConn->dwSize = sizeof( RASCONN );

		// Call the RAS API 
		DWORD ret = RasEnumConnections( lpRasConn, &dwCb, &dwConnections );
		if ( ret == 0 )
			// Ok
			break;
		if ( ret == ERROR_NOT_ENOUGH_MEMORY )
			// Re-allocate more memory
			continue;
		// Error
		return;
	}

	DWORD loop = 0;
	for ( DWORD i = 0; i < dwConnections; ++i ) // Loop through all current connections
	{
		RasHangUp( lpRasConn[i].hrasconn ); // Hang up the connection
		while( ( RasGetConnectStatus( lpRasConn[i].hrasconn,RasConStatus ) || ( loop > 10 ) ) )
		{
			// Loop until the connection handle is invalid, or 3 seconds have passed
			Sleep( 300 );
			loop++;
		}
	}

	// Free the memory if necessary.
	if ( lpRasConn != NULL )
	{
		HeapFree( GetProcessHeap(), 0, lpRasConn );
		lpRasConn = NULL;
	}
}

bool  CScheduler::ShutDownComputer()
{
	int ShutdownSuccess = 0;
	
	// Try 2000/XP way first
	ShutdownSuccess = InitiateSystemShutdownEx( NULL,_T("Shareaza Scheduled Shutdown\n\nA system shutdown was scheduled using Shareaza. The system will now shut down."), 30, FALSE, FALSE, SHTDN_REASON_FLAG_USER_DEFINED );
	
	// Fall back to 9x way if this does not work
	if ( !ShutdownSuccess && GetLastError() != ERROR_SHUTDOWN_IN_PROGRESS )
	{
		UINT ShutdownFlags = EWX_POWEROFF; 
		DWORD dReason;
		dReason = ( SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED );
		ShutdownSuccess = ExitWindowsEx( ShutdownFlags, dReason );
	}
	return (ShutdownSuccess != 0);
}

bool CScheduler::SetShutdownRights()
{
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 

	// Get a token for this process. 
	if ( !OpenProcessToken( GetCurrentProcess(), 
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) ) 
		return( FALSE ); 

	// Get the LUID for the shutdown privilege. 
	LookupPrivilegeValue( NULL, SE_SHUTDOWN_NAME, 
		&tkp.Privileges[0].Luid ); 

	tkp.PrivilegeCount = 1;  // One privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

	// Get the shutdown privilege for this process. 
	AdjustTokenPrivileges( hToken, FALSE, &tkp, 0, 
		(PTOKEN_PRIVILEGES)NULL, 0 ); 

	if ( GetLastError() != ERROR_SUCCESS ) 
		return FALSE; 

	return TRUE;
}
