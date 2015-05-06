//
// CtrlMediaFrame.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

#include "CtrlCoolBar.h"
#include "CtrlMediaList.h"

class CLazySliderCtrl : public CSliderCtrl
{
	DECLARE_DYNAMIC(CLazySliderCtrl)

public:
	CLazySliderCtrl() : m_nPos( -1 ), m_nMin( -1 ), m_nMax( -1 ) {}
	virtual ~CLazySliderCtrl() {}

	int GetPos()
	{
		m_nPos = CSliderCtrl::GetPos();
		return m_nPos;
	}

	void SetPos(_In_ int nPos)
	{
		if ( m_nPos != nPos )
		{
			m_nPos = nPos;
			CSliderCtrl::SetPos( nPos );
		}
	}

	int GetRangeMax()
	{
		m_nMax = CSliderCtrl::GetRangeMax();
		return m_nMax;
	}

	int GetRangeMin()
	{
		m_nMin = CSliderCtrl::GetRangeMin();
		return m_nMin;
	}

	void GetRange(_Out_ int& nMin, _Out_ int& nMax)
	{
		GetRange( m_nMin, m_nMax );
		nMin = m_nMin;
		nMax = m_nMax;
	}

	void SetRange(_In_ int nMin, _In_ int nMax, _In_ BOOL bRedraw = FALSE)
	{
		if ( m_nMin != nMin || m_nMax != nMax )
		{
			m_nMin = nMin;
			m_nMax = nMax;
			CSliderCtrl::SetRange( nMin, nMax, bRedraw );
		}
	}
	
	void SetRangeMax(_In_ int nMax, _In_ BOOL bRedraw = FALSE)
	{
		if ( m_nMax != nMax )
		{
			m_nMax = nMax;
			CSliderCtrl::SetRangeMax( nMax, bRedraw );
		}
	}
	
	void SetRangeMin(_In_ int nMin, _In_ BOOL bRedraw = FALSE)
	{
		if ( m_nMin != nMin )
		{
			m_nMin = nMin;
			CSliderCtrl::SetRangeMin( nMin, bRedraw );
		}
	}

private:
	int m_nPos, m_nMin, m_nMax;
};

class CMediaFrame : public CWnd
{
	DECLARE_DYNAMIC(CMediaFrame)

public:
	CMediaFrame();
	virtual ~CMediaFrame();

	static CMediaFrame* GetMediaFrame();

	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void	OnSkinChange();
	void	OnUpdateCmdUI();
	BOOL	PlayFile(LPCTSTR pszFile, BOOL bForcePlay);
	BOOL	EnqueueFile(LPCTSTR pszFile);
	void	PlayCurrent();
	void	OnFileDelete(LPCTSTR pszFile);
	float	GetPosition() const;
	float	GetVolume() const;
	BOOL	SeekTo(double dPosition);
	BOOL	SetVolume(double dVolume);
	void	OffsetVolume(int nVolumeOffset);
	void	OffsetPosition(int nPositionOffset);
	BOOL	PaintStatusMicro(CDC& dc, CRect& rcBar);
	void	UpdateScreenSaverStatus(BOOL bWindowActive);

	CString	GetNowPlaying() const
	{
		return m_sNowPlaying;
	}

	inline IMediaPlayer* GetPlayer() const
	{
		return m_pPlayer;
	}

	inline MediaState GetState() const
	{
		return m_pPlayer ? m_nState : smsNull;
	}

	inline BOOL IsPlaying() const
	{
		return m_pPlayer && m_nState == smsPlaying;
	}

protected:
	static CMediaFrame*			m_wndMediaFrame;
	CComQIPtr< IMediaPlayer >	m_pPlayer;
	MediaState		m_nState;
	LONGLONG		m_nLength;
	LONGLONG		m_nPosition;
	double			m_nSpeed;
	BOOL			m_bMute;
	BOOL			m_bThumbPlay;
	DWORD			m_tLastPlay;

	CString			m_sFile;
	CMetaList		m_pMetadata;
	DWORD			m_tMetadata;

	CMediaListCtrl	m_wndList;
	CCoolBarCtrl	m_wndListBar;
	CCoolBarCtrl	m_wndToolBar;
	CLazySliderCtrl	m_wndPosition;
	CLazySliderCtrl	m_wndSpeed;
	CLazySliderCtrl	m_wndVolume;

