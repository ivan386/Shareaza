//
// CoolInterface.h
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

#if !defined(AFX_COOLINTERFACE_H__B7C102E2_0267_4F3B_B9C2_BA1031758891__INCLUDED_)
#define AFX_COOLINTERFACE_H__B7C102E2_0267_4F3B_B9C2_BA1031758891__INCLUDED_

#pragma once


class CCoolInterface
{
// Construction
public:
	CCoolInterface();
	virtual ~CCoolInterface();

// Attributes
public:
	CMapStringToPtr	m_pNameMap;
	CMapPtrToPtr	m_pImageMap;
	CImageList		m_pImages;
public:
	CFont		m_fntNormal;
	CFont		m_fntBold;
	CFont		m_fntUnder;
	CFont		m_fntCaption;
	CFont		m_fntItalic;
public:
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
protected:
	CSize		m_czBuffer;
	CDC			m_dcBuffer;
	CBitmap		m_bmBuffer;
	HBITMAP		m_bmOldBuffer;

// Operations
public:
	void		Clear();
public:
	void		NameCommand(UINT nID, LPCTSTR pszName);
	UINT		NameToID(LPCTSTR pszName);
public:
	int			ImageForID(UINT nID);
	void		AddIcon(UINT nID, HICON hIcon);
	void		CopyIcon(UINT nFromID, UINT nToID);
	HICON		ExtractIcon(UINT nID);
	BOOL		AddImagesFromToolbar(UINT nIDToolBar, COLORREF crBack = RGB(0,255,0));
	BOOL		ConfirmImageList();
public:
	CDC*		GetBuffer(CDC& dcScreen, CSize& szItem);
	BOOL		DrawWatermark(CDC* pDC, CRect* pRect, CBitmap* pMark, int nOffX = 0, int nOffY = 0);
public:
	void		CreateFonts(LPCTSTR pszFace = NULL, int nSize = 0);
	void		CalculateColours(BOOL bCustom = FALSE);
	void		OnSysColourChange();
public:
	static COLORREF	CalculateColour(COLORREF crFore, COLORREF crBack, int nAlpha);
	static COLORREF	GetDialogBkColor();
	static BOOL		IsNewWindows();
	static BOOL		EnableTheme(CWnd* pWnd, BOOL bEnable = TRUE);

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
