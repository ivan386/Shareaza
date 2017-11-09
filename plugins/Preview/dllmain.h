//
// dllmain.h : Declaration of module class.
//
// Copyright (c) Nikolay Raspopov, 2009.
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

#include "resource.h"
#include "Preview.h"


#define REGISTRY_PATH	_T("Software\\Shareaza\\Shareaza Preview Plugin")
#define PLUGIN_PATH		_T("Software\\Shareaza\\Shareaza\\Plugins\\DownloadPreview")


class CPreviewModule : public CAtlDllModuleT< CPreviewModule >
{
public :
	DECLARE_LIBID(LIBID_PreviewLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_PREVIEW, "{78537474-C03F-4ADE-A817-BD7CC0A4D2B4}")
};

extern class CPreviewModule _AtlModule;

void LoadData(CAtlMap< CString, CString >& oData);
void SaveData(const CAtlMap< CString, CString >& oData);
