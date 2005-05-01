//
// CoolMenu.h
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
	void		SetWatermark(HBITMAP hBitmap);
	void		OnMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	void		OnDrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
protected:
	void		DrawMenuText(CDC* pDC, CRect* pRect, const CString& strText);
	void		DrawWatermark(CDC* pDC, CRect* pRect, int nOffX, int nOffY);

// Attributes
protected:
	CMap<DWORD, DWORD, CString, CString&>	m_pStrings;
	int			m_nCheckIcon;
protected:
	CBitmap		m_bmWatermark;
	CDC			m_dcWatermark;
	CSize		m_czWatermark;
	HBITMAP		m_hOldMark;
protected:
	BOOL		m_bEnable;
	BOOL		m_bUnhook;

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

};

extern CCoolMenu CoolMenu;

#endif // !defined(AFX_COOLMENU_H__A1413F8B_7E02_4897_9C24_597CA8ACEE8F__INCLUDED_)
