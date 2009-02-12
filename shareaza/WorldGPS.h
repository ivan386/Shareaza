//
// WorldGPS.h
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

#if !defined(AFX_WORLDGPS_H__763AAF9D_7289_45A1_9026_4E2BF703C0E8__INCLUDED_)
#define AFX_WORLDGPS_H__763AAF9D_7289_45A1_9026_4E2BF703C0E8__INCLUDED_

#pragma once

class CWorldCountry;
class CWorldCity;
class CXMLElement;


class CWorldGPS
{
// Construction
public:
	CWorldGPS();
	~CWorldGPS();

// Attributes
public:
	CWorldCountry*	m_pCountry;
	DWORD			m_nCountry;

// Operations
public:
	BOOL		Load();
	void		Clear();
protected:
	void		Serialize(CArchive& ar);
	BOOL		LoadFrom(CXMLElement* pRoot);


};

class CWorldCountry
{
// Construction
public:
	CWorldCountry();
	~CWorldCountry();

// Attributes
public:
	CHAR		m_szID[2];
	CString		m_sName;
public:
	CWorldCity*	m_pCity;
	DWORD		m_nCity;

// Operations
public:
	void		Serialize(CArchive& ar);
	BOOL		LoadFrom(CXMLElement* pRoot);
	void		Clear();

};

class CWorldCity
{
// Attributes
public:
	CString		m_sName;
	CString		m_sState;
	float		m_nLatitude;
	float		m_nLongitude;

// Operations
public:
	void		Serialize(CArchive& ar);
	BOOL		LoadFrom(CXMLElement* pRoot);

};

#endif // !defined(AFX_WORLDGPS_H__763AAF9D_7289_45A1_9026_4E2BF703C0E8__INCLUDED_)
