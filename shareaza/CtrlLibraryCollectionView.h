//
// CtrlLibraryCollectionView.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

#include "CtrlLibraryFileView.h"

class CCollectionFile;
class CLibraryCollectionView;
class CLibraryFile;
class CWebCtrl;


class CHtmlCollection : public CComObject
{
	DECLARE_DYNAMIC(CHtmlCollection)

public:
	CHtmlCollection();
	virtual ~CHtmlCollection();

protected:
	CLibraryCollectionView*	m_pView;

	BEGIN_INTERFACE_PART(View, ICollectionHtmlView)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication **ppApplication);
		STDMETHOD(Detect)(BSTR sURN, BSTR *psState);
		STDMETHOD(Hover)(BSTR sURN);
		STDMETHOD(Open)(BSTR sURN, VARIANT_BOOL *pbResult);
		STDMETHOD(Enqueue)(BSTR sURN, VARIANT_BOOL *pbResult);
		STDMETHOD(Download)(BSTR sURN, VARIANT_BOOL *pbResult);
		STDMETHOD(DownloadAll)();
		STDMETHOD(get_MissingCount)(LONG *pnCount);
	END_INTERFACE_PART(View)

	DECLARE_INTERFACE_MAP()

	friend class CLibraryCollectionView;
};


class CLibraryCollectionView : public CLibraryFileView
{
	DECLARE_DYNAMIC(CLibraryCollectionView)

public:
	CLibraryCollectionView();
	virtual ~CLibraryCollectionView();

	virtual void	SelectAll() {}
	virtual BOOL	CheckAvailable(CLibraryTreeItem* pSel);
	virtual void	Update();

protected:
	CWebCtrl*			m_pWebCtrl;
	DWORD				m_nWebIndex;
	CCollectionFile*	m_pCollection;
    Hashes::Sha1Hash    m_oSHA1;
	CHtmlCollection		m_xExternal;
	BOOL				m_bLockdown;
	TRISTATE			m_bTrusted;

	virtual BOOL Create(CWnd* pParentWnd);
	virtual DWORD_PTR HitTestIndex(const CPoint& /*point*/) const { return 0; };

	BOOL ShowCollection(CLibraryFile* pFile);

	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWebContextMenu(NMHDR* pNMHDR, LPARAM* pResult);
	afx_msg void OnUpdateLibraryFolderDownload(CCmdUI *pCmdUI);
	afx_msg void OnLibraryFolderDownload();
	afx_msg UINT OnGetDlgCode();

	DECLARE_MESSAGE_MAP()

	friend class CHtmlCollection;
};
