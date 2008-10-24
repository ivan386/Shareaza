//
// ShareazaThread.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "ShareazaThread.h"


IMPLEMENT_DYNAMIC(CRazaThread, CWinThread)

CCriticalSection		CRazaThread::m_ThreadMapSection;
CRazaThread::CThreadMap	CRazaThread::m_ThreadMap;

CRazaThread::CRazaThread(AFX_THREADPROC pfnThreadProc /*= NULL*/, LPVOID pParam /*= NULL*/) :
	CWinThread( NULL, pParam ),
	m_bCOM( FALSE ),
	m_pfnThreadProcExt( pfnThreadProc )
{
}

CRazaThread::~CRazaThread()
{
	if ( m_bCOM && m_nThreadID == GetCurrentThreadId() ) // Inside thread itself only
	{
		m_bCOM = FALSE;
		OleUninitialize();
	}

	ASSERT( m_bCOM == FALSE );

	Remove( m_hThread );
}

HANDLE CRazaThread::CreateThread(LPCSTR pszName, int nPriority /*= THREAD_PRIORITY_NORMAL*/,
	DWORD dwCreateFlags /*= 0*/, UINT nStackSize /*= 0*/,
	LPSECURITY_ATTRIBUTES lpSecurityAttrs /*= NULL*/)
{
	if ( CWinThread::CreateThread( dwCreateFlags | CREATE_SUSPENDED, nStackSize,
		lpSecurityAttrs ) )
	{
		Add( this, pszName );

		VERIFY( SetThreadPriority( nPriority ) );

		if ( ! ( dwCreateFlags & CREATE_SUSPENDED ) )
			VERIFY( ResumeThread() != (DWORD)-1 );

		return m_hThread;
	}

	Delete();

	return NULL;
}

BOOL CRazaThread::InitInstance()
{
	CWinThread::InitInstance();

	ASSERT( m_nThreadID == GetCurrentThreadId() );
	ASSERT( m_bCOM == FALSE );
	m_bCOM = SUCCEEDED( OleInitialize( NULL ) );
	ASSERT( m_bCOM == TRUE );

	return m_bCOM;
}

int CRazaThread::Run()
{
	if ( m_pfnThreadProcExt )
		return ( *m_pfnThreadProcExt )( m_pThreadParams );
	else
		return CWinThread::Run();
}

void CRazaThread::Add(CRazaThread* pThread, LPCSTR pszName)
{
	CSingleLock oLock( &m_ThreadMapSection, TRUE );

	if ( pszName )
	{
		SetThreadName( pThread->m_nThreadID, pszName );
	}

	CThreadTag tag = { pThread, pszName };
	m_ThreadMap.SetAt( pThread->m_hThread, tag );

	TRACE( _T("Creating '%hs' thread (0x%08x). Count: %d\n"),
		( pszName ? pszName : "unnamed" ), pThread->m_hThread, m_ThreadMap.GetCount() );
}

void CRazaThread::Remove(HANDLE hThread)
{
	CSingleLock oLock( &m_ThreadMapSection, TRUE );

	CThreadTag tag = { 0 };
	if ( m_ThreadMap.Lookup( hThread, tag ) )
	{
		m_ThreadMap.RemoveKey( hThread );

		TRACE( _T("Removing '%hs' thread (0x%08x). Count: %d\n"),
			( tag.pszName ? tag.pszName : "unnamed" ),
			hThread, m_ThreadMap.GetCount() );
	}
}

void CRazaThread::Terminate(HANDLE hThread)
{
	// Its a very dangerous function produces 100% urecoverable TLS leaks/deadlocks
	if ( TerminateThread( hThread, 0 ) )
	{
		CSingleLock oLock( &m_ThreadMapSection, TRUE );

		CThreadTag tag = { 0 };
		if ( m_ThreadMap.Lookup( hThread, tag ) )
		{
			ASSERT( hThread == tag.pThread->m_hThread );
			ASSERT_VALID( tag.pThread );
			ASSERT( static_cast<CWinThread*>( tag.pThread ) != AfxGetApp() );
			tag.pThread->Delete();
		}
		else
			CloseHandle( hThread );

		theApp.Message( MSG_DEBUG, _T("WARNING: Terminating '%hs' thread (0x%08x)."),
			( tag.pszName ? tag.pszName : "unnamed" ), hThread );
		TRACE( _T("WARNING: Terminating '%hs' thread (0x%08x).\n"),
			( tag.pszName ? tag.pszName : "unnamed" ), hThread );
	}
	else
	{
		theApp.Message( MSG_DEBUG, _T("WARNING: Terminating thread (0x%08x) failed."), hThread );
		TRACE( _T("WARNING: Terminating thread (0x%08x) failed.\n"), hThread );
	}
}

void CRazaThread::YieldProc()
{
	if ( theApp.m_nLogicalProcessors > 1 )
		SwitchToThread();
	else
		Sleep( 0 );
}

void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
{
#ifndef NDEBUG
	struct
	{
		DWORD dwType;		// must be 0x1000
		LPCSTR szName;		// pointer to name (in user addr space)
		DWORD dwThreadID;	// thread ID (-1=caller thread)
		DWORD dwFlags;		// reserved for future use, must be zero
	} info =
	{
		0x1000,
		szThreadName,
		dwThreadID,
		0
	};
	__try
	{
		RaiseException( 0x406D1388, 0, sizeof info / sizeof( DWORD ), (ULONG_PTR*)&info );
	}
	__except( EXCEPTION_CONTINUE_EXECUTION )
	{
	}
#endif
	UNUSED_ALWAYS(dwThreadID);
	UNUSED_ALWAYS(szThreadName);
}

HANDLE BeginThread(LPCSTR pszName, AFX_THREADPROC pfnThreadProc,
	LPVOID pParam, int nPriority, UINT nStackSize, DWORD dwCreateFlags,
	LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
	CRazaThread* pThread = new CRazaThread( pfnThreadProc, pParam );
	ASSERT_VALID( pThread );
	if ( pThread )
	{
		return pThread->CreateThread( pszName, nPriority, dwCreateFlags, nStackSize,
			lpSecurityAttrs );
	}
	return NULL;
}

void CloseThread(HANDLE* phThread, DWORD dwTimeout)
{
	if ( *phThread )
	{
		__try
		{
			::SetThreadPriority( *phThread, THREAD_PRIORITY_NORMAL );

			while( *phThread )
			{
				SafeMessageLoop();

				DWORD res = MsgWaitForMultipleObjects( 1, phThread,
					FALSE, dwTimeout, QS_ALLINPUT | QS_ALLPOSTMESSAGE );
				if ( res == WAIT_OBJECT_0 + 1 )
					// Handle messages
					continue;
				else if ( res != WAIT_TIMEOUT )
					// Handle signaled state or errors
					break;
				else
				{
					// Timeout
					CRazaThread::Terminate( *phThread );
					break;
				}
			}
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			// Thread already ended
		}

		CRazaThread::Remove( *phThread );

		*phThread = NULL;
	}
}
