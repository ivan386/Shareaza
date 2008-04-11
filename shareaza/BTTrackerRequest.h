//
// BTTrackerRequest.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

#if !defined(AFX_BTTRACKERREQUEST_H__DF1C43AA_E49C_4225_A27B_6E1E6EE95328__INCLUDED_)
#define AFX_BTTRACKERREQUEST_H__DF1C43AA_E49C_4225_A27B_6E1E6EE95328__INCLUDED_

#pragma once

#include "HttpRequest.h"

class CDownload;
class CDownloadBase;
class CBENode;


class CBTTrackerRequest : public CWinThread
{
// Construction
public:
	CBTTrackerRequest(CDownload* pDownload, LPCTSTR pszVerb, DWORD nNumWant, BOOL bProcess);
	virtual ~CBTTrackerRequest();

// Attributes
public:
	CDownload*		m_pDownload;
	BOOL			m_bProcess;
	CHttpRequest	m_pRequest;

// Operations
public:
	static void		SendStarted(CDownload* pDownload, DWORD nNumWant);
	static void		SendUpdate(CDownload* pDownload, DWORD nNumWant);
	static void		SendCompleted(CDownload* pDownload);
	static void		SendStopped(CDownload* pDownload);
    static CString	Escape(const Hashes::BtHash& oBTH);
    static CString	Escape(const Hashes::BtGuid& oGUID);
protected:
	void	Process(BOOL bRequest);
	BOOL	Process(CBENode* pRoot);

// Overrides
public:
	//{{AFX_VIRTUAL(CBTTrackerRequest)
	public:
	virtual int Run();
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CBTTrackerRequest)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_BTTRACKERREQUEST_H__DF1C43AA_E49C_4225_A27B_6E1E6EE95328__INCLUDED_)
