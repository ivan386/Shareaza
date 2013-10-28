//
// ImageViewerWindow.h
//
// This software is released into the public domain. You are free to
// redistribute and modify without any restrictions.
// This file is part of SHAREAZA (shareaza.sourceforge.net), original author Michael Stokes. 
//

#pragma once

#include "Image.h"

class CImageViewerPlugin;


class CImageWindow : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CWindow,
	public IPluginWindowOwner
{
public:
	CImageWindow();
	virtual ~CImageWindow();

	CImageViewerPlugin*		m_pPlugin;
	CImageWindow*			m_pNext;
	CString					m_sFile;

	BOOL	Create(CImageViewerPlugin* pPlugin, LPCTSTR pszFile);
	BOOL	Refresh();

protected:
	CComPtr< IApplication >		m_pApplication;
	CComPtr< IPluginWindow >	m_pWindow;

	CImage	m_pImage;
	HBITMAP	m_hBitmap;
	HICON	m_hIcon;
	BOOL	m_bFullSize;
	BOOL	m_bDrag;
	POINTS	m_ptDrag;
	POINTS	m_ptOffset;
	float	m_nZoomFactor;
	int		m_nZoomIndex;

	BOOL	ResizeWindow();
	BOOL	RescaleImage();
	void	OnBestFit();
	void	OnActualSize();

	// View first file
	void	OnFirst();

	// View last file
	void	OnLast();

	// View next file
	void	OnNext();

	// View previous file
	void	OnPrevious();

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

// IPluginWindowOwner
    virtual HRESULT STDMETHODCALLTYPE OnTranslate(MSG __RPC_FAR *pMessage);
	virtual HRESULT STDMETHODCALLTYPE OnMessage(UINT nMessage, WPARAM wParam, LPARAM lParam, LRESULT __RPC_FAR *plResult);
    virtual HRESULT STDMETHODCALLTYPE OnUpdate(UINT nCommandID, TRISTATE __RPC_FAR *pbVisible, TRISTATE __RPC_FAR *pbEnabled, TRISTATE __RPC_FAR *pbChecked);
	virtual HRESULT STDMETHODCALLTYPE OnCommand(UINT nCommandID);
	
	BEGIN_COM_MAP(CImageWindow)
		COM_INTERFACE_ENTRY(IPluginWindowOwner)
	END_COM_MAP()
	
	BEGIN_MSG_MAP(CImageViewerWindow)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
	END_MSG_MAP()
};
