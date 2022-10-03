//
// SWFReader.cpp : Implementation of CSWFReader
//
// Copyright (c) Nikolay Raspopov, 2005-2014.
// This file is part of SHAREAZA (shareaza.sourceforge.net)
//
// GFL Library, GFL SDK and XnView
// Copyright (c) 1991-2004 Pierre-E Gougelet
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
#include "SWFReader.h"

// convert BGR down-up bitmap to RGB up-down bitmap
inline void BGRA_DU2RGBA_UD (char* dest, int width, int height, int components) throw()
{
	register int line_size = ((width * components) + 3) & (-4);
	register char* up = dest;
	register char* down = dest + line_size * (height - 1);
	switch (components) {
		case 3:
			for (register int j = height / 2; j; --j, up += line_size, down -= line_size) {
				for (register int i = 0; i < width * 3; i += 3) {
					register char c1 = up [i];
					register char c2 = up [i + 1];
					register char c3 = up [i + 2];
					up [i]     = down [i + 2];
					up [i + 1] = down [i + 1];
					up [i + 2] = down [i];
					down [i]     = c3;
					down [i + 1] = c2;
					down [i + 2] = c1;
				}
			}
		break;
		case 4:
			for (register int j = height / 2; j; --j, up += line_size, down -= line_size) {
				for (register int i = 0; i < width * 4; i += 4) {
					register char c1 = up [i];
					register char c2 = up [i + 1];
					register char c3 = up [i + 2];
					register char c4 = up [i + 3];
					up [i]     = down [i + 2];
					up [i + 1] = down [i + 1];
					up [i + 2] = down [i];
					up [i + 3] = down [i + 3];
					down [i]     = c3;
					down [i + 1] = c2;
					down [i + 2] = c1;
					down [i + 3] = c4;
				}
			}
		break;
	}
}

#define DECLARE_MAINFRAME_WND_CLASS(WndClassName, style, bkgnd, menuid) \
	static ATL::CWndClassInfo& GetWndClassInfo() { \
	static ATL::CWndClassInfo wc = { \
{ sizeof(WNDCLASSEX), style, StartWindowProc, \
	0, 0, NULL, NULL, NULL, (HBRUSH)(bkgnd + 1), menuid, WndClassName, NULL }, \
	NULL, NULL, IDC_ARROW, TRUE, 0, _T("") \
	}; \
	return wc; \
	}

class CMainWindow : public CWindowImpl<CMainWindow, CWindow, CFrameWinTraits>
{
public:
	DECLARE_MAINFRAME_WND_CLASS(_T("CSWFReaderWindow"), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_WINDOW, 0)

	BEGIN_MSG_MAP(CMainWindow)
	END_MSG_MAP()
};

HRESULT CreateSWF (HWND hWnd, IUnknown** ppControl)
{
	HRESULT hr;
	__try {
		hr = AtlAxCreateControlEx (L"ShockwaveFlash.ShockwaveFlash",
			hWnd, NULL, NULL, ppControl);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		hr = E_FAIL;
	}
	return hr;
}

