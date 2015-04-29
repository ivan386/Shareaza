//
// PageDownloadEdit.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
// This file is part of SHAREAZA (shareaza.sourceforge.net)
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
#include "DlgDownloadSheet.h"
#include "PageDownloadEdit.h"

#include "Download.h"
#include "Downloads.h"
#include "DownloadTask.h"
#include "Transfers.h"
#include "FragmentedFile.h"
#include "CoolInterface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDownloadEditPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CDownloadEditPage, CPropertyPageAdv)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////////
// CDownloadEditPage construction

CDownloadEditPage::CDownloadEditPage() :
	CPropertyPageAdv( CDownloadEditPage::IDD ),
	m_bSHA1Trusted( FALSE ),
	m_bTigerTrusted( FALSE ),
	m_bED2KTrusted( FALSE ),
	m_bMD5Trusted( FALSE ),
	m_bBTHTrusted( FALSE )
{
}

CDownloadEditPage::~CDownloadEditPage()
{
}

void CDownloadEditPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NAME, m_sName);
	DDX_Text(pDX, IDC_DISKNAME, m_sDiskName);
	DDX_Text(pDX, IDC_FILESIZE, m_sFileSize);
	DDX_Text(pDX, IDC_URN_SHA1, m_sSHA1);
	DDX_Text(pDX, IDC_URN_TIGER, m_sTiger);
	DDX_Text(pDX, IDC_URN_ED2K, m_sED2K);
	DDX_Text(pDX, IDC_URN_MD5, m_sMD5);
	DDX_Text(pDX, IDC_URN_BTH, m_sBTH);
	DDX_Check(pDX, IDC_TRUST_SHA1, m_bSHA1Trusted);
	DDX_Check(pDX, IDC_TRUST_TIGER, m_bTigerTrusted);
	DDX_Check(pDX, IDC_TRUST_ED2K, m_bED2KTrusted);
	DDX_Check(pDX, IDC_TRUST_MD5, m_bMD5Trusted);
	DDX_Check(pDX, IDC_TRUST_BTH, m_bBTHTrusted);
}

//////////////////////////////////////////////////////////////////////////////
// CDownloadEditPage message handlers

BOOL CDownloadEditPage::OnInitDialog()
{
	if ( ! CPropertyPageAdv::OnInitDialog() )
		return FALSE;

	ASSUME_LOCK( Transfers.m_pSection );

	CDownload* pDownload = ((CDownloadSheet*)GetParent())->GetDownload();
	ASSERT( pDownload );

	m_sName = pDownload->m_sName;
	m_sDiskName = pDownload->m_sPath;
	if ( pDownload->m_nSize != SIZE_UNKNOWN )
		m_sFileSize.Format( _T("%I64u"), pDownload->m_nSize );

	if ( pDownload->m_oSHA1 )
		m_sSHA1 = pDownload->m_oSHA1.toString();
	if ( pDownload->m_oTiger )
		m_sTiger = pDownload->m_oTiger.toString();
	if ( pDownload->m_oED2K )
        m_sED2K = pDownload->m_oED2K.toString();
	if ( pDownload->m_oMD5 )
        m_sMD5 = pDownload->m_oMD5.toString();
	if ( pDownload->m_oBTH )
        m_sBTH = pDownload->m_oBTH.toString();

	m_bSHA1Trusted	=	pDownload->m_bSHA1Trusted;
	m_bTigerTrusted	=	pDownload->m_bTigerTrusted;
	m_bED2KTrusted	=	pDownload->m_bED2KTrusted;
	m_bMD5Trusted	=	pDownload->m_bMD5Trusted;
	m_bBTHTrusted	=	pDownload->m_bBTHTrusted;

	UpdateData( FALSE );

	return TRUE;
}

