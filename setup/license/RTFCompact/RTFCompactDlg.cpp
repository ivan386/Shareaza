// RTFCompactDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RTFCompact.h"
#include "RTFCompactDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRTFCompactDlg dialog

CRTFCompactDlg::CRTFCompactDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRTFCompactDlg::IDD, pParent)
	, m_sStatus(_T(""))
	, m_sPath(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRTFCompactDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATUS, m_sStatus);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Text(pDX, IDC_FILE_PATH, m_sPath);
	DDX_Control(pDX, IDC_START, m_wndStart);
}

BEGIN_MESSAGE_MAP(CRTFCompactDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPEN_FILE, OnOpenFile)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_START, OnStartCompact)
END_MESSAGE_MAP()


// CRTFCompactDlg message handlers

BOOL CRTFCompactDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_wndProgress.SetRange( 0, 100 );
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRTFCompactDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRTFCompactDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRTFCompactDlg::OnOpenFile()
{
	// TODO: Add your control notification handler code here
	OPENFILENAME ofn = {};
	char szFile[MAX_PATH] = {};	// buffer for file name
	HWND hwnd = GetSafeHwnd();	// owner window

	// Initialize OPENFILENAME
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Rich Text Format\0*.rtf\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if ( GetOpenFileName( &ofn ) == TRUE )
	{
		hFile = CreateFile( ofn.lpstrFile, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

		if ( hFile != INVALID_HANDLE_VALUE )
		{
			m_sPath.SetString( szFile );
			m_wndStart.ShowWindow( SW_SHOW );
			m_wndStart.EnableWindow( TRUE );
			UpdateData( FALSE );
		}
		else
			MessageBox( "File is used by another process!", "Failure", MB_OK | MB_ICONEXCLAMATION );
	}
}

void CRTFCompactDlg::OnClose()
{
	if ( hFile ) 
        CloseHandle( hFile );

	CDialog::OnClose();
}

void CRTFCompactDlg::OnStartCompact()
{
	ASSERT( hFile != INVALID_HANDLE_VALUE );
	m_wndStart.EnableWindow( FALSE );
	m_wndStart.ShowWindow( SW_HIDE );

	DWORD nThreadId = 0;
	m_sStatus = "Status: Reading file...";
	m_wndProgress.ShowWindow( SW_SHOW );
	m_wndProgress.SetPos( 0 );
	UpdateData( FALSE );

	CWinThread* pThread = AfxBeginThread( ThreadStart, this, THREAD_PRIORITY_NORMAL );
}

BOOL CRTFCompactDlg::UpdateProgress(DWORD* nReadBytes, DWORD nStep)
{
	if ( *nReadBytes + 1 == nStep )
	{
		*nReadBytes = 0;
		m_wndProgress.SetPos( m_wndProgress.GetPos() + 1 );
		UpdateData( FALSE );
		return TRUE;
	}
	else
	{
		*nReadBytes = *nReadBytes + 1;
		return FALSE;
	}
}

UINT CRTFCompactDlg::ThreadStart(LPVOID pParam)
{
	CRTFCompactDlg* pInstance = (CRTFCompactDlg*)pParam;
	pInstance->OnRun();
	return 0;
}

void CRTFCompactDlg::OnRun()
{
	DWORD nSize = GetFileSize( hFile, NULL );
	DWORD nReadBytes = 0, nTotalRead = 0;
	DWORD nStep = nSize / 100;
	CHAR szChar;
	CFile fOutput;
	CString strOutPath;
	strOutPath = m_sPath.Left( m_sPath.ReverseFind( '.' ) );
	strOutPath += ".compacted.rtf";

	fOutput.Open( strOutPath, CFile::modeCreate | CFile::modeWrite );
	if ( fOutput.m_hFile == INVALID_HANDLE_VALUE )
	{
		m_sStatus = "Status: Error creating file...";
		UpdateData( FALSE );
		return;
	}

	m_sStatus = "Status: Compacting file...";
	UpdateData( FALSE );

	CWaitCursor pCursor;
	while ( ReadFile( hFile, (LPVOID)&szChar, 1, &nReadBytes, NULL ) && nReadBytes == 1 )
	{
		UpdateProgress( &nTotalRead, nStep );
		if ( szChar == '\\' )
		{
			if ( ReadFile( hFile, (LPVOID)&szChar, 1, &nReadBytes, NULL ) && nReadBytes == 1 )
			{
				UpdateProgress( &nTotalRead, nStep );

				if ( szChar == '\'' )
				{
					CString strFind, strReplace;

					ReadFile( hFile, (LPVOID)&szChar, 1, &nReadBytes, NULL );
					UpdateProgress( &nTotalRead, nStep );
					strFind.Append( CString(szChar) );

					ReadFile( hFile, (LPVOID)&szChar, 1, &nReadBytes, NULL );
					UpdateProgress( &nTotalRead, nStep );
					strFind.Append( CString(szChar) );

					int nCode = 0;
					_stscanf( strFind.GetBuffer(), "%2x", &nCode );

					CHAR szTemp = CHAR( nCode );
					fOutput.Write( &szTemp, 1 );
				}
				else
				{
					CHAR szTemp = '\\';
					fOutput.Write( &szTemp, 1 );
					fOutput.Write( &szChar, 1 );
				}
			}
		}
		else
			fOutput.Write( &szChar, 1 );
	}

	fOutput.Flush();
	int nGain = (int)( (float)GetFileSize( fOutput.m_hFile, NULL ) / nSize * 100 );

	fOutput.Close();
	CloseHandle( hFile );
	hFile = NULL;

	m_sStatus.Empty();
	m_wndStart.EnableWindow( FALSE );
	m_wndStart.ShowWindow( SW_SHOW );
	m_wndProgress.ShowWindow( SW_HIDE );
	m_wndProgress.SetPos( 0 );
	UpdateData( FALSE );
	
	if ( nGain < 100 )
	{
		CString strMessage;
		DeleteFile( m_sPath );
		MoveFile( strOutPath, m_sPath );
		strMessage.Format( "Success! The final size is %i%% of original size.", nGain );
		MessageBox( strMessage, "Done", MB_OK | MB_ICONINFORMATION );
	}
	else
	{
		DeleteFile( strOutPath );
		MessageBox( "Nothing to compact.", "Done", MB_OK | MB_ICONINFORMATION );
	}

	m_sPath.Empty();
	UpdateData( FALSE );
}
