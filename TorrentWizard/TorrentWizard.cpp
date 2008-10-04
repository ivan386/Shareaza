//
// TorrentWizard.cpp
//
// Copyright (c) Shareaza Development Team, 2003-2007.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "StdAfx.h"
#include "TorrentWizard.h"
#include "WizardSheet.h"
#include "CmdLine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CTorrentWizardApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

CTorrentWizardApp theApp;


/////////////////////////////////////////////////////////////////////////////
// CTorrentWizardApp construction

CTorrentWizardApp::CTorrentWizardApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentWizardApp initialization

BOOL CTorrentWizardApp::InitInstance()
{
	CCommandLineInfoEx cmdInfo; 
	ParseCommandLine(cmdInfo); 

	cmdInfo.GetOption( _T("sourcefile"), m_sCommandLineSourceFile );
	cmdInfo.GetOption( _T("destination"), m_sCommandLineDestination );
	cmdInfo.GetOption( _T("tracker"), m_sCommandLineTracker );

	if( ( m_sCommandLineSourceFile.GetLength() > 0 ) && 
		( m_sCommandLineDestination.GetLength() > 0 ) && 
		( m_sCommandLineTracker.GetLength() > 0 ) )
		m_bCommandLine = TRUE;
	else
		m_bCommandLine = FALSE;

	SetRegistryKey( _T("Shareaza") );
	
	InitEnvironment();
	InitResources();
	
	CWizardSheet::Run();
	
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentWizardApp environment

void CTorrentWizardApp::InitEnvironment()
{
	TCHAR szPath[260];
	DWORD dwSize = 0;

	m_nVersion[0] = m_nVersion[1] = m_nVersion[2] = m_nVersion[3] = 0;

	if ( GetModuleFileName( NULL, szPath, 260 ) )
	{
		m_sPath	= szPath;
		dwSize	= GetFileVersionInfoSize( szPath, &dwSize );
	}

	if ( dwSize > 0 )
	{
		BYTE* pBuffer = new BYTE[ dwSize ];
		
		if ( GetFileVersionInfo( szPath, NULL, dwSize, pBuffer ) )
		{
			VS_FIXEDFILEINFO* pTable;
			
			if ( VerQueryValue( pBuffer, _T("\\"), (VOID**)&pTable, (UINT*)&dwSize ) )
			{
				m_nVersion[0] = (WORD)( pTable->dwFileVersionMS >> 16 );
				m_nVersion[1] = (WORD)( pTable->dwFileVersionMS & 0xFFFF );
				m_nVersion[2] = (WORD)( pTable->dwFileVersionLS >> 16 );
				m_nVersion[3] = (WORD)( pTable->dwFileVersionLS & 0xFFFF );
			}
		}
		
		delete [] pBuffer;
	}

	m_sVersion.Format( _T("%i.%i.%i.%i"),
		m_nVersion[0], m_nVersion[1], m_nVersion[2], m_nVersion[3] );

	OSVERSIONINFO pVersion;
	pVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx( &pVersion );

	m_bNT = ( pVersion.dwPlatformId == VER_PLATFORM_WIN32_NT );
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentWizardApp resources

void CTorrentWizardApp::InitResources()
{
	m_fntNormal.CreateFont( -11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, _T("Tahoma") );
	
	m_fntBold.CreateFont( -11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, _T("Tahoma") );
	
	m_fntLine.CreateFont( -11, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, _T("Tahoma") );
	
	m_fntTiny.CreateFont( -8, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, _T("Tahoma") );

	srand( GetTickCount() );
}

/////////////////////////////////////////////////////////////////////////////
// Utilities

CString SmartSize(QWORD nVolume)
{
	LPCTSTR pszUnit = _T("B");
	CString strVolume;
	
	if ( nVolume < 1024 )
	{
		strVolume.Format( _T("%lu %s"), (DWORD)nVolume, pszUnit );
		return strVolume;
	}
	
	nVolume /= 1024;
	
	if ( nVolume < 1024 )
	{
		strVolume.Format( _T("%lu K%s"), (DWORD)nVolume, pszUnit );
	}
	else if ( nVolume < 1024*1024 )
	{
		strVolume.Format( _T("%.2lf M%s"), (double)(__int64)nVolume / 1024, pszUnit );
	}
	else if ( nVolume < 1024*1024*1024 )
	{
		strVolume.Format( _T("%.3lf G%s"), (double)(__int64)nVolume / (1024*1024), pszUnit );
	}
	else
	{
		strVolume.Format( _T("%.3lf T%s"), (double)(__int64)nVolume / (1024*1024*1024), pszUnit );
	}
	
	return strVolume;
}
