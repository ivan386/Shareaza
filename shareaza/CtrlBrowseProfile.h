//
// CtrlBrowseProfile.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#include "RichViewCtrl.h"
#include "ImageFile.h"

class CHostBrowser;
class CGProfile;
class CG2Packet;


class CBrowseProfileCtrl : public CWnd
{
// Construction
public:
	CBrowseProfileCtrl();
	virtual ~CBrowseProfileCtrl();

// Attributes
protected:
	CCriticalSection	m_pSection;
	CRichViewCtrl		m_wndDoc1;
	CRichDocument*		m_pDocument1;
	CRichElement*		m_pdNick;
	CRichElement*		m_pdFullName;
	CRichElement*		m_pdFullLocation;
	CRichElement*		m_pdGenderMale;
	CRichElement*		m_pdGenderFemale;
	CRichElement*		m_pdAge;
	CRichElement*		m_pdContactEmail;
	CRichElement*		m_pdContactMSN;
	CRichElement*		m_pdContactYahoo;
	CRichElement*		m_pdContactICQ;
	CRichElement*		m_pdContactAOL;
	CRichElement*		m_pdContactJabber;
	CRichElement*		m_pdBioText;
	CRichElement*		m_pdInterests;
protected:
	CRichViewCtrl		m_wndDoc2;
	CRichDocument*		m_pDocument2;
	CRichElement*		m_pdBookmarks;
protected:
	CImageFile			m_imgHead;

// Operations
public:
	void	Serialize(CArchive& ar, int nVersion /* BROWSER_SER_VERSION */);
	void	OnSkinChange();
	void	Update(CHostBrowser* pBrowser);
	void	OnHeadPacket(CG2Packet* pPacket);
protected:
	void	UpdateDocument1(CGProfile* pProfile);
	void	UpdateDocument2(CHostBrowser* pBrowser);
	void	LoadDefaultHead();

// Overrides
public:
	//{{AFX_VIRTUAL(CBrowseProfileCtrl)
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CBrowseProfileCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClickView(NMHDR* pNotify, LRESULT *pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

#define IDC_BROWSE_PROFILE	124
