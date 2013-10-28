//
// CtrlMatchTip.h
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

#include "CtrlCoolTip.h"
#include "MetaList.h"

class CMatchFile;
class CQueryHit;


class CMatchTipCtrl : public CCoolTipCtrl
{
	DECLARE_DYNAMIC(CMatchTipCtrl)

public:
	CMatchTipCtrl();

	void		Show(CMatchFile* pFile, CQueryHit* pHit);

protected:
	CMatchFile*		m_pFile;
	CQueryHit*		m_pHit;
	CString			m_sName;
	CString			m_sUser;
	CString			m_sCountryCode;
	CString			m_sCountry;
	CString			m_sSHA1;
	CString			m_sTiger;
	CString			m_sED2K;
	CString			m_sBTH;
	CString			m_sMD5;
	CString			m_sType;
	CString			m_sSize;
	CString			m_sBusy;		// Busy status message
	CString			m_sPush;		// Firewalled status message
	CString			m_sUnstable;	// Unstable status message
	int				m_nIcon;
	CString			m_sStatus;
	COLORREF		m_crStatus;
	CString			m_sPartial;
	CString			m_sQueue;
	CSchemaPtr		m_pSchema;
	CMetaList		m_pMetadata;
	int				m_nKeyWidth;
	int				m_nRating;

protected:
	void		LoadFromFile();
	void		LoadFromHit();
	BOOL		LoadTypeInfo();

	virtual BOOL OnPrepare();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnPaint(CDC* pDC);

	DECLARE_MESSAGE_MAP()
};
