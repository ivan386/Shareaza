// Player.h : Declaration of the CPlayer

#pragma once

#include "MediaPlayer.h"

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

DECLARE_GET_CONTROLLING_UNKNOWN()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
		Destroy();
	}

protected:
	HWND m_hWnd;

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
		/* [out] */ DOUBLE *pnAspect);
	STDMETHOD(SetAspect)(
		/* [in] */ DOUBLE nAspect) ;
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
