//
// DlgDecodeMetadata.h
//
//	Date:			"$Date: 2005/03/11 02:20:01 $"
//	Revision:		"$Revision: 1.1 $"
//  Last change by:	"$Author: rolandas $"
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

#ifndef DLGDECODEMETADATA_H_INCLUDED
#define DLGDECODEMETADATA_H_INCLUDED

#include "DlgSkinDialog.h"

class CDecodeMetadataDlg : public CSkinDialog
{
// Construction
public:
	CDecodeMetadataDlg(CWnd* pParent = NULL);

// Dialog Data
public:
	//{{AFX_DATA(CDecodeMetadataDlg)
	enum { IDD = IDD_CODEPAGES };
	CButton	m_wndOK;
	CComboBox	m_wndCodepages;
	//}}AFX_DATA

	CPtrList	m_pFiles;
	void		AddFile(CLibraryFile* pFile);

// Overrides
public:
	//{{AFX_VIRTUAL(CDecodeMetadataDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CDecodeMetadataDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
};

//{{AFX_INSERT_LOCATION}}

#endif // #ifndef DLGDECODEMETADATA_H_INCLUDED
