// RTFCompact.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "RTFCompact.h"
#include "RTFCompactDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRTFCompactApp

BEGIN_MESSAGE_MAP(CRTFCompactApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CRTFCompactApp construction

CRTFCompactApp::CRTFCompactApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CRTFCompactApp object

CRTFCompactApp theApp;


// CRTFCompactApp initialization

BOOL CRTFCompactApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	CRTFCompactDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	return FALSE;
}
