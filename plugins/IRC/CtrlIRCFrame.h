// CtrlIRCFrame.h : Declaration of the CIRCFrame

#pragma once
#include "IRC.h"
#include "BasePluginWindow.h"
#include "CtrlIRPanel.h"
using namespace ATLControls;

typedef CAtlArray< CString, CStringElementTraits< CString > > CStringArray;
class CIRCPanel;

class CIRCNewMessage
{
public:
	CIRCNewMessage::operator =(CIRCNewMessage &rhs );
public:
	int				nColorID;
	CString			m_sTargetName;
	CStringArray	m_pMessages;
};

class CIRCTabCtrl : public CWindowImpl< CIRCTabCtrl, CContainedWindowT< CTabCtrl, 
	CWinTraitsOR<WS_CHILD | WS_VISIBLE | TCS_FLATBUTTONS | TCS_OWNERDRAWFIXED> > >
{
public:
	void			SetTabColor(int nItem, COLORREF cRGB);
	COLORREF		GetTabColor(int nItem);

	BEGIN_MSG_MAP(CIRCTabCtrl)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
	END_MSG_MAP()

	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};


// CIRCFrame

class CIRCFrame : public CBasePluginWindow< CFrameWinTraits >
{
protected:
	static const int PANEL_WIDTH	    = 200;
	static const int IRCHEADER_HEIGHT   = 70;
	static const int TOOLBAR_HEIGHT	    = 28;
	static const int EDITBOX_HEIGHT		= 20;
	static const int TABBAR_HEIGHT	    = 24;
	static const int SMALLHEADER_HEIGHT = 24;
	static const int SEPERATOR_HEIGHT	= 3;
	static const int STATUSBOX_WIDTH	= 330;

public:
	CIRCFrame();
	virtual ~CIRCFrame();

	void	OnSkinChanged(void);
	virtual BOOL Initialize(CIRCPlugin* pPlugin, HWND hWnd);

	BEGIN_COM_MAP(CIRCFrame)
		COM_INTERFACE_ENTRY(IPluginWindowOwner)
	END_COM_MAP()

	// IPluginWindowOwner Methods
public:
	STDMETHOD(OnTranslate)(MSG* pMessage);
	STDMETHOD(OnMessage)(INT nMessage, WPARAM wParam, LPARAM lParam, LRESULT* plResult);
	STDMETHOD(OnUpdate)(INT nCommandID, STRISTATE* pbVisible, STRISTATE* pbEnabled, STRISTATE* pbChecked);
	STDMETHOD(OnCommand)(INT nCommandID);
	STDMETHOD(GetWndClassName)(BSTR* pszClassName);

public:
	BOOL			m_bConnected;

#define	MAX_CHANNELS	10
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
	// Header
	HICON			m_bHeaderIcon;
	HBITMAP			m_hBuffer;
	//CIRCPanel*		m_pPanel;
	TCHAR*			m_pszLineJoiner;
	CIRCTabCtrl		m_wndTab;

protected:
	CString			m_sFile;
	CString			m_sNickname;
	CStringArray	m_pIrcBuffer[ MAX_CHANNELS ];
	int				m_nBufferCount;
	int				m_nCurrentPosLineBuffer[ MAX_CHANNELS ];
	CStringArray	m_pIrcUsersBuffer[ MAX_CHANNELS ];
	CStringArray	m_pLastLineBuffer[ MAX_CHANNELS ];

protected:
	CString			m_sCurrent;
	POINT			m_ptCursor;
	int				m_nListWidth;
	int				m_nUserListHeight;
	int				m_nChanListHeight;
	SOCKET			m_nSocket;
	int				m_nLocalTextLimit;
	int				m_nLocalLinesLimit;

public:
	static CIRCFrame* g_pIrcFrame;

public:
	BEGIN_MSG_MAP(CIRCFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

