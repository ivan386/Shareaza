//
// CtrlMatchTip.h
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

#if !defined(AFX_CTRLMATCHTIP_H__7464D5F3_15C5_4D17_BCCC_DC8B3728565B__INCLUDED_)
#define AFX_CTRLMATCHTIP_H__7464D5F3_15C5_4D17_BCCC_DC8B3728565B__INCLUDED_

#pragma once

#include "MetaList.h"

class CMatchFile;
class CQueryHit;


class CMatchTipCtrl : public CWnd
{
// Construction
public:
	CMatchTipCtrl();
	virtual ~CMatchTipCtrl();

// Attributes
protected:
	CWnd*			m_pOwner;
	BOOL			m_bVisible;
	CMatchFile*		m_pFile;
	CQueryHit*		m_pHit;
	CPoint			m_pOpen;
	DWORD			m_tOpen;
protected:
	CString			m_sName;
	CString			m_sUser;
	CString			m_sCountryCode;
	CString			m_sCountry;
	CString			m_sSHA1;
	CString			m_sTiger;
	CString			m_sED2K;
	CString			m_sBTH;
	CString			m_sMD5;
	CString			m_sType;
	CString			m_sSize;
	CString			m_sBusy; // Busy status message
	CString			m_sPush; // Firewalled status message
	CString			m_sUnstable; // Unstable status message
	int				m_nIcon;
	CString			m_sStatus;
	COLORREF		m_crStatus;
	CString			m_sPartial;
	CString			m_sQueue;
	CSchemaPtr		m_pSchema;
	CMetaList		m_pMetadata;
	int				m_nKeyWidth;
	int				m_nRating;
protected:
	static LPCTSTR	m_hClass;
	static CBrush	m_brBack;
	static COLORREF	m_crBack;
	static COLORREF	m_crText;
	static COLORREF	m_crBorder;
	static COLORREF	m_crWarnings; // Colour of warning messages

// Operations
public:
	void		Show(CMatchFile* pFile, CQueryHit* pHit);
	void		Hide();
protected:
	void		ShowInternal();
	void		LoadFromFile();
	void		LoadFromHit();
	BOOL		LoadTypeInfo();
	CSize		ComputeSize();
	void		ExpandSize(CDC& dc, CSize& sz, const CString& strText, int nBase = 0);
	void		DrawText(CDC& dc, CPoint& pt, const CString& strText);

// Overrides
public:
	//{{AFX_VIRTUAL(CMatchTipCtrl)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CMatchTipCtrl)
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}

#define IDC_MATCH_TIP	111

#endif // !defined(AFX_CTRLMATCHTIP_H__7464D5F3_15C5_4D17_BCCC_DC8B3728565B__INCLUDED_)
