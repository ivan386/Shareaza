//
// DlgDownloadMonitor.h
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

#include "DlgSkinDialog.h"

class CDownload;
class CLineGraph;
class CGraphItem;


class CDownloadMonitorDlg : public CSkinDialog
{
public:
	CDownloadMonitorDlg(CDownload* pDownload);
	virtual ~CDownloadMonitorDlg();

	CDownload*		m_pDownload;
	CString			m_sName;
	CLineGraph*		m_pGraph;
	CGraphItem*		m_pItem;
	BOOL			m_bTray;
	NOTIFYICONDATA	m_pTray;
	BOOL			m_bCompleted;

	static CList< CDownloadMonitorDlg* >	m_pWindows;

	static void OnSkinChange(BOOL bSet);
	static void CloseAll();

	void Show();

	enum { IDD = IDD_DOWNLOAD_MONITOR };

protected:
	CStatic	m_wndVolume;
	CStatic m_wndVolumeLabel;
	CButton	m_wndCancel;
	CButton	m_wndClose;
	CButton	m_wndStop;
	CStatic	m_wndProgress;
	CStatic	m_wndTime;
	CStatic	m_wndTimeLabel;
	CStatic	m_wndStatus;
	CStatic	m_wndSpeed;
	CStatic	m_wndSpeedLabel;
	CStatic	m_wndSources;
	CStatic	m_wndSourcesLabel;
	CButton	m_wndLibrary;
	CButton	m_wndLaunch;
	CStatic	m_wndIcon;
	CStatic	m_wndGraph;
	CStatic	m_wndFile;

	BOOL	CreateReal(UINT nID);
	void	Update(CWnd* pWnd, LPCTSTR pszText);
	void	Update(CWnd* pWnd, BOOL bEnabled);
	void	DoPaint(CDC& dc);
	void	DrawProgressBar(CDC* pDC, CRect* pRect);
	void	CloseToTray();
	void	OpenFromTray();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnInitDialog();

	afx_msg void OnPaint();
	afx_msg void OnDownloadCancel();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDownloadLaunch();
	afx_msg void OnDownloadLibrary();
	afx_msg void OnDownloadStop();
	afx_msg void OnClose();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg LRESULT OnTray(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnNeedText(UINT nID, NMHDR* pTTT, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};
