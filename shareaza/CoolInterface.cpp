//
// CoolInterface.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "CoolInterface.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CCoolInterface CoolInterface;


//////////////////////////////////////////////////////////////////////
// CCoolInterface construction

CCoolInterface::CCoolInterface()
{
	CreateFonts();
	CalculateColours();
	
	m_czBuffer = CSize( 0, 0 );

	// experimental values
	m_pNameMap.InitHashTable( 509 );
	m_pImageMap16.InitHashTable( 347 );
	m_pImageMap32.InitHashTable( 17 );
	m_pWindowIcons.InitHashTable( 61 );
}

CCoolInterface::~CCoolInterface()
{
	Clear();

	HICON hIcon;
	HWND hWnd;
	for( POSITION pos = m_pWindowIcons.GetStartPosition(); pos; )
	{
		m_pWindowIcons.GetNextAssoc( pos, hIcon, hWnd );
		VERIFY( DestroyIcon( hIcon ) );
	}
	m_pWindowIcons.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface clear

void CCoolInterface::Clear()
{
	m_pNameMap.RemoveAll();

	m_pImageMap16.RemoveAll();
	if ( m_pImages16.m_hImageList )
	{
		m_pImages16.DeleteImageList();
	}

	m_pImageMap32.RemoveAll();
	if ( m_pImages32.m_hImageList )
	{
		m_pImages32.DeleteImageList();
	}

	if ( m_bmBuffer.m_hObject != NULL )
	{
		m_dcBuffer.SelectObject( CGdiObject::FromHandle( m_bmOldBuffer ) );
		m_dcBuffer.DeleteDC();
		m_bmBuffer.DeleteObject();
	}

	m_czBuffer = CSize( 0, 0 );
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface command management

void CCoolInterface::NameCommand(UINT nID, LPCTSTR pszName)
{
	m_pNameMap.SetAt( pszName, nID );
}

UINT CCoolInterface::NameToID(LPCTSTR pszName) const
{
	UINT nID = 0;
	if ( m_pNameMap.Lookup( pszName, nID ) ) return nID;
	if ( _stscanf( pszName, _T("%lu"), &nID ) == 1 ) return nID;
	return 0;
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface image management

int CCoolInterface::ImageForID(UINT nID, int nImageListType)
{
	int nImage = -1;
	if ( nImageListType == LVSIL_SMALL )
		return m_pImageMap16.Lookup( nID, nImage ) ? nImage : -1;
	else
		return m_pImageMap32.Lookup( nID, nImage ) ? nImage : -1;
}

void CCoolInterface::AddIcon(UINT nID, HICON hIcon, int nImageListType)
{
	VERIFY( ConfirmImageList() );

	if ( nImageListType == LVSIL_SMALL )
		m_pImageMap16.SetAt( nID, m_pImages16.Add( hIcon ) );
	else
		m_pImageMap32.SetAt( nID, m_pImages32.Add( hIcon ) );
}

void CCoolInterface::CopyIcon(UINT nFromID, UINT nToID, int nImageListType)
{
	int nImage;
	if ( nImageListType == LVSIL_SMALL )
	{
		if ( m_pImageMap16.Lookup( nFromID, nImage ) )
			m_pImageMap16.SetAt( nToID, nImage );
	}
	else
	{
		if ( m_pImageMap32.Lookup( nFromID, nImage ) )
			m_pImageMap32.SetAt( nToID, nImage );
	}
}

HICON CCoolInterface::ExtractIcon(UINT nID, BOOL bMirrored, int nImageListType)
{
	HICON hIcon = NULL;
	int nImage = ImageForID( nID, nImageListType );
	if ( nImage >= 0 )
	{
		if ( nImageListType == LVSIL_SMALL )
			hIcon = m_pImages16.ExtractIcon( nImage );
		else
			hIcon = m_pImages32.ExtractIcon( nImage );
	}
	if ( hIcon == NULL )
	{
		hIcon = (HICON)LoadImage( AfxGetResourceHandle(),
			MAKEINTRESOURCE( nID ), IMAGE_ICON,
			( ( nImageListType == LVSIL_SMALL ) ? 16 : 32 ),
			( ( nImageListType == LVSIL_SMALL ) ? 16 : 32 ), 0 );
		if ( hIcon )
			AddIcon( nID, hIcon, nImageListType );
	}
	if ( hIcon )
	{
		if ( bMirrored && nID != ID_HELP_ABOUT )
		{
			hIcon = CreateMirroredIcon( hIcon );
			ASSERT( hIcon != NULL );
		}
	}
	return hIcon;
}

void CCoolInterface::SetIcon(UINT nID, BOOL bMirrored, BOOL bBigIcon, CWnd* pWnd)
{
	HICON hIcon = ExtractIcon( nID, bMirrored, bBigIcon ? LVSIL_NORMAL : LVSIL_SMALL );
	if ( hIcon )
	{
		m_pWindowIcons.SetAt( hIcon, pWnd->GetSafeHwnd() );
		hIcon = pWnd->SetIcon( hIcon, bBigIcon );
		if ( hIcon )
		{
			VERIFY( m_pWindowIcons.RemoveKey( hIcon ) );
			VERIFY( DestroyIcon( hIcon ) );
		}
	}
}

void CCoolInterface::SetIcon(HICON hIcon, BOOL bMirrored, BOOL bBigIcon, CWnd* pWnd)
{
	if ( hIcon )
	{
		if ( bMirrored ) hIcon = CreateMirroredIcon( hIcon );
		m_pWindowIcons.SetAt( hIcon, pWnd->GetSafeHwnd() );
		hIcon = pWnd->SetIcon( hIcon, bBigIcon );
		if ( hIcon )
		{
			VERIFY( m_pWindowIcons.RemoveKey( hIcon ) );
			VERIFY( DestroyIcon( hIcon ) );
		}
	}
}

/*BOOL CCoolInterface::AddImagesFromToolbar(UINT nIDToolBar, COLORREF crBack)
{
	VERIFY( ConfirmImageList() );
	
	CBitmap pBmp;
	if ( ! pBmp.LoadBitmap( nIDToolBar ) ) return FALSE;
	int nBase = m_pImages.Add( &pBmp, crBack );
	pBmp.DeleteObject();
	
	if ( nBase < 0 ) return FALSE;
	
	BOOL bRet = FALSE;
	HRSRC hRsrc = FindResource( AfxGetResourceHandle(), MAKEINTRESOURCE(nIDToolBar), RT_TOOLBAR );
	if ( hRsrc )
	{
		HGLOBAL hGlobal = LoadResource( AfxGetResourceHandle(), hRsrc );
		if ( hGlobal )
		{
			TOOLBAR_RES* pData = (TOOLBAR_RES*)LockResource( hGlobal );
			if ( pData )
			{
				for ( WORD nItem = 0 ; nItem < pData->wItemCount ; nItem++ )
				{
					if ( pData->items()[ nItem ] != ID_SEPARATOR )
					{
						m_pImageMap.SetAt( pData->items()[ nItem ], nBase );
						nBase++;
					}
				}
				bRet = TRUE;
			}			
			FreeResource( hGlobal );
		}
	}
	return bRet;
}*/

BOOL CCoolInterface::ConfirmImageList()
{
	return
		( m_pImages16.m_hImageList ||
		  m_pImages16.Create( 16, 16, ILC_COLOR32|ILC_MASK, 16, 4 ) ||
		  m_pImages16.Create( 16, 16, ILC_COLOR24|ILC_MASK, 16, 4 ) ||
		  m_pImages16.Create( 16, 16, ILC_COLOR16|ILC_MASK, 16, 4 ) ) &&
		( m_pImages32.m_hImageList ||
		  m_pImages32.Create( 32, 32, ILC_COLOR32|ILC_MASK, 16, 4 ) ||
		  m_pImages32.Create( 32, 32, ILC_COLOR24|ILC_MASK, 16, 4 ) ||
		  m_pImages32.Create( 32, 32, ILC_COLOR16|ILC_MASK, 16, 4 ) );
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface double buffer

CDC* CCoolInterface::GetBuffer(CDC& dcScreen, CSize& szItem)
{
	if ( szItem.cx <= m_czBuffer.cx && szItem.cy <= m_czBuffer.cy )
	{
		m_dcBuffer.SelectClipRgn( NULL );
		return &m_dcBuffer;
	}
	
	if ( m_bmBuffer.m_hObject )
	{
		m_dcBuffer.SelectObject( CGdiObject::FromHandle( m_bmOldBuffer ) );
		m_bmBuffer.DeleteObject();
	}
	
	m_czBuffer.cx = max( m_czBuffer.cx, szItem.cx );
	m_czBuffer.cy = max( m_czBuffer.cy, szItem.cy );
	
	if ( m_dcBuffer.m_hDC == NULL ) m_dcBuffer.CreateCompatibleDC( &dcScreen );
	m_bmBuffer.CreateCompatibleBitmap( &dcScreen, m_czBuffer.cx, m_czBuffer.cy );
	m_bmOldBuffer = (HBITMAP)m_dcBuffer.SelectObject( &m_bmBuffer )->GetSafeHandle();
	
	return &m_dcBuffer;
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface watermarking

BOOL CCoolInterface::DrawWatermark(CDC* pDC, CRect* pRect, CBitmap* pMark, int nOffX, int nOffY)
{
	BITMAP pWatermark;
	CBitmap* pOldMark;
	CDC dcMark;
	
	if ( pDC == NULL || pRect == NULL || pMark == NULL || pMark->m_hObject == NULL )
		return FALSE;
	
	dcMark.CreateCompatibleDC( pDC );
	if ( Settings.General.LanguageRTL ) theApp.m_pfnSetLayout( dcMark.m_hDC, LAYOUT_BITMAPORIENTATIONPRESERVED );
	pOldMark = (CBitmap*)dcMark.SelectObject( pMark );
	pMark->GetBitmap( &pWatermark );
	
	for ( int nY = pRect->top - nOffY ; nY < pRect->bottom ; nY += pWatermark.bmHeight )
	{
		if ( nY + pWatermark.bmHeight < pRect->top ) continue;
		
		for ( int nX = pRect->left - nOffX ; nX < pRect->right ; nX += pWatermark.bmWidth )
		{
			if ( nX + pWatermark.bmWidth < pRect->left ) continue;
			
			pDC->BitBlt( nX, nY, pWatermark.bmWidth, pWatermark.bmHeight, &dcMark, 0, 0, SRCCOPY );
		}
	}
	
	dcMark.SelectObject( pOldMark );
	dcMark.DeleteDC();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface fonts

void CCoolInterface::CreateFonts(LPCTSTR pszFace, int nSize)
{
	if ( ! pszFace ) pszFace = theApp.m_sDefaultFont;
	if ( ! nSize ) nSize = theApp.m_nDefaultFontSize;
	
	if ( m_fntNormal.m_hObject ) m_fntNormal.DeleteObject();
	if ( m_fntBold.m_hObject ) m_fntBold.DeleteObject();
	if ( m_fntUnder.m_hObject ) m_fntUnder.DeleteObject();
	if ( m_fntCaption.m_hObject ) m_fntCaption.DeleteObject();
	if ( m_fntItalic.m_hObject ) m_fntItalic.DeleteObject();
	if ( m_fntBoldItalic.m_hObject ) m_fntBoldItalic.DeleteObject();
	
	m_fntNormal.CreateFontW( -nSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );
	
	m_fntBold.CreateFontW( -nSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );
	
	m_fntUnder.CreateFontW( -nSize, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );
	
	m_fntCaption.CreateFontW( -nSize - 2, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );

	m_fntItalic.CreateFontW( -nSize, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );

	m_fntBoldItalic.CreateFontW( -nSize, 0, 0, 0, FW_BOLD, TRUE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface colours

void CCoolInterface::CalculateColours(BOOL bCustom)
{
	if ( ( m_bCustom = bCustom ) == FALSE )
	{
		m_crWindow		= GetSysColor( COLOR_WINDOW );
		m_crMidtone		= GetSysColor( COLOR_BTNFACE );
		m_crText		= GetSysColor( COLOR_WINDOWTEXT );
		m_crHiText		= GetSysColor( COLOR_HIGHLIGHTTEXT );
		m_crHiBorder	= GetSysColor( COLOR_HIGHLIGHT );
		m_crHighlight	= GetSysColor( COLOR_HIGHLIGHT );
	}

	m_crBackNormal			= CalculateColour( m_crMidtone, m_crWindow, 215 );
	m_crBackSel				= CalculateColour( m_crHighlight, m_crWindow, 178 );
	m_crBackCheck			= CalculateColour( m_crHighlight, m_crWindow, 200 );
	m_crBackCheckSel		= CalculateColour( m_crHighlight, m_crWindow, 127 );
	m_crMargin				= CalculateColour( m_crMidtone, m_crWindow, 39 );
	m_crShadow				= CalculateColour( m_crHighlight, GetSysColor( COLOR_3DSHADOW ), 200 );
	m_crBorder				= m_crHighlight;
	m_crCmdText				= GetSysColor( COLOR_MENUTEXT );
	m_crCmdTextSel			= GetSysColor( COLOR_MENUTEXT );
	m_crDisabled			= GetSysColor( COLOR_GRAYTEXT );

	m_crTipBack				= GetSysColor( COLOR_INFOBK );
	m_crTipText				= GetSysColor( COLOR_INFOTEXT );
	m_crTipBorder			= CalculateColour( m_crTipBack, (COLORREF)0, 100 );
	m_crTipWarnings			= RGB( 64, 64, 64 ); // Colour of warning messages (Grey)
	
	m_crTaskPanelBack		= RGB( 122, 161, 230 );
	m_crTaskBoxCaptionBack	= RGB( 250, 250, 255 );
	m_crTaskBoxPrimaryBack	= RGB( 30, 87, 199 );
	m_crTaskBoxCaptionText	= RGB( 34, 93, 217 );
	m_crTaskBoxPrimaryText	= RGB( 255, 255, 255 );
	m_crTaskBoxCaptionHover	= RGB( 84, 143, 255 );
	m_crTaskBoxClient		= RGB( 214, 223, 247 );

	m_crMediaWindow			= RGB( 0, 0, 0 );
	m_crMediaWindowText		= RGB( 200, 200, 255 );
	m_crMediaStatus			= RGB( 0, 0, 0x60 );
	m_crMediaStatusText		= RGB( 0xF0, 0xF0, 0xFF );
	m_crMediaPanel  		= RGB( 0, 0, 0x30 );
	m_crMediaPanelText  	= RGB( 255, 255, 255 );
	m_crMediaPanelActive  	= RGB( 128, 0, 0 );
	m_crMediaPanelActiveText = RGB( 255, 255, 255 );
	m_crMediaPanelCaption	= RGB( 0x00, 0x00, 0x80 );
	m_crMediaPanelCaptionText = RGB( 0xFF, 0xFF, 0 );

	m_crTrafficWindowBack	= RGB( 0, 0, 0 );
	m_crTrafficWindowText	= RGB( 193, 196, 255 );
	m_crTrafficWindowGrid	= RGB( 0, 0, 128 );

	m_crMonitorHistoryBack	= RGB( 0, 0, 0 );
	m_crMonitorHistoryBackMax = RGB( 80, 0, 0 );
	m_crMonitorHistoryText	= RGB( 255, 0, 0 );
	m_crMonitorDownloadLine	= RGB( 0, 0xFF, 0 );
	m_crMonitorUploadLine	= RGB( 0xFF, 0xFF, 0 );
	m_crMonitorDownloadBar	= RGB( 0, 0xBB, 0 );
	m_crMonitorUploadBar	= RGB( 0xBB, 0xBB, 0 );

	m_crRatingNull			= RGB( 0, 0, 0 );
 	m_crRating0				= RGB( 255, 0, 0 );
	m_crRating1	 			= RGB( 128, 128, 128 );
	m_crRating2				= RGB( 80, 80, 80 );
	m_crRating3				= RGB( 0, 0, 0 );
	m_crRating4				= RGB( 0, 128, 0 );
	m_crRating5				= RGB( 0, 0, 255 );

	m_crRichdocBack			= RGB( 255, 255, 255 );
	m_crRichdocText  		= RGB( 0, 0, 0 );
	m_crRichdocHeading 		= RGB( 0x80, 0, 0 );
	m_crTextAlert 			= RGB( 255, 0, 0 );
	m_crTextStatus 			= RGB( 0, 128, 0 );
	m_crTextLink  			= RGB( 0, 0, 255 );
	m_crTextLinkHot			= RGB( 255, 0, 0 );

 	m_crChatIn				= RGB( 0, 0, 255 );
	m_crChatOut				= RGB( 255, 0, 0 );
	m_crChatNull	  		= RGB( 128, 128, 128 );
	m_crSearchExists  		= RGB( 0, 127, 0 );
	m_crSearchExistsHit		= RGB( 0, 64, 0 ); 
	m_crSearchExistsSelected	= RGB( 0, 255, 0 );
	m_crSearchQueued 		= RGB( 0, 0, 160 ); 
	m_crSearchQueuedHit		= RGB( 0, 0, 100 );
	m_crSearchQueuedSelected	= GetSysColor( COLOR_HIGHLIGHTTEXT );
	m_crSearchGhostrated	= RGB( 200, 90, 0 );
	m_crSearchNull  		= GetSysColor( COLOR_3DSHADOW ); 
	m_crTransferSource		= RGB( 30, 30, 30 );
	m_crTransferRanges		= RGB( 220, 240, 220 );
	m_crTransferCompleted	= RGB( 0, 127, 0 );
	m_crTransferVerifyPass	= RGB( 0, 0, 127 );
	m_crTransferVerifyFail	= RGB( 255, 0, 0 );
	m_crTransferCompletedSelected	= RGB( 0, 255, 0 );
	m_crTransferVerifyPassSelected	= RGB( 0, 255, 0 );
	m_crTransferVerifyFailSelected	= RGB( 255, 0, 0 );
	m_crNetworkNull  		= RGB( 192, 192, 192 );
	m_crNetworkG1  			= RGB( 80, 80, 80 );
	m_crNetworkG2  			= RGB( 100, 100, 255 );
	m_crNetworkED2K  		= RGB( 128, 128, 0 );
	m_crNetworkUp			= RGB( 127, 0, 0 );
	m_crNetworkDown			= RGB( 0, 0, 127 );
	m_crSecurityAllow		= RGB( 0, 127, 0 );
	m_crSecurityDeny		= RGB( 255, 0, 0 );

	m_crDropdownText		= GetSysColor( COLOR_MENUTEXT );
	m_crDropdownBox			= GetSysColor( COLOR_WINDOW );
	m_crResizebarEdge		= GetSysColor( COLOR_BTNFACE );
	m_crResizebarFace		= GetSysColor( COLOR_BTNFACE );
	m_crResizebarShadow		= GetSysColor( COLOR_3DSHADOW );
	m_crResizebarHighlight	= GetSysColor( COLOR_3DHIGHLIGHT );
	m_crFragmentShaded		= GetSysColor( COLOR_BTNFACE );
	m_crFragmentComplete	= GetSysColor( COLOR_ACTIVECAPTION );
	m_crFragmentSource1		= RGB( 0, 153, 255 );
	m_crFragmentSource2 	= RGB( 0, 153, 0 );
	m_crFragmentSource3 	= RGB( 255, 51, 0 );
	m_crFragmentSource4 	= RGB( 255, 204, 0 );
	m_crFragmentSource5 	= RGB( 153, 153, 255 );
	m_crFragmentSource6 	= RGB( 204, 153, 0 );
	m_crFragmentPass		= RGB( 0, 220, 0 );
	m_crFragmentFail		= RGB( 220, 0, 0 );
	m_crFragmentRequest		= RGB( 255, 255, 0 );
	m_crFragmentBorder		= RGB( 50, 50, 50 );
	m_crFragmentBorderSelected	= RGB( 50, 50, 50 );
	m_crFragmentBorderSimpleBar	= RGB( 50, 50, 50 );
	m_crFragmentBorderSimpleBarSelected	= RGB( 255, 255, 255 );

	m_crSysWindow			= GetSysColor( COLOR_WINDOW );
	m_crSysBtnFace			= GetSysColor( COLOR_BTNFACE );
	m_crSys3DShadow 		= GetSysColor( COLOR_3DSHADOW );
	m_crSys3DHighlight		= GetSysColor( COLOR_3DHIGHLIGHT );
	m_crSysActiveCaption	= GetSysColor( COLOR_ACTIVECAPTION );

}

void CCoolInterface::OnSysColourChange()
{
	if ( ! m_bCustom ) CalculateColours();
}

COLORREF CCoolInterface::CalculateColour(COLORREF crFore, COLORREF crBack, int nAlpha)
{
	int nRed	= GetRValue( crFore ) * ( 255 - nAlpha ) / 255 + GetRValue( crBack ) * nAlpha / 255;
	int nGreen	= GetGValue( crFore ) * ( 255 - nAlpha ) / 255 + GetGValue( crBack ) * nAlpha / 255;
	int nBlue	= GetBValue( crFore ) * ( 255 - nAlpha ) / 255 + GetBValue( crBack ) * nAlpha / 255;

	return RGB( nRed, nGreen, nBlue );
}

COLORREF CCoolInterface::GetDialogBkColor()
{
	return CalculateColour( GetSysColor( COLOR_BTNFACE ), GetSysColor( COLOR_WINDOW ), 200 );
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface windows version check

BOOL CCoolInterface::IsNewWindows()
{
	OSVERSIONINFO pVersion;
	pVersion.dwOSVersionInfoSize = sizeof(pVersion);
	GetVersionEx( &pVersion );

	return pVersion.dwMajorVersion > 5 || ( pVersion.dwMajorVersion == 5 && pVersion.dwMinorVersion >= 1 );
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface windows XP+ themes

BOOL CCoolInterface::EnableTheme(CWnd* pWnd, BOOL bEnable)
{
	BOOL bResult = FALSE;

	if ( theApp.m_pfnSetWindowTheme )
	{
		if ( bEnable )
		{
			bResult = SUCCEEDED( theApp.m_pfnSetWindowTheme( pWnd->GetSafeHwnd(), NULL, NULL ) );
		}
		else
		{
			bResult = SUCCEEDED( theApp.m_pfnSetWindowTheme( pWnd->GetSafeHwnd(), L" ", L" " ) );
		}
	}

	return bResult;
}

int CCoolInterface::GetImageCount(int nImageListType)
{
	return ( nImageListType == LVSIL_SMALL ) ?
		m_pImages16.GetImageCount() :
		m_pImages32.GetImageCount();
}

BOOL CCoolInterface::Add(CSkin* pSkin, CXMLElement* pBase, HBITMAP hbmImage, COLORREF crMask, int nImageListType)
{
	VERIFY( ConfirmImageList() );

	int nBase = ( nImageListType == LVSIL_SMALL ) ?
		m_pImages16.Add( CBitmap::FromHandle( hbmImage ), crMask ) :
		m_pImages32.Add( CBitmap::FromHandle( hbmImage ), crMask );
	if ( nBase < 0 )
	{
		return FALSE;
	}

	const LPCTSTR pszNames[] = {
		_T("id"),  _T("id1"), _T("id2"), _T("id3"), _T("id4"), _T("id5"),
		_T("id6"), _T("id7"), _T("id8"), _T("id9"), NULL };
	int nIndex = 0;
	int nIndexRev = GetImageCount( nImageListType ) - 1;	// Total number of images
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		if ( ! pXML->IsNamed( _T("image") ) ) continue;
		CString strValue = pXML->GetAttributeValue( _T("index") );
		if ( strValue.GetLength() ) _stscanf( strValue, _T("%i"), &nIndex );
		nIndex += nBase;
		for ( int nName = 0 ; pszNames[ nName ] ; nName++ )
		{
			UINT nID = pSkin->LookupCommandID( pXML, pszNames[ nName ] );
			if ( nID ) 
			{
				if ( nImageListType == LVSIL_SMALL )
					m_pImageMap16.SetAt( nID, Settings.General.LanguageRTL ? nIndexRev : nIndex );
				else
					m_pImageMap32.SetAt( nID, Settings.General.LanguageRTL ? nIndexRev : nIndex );
			}
			if ( nName && ! nID ) break;
		}
		nIndexRev--;	
		nIndex -= nBase;
		nIndex ++;
	}
	return TRUE;
}

CImageList* CCoolInterface::SetImageListTo(CListCtrl& pWnd, int nImageListType)
{
	return ( nImageListType == LVSIL_SMALL ) ? 
		pWnd.SetImageList( &m_pImages16, nImageListType ) :
		pWnd.SetImageList( &m_pImages32, nImageListType );
}

BOOL CCoolInterface::Draw(CDC* pDC, int nImage, POINT pt, UINT nStyle, int nImageListType)
{
	return ( nImageListType == LVSIL_SMALL ) ?
		m_pImages16.Draw( pDC, nImage, pt, nStyle ) :
		m_pImages32.Draw( pDC, nImage, pt, nStyle );
}

BOOL CCoolInterface::DrawEx(CDC* pDC, int nImage, POINT pt, SIZE sz, COLORREF clrBk, COLORREF clrFg, UINT nStyle, int nImageListType)
{
	return ( nImageListType == LVSIL_SMALL ) ?
		m_pImages16.DrawEx( pDC, nImage, pt, sz, clrBk, clrFg, nStyle ) :
		m_pImages32.DrawEx( pDC, nImage, pt, sz, clrBk, clrFg, nStyle );
}
