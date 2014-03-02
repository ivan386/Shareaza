//
// AntiVirus.h
//
// Copyright (c) Shareaza Development Team, 2014.
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

class CAntiVirus
{
public:
	// Enum available anti-viruses and output results to ComboBox
	static void Enum(CComboBox& wndAntiVirus);

	// Free ComboBox memory allocated by Enum() method
	static void Free(CComboBox& wndAntiVirus);

	// Get user selected anti-virus from ComboBox filled by Enum() method
	static void UpdateData(CComboBox& wndAntiVirus);

	// Scan file for viruses (TRUE - ok, FALSE - infected)
	static bool Scan(LPCTSTR szPath);
};

extern CAntiVirus AntiVirus;
