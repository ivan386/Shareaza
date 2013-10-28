//
// CtrlUploads.h
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

#include "CtrlUploadTip.h"

class CUploadFile;
class CUploadQueue;


class CUploadsCtrl : public CWnd
{
// Construction
public:
	CUploadsCtrl();

	DECLARE_DYNAMIC(CUploadsCtrl)

// Operations
public:
	BOOL		Create(CWnd* pParentWnd, UINT nID);
	BOOL		Update();
protected:
	void		InsertColumn(int nColumn, LPCTSTR pszCaption, int nFormat, int nWidth);
	void		SaveColumnState();
	BOOL		LoadColumnState();
	void		SelectTo(int nIndex);
    void		DeselectAll(CUploadFile* pExcept = NULL);
	BOOL		HitTest(const CPoint& point, CUploadQueue** ppQueue, CUploadFile** ppFile, int* pnIndex, RECT* prcItem);
	BOOL		GetAt(int nSelect, CUploadQueue** ppQueue, CUploadFile** ppFile);
	void		PaintQueue(CDC& dc, const CRect& rcRow, CUploadQueue* pQueue, BOOL bFocus);
	void		PaintFile(CDC& dc, const CRect& rcRow, CUploadQueue* pQueue, CUploadFile* pFile, int nPosition, BOOL bFocus);
	int			GetExpandableColumnX() const;
	void		OnSkinChange();

	friend class CUploadsWnd;

// Attributes
protected:
	CHeaderCtrl			m_wndHeader;
	CUploadTipCtrl		m_wndTip;
	CImageList			m_gdiProtocols;
	int					m_nFocus;
	CUploadFile*		m_pDeselect;
	int					m_nScrollWheelLines;

// Queue/File Abstractation Layer
public:
	static POSITION			GetQueueIterator();
	static CUploadQueue*	GetNextQueue(POSITION& pos);
	static POSITION			GetFileIterator(CUploadQueue* pQueue);
	static CUploadFile*		GetNextFile(CUploadQueue* pQueue, POSITION& pos, int* pnPosition = NULL);

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnChangeHeader(NMHDR* pNotifyStruct, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg UINT OnGetDlgCode();
};

#define ULF_ACTIVE		0x01
#define ULF_QUEUED		0x02
#define ULF_HISTORY		0x04
#define ULF_TORRENT		0x08

#define ULF_ALL			(ULF_ACTIVE|ULF_QUEUED|ULF_HISTORY|ULF_TORRENT)
