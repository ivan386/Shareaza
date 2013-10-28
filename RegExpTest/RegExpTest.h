// RegExp.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

class CRegExpApp : public CWinApp
{
public:
	CRegExpApp();

	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CRegExpApp theApp;
