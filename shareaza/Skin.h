//
// Skin.h
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

#if !defined(AFX_SKIN_H__E4027F91_1EB1_4B0D_82E2_C8DD1E7F054F__INCLUDED_)
#define AFX_SKIN_H__E4027F91_1EB1_4B0D_82E2_C8DD1E7F054F__INCLUDED_

#pragma once

#include "CoolMenu.h"

class CCoolBarCtrl;
class CXMLElement;
class CSkinWindow;


class CSkin
{
// Construction
public:
	CSkin();
	virtual ~CSkin();

// Operations
public:
	void	Apply();
	void	Clear();
	BOOL	LoadFromFile(LPCTSTR pszFile);
	BOOL	LoadFromResource(HINSTANCE hInstance, UINT nResourceID);
	BOOL	LoadFromString(const CString& strXML, const CString& strPath);
	BOOL	LoadFromXML(CXMLElement* pXML, const CString& strPath);
	BOOL	SelectCaption(CWnd* pWnd, int nIndex);
	BOOL	SelectCaption(CString& strCaption, int nIndex);
	void	DrawWrappedText(CDC* pDC, CRect* pBox, LPCTSTR pszText, CPoint ptStart, BOOL bExclude = TRUE);
protected:
	void	ApplyRecursive(LPCTSTR pszPath);
	void	CreateDefault();
	void	Finalise();
	HBITMAP	LoadBitmap(CString& strName);

// Strings
public:
	BOOL	LoadString(CString& str, UINT nStringID);
	BOOL	LoadControlTip(CString& str, UINT nCtrlID);
	int		GetTextFlowChange(LPCTSTR pszText, BOOL* bIsRTL);
protected:
	BOOL	LoadStrings(CXMLElement* pBase);
	BOOL	LoadControlTips(CXMLElement* pBase);
	CMap<UINT, UINT, CString, CString&>	m_pStrings;
	CMap<UINT, UINT, CString, CString&>	m_pControlTips;

// Menus
public:
	CMenu*	GetMenu(LPCTSTR pszName) const;
	UINT	TrackPopupMenu(LPCTSTR pszMenu, const CPoint& point, UINT nDefaultID = 0, UINT nFlags = 0) const;
	CMenu*	CreatePopupMenu(LPCTSTR pszName);
protected:
	CMap< CString, const CString&, CMenu*, CMenu* > m_pMenus;
	BOOL	LoadMenus(CXMLElement* pBase);
	BOOL	LoadMenu(CXMLElement* pXML);
	BOOL	CreateMenu(CXMLElement* pXML, HMENU hMenu);

// Toolbars
public:
	BOOL			CreateToolBar(LPCTSTR pszName, CCoolBarCtrl* pBar);
	CCoolBarCtrl*	CreateToolBar(LPCTSTR pszName);
	CCoolBarCtrl*	GetToolBar(LPCTSTR pszName) const;
protected:
	CMap< CString, const CString&, CCoolBarCtrl*, CCoolBarCtrl* > m_pToolbars;
	BOOL	LoadToolbars(CXMLElement* pBase);
	BOOL	CreateToolBar(CXMLElement* pElement);

// Documents
public:
	CXMLElement*	GetDocument(LPCTSTR pszName);
protected:
	BOOL			LoadDocuments(CXMLElement* pBase);
	CMap< CString, const CString&, CXMLElement*, CXMLElement* > m_pDocuments;

// Watermarks
public:
	HBITMAP	GetWatermark(LPCTSTR pszName);
	BOOL	GetWatermark(CBitmap* pBitmap, LPCTSTR pszName);
protected:
	BOOL	LoadWatermarks(CXMLElement* pSub, const CString& strPath);
	CMap< CString, const CString&, CString, CString& > m_pWatermarks;

// Translate
public:
	BOOL	Translate(LPCTSTR pszName, CHeaderCtrl* pCtrl);
	CString GetHeaderTranslation(LPCTSTR pszClassName, LPCTSTR pszHeaderName);
protected:
	BOOL	LoadListColumns(CXMLElement* pBase);
	CMap< CString, const CString&, CString, CString& > m_pLists;

// Dialogs
public:
	BOOL	Apply(LPCTSTR pszName, CDialog* pDialog, UINT nIconID = 0, CToolTipCtrl* pWndTooltips = NULL);
	CString	GetDialogCaption(LPCTSTR pszName);
protected:
	BOOL	LoadDialogs(CXMLElement* pBase);
	CMap< CString, const CString&, CXMLElement*, CXMLElement* >	m_pDialogs;

// Window Skins
public:
	CSkinWindow*	GetWindowSkin(LPCTSTR pszWindow, LPCTSTR pszAppend = NULL);
	CSkinWindow*	GetWindowSkin(CWnd* pWnd);
protected:
	BOOL			LoadWindowSkins(CXMLElement* pSub, const CString& strPath);
	CList< CSkinWindow* > m_pSkins;

// Colour Scheme
public:
	COLORREF	m_crDialog;
	CBrush		m_brDialog;
	COLORREF	m_crPanelBack;
	CBitmap		m_bmPanelMark;
	COLORREF	m_crPanelText;
	COLORREF	m_crPanelBorder;
	COLORREF	m_crBannerBack;
	COLORREF	m_crBannerText;
	COLORREF	m_crSchemaRow[2];
protected:
	BOOL		LoadColourScheme(CXMLElement* pBase);

// Fonts
protected:
	CList< CString >	m_pFontPaths;
protected:
	BOOL		LoadFonts(CXMLElement* pBase, const CString& strPath);

// Other
public:
	UINT	LookupCommandID(CXMLElement* pXML, LPCTSTR pszName = NULL) const;
protected:
	BOOL	LoadResourceMap(CXMLElement* pBase);
	BOOL	LoadCommandImages(CXMLElement* pBase, const CString& strPath);
	BOOL	LoadCommandBitmap(CXMLElement* pBase, const CString& strPath);

// Mode Suffixes
protected:
	static LPCTSTR m_pszModeSuffix[3][4];

private:
	CSkin(const CSkin&);
	CSkin& operator=(const CSkin&);
};


extern CSkin Skin;

#endif // !defined(AFX_SKIN_H__E4027F91_1EB1_4B0D_82E2_C8DD1E7F054F__INCLUDED_)
