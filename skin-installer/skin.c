/*
This software is released into the public domain.  You are free to 
redistribute and modify without any restrictions with the exception of
the following:

The Zlib library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.
*/
#include "skin.h"

// globals
int   skinType;
TCHAR* szName;
TCHAR* szVersion;
TCHAR* szAuthor;
TCHAR* szUpdates;
TCHAR* szXML;
TCHAR skins_dir[MAX_PATH];

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR cmdParam, int cmdShow)
{
	int rtn = 0;

	InitCommonControls();

	// globals
	skinType  = 0;
	szName    = NULL;
	szVersion = NULL;
	szAuthor  = NULL;
	szUpdates = NULL;
	szXML     = NULL;

	if ( *cmdParam == 0 )
		MessageBox(NULL,L"Shareaza Skin Installer " VERSION L"\n\nDouble-click on a Shareaza Skin File to use the Shareaza Skin Installer.",L"Shareaza Skin Installer",MB_OK | MB_ICONINFORMATION);
	else if (!_wcsicmp(cmdParam, L"/install") || !_wcsicmp(cmdParam, L"/installsilent"))
		rtn = CreateSkinKeys();
	else if (!_wcsicmp(cmdParam, L"/uninstall") || !_wcsicmp(cmdParam, L"/uninstallsilent"))
		rtn = DeleteSkinKeys();
	else
		ExtractSkinFile(cmdParam);

	// free up memory from globals
	free(szName);
	free(szVersion);
	free(szAuthor);
	free(szUpdates);
	free(szXML);

	return rtn;
}
