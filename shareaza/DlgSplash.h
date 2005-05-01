//
// DlgSplash.h
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

#pragma once


class CSplashDlg : public CDialog
{
// Construction
public:
	CSplashDlg(int nMax = 0, BOOL bSilent = FALSE);
	virtual ~CSplashDlg();

	enum { IDD = IDD_SPLASH };
	DECLARE_DYNAMIC(CSplashDlg)

// Attributes
protected:
	int			m_nPos;
	int			m_nMax;
	BOOL		m_bSilent;
	CString		m_sState;
protected:
	CBitmap		m_bmSplash;
	CBitmap		m_bmBuffer;
	CDC			m_dcBuffer1;
	CDC			m_dcBuffer2;
protected:
	HINSTANCE	m_hUser32;
	BOOL		(WINAPI *m_pfnAnimateWindow)(HWND, DWORD, DWORD);

// Operations
public:
	void	Step(LPCTSTR pszText);
	void	Topmost();
	void	Hide();
protected:
	void	DoPaint(CDC* pDC);

// Overrides
public:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg LONG OnPrintClient(WPARAM wParam, LPARAM lParam);

};
