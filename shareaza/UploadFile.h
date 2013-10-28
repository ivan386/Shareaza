//
// UploadFile.h
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

#if !defined(AFX_UPLOADFILE_H__D2D194F0_8528_482F_A4DA_DE1D9C496335__INCLUDED_)
#define AFX_UPLOADFILE_H__D2D194F0_8528_482F_A4DA_DE1D9C496335__INCLUDED_

#pragma once

#include "FileFragments.hpp"
#include "ShareazaFile.h"

class CUploadTransfer;

class CUploadFile : public CShareazaFile
{
public:
    CUploadFile(CUploadTransfer* pUpload);
	virtual ~CUploadFile();

	void				Add(CUploadTransfer* pUpload);
	BOOL				Remove(CUploadTransfer* pUpload);
	CUploadTransfer*	GetActive() const;
	void				AddFragment(QWORD nOffset, QWORD nLength);
	void				Remove();

	inline BOOL IsEmpty() const
	{
		return m_pTransfers.IsEmpty();
	}

public:
	IN_ADDR						m_pAddress;
	DWORD						m_nRequests;
	Fragments::List				m_oFragments;
	BOOL						m_bSelected;

protected:
	CList< CUploadTransfer* >	m_pTransfers;
};

#endif // !defined(AFX_UPLOADFILE_H__D2D194F0_8528_482F_A4DA_DE1D9C496335__INCLUDED_)
