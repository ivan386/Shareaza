//
// UploadFiles.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_UPLOADFILES_H__0F70ABF7_91B7_436B_A235_C3D01DB0D7F6__INCLUDED_)
#define AFX_UPLOADFILES_H__0F70ABF7_91B7_436B_A235_C3D01DB0D7F6__INCLUDED_

#pragma once

class CUploadFile;
class CUploadTransfer;


class CUploadFiles  
{
// Construction
public:
	CUploadFiles();
	virtual ~CUploadFiles();
	
// Attributes
protected:
	CList< CUploadFile* >	m_pList;

// Operations
public:
	void			Clear();
	CUploadFile*	GetFile(CUploadTransfer* pUpload);
	void			Remove(CUploadTransfer* pTransfer);
	void			MoveToHead(CUploadTransfer* pTransfer);
	void			MoveToTail(CUploadTransfer* pTransfer);
	
// List Access
public:
	inline POSITION GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}
	
	inline CUploadFile* GetNext(POSITION& pos) const
	{
		return m_pList.GetNext( pos );
	}
	
	inline INT_PTR GetCount() const
	{
		return m_pList.GetCount();
	}

	inline BOOL Check(CUploadFile* pFile) const
	{
		return m_pList.Find( pFile ) != NULL;
	}

};

extern CUploadFiles UploadFiles;

#endif // !defined(AFX_UPLOADFILES_H__0F70ABF7_91B7_436B_A235_C3D01DB0D7F6__INCLUDED_)
