//
// Statistics.h
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

#if !defined(AFX_STATISTICS_H__58CE4F23_CE39_4AAF_B3DE_1A77B801D9AC__INCLUDED_)
#define AFX_STATISTICS_H__58CE4F23_CE39_4AAF_B3DE_1A77B801D9AC__INCLUDED_

#pragma once


class CStatistics
{
// Construction
public:
	CStatistics();
	virtual ~CStatistics();

// Attributes
public:
	struct
	{
		struct
		{
			QWORD	Connected;
			QWORD	Hub;
			QWORD	Ultrapeer;
		} Timer;

		struct
		{
			QWORD	Outgoing;
			QWORD	Incoming;
		} Connections;

		struct
		{
			QWORD	Outgoing;
			QWORD	Incoming;
		} Bandwidth;

		struct
		{
			QWORD	Files;
			QWORD	Volume;
		} Uploads;

		struct
		{
			QWORD	Outgoing;
			QWORD	Incoming;
			QWORD	Routed;
			QWORD	Dropped;
			QWORD	Lost;
			QWORD	Queries;
		} Gnutella1, Gnutella2;

		struct
		{
			QWORD	Outgoing;
			QWORD	Incoming;
			QWORD	Dropped;
		} eDonkey;
	}
	Ever, Today, Last, Current;

	DWORD	m_tUpdate;
	DWORD	m_tSeconds;

// Operations
public:
	void	Update();
protected:
	static void Add(LPVOID pTarget, LPCVOID pSource, int nCount);

};

extern CStatistics Statistics;

#endif // !defined(AFX_STATISTICS_H__58CE4F23_CE39_4AAF_B3DE_1A77B801D9AC__INCLUDED_)
