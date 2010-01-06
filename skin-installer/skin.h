/*
This software is released into the public domain.  You are free to 
redistribute and modify without any restrictions with the exception of
the following:

The Zlib library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.
*/

#pragma once

#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#include <stddef.h>
#include <stdio.h>
#include <commctrl.h>
#include <unzip.h>
#include <errno.h>

#include "resource.h"

#define SKIN_RAZA_HWND	L"ShareazaMainWnd"
#define SKIN_SKIN_TITLE	L"Shareaza Skin Installer"
#define SKIN_LANG_TITLE L"Shareaza Language Installer"
#define VERSION         L"1.0.12.7"

// globals
extern int   skinType;
extern TCHAR* szName;
extern TCHAR* szVersion;
extern TCHAR* szAuthor;
extern TCHAR* szUpdates;
extern TCHAR* szXML;
extern TCHAR skins_dir[MAX_PATH];	// Full path to Skin folder
extern BOOL bRunningOnNT;

// extract.c
void ExtractSkinFile(LPCTSTR pszFile);
void GetInstallDirectory();
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
