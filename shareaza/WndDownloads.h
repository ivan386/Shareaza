//
// WndDownloads.h
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

#include "WndPanel.h"
#include "CtrlDownloads.h"
#include "CtrlDownloadTabBar.h"


class CDownloadsWnd : public CPanelWnd
{
// Construction
public:
	CDownloadsWnd();
	virtual ~CDownloadsWnd();
	
	DECLARE_SERIAL(CDownloadsWnd)
	
// Operations
public:
	virtual void	OnSkinChange();
	BOOL			Select(CDownload* pDownload);
	void			Update();
	void			DragDownloads(CList< CDownload* >* pList, CImageList* pImage, const CPoint& ptScreen);
protected:
	void			Prepare();
	void			CancelDrag();
	
// Attributes
protected:
	CDownloadsCtrl	m_wndDownloads;
	CDownloadTabBar	m_wndTabBar;
	CCoolBarCtrl	m_wndToolBar;
	CList< CDownload* >* m_pDragList;
	CImageList*		m_pDragImage;
	CPoint			m_pDragOffs;
	HCURSOR			m_hCursMove;
	HCURSOR			m_hCursCopy;
	int				m_nMoreSourcesLimiter;
	DWORD			m_tMoreSourcesTimer;
	DWORD			m_tLastUpdate;
	bool			m_bMouseCaptured;
	DWORD			m_nSelectedDownloads;
	DWORD			m_nSelectedSources;
	DWORD			m_tSel;
	BOOL			m_bSelAny;
	BOOL			m_bSelDownload;
	BOOL			m_bSelSource;
	BOOL			m_bSelTrying;
	BOOL			m_bSelPaused;
	BOOL			m_bSelNotPausedOrMoving;
	BOOL			m_bSelNoPreview;
	BOOL			m_bSelNotCompleteAndNoPreview;
	BOOL			m_bSelCompletedAndNoPreview;
	BOOL			m_bSelStartedAndNotMoving;
	BOOL			m_bSelCompleted;
	BOOL			m_bSelNotMoving;
	BOOL			m_bSelBoostable;
	BOOL			m_bSelSHA1orTTHorED2KorName;
	BOOL			m_bSelShareState;
	BOOL			m_bSelTorrent;
	BOOL			m_bSelIdleSource;
	BOOL			m_bSelActiveSource;
	BOOL			m_bSelChat;
	BOOL			m_bSelBrowse;
	BOOL			m_bSelShareConsistent;
	BOOL			m_bSelMoreSourcesOK;
	BOOL			m_bSelSourceAcceptConnections;
	BOOL			m_bSelSourceExtended;
	BOOL			m_bSelHasReviews;
	BOOL			m_bSelRemotePreviewCapable;
	BOOL			m_bConnectOkay;

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual HRESULT	GetGenericView(IGenericView** ppView);
	
// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateDownloadsResume(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsResume();
	afx_msg void OnUpdateDownloadsPause(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsPause();
	afx_msg void OnUpdateDownloadsClear(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsClear();
	afx_msg void OnUpdateDownloadsLaunch(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsLaunch();
	afx_msg void OnUpdateDownloadsViewReviews(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsViewReviews();
	afx_msg void OnUpdateDownloadsRemotePreview(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsRemotePreview();
	afx_msg void OnUpdateDownloadsSources(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsSources();
	afx_msg void OnDownloadsClearCompleted();
	afx_msg void OnDownloadsClearPaused();
	afx_msg void OnUpdateTransfersDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnTransfersDisconnect();
	afx_msg void OnUpdateTransfersForget(CCmdUI* pCmdUI);
	afx_msg void OnTransfersForget();
	afx_msg void OnUpdateTransfersChat(CCmdUI* pCmdUI);
	afx_msg void OnTransfersChat();
	afx_msg void OnUpdateDownloadsUrl(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsUrl();
	afx_msg void OnUpdateDownloadsEnqueue(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsEnqueue();
	afx_msg void OnUpdateDownloadsAutoClear(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsAutoClear();
	afx_msg void OnUpdateTransfersConnect(CCmdUI* pCmdUI);
	afx_msg void OnTransfersConnect();
	afx_msg void OnUpdateDownloadsShowSources(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsShowSources();
	afx_msg void OnUpdateBrowseLaunch(CCmdUI* pCmdUI);
	afx_msg void OnBrowseLaunch();
	afx_msg void OnUpdateDownloadsBoost(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsBoost();
	afx_msg void OnUpdateDownloadsLaunchCopy(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsLaunchCopy();
	afx_msg void OnUpdateDownloadsMonitor(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsMonitor();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnUpdateDownloadsFileDelete(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFileDelete();
	afx_msg void OnUpdateDownloadsRate(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsRate();
	afx_msg void OnUpdateDownloadsMoveUp(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsMoveUp();
	afx_msg void OnUpdateDownloadsMoveDown(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsMoveDown();
	afx_msg void OnDownloadsSettings();
	afx_msg void OnUpdateDownloadsFilterAll(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFilterAll();
	afx_msg void OnUpdateDownloadsFilterActive(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFilterActive();
	afx_msg void OnUpdateDownloadsFilterQueued(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFilterQueued();
	afx_msg void OnUpdateDownloadsFilterSources(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFilterSources();
	afx_msg void OnUpdateDownloadsFilterPaused(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsFilterPaused();
	afx_msg void OnUpdateDownloadsLaunchComplete(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsLaunchComplete();
	afx_msg void OnUpdateDownloadsShare(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsShare();
	afx_msg void OnUpdateDownloadsCopy(CCmdUI* pCmdUI);
	afx_msg void OnDownloadsCopy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnUpdateDownloadGroupShow(CCmdUI* pCmdUI);
	afx_msg void OnDownloadGroupShow();
	afx_msg void OnDownloadsHelp();
	afx_msg void OnDownloadsFilterMenu();
	afx_msg void OnUpdateDownloadsClearIncomplete(CCmdUI *pCmdUI);
	afx_msg void OnDownloadsClearIncomplete();
	afx_msg void OnUpdateDownloadsClearComplete(CCmdUI *pCmdUI);
	afx_msg void OnDownloadsClearComplete();
	afx_msg void OnUpdateDownloadsEdit(CCmdUI *pCmdUI);
	afx_msg void OnDownloadsEdit();	
	afx_msg void OnCaptureChanged(CWnd *pWnd);
};

#define IDC_DOWNLOADS	100
