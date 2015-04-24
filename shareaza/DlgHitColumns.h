//
// DlgHitColumns.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

#include "DlgSkinDialog.h"
#include "CtrlSchemaCombo.h"
#include "Schema.h"


class CSchemaColumnsDlg : public CSkinDialog
{
public:
	CSchemaColumnsDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_SCHEMA_COLUMNS };

	CListCtrl			m_wndColumns;
	CSchemaCombo		m_wndSchemas;
	CSchemaPtr			m_pSchema;
	CSchemaMemberList	m_pColumns;

	static BOOL		LoadColumns(CSchemaPtr pSchema, CSchemaMemberList* pColumns);
	static BOOL		SaveColumns(CSchemaPtr pSchema, CSchemaMemberList* pColumns);
	static CMenu*	BuildColumnMenu(CSchemaPtr pSchema, CSchemaMemberList* pColumns = NULL);
	static BOOL		ToggleColumnHelper(CSchemaPtr pSchema, CSchemaMemberList* pSource, CSchemaMemberList* pTarget, UINT nToggleID, BOOL bSave = FALSE);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg void OnSelChangeSchemas();

	DECLARE_MESSAGE_MAP()
};
