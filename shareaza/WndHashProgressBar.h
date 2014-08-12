//
// WndHashProgressBar.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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


class CHashProgressBar : public CWnd
{
	DECLARE_DYNCREATE(CHashProgressBar)

public:
	CHashProgressBar();

	void		Run();

protected:
	CString		m_sCurrent;				// Hashing filename
	size_t		m_nRemaining;			// Hashing queue size
	DWORD		m_nPercentage;			// Hashing file progress (0..100%)
	DWORD		m_nLastShow;			// Time of last update
	int			m_nPerfectWidth;		// Window perfect width for text fitting
	BYTE		m_nAlpha;
	int			m_nIcon;

	void		Draw(CDC* pDC);			// Redraw window and calculate perfect width

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
