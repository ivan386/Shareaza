//
// PageFileGeneral.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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
#include "Library.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "ShellIcons.h"
#include "PageFileGeneral.h"

#include "SHA.h"
#include "TigerTree.h"
#include "MD5.h"
#include "ED2K.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CFileGeneralPage, CFilePropertiesPage)

BEGIN_MESSAGE_MAP(CFileGeneralPage, CFilePropertiesPage)
	//{{AFX_MSG_MAP(CFileGeneralPage)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileGeneralPage property page

CFileGeneralPage::CFileGeneralPage() : CFilePropertiesPage( CFileGeneralPage::IDD )
{
	//{{AFX_DATA_INIT(CFileGeneralPage)
	m_sSHA1 = _T("");
	m_sTiger = _T("");
	m_sType = _T("");
	m_sSize = _T("");
	m_sPath = _T("");
	m_sModified = _T("");
	m_sIndex = _T("");
	m_sMD5 = _T("");
	m_sED2K = _T("");
	//}}AFX_DATA_INIT
}

CFileGeneralPage::~CFileGeneralPage()
{
}

void CFileGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CFilePropertiesPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileGeneralPage)
	DDX_Text(pDX, IDC_FILE_SHA1, m_sSHA1);
	DDX_Text(pDX, IDC_FILE_TIGER, m_sTiger);
	DDX_Text(pDX, IDC_FILE_TYPE, m_sType);
	DDX_Text(pDX, IDC_FILE_SIZE, m_sSize);
	DDX_Text(pDX, IDC_FILE_PATH, m_sPath);
	DDX_Text(pDX, IDC_FILE_MODIFIED, m_sModified);
	DDX_Text(pDX, IDC_FILE_INDEX, m_sIndex);
	DDX_Text(pDX, IDC_FILE_MD5, m_sMD5);
	DDX_Text(pDX, IDC_FILE_ED2K, m_sED2K);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFileGeneralPage message handlers

BOOL CFileGeneralPage::OnInitDialog() 
{
	CFilePropertiesPage::OnInitDialog();
	
	CLibraryFile* pFile = GetFile();
	if ( ! pFile ) return TRUE;

	if ( pFile->m_pFolder != NULL ) m_sPath = pFile->m_pFolder->m_sPath;
	m_sSize = Settings.SmartVolume( pFile->GetSize(), FALSE );
	m_sType = ShellIcons.GetTypeString( pFile->m_sName );	
	m_sIndex.Format( _T("# %lu"), pFile->m_nIndex );

	if ( pFile->m_oSHA1.IsValid() )
	{
		m_sSHA1 = _T("sha1:") + pFile->m_oSHA1.ToString();
	}
	else
	{
		LoadString(m_sSHA1, IDS_GENERAL_NOURNAVAILABLE );
	}
	
	if ( pFile->m_oTiger.IsValid() )
	{
		m_sTiger = _T("tree:tiger/:") + pFile->m_oTiger.ToString();
	}
	else
	{
		m_sTiger.Empty();
	}
	
	if ( pFile->m_oMD5.IsValid() )
	{
		m_sMD5 = _T("md5:") + pFile->m_oMD5.ToString();
	}
	else
	{
		m_sMD5.Empty();
	}
	
	if ( pFile->m_oED2K.IsValid() )
	{
		m_sED2K = _T("ed2k:") + pFile->m_oED2K.ToString();
	}
	else
	{
		m_sED2K.Empty();
	}
	
	CString strDate, strTime;
	SYSTEMTIME pTime;
	
	FileTimeToSystemTime( &pFile->m_pTime, &pTime );
	SystemTimeToTzSpecificLocalTime( NULL, &pTime, &pTime );
	
	GetDateFormat( LOCALE_USER_DEFAULT, DATE_LONGDATE, &pTime, NULL, strDate.GetBuffer( 64 ), 64 );
	GetTimeFormat( LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &pTime, NULL, strTime.GetBuffer( 64 ), 64 );
	strDate.ReleaseBuffer(); strTime.ReleaseBuffer();
	
	m_sModified = strDate + _T(", ") + strTime;
	
	Library.Unlock();
	UpdateData( FALSE );
	
	return TRUE;
}

void CFileGeneralPage::OnOK() 
{
	UpdateData();
	
	// Nothing to update now
	
	CFilePropertiesPage::OnOK();
}

