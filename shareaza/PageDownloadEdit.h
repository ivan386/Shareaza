//
// PageDownloadEdit.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

class CDownload;


class CDownloadEditPage : public CPropertyPageAdv
{
public:
	CDownloadEditPage();
	virtual ~CDownloadEditPage();

	DECLARE_DYNAMIC(CDownloadEditPage)

	enum { IDD = IDD_DOWNLOAD_EDIT };

protected:
	CString m_sName;
	CString m_sDiskName;
	CString m_sFileSize;
	CString m_sSHA1;
	CString m_sTiger;
	CString m_sED2K;
	CString m_sMD5;
	CString m_sBTH;
	BOOL m_bSHA1Trusted;
	BOOL m_bTigerTrusted;
	BOOL m_bED2KTrusted;
	BOOL m_bMD5Trusted;
	BOOL m_bBTHTrusted;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()
};
