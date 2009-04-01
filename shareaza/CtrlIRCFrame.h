//
// CtrlIRCFrame.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
// Author: peer_l_@hotmail.com
//

#pragma once

#include "RichViewCtrl.h"
#include "RichDocument.h"
#include "CtrlCoolBar.h"
#include "CtrlIRCPanel.h"

class CIRCNewMessage
{
public:
	BOOL operator =(const CIRCNewMessage &rhs );
public:
	int             nColorID;
	CString         m_sTargetName;
    CStringArray    m_pMessages;
};

class CIRCTabCtrl : public CTabCtrl
{
	enum
	{ 
		paintNone = 0,
		paintBody = 0x1, 
		paintSelected = 0x2, 
		paintHotTrack = 0x4
	};

	// Construction
public:
	CIRCTabCtrl();
	virtual ~CIRCTabCtrl();

protected:
	HANDLE	m_hTheme;
public:
	void			SetTabColor(int nItem, COLORREF cRGB);
	COLORREF		GetTabColor(int nItem);

	void DrawTabControl(CDC* pDC);
	HRESULT DrawThemesPart(HDC dc, int nPartID, int nStateID, LPRECT prcBox);
	void DrawXPTabItem(HDC dc, int nItem, const RECT& rcItem, UINT flags);
	void DrawTabItem(HDC dc, int nItem, const RECT& rcItem, UINT flags);

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
//	virtual BOOL OnEraseBkgnd(CDC* pDC);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};

class CIRCChannelList
{
// Operations
public:
	void			AddChannel(CString strDisplayName, CString strName, BOOL bUserDefined = FALSE );
	void			RemoveChannel(CString strDisplayName);
	void			RemoveAll(int nType = -1);
	int				GetCount(int nType = -1);
	BOOL			GetType(CString strDisplayName);
	int				GetIndexOfDisplay(CString strDisplayName);
	int				GetIndexOfName(CString strName);
	void			Initialize();
	CString			GetDisplayOfIndex(int nIndex) const;
	CString			GetNameOfIndex(int nIndex) const;
// Attributes
protected:
	int				m_nCountUserDefined;
	int				m_nCount;
	CStringArray	m_sChannelName;
	CStringArray	m_sChannelDisplayName;
	CArray<BOOL>	m_bUserDefined;
};

class CIRCFrame : public CWnd
{
protected:
	static const int IRCHEADER_HEIGHT   = 70;
	static const int TOOLBAR_HEIGHT	    = 28;
	static const int EDITBOX_HEIGHT		= 20;
	static const int TABBAR_HEIGHT	    = 24;
	static const int SMALLHEADER_HEIGHT = 24;
	static const int SEPERATOR_HEIGHT	= 3;
	static const int STATUSBOX_WIDTH	= 330;
// Construction
public:
	CIRCFrame();
	virtual ~CIRCFrame();
	DECLARE_DYNAMIC(CIRCFrame)
// Operations
public:
	void	OnSkinChange();
	void	OnUpdateCmdUI();
protected:
	void	PaintListHeader(CDC& dc, CRect& rcBar, CString strText);
public:
	BOOL			m_bConnected;
// Attributes
protected:
	#define			MAX_CHANNELS					10
	NOTIFYICONDATA	m_pTray;
	int             m_nSelectedTab;
	CString			m_sStatus;
	int				m_nMsgsInSec;
	int				m_nTimerVal;
	int             m_nSelectedTabType;
	int				m_nRSelectedTab;
	BOOL			m_bFloodProtectionRunning;
	int				m_nFloodLimit;
	int				m_nFloodingDelay;
	int				m_nUpdateFrequency;
	int				m_nUpdateChanListFreq;
	CString			m_sDestinationIP;
	CString			m_sDestinationPort;
	int				m_nBuiltInChanCount;
protected:
	CString			m_sFile;
	CString			m_sNickname;
	CStringArray	m_pIrcBuffer[ MAX_CHANNELS ];
	int				m_nBufferCount;
	int				m_nCurrentPosLineBuffer[ MAX_CHANNELS ];
	CStringArray	m_pIrcUsersBuffer[ MAX_CHANNELS ];
	CStringArray	m_pLastLineBuffer[ MAX_CHANNELS ];
	CIRCChannelList	m_pChanList;
protected:
	// Header
	CBitmap			m_bmWatermark;
	int				m_nHeaderIcon;
	CDC				m_dcBuffer;
	CBitmap			m_bmBuffer;
	HBITMAP			m_hBuffer;
	CIRCPanel		m_wndPanel;
	TCHAR*			m_pszLineJoiner;
	// Controls
	CEdit			m_wndEdit;
	CRichDocument	m_pContent;
	CRichViewCtrl	m_wndView;
	CRichDocument	m_pContentStatus;
	CRichViewCtrl	m_wndViewStatus;
	CIRCTabCtrl		m_wndTab;
	CCoolBarCtrl	m_wndMainBar;
protected:
	CString			m_sCurrent;
	CPoint			m_ptCursor;
	int				m_nListWidth;
	int				m_nUserListHeight;
	int				m_nChanListHeight;
	SOCKET			m_nSocket;
	int				m_nLocalTextLimit;
	int				m_nLocalLinesLimit;
	CEvent 			m_pWakeup;
	CString         m_sWsaBuffer;
	CMutex			m_pSection;
protected:
	CFont			m_fntEdit;
public:
	static CIRCFrame* g_pIrcFrame;
	void			ConnectIrc();
	void			SetFonts();
	void            SendString(CString strMessage);
	BOOL            OnNewMessage(CString strMessage);
	int				FindParsedItem(CString strMessage, int nFirst = 0);
	int				IsTabExist(CString strTabName);
	void            LoadBufferForWindow(int nTab);
	void			ParseString(CString strMessage, CIRCNewMessage* oNewMessage);
	CString			TrimString(CString strMessage) const;
	CString			GetStringAfterParsedItem(int nItem) const;
	void			OnSettings();
	CString			GetTargetName(CString strRecieverName, int nRecieverType, CString strSenderName, int nSenderType) const;
	int				AddTab(CString TabName, int nKindOfTab);
	void			TabClick();
	void			SortUserList();
	int				CompareUsers(CString strUser1, CString strUser2);
	void			ReloadViewText();
	CString			GetTabText(int nTabIndex = -1);
	int				FindInList(CString strName, int nList=0, int nTab=0);
protected:
	CStringArray	m_pWords;
	int				ParseMessageID();
	void			ActivateMessageByID(CString strMessage, CIRCNewMessage* oNewMessage, int nMessageID);
	CString			GetTextFromRichPoint();
	int				IsUserInList(CString strUser);
	void			UserListDblClick();
	void			ChanListDblClick();
	void			FillChanList();
	void			FillCountChanList(CString strUserCount, CString strChannelName);
	void			PaintHeader(CRect rcHeader, CDC &dc);
	void			DrawText(CDC* pDC, int nX, int nY, LPCTSTR pszText);
	CString			RemoveModeOfNick(CString strNick) const;
	BOOL			ShowTrayPopup(LPCTSTR szText, LPCTSTR szTitle, DWORD dwIcon, UINT uTimeout);
// Operations
	CString         m_sUser;
public:
	virtual void OnLocalText(LPCTSTR pszText);
// Overrides
public:
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnStatusMessage(LPCTSTR pszText, int nFlags);

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnRichCursorMove(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRichClk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRichDblClk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnUpdateIrcConnect(CCmdUI* pCmdUI);
	afx_msg void OnIrcShowSettings();
	afx_msg void OnIrcChanCmdOpen();

	afx_msg void OnIrcUserCmdWhois();
	afx_msg void OnUpdateIrcUserCmd(CCmdUI* pCmdUI);

	afx_msg void OnIrcUserCmdTime();

	afx_msg void OnIrcUserCmdVersion();
	afx_msg void OnIrcUserCmdIgnore();
	afx_msg void OnIrcUserCmdUnignore();

	afx_msg void OnIrcUserCmdOp();
	afx_msg void OnIrcUserCmdDeop();
	afx_msg void OnIrcUserCmdVoice();
	afx_msg void OnIrcUserCmdDevoice();

	afx_msg void OnIrcUserCmdKick();
	afx_msg void OnIrcUserCmdUnban();
	afx_msg void OnIrcUserCmdBan();
	afx_msg void OnIrcUserCmdBanKick();
	afx_msg void OnIrcUserCmdBanKickWhy();
	afx_msg void OnIrcUserCmdKickWhy();

	afx_msg void OnIrcChanCmdSave();

	afx_msg void OnIrcConnect();
	afx_msg void OnUpdateIrcDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnIrcDisconnect();
	afx_msg void OnUpdateIrcCloseTab(CCmdUI* pCmdUI);
	afx_msg void OnIrcCloseTab();

};

//{{AFX_INSERT_LOCATION}}

#define	ID_MESSAGE_SERVER_DISCONNECT    209
#define ID_MESSAGE_SERVER_PING          210
#define ID_MESSAGE_SERVER_NOTICE        211
#define	ID_MESSAGE_SERVER_ERROR			212
#define	ID_MESSAGE_SERVER_CONNECTED		213
#define	ID_MESSAGE_SERVER_MSG			214


#define ID_MESSAGE_CLIENT_JOIN_USERLIST 215
#define ID_MESSAGE_CLIENT_JOIN_ENDNAMES 216
#define ID_MESSAGE_CLIENT_JOIN          217
#define	ID_MESSAGE_CLIENT_NOTICE		218
#define	ID_MESSAGE_CLIENT_INVITE        219
#define	ID_MESSAGE_CLIENT_WHOWAS		220
#define	ID_MESSAGE_CLIENT_WHOIS         221

#define ID_MESSAGE_CHANNEL_TOPICSETBY   222
#define ID_MESSAGE_CHANNEL_TOPICSHOW    223
#define ID_MESSAGE_CHANNEL_PART         224
#define ID_MESSAGE_CHANNEL_QUIT         225
#define ID_MESSAGE_CHANNEL_JOIN         226
#define ID_MESSAGE_CHANNEL_SETMODE      227
#define	ID_MESSAGE_CHANNEL_NOTICE		228
#define	ID_MESSAGE_CHANNEL_MESSAGE		229
#define	ID_MESSAGE_CHANNEL_LIST			230
#define	ID_MESSAGE_CHANNEL_ME           231
#define	ID_MESSAGE_CHANNEL_LISTEND      232

#define	ID_MESSAGE_USER_MESSAGE			233
#define	ID_MESSAGE_USER_CTCPTIME		234
#define	ID_MESSAGE_USER_CTCPVERSION		235
#define	ID_MESSAGE_USER_ME				236
#define	ID_MESSAGE_USER_AWAY			237
#define	ID_MESSAGE_USER_INVITE			238
#define	ID_MESSAGE_USER_KICK			239

#define	ID_MESSAGE_NICK					240
#define	ID_MESSAGE_IGNORE				241
#define	ID_MESSAGE_STOPAWAY				242
#define	ID_MESSAGE_SETAWAY				243

#define	ID_COLOR_NEWMSG					0
#define	ID_COLOR_MSG					1
#define	ID_COLOR_SERVERMSG				2
#define	ID_COLOR_TOPIC					3
#define	ID_COLOR_CHANNELACTION			4
#define	ID_COLOR_NOTICE					5
#define	ID_COLOR_TABS					6
#define	ID_COLOR_CHATBACK				7
#define ID_COLOR_TEXT					8
#define	ID_COLOR_SERVERERROR			9
#define	ID_COLOR_ME						10

#define	ID_KIND_CLIENT					51
#define	ID_KIND_PRIVATEMSG				52
#define	ID_KIND_CHANNEL					53

#define IDC_IRC_FRAME					400

#define IDC_IRC_DBLCLKCHANNELS			200
#define IDC_IRC_DBLCLKUSERS				201
#define	IDC_IRC_MENUUSERS				202
#define IDC_IRC_CHANNELS				122

#define IDC_CHAT_TEXT					100
#define IDC_CHAT_EDIT					101
#define IDC_CHAT_TABS					102
#define IDC_CHAT_TEXTSTATUS				100

#define	WM_REMOVECHANNEL				20933
#define	WM_ADDCHANNEL					20934

