//
// DownloadBase.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#include "ShareazaFile.h"

class CDownloadTask;


class CDownloadBase : public CShareazaFile
{
// Construction
protected:
	CDownloadBase();
	virtual ~CDownloadBase();

// Attributes
public:
	bool		m_bSHA1Trusted;
	bool		m_bTigerTrusted;
	bool		m_bED2KTrusted;
	bool		m_bBTHTrusted;
	bool		m_bMD5Trusted;
	int			m_nCookie;
public:
	CString		m_sSearchKeyword;			// Search keyword to override G1 keyword search.
	CString		m_sSafeName;				// The name, with invalid characters removed. (A meaningful local disk name)
	CString		m_sPath;					// The name and path of the incomplete file on disk (the .partial). 
											// The .sd will be the same as above with ".sd" on the end
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
