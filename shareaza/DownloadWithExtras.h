//
// DownloadWithExtras.h
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

#if !defined(AFX_DOWNLOADWITHEXTRAS_H__EDD3177D_5313_4C8C_900A_4D5B3DE93BB9__INCLUDED_)
#define AFX_DOWNLOADWITHEXTRAS_H__EDD3177D_5313_4C8C_900A_4D5B3DE93BB9__INCLUDED_

#pragma once

#include "DownloadWithSearch.h"

class CDownloadMonitorDlg;
class CFilePreviewDlg;


class CDownloadWithExtras : public CDownloadWithSearch
{
// Construction
public:
	CDownloadWithExtras();
	virtual ~CDownloadWithExtras();
	
// Attributes
protected:
	CStringList				m_pPreviews;
	CDownloadMonitorDlg*	m_pMonitorWnd;
	CFilePreviewDlg*		m_pPreviewWnd;
	
// Operations
public:
	BOOL		Preview(CSingleLock* pLock = NULL);
	BOOL		IsPreviewVisible() const;
	BOOL		CanPreview();
	void		AddPreviewName(LPCTSTR pszFile);
	void		DeletePreviews();
public:
	void		ShowMonitor(CSingleLock* pLock = NULL);
	BOOL		IsMonitorVisible() const;
public:
	virtual void Serialize(CArchive& ar, int nVersion);
	
	friend class CDownloadMonitorDlg;
	friend class CFilePreviewDlg;
};

#endif // !defined(AFX_DOWNLOADWITHEXTRAS_H__EDD3177D_5313_4C8C_900A_4D5B3DE93BB9__INCLUDED_)
