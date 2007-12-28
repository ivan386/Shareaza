//
// CoolMenu.h
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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

#if !defined(AFX_COOLMENU_H__A1413F8B_7E02_4897_9C24_597CA8ACEE8F__INCLUDED_)
#define AFX_COOLMENU_H__A1413F8B_7E02_4897_9C24_597CA8ACEE8F__INCLUDED_

#pragma once


class CCoolMenu
{
// Construction
public:
	CCoolMenu();
	virtual ~CCoolMenu();

// Operations
public:
	BOOL		AddMenu(CMenu* pMenu, BOOL bChild = FALSE);
	BOOL		ReplaceMenuText(CMenu* pMenu, int nPosition, MENUITEMINFO FAR* mii, LPCTSTR pszText);
	void		SetWatermark(HBITMAP hBitmap);
	void		OnMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	void		OnDrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
protected:
	void		DrawMenuText(CDC* pDC, CRect* pRect, const CString& strText);
	void		DrawWatermark(CDC* pDC, CRect* pRect, int nOffX, int nOffY);
	void		SetSelectmark(HBITMAP hBitmap);

// Attributes
protected:
	// Note: CMap< DWORD_PTR, DWORD_PTR, ... causes a conversion to DWORD within CMap
	// DWORD_PTR& seems to solve the problem - should be investigated (TODO)
	// my guess: the compiler cannot distinguish between T and __w64 T with respect to overload resolution
	//   or template instantiation - in that case it's a false warning and should be supressed
	CMap< DWORD_PTR, DWORD_PTR&, CString, CString& >	m_pStrings;
protected:
	BOOL		m_bSelectTest;
	CBitmap		m_bmSelectmark;
	CBitmap		m_bmWatermark;
	CDC			m_dcWatermark;
	CSize		m_czWatermark;
	HBITMAP		m_hOldMark;
protected:
	BOOL		m_bEnable;
	BOOL		m_bUnhook;
	CString		m_sFilterString;
	CString		m_sOldFilterString;

// Border Hook
public:
	void			EnableHook();
	static void		EnableHook(BOOL bEnable);
	static void		RegisterEdge(int nLeft, int nTop, int nLength);
	static BOOL		IsModernVersion();
protected:
	static LRESULT	CALLBACK MsgHook(int nCode, WPARAM wParam, LPARAM lParam);
	static LRESULT	CALLBACK MenuProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	static HHOOK	m_hMsgHook;
	static LPCTSTR	wpnOldProc;
	static BOOL		m_bPrinted;
	static int		m_nEdgeLeft;
	static int		m_nEdgeTop;
	static int		m_nEdgeSize;

private:
	CCoolMenu(const CCoolMenu&);
	CCoolMenu& operator=(const CCoolMenu&);
};

extern CCoolMenu CoolMenu;

#endif // !defined(AFX_COOLMENU_H__A1413F8B_7E02_4897_9C24_597CA8ACEE8F__INCLUDED_)
