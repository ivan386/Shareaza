//
// ED2K.h
//
// Copyright (c) Shareaza Pty. Ltd., 2003.
// This file is part of TorrentAid Torrent Wizard (www.torrentaid.com).
//
// TorrentAid Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// TorrentAid is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TorrentAid; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#if !defined(AFX_ED2K_H__0ED688AE_E4F5_49C6_8EC8_5C80EFA6EF6C__INCLUDED_)
#define AFX_ED2K_H__0ED688AE_E4F5_49C6_8EC8_5C80EFA6EF6C__INCLUDED_

#pragma once

#include "MD4.h"


class CED2K  
{
// Construction
public:
	CED2K();
	virtual ~CED2K();
	
// Attributes
public:
	MD4		m_pHash;
protected:
	CMD4	m_pComposite;
	CMD4	m_pBlock;
	DWORD	m_nBlockCount;
	DWORD	m_nBlockBytes;
	MD4		m_pLastBlock;
	
// Operations
public:
	void	Reset();
	void	Add(LPCVOID pData, DWORD nLength);
	void	Finish();
	void	GetHash(MD4* pHash);
	
};

#define ED2K_BLOCK_SIZE		0x947000

#endif // !defined(AFX_ED2K_H__0ED688AE_E4F5_49C6_8EC8_5C80EFA6EF6C__INCLUDED_)
