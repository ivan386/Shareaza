//
// DlgFilePropertiesPage.h
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

#if !defined(AFX_DLGFILEPROPERTIESPAGE_H__A39F7917_D5EC_493E_AE35_7B2543BE9650__INCLUDED_)
#define AFX_DLGFILEPROPERTIESPAGE_H__A39F7917_D5EC_493E_AE35_7B2543BE9650__INCLUDED_

#pragma once
#include "PagePropertyAdv.h"

class CLibraryFile;
class CLibraryList;

class CFilePropertiesPage : public CPropertyPageAdv
{
// Construction
public:
	CFilePropertiesPage(UINT nIDD);
	virtual ~CFilePropertiesPage();

	DECLARE_DYNAMIC(CFilePropertiesPage)

// Helpers
protected:
	CLibraryFile*	GetFile();
	CLibraryList*	GetList() const;
private:
	void	PaintStaticHeader(CDC* pDC, CRect* prc, LPCTSTR psz);

// Dialog Data
public:
	//{{AFX_DATA(CFilePropertiesPage)
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CFilePropertiesPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CFilePropertiesPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGFILEPROPERTIESPAGE_H__A39F7917_D5EC_493E_AE35_7B2543BE9650__INCLUDED_)
