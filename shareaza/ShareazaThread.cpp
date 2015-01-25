//
// ShareazaThread.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "ShareazaThread.h"

inline void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
{
#ifdef _DEBUG
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
		RaiseException( MS_VC_EXCEPTION, 0,
			sizeof( info ) / sizeof( ULONG_PTR ), (ULONG_PTR*)&info );
	}
	__except( EXCEPTION_CONTINUE_EXECUTION )
	{
	}
#endif
	UNUSED(dwThreadID);
	UNUSED(szThreadName);
}


IMPLEMENT_DYNAMIC(CRazaThread, CWinThread)

CCriticalSection		CRazaThread::m_ThreadMapSection;
CRazaThread::CThreadMap	CRazaThread::m_ThreadMap;

CRazaThread::CRazaThread(AFX_THREADPROC pfnThreadProc /*= NULL*/, LPVOID pParam /*= NULL*/)
	: CWinThread		( NULL, pParam )
	, m_pfnThreadProcExt( pfnThreadProc )
	, m_pnOwnerThreadID	( NULL )
{
}

CRazaThread::~CRazaThread()
{
	Remove( m_nThreadID );
}

HANDLE CRazaThread::CreateThread(LPCSTR pszName, int nPriority, DWORD dwCreateFlags, UINT nStackSize, LPSECURITY_ATTRIBUTES lpSecurityAttrs, DWORD* pnThreadID)
{
	if ( CWinThread::CreateThread( dwCreateFlags | CREATE_SUSPENDED, nStackSize, lpSecurityAttrs ) )
	{
		if ( pnThreadID )
		{
			m_pnOwnerThreadID = pnThreadID;
			*pnThreadID = m_nThreadID;
		}

		Add( this, pszName );

		VERIFY( ::SetThreadPriority( m_hThread, nPriority ) );

		if ( ! ( dwCreateFlags & CREATE_SUSPENDED ) )
			VERIFY( ResumeThread() != (DWORD)-1 );

		return m_hThread;
	}

	if  ( pnThreadID )
		*pnThreadID = 0;

	Delete();

	return NULL;
}

BOOL CRazaThread::InitInstance()
{
	CWinThread::InitInstance();

	return TRUE;
}

int CRazaThread::Run()
{
	BOOL bCOM = SUCCEEDED( OleInitialize( NULL ) );

	int ret;
	if ( m_pfnThreadProcExt )
		ret = ( *m_pfnThreadProcExt )( m_pThreadParams );
	else
		ret = CWinThread::Run();

	if ( bCOM )
	{
		__try
		{
			OleUninitialize();
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
		}
	}

	if ( m_pnOwnerThreadID )
	{
		*m_pnOwnerThreadID = 0;
		m_pnOwnerThreadID = NULL;
	}

	return ret;
}

void CRazaThread::Add(CRazaThread* pThread, LPCSTR pszName)
{
	CSingleLock oLock( &m_ThreadMapSection, TRUE );

	ASSERT( pThread->m_nThreadID );
	ASSERT( ! IsThreadAlive( pThread->m_nThreadID ) );

	if ( pszName )
	{
		SetThreadName( pThread->m_nThreadID, pszName );
	}

	CThreadTag tag = { pThread, pszName };
	m_ThreadMap.SetAt( pThread->m_nThreadID, tag );

	TRACE( _T("Creating '%hs' thread (0x%x). Count: %d\n"), ( pszName ? pszName : "unnamed" ), pThread->m_nThreadID, m_ThreadMap.GetCount() );
}

void CRazaThread::Remove(DWORD nThreadID)
{
	if ( ! nThreadID )
		return;

	CSingleLock oLock( &m_ThreadMapSection, TRUE );

	CThreadTag tag;
	if ( m_ThreadMap.Lookup( nThreadID, tag ) )
	{
		m_ThreadMap.RemoveKey( nThreadID );

		TRACE( _T("Removing '%hs' thread (0x%x). Count: %d\n"), ( tag.pszName ? tag.pszName : "unnamed" ), nThreadID, m_ThreadMap.GetCount() );
	}
}

