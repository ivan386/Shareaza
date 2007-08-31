//
// CtrlLibraryTip.h
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

#if !defined(AFX_CTRLLIBRARYTIP_H__DA7457FB_37EE_4940_8619_ED56D5DA593A__INCLUDED_)
#define AFX_CTRLLIBRARYTIP_H__DA7457FB_37EE_4940_8619_ED56D5DA593A__INCLUDED_

#pragma once

#include "CtrlCoolTip.h"
#include "MetaList.h"


class CLibraryTipCtrl : public CCoolTipCtrl
{
// Construction
public:
	CLibraryTipCtrl();
	virtual ~CLibraryTipCtrl();

	DECLARE_DYNAMIC(CLibraryTipCtrl)

// Attributes
protected:
	CString			m_sName;
	CString			m_sPath;
	CString			m_sFolder;
	CString			m_sSize;
	CString			m_sType;
	CString			m_sSHA1;
	CString			m_sTTH;
	CString			m_sED2K;
	CString			m_sBTH;
	CString			m_sMD5;
	int				m_nIndex;
	int				m_nIcon;
	CMetaList		m_pMetadata;
	int				m_nKeyWidth;
	COLORREF		m_crLight;
protected:
	CCriticalSection	m_pSection;
	CEvent				m_pWakeup;
	CSize				m_szThumbSize;
	HANDLE				m_hThread;
	BOOL				m_bThread;
	CSize				m_szThumb;
	CBitmap				m_bmThumb;
	DWORD				m_tHidden;

// Operations
public:
	virtual BOOL OnPrepare();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnPaint(CDC* pDC);
protected:
	void		DrawThumb(CDC* pDC, CRect& rcThumb);
	void		StopThread();
	static UINT	CLibraryTipCtrl::ThreadStart(LPVOID pParam);
	void		OnRun();

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryTipCtrl)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CLibraryTipCtrl)
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLLIBRARYTIP_H__DA7457FB_37EE_4940_8619_ED56D5DA593A__INCLUDED_)
