//
// ThreadImpl.h
//
// Copyright (c) Shareaza Development Team, 2008-2014.
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

#include "Shareaza.h"
#include "ShareazaThread.h"


class CThreadImpl
{
public:
	CThreadImpl()
		: m_pCancel		( FALSE, TRUE )
		, m_nThreadID	( 0 )
		, m_bCanceling	( FALSE )
	{
	}

	virtual ~CThreadImpl()
	{
		CRazaThread::DetachThread( m_nThreadID );
	}

private:
	CEvent			m_pCancel;		// Thread cancel event (signaled if abort requested)
	DWORD			m_nThreadID;	// Thread ID
	CEvent			m_pWakeup;		// Thread wakeup event (optional)
	volatile LONG	m_bCanceling;	// Thread is canceling

	static UINT ThreadStart(LPVOID pParam)
	{
		CThreadImpl* pThis = reinterpret_cast< CThreadImpl* >( pParam );
		pThis->OnRun();
		return 0;
	}

	CThreadImpl(const CThreadImpl&);
	CThreadImpl& operator=(const CThreadImpl&);

protected:
	virtual void OnRun() = 0;

	inline bool BeginThread(LPCSTR szName = NULL, int nPriority = THREAD_PRIORITY_NORMAL) throw()
	{
		if ( IsThreadAlive() )
			return true;

		m_pCancel.ResetEvent();	// Enable thread run
		return ( CRazaThread::BeginThread( szName, ThreadStart, this, nPriority, 0, 0, NULL, &m_nThreadID ) != NULL );
	}

public:
	inline void CloseThread(DWORD dwTimeout = ALMOST_INFINITE) throw()
	{
		m_pCancel.SetEvent();	// Ask thread for exit
		m_pWakeup.SetEvent();	// Wakeup thread if any
		if ( ! InterlockedCompareExchange( &m_bCanceling, TRUE, FALSE ) )
		{
			if ( m_nThreadID != GetCurrentThreadId() )
			{
				CRazaThread::CloseThread( m_nThreadID, dwTimeout );
				m_nThreadID = 0;
			}
			InterlockedExchange( &m_bCanceling, FALSE );
		}
	}

	inline void Wait() throw()
	{
		if ( ! InterlockedCompareExchange( &m_bCanceling, TRUE, FALSE ) )
		{
			if ( m_nThreadID != GetCurrentThreadId() )
			{
				CRazaThread::CloseThread( m_nThreadID, INFINITE );
				m_nThreadID = 0;
			}
			InterlockedExchange( &m_bCanceling, FALSE );
		}
	}

	inline bool Wakeup() throw()
	{
		return ( m_pWakeup.SetEvent() != FALSE );
	}

	inline void Doze(DWORD dwTimeout) throw()
	{
		SwitchToThread();
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

	// Can thread continue?
	inline bool IsThreadEnabled(DWORD dwTimeout = 0) const throw()
	{
		return ( WaitForSingleObject( m_pCancel, dwTimeout ) == WAIT_TIMEOUT );
	}

	inline bool IsThreadAlive() const throw()
	{
		return CRazaThread::IsThreadAlive( m_nThreadID );
	}

	inline void Exit() throw()
	{
		m_pCancel.SetEvent();
	}

	inline bool SetThreadPriority(int nPriority) throw()
	{
		return CRazaThread::SetThreadPriority( m_nThreadID, nPriority );
	}
};
