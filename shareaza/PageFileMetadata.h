//
// PageFileMetadata.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#if !defined(AFX_PAGEFILEMETADATA_H__77265CC6_6566_4120_BF95_563EADCC326D__INCLUDED_)
#define AFX_PAGEFILEMETADATA_H__77265CC6_6566_4120_BF95_563EADCC326D__INCLUDED_

#pragma once

#include "DlgFilePropertiesPage.h"
#include "CtrlSchema.h"
#include "CtrlSchemaCombo.h"

class CFileMetadataPage : public CFilePropertiesPage
{
public:
	CFileMetadataPage();
	virtual ~CFileMetadataPage();

	DECLARE_DYNCREATE(CFileMetadataPage)
	enum { IDD = IDD_FILE_METADATA };

	CSchemaCombo	m_wndSchemas;
	CSchemaCtrl		m_wndData;
	CXMLElement*	m_pXML;

protected:
	CXMLElement*	m_pSchemaContainer;

	void AddCrossAttributes(CXMLElement* pXML, LPCTSTR pszTargetURI);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeSchemas();
	afx_msg void OnCloseUpSchemas();

	DECLARE_MESSAGE_MAP()
};

#define IDC_METADATA	100

#endif // !defined(AFX_PAGEFILEMETADATA_H__77265CC6_6566_4120_BF95_563EADCC326D__INCLUDED_)
