//
// Scheduler.h
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
#pragma once

class CXMLElement;


#define SCHEDULER_SER_VERSION	1
//History: m_nDay is added

//TODO: Add new tasks here
enum ScheduleTask
{
	BANDWIDTH_FULL_SPEED = 0x1, BANDWIDTH_REDUCED_SPEED = 0x2, 
	BANDWIDTH_STOP = 0x4, SYSTEM_DISCONNECT = 0x8, 
	SYSTEM_EXIT = 0x10, SYSTEM_SHUTDOWN = 0x20
};

enum DayOfWeek
{
	SUNDAY = 0x1,
	MONDAY = 0x2,
	TUESDAY = 0x4,
	WEDNESDAY = 0x8,
	THURSDAY = 0x10,
	FRIDAY = 0x20,
	SATURDAY = 0x40
};

//////////////////////////////////////////////////////////////////////
// CScheduleTask class: Represents a scheduled task
//////////////////////////////////////////////////////////////////////

class CScheduleTask
{
// Construction
public:
	CScheduleTask(BOOL bCreate = TRUE);
	CScheduleTask(const CScheduleTask& pItem);
	virtual ~CScheduleTask();

// Attributes
public:
	unsigned int	m_nDays;		//Will have a combination of DayOfWeek
	unsigned int	m_nAction;		//Will have one of ScheduleTask values plus 0 as invalid state indicator
	bool		m_bSpecificDays;		//Task is scheduled for everyday or just today
	CString		m_sDescription;		//Optional task description
	CTime		m_tScheduleDateTime;//Time the task is scheduled for
	bool		m_bActive;			//Task should be executed or not
	bool		m_bExecuted;		//Task is executed or not
	bool		m_bToggleBandwidth;	//Up/Down bandwidth are limited seperately or not
	bool		m_bLimitedNetworks;	//Network is limited to G2 or not (in SCHEDULE_LIMITED_SPEED)
	int			m_nLimit;			//Bandwidth limit when m_bToggleBandwidth is FALSE
	int			m_nLimitDown;		//Down stream bandwidth limit when m_bToggleBandwidth is TRUE
	int			m_nLimitUp;			//Up stream bandwidth limit when m_bToggleBandwidth is TRUE
	GUID		m_pGUID;			//GUID for each scheduled item

// Operations
public:
	void			Serialize(CArchive& ar, int nVersion);
	CXMLElement*	ToXML();
	BOOL			FromXML(CXMLElement* pXML);
};

//////////////////////////////////////////////////////////////////////
// CScheduler class: Controls scheduler operations
//////////////////////////////////////////////////////////////////////

class CScheduler  
{
// Construction
public:
	CScheduler();
	virtual ~CScheduler();
	
// Attributes
public:
	static LPCTSTR					xmlns;
	
	//Lock is used when objects reads/wirtes from/to m_pScheduleItems 
	mutable CCriticalSection		m_pSection;

protected:
	CList< CScheduleTask* >			m_pScheduleTasks;
	//DWORD							m_tLastCheckTicks;


// Operations
public:

	//To iterate through m_pScheduleItems
	POSITION		GetIterator() const;
	CScheduleTask*	GetNext(POSITION& pos) const;
	int				GetCount() const;
	//Checks to see pItem exists in m_pScheduleItems or not, by comparing GUID values
	bool			Check(CScheduleTask* pItem) const;
	
	//It is called regularly by timers to see if any scheduled item should be executed
	//This method also sets Settings.Scheduler.Enable to indiate globally if any item 
	//is going to be executed
	void			CheckSchedule();

	//Is used to disconnect dial up connection
	void			HangUpConnection();

	//Is used to shut down computer
	bool			ShutDownComputer();

	//Is called by Load(). Tries to get shutdown privilege for the process
	bool			SetShutdownRights();
	
	//Is called by CheckSchedule(). Checks to see if Now is grater than Then or not.
	bool			IsScheduledTimePassed(CScheduleTask* pSchTask) const;
	
	//Checks to see if task should be executed today 0,
	//should have been executed in the past -1 or
	//should be executed later 1.
	int				ScheduleFromToday(CScheduleTask* pSchTask) const;

	//Adds a new task to m_pScheduleItems after giving it a GUID
	void			Add(CScheduleTask* pSchTask);
	
	//Removes a task from m_pScheduleItems if it exists in the m_pScheduleItems
	void			Remove(CScheduleTask* pSchTask);
	
	//Clears all m_pScheduleItems items
	void			Clear();

	BOOL			Load();
	BOOL			Save();
	BOOL			Import(LPCTSTR pszFile);

	//Calculates hours remaining to execution of a combination of scheduled tasks
	LONGLONG		GetHoursTo(unsigned int nTaskCombination);	//Example: nEventCombination = BANDWIDTH_FULL_SPEED | SYSTEM_DISCONNECT

protected:
	void			Serialize(CArchive& ar);
	BOOL			FromXML(CXMLElement* pXML);
	CXMLElement*	ToXML(BOOL bTasks);

	//Called by CheckSchedule() to execute a task
	void			ExecuteScheduledTask(CScheduleTask* pItem);

	CScheduleTask*	GetGUID(const GUID& pGUID) const;
};

extern CScheduler Scheduler;


