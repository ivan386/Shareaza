//
// CtrlDownloadTip.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

#include "CtrlCoolTip.h"

class CShareazaFile;
class CDownload;
class CDownloadSource;
class CLineGraph;
class CGraphItem;


class CDownloadTipCtrl : public CCoolTipCtrl
{
	DECLARE_DYNAMIC(CDownloadTipCtrl)

public:
	CDownloadTipCtrl();
	virtual ~CDownloadTipCtrl();

	void Show(CDownload* pContext, HWND hAltWnd = NULL)
	{
		bool bChanged = ( pContext != m_pDownload );
		m_pDownload = pContext;
		m_pSource = NULL;
		m_hAltWnd = hAltWnd;
		ShowImpl( bChanged );
	}

	void Show(CDownloadSource* pContext, HWND hAltWnd = NULL)
	{
		bool bChanged = ( pContext != m_pSource );
		m_pDownload = NULL;
		m_pSource = pContext;
		m_hAltWnd = hAltWnd;
		ShowImpl( bChanged );
	}

protected:
	CDownload*			m_pDownload;
	CDownloadSource*	m_pSource;
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
	CLineGraph*		m_pGraph;
	CGraphItem*		m_pItem;
	int				m_nHeaderWidth;
	int				m_nValueWidth;
	int				m_nHeaders;
	int				m_nStatWidth;
	BOOL			m_bDrawGraph;		//Draw the download graph?

	virtual BOOL OnPrepare();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnPaint(CDC* pDC);

	void OnCalcSize(CDC* pDC, CDownload* pDownload);
	void OnCalcSize(CDC* pDC, CDownloadSource* pSource);
	void OnPaint(CDC* pDC, CDownload* pDownload);
	void OnPaint(CDC* pDC, CDownloadSource* pSource);

	void PrepareDownloadInfo(CDownload* pDownload);
	void PrepareFileInfo(CShareazaFile* pDownload);
	void DrawProgressBar(CDC* pDC, CPoint* pPoint, CDownload* pDownload);
	void DrawProgressBar(CDC* pDC, CPoint* pPoint, CDownloadSource* pSource);

	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
