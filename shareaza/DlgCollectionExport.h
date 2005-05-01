//
// DlgCollectionExport.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

#pragma once

#include "DlgSkinDialog.h"

class CAlbumFolder;
class CXMLElement;


class CCollectionExportDlg : public CSkinDialog
{
// Construction
public:
	CCollectionExportDlg(CAlbumFolder* pFolder, CWnd* pParent = NULL);
	virtual ~CCollectionExportDlg();
	DECLARE_DYNAMIC(CCollectionExportDlg)
	enum { IDD = IDD_COLLECTION_EXPORT };

// Attributes
protected:
	CAlbumFolder*	m_pFolder;

protected:
	CString			BrowseForFolder();
	CXMLElement*	CreateXML();
	CXMLElement*	CopyMetadata(CXMLElement* pMetadata);

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
};
