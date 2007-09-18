//
// DownloadBase.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_DOWNLOADBASE_H__71956CAE_7D13_4809_837B_5F17DE46856E__INCLUDED_)
#define AFX_DOWNLOADBASE_H__71956CAE_7D13_4809_837B_5F17DE46856E__INCLUDED_

#pragma once

class CDownloadTask;


class CDownloadBase
{
// Construction
protected:
	CDownloadBase();
	virtual ~CDownloadBase();

// Attributes
public:
	int			m_nCookie;
public:
	CString		m_sDisplayName;				// The name of the file (Displayed in windows, etc). May have 'unsafe' characters
	CString		m_sSearchKeyword;			// Search keyword to override G1 keyword search.
	CString		m_sSafeName;				// The name, with invalid characters removed. (A meaningful local disk name)
	CString		m_sDiskName;				// The name and path of the incomplete file on disk (the .partial). 
											// The .sd will be the same as above with ".sd" on the end
	QWORD		m_nSize;					// Size of download in Bytes
public:
    Hashes::Sha1ManagedHash m_oSHA1;
    Hashes::TigerManagedHash m_oTiger;
    Hashes::Md5ManagedHash m_oMD5;
    Hashes::Ed2kManagedHash m_oED2K;
    Hashes::BtManagedHash m_oBTH;
protected:
	CDownloadTask*	m_pTask;

// Operations
public:
	BOOL			IsTasking() const { return m_pTask != NULL; }
	BOOL			SetNewTask(CDownloadTask* pTask);
	void			SetModified();
protected:
	virtual BOOL	IsCompleted() const = 0;
	virtual BOOL	IsMoving() const = 0;
	virtual BOOL	IsPaused(BOOL bRealState = FALSE) const = 0;
	virtual BOOL	IsTrying() const = 0;
	void			GenerateDiskName(bool bTorrent = false);
	virtual void	Serialize(CArchive& ar, int nVersion);

};

#endif // !defined(AFX_DOWNLOADBASE_H__71956CAE_7D13_4809_837B_5F17DE46856E__INCLUDED_)
