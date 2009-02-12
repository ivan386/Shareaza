//
// Scheduler.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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
#if !defined(AFX_SCHEDULER_H__INCLUDED_)
#define AFX_SCHEDULER_H__INCLUDED_

#pragma once

class CScheduler
{
// Construction
public:
	CScheduler();
	~CScheduler();

// Attributes
public:
	BYTE			m_pSchedule[7][24];
protected:
	BYTE			m_nCurrentHour;
	DWORD			m_tLastCheck;


// Operations
public:
	BOOL			Load();
	void			Save();
	void			Update();
protected:
	void			SetVariables(BYTE nCurrentSettings);
	void			Serialize(CArchive& ar);
};

enum
{
	SCHEDULE_OFF, SCHEDULE_LIMITED_SPEED, SCHEDULE_FULL_SPEED
};

extern CScheduler Schedule;

#endif // !defined(AFX_SCHEDULER_H__INCLUDED_)
