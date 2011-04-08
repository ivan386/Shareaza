//
// GProfile.h
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

class CXMLElement;
class CG2Packet;


class CGProfile : public CComObject
{
// Construction
public:
	CGProfile();
	virtual ~CGProfile();

	// Gnutella GUID (128 bit)
	CGuarded< Hashes::Guid >	oGUID;
	// BitTorrent GUID (160 bit), first 128 bit are same as oGUID
	CGuarded< Hashes::BtGuid >	oGUIDBT;

	// Create default local profile
	void			Create();
	// Create remote profile from XML
	BOOL			FromXML(const CXMLElement* pXML);
	// Load local profile from file at Shareaza start up
	BOOL			Load();
	// Save local profile to file
	BOOL			Save();
	// Load/Save browsed host profile
	void			Serialize(CArchive& ar, int nVersion /* BROWSER_SER_VERSION */);

	BOOL			IsValid() const;

	CXMLElement*	GetXML(LPCTSTR pszElement = NULL, BOOL bCreate = FALSE);
	CString			GetNick() const;
	CString			GetContact(LPCTSTR pszType) const;
	CString			GetLocation() const;
	DWORD			GetPackedGPS() const;

	CG2Packet*		CreateAvatar() const;

protected:
	CAutoPtr< CXMLElement >	m_pXML;			// Real profile
	CAutoPtr< CXMLElement >	m_pXMLExport;	// Profile for export recreated from m_pXML by GetXML()
	static LPCTSTR	xmlns;

	// Create BitTorrent GUID from Gnutella GUID
	void			CreateBT();

	DECLARE_INTERFACE_MAP()
};

extern CGProfile MyProfile;
