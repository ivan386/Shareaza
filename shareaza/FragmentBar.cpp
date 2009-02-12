//
// FragmentBar.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "Settings.h"
#include "CoolInterface.h"
#include "FragmentBar.h"

#include "Download.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadTransferHTTP.h"
#include "DownloadTransferED2K.h"
#include "DownloadTransferBT.h"
#include "UploadFile.h"
#include "UploadTransfer.h"
#include "UploadTransferHTTP.h"
#include "UploadTransferED2K.h"
#include "FragmentedFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CFragmentBar fragment

void CFragmentBar::DrawFragment(CDC* pDC, CRect* prcBar, QWORD nTotal, QWORD nOffset, QWORD nLength, COLORREF crFill, BOOL b3D)
{
	CRect rcArea;
	
	if ( Settings.General.LanguageRTL )
		nOffset = nTotal - nOffset - nLength;

	rcArea.left		= prcBar->left + (int)( (double)( prcBar->Width() + 1 ) / (double)nTotal * (double)nOffset );
	rcArea.right	= prcBar->left + (int)( (double)( prcBar->Width() + 1 ) / (double)nTotal * (double)( nOffset + nLength ) );
	
	rcArea.top		= prcBar->top;
	rcArea.bottom	= prcBar->bottom;
	
	rcArea.left		= max( rcArea.left, prcBar->left );
	rcArea.right	= min( rcArea.right, prcBar->right );
	
	if ( rcArea.right <= rcArea.left ) return;
	
	if ( b3D && rcArea.Width() > 2 )
	{
		pDC->Draw3dRect( &rcArea,	CCoolInterface::CalculateColour( crFill, RGB(255,255,255), 75 ),
									CCoolInterface::CalculateColour( crFill, RGB(0,0,0), 75 ) );
		
		rcArea.DeflateRect( 1, 1 );
		pDC->FillSolidRect( &rcArea, crFill );
		rcArea.InflateRect( 1, 1 );
	}
	else
	{
		pDC->FillSolidRect( &rcArea, crFill );
	}
	
	pDC->ExcludeClipRect( &rcArea );
}

//////////////////////////////////////////////////////////////////////
// CFragmentBar state bar

void CFragmentBar::DrawStateBar(CDC* pDC, CRect* prcBar, QWORD nTotal, QWORD nOffset, QWORD nLength, COLORREF crFill, BOOL bTop)
{
	CRect rcArea;
	// Investigate why nLength is greater than nTotal !!! 
	if ( nLength > nTotal - nOffset )
		nLength = nTotal - nOffset;

	if ( Settings.General.LanguageRTL )
		nOffset = nTotal - nOffset - nLength;

	rcArea.left		= prcBar->left + (int)( (double)( prcBar->Width() + 1 ) / (double)nTotal * (double)nOffset );
	rcArea.right	= prcBar->left + (int)( (double)( prcBar->Width() + 1 ) / (double)nTotal * (double)( nOffset + nLength ) );
	rcArea.left		= max( rcArea.left, prcBar->left );
	rcArea.right	= min( rcArea.right, prcBar->right );
	
	if ( bTop )
	{
		rcArea.top		= prcBar->top;
		rcArea.bottom	= min( ( prcBar->top + prcBar->bottom ) / 2, prcBar->top + 2 ) - 1;
	}
	else
	{
		rcArea.top		= max( ( prcBar->top + prcBar->bottom ) / 2, prcBar->bottom - 3 ) + 1;
		rcArea.bottom	= prcBar->bottom;
	}
	
	if ( rcArea.right <= rcArea.left ) return;
	
	if ( rcArea.Width() > 2 )
	{
		rcArea.DeflateRect( 1, 0 );
		pDC->FillSolidRect( &rcArea, crFill );
		rcArea.InflateRect( 1, 0 );
		
		pDC->FillSolidRect( rcArea.left, rcArea.top, 1, rcArea.Height(),
			CCoolInterface::CalculateColour( crFill, RGB(255,255,255), 100 ) );
		pDC->FillSolidRect( rcArea.right - 1, rcArea.top, 1, rcArea.Height(),
			CCoolInterface::CalculateColour( crFill, RGB(0,0,0), 75 ) );
	}
	else
	{
		pDC->FillSolidRect( &rcArea, crFill );
	}
	
	if ( bTop )
	{
		pDC->FillSolidRect( rcArea.left, rcArea.bottom, rcArea.Width(), 1,
			CCoolInterface::CalculateColour( crFill, RGB(0,0,0), 100 ) );
		rcArea.bottom ++;
	}
	else
	{
		rcArea.top --;
		pDC->FillSolidRect( rcArea.left, rcArea.top, rcArea.Width(), 1,
			CCoolInterface::CalculateColour( crFill, RGB(255,255,255), 100 ) );
	}
	
	pDC->ExcludeClipRect( &rcArea );
}

