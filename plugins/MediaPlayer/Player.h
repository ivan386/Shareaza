// Player.h : Declaration of the CPlayer

#pragma once

#include "MediaPlayer_h.h"

// CPlayerWindow

class CPlayerWindow : 
	public CWindowImpl< CPlayerWindow >
{
public:
	CPlayerWindow();

	HBITMAP	m_hLogo;

	BEGIN_MSG_MAP(CPlayerWindow)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND,OnErase)
	END_MSG_MAP()

	LRESULT OnErase(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

// CPlayer

class ATL_NO_VTABLE CPlayer :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPlayer, &CLSID_MediaPlayer>,
	public IMediaPlayer
{
public:
	CPlayer();

DECLARE_REGISTRY_RESOURCEID(IDR_PLAYER)

BEGIN_COM_MAP(CPlayer)
	COM_INTERFACE_ENTRY(IMediaPlayer)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

protected:
	HRESULT AdjustVideoPosAndZoom(void);

	BOOLEAN				m_bAudioOnly;
	HWND				m_hwndOwner;
	CPlayerWindow		m_wndPlayer;
	RECT				m_rcWindow;
	CComPtr< IGraphBuilder >	m_pGraph;
	CComQIPtr< IMediaControl >	m_pControl;
	CComQIPtr< IMediaEvent >	m_pEvent;
	CComQIPtr< IBasicVideo >	m_pVideo;
	CComQIPtr< IVideoWindow >	m_pWindow;
	MediaZoom			m_nZoom;
	DOUBLE				m_dAspect;

// IMediaPlayer
public:
	STDMETHOD(Create)(
		/* [in] */ HWND hWnd);
	STDMETHOD(Destroy)(void);
	STDMETHOD(Reposition)(
		/* [in] */ RECT *prcWnd);
	STDMETHOD(SetLogoBitmap)(
		/* [in] */ HBITMAP hLogo);
	STDMETHOD(GetVolume)(
		/* [out] */ DOUBLE *pnVolume);
	STDMETHOD(SetVolume)(
		/* [in] */ DOUBLE nVolume);
	STDMETHOD(GetZoom)(
		/* [out] */ MediaZoom *pnZoom);
	STDMETHOD(SetZoom)(
		/* [in] */ MediaZoom nZoom);
	STDMETHOD(GetAspect)(
		/* [out] */ DOUBLE *pdAspect);
	STDMETHOD(SetAspect)(
		/* [in] */ DOUBLE dAspect) ;
	STDMETHOD(Open)(
		/* [in] */ BSTR sFilename);
	STDMETHOD(Close)(void);
	STDMETHOD(Play)(void);
	STDMETHOD(Pause)(void);
	STDMETHOD(Stop)(void);
	STDMETHOD(GetState)(
		/* [out] */ MediaState *pnState);
	STDMETHOD(GetLength)(
		/* [out] */ LONGLONG *pnLength);
	STDMETHOD(GetPosition)(
		/* [out] */ LONGLONG *pnPosition);
	STDMETHOD(SetPosition)(
		/* [in] */ LONGLONG nPosition);
	STDMETHOD(GetSpeed)(
		/* [out] */ DOUBLE *pnSpeed);
	STDMETHOD(SetSpeed)(
		/* [in] */ DOUBLE nSpeed);
	STDMETHOD(GetPlugin)(
		/* [out] */ IAudioVisPlugin **ppPlugin);
	STDMETHOD(SetPlugin)(
		/* [in] */ IAudioVisPlugin *pPlugin);
	STDMETHOD(GetPluginSize)(
		/* [out] */ LONG *pnSize);
	STDMETHOD(SetPluginSize)(
		/* [in] */ LONG nSize);
};

OBJECT_ENTRY_AUTO(__uuidof(MediaPlayer), CPlayer)
