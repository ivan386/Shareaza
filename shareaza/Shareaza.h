//
// Shareaza.h
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

#pragma once

#include "Resource.h"
#include "ComObject.h"
#include "ShareazaOM.h"

class CMainWnd;


class CShareazaApp : public CWinApp
{
// Construction
public:
	CShareazaApp();

// Attributes
public:
	CMutex				m_pMutex;
	CMutex				m_pSection;
	WORD				m_nVersion[4];
	CString				m_sVersion;
	CFont				m_gdiFont;
	CFont				m_gdiFontBold;
	CFont				m_gdiFontLine;
	CWnd*				m_pSafeWnd;
	BOOL				m_bLive;
	BOOL				m_bNT;						//NT based core. (NT, 2000, XP, etc)
	BOOL				m_bLimitedConnections;		//Networking is limited (XP SP2)
	DWORD				m_dwWindowsVersion;			//Windows version
	DWORD				m_dwWindowsVersionMinor;	//Windows minor version
	QWORD				m_nPhysicalMemory;			//Physical RAM installed

	HINSTANCE m_hUser32;
	BOOL (WINAPI *m_pfnSetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD);
	BOOL (WINAPI *m_pfnGetMonitorInfoA)(HMONITOR, LPMONITORINFO);
	HMONITOR (WINAPI *m_pfnMonitorFromRect)(LPCRECT, DWORD);
	HMONITOR (WINAPI *m_pfnMonitorFromWindow)(HWND, DWORD);

protected:
	CCriticalSection	m_csMessage;
	static TCHAR		szMessageBuffer[16384];
	
// Operations
public:
	static CMainWnd* SafeMainWnd();
	void		Message(int nType, UINT nID, ...);
	void		Message(int nType, LPCTSTR pszFormat, ...);
	CString		GetErrorString();
	BOOL		InternalURI(LPCTSTR pszURI);
protected:
	void		GetVersionNumber();
	void		InitResources();
public:
	void		PrintMessage(int nType, LPCTSTR pszLog);
	void		LogMessage(LPCTSTR pszLog);
	void		DebugState(BOOL bState);

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);

// Implementation
public:
	DECLARE_MESSAGE_MAP()
};

extern CShareazaApp theApp;

//
// Utility Functions
//

CRuntimeClass* AfxClassForName(LPCTSTR pszClass);

BOOL LoadString(CString& str, UINT nID);
LPCTSTR _tcsistr(LPCTSTR pszString, LPCTSTR pszPattern);
LPCTSTR _tcsnistr(LPCTSTR pszString, LPCTSTR pszPattern, DWORD plen);
void Replace(CString& strBuffer, LPCTSTR pszFind, LPCTSTR pszReplace);
void Split(CString strSource, LPCTSTR strDelimiter, CStringArray& pAddIt, BOOL bAddFirstEmpty);
BOOL LoadSourcesString(CString& str, DWORD num);

DWORD	TimeFromString(LPCTSTR psz);
CString	TimeToString(DWORD tVal);
BOOL	TimeFromString(LPCTSTR psz, FILETIME* pTime);
CString	TimeToString(FILETIME* pTime);

#ifdef _DEBUG
#define MLOG(x) theApp.Message( MSG_DEBUG, x )
#else
#define MLOG(x)
#endif

static inline bool IsCharacter(TCHAR nChar)
{
	if ( nChar >= 0 && nChar <= 255 )
	{
		return ( _istalnum( nChar ) ) != 0;
	}
	else
	{
		return ( _istspace( nChar ) ) == 0;
	}
}

static inline bool IsWord(LPCTSTR pszString, int nStart, int nLength)
{
	for ( pszString += nStart ; *pszString && nLength ; pszString++, nLength-- )
	{
		if ( ! _istdigit( *pszString ) ) return true;
	}
	return false;
}

#define MSG_DEFAULT		0
#define MSG_SYSTEM		1
#define MSG_ERROR		2
#define MSG_DEBUG		3
#define MSG_TEMP		4
#define MSG_DOWNLOAD	1

#define WM_WINSOCK		(WM_USER+101)
#define WM_VERSIONCHECK	(WM_USER+102)
#define WM_OPENCHAT		(WM_USER+103)
#define WM_TRAY			(WM_USER+104)
#define WM_URL			(WM_USER+105)
#define WM_SKINCHANGED	(WM_USER+106)
#define WM_COLLECTION	(WM_USER+107)
#define WM_OPENSEARCH	(WM_USER+108)
#define WM_LOG			(WM_USER+109)

#define WM_AFX_SETMESSAGESTRING 0x0362
#define WM_AFX_POPMESSAGESTRING 0x0375
#define WM_IDLEUPDATECMDUI		0x0363

#define ID_PLUGIN_FIRST	27000
#define ID_PLUGIN_LAST	27999

#undef ON_NOTIFY
#define ON_NOTIFY(wNotifyCode, id, memberFxn) \
	{ WM_NOTIFY, (WORD)(int)wNotifyCode, (WORD)id, (WORD)id, AfxSigNotify_v, \
		(AFX_PMSG) \
		(reinterpret_cast< void (AFX_MSG_CALL CCmdTarget::*)(NMHDR*, LRESULT*) > \
		(memberFxn)) },

#undef ON_NOTIFY_RANGE
#define ON_NOTIFY_RANGE(wNotifyCode, id, idLast, memberFxn) \
	{ WM_NOTIFY, (WORD)(int)wNotifyCode, (WORD)id, (WORD)idLast, AfxSigNotify_RANGE, \
		(AFX_PMSG) \
		(reinterpret_cast< void (AFX_MSG_CALL CCmdTarget::*)(UINT, NMHDR*, LRESULT*) > \
		(memberFxn)) },

#undef ON_NOTIFY_EX
#define ON_NOTIFY_EX(wNotifyCode, id, memberFxn) \
	{ WM_NOTIFY, (WORD)(int)wNotifyCode, (WORD)id, (WORD)id, AfxSigNotify_EX, \
		(AFX_PMSG) \
		(reinterpret_cast< BOOL (AFX_MSG_CALL CCmdTarget::*)(UINT, NMHDR*, LRESULT*) > \
		(memberFxn)) },

#undef ON_NOTIFY_EX_RANGE
#define ON_NOTIFY_EX_RANGE(wNotifyCode, id, idLast, memberFxn) \
	{ WM_NOTIFY, (WORD)(int)wNotifyCode, (WORD)id, (WORD)idLast, AfxSigNotify_EX, \
		(AFX_PMSG) \
		(reinterpret_cast< BOOL (AFX_MSG_CALL CCmdTarget::*)(UINT, NMHDR*, LRESULT*) > \
		(memberFxn)) },

#undef ON_NOTIFY_REFLECT
#define ON_NOTIFY_REFLECT(wNotifyCode, memberFxn) \
	{ WM_NOTIFY+WM_REFLECT_BASE, (WORD)(int)wNotifyCode, 0, 0, AfxSigNotify_v, \
		(AFX_PMSG) \
		(reinterpret_cast<void (AFX_MSG_CALL CCmdTarget::*)(NMHDR*, LRESULT*) > \
		(memberFxn)) },

#undef ON_NOTIFY_REFLECT_EX
#define ON_NOTIFY_REFLECT_EX(wNotifyCode, memberFxn) \
	{ WM_NOTIFY+WM_REFLECT_BASE, (WORD)(int)wNotifyCode, 0, 0, AfxSigNotify_b, \
		(AFX_PMSG) \
		(reinterpret_cast<BOOL (AFX_MSG_CALL CCmdTarget::*)(NMHDR*, LRESULT*) > \
		(memberFxn)) },