	BOOL			m_bFullScreen;
	DWORD			m_tBarTime;
	CPoint			m_ptCursor;
	BOOL			m_bListVisible;
	BOOL			m_bListWasVisible;
	int				m_nListSize;
	BOOL			m_bStatusVisible;

	CRect			m_rcVideo;
	CRect			m_rcStatus;
	VARIANT_BOOL	m_bNoLogo;		// VARIANT_TRUE - don't paint logo and window background
	CImageList		m_pIcons;
	CFont			m_pFontDefault;
	CFont			m_pFontKey;
	CFont			m_pFontValue;

	BOOL			m_bScreenSaverEnabled;
	ULONG			m_nVidAC, m_nVidDC;
	UINT			m_nPowerSchemeId, m_nScreenSaverTime;
	GLOBAL_POWER_POLICY m_CurrentGP;	// Current Global Power Policy
	POWER_POLICY	m_CurrentPP;		// Current Power Policy
	CString			m_sNowPlaying;

	void	SetFullScreen(BOOL bFullScreen);
	void	PaintListHeader(CDC& dc, CRect& rcBar);
	void	PaintStatus(CDC& dc, CRect& rcBar);
	BOOL	DoSizeList();
	BOOL	Prepare();
	BOOL	PrepareVis();
	BOOL	OpenFile(LPCTSTR pszFile);
	void	Cleanup(BOOL bUnexpected = FALSE);
	void	ZoomTo(MediaZoom nZoom);
	void	AspectTo(double nAspect);
	BOOL	UpdateState();
	void	DisableScreenSaver();
	void	EnableScreenSaver();
	HRESULT PluginPlay(BSTR bsFileName);
	void	UpdateNowPlaying(BOOL bEmpty = FALSE);
	void	ReportError();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnUpdateMediaClose(CCmdUI* pCmdUI);
	afx_msg void OnMediaClose();
	afx_msg void OnUpdateMediaPlay(CCmdUI* pCmdUI);
	afx_msg void OnMediaPlay();
	afx_msg void OnUpdateMediaPause(CCmdUI* pCmdUI);
	afx_msg void OnMediaPause();
	afx_msg void OnUpdateMediaStop(CCmdUI* pCmdUI);
	afx_msg void OnMediaStop();
	afx_msg void OnMediaZoom();
	afx_msg void OnUpdateMediaSizeFill(CCmdUI* pCmdUI);
	afx_msg void OnMediaSizeFill();
	afx_msg void OnUpdateMediaSizeDistort(CCmdUI* pCmdUI);
	afx_msg void OnMediaSizeDistort();
	afx_msg void OnUpdateMediaSizeOne(CCmdUI* pCmdUI);
	afx_msg void OnMediaSizeOne();
	afx_msg void OnUpdateMediaSizeTwo(CCmdUI* pCmdUI);
	afx_msg void OnMediaSizeTwo();
	afx_msg void OnUpdateMediaSizeThree(CCmdUI* pCmdUI);
	afx_msg void OnMediaSizeThree();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnUpdateMediaAspectDefault(CCmdUI* pCmdUI);
	afx_msg void OnMediaAspectDefault();
	afx_msg void OnUpdateMediaAspect43(CCmdUI* pCmdUI);
	afx_msg void OnMediaAspect43();
	afx_msg void OnUpdateMediaAspect169(CCmdUI* pCmdUI);
	afx_msg void OnMediaAspect169();
	afx_msg void OnUpdateMediaFullScreen(CCmdUI* pCmdUI);
	afx_msg void OnMediaFullScreen();
	afx_msg void OnUpdateMediaPlaylist(CCmdUI* pCmdUI);
	afx_msg void OnMediaPlaylist();
	afx_msg void OnMediaSettings();
	afx_msg void OnUpdateMediaSettings(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMediaVis(CCmdUI* pCmdUI);
	afx_msg void OnMediaVis();
	afx_msg void OnUpdateMediaStatus(CCmdUI* pCmdUI);
	afx_msg void OnMediaStatus();
	afx_msg void OnUpdateMediaMute(CCmdUI* pCmdUI);
	afx_msg void OnMediaMute();
	afx_msg LRESULT OnMediaKey(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

#define IDC_MEDIA_PLAYLIST	120

#define VK_MEDIA_NEXT_TRACK    0xB0
#define VK_MEDIA_PREV_TRACK    0xB1
#define VK_MEDIA_STOP          0xB2
#define VK_MEDIA_PLAY_PAUSE    0xB3