unsigned WINAPI LoadSWF (void* filename)
{
	DWORD dwBegin = GetTickCount();
	HRESULT hr = CoInitializeEx (NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED (hr)) {
		if (AtlAxWinInit ()) {
			RECT rc = {-1 - cx, -1 - cy, -1, -1};
			CMainWindow wndMain;
			if (wndMain.Create (NULL, &rc, NULL, WS_POPUP)) {
				CComPtr <IUnknown> pControl;
				hr = CreateSWF (wndMain.m_hWnd, &pControl);
				if (SUCCEEDED (hr)) {
					hr = OleRun (pControl);
					if (SUCCEEDED (hr)) {
						CComPtr <IDispatch> pIDispatch;
						hr = pControl->QueryInterface (IID_IDispatch, (void**) &pIDispatch);
						if (SUCCEEDED (hr)) {
							// void LoadMovie (dwLayer, bstrFilename);
							VARIANT varArg [2];
							VariantInit (&varArg [1]);
							varArg [1].vt = VT_I4;
							varArg [1].lVal = 0;
							VariantInit (&varArg [0]);
							varArg [0].vt = VT_BSTR;
							varArg [0].bstrVal = SysAllocString ((LPCWSTR) filename);
							DISPPARAMS dispparams;
							memset(&dispparams, 0, sizeof dispparams);
							dispparams.rgvarg = varArg;
							dispparams.cArgs = 2;
							VARIANT varResult;
							VariantInit (&varResult);
							UINT nArgErr = (UINT) -1;
							hr = pIDispatch->Invoke (0x8e, IID_NULL, 0, DISPATCH_METHOD,
								&dispparams, &varResult, NULL, &nArgErr);
							for (LONG state = -1; SUCCEEDED (hr) && state != 4 &&
								( GetTickCount() - dwBegin ) < 20000; ) {
								// long get_ReadyState ()
								dispparams.cArgs = 0;
								VariantInit (&varResult);
								hr = pIDispatch->Invoke (DISPID_READYSTATE, IID_NULL, 0, DISPATCH_PROPERTYGET,
									&dispparams, &varResult, NULL, NULL);
								if (varResult.lVal != state) {
									state = varResult.lVal;
									switch (state) {
										case 0:
											ATLTRACE( "Loading\n" );
											break;
										case 1:
											ATLTRACE( "Uninitialized\n" );
											break;
										case 2:
											ATLTRACE( "Loaded\n" );
											break;
										case 3:
											ATLTRACE( "Interactive\n" );
											break;
										case 4:
											// long get_TotalFrames ()
											dispparams.cArgs = 0;
											VariantInit (&varResult);
											pIDispatch->Invoke (0x7c, IID_NULL, 0, DISPATCH_PROPERTYGET,
												&dispparams, &varResult, NULL, NULL);
											ATLTRACE( "Complete. Frames: %d\n", varResult.lVal);
											// void GotoFrame (dwFrameNumber)
											VariantInit (&varArg [0]);
											varArg [0].vt = VT_I4;
											varArg [0].lVal = min (varResult.lVal, 2);
											dispparams.cArgs = 1;
											VariantInit (&varResult);
											nArgErr = (UINT) -1;
											hr = pIDispatch->Invoke (0x7f, IID_NULL, 0, DISPATCH_METHOD,
												&dispparams, &varResult, NULL, &nArgErr);
											break;
										default:
											ATLTRACE( "Unknown state (%d)\n", state);
									}
								}
								Sleep (0);
							}
							hr = E_OUTOFMEMORY;
							_Data = new (std::nothrow) MY_DATA;
							if (_Data) {
								_Data->hBitmap = NULL;
								ZeroMemory (&_Data->bmiHeader, sizeof (_Data->bmiHeader));
								_Data->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
								HDC hDC = GetDC (NULL);
								HDC hMemDC = CreateCompatibleDC (hDC);
								_Data->hBitmap = CreateCompatibleBitmap (hDC, cx, cy);
								HBITMAP hOldBitmap = (HBITMAP) SelectObject (hMemDC, _Data->hBitmap);
								RECT rcMem = {0, 0, cx, cy};
								hr = OleDraw (pControl, DVASPECT_CONTENT, hMemDC, &rcMem);
								SelectObject (hMemDC, hOldBitmap);
								DeleteDC (hMemDC);
								GetDIBits (hDC, _Data->hBitmap, 0, 0, NULL,
									(BITMAPINFO*) &_Data->bmiHeader, DIB_RGB_COLORS);
								_Data->bmiHeader.biCompression = BI_RGB;
								_Data->bmiHeader.biXPelsPerMeter = 72;
								_Data->bmiHeader.biYPelsPerMeter = 72;
								ReleaseDC (NULL, hDC);
							}
						}
					}
				}
				wndMain.DestroyWindow ();
			}
			AtlAxWinTerm ();
		}
		CoUninitialize ();
	}
	if (FAILED (hr)) {
		ATLTRACE( "LoadSWF failed: 0x%08x\n", hr);
		if (_Data) {
			if (_Data->hBitmap)
				DeleteObject (_Data->hBitmap);
			delete _Data;
			_Data = NULL;
		}
	}
	return 0;
}

