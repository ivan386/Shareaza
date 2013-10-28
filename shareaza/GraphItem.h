//
// GraphItem.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

typedef struct
{
	DWORD	m_nCode;
	UINT	m_nStringID;
	UINT	m_nUnits;
	float	m_nMultiplier;
} GRAPHITEM;


class CGraphItem
{
public:
	CGraphItem(DWORD nCode = 0, float nMultiplier = 1.0f, COLORREF nColour = RGB(255,255,255));
	~CGraphItem();

	DWORD		m_nCode;
	float		m_nMultiplier;
	COLORREF	m_nColour;
	CString		m_sName;
	CPen		m_pPen[4];
	COLORREF	m_cPen[4];
	DWORD*		m_pData;
	DWORD		m_nData;
	DWORD		m_nLength;
	DWORD		m_nPosition;

	static const GRAPHITEM	m_pItemDesc[];

	void		SetCode(DWORD nCode);
	void		Clear();
	DWORD		Add(DWORD nValue);
	DWORD		GetValueAt(DWORD nPosition) const;
	DWORD		GetMaximum() const;
	void		SetHistory(DWORD nSize, BOOL bMax = FALSE);
	DWORD		Update();
	void		Serialize(CArchive& ar);
	void		MakeGradient(COLORREF crBack);

	static QWORD GetValue(const DWORD nCode, const float nMultiplier = 1.0f);
	static const GRAPHITEM* GetItemDesc(const DWORD nCode);
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
#define GRC_GNUTELLA_QUERIES_PROCESSED	45

#define GRC_GNUTELLA_PINGS				51
#define GRC_GNUTELLA_PONGS				52

#define GRC_CONNECTION_ERRORS			61

#define GRC_RANDOM						100
