// RTFCompactDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CRTFCompactDlg dialog
class CRTFCompactDlg : public CDialog
{
// Construction
public:
	CRTFCompactDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_RTFCOMPACT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOpenFile();
	CString m_sStatus;
	CProgressCtrl m_wndProgress;
	CString m_sPath;
	CButton m_wndStart;
	HANDLE hFile;
	afx_msg void OnClose();
	afx_msg void OnStartCompact();
	BOOL UpdateProgress(DWORD* nReadBytes, DWORD nStep);
private:
	void OnRun(void);
	static UINT	ThreadStart(LPVOID pParam);
};
