//
// CtrlUploadTip.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

class CUpload;
class CUploadFile;
class CUploadTransfer;
class CLineGraph;
class CGraphItem;


class CUploadTipCtrl : public CCoolTipCtrl
{
	DECLARE_DYNAMIC(CUploadTipCtrl)

public:
	CUploadTipCtrl();
	virtual ~CUploadTipCtrl();

	void Show(CUploadFile* pContext, HWND hAltWnd = NULL)
	{
		bool bChanged = ( pContext != m_pUploadFile );
		m_pUploadFile = pContext;
		m_hAltWnd = hAltWnd;
		ShowImpl( bChanged );
	}

protected:
	CUploadFile*	m_pUploadFile;
	CString			m_sAddress;
	CLineGraph*		m_pGraph;
	CGraphItem*		m_pItem;
	int				m_nHeaderWidth;
	int				m_nValueWidth;
	int				m_nHeaders;

	virtual BOOL OnPrepare();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnPaint(CDC* pDC);

	void DrawProgressBar(CDC* pDC, CPoint* pPoint, CUploadFile* pFile);
	void OnCalcSize(CDC* pDC, CUploadTransfer* pUpload);
	void OnPaint(CDC* pDC, CUploadTransfer* pUpload);

	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
