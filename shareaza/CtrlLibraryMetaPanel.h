//
// CtrlLibraryMetaPanel.h
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

#if !defined(AFX_CTRLLIBRARYMETAPANEL_H__A50ED736_E285_4656_903C_8C32D565ABD5__INCLUDED_)
#define AFX_CTRLLIBRARYMETAPANEL_H__A50ED736_E285_4656_903C_8C32D565ABD5__INCLUDED_

#pragma once

#include "CtrlLibraryPanel.h"
#include "MetaPanel.h"

class CSchema;
class CLibraryFile;


class CLibraryMetaPanel : public CLibraryPanel
{
// Construction
public:
	CLibraryMetaPanel();
	virtual ~CLibraryMetaPanel();

	DECLARE_DYNCREATE(CLibraryMetaPanel)

// Attributes
protected:
	int				m_nSelected;
	DWORD			m_nIndex;
	CString			m_sName;
	CString			m_sPath;
	CString			m_sFolder;
	CString			m_sType;
	CString			m_sSize;
	int				m_nIcon32;
	int				m_nIcon48;
	int				m_nRating;
	CSchema*		m_pSchema;
	CMetaPanel		m_pMetadata;
	CRect			m_rcFolder;
	CRect			m_rcRating;
	int				m_nScrollWheelLines;
protected:
	CCriticalSection	m_pSection;
	CEvent				m_pWakeup;
	HANDLE				m_hThread;
	BOOL				m_bThread;
	CSize				m_szThumb;
	CBitmap				m_bmThumb;
	COLORREF			m_crLight;
	int					m_nThumbSize;
	CString				m_sThumb;

// Operations
public:
	virtual BOOL CheckAvailable(CLibraryTreeItem* pFolders, CLibraryList* pObjects);
	virtual void Update();
protected:
	void	DrawText(CDC* pDC, int nX, int nY, LPCTSTR pszText, RECT* pRect = NULL);
	void	DrawThumbnail(CDC* pDC, CRect& rcClient, CRect& rcWork);
	void	DrawThumbnail(CDC* pDC, CRect& rcThumb);
protected:
	static UINT ThreadStart(LPVOID pParam);
	void		OnRun();

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryMetaPanel)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CLibraryMetaPanel)
	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLLIBRARYMETAPANEL_H__A50ED736_E285_4656_903C_8C32D565ABD5__INCLUDED_)
