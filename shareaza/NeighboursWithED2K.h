//
// NeighboursWithED2K.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#if !defined(AFX_NEIGHBOURSWITHED2K_H__0165D8B7_0351_4EF7_B669_73D0072F5107__INCLUDED_)
#define AFX_NEIGHBOURSWITHED2K_H__0165D8B7_0351_4EF7_B669_73D0072F5107__INCLUDED_

#pragma once

#include "NeighboursWithG2.h"

class CEDNeighbour;
class CDownload;


class CNeighboursWithED2K : public CNeighboursWithG2
{
// Construction
public:
	CNeighboursWithED2K();
	virtual ~CNeighboursWithED2K();

// Operations
public:
	CEDNeighbour*	GetDonkeyServer() const;
	void			CloseDonkeys();
	void			SendDonkeyDownload(CDownload* pDownload);
	BOOL			PushDonkey(DWORD nClientID, IN_ADDR* pServerAddress, WORD nServerPort);
	BOOL			FindDonkeySources(MD4* pED2K, IN_ADDR* pServerAddress, WORD nServerPort);

// Attributes
protected:
	DWORD			m_tEDSources[256];
	MD4				m_pEDSources[256];
	
};

#endif // !defined(AFX_NEIGHBOURSWITHED2K_H__0165D8B7_0351_4EF7_B669_73D0072F5107__INCLUDED_)
