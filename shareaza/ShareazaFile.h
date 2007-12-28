//
// SharedFile.h
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

#pragma once

class CShareazaFile
{
public:
	CShareazaFile() :
		m_nSize( SIZE_UNKNOWN )
	{
	}
	CString				m_sName;
	QWORD				m_nSize;	// Size if any
									// (there is no size if it equal to 0 or SIZE_UNKNOWN)
	Hashes::Sha1Hash	m_oSHA1;
	Hashes::TigerHash	m_oTiger;
	Hashes::Ed2kHash	m_oED2K;
	Hashes::BtHash		m_oBTH;
	Hashes::Md5Hash		m_oMD5;
	CString				m_sPath;	// Local path if any
	CString				m_sURL;		// Host if any
};
