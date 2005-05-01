//
// VersionChecker.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
// This file is part of SHAREAZA (www.shareaza.com)
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "VersionChecker.h"
#include "Transfer.h"
#include "Network.h"
#include "GProfile.h"
#include "DiscoveryServices.h"
#include "SHA.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CVersionChecker VersionChecker;

#define VERSIONCHECKER_FREQUENCY	7


//////////////////////////////////////////////////////////////////////
// CVersionChecker construction

CVersionChecker::CVersionChecker()
{
	m_bUpgrade		= FALSE;
	m_hThread		= NULL;
	m_hWndNotify	= NULL;
}

CVersionChecker::~CVersionChecker()
{
	Stop();
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker time check

BOOL CVersionChecker::NeedToCheck()
{
	if(!Settings.General.UpdateCheck)
		return FALSE;
	
	DWORD nNextCheck = theApp.GetProfileInt( _T("VersionCheck"), _T("NextCheck"), 0 );
	
	m_sQuote = theApp.GetProfileString( _T("VersionCheck"), _T("Quote"), _T("") );
	
	m_sUpgradePrompt	= theApp.GetProfileString( _T("VersionCheck"), _T("UpgradePrompt"), _T("") );
	m_sUpgradeFile		= theApp.GetProfileString( _T("VersionCheck"), _T("UpgradeFile"), _T("") );
	m_sUpgradeSHA1		= theApp.GetProfileString( _T("VersionCheck"), _T("UpgradeSHA1"), _T("") );
	m_sUpgradeTiger		= theApp.GetProfileString( _T("VersionCheck"), _T("UpgradeTiger"), _T("") );
	m_sUpgradeSize		= theApp.GetProfileString( _T("VersionCheck"), _T("UpgradeSize"), _T("") );
	m_sUpgradeSources	= theApp.GetProfileString( _T("VersionCheck"), _T("UpgradeSources"), _T("") );
	m_bUpgrade			= ( m_sUpgradePrompt.GetLength() > 0 );
	
	if ( ! nNextCheck ) return TRUE;
	if ( nNextCheck == 0xFFFFFFFF ) return FALSE;
	
	return (DWORD)CTime::GetCurrentTime().GetTime() >= nNextCheck;
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker start

BOOL CVersionChecker::Start(HWND hWndNotify)
{
	//return FALSE;
	
	Stop();
	
	m_pRequest.Clear();
	
	m_hWndNotify	= hWndNotify;
	m_bUpgrade		= FALSE;
	
	CWinThread* pThread = AfxBeginThread( ThreadStart, this, THREAD_PRIORITY_IDLE );
	m_hThread = pThread->m_hThread;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker stop

void CVersionChecker::Stop()
{
	m_pRequest.Cancel();
	CHttpRequest::CloseThread( &m_hThread, _T("CVersionChecker") );
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker thread bootstrap

UINT CVersionChecker::ThreadStart(LPVOID pParam)
{
	CVersionChecker* pClass = (CVersionChecker*)pParam;
	pClass->OnRun();
	return 0;
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker thread run

void CVersionChecker::OnRun()
{
	CString strRequest;
	
	BuildRequest( strRequest );
	
	BOOL bRequest = UndertakeRequest( strRequest );
	
	if ( bRequest )
	{
		ProcessResponse();
		m_pResponse.RemoveAll();
	}
	else
	{
		SetNextCheck( VERSIONCHECKER_FREQUENCY );
	}
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker build request

void CVersionChecker::BuildRequest(CString& strRequest)
{
	strRequest += _T("Version=");
	strRequest += theApp.m_sVersion;
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker undertake request

BOOL CVersionChecker::UndertakeRequest(CString& strPost)
{
	m_pRequest.SetURL( _T("http://update.trillinux.org/version/beta.php?") + strPost );
	//Remember to set the update server for final releases.
	
	if ( ! m_pRequest.Execute( FALSE ) ) return FALSE;
	
	int nStatusCode = m_pRequest.GetStatusCode();
	if ( nStatusCode < 200 || nStatusCode > 299 ) return FALSE;
	
	CString strResponse = m_pRequest.GetResponseString();
	CString strHack = theApp.GetProfileString( _T("VersionCheck"), _T("TestResponse"), _T("") );
	if ( strHack.GetLength() ) strResponse = strHack;
	
	for ( strResponse += '&' ; strResponse.GetLength() ; )
	{
		CString strItem	= strResponse.SpanExcluding( _T("&") );
		strResponse		= strResponse.Mid( strItem.GetLength() + 1 );
		
		CString strKey = strItem.SpanExcluding( _T("=") );
		if ( strKey.GetLength() == strItem.GetLength() ) continue;
		strItem = CTransfer::URLDecode( strItem.Mid( strKey.GetLength() + 1 ) );

		strItem.TrimLeft();
		strItem.TrimRight();

		theApp.Message( MSG_SYSTEM, strKey );
		theApp.Message( MSG_SYSTEM, strItem );
		
		m_pResponse.SetAt( strKey, strItem );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker process response

void CVersionChecker::ProcessResponse()
{
	int nDays = VERSIONCHECKER_FREQUENCY;
	CString strValue;
	
	if ( m_pResponse.Lookup( _T("Message"), strValue ) || m_pResponse.Lookup( _T("MessageBox"), strValue ) )
	{
		m_sMessage = strValue;
	}
	
	if ( m_pResponse.Lookup( _T("Quote"), strValue ) )
	{
		m_sQuote = strValue;
		theApp.WriteProfileString( _T("VersionCheck"), _T("Quote"), m_sQuote );
	}
	
	if ( m_pResponse.Lookup( _T("SystemMsg"), strValue ) )
	{
		for ( strValue += '\n' ; strValue.GetLength() ; )
		{
			CString strLine	= strValue.SpanExcluding( _T("\r\n") );
			strValue		= strValue.Mid( strLine.GetLength() + 1 );
			if ( strLine.GetLength() ) theApp.Message( MSG_SYSTEM, strLine );
		}
	}
	
	if ( m_pResponse.Lookup( _T("UpgradePrompt"), strValue ) )
	{
		m_sUpgradePrompt = strValue;
		
		m_pResponse.Lookup( _T("UpgradeFile"), m_sUpgradeFile );
		m_pResponse.Lookup( _T("UpgradeSHA1"), m_sUpgradeSHA1 );
		m_pResponse.Lookup( _T("UpgradeTiger"), m_sUpgradeTiger );
		m_pResponse.Lookup( _T("UpgradeSize"), m_sUpgradeSize );
		m_pResponse.Lookup( _T("UpgradeSources"), m_sUpgradeSources );

		// Old name
		if ( ! m_sUpgradeSHA1.GetLength() )
			m_pResponse.Lookup( _T("UpgradeHash"), m_sUpgradeSHA1 );
		
		theApp.WriteProfileString( _T("VersionCheck"), _T("UpgradePrompt"), m_sUpgradePrompt );
		theApp.WriteProfileString( _T("VersionCheck"), _T("UpgradeFile"), m_sUpgradeFile );
		theApp.WriteProfileString( _T("VersionCheck"), _T("UpgradeSHA1"), m_sUpgradeSHA1 );
		theApp.WriteProfileString( _T("VersionCheck"), _T("UpgradeTiger"), m_sUpgradeTiger);
		theApp.WriteProfileString( _T("VersionCheck"), _T("UpgradeSize"), m_sUpgradeSize );
		theApp.WriteProfileString( _T("VersionCheck"), _T("UpgradeSources"), m_sUpgradeSources );
		
		m_bUpgrade = TRUE;
	}
	else
	{
		theApp.WriteProfileString( _T("VersionCheck"), _T("UpgradePrompt"), _T("") );
		m_bUpgrade = FALSE;
	}
	
	if ( m_pResponse.Lookup( _T("AddDiscovery"), strValue ) )
	{
		strValue.TrimLeft();
		strValue.TrimRight();
		DiscoveryServices.Add( strValue, CDiscoveryService::dsWebCache );
	}
	
	if ( m_pResponse.Lookup( _T("NextCheck"), strValue ) )
	{
		_stscanf( strValue, _T("%lu"), &nDays );
	}
	
	SetNextCheck( nDays );
	
	PostMessage( m_hWndNotify, WM_VERSIONCHECK, 0, 0 );
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker NextCheck update

void CVersionChecker::SetNextCheck(int nDays)
{
	CTimeSpan tPeriod( nDays, 0, 0, 0 );
	CTime tNextCheck = CTime::GetCurrentTime() + tPeriod;
	theApp.WriteProfileInt( _T("VersionCheck"), _T("NextCheck"), (DWORD)tNextCheck.GetTime() );
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker check if a download is an upgrade

BOOL CVersionChecker::CheckUpgradeHash(const SHA1* pHash, LPCTSTR pszPath)
{
	if ( ! m_bUpgrade ) return FALSE;

	if ( CSHA::HashToString( pHash ) != m_sUpgradeSHA1 ) return FALSE;

	if ( _tcsstr( pszPath, _T(".exe") ) == NULL ) return FALSE;

	m_sUpgradePath = pszPath;

	AfxGetMainWnd()->PostMessage( WM_VERSIONCHECK, 2 );

	return TRUE;
}
