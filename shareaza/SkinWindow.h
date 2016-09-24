//
// SkinWindow.h
//
// Copyright (c) Shareaza Development Team, 2002-2016.
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

class CXMLElement;


class CSkinWindow
{
public:
	CSkinWindow();
	~CSkinWindow();

	typedef CMap< CString, const CString&, CRect*, CRect* > CRectMap;

	CString			m_sTargets;
	CString			m_sLanguage;
	CDC				m_dcSkin;
	CBitmap			m_bmSkin;
	CBitmap			m_bmWatermark;
	CRectMap		m_pAnchorList;
	CFont			m_fnCaption;
	COLORREF		m_crCaptionText;
	COLORREF		m_crCaptionInactive;
	COLORREF		m_crCaptionShadow;
	COLORREF		m_crCaptionOutline;

	BOOL		Parse(CXMLElement* pXML, const CString& strPath);
	void		Prepare(CDC* pDC);
	void		CalcWindowRect(RECT* pRect, BOOL bToClient = FALSE, BOOL bZoomed = FALSE);
	BOOL		GetPart(LPCTSTR pszName, CRect& rcPart);
	BOOL		GetAnchor(LPCTSTR pszName, CRect& rcAnchor);
	BOOL		GetAnchor(LPCTSTR pszName, const CRect& rcClient, CRect& rcAnchor);
	BOOL		PaintPartOnAnchor(CDC* pDC, const CRect& rcClient, LPCTSTR pszPart, LPCTSTR pszAnchor);
	BOOL		PreBlend(CBitmap* pbmTarget, const CRect& rcTarget, const CRect& rcSource);
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

protected:
	CBitmap			m_bmAlpha;
	HBITMAP			m_hoSkin;
	BOOL*			m_bPart;
	int*			m_nPart;
	CRect*			m_rcPart;
	BOOL*			m_bAnchor;
	CRect*			m_rcAnchor;
	CRectMap		m_pPartList;
	CSize			m_szMinSize;
	CRect			m_rcMaximise;
	CRect			m_rcResize;
	BOOL			m_bCaption;
	BOOL			m_bCaptionCaps;
	CRect			m_rcCaption;
	CXMLElement*	m_pRegionXML;
	int				m_nHoverAnchor;
	int				m_nDownAnchor;
	int             m_nMirror;
	CRect           m_rcMirror;
	int				m_nCaptionAlign;

	CSize		GetRegionSize();
	void		Paint(CWnd* pWnd, TRISTATE bActive = TRI_UNKNOWN);
	BOOL		ParseRect(const CXMLElement* pXML, CRect* pRect);
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
