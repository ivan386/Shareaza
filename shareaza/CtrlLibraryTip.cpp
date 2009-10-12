//
// CtrlLibraryTip.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "ImageServices.h"
#include "ImageFile.h"
#include "CtrlLibraryTip.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CLibraryTipCtrl, CCoolTipCtrl)

BEGIN_MESSAGE_MAP(CLibraryTipCtrl, CCoolTipCtrl)
	//{{AFX_MSG_MAP(CLibraryTipCtrl)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLibraryTipCtrl construction

CLibraryTipCtrl::CLibraryTipCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTipCtrl prepare

BOOL CLibraryTipCtrl::OnPrepare()
{
	{
		CQuickLock oLock( Library.m_pSection );

		CLibraryFile* pLibraryFile = Library.LookupFile(
			reinterpret_cast< DWORD_PTR >( m_pContext ) );

		CShareazaFile* pFile = pLibraryFile ? 
			static_cast< CShareazaFile* >( pLibraryFile ) :
			reinterpret_cast< CShareazaFile* >( m_pContext );

		if ( ! pLibraryFile )
			if ( CLibraryFile* pShared = LibraryMaps.LookupFileByHash( pFile, FALSE, TRUE ) )
				pLibraryFile = pShared;

		CSingleLock pLock( &m_pSection, TRUE );

		// Basic data

		m_sName = pFile->m_sName.IsEmpty() ? pFile->m_sPath : pFile->m_sName;
		if ( pLibraryFile )
			m_sPath = pLibraryFile->GetPath();
		m_sSize = Settings.SmartVolume( pFile->GetSize() );
		m_nIcon = 0;

		if ( pLibraryFile && pLibraryFile->m_pFolder ) 
			m_sFolder = pLibraryFile->m_pFolder->m_sPath;
		else
			m_sFolder.Empty(); // Ghost files have no location

		m_sType.Empty();
		m_sSHA1.Empty();
		m_sTTH.Empty();
		m_sED2K.Empty();
		m_sBTH.Empty();
		m_sMD5.Empty();

		// Type information and icons

		m_sType = ShellIcons.GetTypeString( m_sName );
		m_nIcon = ShellIcons.Get( m_sName, 48 );

		// URN
		if ( Settings.General.GUIMode != GUI_BASIC )
		{
			m_sSHA1 = pFile->m_oSHA1.toShortUrn();
			m_sTTH = pFile->m_oTiger.toShortUrn();
			m_sED2K = pFile->m_oED2K.toShortUrn();
			m_sBTH = pFile->m_oBTH.toShortUrn();
			m_sMD5 = pFile->m_oMD5.toShortUrn();
		}

		// Metadata

		CSchema* pSchema = pLibraryFile ? pLibraryFile->m_pSchema : NULL;
		CString str, sData, sFormat;

		m_pMetadata.Clear();

		if ( ! m_sFolder.IsEmpty() )
		{
			LoadString( str, IDS_TIP_LOCATION );
			m_pMetadata.Add( str, m_sFolder );
		}

		if ( ! m_sType.IsEmpty() )
		{
			LoadString( str, IDS_TIP_TYPE );
			m_pMetadata.Add( str, m_sType );
		}

		if ( m_sSize )
		{
			LoadString( str, IDS_TIP_SIZE );
			m_pMetadata.Add( str, m_sSize );
		}

		if ( pLibraryFile )
		{
			LoadString( sFormat, IDS_TIP_TODAYTOTAL );
			sData.Format( sFormat, pLibraryFile->m_nHitsToday, pLibraryFile->m_nHitsTotal );
			LoadString( str, IDS_TIP_HITS );
			m_pMetadata.Add( str, sData );
			sData.Format( sFormat, pLibraryFile->m_nUploadsToday, pLibraryFile->m_nUploadsTotal );
			LoadString( str, IDS_TIP_UPLOADS );
			m_pMetadata.Add( str, sData );

			if ( pLibraryFile->m_pMetadata && pSchema )
			{
				m_pMetadata.Setup( pSchema, FALSE );
				m_pMetadata.Combine( pLibraryFile->m_pMetadata );
				m_pMetadata.Clean();
			}
		}
	}

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
	m_sz.cx =  min( max( m_sz.cx, m_nKeyWidth + nValueWidth +
		(int)Settings.Library.ThumbSize + 16 ), GetSystemMetrics( SM_CXSCREEN ) / 2 );
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
	CloseThread();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTipCtrl thread run

void CLibraryTipCtrl::OnRun()
{
	while ( IsThreadEnabled() )
	{
		Doze();
		if ( ! IsThreadEnabled() )
			break;

		m_pSection.Lock();
		CString strPath = m_sPath;
		m_pSection.Unlock();

		if ( strPath.IsEmpty() ) // TODO: Make preview requests by hash
			break;

		CImageFile pFile;
		if ( CThumbCache::Cache( strPath, &pFile ) )
		{
			m_pSection.Lock();

			if ( m_bmThumb.m_hObject ) m_bmThumb.DeleteObject();

			if ( m_sPath == strPath )
			{
				m_bmThumb.Attach( pFile.CreateBitmap() );
				Invalidate();
			}

			m_pSection.Unlock();
		}
	}
}

