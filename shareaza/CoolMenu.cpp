//
// CoolMenu.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "CoolMenu.h"
#include "Skin.h"
#include "ResultFilters.h"
#include "Shell.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CCoolMenu CoolMenu;


//////////////////////////////////////////////////////////////////////
// CCoolMenu construction

CCoolMenu::CCoolMenu()
{
}

CCoolMenu::~CCoolMenu()
{
	Clear();
}

void CCoolMenu::Clear()
{
	// TODO: Find why sometimes raza crashes inside Windows Shell SetSite() function
	SafeRelease( m_pContextMenuCache );

	SetWatermark( NULL );
	EnableHook( false );
}

void CCoolMenu::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	HRESULT hr;

	if ( bSysMenu )
		return;

	if ( m_pContextMenu2 )
	{
		MENUITEMINFO mii = {};
		mii.cbSize = sizeof( mii );
		mii.fMask = MIIM_ID;
		for ( UINT i = 0; i < (UINT)pPopupMenu->GetMenuItemCount(); i++ )
		{
			if ( pPopupMenu->GetMenuItemInfo( i, &mii, TRUE ) &&
				mii.wID >= ID_SHELL_MENU_MIN && mii.wID <= ID_SHELL_MENU_MAX )
			{
				// Its shell menu
				CString strHelp;
				hr = m_pContextMenu2->GetCommandString( mii.wID - ID_SHELL_MENU_MIN,
					GCS_HELPTEXTW, NULL, (LPSTR)strHelp.GetBuffer( 256 ), 256 );
				strHelp.ReleaseBuffer();
				if ( SUCCEEDED( hr ) )
					Skin.AddString( strHelp, mii.wID );
			}
		}
		for ( UINT i = 0; i < (UINT)pPopupMenu->GetMenuItemCount(); i++ )
		{
			if ( ! pPopupMenu->GetMenuItemInfo( i, &mii, TRUE ) ||
				mii.wID == ID_SEPARATOR || mii.wID == -1 )
				continue;
			if ( mii.wID >= ID_SHELL_MENU_MIN && mii.wID <= ID_SHELL_MENU_MAX )
			{
				// Its shell menu
				hr = m_pContextMenu2->HandleMenuMsg( WM_INITMENUPOPUP,
					(WPARAM)pPopupMenu->GetSafeHmenu(),
					(LPARAM)MAKELONG( nIndex, TRUE ) );
				return;
			}
			// Its regular menu
			break;
		}
	}

	AddMenu( pPopupMenu, TRUE );
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu add menu

