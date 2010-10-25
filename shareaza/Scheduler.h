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

//TODO: Add new tasks here
enum ScheduleTask
{
	BANDWIDTH_FULLSPEED = 0,
	BANDWIDTH_REDUCEDSPEED,
	BANDWIDTH_STOP,
	SYSTEM_DIALUP_DC,
	SYSTEM_EXIT,
	SYSTEM_SHUTDOWN,
	SYSTEM_START
};

//////////////////////////////////////////////////////////////////////
// CScheduler class: Controls scheduler operations

class CScheduler  
{
public:
	static void		Execute(const CString& sTaskData);
	static void		Execute(HWND hShareazaWnd, const CString& sTaskData);
	static void		HangUpConnection();
	static bool		ShutDownComputer();
	static bool		SetShutdownRights();
};
