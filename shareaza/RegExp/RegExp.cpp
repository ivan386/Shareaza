// RegExp.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "RegExp.h"
#include "RegExpDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CRegExpApp, CWinApp)
END_MESSAGE_MAP()

CRegExpApp::CRegExpApp()
{
}

CRegExpApp theApp;

BOOL CRegExpApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	SetRegistryKey( _T("Shareaza") );

	CRegExpDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}
