/*
This software is released into the public domain.  You are free to 
redistribute and modify without any restrictions with the exception of
the following:

The Zlib library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.
*/
#include "skin.h"

// EXPORT BEGIN
BOOL CALLBACK ExtractProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HBITMAP hBannerBmp = NULL;
	static char *szFile = NULL;
	static int maxPos = 1;

    switch (msg) {
        case WM_INITDIALOG:
        {
			HWND hBanner;
			
			EnableWindow(GetDlgItem(hwndDlg,IDC_CONFIG), FALSE);

			szFile = (char*)lParam;
			maxPos = GetSkinFileCount(szFile);
			if (!maxPos) maxPos = 1;
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM) LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SHAREAZA)));
			hBannerBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_BANNER),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS);
			hBanner = CreateWindow("STATIC", NULL, WS_VISIBLE|WS_CHILD|SS_BITMAP, 0, 0, 293, 172, hwndDlg, NULL, GetModuleHandle(NULL), NULL);
			SendMessage(hBanner, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBannerBmp);
			SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0,maxPos));
			SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETSTEP, 1, 0);
			if (!ValidateSkin(szFile, hwndDlg)) {
				SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETPOS, maxPos, 0);
				SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), "Please verify the skin is a valid Shareaza Skin and try again.");
				EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_INSTALL), FALSE);
			}
			if (skinType==1) {
				SetWindowText(hwndDlg, SKIN_LANG_TITLE);
				SetWindowText(GetDlgItem(hwndDlg, IDC_CONFIG), "Configure &Language...");
			}
			else {
				SetWindowText(hwndDlg, SKIN_SKIN_TITLE);
			}
			{	LOGFONT lf;
				HFONT hFont;
				hFont=(HFONT)SendDlgItemMessage(hwndDlg,IDC_NAME,WM_GETFONT,0,0);
				GetObject(hFont,sizeof(lf),&lf);
				lf.lfWeight=FW_BOLD;
				hFont=CreateFontIndirect(&lf);
				SendDlgItemMessage(hwndDlg,IDC_NAME,WM_SETFONT,(WPARAM)hFont,0);
			}
			if (szName) {
				char buf[256], tbuf[256];
				_snprintf(buf, sizeof(buf), "%s %s", szName, szVersion?szVersion:"");
				_snprintf(tbuf, sizeof(tbuf), "%s - %s", buf, skinType? SKIN_LANG_TITLE : SKIN_SKIN_TITLE);
				SetDlgItemText(hwndDlg, IDC_NAME, buf);
				SetWindowText(hwndDlg, tbuf);
			}
			if (szAuthor) {
				char buf[256];
				_snprintf(buf, sizeof(buf), "By %s", szAuthor);
				SetDlgItemText(hwndDlg, IDC_AUTH, buf);
			}
			break;
		}
		case WM_CLOSE:
			EndDialog(hwndDlg, 0);
			break;
		case WM_DESTROY:
			DeleteObject(hBannerBmp);
			break;
		case WM_CTLCOLORSTATIC:
			if ((HWND) lParam == GetDlgItem(hwndDlg, IDC_WHITERECT) 
					|| (HWND) lParam == GetDlgItem(hwndDlg, IDC_NAME)
					|| (HWND) lParam == GetDlgItem(hwndDlg, IDC_AUTH)
					|| (HWND) lParam == GetDlgItem(hwndDlg, IDC_STATUS)) {
				SetBkColor((HDC) wParam, RGB(255, 255, 255));
				return (BOOL) GetStockObject(WHITE_BRUSH);
			}
			break;
		case WM_COMMAND:
		{
			switch(LOWORD(wParam)) {
				case IDOK:
					EndDialog(hwndDlg, 0);
					break;
				case IDC_INSTALL:
				{
					char modDir[MAX_PATH], *tmp;
					
					EnableWindow(GetDlgItem(hwndDlg,IDOK), FALSE);
					GetModuleFileName(NULL,modDir,sizeof(modDir));
					tmp=strrchr(modDir,'\\');
					if (tmp) *tmp=0;
					SetCurrentDirectory(modDir);
					if (!GetInstallDirectory()) {
					    SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETPOS, maxPos, 0);
					    SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), "Could not determine install directory. Please re-install Shareaza");
					    EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
					    break;
					}
					if (!ExtractSkin(szFile, hwndDlg)) { 
						SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETPOS, maxPos, 0);
						SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), "An error occured extracting the skin.  Please try again.");
						EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
						break;
					}
					SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETPOS, maxPos, 0);
					if (skinType==1) 
						SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), "Language successfully installed.");
					else SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), "Skin successfully installed.");
					EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_INSTALL), FALSE);
					if (FindWindow(SKIN_RAZA_HWND,NULL)) EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIG), TRUE);
					break;
				}
				case IDC_CONFIG:
				{
					HWND app = FindWindow(SKIN_RAZA_HWND,NULL);
					if (app) {
						if (!IsZoomed(app)) {
							PostMessage(app,WM_SYSCOMMAND,SC_RESTORE,0);
						}
						PostMessage(app,WM_COMMAND,32879,0);
						SetFocus(app);
						if (skinType==1) {
							PostMessage(app,WM_COMMAND,32974,0);
						}
						else {
							if (SetSkinAsDefault()) PostMessage(app,WM_COMMAND,32965,0);
						}
						EndDialog(hwndDlg, 0);
					}
					break;
				}
			}
			break;
		}
	}
	return FALSE;
}
// EXPORT END
