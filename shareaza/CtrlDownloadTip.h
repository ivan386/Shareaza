//
// CtrlDownloadTip.h
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

#if !defined(AFX_CTRLDOWNLOADTIP_H__8F881E85_89E4_48CE_81DB_F59065A37A2C__INCLUDED_)
#define AFX_CTRLDOWNLOADTIP_H__8F881E85_89E4_48CE_81DB_F59065A37A2C__INCLUDED_

#pragma once

#include "CtrlCoolTip.h"

class CDownload;
class CDownloadSource;
class CLineGraph;
class CGraphItem;


class CDownloadTipCtrl : public CCoolTipCtrl
{
// Construction
public:
	CDownloadTipCtrl();
	virtual ~CDownloadTipCtrl();

	DECLARE_DYNAMIC(CDownloadTipCtrl)

// Attributes
protected:
	CString			m_sName;
	CString			m_sSHA1;
	CString			m_sTiger;
	CString			m_sED2K;
	CString			m_sBTH;
	CString			m_sMD5;
	CString			m_sURL;
	CString			m_sSize;
	CString			m_sType;
	CString			m_sCountryName;
	int				m_nIcon;
protected:
	CLineGraph*		m_pGraph;
	CGraphItem*		m_pItem;
protected:
	CArray< CString >	m_pHeaderName;
	CArray< CString >	m_pHeaderValue;
	int				m_nHeaderWidth;
	int				m_nStatWidth;
	BOOL			m_bDrawGraph;		//Draw the download graph?
	BOOL			m_bDrawError;		//Display the tracker error?

// Operations
protected:
	virtual BOOL OnPrepare();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnPaint(CDC* pDC);
protected:
	void OnCalcSize(CDC* pDC, CDownload* pDownload);
	void OnCalcSize(CDC* pDC, CDownloadSource* pSource);
	void OnPaint(CDC* pDC, CDownload* pDownload);
	void OnPaint(CDC* pDC, CDownloadSource* pSource);
protected:
	void PrepareFileInfo(CDownload* pDownload);
	void DrawProgressBar(CDC* pDC, CPoint* pPoint, CDownload* pDownload);
	void DrawProgressBar(CDC* pDC, CPoint* pPoint, CDownloadSource* pSource);

// Overrides
public:
	//{{AFX_VIRTUAL(CDownloadTipCtrl)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CDownloadTipCtrl)
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLDOWNLOADTIP_H__8F881E85_89E4_48CE_81DB_F59065A37A2C__INCLUDED_)

