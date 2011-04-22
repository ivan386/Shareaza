//
// CtrlTipAlbum.h
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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

#include "CtrlCoolTip.h"
#include "MetaList.h"


class CAlbumTipCtrl : public CCoolTipCtrl
{
	DECLARE_DYNAMIC(CAlbumTipCtrl)

public:
	CAlbumTipCtrl();
	virtual ~CAlbumTipCtrl();

	void Show(CAlbumFolder* pContext, HWND hAltWnd = NULL)
	{
		bool bChanged = ( pContext != m_pAlbumFolder );
		m_pAlbumFolder = pContext;
		m_hAltWnd = hAltWnd;
		ShowImpl( bChanged );
	}

protected:
	CAlbumFolder*	m_pAlbumFolder;
	CString			m_sName;
	CString			m_sType;
	CString			m_sFilesTitle;
	CString			m_sFiles;
	CString			m_sVolumeTitle;
	CString			m_sVolume;
	int				m_nIcon32;
	BOOL			m_bCollection;
	CMetaList		m_pMetadata;
	int				m_nKeyWidth;
	COLORREF		m_crLight;

	virtual BOOL OnPrepare();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnPaint(CDC* pDC);

	DECLARE_MESSAGE_MAP()
};
