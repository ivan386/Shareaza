//
// DlgFilePropertiesSheet.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

#include "PagePropertyAdv.h"
#include "LibraryList.h"


class CFilePropertiesSheet : public CPropertySheetAdv
{
	DECLARE_DYNAMIC(CFilePropertiesSheet)

public:
	CFilePropertiesSheet(CLibraryListItem oObject = CLibraryListItem());

	CLibraryListPtr	m_pList;

	void	Add(CLibraryListItem oObject);
	void	Add(const CLibraryList* pList);

	virtual INT_PTR DoModal(int nPage = -1);

protected:
	CString			m_sGeneralTitle;
	CString			m_sMetadataTitle;
	CString			m_sCommentsTitle;
	CString			m_sSharingTitle;
	CString			m_sSourcesTitle;

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
