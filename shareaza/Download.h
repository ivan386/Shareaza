//
// Download.h
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

#if !defined(AFX_DOWNLOAD_H__156689EC_D090_4285_BB8C_9AD058024BB5__INCLUDED_)
#define AFX_DOWNLOAD_H__156689EC_D090_4285_BB8C_9AD058024BB5__INCLUDED_

#pragma once

#include "DownloadWithExtras.h"


class CDownload : public CDownloadWithExtras
{
// Construction
public:
	CDownload();
	virtual ~CDownload();

// Attributes
public:
	DWORD		m_nSerID;
	BOOL		m_bExpanded;
	BOOL		m_bSelected;
	TRISTATE	m_bVerify;
	DWORD		m_tCompleted;
	int			m_nRunCookie;
	int			m_nSaveCookie;
	int			m_nGroupCookie;
protected:
	BOOL		m_bPaused;
	BOOL		m_bBoosted;
	BOOL		m_bShared;
	BOOL		m_bComplete;
	DWORD		m_tSaved;
	
// Operations
public:
	virtual void	Pause();
	virtual void	Resume();
	virtual void	Remove(BOOL bDelete = FALSE);
	virtual void	Boost();
	virtual void	Share(BOOL bShared);
	virtual BOOL	Rename(LPCTSTR pszName);
public:
	virtual BOOL	IsStarted() const;
	virtual BOOL	IsPaused() const;
	virtual BOOL	IsDownloading() const;
	virtual BOOL	IsMoving() const;
	virtual BOOL	IsCompleted() const;
	virtual BOOL	IsBoosted() const;
	virtual BOOL	IsShared() const;
public:
	BOOL			Load(LPCTSTR pszPath);
	BOOL			Save(BOOL bFlush = FALSE);
	virtual void	Serialize(CArchive& ar, int nVersion);
public:
	void			OnRun();
	void			OnTaskComplete(CDownloadTask* pTask);
	BOOL			OnVerify(LPCTSTR pszPath, BOOL bVerified);
protected:
	void			OnDownloaded();
	void			OnMoved(CDownloadTask* pTask);
	void			SerializeOld(CArchive& ar, int nVersion);
	
	friend class CDownloadTask;
	friend class CDownloadTransfer;
	friend class CDownloadWithTorrent;
};


#endif // !defined(AFX_DOWNLOAD_H__156689EC_D090_4285_BB8C_9AD058024BB5__INCLUDED_)