BOOL CDownloadEditPage::OnApply()
{
	if ( ! UpdateData() )
		return FALSE;

	Hashes::Sha1Hash oSHA1;
	Hashes::TigerHash oTiger;
	Hashes::Ed2kHash oED2K;
	Hashes::Md5Hash oMD5;
	Hashes::BtHash oBTH;

    oSHA1.fromString( m_sSHA1 ) || oSHA1.fromString< Hashes::base16Encoding >( m_sSHA1 );
    oTiger.fromString( m_sTiger ) || oTiger.fromString< Hashes::base16Encoding >( m_sTiger );
    oED2K.fromString( m_sED2K );
    oMD5.fromString( m_sMD5 );
    oBTH.fromString( m_sBTH ) || oBTH.fromString< Hashes::base16Encoding >( m_sBTH );
	
	if ( m_sSHA1.GetLength() > 0 && !oSHA1 )
	{
		CString strMessage;
		strMessage.Format( LoadString( IDS_DOWNLOAD_EDIT_BAD_HASH ), _T("SHA1") );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		GetDlgItem( IDC_URN_SHA1 )->SetFocus();
		return FALSE;
	}
	else if ( m_sTiger.GetLength() > 0 && !oTiger )
	{
		CString strMessage;
		strMessage.Format( LoadString( IDS_DOWNLOAD_EDIT_BAD_HASH ), _T("Tiger-Root") );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		GetDlgItem( IDC_URN_TIGER )->SetFocus();
		return FALSE;
	}
	else if ( m_sED2K.GetLength() > 0 && !oED2K )
	{
		CString strMessage;
		strMessage.Format( LoadString( IDS_DOWNLOAD_EDIT_BAD_HASH ), _T("ED2K") );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		GetDlgItem( IDC_URN_ED2K )->SetFocus();
		return FALSE;
	}
	else if ( m_sMD5.GetLength() > 0 && !oMD5 )
	{
		CString strMessage;
		strMessage.Format( LoadString( IDS_DOWNLOAD_EDIT_BAD_HASH ), _T("MD5") );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		GetDlgItem( IDC_URN_MD5 )->SetFocus();
		return FALSE;
	}
	else if ( m_sBTH.GetLength() > 0 && !oBTH )
	{
		CString strMessage;
		strMessage.Format( LoadString( IDS_DOWNLOAD_EDIT_BAD_HASH ), _T("BitTorrent") );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		GetDlgItem( IDC_URN_BTH )->SetFocus();
		return FALSE;
	}

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	CDownloadSheet* pSheet = (CDownloadSheet*)GetParent();
	CDownload* pDownload = pSheet->GetDownload();
	if ( ! pDownload )
		// Invalid download
		return CPropertyPageAdv::OnApply();

	bool bNeedUpdate = false;
	bool bCriticalChange = false;

	bNeedUpdate	= pDownload->m_bSHA1Trusted ^ ( m_bSHA1Trusted == TRUE );
	bNeedUpdate	|= pDownload->m_bTigerTrusted ^ ( m_bTigerTrusted == TRUE );
	bNeedUpdate	|= pDownload->m_bED2KTrusted ^ ( m_bED2KTrusted == TRUE );
	bNeedUpdate	|= pDownload->m_bMD5Trusted ^ ( m_bMD5Trusted == TRUE );
	bNeedUpdate	|= pDownload->m_bBTHTrusted ^ ( m_bBTHTrusted == TRUE );

	if ( pDownload->m_sName != m_sName )
	{
		pLock.Unlock();
		if ( AfxMessageBox( IDS_DOWNLOAD_EDIT_RENAME, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		pDownload = pSheet->GetDownload();
		if ( ! pDownload ) return CPropertyPageAdv::OnApply();
		pDownload->Rename( m_sName );
		bNeedUpdate = true;
	}

	QWORD nNewSize = 0;
    if ( _stscanf( m_sFileSize, _T("%I64u"), &nNewSize ) == 1 && nNewSize != pDownload->m_nSize )
	{
		pLock.Unlock();
		if ( AfxMessageBox( IDS_DOWNLOAD_EDIT_CHANGE_SIZE, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		pDownload = pSheet->GetDownload();
		if ( ! pDownload ) return CPropertyPageAdv::OnApply();
		pDownload->Resize( nNewSize );
		pDownload->CloseTransfers();
		pDownload->ClearVerification();
		bCriticalChange = true;
	}
	
	if ( pDownload->m_oSHA1.isValid() != oSHA1.isValid()
		|| validAndUnequal( pDownload->m_oSHA1, oSHA1 ) )
	{
		pLock.Unlock();
		CString strMessage;
		strMessage.Format( LoadString( IDS_DOWNLOAD_EDIT_CHANGE_HASH ), _T("SHA1"), _T("SHA1") );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		pDownload = pSheet->GetDownload();
		if ( ! pDownload ) return CPropertyPageAdv::OnApply();
		bCriticalChange = bCriticalChange || validAndUnequal( pDownload->m_oSHA1, oSHA1 );
		pDownload->m_oSHA1 = oSHA1;
		if ( oSHA1 ) pDownload->m_bSHA1Trusted = true;
		pDownload->CloseTransfers();
		pDownload->ClearVerification();
	}
	
	if ( pDownload->m_oTiger.isValid() != oTiger.isValid()
		|| validAndUnequal( pDownload->m_oTiger, oTiger ) )
	{
		pLock.Unlock();
		CString strMessage;
		strMessage.Format( LoadString( IDS_DOWNLOAD_EDIT_CHANGE_HASH ), _T("Tiger-Root"), _T("Tiger-Root") );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		pDownload = pSheet->GetDownload();
		if ( ! pDownload ) return CPropertyPageAdv::OnApply();
		bCriticalChange = bCriticalChange || validAndUnequal( pDownload->m_oTiger, oTiger );
		pDownload->m_oTiger = oTiger;
		if ( oTiger ) pDownload->m_bTigerTrusted = true;
		pDownload->CloseTransfers();
		pDownload->ClearVerification();
	}
	
	if ( pDownload->m_oED2K.isValid() != oED2K.isValid()
		|| validAndUnequal( pDownload->m_oED2K, oED2K ) )
	{
		pLock.Unlock();
		CString strMessage;
		strMessage.Format( LoadString( IDS_DOWNLOAD_EDIT_CHANGE_HASH ), _T("ED2K"), _T("ED2K") );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		pDownload = pSheet->GetDownload();
		if ( ! pDownload ) return CPropertyPageAdv::OnApply();
		bCriticalChange = bCriticalChange || validAndUnequal( pDownload->m_oED2K, oED2K );
		pDownload->m_oED2K = oED2K;
		if ( oED2K ) pDownload->m_bED2KTrusted = true;
		pDownload->CloseTransfers();
		pDownload->ClearVerification();
	}

	if ( pDownload->m_oMD5.isValid() != oMD5.isValid()
		|| validAndUnequal( pDownload->m_oMD5, oMD5 ) )
	{
		pLock.Unlock();
		CString strMessage;
		strMessage.Format( LoadString( IDS_DOWNLOAD_EDIT_CHANGE_HASH ), _T("MD5"), _T("MD5") );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		pDownload = pSheet->GetDownload();
		if ( ! pDownload ) return CPropertyPageAdv::OnApply();
		bCriticalChange = bCriticalChange || validAndUnequal( pDownload->m_oMD5, oMD5 );
		pDownload->m_oMD5 = oMD5;
		if ( oMD5 ) pDownload->m_bMD5Trusted = true;
		pDownload->CloseTransfers();
		pDownload->ClearVerification();
	}

	BOOL bNewBTH = FALSE;
	if ( pDownload->m_oBTH.isValid() != oBTH.isValid()
		|| validAndUnequal( pDownload->m_oBTH, oBTH ) )
	{
		pLock.Unlock();
		CString strMessage;
		strMessage.Format( LoadString( IDS_DOWNLOAD_EDIT_CHANGE_HASH ), _T("BitTorrent"), _T("BitTorrent") );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		pLock.Lock();
		pDownload = pSheet->GetDownload();
		if ( ! pDownload ) return CPropertyPageAdv::OnApply();
		bCriticalChange = bCriticalChange || validAndUnequal( pDownload->m_oBTH, oBTH );
		pDownload->m_oBTH = oBTH;
		if ( oBTH ) pDownload->m_bBTHTrusted = true;
		pDownload->CloseTransfers();
		pDownload->ClearVerification();
		bNewBTH = TRUE;
	}

	pDownload->m_bSHA1Trusted = m_bSHA1Trusted != FALSE;
	pDownload->m_bTigerTrusted = m_bTigerTrusted != FALSE;
	pDownload->m_bED2KTrusted = m_bED2KTrusted != FALSE;
	pDownload->m_bMD5Trusted = m_bMD5Trusted != FALSE;
	pDownload->m_bBTHTrusted = m_bBTHTrusted != FALSE;

	if ( bCriticalChange )
	{
		pDownload->CloseTransfers();
		pDownload->ClearSources();
		pDownload->ClearFailedSources();
		pDownload->ClearVerification();
		bNeedUpdate = true;
	}

	if ( bNewBTH )
	{
		if ( ! pDownload->m_oBTH && pDownload->IsTorrent() )
		{
			// Mutate torrent download to regular download
			pDownload->m_pTorrent.Clear();
			pDownload->SetTorrent();
		}
	}

	if ( bNeedUpdate )
	{
		pDownload->SetModified();
	}

	return CPropertyPageAdv::OnApply();
}
