//
// WndMain.h
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

#pragma once

#include "WindowManager.h"
#include "CtrlCoolBar.h"
#include "CtrlCoolMenuBar.h"
#include "CtrlMainTabBar.h"
#include "CtrlWndTabBar.h"
#include "CtrlMonitorBar.h"
#include "WndMonitor.h"
#include "WndHashProgressBar.h"

class CChildWnd;
class CURLActionDlg;


class CMainWnd : public CMDIFrameWnd
{
// Construction
public:
	CMainWnd();
	virtual ~CMainWnd();

	DECLARE_DYNCREATE(CMainWnd)

// Attributes
public:
	CWindowManager		m_pWindows;
	CCoolMenuBarCtrl	m_wndMenuBar;
	CMainTabBarCtrl		m_wndNavBar;
	CCoolBarCtrl		m_wndToolBar;
	CWndTabBar			m_wndTabBar;
	CStatusBar			m_wndStatusBar;
	CMonitorBarCtrl		m_wndMonitorBar;
	CRemoteWnd			m_wndRemoteWnd;
	CHashProgressBar	m_wndHashProgressBar;
public:
	HINSTANCE			m_hInstance;
	BOOL				m_bTrayHide;
	BOOL				m_bTrayIcon;
	NOTIFYICONDATA		m_pTray;
	BOOL				m_bTimer;
	CString				m_sMsgStatus;
	CSkinWindow*		m_pSkin;
	CURLActionDlg*		m_pURLDialog;
	DWORD				m_tURLTime;
	
// Operations
public:
	void		SetGUIMode(int nMode, BOOL bSaveState = TRUE);
	void		CloseToTray();
	void		OpenFromTray(int nShowCmd = SW_SHOW);
	void		UpdateMessages();
	void		LocalSystemChecks();

// Overrides
public:
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	virtual void GetMessageString(UINT nID, CString& rMessage) const;
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnSysColorChange();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg LONG OnWinsock(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnHandleURL(UINT wParam, LONG lParam);
	afx_msg LONG OnHandleCollection(UINT wParam, LONG lParam);
	afx_msg LONG OnVersionCheck(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnOpenChat(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnOpenSearch(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnTray(UINT wParam, LONG lParam);
	afx_msg LONG OnLog(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnSkinChanged(UINT wParam, LONG lParam);
	afx_msg LONG OnSetMessageString(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnMediaKey(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnDevModeChange(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnDisplayChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdatePluginRange(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSystem(CCmdUI* pCmdUI);
	afx_msg void OnViewSystem();
	afx_msg void OnUpdateViewNeighbours(CCmdUI* pCmdUI);
	afx_msg void OnViewNeighbours();
	afx_msg void OnUpdateNetworkConnect(CCmdUI* pCmdUI);
	afx_msg void OnNetworkConnect();
	afx_msg void OnUpdateNetworkDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnNetworkDisconnect();
	afx_msg void OnUpdateViewPackets(CCmdUI* pCmdUI);
	afx_msg void OnViewPackets();
	afx_msg void OnUpdateViewHosts(CCmdUI* pCmdUI);
	afx_msg void OnViewHosts();
	afx_msg void OnNetworkConnectTo();
	afx_msg void OnUpdateViewSearchMonitor(CCmdUI* pCmdUI);
	afx_msg void OnViewSearchMonitor();
	afx_msg void OnNetworkExit();
	afx_msg void OnUpdateNetworkSearch(CCmdUI* pCmdUI);
	afx_msg void OnNetworkSearch();
	afx_msg void OnUpdateViewResultsMonitor(CCmdUI* pCmdUI);
	afx_msg void OnViewResultsMonitor();
	afx_msg void OnUpdateNetworkConnectTo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewDownloads(CCmdUI* pCmdUI);
	afx_msg void OnViewDownloads();
	afx_msg void OnUpdateViewLibrary(CCmdUI* pCmdUI);
	afx_msg void OnViewLibrary();
	afx_msg void OnUpdateViewUploads(CCmdUI* pCmdUI);
	afx_msg void OnViewUploads();
	afx_msg void OnToolsSettings();
	afx_msg void OnHelpAbout();
	afx_msg void OnHelpHomepage();
	afx_msg void OnHelpWeb1();
	afx_msg void OnHelpWeb2();
	afx_msg void OnHelpWeb3();
	afx_msg void OnHelpFaq();
	afx_msg void OnUpdateViewTraffic(CCmdUI* pCmdUI);
	afx_msg void OnViewTraffic();
	afx_msg void OnWindowCascade();
	afx_msg void OnToolsWizard();
	afx_msg void OnTrayOpen();
	afx_msg void OnUpdateNetworkAutoClose(CCmdUI* pCmdUI);
	afx_msg void OnNetworkAutoClose();
	afx_msg void OnHelpWeb4();
	afx_msg void OnHelpWeb5();
	afx_msg void OnHelpWeb6();
	afx_msg void OnUpdateToolsDownload(CCmdUI* pCmdUI);
	afx_msg void OnToolsDownload();
	afx_msg void OnUpdateViewSecurity(CCmdUI* pCmdUI);
	afx_msg void OnViewSecurity();
	afx_msg void OnUpdateWindowCascade(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowTileHorz(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowTileVert(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTabConnect(CCmdUI* pCmdUI);
	afx_msg void OnTabConnect();
	afx_msg void OnUpdateTabNetwork(CCmdUI* pCmdUI);
	afx_msg void OnTabNetwork();
	afx_msg void OnUpdateTabLibrary(CCmdUI* pCmdUI);
	afx_msg void OnTabLibrary();
	afx_msg void OnUpdateTabTransfers(CCmdUI* pCmdUI);
	afx_msg void OnTabTransfers();
	afx_msg void OnUpdateViewTabbed(CCmdUI* pCmdUI);
	afx_msg void OnViewTabbed();
	afx_msg void OnUpdateViewWindowed(CCmdUI* pCmdUI);
	afx_msg void OnViewWindowed();
	afx_msg void OnUpdateViewDiscovery(CCmdUI* pCmdUI);
	afx_msg void OnViewDiscovery();
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnUpdateTabHome(CCmdUI* pCmdUI);
	afx_msg void OnTabHome();
	afx_msg void OnToolsReskin();
	afx_msg void OnUpdateWindowTabBar(CCmdUI* pCmdUI);
	afx_msg void OnWindowTabBar();
	afx_msg void OnUpdateWindowToolBar(CCmdUI* pCmdUI);
	afx_msg void OnWindowToolBar();
	afx_msg void OnUpdateWindowMonitor(CCmdUI* pCmdUI);
	afx_msg void OnWindowMonitor();
	afx_msg void OnNetworkBrowseTo();
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnToolsSkin();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnToolsLanguage();
	afx_msg void OnToolsMercora();
	afx_msg void OnToolsSeedTorrent();
	afx_msg void OnToolsReseedTorrent();
	afx_msg void OnDiskSpace();
	afx_msg void OnDiskWriteFail();
	afx_msg void OnConnectionFail();
	afx_msg void OnNoDonkeyServers();
	afx_msg void OnUpdateViewMedia(CCmdUI* pCmdUI);
	afx_msg void OnViewMedia();
	afx_msg void OnUpdateTabMedia(CCmdUI* pCmdUI);
	afx_msg void OnTabMedia();
	afx_msg void OnUpdateTabSearch(CCmdUI* pCmdUI);
	afx_msg void OnTabSearch();
	afx_msg void OnToolsProfile();
	afx_msg void OnUpdateViewHelp(CCmdUI* pCmdUI);
	afx_msg void OnViewHelp();
	afx_msg void OnUpdateTabHelp(CCmdUI* pCmdUI);
	afx_msg void OnTabHelp();
	afx_msg void OnLibraryFolders();
	afx_msg void OnHelpWarnings();
	afx_msg void OnHelpPromote();
	afx_msg void OnHelpFirewalls();
	afx_msg void OnHelpSecurity();
	afx_msg void OnUpdateNetworkG2(CCmdUI* pCmdUI);
	afx_msg void OnNetworkG2();
	afx_msg void OnUpdateNetworkG1(CCmdUI* pCmdUI);
	afx_msg void OnNetworkG1();
	afx_msg void OnUpdateNetworkED2K(CCmdUI* pCmdUI);
	afx_msg void OnNetworkED2K();
	afx_msg void OnUpdateViewBasic(CCmdUI* pCmdUI);
	afx_msg void OnViewBasic();
	afx_msg void OnHelpForums();
	afx_msg void OnUpdateLibraryHashPriority(CCmdUI* pCmdUI);
	afx_msg void OnLibraryHashPriority();
	afx_msg void OnUpdateWindowNavBar(CCmdUI *pCmdUI);
	afx_msg void OnWindowNavBar();
	afx_msg void OnUpdateWindowRemote(CCmdUI *pCmdUI);
	afx_msg void OnWindowRemote();
	afx_msg void OnRemoteClose();
	afx_msg void OnUpdateMediaCommand(CCmdUI *pCmdUI);
	afx_msg void OnMediaCommand();

// COM
protected:
	BEGIN_INTERFACE_PART(DropTarget, IDropTarget)
		STDMETHOD(DragEnter)(IDataObject FAR* pDataObj, DWORD grfKeyState, POINTL pt, DWORD FAR* pdwEffect);
		STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD FAR* pdwEffect);
		STDMETHOD(DragLeave)();
		STDMETHOD(Drop)(IDataObject FAR* pDataObj, DWORD grfKeyState, POINTL pt, DWORD FAR* pdwEffect);
	protected:
		CString	ObjectToURL(IDataObject* pObject);
		BOOL	ObjectToFiles(IDataObject* pObject);
		BOOL	IsTorrentFile() const;
	protected:
		CStringList	m_pFiles;
	END_INTERFACE_PART(DropTarget)
	
	DECLARE_INTERFACE_MAP()
};

#define IDW_MENU_BAR		0xE810
#define IDW_TOOL_BAR		AFX_IDW_TOOLBAR
#define IDW_NAV_BAR			0xE811
#define IDW_TAB_BAR			0xE812
#define IDW_MONITOR_BAR		0xE813
