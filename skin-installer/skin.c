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
char* szName;
char* szVersion;
char* szAuthor;
char* szXML;
char  prefix[MAX_PATH];

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, char *cmdParam, int cmdShow) {
	InitCommonControls();

	// globals
	skinType  = 0;
	szName    = NULL;
	szVersion = NULL;
	szAuthor  = NULL;
	szXML     = NULL;
    
	if (strlen(cmdParam)==0) MessageBox(NULL,"Shareaza Skin Installer " VERSION "\n\nDouble-click on a Shareaza Skin File to use the Shareaza Skin Installer.","Shareaza Skin Installer",MB_OK | MB_ICONINFORMATION);
	else if (!strcmp(cmdParam, "/install") || !strcmp(cmdParam, "/installsilent")) CreateSkinKeys();
	else if (!strcmp(cmdParam, "/uninstall") || !strcmp(cmdParam, "/uninstallsilent")) DeleteSkinKeys();
	else ExtractSkinFile(cmdParam);
	
	// free up memory from globals
	if (szName) free(szName);
	if (szVersion) free(szVersion);
	if (szAuthor) free(szAuthor);
	if (szXML) free(szXML);
	return 0;
}
