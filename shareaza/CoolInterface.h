//
// CoolInterface.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_COOLINTERFACE_H__B7C102E2_0267_4F3B_B9C2_BA1031758891__INCLUDED_)
#define AFX_COOLINTERFACE_H__B7C102E2_0267_4F3B_B9C2_BA1031758891__INCLUDED_

#pragma once

#include "Skin.h"


class CCoolInterface
{
// Construction
public:
	CCoolInterface();
	virtual ~CCoolInterface();

public:
	CFont		m_fntNormal;
	CFont		m_fntBold;
	CFont		m_fntUnder;
	CFont		m_fntCaption;
	CFont		m_fntItalic;
	CFont		m_fntBoldItalic;
	BOOL		m_bCustom;

	COLORREF	m_crWindow;
	COLORREF	m_crMidtone;
	COLORREF	m_crHighlight;
	COLORREF	m_crText;
	COLORREF	m_crHiText;
	COLORREF	m_crBackNormal;
	COLORREF	m_crBackSel;
	COLORREF	m_crBackCheck;
	COLORREF	m_crBackCheckSel;
	COLORREF	m_crMargin;
	COLORREF	m_crBorder;
	COLORREF	m_crShadow;
	COLORREF	m_crCmdText;
	COLORREF	m_crCmdTextSel;
	COLORREF	m_crDisabled;
	COLORREF	m_crRanges;

	COLORREF	m_crCompleted;
	COLORREF	m_crVerifyPass;
	COLORREF	m_crVerifyFail;

	COLORREF	m_crTipBack;
	COLORREF	m_crTipText;
	COLORREF	m_crTipBorder;
	COLORREF	m_crTipWarnings; // Colour of warning messages

	COLORREF	m_crTaskPanelBack;

	COLORREF	m_crTaskBoxCaptionBack;
	COLORREF	m_crTaskBoxPrimaryBack;
	COLORREF	m_crTaskBoxCaptionText;
	COLORREF	m_crTaskBoxPrimaryText;
	COLORREF	m_crTaskBoxCaptionHover;
	COLORREF	m_crTaskBoxClient;

	COLORREF	m_crMediaWindow;
	COLORREF	m_crMediaWindowText;
	COLORREF	m_crMediaStatus;
	COLORREF	m_crMediaStatusText;
	COLORREF	m_crMediaPanel;
	COLORREF	m_crMediaPanelText;
	COLORREF	m_crMediaPanelActive;
	COLORREF	m_crMediaPanelActiveText;
	COLORREF	m_crMediaPanelCaption;
	COLORREF	m_crMediaPanelCaptionText;

	COLORREF	m_crTrafficWindowBack;
	COLORREF	m_crTrafficWindowText;
	COLORREF	m_crTrafficWindowGrid;

	COLORREF	m_crMonitorHistoryBack;
	COLORREF	m_crMonitorHistoryBackMax;
	COLORREF	m_crMonitorHistoryText;
	COLORREF	m_crMonitorDownloadLine;
	COLORREF	m_crMonitorUploadLine;
	COLORREF	m_crMonitorDownloadBar;
	COLORREF	m_crMonitorUploadBar;

	void		Clear();
	void		NameCommand(UINT nID, LPCTSTR pszName);
	UINT		NameToID(LPCTSTR pszName);
	int			ImageForID(UINT nID, int nImageListType = LVSIL_SMALL);
	void		AddIcon(UINT nID, HICON hIcon, int nImageListType = LVSIL_SMALL);
	void		CopyIcon(UINT nFromID, UINT nToID, int nImageListType = LVSIL_SMALL);
	HICON		ExtractIcon(UINT nID, BOOL bMirrored, int nImageListType = LVSIL_SMALL);
	// Set skinned icon to window i.e. pWnd->SetIcon( hIcon, bBigIcon )
	void		SetIcon(UINT nID, BOOL bMirrored, BOOL bBigIcon, CWnd* pWnd);
	// Set skinned icon to window i.e. pWnd->SetIcon( hIcon, bBigIcon )
	void		SetIcon(HICON hIcon, BOOL bMirrored, BOOL bBigIcon, CWnd* pWnd);
	//	BOOL	AddImagesFromToolbar(UINT nIDToolBar, COLORREF crBack = RGB(0,255,0));
	int			GetImageCount(int nImageListType = LVSIL_SMALL);
	BOOL		Add(CSkin* pSkin, CXMLElement* pBase, HBITMAP hbmImage, COLORREF crMask, int nImageListType = LVSIL_SMALL);
	CImageList*	SetImageListTo(CListCtrl& pWnd, int nImageListType = LVSIL_SMALL);
	BOOL		Draw(CDC* pDC, int nImage, POINT pt, UINT nStyle, int nImageListType = LVSIL_SMALL);
	BOOL		DrawEx(CDC* pDC, int nImage, POINT pt, SIZE sz, COLORREF clrBk, COLORREF clrFg, UINT nStyle, int nImageListType = LVSIL_SMALL);
	CDC*		GetBuffer(CDC& dcScreen, CSize& szItem);
	BOOL		DrawWatermark(CDC* pDC, CRect* pRect, CBitmap* pMark, int nOffX = 0, int nOffY = 0);
	void		CreateFonts(LPCTSTR pszFace = NULL, int nSize = 0);
	void		CalculateColours(BOOL bCustom = FALSE);
	void		OnSysColourChange();
	static COLORREF	CalculateColour(COLORREF crFore, COLORREF crBack, int nAlpha);
	static COLORREF	GetDialogBkColor();
	static BOOL		IsNewWindows();
	static BOOL		EnableTheme(CWnd* pWnd, BOOL bEnable = TRUE);

protected:
	typedef CMap< UINT, UINT, int, int > CUINTintMap;
	typedef CMap< CString, const CString&, UINT, UINT > CStringUINTMap;
	typedef CMap< HICON, HICON, HWND, HWND > CHICONHWNDMap;

	CStringUINTMap	m_pNameMap;
	CUINTintMap		m_pImageMap16;		// Small images (LVSIL_SMALL)
	CImageList		m_pImages16;		// Small images (LVSIL_SMALL)
	CUINTintMap		m_pImageMap32;		// Normal images (LVSIL_NORMAL)
	CImageList		m_pImages32;		// Normal images (LVSIL_NORMAL)
	CSize			m_czBuffer;
	CDC				m_dcBuffer;
	CBitmap			m_bmBuffer;
	HBITMAP			m_bmOldBuffer;
	CHICONHWNDMap	m_pWindowIcons;

	BOOL			ConfirmImageList();
};

extern CCoolInterface CoolInterface;

typedef struct
{
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	WORD* items() { return (WORD*)(this+1); }
} TOOLBAR_RES;

#endif // !defined(AFX_COOLINTERFACE_H__B7C102E2_0267_4F3B_B9C2_BA1031758891__INCLUDED_)
