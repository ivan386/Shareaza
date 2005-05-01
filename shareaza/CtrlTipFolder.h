//
// CtrlTipFolder.h
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

#if !defined(AFX_CTRLTIPFOLDER_H__5036737D_EC31_4EFD_83EA_86AF31CAF35E__INCLUDED_)
#define AFX_CTRLTIPFOLDER_H__5036737D_EC31_4EFD_83EA_86AF31CAF35E__INCLUDED_

#pragma once

#include "CtrlCoolTip.h"


class CFolderTipCtrl : public CCoolTipCtrl
{
// Construction
public:
	CFolderTipCtrl();
	virtual ~CFolderTipCtrl();

	DECLARE_DYNAMIC(CFolderTipCtrl)

// Attributes
protected:
	CString		m_sName;
	CString		m_sPath;
	CString		m_sFiles;
	CString		m_sVolume;
	CString		m_sPercentage;

// Operations
public:
	virtual BOOL OnPrepare();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnPaint(CDC* pDC);

// Overrides
public:
	//{{AFX_VIRTUAL(CFolderTipCtrl)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CFolderTipCtrl)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLTIPFOLDER_H__5036737D_EC31_4EFD_83EA_86AF31CAF35E__INCLUDED_)
