//
// CtrlLibraryFileView.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

#include "CtrlLibraryView.h"

class CLibraryFile;
class CMetaList;


class CLibraryFileView : public CLibraryView
{
// Construction
public:
	CLibraryFileView();
	virtual ~CLibraryFileView();

	DECLARE_DYNAMIC(CLibraryFileView)

// Attributes
protected:
	BOOL				m_bEditing;
	BOOL				m_bRequestingService;
	INT_PTR				m_nCurrentPage;
	CList<CMetaList*>	m_pServiceDataPages;
	BOOL				m_bServiceFailed;

protected:
	virtual BOOL		CheckAvailable(CLibraryTreeItem* pSel);
	virtual void		SelectAll() = 0;

protected:
	void			CheckDynamicBar();
	void			ClearServicePages();

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
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
	afx_msg void OnUpdateLibraryRefreshMetadata(CCmdUI* pCmdUI);
	afx_msg void OnLibraryRefreshMetadata();
	afx_msg void OnUpdateLibraryShared(CCmdUI* pCmdUI);
	afx_msg void OnLibraryShared();
	afx_msg void OnUpdateLibraryProperties(CCmdUI* pCmdUI);
	afx_msg void OnLibraryProperties();
	afx_msg void OnUpdateMusicBrainzLookup(CCmdUI* pCmdUI);
	afx_msg void OnMusicBrainzLookup();
	afx_msg void OnUpdateMusicBrainzMatches(CCmdUI* pCmdUI);
	afx_msg void OnMusicBrainzMatches();
	afx_msg void OnUpdateMusicBrainzAlbums(CCmdUI* pCmdUI);
	afx_msg void OnMusicBrainzAlbums();
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
	afx_msg void OnUpdateLibraryRebuild(CCmdUI* pCmdUI);
	afx_msg void OnLibraryRebuild();
	afx_msg LRESULT OnServiceDone(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