STDMETHODIMP CSWFReader::LoadFromFile (
	/* [in] */ BSTR sFile,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [out] */ SAFEARRAY** ppImage )
{
	ATLTRACE( "SWFPlugin::LoadFromFile (\"%s\", 0x%08x, 0x%08x)\n", (LPCSTR)CW2A( (LPCWSTR)sFile ), pParams, ppImage);

	if (!pParams || !ppImage)
		return E_POINTER;

	*ppImage = NULL;

	EnterCriticalSection (&_CS);

	HRESULT hr = E_FAIL;

	// Changing threading model
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, LoadSWF, (LPVOID)sFile, 0, NULL);
	WaitForSingleObject (hThread, INFINITE);
	CloseHandle (hThread);

	if (_Data) {
		switch (_Data->bmiHeader.biBitCount) {
			case 24:
				pParams->nComponents = 3;
				break;
			case 32:
				pParams->nComponents = 4;
				break;
			default:
				pParams->nComponents = 0;
				hr = E_UNEXPECTED;
		}
		if (pParams->nComponents) {
			pParams->nWidth = _Data->bmiHeader.biWidth;
			pParams->nHeight = _Data->bmiHeader.biHeight;
			ULONG line_size = ((pParams->nWidth * pParams->nComponents) + 3) & (-4);
			ULONG total_size = line_size * pParams->nHeight;
			hr = E_OUTOFMEMORY;
			*ppImage = SafeArrayCreateVector (VT_UI1, 0, total_size);
			if (*ppImage) {
				char* pDest = NULL;
				hr = SafeArrayAccessData (*ppImage, (void**) &pDest);
				if (SUCCEEDED (hr)) {
					HDC hDC = GetDC (NULL);
					GetDIBits (hDC, _Data->hBitmap, 0, pParams->nHeight, pDest,
						(BITMAPINFO*) &_Data->bmiHeader, DIB_RGB_COLORS);
					ReleaseDC (NULL, hDC);
					BGRA_DU2RGBA_UD (pDest, pParams->nWidth, pParams->nHeight,
						pParams->nComponents);
					SafeArrayUnaccessData (*ppImage);
				}
			}
		}
		if (_Data->hBitmap)
			DeleteObject (_Data->hBitmap);
		delete _Data;
		_Data = NULL;
	}

	LeaveCriticalSection (&_CS);

	if (FAILED (hr) && *ppImage) {
		SafeArrayDestroy (*ppImage);
		*ppImage = NULL;
	}
	return hr;
}

STDMETHODIMP CSWFReader::LoadFromMemory (
	/* [in] */ BSTR /* sType */,
	/* [in] */ SAFEARRAY* /* pMemory */,
	/* [in,out] */ IMAGESERVICEDATA* /* pParams */,
	/* [out] */ SAFEARRAY** /* ppImage */)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSWFReader::SaveToFile (
	/* [in] */ BSTR /* sFile */,
	/* [in,out] */ IMAGESERVICEDATA* /* pParams */,
	/* [in] */ SAFEARRAY* /* pImage */)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSWFReader::SaveToMemory (
	/* [in] */ BSTR /* sType */,
	/* [out] */ SAFEARRAY** /* ppMemory */,
	/* [in,out] */ IMAGESERVICEDATA* /* pParams */,
	/* [in] */ SAFEARRAY* /* pImage */)
{
	return E_NOTIMPL;
}
