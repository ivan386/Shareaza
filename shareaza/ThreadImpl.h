//
// ThreadImpl.h
//
// Copyright (c) Shareaza Development Team, 2008.
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


#include "ShareazaThread.h"


class CThreadImpl
{
public:
	CThreadImpl() :
		m_bCompleted( false ),
		m_bThread( false ),
		m_hThread( NULL )
	{
	}
	virtual ~CThreadImpl()
	{
		CloseThread();
	}

private:
	volatile bool	m_bCompleted;	// TRUE - thread runs at least once
	volatile bool	m_bThread;		// TRUE - enable thread; FALSE - terminate thread.
	volatile HANDLE m_hThread;		// Thread handle
	CEvent			m_pWakeup;		// Thread wakeup event (optional)

	static UINT ThreadStart(LPVOID pParam)
	{
		CThreadImpl* pThis = reinterpret_cast< CThreadImpl* >( pParam );
		pThis->OnRun();
		pThis->m_bCompleted = true;	// Set complete status
		pThis->m_hThread = NULL;
		return 0;
	}

protected:
	virtual void OnRun() = 0;

public:
	inline bool BeginThread(LPCSTR szName = NULL, int nPriority = THREAD_PRIORITY_NORMAL) throw()
	{
		if ( ! IsThreadAlive() )
		{
			m_bCompleted = false;	// Reset complete status
			m_bThread = true;		// Enable thread run
			m_hThread = ::BeginThread( szName, ThreadStart, this, nPriority );
		}
		return ( m_hThread != NULL );
	}

	inline void CloseThread(DWORD dwTimeout = ALMOST_INFINITE) throw()
	{
		Exit();		// Ask thread for exit
		Wakeup();	// Wakeup thread if any
		::CloseThread( (HANDLE*)&m_hThread, dwTimeout );
	}

	inline void Wait() throw()
	{
		::CloseThread( (HANDLE*)&m_hThread, INFINITE );
	}

	inline bool Wakeup() throw()
	{
		return ( m_pWakeup.SetEvent() != FALSE );
	}

	inline void Doze(DWORD dwTimeout = INFINITE) throw()
	{
		do
		{
			SafeMessageLoop();
		}
		while( MsgWaitForMultipleObjects( 1, &m_pWakeup.m_hObject, FALSE, dwTimeout,
			QS_ALLINPUT | QS_ALLPOSTMESSAGE ) == WAIT_OBJECT_0 + 1 );
	}

	inline HANDLE GetWakeupEvent() const throw()
	{
		return m_pWakeup;
	}

	inline bool IsThreadCompleted() const throw()
	{
		return m_bCompleted;
	}

	inline bool IsThreadEnabled() const throw()
	{
		return m_bThread;
	}

	inline bool IsThreadAlive() const throw()
	{
		return m_hThread && ( WaitForSingleObject( m_hThread, 0 ) == WAIT_TIMEOUT );
	}

	inline void Exit() throw()
	{
		m_bThread = false;
	}

	inline bool SetThreadPriority(int nPriority) throw()
	{
		return m_hThread && ( ::SetThreadPriority( m_hThread, nPriority ) != FALSE );
	}
};
