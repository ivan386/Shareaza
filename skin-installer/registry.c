/*
This software is released into the public domain.  You are free to 
redistribute and modify without any restrictions with the exception of
the following:

The Zlib library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.
*/
#include "skin.h"

static int CreateHKCRKey(LPCTSTR lpSubKey, LPTSTR lpClass, LPTSTR lpData);
static int DeleteHKCRKey(LPCTSTR lpSubKey, LPTSTR lpClass);

// EXPORT BEGIN
void CreateSkinKeys() {
	char filename[MAX_PATH];
	char fullpath[MAX_PATH];
	char imagpath[MAX_PATH];

	GetModuleFileName(NULL,filename,sizeof(filename));
	_snprintf(fullpath, sizeof(fullpath), "\"%s\" \"%%1\"", filename);
	_snprintf(imagpath, sizeof(imagpath), "%s,0", filename);
	CreateHKCRKey(".sks", "", "Shareaza.SkinFile");
	CreateHKCRKey("Shareaza.SkinFile", "", "Shareaza Skin File");
	CreateHKCRKey("Shareaza.SkinFile\\DefaultIcon", "", imagpath);
	CreateHKCRKey("Shareaza.SkinFile\\shell", "", "open");
	CreateHKCRKey("Shareaza.SkinFile\\shell\\open\\command", "", fullpath);
	CreateHKCRKey("Shareaza.SkinFile\\shell\\skininstall", "", "Install Shareaza Skin");
	CreateHKCRKey("Shareaza.SkinFile\\shell\\skininstall\\command", "", fullpath);
}

void DeleteSkinKeys() {
	DeleteHKCRKey(".sks", "");
	DeleteHKCRKey("Shareaza.SkinFile\\shell\\open\\command", "");
	DeleteHKCRKey("Shareaza.SkinFile\\shell\\open", "");
	DeleteHKCRKey("Shareaza.SkinFile\\shell\\skininstall\\command", "");
	DeleteHKCRKey("Shareaza.SkinFile\\shell\\skininstall", "");
	DeleteHKCRKey("Shareaza.SkinFile\\shell", "");
	DeleteHKCRKey("Shareaza.SkinFile\\DefaultIcon", "");
	DeleteHKCRKey("Shareaza.SkinFile", "");
}
//EXPORT END

static int CreateHKCRKey(LPCTSTR lpSubKey, LPTSTR lpClass, LPTSTR lpData) {
    HKEY keyHandle;
    long rtn =0;
    DWORD lpdw;
    int aLen;

    aLen = strlen(lpData) + 1;
    rtn = RegCreateKeyEx(HKEY_CLASSES_ROOT,lpSubKey,0,NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,0,&keyHandle,&lpdw);
    if(rtn == ERROR_SUCCESS) {
		RegSetValueEx(keyHandle,lpClass,0,REG_SZ,(LPBYTE)lpData,aLen);
        rtn = RegCloseKey(keyHandle);
    }
    return rtn;
}

static int DeleteHKCRKey(LPCTSTR lpSubKey, LPTSTR lpClass) {
    HKEY keyHandle;
    long rtn =0;
    DWORD lpdw;

    rtn = RegCreateKeyEx(HKEY_CLASSES_ROOT,lpSubKey,0,NULL,REG_OPTION_NON_VOLATILE, KEY_READ,0,&keyHandle,&lpdw);
    if(rtn == ERROR_SUCCESS) {
		RegDeleteKey(keyHandle,lpClass);
        rtn = RegCloseKey(keyHandle);
    }
    return rtn;
}
