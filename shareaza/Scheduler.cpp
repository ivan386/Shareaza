//
// Scheduler.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
//Explanation:
//Scheduler module consists of four objects which work together to setup and execute
//user defined tasks. These objects are:
//ScheduleTask: Keeps type and time of execution for a particular task.
//Scheduler: Keeps a list of ScheduleTasks. Iterates through tasks to see if time has come
//for tasks and sets global Settings.Scheduler.Enable if not.
//WndScheduler: The window which shows the list of ScheduleTasks and gives user
//the ability to add, edit, remove, export or import tasks.
//DlgScheduleTask: A dialog to edit or create a ScheduleTask.

#include "StdAfx.h"
#include "atltime.h"
#include "Shareaza.h"
#include "Settings.h"
#include "DlgHelp.h"
#include "Scheduler.h"
#include "XML.h"
#include "Network.h"
#include "Ras.h"
#include "Reason.h"

CScheduler Scheduler;

//////////////////////////////////////////////////////////////////////
// CScheduler load and save

BOOL CScheduler::Load()
{
	CQuickLock oLock( Scheduler.m_pSection );
	CFile pFile;
	CString strFile = Settings.General.UserPath + _T("\\Data\\SchTasks.dat");

	// Try to open the scheduler file
	if ( ! pFile.Open( strFile, CFile::modeRead ) ) 
	{
		theApp.Message( MSG_ERROR, _T("Failed to open SchTasks.dat") );
		return FALSE;
	}
	try
	{
		CArchive ar( &pFile, CArchive::load );	// 4 KB buffer

		Serialize( ar );
		ar.Close();
	}
	catch ( CException* pException )
	{
		pException->Delete();
	}

	pFile.Close();

	return TRUE;
}

