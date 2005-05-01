//
// CtrlSearchDetailPanel.h
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

#if !defined(AFX_CTRLSEARCHDETAILPANEL_H__946418DF_EE15_4346_A4A7_FA1E4672FC3C__INCLUDED_)
#define AFX_CTRLSEARCHDETAILPANEL_H__946418DF_EE15_4346_A4A7_FA1E4672FC3C__INCLUDED_

#pragma once

#include "MetaPanel.h"
#include "RichDocument.h"
#include "RichViewCtrl.h"
#include "HttpRequest.h"

class CMatchFile;
class CImageFile;


class CSearchDetailPanel : public CWnd
{
// Construction
public:
	CSearchDetailPanel();
	virtual ~CSearchDetailPanel();

	DECLARE_DYNAMIC(CSearchDetailPanel)

// Operations
public:
	void		Update(CMatchFile* pFile);
protected:
	static void	DrawText(CDC* pDC, int nX, int nY, LPCTSTR pszText, RECT* pRect = NULL);
	void		DrawThumbnail(CDC* pDC, CRect& rcClient, CRect& rcWork);
	void		DrawThumbnail(CDC* pDC, CRect& rcThumb);
	void		ClearReviews();
protected:
	BOOL		RequestPreview();
	void		CancelPreview();
	static UINT	ThreadStart(LPVOID pParam);
	void		OnRun();
	BOOL		ExecuteRequest(CString strURL, BYTE** ppBuffer, DWORD* pnBuffer);
	void		OnPreviewLoaded(SHA1* pSHA1, CImageFile* pImage);
	BOOL		CachePreviewImage(SHA1* pSHA1, LPBYTE pBuffer, DWORD nBuffer);

// Item
protected:
	class Review
	{
	public:
		Review(GGUID* pGUID, IN_ADDR* pAddress, LPCTSTR pszNick, int nRating, LPCTSTR pszComments);
		virtual ~Review();
		void			Layout(CSearchDetailPanel* pParent, CRect* pRect);
		void			Reposition(int nScroll);
		void			Paint(CDC* pDC, int nScroll);
	public:
		GGUID			m_pGUID;
		CString			m_sNick;
		int				m_nRating;
		CRichDocument	m_pComments;
		CRichViewCtrl	m_wndComments;
		CRect			m_rc;
	};
	friend class Review;

// Attributes
protected:
	CMatchList*			m_pMatches;
	BOOL				m_bValid;
	CMatchFile*			m_pFile;
	SHA1				m_pSHA1;
	CString				m_sName;
	CString				m_sStatus;
	CRect				m_rcStatus;
	CString				m_sSize;
	int					m_nIcon48;
	int					m_nIcon32;
	int					m_nRating;
	CSchema*			m_pSchema;
	CMetaPanel			m_pMetadata;
	CPtrList			m_pReviews;
protected:
	CCriticalSection	m_pSection;
	CEvent				m_pWakeup;
	BOOL				m_bCanPreview;
	BOOL				m_bRunPreview;
	BOOL				m_bIsPreviewing;
	HANDLE				m_hThread;
	BOOL				m_bThread;
	CHttpRequest		m_pRequest;
	CStringList			m_pPreviewURLs;
protected:
	CBitmap				m_bmThumb;
	CSize				m_szThumb;
	CRect				m_rcThumb;
	COLORREF			m_crLight;
	int					m_nThumbSize;

// Overrides
public:
	//{{AFX_VIRTUAL(CSearchDetailPanel)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CSearchDetailPanel)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClickReview(RVN_ELEMENTEVENT* pNotify, LRESULT *pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

#define IDC_DETAIL_PANEL	104
#define IDC_REVIEW_VIEW		99

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLSEARCHDETAILPANEL_H__946418DF_EE15_4346_A4A7_FA1E4672FC3C__INCLUDED_)
