//
// BTInfo.h
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

#if !defined(AFX_BTINFO_H__AA44CA36_464F_4FB8_9D79_884D8092ADA0__INCLUDED_)
#define AFX_BTINFO_H__AA44CA36_464F_4FB8_9D79_884D8092ADA0__INCLUDED_

#pragma once

#include "Hashes.h"
#include "SHA.h"
#include "Buffer.h"

class CBuffer;
class CBENode;


class CBTInfo  
{
// Construction
public:
	CBTInfo();
	~CBTInfo();
	
// Subclass
public:
	class CBTFile
	{
	public:
		CBTFile();
		void	Copy(CBTFile* pSource);
		void	Serialize(CArchive& ar, int nVersion);
	public:
		CString	m_sPath;
		QWORD	m_nSize;
		CManagedBTH m_oBTH;
	};
	
// Attributes
public:
	CManagedBTH	m_oInfoBTH;
	CManagedBTH	m_oDataBTH;
public:
	QWORD		m_nTotalSize;
	DWORD		m_nBlockSize;
	DWORD		m_nBlockCount;
	CHashBT*	m_pBlockBTH;
public:
	CString		m_sName;
	CString		m_sTracker;
	int			m_nFiles;
	CBTFile*	m_pFiles;
private:
	CBTH		m_oTestBTH;
	DWORD		m_nTestByte;
	CBuffer		m_pSource;
	
// Operations
public:
	void		Clear();
	void		Copy(CBTInfo* pSource);
	void		Serialize(CArchive& ar);
public:
	BOOL		LoadTorrentFile(LPCTSTR pszFile);
	BOOL		LoadTorrentBuffer(CBuffer* pBuffer);
	BOOL		LoadTorrentTree(CBENode* pRoot);
	BOOL		SaveTorrentFile(LPCTSTR pszPath);
public:
	void		BeginBlockTest();
	void		AddToTest(LPCVOID pInput, DWORD nLength);
	BOOL		FinishBlockTest(DWORD nBlock);
protected:
	BOOL		CheckFiles();

// Inlines
public:
	inline BOOL IsAvailable() const { return m_oInfoBTH.IsValid(); }

};


#endif // !defined(AFX_BTINFO_H__AA44CA36_464F_4FB8_9D79_884D8092ADA0__INCLUDED_)
