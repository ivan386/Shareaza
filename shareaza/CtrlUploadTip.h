//
// CtrlUploadTip.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#if !defined(AFX_CTRLUPLOADTIP_H__09185D95_72F6_41F4_98B0_D3B42F01D967__INCLUDED_)
#define AFX_CTRLUPLOADTIP_H__09185D95_72F6_41F4_98B0_D3B42F01D967__INCLUDED_

#pragma once

#include "CtrlCoolTip.h"

class CUpload;
class CUploadFile;
class CLineGraph;
class CGraphItem;


class CUploadTipCtrl : public CCoolTipCtrl
{
// Construction
public:
	CUploadTipCtrl();
	virtual ~CUploadTipCtrl();

	DECLARE_DYNAMIC(CUploadTipCtrl)

// Attributes
protected:
	CString			m_sAddress;
	CLineGraph*		m_pGraph;
	CGraphItem*		m_pItem;
protected:
	CStringArray	m_pHeaderName;
	CStringArray	m_pHeaderValue;
	int				m_nHeaderWidth;

// Operations
protected:
	virtual BOOL OnPrepare();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnPaint(CDC* pDC);
protected:
	void DrawProgressBar(CDC* pDC, CPoint* pPoint, CUploadFile* pFile);

// Overrides
public:
	//{{AFX_VIRTUAL(CUploadTipCtrl)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CUploadTipCtrl)
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLUPLOADTIP_H__09185D95_72F6_41F4_98B0_D3B42F01D967__INCLUDED_)
