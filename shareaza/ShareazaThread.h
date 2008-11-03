//
// ShareazaThread.h
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


#ifdef _DEBUG
	#define MS_VC_EXCEPTION	0x406D1388
	#define ALMOST_INFINITE	INFINITE
#else
	#define ALMOST_INFINITE	20000
#endif


class CRazaThread : public CWinThread
{
	DECLARE_DYNAMIC(CRazaThread)

public:
	CRazaThread(AFX_THREADPROC pfnThreadProc = NULL, LPVOID pParam = NULL);
	virtual ~CRazaThread();

	virtual HANDLE CreateThread(LPCSTR pszName, int nPriority = THREAD_PRIORITY_NORMAL,
		DWORD dwCreateFlags = 0, UINT nStackSize = 0,
		LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);
	virtual BOOL InitInstance();
	virtual int Run();

	static void Add(CRazaThread* pThread, LPCSTR pszName);
	static void Remove(HANDLE hThread);
	static void Terminate(HANDLE hThread);
	static void YieldProc();

protected:
	typedef struct
	{
		CRazaThread*	pThread;	// Thread object
		LPCSTR			pszName;	// Thread name
	} CThreadTag;

	typedef CMap< HANDLE, HANDLE, CThreadTag, const CThreadTag& > CThreadMap;

	static CCriticalSection	m_ThreadMapSection;	// Guarding of m_ThreadMap
	static CThreadMap		m_ThreadMap;		// Map of running threads
	BOOL					m_bCOM;				// OLE initialized
	AFX_THREADPROC			m_pfnThreadProcExt;
};

void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName);

HANDLE BeginThread(LPCSTR pszName, AFX_THREADPROC pfnThreadProc,
				   LPVOID pParam, int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
				   DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

void CloseThread(HANDLE* phThread, DWORD dwTimeout = ALMOST_INFINITE);
