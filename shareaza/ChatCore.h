//
// ChatCore.h
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

#if !defined(AFX_CHATCORE_H__F550EC04_AC7C_42B1_BE5B_7CDF69EA2286__INCLUDED_)
#define AFX_CHATCORE_H__F550EC04_AC7C_42B1_BE5B_7CDF69EA2286__INCLUDED_

#pragma once

class CConnection;
class CChatSession;


class CChatCore  
{
// Construction
public:
	CChatCore();
	virtual ~CChatCore();
	
// Attributes
public:
	CMutex		m_pSection;
protected:
	CPtrList	m_pSessions;
	HANDLE		m_hThread;
	BOOL		m_bThread;
	CEvent		m_pWakeup;
	
// Operations
public:
	POSITION		GetIterator() const;
	CChatSession*	GetNext(POSITION& pos) const;
	int				GetCount() const;
	BOOL			Check(CChatSession* pSession) const;
	void			Close();
	void			OnAccept(CConnection* pConnection);
	BOOL			OnPush(GGUID* pGUID, CConnection* pConnection);
	void			StopThread();
protected:
	void			Add(CChatSession* pSession);
	void			Remove(CChatSession* pSession);
	void			StartThread();
protected:
	static UINT		ThreadStart(LPVOID pParam);
	void			OnRun();
	
	friend class CChatSession;
};

extern CChatCore ChatCore;

#endif // !defined(AFX_CHATCORE_H__F550EC04_AC7C_42B1_BE5B_7CDF69EA2286__INCLUDED_)
