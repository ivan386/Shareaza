//
// SkinWindow.h
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

#if !defined(AFX_SKINWINDOW_H__1FD170CD_1891_4A41_9ADF_50FCDB87CF7B__INCLUDED_)
#define AFX_SKINWINDOW_H__1FD170CD_1891_4A41_9ADF_50FCDB87CF7B__INCLUDED_

#pragma once

class CXMLElement;


class CSkinWindow
{
// Construction
public:
	CSkinWindow();
	virtual ~CSkinWindow();

// Attributes
public:
	CString			m_sTargets;
	CString			m_sLanguage;
	CDC				m_dcSkin;
	CBitmap			m_bmSkin;
	CBitmap			m_bmAlpha;
	CBitmap			m_bmWatermark;
	HBITMAP			m_hoSkin;
public:
	BOOL*			m_bPart;
	int*			m_nPart;
	CRect*			m_rcPart;
	BOOL*			m_bAnchor;
	CRect*			m_rcAnchor;
	CMapStringToPtr	m_pPartList;
	CMapStringToPtr	m_pAnchorList;
public:
	CSize			m_szMinSize;
	CRect			m_rcMaximise;
	CRect			m_rcResize;
	BOOL			m_bCaption;
	BOOL			m_bCaptionCaps;
	CRect			m_rcCaption;
	CFont			m_fnCaption;
	COLORREF		m_crCaptionFill;
	COLORREF		m_crCaptionText;
	COLORREF		m_crCaptionInactive;
	COLORREF		m_crCaptionShadow;
	COLORREF		m_crCaptionOutline;
	int				m_nCaptionAlign;
protected:
	CXMLElement*	m_pRegionXML;
	int				m_nHoverAnchor;
	int				m_nDownAnchor;
	int             m_nMirror;
	CRect           m_rcMirror;

// Operations
public:
	BOOL		Parse(CXMLElement* pXML, const CString& strPath);
	void		Prepare(CDC* pDC);
	void		Paint(CWnd* pWnd, CDC& dc, BOOL bCaption, TRISTATE bActive = TS_UNKNOWN);
	void		CalcWindowRect(RECT* pRect, BOOL bToClient = FALSE, BOOL bZoomed = FALSE);
	BOOL		GetPart(LPCTSTR pszName, CRect& rcPart);
	BOOL		GetAnchor(LPCTSTR pszName, CRect& rcAnchor);
	BOOL		GetAnchor(LPCTSTR pszName, const CRect& rcClient, CRect& rcAnchor);
	BOOL		PaintPartOnAnchor(CDC* pDC, const CRect& rcClient, LPCTSTR pszPart, LPCTSTR pszAnchor);
	BOOL		PreBlend(CBitmap* pbmTarget, const CRect& rcTarget, const CRect& rcSource);
public:
	void		OnNcCalcSize(CWnd* pWnd, BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	void		OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	UINT		OnNcHitTest(CWnd* pWnd, CPoint point, BOOL bResizable = FALSE);
	void		OnNcPaint(CWnd* pWnd);
	BOOL		OnNcActivate(CWnd* pWnd, BOOL bActive);
	void		OnSetText(CWnd* pWnd);
	void		OnSize(CWnd* pWnd);
	BOOL		OnEraseBkgnd(CWnd* pWnd, CDC* pDC);
	void		OnNcMouseMove(CWnd* pWnd, UINT nHitTest, CPoint point);
	BOOL		OnNcLButtonDown(CWnd* pWnd, UINT nHitTest, CPoint point);
	BOOL		OnNcLButtonUp(CWnd* pWnd, UINT nHitTest, CPoint point);
	BOOL		OnNcLButtonDblClk(CWnd* pWnd, UINT nHitTest, CPoint point);
	CSize		GetRegionSize();
protected:
	BOOL		ParseRect(CXMLElement* pXML, CRect* pRect);
	BOOL		ParseColour(const CString& str, COLORREF& cr);
	void		ResolveAnchor(const CRect& rcClient, CRect& rcAnchor, int nAnchor);
	void		SelectRegion(CWnd* pWnd);

};

enum
{
	SKINPART_TOP_LEFT, SKINPART_TOP, SKINPART_TOP_RIGHT,
	SKINPART_IA_TOP_LEFT, SKINPART_IA_TOP, SKINPART_IA_TOP_RIGHT,
	SKINPART_LEFT_TOP, SKINPART_LEFT, SKINPART_LEFT_BOTTOM,
	SKINPART_RIGHT_TOP, SKINPART_RIGHT, SKINPART_RIGHT_BOTTOM,
	SKINPART_BOTTOM_LEFT, SKINPART_BOTTOM, SKINPART_BOTTOM_RIGHT,

	SKINPART_SYSTEM, SKINPART_SYSTEM_HOT, SKINPART_SYSTEM_DOWN,
	SKINPART_MINIMISE, SKINPART_MINIMISE_HOT, SKINPART_MINIMISE_DOWN,
	SKINPART_MAXIMISE, SKINPART_MAXIMISE_HOT, SKINPART_MAXIMISE_DOWN,
	SKINPART_CLOSE, SKINPART_CLOSE_HOT, SKINPART_CLOSE_DOWN,

	SKINPART_COUNT
};

enum
{
	SKINANCHOR_ICON, SKINANCHOR_SYSTEM,
	SKINANCHOR_MINIMISE, SKINANCHOR_MAXIMISE, SKINANCHOR_CLOSE,

	SKINANCHOR_COUNT
};

enum
{
	SKINPARTMODE_TILE, SKINPARTMODE_STRETCH
};

#endif // !defined(AFX_SKINWINDOW_H__1FD170CD_1891_4A41_9ADF_50FCDB87CF7B__INCLUDED_)