bool CRazaThread::IsThreadAlive(DWORD nThreadID)
{
	if ( ! nThreadID )
		return false;

	CSingleLock oLock( &m_ThreadMapSection, TRUE );

	CThreadTag tag;
	return ( m_ThreadMap.Lookup( nThreadID, tag ) != FALSE );
}

bool CRazaThread::SetThreadPriority(DWORD nThreadID, int nPriority)
{
	if ( ! nThreadID )
		return false;

	CSingleLock oLock( &m_ThreadMapSection, TRUE );

	CThreadTag tag;
	return ( m_ThreadMap.Lookup( nThreadID, tag ) && ( ::SetThreadPriority( tag.pThread->m_hThread, nPriority ) != FALSE ) );
}

HANDLE CRazaThread::GetHandle(DWORD nThreadID)
{
	if ( ! nThreadID )
		return NULL;

	CSingleLock oLock( &m_ThreadMapSection, TRUE );

	CThreadTag tag;
	return ( m_ThreadMap.Lookup( nThreadID, tag ) ? tag.pThread->m_hThread : NULL );
}

void CRazaThread::DeleteThread(DWORD nThreadID)
{
	if ( ! nThreadID )
		return;

	CSingleLock oLock( &m_ThreadMapSection, TRUE );

	CThreadTag tag;
	if ( m_ThreadMap.Lookup( nThreadID, tag ) )
		tag.pThread->Delete();
}

void CRazaThread::DetachThread(DWORD nThreadID)
{
	if ( ! nThreadID )
		return;

	CSingleLock oLock( &m_ThreadMapSection, TRUE );

	CThreadTag tag;
	if ( m_ThreadMap.Lookup( nThreadID, tag ) )
		tag.pThread->m_pnOwnerThreadID = NULL;
}

HANDLE CRazaThread::BeginThread(LPCSTR pszName, AFX_THREADPROC pfnThreadProc, LPVOID pParam, int nPriority, UINT nStackSize, DWORD dwCreateFlags, LPSECURITY_ATTRIBUTES lpSecurityAttrs, DWORD* pnThreadID)
{
	if ( CRazaThread* pThread = new CRazaThread( pfnThreadProc, pParam ) )
	{
		return pThread->CreateThread( pszName, nPriority, dwCreateFlags, nStackSize, lpSecurityAttrs, pnThreadID );
	}
	return NULL;
}

void CRazaThread::CloseThread(DWORD nThreadID, DWORD dwTimeout)
{
	__try
	{
		if ( HANDLE hThread = GetHandle( nThreadID ) )
		{
			DWORD dwExitCode;
			while ( GetExitCodeThread( hThread, &dwExitCode ) && dwExitCode == STILL_ACTIVE )
			{
				if ( ! IsThreadAlive( nThreadID ) )
					return;

				::SetThreadPriority( hThread, THREAD_PRIORITY_NORMAL );

				SafeMessageLoop();

				DWORD res = MsgWaitForMultipleObjects( 1, &hThread, FALSE, dwTimeout, QS_ALLINPUT | QS_ALLPOSTMESSAGE );
				if ( res == WAIT_OBJECT_0 + 1 )
					// Handle messages
					continue;
				else if ( res != WAIT_TIMEOUT )
					// Handle signaled state or errors
					break;
				else
				{
					// Timeout
						
					// It's a very dangerous function produces 100% unrecoverable TLS leaks/deadlocks
					if ( TerminateThread( hThread, 0 ) )
					{
						theApp.Message( MSG_DEBUG, _T("WARNING: Terminating thread (0x%x)."), nThreadID );
						TRACE( _T("WARNING: Terminating thread (0x%x).\n"), nThreadID );

						DeleteThread( nThreadID );
					}
					break;
				}
			}
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		// Thread already ended
	}

	CRazaThread::Remove( nThreadID );
}
