//
// PageFileMetadata.h
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

#if !defined(AFX_PAGEFILEMETADATA_H__77265CC6_6566_4120_BF95_563EADCC326D__INCLUDED_)
#define AFX_PAGEFILEMETADATA_H__77265CC6_6566_4120_BF95_563EADCC326D__INCLUDED_

#pragma once

#include "DlgFilePropertiesPage.h"
#include "CtrlSchema.h"
#include "CtrlSchemaCombo.h"


class CFileMetadataPage : public CFilePropertiesPage
{
// Construction
public:
	CFileMetadataPage();
	virtual ~CFileMetadataPage();

	DECLARE_DYNCREATE(CFileMetadataPage)

// Dialog Data
public:
	//{{AFX_DATA(CFileMetadataPage)
	enum { IDD = IDD_FILE_METADATA };
	CSchemaCombo	m_wndSchemas;
	//}}AFX_DATA

	CSchemaCtrl	m_wndData;

// Overrides
public:
	//{{AFX_VIRTUAL(CFileMetadataPage)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CFileMetadataPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeSchemas();
	afx_msg void OnCloseUpSchemas();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#define IDC_METADATA	100

#endif // !defined(AFX_PAGEFILEMETADATA_H__77265CC6_6566_4120_BF95_563EADCC326D__INCLUDED_)