BOOL CCoolMenu::AddMenu(CMenu* pMenu, BOOL bChild)
{
	EnableHook();

	if ( ! Settings.Interface.CoolMenuEnable ) return FALSE;

	for ( int i = 0 ; i < (int)pMenu->GetMenuItemCount() ; i++ )
	{
		TCHAR szBuffer[128] = {};
		CString strText;

		MENUITEMINFO mii = {};
		mii.cbSize		= sizeof(mii);
		mii.fMask		= MIIM_DATA|MIIM_ID|MIIM_FTYPE|MIIM_STRING|MIIM_SUBMENU;
		mii.dwTypeData	= szBuffer;
		mii.cch			= sizeof(szBuffer) / sizeof(TCHAR) - 1;

		GetMenuItemInfo( pMenu->GetSafeHmenu(), i, TRUE, &mii );

		if ( mii.wID >= ID_SHELL_MENU_MIN && mii.wID <= ID_SHELL_MENU_MAX )
			// Bypass shell menu items
			break;

		// Non-XML parsed menu items
		int nItemID = pMenu->GetMenuItemID( i );
		if ( nItemID == ID_SEARCH_FILTER ||
			nItemID == -1 && !m_sFilterString.IsEmpty() && m_sFilterString == szBuffer )
		{
			CResultFilters* pResultFilters = new CResultFilters;
			pResultFilters->Load();

			if ( nItemID > 0 )
			{
				m_sOldFilterString = szBuffer;
				m_sFilterString = szBuffer;
				m_sFilterString.TrimRight( L".\x2026" );
			}
			else if ( pResultFilters->m_nFilters == 0 )
			{
				CMenu* pSubMenu = pMenu->GetSubMenu( i );
				if ( pSubMenu )
					pSubMenu->DestroyMenu();

				mii.hSubMenu = NULL;
				mii.wID = ID_SEARCH_FILTER;

				m_sFilterString = m_sOldFilterString;
				SetMenuItemInfo( pMenu->GetSafeHmenu(), i, TRUE, &mii );
			}

			if ( pResultFilters->m_nFilters )
			{
				HMENU pFilters = CreatePopupMenu();
				DWORD nDefaultFilter = pResultFilters->m_nDefault;

				for ( DWORD nFilter = 0 ; nFilter < pResultFilters->m_nFilters ; nFilter++ )
				{
					AppendMenu( pFilters, MF_STRING|( nFilter == nDefaultFilter ? MF_CHECKED : 0 ),
						3000 + nFilter, pResultFilters->m_pFilters[ nFilter ]->m_sName );
				}
				ReplaceMenuText( pMenu, i, &mii, m_sFilterString.GetBuffer() );

				mii.hSubMenu = pFilters;
				mii.fMask |= MIIM_SUBMENU;
				strText = m_sFilterString;
			}
			else
			{
				ReplaceMenuText( pMenu, i, &mii, m_sOldFilterString.GetBuffer() );

				mii.hSubMenu = NULL;
				mii.fMask ^= MIIM_SUBMENU;
				strText = m_sOldFilterString;
			}

			delete pResultFilters;
		}

		if ( mii.fType & (MF_OWNERDRAW|MF_SEPARATOR) )
		{
			mii.fType |= MF_OWNERDRAW;
			if ( mii.fType & MF_SEPARATOR ) mii.dwItemData = 0;
			SetMenuItemInfo( pMenu->GetSafeHmenu(), i, TRUE, &mii );
			continue;
		}

		mii.fType		|= MF_OWNERDRAW;
		mii.dwItemData	= ( (DWORD_PTR)pMenu->GetSafeHmenu() << 16 ) | ( mii.wID & 0xFFFF );

		if ( strText.IsEmpty() )
			strText = szBuffer;

		m_pStrings.SetAt( mii.dwItemData, strText );

		if ( bChild ) SetMenuItemInfo( pMenu->GetSafeHmenu(), i, TRUE, &mii );

		if ( mii.hSubMenu != NULL )
			AddMenu( pMenu->GetSubMenu( i ), TRUE );
	}

	return TRUE;
}

