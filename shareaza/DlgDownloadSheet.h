//
// DlgDownloadSheet.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "Download.h"


class CDownloadSheet : public CPropertySheetAdv
{
	DECLARE_DYNAMIC(CDownloadSheet)

public:
	CDownloadSheet(CDownload* pDownload);

	CDownload*		m_pDownload;

	virtual INT_PTR DoModal(int nPage = -1);

protected:
	CString			m_sDownloadTitle;
	CString			m_sActionsTitle;
	CString			m_sGeneralTitle;
	CString			m_sFilesTitle;
	CString			m_sTrackersTitle;

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
