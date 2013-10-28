//
// CtrlHomePanel.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

#include "CtrlRichTaskBox.h"
#include "CtrlDownloadTip.h"
#include "CtrlLibraryTip.h"
#include "Neighbour.h"

class CDownload;
class CLibraryRecent;


class CHomeDownloadsBox : public CRichTaskBox
{
// Construction
public:
	CHomeDownloadsBox();
	virtual ~CHomeDownloadsBox();
	DECLARE_DYNAMIC(CHomeDownloadsBox)

// Attributes
protected:
	class Item
	{
	public:
		inline Item () throw() :
			m_pDownload( NULL ),
			m_nIcon16( 0 ),
			m_nSize( 0 ),
			m_nComplete( 0 ),
			m_bPaused( FALSE ) {}

		CDownload*	m_pDownload;
		CString		m_sText;
		int			m_nIcon16;
		QWORD		m_nSize;
		QWORD		m_nComplete;
		BOOL		m_bPaused;
	};

	CRichElement*		m_pdDownloadsNone;
	CRichElement*		m_pdDownloadsOne;
	CRichElement*		m_pdDownloadsMany;
	CString				m_sDownloadsMany;
	CArray< Item* >		m_pList;
	CFont				m_pFont;
	HCURSOR				m_hHand;
	CDownloadTipCtrl	m_wndTip;
	Item*				m_pHover;

// Operations
public:
	void	OnSkinChange();
	void	Update();
	Item*	HitTest(const CPoint& point) const;
	BOOL	ExecuteDownload(CDownload* pDownload);

// Implementation
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};


class CHomeLibraryBox : public CRichTaskBox
{
// Construction
public:
	CHomeLibraryBox();
	virtual ~CHomeLibraryBox();
	DECLARE_DYNAMIC(CHomeLibraryBox)

// Attributes
protected:
	class Item
	{
	public:
		inline Item() throw() :
			m_pRecent( NULL ),
			m_nIndex( 0 ),
			m_nIcon16( 0 ) {}

		CLibraryRecent*	m_pRecent;
		DWORD			m_nIndex;
		CString			m_sText;
		int				m_nIcon16;
	};

	CRichElement*	m_pdLibraryFiles;
	CRichElement*	m_pdLibraryVolume;
	CRichElement*	m_pdLibraryHashRemaining;
	CArray< Item* >	m_pList;
	CFont			m_pFont;
	HCURSOR			m_hHand;
	Item*			m_pHover;
	CLibraryTipCtrl	m_wndTip;

// Operations
public:
	void	OnSkinChange();
	void	Update();
	Item*	HitTest(const CPoint& point) const;

// Implementation
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};


class CHomeUploadsBox : public CRichTaskBox
{
// Construction
public:
	CHomeUploadsBox();
	virtual ~CHomeUploadsBox();
	DECLARE_DYNAMIC(CHomeUploadsBox)

// Attributes
protected:
	CRichElement*	m_pdUploadsNone;
	CRichElement*	m_pdUploadsOne;
	CRichElement*	m_pdUploadsMany;
	CRichElement*	m_pdUploadedNone;
	CRichElement*	m_pdUploadedOne;
	CRichElement*	m_pdUploadedMany;
	CString			m_sUploadsMany;
	CString			m_sUploadedOne;
	CString			m_sUploadedMany;

// Operations
public:
	void		OnSkinChange();
	void		Update();

// Implementation
protected:

	DECLARE_MESSAGE_MAP()
};


class CHomeConnectionBox : public CRichTaskBox
{
// Construction
public:
	CHomeConnectionBox();
	virtual ~CHomeConnectionBox();
	DECLARE_DYNAMIC(CHomeConnectionBox)

// Attributes
protected:
	CRichElement*	m_pdConnectedHours;
	CRichElement*	m_pdConnectedMinutes;
	CRichElement*	m_pdCount[ PROTOCOL_LAST ][ ntLeaf + 1 ];
	CString			m_sCount[ PROTOCOL_LAST ][ ntLeaf + 1 ];

// Operations
public:
	void	OnSkinChange();
	void	Update();

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

class CHomeTorrentsBox : public CRichTaskBox
{
// Construction
public:
	CHomeTorrentsBox();
	virtual ~CHomeTorrentsBox();
	DECLARE_DYNAMIC(CHomeTorrentsBox)

// Attributes
protected:
	CRichElement*	m_pdTorrentsNone;
	CRichElement*	m_pdTorrentsOne;
	CRichElement*	m_pdTorrentsMany;
	CString			m_sTorrentsMany;
	CRichElement*	m_pdReseedTorrent;
	CString			m_sReseedTorrent;

// Operations
public:
	void	OnSkinChange();
	void	Update();

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

class CHomePanel : public CTaskPanel
{
// Construction
public:
	CHomePanel();

	DECLARE_DYNAMIC(CHomePanel)

// Attributes
	CHomeDownloadsBox	m_boxDownloads;
	CHomeUploadsBox		m_boxUploads;
	CHomeConnectionBox	m_boxConnection;
	CHomeLibraryBox		m_boxLibrary;
#ifndef LAN_MODE
	CHomeTorrentsBox	m_boxTorrents;
#endif // LAN_MODE

	virtual BOOL Create(CWnd* pParentWnd);

	void	OnSkinChange();
	void	Update();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#define IDC_HOME_PANEL	111
