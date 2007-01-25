
#pragma once
#include "IRC.h"
#include "BasePluginWindow.h"

// CIRCPanel

static const int PANEL_WIDTH	    = 200;
static const int BOXUSERS_HEIGHT	= 240;
static const int BOXCHANS_MINHEIGHT = 90;
static const int BOX_VOFFSET		= 9;
static const int BOX_HOFFSET		= 9;
static const int BUTTON_HEIGHT	    = 24;
static const int PANELOFFSET_HEIGHT = 130;

class CIRCPanel : public CBasePluginWindow< CControlWinTraits >
{
public:
	CIRCPanel();
	virtual ~CIRCPanel();

private:
	HFONT					m_hFont;

	// Operations
public:
	void	Setup();

	BEGIN_MSG_MAP(CIRCPanel)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	// IPluginWindowOwner Methods
public:
	STDMETHOD(OnTranslate)(MSG* pMessage)
	{
		// Add your function implementation here.
		return E_NOTIMPL;
	}
	STDMETHOD(OnMessage)(INT nMessage, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
	{
		// Add your function implementation here.
		return E_NOTIMPL;
	}
	STDMETHOD(OnUpdate)(INT nCommandID, STRISTATE* pbVisible, STRISTATE* pbEnabled, STRISTATE* pbChecked)
	{
		// Add your function implementation here.
		return E_NOTIMPL;
	}
	STDMETHOD(OnCommand)(INT nCommandID)
	{
		// Add your function implementation here.
		return E_NOTIMPL;
	}
};


