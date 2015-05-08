//
// CtrlWizard.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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


class CWizardCtrl : public CWnd
{
	DECLARE_DYNAMIC(CWizardCtrl)

public:
	CWizardCtrl();
	virtual ~CWizardCtrl();

	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, LPCTSTR pszXMLPath, const CAlbumFolder* pFolder);

	BOOL			IsValid() const { return m_bValid; }
	size_t			GetSize() const { return m_pControls.GetSize(); }
	BOOL			OnTab();

	CString				m_sMainFilePath;
	CString				m_sEvenFilePath;
	CString				m_sOddFilePath;
	CArray< CString >	m_pFileDocs;	// all documents for each file
	CArray< CString >	m_pFilePaths;	// all file paths
	CArray< CString >	m_pImagePaths;
	CArray< CString >	m_pTemplatePaths;
	CStringIMap			m_pItems;

protected:
	int					m_nCaptionWidth;
	int					m_nItemHeight;
	BOOL				m_bShowBorder;
	BOOL				m_bValid;
	CString				m_sEvenFile;
	CString				m_sOddFile;
	CArray< CWnd* >		m_pControls;	// holds all controls
	CArray< CString >	m_pCaptions;	// all label texts
	int					m_nScroll;

	void			Layout();
	void			SetFocusTo(CWnd* pCtrl);
	BOOL			CollectFiles(CXMLElement* pBase);
	BOOL			CollectImages(CXMLElement* pBase);
	BOOL			MakeControls(const CString& sXMLPath, CXMLElement* pBase, std::vector< const CLibraryFile* > pList);
	BOOL			PrepareDoc(CLibraryFile* pFile, LPCTSTR pszTemplate = _T("") );
	void			MakeAll(const CString& sXMLPath, const CAlbumFolder* pFolder);
	void			ScrollBy(int nDelta);
	void			Clear();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBtnPress();
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};

#define IDC_WIZARD_CONTROL		100