//////////////////////////////////////////////////////////////////////
// CFragmentBar download

void CFragmentBar::DrawDownload(CDC* pDC, CRect* prcBar, CDownload* pDownload, COLORREF crNatural)
{
	QWORD nvOffset, nvLength;
	BOOL bvSuccess;
		
	if ( Settings.Downloads.ShowPercent && pDownload->IsStarted() )
	{
		DrawStateBar( pDC, prcBar, pDownload->m_nSize, 0, pDownload->GetVolumeComplete(),
			RGB( 0, 255, 0 ), TRUE );
	}
		
	for ( nvOffset = 0 ; pDownload->GetNextVerifyRange( nvOffset, nvLength, bvSuccess ) ; )
	{
		DrawStateBar( pDC, prcBar, pDownload->m_nSize, nvOffset, nvLength,
			bvSuccess ? CoolInterface.m_crFragmentPass : CoolInterface.m_crFragmentFail );
		nvOffset += nvLength;
	}
	
	Fragments::List oList( pDownload->GetEmptyFragmentList() );
	for ( Fragments::List::const_iterator pFragment = oList.begin();
		pFragment != oList.end(); ++pFragment )
	{
		DrawFragment( pDC, prcBar, pDownload->m_nSize,
			pFragment->begin(), pFragment->size(), crNatural, FALSE );
	}
		
	for ( CDownloadSource* pSource = pDownload->GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
	{
		DrawSourceImpl( pDC, prcBar, pSource );
	}

	pDC->FillSolidRect( prcBar, pDownload->IsStarted() ? CoolInterface.m_crFragmentComplete : crNatural );
}


void CFragmentBar::DrawDownloadSimple(CDC* pDC, CRect* prcBar, CDownload* pDownload, COLORREF crNatural)
{
	pDC->FillSolidRect( prcBar, crNatural );		
		
	if ( pDownload->IsStarted() )
	{
		DrawFragment( pDC, prcBar, pDownload->m_nSize,0, pDownload->GetVolumeComplete(), 
			CoolInterface.m_crFragmentComplete, FALSE );
	}
}


//////////////////////////////////////////////////////////////////////
// CFragmentBar download source

void CFragmentBar::DrawSource(CDC* pDC, CRect* prcBar, CDownloadSource* pSource, COLORREF crNatural)
{
	if ( pSource->m_pTransfer != NULL )
	{
		if ( pSource->m_pTransfer->m_nLength < SIZE_UNKNOWN )
		{
			DrawStateBar( pDC, prcBar, pSource->m_pDownload->m_nSize,
				pSource->m_pTransfer->m_nOffset, pSource->m_pTransfer->m_nLength,
				CoolInterface.m_crFragmentRequest, TRUE );
		}

		switch( pSource->m_pTransfer->m_nProtocol )
		{
		case PROTOCOL_G1:
		case PROTOCOL_G2:
		case PROTOCOL_HTTP:
		case PROTOCOL_FTP:
			// Do nothing more
			break;
		case PROTOCOL_ED2K:
			for ( Fragments::Queue::const_iterator pRequested
				= static_cast< CDownloadTransferED2K* >( pSource->m_pTransfer )->m_oRequested.begin();
				pRequested
				!= static_cast< CDownloadTransferED2K* >( pSource->m_pTransfer )->m_oRequested.end();
				++pRequested )
			{
				DrawStateBar( pDC, prcBar, pSource->m_pDownload->m_nSize,
					pRequested->begin(), pRequested->size(), CoolInterface.m_crFragmentRequest, TRUE );
			}
			break;
		case PROTOCOL_BT:
			for ( Fragments::Queue::const_iterator pRequested
				= static_cast< CDownloadTransferBT* >( pSource->m_pTransfer )->m_oRequested.begin();
				pRequested
				!= static_cast< CDownloadTransferBT* >( pSource->m_pTransfer )->m_oRequested.end();
				++pRequested )
			{
				DrawStateBar( pDC, prcBar, pSource->m_pDownload->m_nSize,
					pRequested->begin(), pRequested->size(), CoolInterface.m_crFragmentRequest, TRUE );
			}
			break;
		case PROTOCOL_NULL:
		case PROTOCOL_ANY:
		default:
			;
		}
	}

	DrawSourceImpl( pDC, prcBar, pSource );

	if ( !pSource->m_oAvailable.empty() )
	{
		for ( Fragments::List::const_iterator pFragment = pSource->m_oAvailable.begin();
			pFragment != pSource->m_oAvailable.end(); ++pFragment )
		{
			DrawFragment( pDC, prcBar, pSource->m_pDownload->m_nSize,
				pFragment->begin(), pFragment->size(), crNatural, FALSE );
		}
		
		pDC->FillSolidRect( prcBar, CoolInterface.m_crWindow );
	}
	else if ( pSource->IsOnline() && pSource->HasUsefulRanges() || !pSource->m_oPastFragments.empty() )
	{
		pDC->FillSolidRect( prcBar, crNatural );
	}
	else
	{
		pDC->FillSolidRect( prcBar, CoolInterface.m_crWindow );
	}
}

void CFragmentBar::DrawSourceImpl(CDC* pDC, CRect* prcBar, CDownloadSource* pSource)
{
	static COLORREF crFill[] =
	{
		CoolInterface.m_crFragmentSource1, CoolInterface.m_crFragmentSource2, CoolInterface.m_crFragmentSource3,
		CoolInterface.m_crFragmentSource4, CoolInterface.m_crFragmentSource5, CoolInterface.m_crFragmentSource6
	};
	
	COLORREF crTransfer;
	
	if ( pSource->m_bReadContent )
	{
		crTransfer = crFill[ pSource->GetColour() ];
	}
	else
	{
		crTransfer = CoolInterface.m_crFragmentComplete;
	}
	
	crTransfer = CCoolInterface::CalculateColour( crTransfer, CoolInterface.m_crHighlight, 90 );
	
	if ( pSource->m_pTransfer != NULL )
	{
		if ( pSource->m_pTransfer->m_nState == dtsDownloading &&
			 pSource->m_pTransfer->m_nOffset < SIZE_UNKNOWN )
		{
			if ( pSource->m_pTransfer->m_bRecvBackwards )
			{
				DrawFragment( pDC, prcBar, pSource->m_pDownload->m_nSize,
					pSource->m_pTransfer->m_nOffset + pSource->m_pTransfer->m_nLength - pSource->m_pTransfer->m_nPosition,
					pSource->m_pTransfer->m_nPosition, crTransfer, TRUE );
			}
			else
			{
				DrawFragment( pDC, prcBar, pSource->m_pDownload->m_nSize,
					pSource->m_pTransfer->m_nOffset,
					pSource->m_pTransfer->m_nPosition, crTransfer, TRUE );
			}
		}
	}
	
	for ( Fragments::List::const_iterator pFragment = pSource->m_oPastFragments.begin();
		pFragment != pSource->m_oPastFragments.end(); ++pFragment )
	{
		DrawFragment( pDC, prcBar, pSource->m_pDownload->m_nSize,
			pFragment->begin(), pFragment->size(), crTransfer, TRUE );
	}
}

//////////////////////////////////////////////////////////////////////
// CFragmentBar upload

void CFragmentBar::DrawUpload(CDC* pDC, CRect* prcBar, CUploadFile* pFile, COLORREF crNatural)
{
	CUploadTransfer* pUpload = pFile->GetActive();
	if ( pUpload == NULL ) return;
	
	for ( Fragments::List::const_iterator pFragment = pFile->m_oFragments.begin();
		pFragment != pFile->m_oFragments.end(); ++pFragment  )
	{
		DrawFragment( pDC, prcBar, pFile->m_nSize, pFragment->begin(),
			pFragment->size(), CoolInterface.m_crFragmentComplete, TRUE );
	}
	
	if ( pFile == pUpload->m_pBaseFile )
	{
		if ( pUpload->m_nProtocol == PROTOCOL_HTTP && ((CUploadTransferHTTP*)pUpload)->IsBackwards() )
		{
			DrawFragment( pDC, prcBar, pFile->m_nSize,
				pUpload->m_nOffset + pUpload->m_nLength - pUpload->m_nPosition,
				pUpload->m_nPosition, CoolInterface.m_crFragmentComplete, TRUE );
			
			DrawFragment( pDC, prcBar, pFile->m_nSize,
				pUpload->m_nOffset,
				pUpload->m_nLength - pUpload->m_nPosition, crNatural, FALSE );
		}
		else
		{
			DrawFragment( pDC, prcBar, pFile->m_nSize,
				pUpload->m_nOffset, pUpload->m_nPosition,
				CoolInterface.m_crFragmentComplete, TRUE );
			
			DrawFragment( pDC, prcBar, pFile->m_nSize,
				pUpload->m_nOffset + pUpload->m_nPosition,
				pUpload->m_nLength - pUpload->m_nPosition, crNatural, FALSE );
		}
	}
	
	pDC->FillSolidRect( prcBar, ( pFile == pUpload->m_pBaseFile )
		? CoolInterface.m_crFragmentShaded : crNatural );
}