BOOL CCoolMenu::ReplaceMenuText(CMenu* pMenu, int nPosition, MENUITEMINFO FAR* mii, LPCTSTR pszText)
{
	if ( !pMenu || mii == NULL )
		return FALSE;
	ASSERT( mii->dwTypeData );

	int nItemID = pMenu->GetMenuItemID( nPosition );

	if ( ! ModifyMenu( pMenu->GetSafeHmenu(), nPosition, MF_BYPOSITION|MF_STRING,
					 nItemID, pszText ) )
		return FALSE;

	mii->dwTypeData = (LPTSTR)pszText;

	mii->cch = CString( pszText ).GetLength() + 1;
	mii->fMask = MIIM_DATA|MIIM_ID|MIIM_FTYPE|MIIM_STRING;

	// We modified menu, retrieve a new MII (validates and changes data)
	if ( !GetMenuItemInfo( pMenu->GetSafeHmenu(), nPosition, TRUE, mii ) )
		return FALSE;

	// Replace the corresponding value in the collection
	mii->dwItemData	= ( (DWORD_PTR)pMenu->GetSafeHmenu() << 16 ) | ( mii->wID & 0xFFFF );
	CString strNew( (LPCTSTR)mii->dwTypeData );
	m_pStrings.SetAt( mii->dwItemData, strNew );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu watermark

void CCoolMenu::SetWatermark(HBITMAP hBitmap)
{
	if ( m_bmWatermark.m_hObject != NULL )
	{
		m_dcWatermark.SelectObject( CBitmap::FromHandle( m_hOldMark ) );
		m_bmWatermark.DeleteObject();
		m_dcWatermark.DeleteDC();
	}

	if ( hBitmap != NULL )
	{
		CDC dc;
		dc.Attach( GetDC( 0 ) );
		if ( Settings.General.LanguageRTL )
			SetLayout( dc.m_hDC, LAYOUT_BITMAPORIENTATIONPRESERVED );
		m_dcWatermark.CreateCompatibleDC( &dc );
		ReleaseDC( 0, dc.Detach() );

		m_bmWatermark.Attach( hBitmap );
		m_hOldMark = (HBITMAP)m_dcWatermark.SelectObject( &m_bmWatermark )->GetSafeHandle();

		BITMAP pInfo;
		m_bmWatermark.GetBitmap( &pInfo );
		m_czWatermark.cx = pInfo.bmWidth;
		m_czWatermark.cy = pInfo.bmHeight;
	}
}

void CCoolMenu::SetSelectmark(HBITMAP hBitmap)
{
	m_bSelectTest = FALSE;
	if ( hBitmap != NULL )
	{
		if ( m_bmSelectmark.m_hObject )
			m_bmSelectmark.DeleteObject();
		m_bmSelectmark.Attach( hBitmap );
		m_bSelectTest = TRUE;
	}
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu measure item

void CCoolMenu::OnMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if ( m_pContextMenu2 &&
		lpMeasureItemStruct->itemID >= ID_SHELL_MENU_MIN &&
		lpMeasureItemStruct->itemID <= ID_SHELL_MENU_MAX )
	{
		__try
		{
			m_pContextMenu2->HandleMenuMsg( WM_MEASUREITEM, 0,
				(LPARAM)lpMeasureItemStruct );
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
		}
	}
	else
		OnMeasureItemInternal( lpMeasureItemStruct );
}

void CCoolMenu::OnMeasureItemInternal(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if ( lpMeasureItemStruct->itemID == ID_SEPARATOR )
	{
		lpMeasureItemStruct->itemWidth	= 16;
		lpMeasureItemStruct->itemHeight	= 2;
	}
	else
	{
		CString strText;
		CDC dc;

		m_pStrings.Lookup( lpMeasureItemStruct->itemData, strText );

		dc.Attach( GetDC( 0 ) );

		CFont* pOld = (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );
		CSize sz = dc.GetTextExtent( strText );
		dc.SelectObject( pOld );

		ReleaseDC( 0, dc.Detach() );

		lpMeasureItemStruct->itemWidth	= sz.cx + 32;
		lpMeasureItemStruct->itemHeight	= 23;
	}

	if ( m_hMsgHook == NULL ) lpMeasureItemStruct->itemHeight ++;
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu draw item

void CCoolMenu::OnDrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( m_pContextMenu2 &&
		lpDrawItemStruct->itemID >= ID_SHELL_MENU_MIN &&
		lpDrawItemStruct->itemID <= ID_SHELL_MENU_MAX )
	{
		__try
		{
			m_pContextMenu2->HandleMenuMsg( WM_DRAWITEM, 0,
				(LPARAM)lpDrawItemStruct );
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
		}
	}
	else
		OnDrawItemInternal( lpDrawItemStruct );
}

void CCoolMenu::OnDrawItemInternal(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rcItem, rcText;
	CString strText;
	int nIcon = -1;
	CDC dc;

	BOOL	bSelected	= lpDrawItemStruct->itemState & ODS_SELECTED;
	BOOL	bChecked	= lpDrawItemStruct->itemState & ODS_CHECKED;
	BOOL	bDisabled	= lpDrawItemStruct->itemState & ODS_GRAYED;
	BOOL	bKeyboard	= FALSE;
	BOOL	bEdge		= TRUE;

	dc.Attach( lpDrawItemStruct->hDC );

	if ( CWnd* pWnd = dc.GetWindow() )
	{
		CRect rcScreen( &lpDrawItemStruct->rcItem );
		CPoint ptCursor;

		GetCursorPos( &ptCursor );
		pWnd->ClientToScreen( &rcScreen );

		bKeyboard = ! rcScreen.PtInRect( ptCursor );
	}

	rcItem.CopyRect( &lpDrawItemStruct->rcItem );
	rcItem.OffsetRect( -rcItem.left, -rcItem.top );
	if ( m_hMsgHook != NULL ) rcItem.bottom += ( bEdge = m_bPrinted );

	rcText.CopyRect( &rcItem );
	rcText.left += 32;
	rcText.right -= 2;

	CSize size = rcItem.Size();
	CDC* pDC = CoolInterface.GetBuffer( dc, size );

	if ( m_bmWatermark.m_hObject != NULL )
	{
		DrawWatermark( pDC, &rcItem, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top );
	}
	else
	{
		pDC->FillSolidRect( rcItem.left, rcItem.top, 24, rcItem.Height(), CoolInterface.m_crMargin );
		pDC->FillSolidRect( rcItem.left + 24, rcItem.top, rcItem.Width() - 24, rcItem.Height(), CoolInterface.m_crBackNormal );
	}

	if ( m_pStrings.Lookup( lpDrawItemStruct->itemData, strText ) == FALSE )
	{
		int nMiddle = rcText.top + 1;

		pDC->FillSolidRect( rcText.left, nMiddle, rcText.Width() + 2, 1, CoolInterface.m_crDisabled );

		dc.BitBlt( lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top,
			rcItem.Width(), rcItem.Height(), pDC, 0, 0, SRCCOPY );
		dc.Detach();

		return;
	}

	if ( bSelected )
	{
		SetSelectmark( Skin.GetWatermark( L"CCoolMenu.Hover" ) );
		if ( m_bSelectTest && ( !bDisabled || bKeyboard ) )
		{
			CoolInterface.DrawWatermark( pDC, &rcItem, &m_bmSelectmark );
		}
		else if ( ! bDisabled )
		{
			pDC->Draw3dRect( rcItem.left + 1, rcItem.top + 1,
				rcItem.Width() - 2, rcItem.Height() - 1 - bEdge,
				CoolInterface.m_crBorder, CoolInterface.m_crBorder );
			pDC->FillSolidRect( rcItem.left + 2, rcItem.top + 2,
				rcItem.Width() - 4, rcItem.Height() - 3 - bEdge,
				CoolInterface.m_crBackSel );

			pDC->SetBkColor( CoolInterface.m_crBackSel );
		}
		else if ( bKeyboard )
		{
			pDC->Draw3dRect( rcItem.left + 1, rcItem.top + 1,
				rcItem.Width() - 2, rcItem.Height() - 1 - bEdge,
				CoolInterface.m_crBorder, CoolInterface.m_crBorder );
			pDC->FillSolidRect( rcItem.left + 2, rcItem.top + 2,
				rcItem.Width() - 4, rcItem.Height() - 3 - bEdge,
				CoolInterface.m_crBackNormal );

			pDC->SetBkColor( CoolInterface.m_crBackNormal );
		}
	}
	else
	{
		pDC->SetBkColor( CoolInterface.m_crBackNormal );
	}

	if ( bChecked )
	{
		pDC->Draw3dRect( rcItem.left + 2, rcItem.top + 2, 20, rcItem.Height() - 3 - bEdge,
			CoolInterface.m_crBorder, CoolInterface.m_crBorder );
		pDC->FillSolidRect( rcItem.left + 3, rcItem.top + 3, 18, rcItem.Height() - 5 - bEdge,
			( bSelected && !bDisabled ) ? CoolInterface.m_crBackCheckSel : CoolInterface.m_crBackCheck );
	}

	nIcon = CoolInterface.ImageForID( (DWORD)lpDrawItemStruct->itemID );

	if ( bChecked && nIcon < 0 ) nIcon = CoolInterface.ImageForID( ID_CHECKMARK );

	if ( nIcon >= 0 )
	{
		CPoint pt( rcItem.left + 4, rcItem.top + 4 );

		if ( bDisabled )
		{
			CoolInterface.DrawEx( pDC, nIcon,
				pt, CSize( 0, 0 ), CLR_NONE, CoolInterface.m_crDisabled, ILD_BLEND50 );
		}
		else if ( bChecked )
		{
			CoolInterface.Draw( pDC, nIcon, pt, ILD_NORMAL );
		}
		else if ( bSelected )
		{
			pt.Offset( 1, 1 );
			pDC->SetTextColor( CoolInterface.m_crShadow );
			CoolInterface.Draw( pDC, nIcon, pt, ILD_MASK );
			pt.Offset( -2, -2 );
			CoolInterface.Draw( pDC, nIcon, pt, ILD_NORMAL );
		}
		else
		{
			CoolInterface.DrawEx( pDC, nIcon,
				pt, CSize( 0, 0 ), CLR_NONE, CoolInterface.m_crMargin, ILD_NORMAL );
		}
	}

	CFont* pOld = (CFont*)pDC->SelectObject(
					( lpDrawItemStruct->itemState & ODS_DEFAULT ) && ! bDisabled ?
					&CoolInterface.m_fntBold : &CoolInterface.m_fntNormal );

	pDC->SetBkMode( TRANSPARENT );
	pDC->SetTextColor( bDisabled ? CoolInterface.m_crDisabled :
		( bSelected ? CoolInterface.m_crCmdTextSel : CoolInterface.m_crCmdText ) );
	DrawMenuText( pDC, &rcText, strText );

	pDC->SelectObject( pOld );

	dc.BitBlt( lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top,
		rcItem.Width(), rcItem.Height(), pDC, 0, 0, SRCCOPY );
	dc.Detach();
}

LRESULT CCoolMenu::OnMenuChar(UINT nChar, UINT /*nFlags*/, CMenu* pMenu)
{
	LRESULT result = 0;
	if ( m_pContextMenu3 )
	{
		m_pContextMenu3->HandleMenuMsg2( WM_MENUCHAR,
			(WPARAM)MAKELONG( nChar, MF_POPUP ),
			(LPARAM)pMenu->GetSafeHmenu(), &result );
	}
	return result;
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu formatted text helper

void CCoolMenu::DrawMenuText(CDC* pDC, CRect* pRect, const CString& strText)
{
	theApp.m_bMenuWasVisible = TRUE;
	int nPos = strText.Find( '\t' );

	if ( nPos >= 0 )
	{
		pRect->right -= 8;
		pDC->DrawText( strText.Left( nPos ), pRect, DT_SINGLELINE|DT_VCENTER|DT_LEFT );
		pDC->DrawText( strText.Mid( nPos + 1 ), pRect, DT_SINGLELINE|DT_VCENTER|DT_RIGHT );
		pRect->right += 8;
	}
	else
	{
		pDC->DrawText( strText, pRect, DT_SINGLELINE|DT_VCENTER|DT_LEFT );
	}
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu watermark helper

void CCoolMenu::DrawWatermark(CDC* pDC, CRect* pRect, int nOffX, int nOffY)
{
	for ( int nY = pRect->top - nOffY ; nY < pRect->bottom ; nY += m_czWatermark.cy )
	{
		if ( nY + m_czWatermark.cy < pRect->top ) continue;

		for ( int nX = pRect->left - nOffX ; nX < pRect->right ; nX += m_czWatermark.cx )
		{
			if ( nX + m_czWatermark.cx < pRect->left ) continue;

			pDC->BitBlt( nX, nY, m_czWatermark.cx, m_czWatermark.cy, &m_dcWatermark, 0, 0, SRCCOPY );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu border effects

HHOOK	CCoolMenu::m_hMsgHook	= NULL;
LPCTSTR CCoolMenu::wpnOldProc	= _T("RAZA_MenuOldWndProc");
BOOL	CCoolMenu::m_bPrinted	= TRUE;
int		CCoolMenu::m_nEdgeLeft	= 0;
int		CCoolMenu::m_nEdgeTop	= 0;
int		CCoolMenu::m_nEdgeSize	= 0;

void CCoolMenu::EnableHook()
{
	EnableHook( Settings.Interface.CoolMenuEnable );
}

void CCoolMenu::EnableHook(bool bEnable)
{
	if ( bEnable == ( m_hMsgHook != NULL ) ) return;

	if ( bEnable )
	{
		m_hMsgHook = SetWindowsHookEx( WH_CALLWNDPROC, MsgHook,
			AfxGetInstanceHandle(), GetCurrentThreadId() );
	}
	else
	{
		UnhookWindowsHookEx( m_hMsgHook );
		m_hMsgHook = NULL;
	}
}

void CCoolMenu::RegisterEdge(int nLeft, int nTop, int nLength)
{
	m_nEdgeLeft	= nLeft;
	m_nEdgeTop	= nTop;
	m_nEdgeSize	= nLength;
}

static HRESULT SafeQueryContextMenu(IContextMenu* pContextMenu, HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags) throw()
{
	__try
	{
		return pContextMenu->QueryContextMenu( hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return E_UNEXPECTED;
	}
}

static BOOL SafeTrackPopupMenu(HMENU hMenu, UINT nFlags, POINT point, HWND hWnd) throw()
{
	__try
	{
		::SetForegroundWindow( hWnd );
		BOOL nCmd = ::TrackPopupMenu( hMenu, nFlags, point.x, point.y, 0, hWnd, NULL );
		::PostMessage( hWnd, WM_NULL, 0, 0 );
		return nCmd;
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return 0;
	}
}

BOOL CCoolMenu::DoExplorerMenu(HWND hwnd, const CStringList& oFiles, POINT point, HMENU hMenu, HMENU hSubMenu, UINT nFlags)
{
	BOOL nCmd = 0;
	HRESULT hr = S_OK;
	CComPtr< IContextMenu > pContextMenu1;
	CShellList oItemIDListList( oFiles );
	oItemIDListList.GetMenu( hwnd, (void**)&pContextMenu1 );
	if ( pContextMenu1 )
	{
		CWaitCursor wc;
		hr = SafeQueryContextMenu( pContextMenu1, hSubMenu, 0,
			ID_SHELL_MENU_MIN, ID_SHELL_MENU_MAX, CMF_NORMAL | CMF_EXPLORE );
	}
	if ( SUCCEEDED( hr ) )
	{
		if ( pContextMenu1 )
		{
			pContextMenu1.QueryInterface( &m_pContextMenu2 );
			pContextMenu1.QueryInterface( &m_pContextMenu3 );
		}

		nCmd = SafeTrackPopupMenu( hMenu, TPM_RETURNCMD | nFlags, point, hwnd );

		// If a command was selected from the shell menu, execute it.
		if ( pContextMenu1 && nCmd >= ID_SHELL_MENU_MIN && nCmd <= ID_SHELL_MENU_MAX )
		{
			CMINVOKECOMMANDINFOEX ici = {};
			ici.cbSize = sizeof( CMINVOKECOMMANDINFOEX );
			ici.fMask = CMIC_MASK_ASYNCOK | CMIC_MASK_NOZONECHECKS | CMIC_MASK_FLAG_LOG_USAGE;
			ici.hwnd = hwnd;
			ici.lpVerb = (LPCSTR)(DWORD_PTR)( nCmd - ID_SHELL_MENU_MIN );
			ici.lpVerbW = (LPCWSTR)(DWORD_PTR)( nCmd - ID_SHELL_MENU_MIN );
			ici.nShow = SW_SHOWNORMAL;
			pContextMenu1->InvokeCommand( (CMINVOKECOMMANDINFO*)&ici );
		}
		else if ( ( nFlags & TPM_RETURNCMD ) == 0 )
		{
			// Emulate normal message handling
			::PostMessage( hwnd, WM_COMMAND, nCmd, 0 );
		}

		m_pContextMenu3.Release();
		m_pContextMenu2.Release();
	}
	CComPtr< IContextMenu > pContextMenuCache;
	pContextMenuCache = m_pContextMenuCache;
	m_pContextMenuCache = pContextMenu1;

	// TODO: Find why sometimes raza crashes inside Windows Shell SetSite() function
	SafeRelease( pContextMenuCache );

	return nCmd;
}

//////////////////////////////////////////////////////////////////////
// CCoolMenu message hook and subclassed window procedure

LRESULT CALLBACK CCoolMenu::MsgHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT* pCWP = (CWPSTRUCT*)lParam;

	while ( nCode == HC_ACTION )
	{
		if ( pCWP->message != WM_CREATE && pCWP->message != 0x01E2 ) break;

		TCHAR szClassName[16];
		int nClassName = GetClassName( pCWP->hwnd, szClassName, 16 );
		if ( nClassName != 6 || _tcscmp( szClassName, _T("#32768") ) != 0 ) break;

		if ( ::GetProp( pCWP->hwnd, wpnOldProc ) != NULL ) break;

		HWND hWndFore = GetForegroundWindow();
		if ( hWndFore != NULL && CWnd::FromHandlePermanent( hWndFore ) == NULL ) break;

		WNDPROC pWndProc = (WNDPROC)(LONG_PTR)::GetWindowLongPtr( pCWP->hwnd, GWLP_WNDPROC );
		if ( pWndProc == NULL ) break;
		ASSERT( pWndProc != MenuProc );

		if ( ! SetProp( pCWP->hwnd, wpnOldProc, pWndProc ) ) break;

		if ( ! SetWindowLongPtr( pCWP->hwnd, GWLP_WNDPROC, (LONG_PTR)MenuProc ) )
		{
			::RemoveProp( pCWP->hwnd, wpnOldProc );
			break;
		}

		break;
	}

	__try
	{
		return CallNextHookEx( CCoolMenu::m_hMsgHook, nCode, wParam, lParam );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return 0;
	}
}

LRESULT CALLBACK CCoolMenu::MenuProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC pWndProc = (WNDPROC)::GetProp( hWnd, wpnOldProc );

	switch ( uMsg )
	{
	case WM_NCCALCSIZE:
		{
			NCCALCSIZE_PARAMS* pCalc = (NCCALCSIZE_PARAMS*)lParam;
			pCalc->rgrc[0].left ++;
			pCalc->rgrc[0].top ++;
			pCalc->rgrc[0].right --;
			pCalc->rgrc[0].bottom --;
		}
		return 0;

	case WM_WINDOWPOSCHANGING:
		if ( WINDOWPOS* pWndPos = (WINDOWPOS*)lParam )
		{
			DWORD nStyle	= (DWORD)GetWindowLongPtr( hWnd, GWL_STYLE );
			DWORD nExStyle	= (DWORD)GetWindowLongPtr( hWnd, GWL_EXSTYLE );
			CRect rc( 0, 0, 32, 32 );

			AdjustWindowRectEx( &rc, nStyle, FALSE, nExStyle );

			pWndPos->cx -= ( rc.Width() - 34 );
			pWndPos->cy -= ( rc.Height() - 34 ) - 1;

			if ( pWndPos->x != m_nEdgeLeft || pWndPos->y != m_nEdgeTop )
				pWndPos->x ++;
		}
		break;

	case WM_PRINT:
		if ( ( lParam & PRF_CHECKVISIBLE ) && ! IsWindowVisible( hWnd ) ) return 0;
		if ( lParam & PRF_NONCLIENT )
		{
			CWnd* pWnd = CWnd::FromHandle( hWnd );
			CDC* pDC = CDC::FromHandle( (HDC)wParam );
			CRect rc;

			pWnd->GetWindowRect( &rc );
			BOOL bEdge = ( rc.left == m_nEdgeLeft && rc.top == m_nEdgeTop );
			rc.OffsetRect( -rc.left, -rc.top );

			pDC->Draw3dRect( &rc, CoolInterface.m_crDisabled, CoolInterface.m_crDisabled );
			if ( bEdge ) pDC->FillSolidRect( rc.left + 1, rc.top, min( rc.Width(), m_nEdgeSize ) - 2, 1, CoolInterface.m_crBackNormal );
		}
		if ( lParam & PRF_CLIENT )
		{
			CWnd* pWnd = CWnd::FromHandle( hWnd );
			CDC* pDC = CDC::FromHandle( (HDC)wParam );
			CBitmap bmBuf, *pbmOld;
			CDC dcBuf;
			CRect rc;

			pWnd->GetClientRect( &rc );
			dcBuf.CreateCompatibleDC( pDC );
			bmBuf.CreateCompatibleBitmap( pDC, rc.Width(), rc.Height() );
			pbmOld = (CBitmap*)dcBuf.SelectObject( &bmBuf );

			m_bPrinted = TRUE;
			dcBuf.FillSolidRect( &rc, GetSysColor( COLOR_MENU ) );
			SendMessage( hWnd, WM_PRINTCLIENT, (WPARAM)dcBuf.GetSafeHdc(), 0 );

			pDC->BitBlt( 1, 1, rc.Width(), rc.Height(), &dcBuf, 0, 0, SRCCOPY );
			dcBuf.SelectObject( pbmOld );
		}
		return 0;

	case WM_NCPAINT:
		{
			CWnd* pWnd = CWnd::FromHandle( hWnd );
			CWindowDC dc( pWnd );
			CRect rc;

			pWnd->GetWindowRect( &rc );
			BOOL bEdge = ( rc.left == m_nEdgeLeft && rc.top == m_nEdgeTop );
			rc.OffsetRect( -rc.left, -rc.top );

			dc.Draw3dRect( &rc, CoolInterface.m_crDisabled, CoolInterface.m_crDisabled );
			if ( bEdge ) dc.FillSolidRect( rc.left + 1, rc.top, min( rc.Width(), m_nEdgeSize ) - 2, 1, CoolInterface.m_crBackNormal );
		}
		return 0;

	case WM_PAINT:
		m_bPrinted = FALSE;
		break;

	case WM_NCDESTROY:
		::RemoveProp( hWnd, wpnOldProc );
		break;
	}

	return CallWindowProc( pWndProc, hWnd, uMsg, wParam, lParam );
}
