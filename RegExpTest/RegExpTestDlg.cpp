// RegExpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RegExpTest.h"
#include "RegExpTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CRegExpDlg::CRegExpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegExpDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_strInput = AfxGetApp()->GetProfileString( _T("RegExp"), _T("Input") );
	m_strRegExp = AfxGetApp()->GetProfileString( _T("RegExp"), _T("RegExp") );
}

void CRegExpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_INPUT, m_strInput);
	DDX_Text(pDX, IDC_REGEXP, m_strRegExp);
	DDX_Control(pDX, IDC_RESULT, m_oResult);
}

BEGIN_MESSAGE_MAP(CRegExpDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_REGEXP, &CRegExpDlg::OnEnChangeRegexp)
	ON_EN_CHANGE(IDC_INPUT, &CRegExpDlg::OnEnChangeInput)
END_MESSAGE_MAP()

BOOL CRegExpDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	DoItSafe();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRegExpDlg::OnPaint()
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

HCURSOR CRegExpDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRegExpDlg::OnEnChangeRegexp()
{
	DoItSafe();
}

void CRegExpDlg::OnEnChangeInput()
{
	DoItSafe();
}

void CRegExpDlg::DoIt()
{
	try
	{
		const std::wstring exp( (LPCTSTR)m_strRegExp );
		const std::wstring input( (LPCTSTR)m_strInput );

		const regex::rpattern pat( exp, regex::NOCASE, regex::MODE_SAFE );

		regex::match_results res1;
		regex::rpattern::backref_type matches = pat.match( input, res1 );
		if ( matches.matched )
			m_oResult.AddString( _T("Matches.") );
		else
			m_oResult.AddString( _T("No matches.") );

		regex::split_results res2;
		const size_t nCount = pat.split( input, res2 );
		if ( nCount )
		{
			m_oResult.AddString( _T("Splitted strings:") );
			int n = 1;
			const std::vector< std::wstring > str = res2.strings();
			for ( std::vector< std::wstring >::const_iterator i = str.begin();
				i != str.end(); ++i, ++n )
			{
				CString msg;
				msg.Format( _T("%d. %s"), n, (*i).c_str() );
				m_oResult.AddString( msg );
			}
		}
		else
			m_oResult.AddString( _T("No splitted strings.") );
	}
	catch(...)
	{
		m_oResult.AddString( _T("C++ exception. Bad regular expression.") );
	}
}

void CRegExpDlg::DoItSafe()
{
	UpdateData();

	m_oResult.ResetContent();

	__try
	{
		DoIt();
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		m_oResult.AddString( _T("Unhandled exception!") );
	}

	AfxGetApp()->WriteProfileString( _T("RegExp"), _T("Input"), m_strInput );
	AfxGetApp()->WriteProfileString( _T("RegExp"), _T("RegExp"), m_strRegExp );
}
