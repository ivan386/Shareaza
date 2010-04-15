//
// FragmentBar.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

	if ( nTotal == 0 )
		return;

	rcArea.left		= prcBar->left + ( ( prcBar->Width() + 1 ) * nOffset ) / nTotal;
	rcArea.right	= prcBar->left + ( ( prcBar->Width() + 1 ) * ( nOffset + nLength ) ) / nTotal;
	
	rcArea.top		= prcBar->top;
	rcArea.bottom	= prcBar->bottom;
	
	rcArea.left		= max( rcArea.left, prcBar->left );
	rcArea.right	= min( rcArea.right, prcBar->right );
	
	if ( rcArea.right <= rcArea.left )
		return;
	
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

	rcArea.left		= prcBar->left + (int)( (float)( prcBar->Width() + 1 ) / (float)nTotal * (float)nOffset );
	rcArea.right	= prcBar->left + (int)( (float)( prcBar->Width() + 1 ) / (float)nTotal * (float)( nOffset + nLength ) );
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
	Fragments::List::const_iterator pItr = oList.begin();
	const Fragments::List::const_iterator pEnd = oList.end();
	for ( ; pItr != pEnd ; ++pItr )
	{
		DrawFragment( pDC, prcBar, pDownload->m_nSize, pItr->begin(),
			pItr->size(), crNatural, FALSE );
	}

	for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
	{
		CDownloadSource* pSource = pDownload->GetNext( posSource );

		pSource->Draw( pDC, prcBar );
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
// CFragmentBar upload

void CFragmentBar::DrawUpload(CDC* pDC, CRect* prcBar, CUploadFile* pFile, COLORREF crNatural)
{
	CUploadTransfer* pUpload = pFile->GetActive();
	if ( pUpload == NULL )
		return;

	Fragments::List::const_iterator pItr = pFile->m_oFragments.begin();
	const Fragments::List::const_iterator pEnd = pFile->m_oFragments.end();
	for ( ; pItr != pEnd ; ++pItr  )
	{
		DrawFragment( pDC, prcBar, pFile->m_nSize, pItr->begin(), pItr->size(),
			CoolInterface.m_crFragmentComplete, TRUE );
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
