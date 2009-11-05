//
// CtrlLibraryTip.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

#include "ShareazaFile.h"
#include "ThreadImpl.h"
#include "CtrlCoolTip.h"
#include "MetaList.h"


class CLibraryTipCtrl : public CCoolTipCtrl, public CThreadImpl
{
	DECLARE_DYNAMIC(CLibraryTipCtrl)

public:
	CLibraryTipCtrl();
	virtual ~CLibraryTipCtrl();

	void Show(DWORD pContext, HWND hAltWnd = NULL)
	{
		bool bChanged = ( pContext != m_nFileIndex );
		m_nFileIndex = pContext;
		m_pFile = NULL;
		m_hAltWnd = hAltWnd;
		ShowImpl( bChanged );
	}

	void Show(CShareazaFile* pContext, HWND hAltWnd = NULL)
	{
		bool bChanged = ( pContext != m_pFile );
		m_nFileIndex = 0;
		m_pFile = pContext;
		m_hAltWnd = hAltWnd;
		ShowImpl( bChanged );
	}

protected:
	DWORD			m_nFileIndex;
	CShareazaFile*	m_pFile;
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
	int				m_nIcon;
	CMetaList		m_pMetadata;
	int				m_nKeyWidth;
	CCriticalSection	m_pSection;
	CBitmap			m_bmThumb;
	DWORD			m_tHidden;

	virtual BOOL OnPrepare();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnPaint(CDC* pDC);

	void		StopThread();
	void		OnRun();

	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
