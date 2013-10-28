//
// Statistics.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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


class CStatistics
{
public:
	CStatistics();
	~CStatistics();

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
			QWORD	Errors;
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
			QWORD	QueriesProcessed;
			QWORD	PingsSent;
			QWORD	PingsReceived;
			QWORD	PongsSent;
			QWORD	PongsReceived;
		} Gnutella1, Gnutella2;

		struct
		{
			QWORD	Outgoing;
			QWORD	Incoming;
			QWORD	Dropped;
		} BitTorrent, eDonkey;
	}
	Ever, Today, Last, Current;

	void	Update();

protected:
	QWORD	m_tSeconds;

	static void Add(LPVOID pTarget, LPCVOID pSource, int nCount);
};

extern CStatistics Statistics;
