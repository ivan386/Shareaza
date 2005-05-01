//
// DDEServer.h
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

#if !defined(AFX_DDESERVER_H__3C8F46D7_D308_414A_BCC4_5F68389331C5__INCLUDED_)
#define AFX_DDESERVER_H__3C8F46D7_D308_414A_BCC4_5F68389331C5__INCLUDED_

#pragma once


class CDDEServer
{
// Construction
public:
	CDDEServer(LPCTSTR pszService);
	virtual ~CDDEServer();

// Attributes
protected:
	static CDDEServer* m_pServer;
protected:
	DWORD	m_hInstance;
	HSZ		m_hszService;
	CString	m_sService;

// Operations
public:
	BOOL	Create();
	void	Close();
protected:
	CString StringFromHsz(HSZ hsz);
	static CString	ReadArgument(LPCTSTR& pszMessage);
	static HDDEDATA CALLBACK DDECallback(UINT wType, UINT wFmt, HCONV hConv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD dwData1, DWORD dwData2);
protected:
	virtual BOOL	CheckAccept(LPCTSTR pszTopic);
	virtual BOOL	Execute(LPCTSTR pszTopic, HDDEDATA hData, HDDEDATA* phResult);
	virtual BOOL	Execute(LPCTSTR pszTopic, LPCVOID pData, DWORD nLength);
	virtual BOOL	Execute(LPCTSTR pszTopic, LPCTSTR pszMessage);

};

extern CDDEServer DDEServer;

#endif // !defined(AFX_DDESERVER_H__3C8F46D7_D308_414A_BCC4_5F68389331C5__INCLUDED_)
