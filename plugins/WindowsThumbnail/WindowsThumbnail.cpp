//
// WindowsThumbnail.cpp : Implementation of WinMain
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

#include "stdafx.h"
#include "resource.h"
#include "WindowsThumbnail.h"

class CWindowsThumbnailModule : public CAtlExeModuleT< CWindowsThumbnailModule >
{
public :
	DECLARE_LIBID(LIBID_WindowsThumbnailLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_WINDOWSTHUMBNAIL, "{3C5A89BA-2446-4310-9B37-5CEE375E08BE}")
};

CWindowsThumbnailModule _AtlModule;

extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int nShowCmd)
{
	SetErrorMode( SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX |
		SEM_NOALIGNMENTFAULTEXCEPT | SEM_FAILCRITICALERRORS );

    return _AtlModule.WinMain( nShowCmd );
}
