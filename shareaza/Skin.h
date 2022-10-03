//
// Skin.h
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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

#include "CoolMenu.h"

class CCoolBarCtrl;
class CXMLElement;
class CSkinWindow;


class CSkin
{
public:
	CSkin();
	~CSkin();

	void	CreateDefault();
	void	Apply();
	void	Clear();
	BOOL	LoadFromFile(LPCTSTR pszFile);
	BOOL	LoadFromResource(HINSTANCE hInstance, UINT nResourceID);
	BOOL	LoadFromString(const CString& strXML, const CString& strPath);
	BOOL	LoadFromXML(CXMLElement* pXML, const CString& strPath);
	static BOOL	SelectCaption(CWnd* pWnd, int nIndex);
	static BOOL	SelectCaption(CString& strCaption, int nIndex);
	static void	DrawWrappedText(CDC* pDC, CRect* pBox, LPCTSTR pszText, CPoint ptStart, BOOL bExclude = TRUE);
	static int GetTextFlowChange(LPCTSTR pszText, BOOL* bIsRTL);
	static BOOL LoadColour(CXMLElement* pXML, LPCTSTR pszName, COLORREF* pColour);

protected:
	mutable CCriticalSection m_pSection;
	void	ApplyRecursive(LPCTSTR pszPath);
	void	CreateDefaultColors();

// Strings
public:
	void	AddString(const CString& strString, UINT nStringID);
	BOOL	LoadString(CString& str, UINT nStringID) const;
protected:
	BOOL	LoadControlTip(CString& str, UINT nCtrlID);
	BOOL	LoadStrings(CXMLElement* pBase);
	BOOL	LoadControlTips(CXMLElement* pBase);
	CMap<UINT, UINT, CString, const CString&>	m_pStrings;
	CMap<UINT, UINT, CString, const CString&>	m_pControlTips;

// Menus
public:
	CMenu*	GetMenu(LPCTSTR pszName) const;
	BOOL	TrackPopupMenu(LPCTSTR pszMenu, const CPoint& point, UINT nDefaultID = 0, const CStringList& oFiles = CStringList(), CWnd* pWnd = AfxGetMainWnd(), UINT nFlags = TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON) const;
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

// Watermarks (images)
public:
	HBITMAP	GetWatermark(LPCTSTR pszName, BOOL bShared = FALSE);
	HBITMAP	LoadBitmap(const CString& strName, BOOL bShared = FALSE);
	HBITMAP	LoadBitmap(UINT nID, BOOL bShared = FALSE);
protected:
	BOOL	LoadWatermarks(CXMLElement* pSub, const CString& strPath);
	CMap< CString, const CString&, CString, CString& > m_pWatermarks;
	CMap< CString, const CString&, HBITMAP, const HBITMAP& > m_pBitmaps;

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
	CString	GetImagePath(UINT nImageID) const;
protected:
	CMap< UINT, const UINT&, CString, const CString& > m_pImages;
	static UINT LookupCommandID(CXMLElement* pXML, LPCTSTR pszName = _T("id"));
	BOOL	LoadResourceMap(CXMLElement* pBase);
	BOOL	LoadCommandImages(CXMLElement* pBase, const CString& strPath);
	BOOL	LoadCommandIcon(CXMLElement* pXML, const CString& strPath);
	BOOL	LoadCommandBitmap(CXMLElement* pBase, const CString& strPath);

// Mode Suffixes
protected:
	static LPCTSTR m_pszModeSuffix[3][4];

// NavBar
public:
	BOOL		m_bBordersEnabled;
	COLORREF	m_crNavBarText;
	COLORREF	m_crNavBarTextUp;
	COLORREF	m_crNavBarTextDown;
	COLORREF	m_crNavBarTextHover;
	COLORREF	m_crNavBarTextChecked;
	COLORREF	m_crNavBarShadow;
	COLORREF	m_crNavBarShadowUp;
	COLORREF	m_crNavBarShadowDown;
	COLORREF	m_crNavBarShadowHover;
	COLORREF	m_crNavBarShadowChecked;
	COLORREF	m_crNavBarOutline;
	COLORREF	m_crNavBarOutlineUp;
	COLORREF	m_crNavBarOutlineDown;
	COLORREF	m_crNavBarOutlineHover;
	COLORREF	m_crNavBarOutlineChecked;
	CRect		m_rcNavBarOffset;
	enum { NavBarNormal, NavBarUpper, NavBarLower } m_NavBarMode;
protected:
	BOOL	LoadNavBar(CXMLElement* pBase);

private:
	CSkin(const CSkin&);
	CSkin& operator=(const CSkin&);
};


extern CSkin Skin;
