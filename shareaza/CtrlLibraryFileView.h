//
// CtrlLibraryFileView.h
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

#if !defined(CTRLLIBRARYFILEVIEW_H)
#define CTRLLIBRARYFILEVIEW_H

#pragma once

#include "CtrlLibraryView.h"

class CLibraryFile;

class CLibraryFileView : public CLibraryView
{
// Construction
public:
	CLibraryFileView();
	virtual ~CLibraryFileView();

	DECLARE_DYNAMIC(CLibraryFileView)

// Attributes
protected:
	POSITION		m_posSel;
	BOOL			m_bEditing;
// Operations
protected:
	virtual BOOL	CheckAvailable(CLibraryTreeItem* pSel);
	virtual DWORD_PTR	HitTestIndex(const CPoint& point) const = 0;
protected:
	void			StartSelectedFileLoop();
	CLibraryFile*	GetNextSelectedFile();
	CLibraryFile*	GetSelectedFile();

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUpdateLibraryLaunch(CCmdUI* pCmdUI);
	afx_msg void OnLibraryLaunch();
	afx_msg void OnUpdateLibraryEnqueue(CCmdUI* pCmdUI);
	afx_msg void OnLibraryEnqueue();
	afx_msg void OnUpdateLibraryURL(CCmdUI* pCmdUI);
	afx_msg void OnLibraryURL();
	afx_msg void OnUpdateLibraryMove(CCmdUI* pCmdUI);
	afx_msg void OnLibraryMove();
	afx_msg void OnUpdateLibraryCopy(CCmdUI* pCmdUI);
	afx_msg void OnLibraryCopy();
	afx_msg void OnUpdateLibraryDelete(CCmdUI* pCmdUI);
	afx_msg void OnLibraryDelete();
	afx_msg void OnUpdateLibraryBitziWeb(CCmdUI* pCmdUI);
	afx_msg void OnLibraryBitziWeb();
	afx_msg void OnUpdateLibraryBitziDownload(CCmdUI* pCmdUI);
	afx_msg void OnLibraryBitziDownload();
	afx_msg void OnUpdateLibraryRefreshMetadata(CCmdUI* pCmdUI);
	afx_msg void OnLibraryRefreshMetadata();
	afx_msg void OnUpdateLibraryShared(CCmdUI* pCmdUI);
	afx_msg void OnLibraryShared();
	afx_msg void OnUpdateLibraryProperties(CCmdUI* pCmdUI);
	afx_msg void OnLibraryProperties();
	afx_msg void OnUpdateShareMonkeyLookup(CCmdUI* pCmdUI);
	afx_msg void OnShareMonkeyLookup();
	afx_msg void OnUpdateLibraryUnlink(CCmdUI* pCmdUI);
	afx_msg void OnLibraryUnlink();
	afx_msg void OnUpdateSearchForThis(CCmdUI* pCmdUI);
	afx_msg void OnSearchForThis();
	afx_msg void OnUpdateSearchForSimilar(CCmdUI* pCmdUI);
	afx_msg void OnSearchForSimilar();
	afx_msg void OnUpdateSearchForArtist(CCmdUI* pCmdUI);
	afx_msg void OnSearchForArtist();
	afx_msg void OnUpdateSearchForAlbum(CCmdUI* pCmdUI);
	afx_msg void OnSearchForAlbum();
	afx_msg void OnUpdateSearchForSeries(CCmdUI* pCmdUI);
	afx_msg void OnSearchForSeries();
	afx_msg void OnUpdateLibraryCreateTorrent(CCmdUI* pCmdUI);
	afx_msg void OnLibraryCreateTorrent();
	afx_msg void OnUpdateLibraryRebuildAnsi(CCmdUI* pCmdUI);
	afx_msg void OnLibraryRebuildAnsi();

	DECLARE_MESSAGE_MAP()

};

#endif // !defined(CTRLLIBRARYFILEVIEW_H)
