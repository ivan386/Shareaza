//
// WndHitMonitor.h
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

#if !defined(AFX_WNDHITMONITOR_H__0979A194_3CF6_4921_81DC_1247DD774BCA__INCLUDED_)
#define AFX_WNDHITMONITOR_H__0979A194_3CF6_4921_81DC_1247DD774BCA__INCLUDED_

#pragma once

#include "WndBaseMatch.h"


class CHitMonitorWnd : public CBaseMatchWnd
{
// Construction
public:
	CHitMonitorWnd();
	virtual ~CHitMonitorWnd();

	DECLARE_SERIAL(CHitMonitorWnd)

// Attributes
protected:

// Operations
public:
	virtual BOOL	OnQueryHits(const CQueryHit* pHits);
	virtual void	OnSkinChange();

// Overrides
public:
	//{{AFX_VIRTUAL(CHitMonitorWnd)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CHitMonitorWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnHitMonitorClear();
	afx_msg void OnUpdateHitMonitorPause(CCmdUI* pCmdUI);
	afx_msg void OnHitMonitorPause();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WNDHITMONITOR_H__0979A194_3CF6_4921_81DC_1247DD774BCA__INCLUDED_)
