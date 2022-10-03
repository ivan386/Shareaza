//
// CtrlLibraryTip.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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
#include "ShellIcons.h"
#include "Library.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "SchemaMember.h"
#include "ThumbCache.h"
#include "ImageFile.h"
#include "CtrlLibraryTip.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CLibraryTipCtrl, CCoolTipCtrl)

BEGIN_MESSAGE_MAP(CLibraryTipCtrl, CCoolTipCtrl)
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLibraryTipCtrl construction

CLibraryTipCtrl::CLibraryTipCtrl()
	: m_nFileIndex( 0 )
	, m_pFile( NULL )
	, m_nIcon( 0 )
	, m_nKeyWidth( 0 )
	, m_tHidden( 0 )
{
}

CLibraryTipCtrl::~CLibraryTipCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTipCtrl prepare

BOOL CLibraryTipCtrl::OnPrepare()
{

	CSingleLock oLock( &Library.m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return FALSE;

	CLibraryFile* pLibraryFile = NULL;
	CShareazaFile* pFile = NULL;

	if ( m_nFileIndex )
	{
		pLibraryFile = Library.LookupFile( m_nFileIndex );
		pFile = static_cast< CShareazaFile* >( pLibraryFile );
	}
	else if ( m_pFile )
	{
		pLibraryFile = LibraryMaps.LookupFileByHash( m_pFile );
		pFile = m_pFile;
	}
	else
		return FALSE;

	if ( ! pFile )
		return FALSE;

	CSingleLock pLock( &m_pSection, TRUE );

	// Basic data

	m_sName = pFile->m_sName.IsEmpty() ? pFile->m_sPath : pFile->m_sName;
	if ( pLibraryFile )
		m_sPath = pLibraryFile->GetPath();
	else
		m_sPath.Empty();

	m_sSize = Settings.SmartVolume( pFile->GetSize() );

	if ( pLibraryFile )
		m_sFolder = pLibraryFile->GetFolder();
	else
		m_sFolder.Empty(); // Ghost files have no location

	// Type information and icons

	m_sType = ShellIcons.GetTypeString( m_sName );
	m_nIcon = ShellIcons.Get( ( m_sPath.IsEmpty() ? m_sName : m_sPath ), 48 );

	// URN
	if ( Settings.General.GUIMode != GUI_BASIC )
	{
		m_sSHA1 = pFile->m_oSHA1.toShortUrn();
		m_sTTH = pFile->m_oTiger.toShortUrn();
		m_sED2K = pFile->m_oED2K.toShortUrn();
		m_sBTH = pFile->m_oBTH.toShortUrn();
		m_sMD5 = pFile->m_oMD5.toShortUrn();
	}
	else
	{
		m_sSHA1.Empty();
		m_sTTH.Empty();
		m_sED2K.Empty();
		m_sBTH.Empty();
		m_sMD5.Empty();
	}

	// Metadata

	CSchemaPtr pSchema = pLibraryFile ? pLibraryFile->m_pSchema : NULL;

	m_pMetadata.Clear();

	if ( ! m_sFolder.IsEmpty() )
		m_pMetadata.Add( LoadString( IDS_TIP_LOCATION ), m_sFolder );

	if ( ! m_sType.IsEmpty() )
		m_pMetadata.Add( LoadString( IDS_TIP_TYPE ), m_sType );

	if ( m_sSize )
		m_pMetadata.Add( LoadString( IDS_TIP_SIZE ), m_sSize );

	if ( pLibraryFile )
	{
		CString sData, sFormat = LoadString( IDS_TIP_TODAYTOTAL );
		sData.Format( sFormat, pLibraryFile->m_nHitsToday, pLibraryFile->m_nHitsTotal );
		m_pMetadata.Add( LoadString( IDS_TIP_HITS ), sData );
		sData.Format( sFormat, pLibraryFile->m_nUploadsToday, pLibraryFile->m_nUploadsTotal );
		m_pMetadata.Add( LoadString( IDS_TIP_UPLOADS ), sData );

		if ( pLibraryFile->m_pMetadata && pSchema )
		{
			m_pMetadata.Setup( pSchema, FALSE );
			m_pMetadata.Combine( pLibraryFile->m_pMetadata );
			m_pMetadata.Clean();
		}
	}

	oLock.Unlock();

	CalcSizeHelper();

	return m_sz.cx > 0;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTipCtrl compute size

void CLibraryTipCtrl::OnCalcSize(CDC* pDC)
{
	AddSize( pDC, m_sName );
	m_sz.cy += TIP_TEXTHEIGHT;
	pDC->SelectObject( &CoolInterface.m_fntNormal );

	if ( m_sSHA1.GetLength() )
	{
		AddSize( pDC, m_sSHA1 );
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	if ( m_sTTH.GetLength() )
	{
		AddSize( pDC, m_sTTH );
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	if ( m_sED2K.GetLength() )
	{
		AddSize( pDC, m_sED2K );
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	if ( m_sBTH.GetLength() )
	{
		AddSize( pDC, m_sBTH );
		m_sz.cy += TIP_TEXTHEIGHT;
	}
	if ( m_sMD5.GetLength() )
	{
		AddSize( pDC, m_sMD5 );
		m_sz.cy += TIP_TEXTHEIGHT;
	}

	m_sz.cy += TIP_RULE;

	int nMetaHeight = static_cast< int >( m_pMetadata.GetCount( TRUE ) * TIP_TEXTHEIGHT );
	int nValueWidth = 0;
	m_nKeyWidth = 40;

	m_pMetadata.ComputeWidth( pDC, m_nKeyWidth, nValueWidth );

	if ( m_nKeyWidth ) m_nKeyWidth += TIP_GAP;
	m_sz.cx =  min( max( m_sz.cx, (LONG)m_nKeyWidth + nValueWidth +
		(LONG)Settings.Library.ThumbSize + 16 ), (LONG)GetSystemMetrics( SM_CXSCREEN ) / 2 );
	m_sz.cy += max( nMetaHeight, (int)Settings.Library.ThumbSize + 4 );
	m_sz.cy += 6;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTipCtrl painting

void CLibraryTipCtrl::OnPaint(CDC* pDC)
{
	CSingleLock pLock( &m_pSection, TRUE );
	CPoint pt( 0, 0 );
	CSize sz( m_sz.cx, TIP_TEXTHEIGHT );

	DrawText( pDC, &pt, m_sName, &sz );
	pt.y += TIP_TEXTHEIGHT;
	pDC->SelectObject( &CoolInterface.m_fntNormal );

	if ( m_sSHA1.GetLength() )
	{
		DrawText( pDC, &pt, m_sSHA1, &sz );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( m_sTTH.GetLength() )
	{
		DrawText( pDC, &pt, m_sTTH, &sz );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( m_sED2K.GetLength() )
	{
		DrawText( pDC, &pt, m_sED2K, &sz );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( m_sBTH.GetLength() )
	{
		DrawText( pDC, &pt, m_sBTH, &sz );
		pt.y += TIP_TEXTHEIGHT;
	}
	if ( m_sMD5.GetLength() )
	{
		DrawText( pDC, &pt, m_sMD5, &sz );
		pt.y += TIP_TEXTHEIGHT;
	}

	DrawRule( pDC, &pt );

	CRect rcThumb( pt.x, pt.y,
		pt.x + Settings.Library.ThumbSize + 2, pt.y + Settings.Library.ThumbSize + 2 );
	CoolInterface.DrawThumbnail( pDC, rcThumb, IsThreadAlive(), FALSE, m_bmThumb, m_nIcon, -1 );
	pDC->ExcludeClipRect( &rcThumb );

	int nCount = 0;
	pt.x += Settings.Library.ThumbSize;
	sz.cx -= pt.x + 8 + m_nKeyWidth;
	for ( POSITION pos = m_pMetadata.GetIterator() ; pos ; )
	{
		CMetaItem* pItem = m_pMetadata.GetNext( pos );
		if ( pItem->m_pMember && pItem->m_pMember->m_bHidden ) continue;

		pt.x += 8;
		DrawText( pDC, &pt,
			Settings.General.LanguageRTL ? ':' + pItem->m_sKey : pItem->m_sKey + ':', &sz );
		pt.x += m_nKeyWidth;
		DrawText( pDC, &pt, pItem->m_sValue, &sz );
		pt.x -= 8 + m_nKeyWidth;
		pt.y += TIP_TEXTHEIGHT;

		if ( ++nCount == 5 )
		{
			pt.x += 4; pt.y -= 2;
			DrawRule( pDC, &pt, TRUE );
			pt.x -= 4; pt.y -= 2;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTipCtrl show and hide

void CLibraryTipCtrl::OnShow()
{
	OnHide();	// Do the old image destroy

	BeginThread( "CtrlLibraryTip" );

	Wakeup();
}

void CLibraryTipCtrl::OnHide()
{
	m_pSection.Lock();
	if ( m_bmThumb.m_hObject ) m_bmThumb.DeleteObject();
	m_pSection.Unlock();
	m_tHidden = GetTickCount();
}

void CLibraryTipCtrl::OnTimer(UINT_PTR nIDEvent)
{
	CCoolTipCtrl::OnTimer( nIDEvent );

	if ( ! m_bVisible && GetTickCount() - m_tHidden > 20000 )
	{
		StopThread();
	}
}

void CLibraryTipCtrl::OnDestroy()
{
	StopThread();

	CCoolTipCtrl::OnDestroy();
}

void CLibraryTipCtrl::StopThread()
{
	if ( IsThreadAlive() )
	{
		CloseThread();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTipCtrl thread run

void CLibraryTipCtrl::OnRun()
{
	while ( IsThreadEnabled() )
	{
		Doze( 1000 );

		if ( ! IsThreadEnabled() )
			break;

		m_pSection.Lock();
		CString strPath = m_sPath;
		m_pSection.Unlock();

		if ( strPath.IsEmpty() )
			continue;

		CImageFile pFile;
		BOOL bSuccess = CThumbCache::Cache( strPath, &pFile );

		m_pSection.Lock();

		if ( m_bmThumb.m_hObject ) m_bmThumb.DeleteObject();

		if ( m_sPath == strPath )
		{
			m_sPath.Empty();

			if ( bSuccess )
			{
				m_bmThumb.Attach( pFile.CreateBitmap() );
				Invalidate();
			}
		}

		m_pSection.Unlock();
	}
}
