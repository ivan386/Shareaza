//
// DownloadBase.h
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

#if !defined(AFX_DOWNLOADBASE_H__71956CAE_7D13_4809_837B_5F17DE46856E__INCLUDED_)
#define AFX_DOWNLOADBASE_H__71956CAE_7D13_4809_837B_5F17DE46856E__INCLUDED_

#pragma once

#include "Hashes.h"
#include "FileFragment.h"

class CDownloadTask;


class CDownloadBase  
{
// Construction
public:
	CDownloadBase();
	virtual ~CDownloadBase();
	
// Attributes
public:
	int			m_nCookie;
public:
	CString		m_sRemoteName;
	CString		m_sLocalName;
	QWORD		m_nSize;
public:
	CExtendedSHA1	m_oSHA1;
	CExtendedTiger	m_oTiger;
	CExtendedMD5	m_oMD5;
	CExtendedED2K	m_oED2K;
	CExtendedBTH	m_oBTH;
protected:
	CDownloadTask*	m_pTask;
public:
	CFileFragmentList m_oVerified;
	CFileFragmentList m_oInvalid;
		
// Operations
public:
	virtual void	Pause() = 0;
	virtual void	Resume() = 0;
	virtual void	Remove(BOOL bDelete = FALSE) = 0;
	virtual void	Boost() = 0;
	virtual BOOL	IsPaused() const = 0;
	virtual BOOL	IsMoving() const = 0;
	virtual BOOL	IsCompleted() const = 0;
	virtual BOOL	IsTasking() { return m_pTask != NULL; }
public:
	void			SetModified();
protected:
	void			GenerateLocalName();
	virtual void	Serialize(CArchive& ar, int nVersion);
	
};

#endif // !defined(AFX_DOWNLOADBASE_H__71956CAE_7D13_4809_837B_5F17DE46856E__INCLUDED_)
