//
// GraphItem.h
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

#if !defined(AFX_GRAPHITEM_H__6C5709B5_EB88_4CAB_B61E_74F11AAB9F65__INCLUDED_)
#define AFX_GRAPHITEM_H__6C5709B5_EB88_4CAB_B61E_74F11AAB9F65__INCLUDED_

#pragma once

typedef struct
{
	DWORD	m_nCode;
	UINT	m_nStringID;
	UINT	m_nUnits;
} GRAPHITEM;


class CGraphItem
{
// Construction
public:
	CGraphItem(DWORD nCode = 0, DWORD nParam = 0, COLORREF nColour = RGB(255,255,255));
	virtual ~CGraphItem();

// Attributes
public:
	DWORD		m_nCode;
	DWORD		m_nParam;
	COLORREF	m_nColour;
public:
	CString		m_sName;
	CPen		m_pPen[4];
	COLORREF	m_cPen[4];
public:
	DWORD*		m_pData;
	DWORD		m_nData;
	DWORD		m_nLength;
	DWORD		m_nPosition;
public:
	static GRAPHITEM	m_pItemDesc[];

// Operations
public:
	void		SetCode(DWORD nCode);
	void		Clear();
	DWORD		Add(DWORD nValue);
	DWORD		GetValueAt(DWORD nPosition) const;
	DWORD		GetMaximum() const;
	void		SetHistory(DWORD nSize, BOOL bMax = FALSE);
	DWORD		Update();
	void		Serialize(CArchive& ar);
	void		MakeGradient(COLORREF crBack);
public:
	static QWORD		GetValue(DWORD nCode, DWORD nParam = 0);
	static GRAPHITEM*	GetItemDesc(DWORD nCode);

};

#define GRC_TOTAL_BANDWIDTH_IN			1
#define GRC_TOTAL_BANDWIDTH_OUT			2
#define GRC_TOTAL_BANDWIDTH_TCP_IN		3
#define GRC_TOTAL_BANDWIDTH_TCP_OUT		4
#define GRC_TOTAL_BANDWIDTH_UDP_IN		5
#define GRC_TOTAL_BANDWIDTH_UDP_OUT		6

#define GRC_GNUTELLA_CONNECTIONS		11
#define GRC_GNUTELLA_CONNECTIONS_ALL	12
#define GRC_GNUTELLA_BANDWIDTH_IN		13
#define GRC_GNUTELLA_BANDWIDTH_OUT		14
#define GRC_GNUTELLA_PACKETS_IN			15
#define GRC_GNUTELLA_PACKETS_OUT		16

#define GRC_DOWNLOADS_FILES				21
#define GRC_DOWNLOADS_TRANSFERS			22
#define GRC_DOWNLOADS_BANDWIDTH			23

#define GRC_UPLOADS_TRANSFERS			31
#define GRC_UPLOADS_BANDWIDTH			32

#define GRC_GNUTELLA_ROUTED				41
#define GRC_GNUTELLA_DROPPED			42
#define GRC_GNUTELLA_LOST				43
#define GRC_GNUTELLA_QUERIES			44

#define GRC_RANDOM						100

#endif // !defined(AFX_GRAPHITEM_H__6C5709B5_EB88_4CAB_B61E_74F11AAB9F65__INCLUDED_)
