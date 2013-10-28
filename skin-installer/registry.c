/*
This software is released into the public domain.  You are free to 
redistribute and modify without any restrictions with the exception of
the following:

The Zlib library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.
*/

#include "skin.h"

static LSTATUS CreateHKCRKey(LPCTSTR lpSubKey, LPTSTR lpClass, LPTSTR lpData);
static LSTATUS DeleteHKCRKey(LPCTSTR lpSubKey, LPTSTR lpClass);

LSTATUS CreateSkinKeys()
{
	LSTATUS rtn;
	TCHAR filename[MAX_PATH];
	TCHAR fullpath[MAX_PATH];
	TCHAR imagpath[MAX_PATH];

	GetModuleFileName(NULL,filename,MAX_PATH);
	_snwprintf(fullpath, MAX_PATH, L"\"%s\" \"%%1\"", filename);
	_snwprintf(imagpath, MAX_PATH, L"%s,0", filename);

	rtn = CreateHKCRKey(L"SOFTWARE\\Classes\\.sks", L"", L"Shareaza.SkinFile");
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateHKCRKey(L"SOFTWARE\\Classes\\Shareaza.SkinFile", L"", L"Shareaza Skin File");
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateHKCRKey(L"SOFTWARE\\Classes\\Shareaza.SkinFile\\DefaultIcon", L"", imagpath);
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateHKCRKey(L"SOFTWARE\\Classes\\Shareaza.SkinFile\\shell", L"", L"open");
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateHKCRKey(L"SOFTWARE\\Classes\\Shareaza.SkinFile\\shell\\open\\command", L"", fullpath);
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateHKCRKey(L"SOFTWARE\\Classes\\Shareaza.SkinFile\\shell\\skininstall", L"", L"Install Shareaza Skin");
	if ( rtn != ERROR_SUCCESS ) return rtn;
	rtn = CreateHKCRKey(L"SOFTWARE\\Classes\\Shareaza.SkinFile\\shell\\skininstall\\command", L"", fullpath);
	return rtn;
}

LSTATUS DeleteSkinKeys()
{
	DeleteHKCRKey(L"SOFTWARE\\Classes\\.sks", L"");
	DeleteHKCRKey(L"SOFTWARE\\Classes\\Shareaza.SkinFile\\shell\\open\\command", L"");
	DeleteHKCRKey(L"SOFTWARE\\Classes\\Shareaza.SkinFile\\shell\\open", L"");
	DeleteHKCRKey(L"SOFTWARE\\Classes\\Shareaza.SkinFile\\shell\\skininstall\\command", L"");
	DeleteHKCRKey(L"SOFTWARE\\Classes\\Shareaza.SkinFile\\shell\\skininstall", L"");
	DeleteHKCRKey(L"SOFTWARE\\Classes\\Shareaza.SkinFile\\shell", L"");
	DeleteHKCRKey(L"SOFTWARE\\Classes\\Shareaza.SkinFile\\DefaultIcon", L"");
	DeleteHKCRKey(L"SOFTWARE\\Classes\\Shareaza.SkinFile", L"");
	return ERROR_SUCCESS;
}

static LSTATUS CreateHKCRKey(LPCTSTR lpSubKey, LPTSTR lpClass, LPTSTR lpData)
{
	HKEY keyHandle;
	LSTATUS rtn;
	DWORD aLen;

	rtn = RegCreateKeyEx( HKEY_CURRENT_USER, lpSubKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &keyHandle, NULL );
	if ( rtn == ERROR_SUCCESS )
	{
		aLen = (DWORD)wcslen( lpData ) * sizeof(TCHAR) + 1;
		RegSetValueEx( keyHandle, lpClass, 0, REG_SZ, (LPBYTE)lpData, aLen );
		RegCloseKey( keyHandle );
	}
	return rtn;
}

static LSTATUS DeleteHKCRKey(LPCTSTR lpSubKey, LPTSTR lpClass)
{
	HKEY keyHandle;
	LSTATUS rtn;

	rtn = RegOpenKeyEx( HKEY_CURRENT_USER, lpSubKey, 0, KEY_ALL_ACCESS, &keyHandle );
	if ( rtn == ERROR_SUCCESS )
	{
		RegDeleteKey( keyHandle, lpClass );
		RegCloseKey( keyHandle );
	}
	return rtn;
}