BOOL CScheduler::Save()
{
	CQuickLock oLock( Scheduler.m_pSection );
	CString strFile = Settings.General.UserPath + _T("\\Data\\SchTasks.dat");

	CFile pFile;
	if ( ! pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) )
		return FALSE;

	try
	{
		CArchive ar( &pFile, CArchive::store );	// 4 KB buffer
		try
		{
			Serialize( ar );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		return FALSE;
	}


	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CScheduler serialize

void CScheduler::Serialize(CArchive& ar)
{
	int nVersion = SCHEDULER_SER_VERSION;

	if (ar.IsStoring())
	{
		ar << nVersion;

		// Write the number of scheduled tasks
		ar.WriteCount( GetCount() );
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			// Get a pointer to each task
			CScheduleTask *pSchTask = GetNext( pos );
			// Store each task's data
			pSchTask->Serialize( ar, nVersion );
		}
	}
	else
	{
		// First clear any existing tasks
		Clear();

		ar >> nVersion;

		// Read the number of tasks to load
		for (int nNumTasks = ar.ReadCount(); nNumTasks > 0; nNumTasks--)
		{
			// Create a new instance of each task
			CScheduleTask *pSchTask = new CScheduleTask();
			// Read each task's data
			pSchTask->Serialize( ar,nVersion );
			// Add the task to the task list
			m_pScheduleTasks.AddTail( pSchTask );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CScheduleTask construction

CScheduleTask::CScheduleTask(BOOL bCreate)
{
	m_nDays = 0x7F; //All days of week
	m_bSpecificDays = false;
	m_nAction = 0;		//Invalid value
	m_sDescription = "";
	m_tScheduleDateTime = 0;

	m_bActive = false;
	m_bExecuted = false;

	if ( bCreate ) CoCreateGuid( &m_pGUID );
}

CScheduleTask::CScheduleTask(const CScheduleTask& pSchTask)
{
	*this = pSchTask;
}

CScheduleTask::~CScheduleTask()
{
}

//////////////////////////////////////////////////////////////////////
// CScheduleTask class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CScheduleTask serialize

void CScheduleTask::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		if( nVersion == 1 )
		{
			// Store all task variables
			ar << m_bSpecificDays;
			ar << m_nAction;
			ar << m_sDescription;
			ar << m_tScheduleDateTime;
			ar << m_bActive;
			ar << m_bExecuted;
			ar << m_nLimit;
			ar << m_nLimitDown;
			ar << m_nLimitUp;
			ar << m_bToggleBandwidth;
			ar << m_bLimitedNetworks;
			ar << m_nDays;
			ar.Write( &m_pGUID, sizeof(GUID) );
		}
	}
	else
	{
		if( nVersion == 1 )
		{
			// Load all task variables
			ar >> m_bSpecificDays;
			ar >> m_nAction;
			ar >> m_sDescription;
			ar >> m_tScheduleDateTime;
			ar >> m_bActive;
			ar >> m_bExecuted;
			ar >> m_nLimit;
			ar >> m_nLimitDown;
			ar >> m_nLimitUp;
			ar >> m_bToggleBandwidth;
			ar >> m_bLimitedNetworks;
			ar >> m_nDays;
			ReadArchive( ar, &m_pGUID, sizeof(GUID) );
		}
	}
}
/////////////////////////////////////////////////////////////////////
// CSchedulerTask XML

CXMLElement* CScheduleTask::ToXML()
{
	CXMLElement* pXML = new CXMLElement( NULL, _T("task") );
	CString strValue;

	if ( m_sDescription.GetLength() )
	{
		pXML->AddAttribute( _T("description"), m_sDescription );
	}

	switch ( m_nAction)
	{
	case BANDWIDTH_FULL_SPEED:
		pXML->AddAttribute( _T("action"), _T("Bandwidth - Full Speed") );
		break;
	case BANDWIDTH_REDUCED_SPEED:
		pXML->AddAttribute( _T("action"), _T("Bandwidth - Reduced Speed") );
		break;
	case BANDWIDTH_STOP:
		pXML->AddAttribute( _T("action"), _T("Bandwidth - Stop") );
		break;
	case SYSTEM_DISCONNECT:
		pXML->AddAttribute( _T("action"), _T("System - Dial-Up Disconnect") );
		break;
	case SYSTEM_EXIT: 
		pXML->AddAttribute( _T("action"), _T("System - Exit Shareaza") );
		break;
	case SYSTEM_SHUTDOWN:
		pXML->AddAttribute( _T("action"), _T("System - Shutdown") );
		break;
	}

	strValue.Format(_T("%I64i"), m_tScheduleDateTime.GetTime());
	pXML->AddAttribute( _T("time"), strValue );


	if(m_bActive)
		pXML->AddAttribute( _T("active"), _T("Yes") );
	else
		pXML->AddAttribute( _T("active"), _T("No") );

	if(m_bSpecificDays)
		pXML->AddAttribute( _T("specificdays"), _T("Yes") );
	else
		pXML->AddAttribute( _T("specificdays"), _T("No") );

	if(m_bExecuted)
		pXML->AddAttribute( _T("executed"), _T("Yes") );
	else
		pXML->AddAttribute( _T("executed"), _T("No") );

	if(m_bToggleBandwidth)
		pXML->AddAttribute( _T("tglbandwidth"), _T("Yes") );
	else
		pXML->AddAttribute( _T("tglbandwidth"), _T("No") );

	if(m_bLimitedNetworks)
		pXML->AddAttribute( _T("limitednet"), _T("Yes") );
	else
		pXML->AddAttribute( _T("limitednet"), _T("No") );

	strValue.Format( _T("%i") , m_nLimit);
	pXML->AddAttribute( _T("limit"), strValue );

	strValue.Format( _T("%i") , m_nLimitDown );
	pXML->AddAttribute( _T("limitdown"), strValue );

	strValue.Format( _T("%i") , m_nLimitUp );
	pXML->AddAttribute( _T("limitup"), strValue );

	strValue.Format( _T("%i|%i|%i|%i|%i|%i|%i") , (m_nDays & SUNDAY) != 0, 
		(m_nDays & MONDAY) != 0,
		(m_nDays & TUESDAY) != 0, 
		(m_nDays & WEDNESDAY) != 0, 
		(m_nDays & THURSDAY) != 0, 
		(m_nDays & FRIDAY) != 0, 
		(m_nDays & SATURDAY) != 0);

	pXML->AddAttribute( _T("days"), strValue );

	wchar_t szGUID[39];
	szGUID[ StringFromGUID2( *(GUID*)&m_pGUID, szGUID, 39 ) - 2 ] = 0;
	pXML->AddAttribute( _T("guid"), (CString)&szGUID[1] );

	return pXML;
}

BOOL CScheduleTask::FromXML(CXMLElement* pXML)
{
	CString strValue;

	m_sDescription = pXML->GetAttributeValue( _T("description") );

	strValue = pXML->GetAttributeValue( _T("action") );

	if ( strValue.CompareNoCase( _T("Bandwidth - Full Speed") ) == 0 )
	{
		m_nAction = BANDWIDTH_FULL_SPEED;
	}
	else if ( strValue.CompareNoCase( _T("Bandwidth - Reduced Speed") ) == 0 )
	{
		m_nAction = BANDWIDTH_REDUCED_SPEED;
	}
	else if ( strValue.CompareNoCase( _T("Bandwidth - Stop") ) == 0 )
	{
		m_nAction = BANDWIDTH_STOP;
	}
	else if ( strValue.CompareNoCase( _T("System - Dial-Up Disconnect") ) == 0 )
	{
		m_nAction = SYSTEM_DISCONNECT;
	}
	else if ( strValue.CompareNoCase( _T("System - Exit Shareaza") ) == 0 )
	{
		m_nAction = SYSTEM_EXIT;
	}
	else if ( strValue.CompareNoCase( _T("System - Shutdown") ) == 0 )
	{
		m_nAction = SYSTEM_SHUTDOWN;
	}
	else
	{
		return FALSE;
	}

	strValue = pXML->GetAttributeValue( _T("time") );
	__time64_t tTemp;
	if( _stscanf( strValue, _T("%I64i"), &tTemp ) == EOF ) return FALSE;
	if( tTemp > 0 )
		m_tScheduleDateTime = tTemp;
	else
		return FALSE;


	strValue = pXML->GetAttributeValue( _T("active") );
	if( strValue.CompareNoCase( _T("Yes") ) == 0 )
		m_bActive = TRUE;
	else if( strValue.CompareNoCase( _T("No") ) == 0 )
		m_bActive = FALSE;
	else
		return FALSE;

	strValue = pXML->GetAttributeValue( _T("specificdays") );
	if( strValue.CompareNoCase( _T("Yes") ) == 0 )
		m_bSpecificDays = TRUE;
	else if( strValue.CompareNoCase( _T("No") ) == 0 )
		m_bSpecificDays = FALSE;
	else
		return FALSE;

	strValue = pXML->GetAttributeValue( _T("executed") );
	if( strValue.CompareNoCase( _T("Yes") ) == 0 )
		m_bExecuted = TRUE;
	else if( strValue.CompareNoCase( _T("No") ) == 0 )
		m_bExecuted = FALSE;
	else
		return FALSE;

	strValue = pXML->GetAttributeValue( _T("tglbandwidth") );
	if( strValue.CompareNoCase( _T("Yes") ) == 0 )
		m_bToggleBandwidth = TRUE;
	else if( strValue.CompareNoCase( _T("No") ) == 0 )
		m_bToggleBandwidth = FALSE;
	else
		return FALSE;

	strValue = pXML->GetAttributeValue( _T("limitednet") );
	if( strValue.CompareNoCase( _T("Yes") ) == 0 )
		m_bLimitedNetworks = TRUE;
	else if( strValue.CompareNoCase( _T("No") ) == 0 )
		m_bLimitedNetworks = FALSE;
	else
		return FALSE;

	strValue = pXML->GetAttributeValue( _T("limit") );
	if( _stscanf( strValue, _T("%i"), &m_nLimit) == EOF ) return FALSE;

	strValue = pXML->GetAttributeValue( _T("limitdown") );
	if( _stscanf( strValue, _T("%i"), &m_nLimitDown) == EOF ) return FALSE;

	strValue = pXML->GetAttributeValue( _T("limitup") );
	if( _stscanf( strValue, _T("%i"), &m_nLimitUp) == EOF ) return FALSE;

	strValue = pXML->GetAttributeValue( _T("days") );

	//_wtoi returns 0 on failure so bad data won't crash the application
	m_nDays = 0;
	wchar_t wcTmp;
	wcTmp = strValue[0];
	if( _wtoi(&wcTmp )) m_nDays |= SUNDAY; 
	wcTmp =strValue[2];
	if( _wtoi(&wcTmp )) m_nDays |= MONDAY;
	wcTmp =strValue[4];
	if( _wtoi(&wcTmp )) m_nDays |= TUESDAY; 
	wcTmp =	strValue[6];
	if( _wtoi(&wcTmp) ) m_nDays |= WEDNESDAY;
	wcTmp =	strValue[8];
	if( _wtoi(&wcTmp) ) m_nDays |= THURSDAY;
	wcTmp =strValue[10];
	if( _wtoi(&wcTmp) ) m_nDays |= FRIDAY;
	wcTmp =strValue[12];
	if( _wtoi(&wcTmp) ) m_nDays |= SATURDAY;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CScheduler class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CScheduler construction

CScheduler::CScheduler()
{

}

CScheduler::~CScheduler()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
//GUID

CScheduleTask* CScheduler::GetGUID(const GUID& pGUID) const
{
	CQuickLock oLock( m_pSection );

	for ( POSITION pos = m_pScheduleTasks.GetHeadPosition() ; pos ; )
	{
		CScheduleTask* pSchTask = m_pScheduleTasks.GetNext( pos );
		if ( pSchTask->m_pGUID == pGUID ) return pSchTask;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CScheduler item modification

void CScheduler::Add(CScheduleTask* pSchTask)
{
	CQuickLock oLock( m_pSection );

	CScheduleTask* pExistingTask = GetGUID( pSchTask->m_pGUID );
	if ( pExistingTask == NULL )
	{
		m_pScheduleTasks.AddHead( pSchTask );
	}
	else if ( pExistingTask != pSchTask )
	{
		*pExistingTask = *pSchTask;
		delete pSchTask;
	}
}

void CScheduler::Remove(CScheduleTask* pSchTask)
{
	CQuickLock oLock(m_pSection);
	POSITION pos = m_pScheduleTasks.Find( pSchTask );
	if ( pos ) m_pScheduleTasks.RemoveAt( pos );
	delete pSchTask;
}

void CScheduler::Clear()
{
	CQuickLock oLock(m_pSection);
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		delete GetNext( pos );
	}

	m_pScheduleTasks.RemoveAll();
}


//////////////////////////////////////////////////////////////////////
// CScheduler schedule checking

void CScheduler::CheckSchedule()
{
	//m_tLastCheckTicks = 0;
	bool bSchedulerIsEnabled = false;
	CTime tNow = CTime::GetCurrentTime();

	//Enable it to test GetHoursTo()
	//int nHoursToDisconnect = Scheduler.GetHoursTo(BANDWIDTH_STOP|SYSTEM_DISCONNECT|SYSTEM_EXIT|SYSTEM_SHUTDOWN );
	//theApp.Message( MSG_DEBUG, _T("Calculated time to disconnect is %i hours."), nHoursToDisconnect );

	CQuickLock oLock(m_pSection);
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CScheduleTask *pSchTask = GetNext(pos);

		// Check if a task should be executed
		if ( pSchTask->m_bActive )
		{	//We always ignore deactivated tasks
			bSchedulerIsEnabled = true;

			//Shorter if structure
			if(pSchTask->m_bExecuted)
			{
				//Task is executed and active. The task is either "Only Once" or "Specific Day(s) of Week"
				//In the first case if the date is for the days passed, its a task not executed and expired
				//In the second case it should mark as not executed so in the next CheckSchedule() call 
				//it will enter else block.
				if( !pSchTask->m_bSpecificDays || (ScheduleFromToday(pSchTask) < 0 ) )
					pSchTask->m_bExecuted = false;
			}
			else
			{
				//Time is passed so task should be executed if one of two conditions is met
				if( IsScheduledTimePassed(pSchTask)	)
				{	//It is scheduled for a specific date and time ("Only Once"). Checking for date
					if( (!pSchTask->m_bSpecificDays && ScheduleFromToday(pSchTask) == 0 )  || 
						//Or, it is scheduled for specific days of week. Checking for day
						(pSchTask->m_bSpecificDays &&  ((1 << (tNow.GetDayOfWeek() - 1)) & pSchTask->m_nDays)))
					{
						//static_cast<int>(pow(2.0f, tNow.GetDayOfWeek() - 1)
						//It will also mark it as executed
						ExecuteScheduledTask( pSchTask );
						//If active but not executed, scheduler will remain enabled
						bSchedulerIsEnabled = false;
						//Smart way for deactivating task if it is "Only Once"
						pSchTask->m_bActive = pSchTask->m_bSpecificDays;
						//Setting the date of task to last execution for further checks
						pSchTask->m_tScheduleDateTime = tNow;
					}
				}
			}

			/////////////////////////////////////////////////
			Scheduler.Save();
		}
	}

	//Scheduler is enable when an active task is scheduled
	Settings.Scheduler.Enable = bSchedulerIsEnabled;
}

//////////////////////////////////////////////////////////////////////
// CScheduler execute task
// TODO: Add new tasks here 
void CScheduler::ExecuteScheduledTask(CScheduleTask *pSchTask)
{
	// Execute the selected scheduled task
	pSchTask->m_bExecuted = true;
	switch ( pSchTask->m_nAction )
	{
	case BANDWIDTH_FULL_SPEED:  // Set the bandwidth to full speed
		theApp.Message( MSG_DEBUG, _T("Scheduler - Bandwidth: Full Speed") );
		Settings.Live.BandwidthScale = 100;
		Settings.Bandwidth.Downloads = 0;
		Settings.Bandwidth.Uploads = ( ( ( Settings.Connection.OutSpeed *
			( 100 - Settings.Uploads.FreeBandwidthFactor ) ) / 100 ) / 8 ) * 1024;
		Settings.Gnutella2.EnableToday	= TRUE;
		Settings.Gnutella1.EnableToday	= Settings.Gnutella1.EnableAlways;
		Settings.eDonkey.EnableToday	= Settings.eDonkey.EnableAlways;
		if ( ! Network.IsConnected() ) Network.Connect( TRUE );
		break;
	case BANDWIDTH_REDUCED_SPEED:  // Set the bandwidth to limited speed
		theApp.Message( MSG_DEBUG, _T("Scheduler - Bandwidth: Limited Speed") );

		if (!pSchTask->m_bToggleBandwidth)
		{
			Settings.Live.BandwidthScale = pSchTask->m_nLimit;
			Settings.Bandwidth.Uploads = ( ( ( Settings.Connection.OutSpeed *
				( 100 - Settings.Uploads.FreeBandwidthFactor ) ) / 100 ) / 8 ) * 1024;
			Settings.Bandwidth.Downloads = 0;
		}
		else
		{
			Settings.Live.BandwidthScale = 100;
			Settings.Bandwidth.Downloads = ( ( ( Settings.Connection.InSpeed * 1024) / 8) *
				pSchTask->m_nLimitDown ) / 100;
			Settings.Bandwidth.Uploads = ( ( ( Settings.Connection.OutSpeed * 1024) / 8) *
				pSchTask->m_nLimitUp ) / 100;
		}
		Settings.Gnutella2.EnableToday	= TRUE;
		Settings.Gnutella1.EnableToday	= pSchTask->m_bLimitedNetworks ? FALSE :Settings.Gnutella1.EnableAlways;
		Settings.eDonkey.EnableToday	= pSchTask->m_bLimitedNetworks ? FALSE :Settings.eDonkey.EnableAlways;
		if ( ! Network.IsConnected() ) Network.Connect( TRUE );
		break;
	case BANDWIDTH_STOP:	// Set the bandwidth to 0 and disconnect all networks
		theApp.Message( MSG_DEBUG, _T("Scheduler - Bandwidth: Stop") );
		Settings.Live.BandwidthScale = 0;
		Settings.Gnutella2.EnableToday	= FALSE;
		Settings.Gnutella1.EnableToday	= FALSE;
		Settings.eDonkey.EnableToday	= FALSE;
		if ( Network.IsConnected() ) Network.Disconnect();
		break;
	case SYSTEM_DISCONNECT:	// Disconnect
		theApp.Message( MSG_DEBUG, _T("Scheduler - System: Disconnect Dial-up Connection") );
		Settings.Live.BandwidthScale = 0;
		Settings.Gnutella2.EnableToday	= FALSE;
		Settings.Gnutella1.EnableToday	= FALSE;
		Settings.eDonkey.EnableToday	= FALSE;
		if ( Network.IsConnected() ) Network.Disconnect();
		HangUpConnection(); // Hang up the connection
		break;
	case SYSTEM_EXIT:		// Exit Shareaza
		theApp.Message( MSG_DEBUG, _T("Scheduler - System: Exit Shareaza") );
		if(!PostMainWndMessage( WM_CLOSE)) theApp.Message(MSG_ERROR, _T("Scheduler failed to send CLOSE message"));

		break;
	case SYSTEM_SHUTDOWN:	// Shut down the computer
		theApp.Message( MSG_DEBUG, _T("Scheduler - System: Shut Down the Computer") );

		// If we dont have shutdown rights
		if (!SetShutdownRights()) 
		{
			theApp.Message( MSG_DEBUG, _T("Insufficient rights to shut down the system") );
			return;
		}
		if (ShutDownComputer())// Close Shareaza if shutdown successfully started
			if(!PostMainWndMessage( WM_CLOSE)) theApp.Message(MSG_ERROR, _T("Scheduler failed to send CLOSE message"));

			else theApp.Message( MSG_DEBUG, _T("System shutdown failed!"));

			break;
	default: //Error
		pSchTask->m_bExecuted = false;
		theApp.Message( MSG_ERROR, _T("Invalid task in scheduler") );
		break;
	}
}

// Disconnect a dial-up connection
void CScheduler::HangUpConnection()
{
	DWORD dwCb = sizeof(RASCONN);
	DWORD dwConnections = 0;
	RASCONN* lpRasConn = NULL;
	LPRASCONNSTATUS RasConStatus = 0;

	// Allocate the size needed for the RAS structure.
	lpRasConn = (RASCONN*)HeapAlloc(GetProcessHeap(), 0, dwCb);

	// Set the structure size for version checking purposes.
	lpRasConn->dwSize = sizeof(RASCONN);

	// Call the RAS API 
	RasEnumConnections(lpRasConn, &dwCb, &dwConnections);

	DWORD i;
	int loop = 0;

	for (i = 0; i < dwConnections; i++) // Loop through all current connections
	{
		RasHangUp(lpRasConn[i].hrasconn); // Hang up the connection
		while((RasGetConnectStatus(lpRasConn[i].hrasconn,RasConStatus) || (loop > 10)))
		{
			// Loop until the connection handle is invalid, or 3 seconds have passed
			Sleep(300);
			loop++;
		}
	}

	// Free the memory if necessary.
	if (lpRasConn != NULL)
	{
		HeapFree(GetProcessHeap(), 0, lpRasConn);
		lpRasConn = NULL;
	}
}

bool  CScheduler::ShutDownComputer()
{
	int ShutdownSuccess = 0;
	UINT ShutdownFlags = (EWX_POWEROFF | EWX_FORCEIFHUNG); // Force only hung programs to close (not so safe)
	//UINT ShutdownFlags = (EWX_POWEROFF | EWX_FORCE); // Force all programs to close (safer)

	// Try 2000/XP way first
	ShutdownSuccess = InitiateSystemShutdownEx(NULL,_T("Shareaza Scheduled Shutdown\n\nA system shutdown was scheduled using Shareaza. The system will now shut down."),30,TRUE,FALSE,SHTDN_REASON_FLAG_USER_DEFINED);
	// Fall back to 9x way if this does not work
	if (!ShutdownSuccess)
	{
		DWORD dReason;
		dReason = (SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED);
		ShutdownSuccess = ExitWindowsEx(ShutdownFlags,dReason);
	}
	return (ShutdownSuccess != 0);
}

// Give the process shutdown rights
bool  CScheduler::SetShutdownRights()
{
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 

	// Get a token for this process. 
	if (!OpenProcessToken(GetCurrentProcess(), 
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
		return( FALSE ); 

	// Get the LUID for the shutdown privilege. 
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
		&tkp.Privileges[0].Luid); 

	tkp.PrivilegeCount = 1;  // One privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

	// Get the shutdown privilege for this process. 
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
		(PTOKEN_PRIVILEGES)NULL, 0); 

	if (GetLastError() != ERROR_SUCCESS) 
		return FALSE; 

	return TRUE;
}

bool  CScheduler::IsScheduledTimePassed(CScheduleTask* pSchTask) const
{
	CTime tNow = CTime::GetCurrentTime();

	if (tNow.GetHour() < pSchTask->m_tScheduleDateTime.GetHour() ) return false;
	else
		if (tNow.GetHour() == pSchTask->m_tScheduleDateTime.GetHour() )
			if (tNow.GetMinute() < pSchTask->m_tScheduleDateTime.GetMinute() ) return false;
			else
				if (tNow.GetMinute() == pSchTask->m_tScheduleDateTime.GetMinute() )
					if (tNow.GetSecond() < pSchTask->m_tScheduleDateTime.GetSecond() ) return false;
					else
						if (tNow.GetSecond() == pSchTask->m_tScheduleDateTime.GetSecond() )
							return true;
						else return true;
				else return true;
		else return true;
}

int  CScheduler::ScheduleFromToday(CScheduleTask* pSchTask) const
{
	CTime tNow  = CTime::GetCurrentTime();
	int nDirection = 0;
	if (tNow.GetYear() > pSchTask->m_tScheduleDateTime.GetYear() ) 
		nDirection = -1;
	else
		if( tNow.GetYear() == pSchTask->m_tScheduleDateTime.GetYear() )
			if (tNow.GetMonth() > pSchTask->m_tScheduleDateTime.GetMonth() )
				nDirection = -1;
			else
				if( tNow.GetMonth() == pSchTask->m_tScheduleDateTime.GetMonth() )
					if (tNow.GetDay() > pSchTask->m_tScheduleDateTime.GetDay() )
						nDirection = -1;
					else 
						if( tNow.GetDay() == pSchTask->m_tScheduleDateTime.GetDay() )
							nDirection = 0;
						else
							nDirection = 1;
				else
					nDirection = 1;
		else
			nDirection = 1;


	return nDirection;

}

//Calculates the different between current hour and shutdown hour
//Caller must first check to see if scheduler is enabled or not
LONGLONG CScheduler::GetHoursTo(unsigned int nTaskCombination)
{
	int nHoursToTasks = 0xFFFF;
	POSITION pos = GetIterator();
	CTime tNow = CTime::GetCurrentTime();

	CQuickLock oLock(m_pSection);
	while (pos)
	{
		CScheduleTask *pSchTask = GetNext( pos );
		if ( pSchTask->m_bActive && (pSchTask->m_nAction & nTaskCombination))
		{
			CTimeSpan tToTasks(1, 0, 0, 0);
			if( pSchTask->m_bSpecificDays )
			{
				for(int i = -1; i < 6 ; ++i)
				{
					if(((1 << ((tNow.GetDayOfWeek() + i)%7) )& pSchTask->m_nDays) && ( i!=-1 || !pSchTask->m_bExecuted ))
					{
						tToTasks = CTime(tNow.GetYear(), tNow.GetMonth(), tNow.GetDay(), pSchTask->m_tScheduleDateTime.GetHour(), pSchTask->m_tScheduleDateTime.GetMinute(), pSchTask->m_tScheduleDateTime.GetSecond() ) + CTimeSpan(i+1,0,0,0) - tNow;
						break;
					};
				}
			}
			else
			{
				tToTasks = pSchTask->m_tScheduleDateTime - tNow;
			}

			if( tToTasks.GetTotalHours() < nHoursToTasks ) 
				nHoursToTasks = tToTasks.GetTotalHours();
		}
	}

	return nHoursToTasks;
}

//////////////////////////////////////////////////////////////////////
// CScheduler XML

LPCTSTR CScheduler::xmlns = _T("http://shareaza.sourceforge.com/schemas/Scheduler.xsd");

CXMLElement* CScheduler::ToXML(BOOL bTasks)
{
	CXMLElement* pXML = new CXMLElement( NULL, _T("scheduler") );
	pXML->AddAttribute( _T("xmlns"), CScheduler::xmlns );

	if ( bTasks)
	{
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			pXML->AddElement( GetNext( pos )->ToXML() );
		}
	}

	return pXML;
}

BOOL CScheduler::FromXML(CXMLElement* pXML)
{
	if ( ! pXML->IsNamed( _T("scheduler") ) ) return FALSE;

	int nCount = 0;

	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pXML->GetNextElement( pos );

		if ( pElement->IsNamed( _T("task") ) )
		{
			CQuickLock oLock( m_pSection );
			CScheduleTask* pSchTask	= NULL;
			CString strGUID		= pElement->GetAttributeValue( _T("guid") );
			BOOL bExisting		= FALSE;
			GUID pGUID;

			if ( Hashes::fromGuid( strGUID, &pGUID ) )
			{
				if ( ( pSchTask = GetGUID( pGUID ) ) != NULL ) bExisting = TRUE;

				if ( pSchTask == NULL )
				{
					pSchTask = new CScheduleTask( FALSE );
					pSchTask->m_pGUID = pGUID;
				}
			}
			else
			{
				pSchTask = new CScheduleTask();
			}

			if ( pSchTask->FromXML( pElement ) )
			{
				if ( ! bExisting )
				{
					m_pScheduleTasks.AddTail( pSchTask );
				}

				nCount++;
			}
			else
			{
				//Unsuccessful read from XML
				if ( ! bExisting ) delete pSchTask;
			}
		}
	}

	return nCount > 0;
}

//////////////////////////////////////////////////////////////////////
// CScheduler import

BOOL CScheduler::Import(LPCTSTR pszFile)
{
	CString strText;
	CBuffer pBuffer;
	CFile pFile;

	if ( ! pFile.Open( pszFile, CFile::modeRead ) ) return FALSE;
	pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
	pBuffer.m_nLength = (DWORD)pFile.GetLength();
	pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
	pFile.Close();

	CXMLElement* pXML = CXMLElement::FromBytes( pBuffer.m_pBuffer, pBuffer.m_nLength, TRUE );
	BOOL bResult = FALSE;

	if ( pXML != NULL )
	{
		bResult = FromXML( pXML );
		delete pXML;
	}

	return bResult;
}
