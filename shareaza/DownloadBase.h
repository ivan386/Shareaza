//
// DownloadBase.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

#include "SharedFile.h"
#include "DownloadTask.h"


class CDownloadBase : public CShareazaFile
{
	DECLARE_DYNAMIC(CDownloadBase)

protected:
	CDownloadBase();
	virtual ~CDownloadBase();

public:
	bool			m_bSHA1Trusted;		// True if SHA1 hash is trusted
	bool			m_bTigerTrusted;	// True if TTH hash is trusted
	bool			m_bED2KTrusted;		// True if ED2K hash is trusted
	bool			m_bBTHTrusted;		// True if BTH hash is trusted
	bool			m_bMD5Trusted;		// True if MD5 hash is trusted

	void			SetModified();
	bool			IsModified() const;

	virtual BOOL	SubmitData(QWORD nOffset, LPBYTE pData, QWORD nLength) = 0;

	// Set download new (and safe) name
	virtual bool	Rename(const CString& strName) = 0;

	// Set download new size
	virtual bool	Resize(QWORD nNewSize) = 0;

	// Return currently running task
	virtual dtask	GetTaskType() const = 0;

	// Statistics
	virtual float	GetProgress() const = 0;

	// Check if a task is already running
	virtual bool	IsTasking() const = 0;

	// Check if a task is already running and its a moving task
	virtual bool	IsMoving() const = 0;

	virtual bool	IsCompleted() const = 0;

	virtual bool	IsPaused(bool bRealState = false) const = 0;

	// Is the download currently trying to download?
	virtual bool	IsTrying() const = 0;

	// File was moved to the Library
	virtual void	OnMoved() = 0;

	// File was hashed and verified in the Library
	virtual BOOL	OnVerify(const CLibraryFile* pFile, TRISTATE bVerified) = 0;

protected:
	int				m_nCookie;
	int				m_nSaveCookie;

	virtual void	Serialize(CArchive& ar, int nVersion);
};
