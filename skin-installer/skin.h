/*
This software is released into the public domain.  You are free to 
redistribute and modify without any restrictions with the exception of
the following:

The Zlib library is Copyright © 1995-2002 Jean-loup Gailly and Mark Adler.
The Unzip library is Copyright © 1998-2003 Gilles Vollant.
*/
#ifndef SKIN_H
#define SKIN_H

#include <stddef.h>
#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include <unzip.h>
#include <errno.h>
#include "resource.h"
#include <tchar.h>

#define SKIN_RAZA_HWND	L"ShareazaMainWnd"
#define SKIN_SKIN_TITLE	L"Shareaza Skin Installer"
#define SKIN_LANG_TITLE L"Shareaza Language Installer"
#define VERSION         L"1.0.12"

// globals
extern int   skinType;
extern TCHAR* szName;
extern TCHAR* szVersion;
extern TCHAR* szAuthor;
extern TCHAR* szUpdates;
extern TCHAR* szXML;
extern TCHAR prefix[MAX_PATH];
extern BOOL bRunningOnNT;

// extract.c
void ExtractSkinFile(LPCTSTR pszFile);
int GetInstallDirectory();
int GetSkinFileCount(LPTSTR pszFile);
int ValidateSkin(LPTSTR pszFile, HWND hwndDlg);
int ExtractSkin(LPTSTR pszFile, HWND hwndDlg);
LPCTSTR GetUnicodeString(char* pszString);

// registry.c
void CreateSkinKeys();
void DeleteSkinKeys();

// utils.c
void LoadManifestInfo(char *buf);
int SetSkinAsDefault();
int MakeDirectory(LPCTSTR newdir);

// window.c
INT_PTR CALLBACK ExtractProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
