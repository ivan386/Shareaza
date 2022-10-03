/*
This software is released into the public domain.  You are free to 
redistribute and modify without any restrictions with the exception of
the following:

The Zlib library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.
*/
#include "skin.h"

// EXPORT BEGIN
INT_PTR CALLBACK ExtractProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HBITMAP hBannerBmp = NULL;
	static TCHAR* szFile = NULL;
	static int maxPos = 1;

    switch (msg) {
        case WM_INITDIALOG:
        {
			HWND hBanner;
			
			EnableWindow(GetDlgItem(hwndDlg,IDC_CONFIG), FALSE);

			szFile = (LPTSTR)lParam;
			maxPos = GetSkinFileCount( szFile );
			if (!maxPos) maxPos = 1;
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM) LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SHAREAZA)));
			hBannerBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_BANNER),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS);
			hBanner = CreateWindow(L"STATIC", NULL, WS_VISIBLE|WS_CHILD|SS_BITMAP, 0, 0, 293, 172, hwndDlg, NULL, GetModuleHandle(NULL), NULL);
			SendMessage(hBanner, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBannerBmp);
			SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0,maxPos));
			SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETSTEP, 1, 0);
			if (!ValidateSkin(szFile, hwndDlg)) {
				SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETPOS, maxPos, 0);
				SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), L"Please verify the skin is a valid Shareaza Skin and try again.");
				EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_INSTALL), FALSE);
			}
			if (skinType==1) {
				SetWindowText(hwndDlg, SKIN_LANG_TITLE);
				SetWindowText(GetDlgItem(hwndDlg, IDC_CONFIG), L"Configure &Language...");
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
				TCHAR buf[MAX_PATH], tbuf[MAX_PATH];
				_snwprintf(buf, MAX_PATH, L"%s %s", szName, szVersion?szVersion:L"");
				_snwprintf(tbuf, MAX_PATH, L"%s - %s", buf, skinType? SKIN_LANG_TITLE : SKIN_SKIN_TITLE);
				SetDlgItemText(hwndDlg, IDC_NAME, buf);
				SetWindowText(hwndDlg, tbuf);
			}
			if (szAuthor) {
				TCHAR buf[MAX_PATH];
				_snwprintf(buf, MAX_PATH, L"By %s", szAuthor);
				SetDlgItemText(hwndDlg, IDC_AUTH, buf);
			}
			if ( szUpdates && wcscmp( szAuthor, szUpdates ) != 0 ) {
				TCHAR buf[MAX_PATH];
				size_t len;
				if (szAuthor) {
					GetDlgItemText(hwndDlg, IDC_AUTH, buf, MAX_PATH );
					len = wcslen(buf);
					_snwprintf(buf + len, MAX_PATH - len, L"; Updated by %s", szUpdates);
				}
				else
					_snwprintf(buf, MAX_PATH, L"Updated by %s", szUpdates);
				SetDlgItemText(hwndDlg, IDC_AUTH, buf);
			}
			SetWindowLongPtr( GetDlgItem(hwndDlg,IDC_WHITERECT), GWL_STYLE, WS_VISIBLE|WS_CHILD|SS_LEFT|SS_OWNERDRAW );
			SetWindowLongPtr( GetDlgItem(hwndDlg,IDC_NAME), GWL_STYLE, WS_VISIBLE|WS_CHILD|SS_LEFT|SS_OWNERDRAW  );
			SetWindowLongPtr( GetDlgItem(hwndDlg,IDC_AUTH), GWL_STYLE, WS_VISIBLE|WS_CHILD|SS_LEFT|SS_OWNERDRAW );
			SetWindowLongPtr( GetDlgItem(hwndDlg,IDC_STATUS), GWL_STYLE, WS_VISIBLE|WS_CHILD|SS_LEFT|SS_OWNERDRAW );
			break;
		}
		case WM_CLOSE:
			EndDialog(hwndDlg, 0);
			break;
		case WM_DESTROY:
			DeleteObject(hBannerBmp);
			break;
		case WM_DRAWITEM:
			if ( (UINT)wParam == IDC_WHITERECT ||
				 (UINT)wParam == IDC_NAME ||
				 (UINT)wParam == IDC_AUTH ||
				 (UINT)wParam == IDC_STATUS )
			{
				TCHAR buf[256];
				LPDRAWITEMSTRUCT lpDrawItemStruct;
				lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
				FillRect(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, (HBRUSH)(DWORD_PTR)(WHITE_BRUSH+1));
				ExtFloodFill(lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem.top,
					lpDrawItemStruct->rcItem.left, RGB(0, 0, 0), FLOODFILLBORDER);
				SetBkMode(lpDrawItemStruct->hDC, TRANSPARENT);
				SetTextColor(lpDrawItemStruct->hDC, RGB(0, 0, 0));
				GetDlgItemText(hwndDlg, (UINT)wParam, buf, 256);
				DrawText(lpDrawItemStruct->hDC, buf, (int)wcslen(buf), &lpDrawItemStruct->rcItem, DT_LEFT);
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
					EnableWindow(GetDlgItem(hwndDlg,IDOK), FALSE);
					GetInstallDirectory ();
					if (!ExtractSkin(szFile, hwndDlg)) { 
						SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETPOS, maxPos, 0);
						SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), L"An error occurred extracting the skin.  Please try again.");
						EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
						break;
					}
					SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETPOS, maxPos, 0);
					if (skinType==1) 
						SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), L"Language successfully installed.");
					else SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), L"Skin successfully installed.");
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
							if (SetSkinAsDefault()) {
								PostMessage(app,WM_COMMAND,32959,0);
								PostMessage(app,WM_COMMAND,32965,0);								
							}
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
