/*
This software is released into the public domain.  You are free to 
redistribute and modify without any restrictions with the exception of
the following:

The Zlib library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.
*/
#ifndef SKIN_H
#define SKIN_H

#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include <unzip.h>
#include <errno.h>
#include "resource.h"

#define SKIN_RAZA_HWND	"ShareazaMainWnd"
#define SKIN_SKIN_TITLE	"Shareaza Skin Installer"
#define SKIN_LANG_TITLE "Shareaza Language Installer"
#define VERSION         "1.0.11"

// globals
extern int   skinType;
extern char* szName;
extern char* szVersion;
extern char* szAuthor;
extern char* szXML;
extern char prefix[MAX_PATH];

// extract.c
void ExtractSkinFile(char *szFile);
int GetInstallDirectory();
int GetSkinFileCount(char *szFile);
int ValidateSkin(char *szFile, HWND hwndDlg);
int ExtractSkin(char *szFile, HWND hwndDlg);

// registry.c
void CreateSkinKeys();
void DeleteSkinKeys();

// utils.c
void XMLDecode(char *str);
void LoadManifestInfo(char *buf);
int SetSkinAsDefault();
int MakeDirectory(char *newdir);

// window.c
BOOL CALLBACK ExtractProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
