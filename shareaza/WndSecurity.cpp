//
// WndSecurity.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "Security.h"
#include "LiveList.h"
#include "WndSecurity.h"
#include "DlgSecureRule.h"
#include "CoolInterface.h"
#include "SchemaCache.h"
#include "XML.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const static UINT nImageID[] =
{
	IDR_SECURITYFRAME,
	IDI_GRANTED,
	IDI_FIREWALLED,
	NULL
};

IMPLEMENT_SERIAL(CSecurityWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CSecurityWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_RULES, OnCustomDrawList)
	ON_NOTIFY(NM_DBLCLK, IDC_RULES, OnDblClkList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_RULES, OnSortList)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_EDIT, OnUpdateSecurityEdit)
	ON_COMMAND(ID_SECURITY_EDIT, OnSecurityEdit)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_RESET, OnUpdateSecurityReset)
	ON_COMMAND(ID_SECURITY_RESET, OnSecurityReset)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_REMOVE, OnUpdateSecurityRemove)
	ON_COMMAND(ID_SECURITY_REMOVE, OnSecurityRemove)
	ON_COMMAND(ID_SECURITY_ADD, OnSecurityAdd)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_POLICY_ACCEPT, OnUpdateSecurityPolicyAccept)
	ON_COMMAND(ID_SECURITY_POLICY_ACCEPT, OnSecurityPolicyAccept)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_POLICY_DENY, OnUpdateSecurityPolicyDeny)
	ON_COMMAND(ID_SECURITY_POLICY_DENY, OnSecurityPolicyDeny)
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_SECURITY_MOVE_UP, OnUpdateSecurityMoveUp)
	ON_COMMAND(ID_SECURITY_MOVE_UP, OnSecurityMoveUp)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_MOVE_DOWN, OnUpdateSecurityMoveDown)
	ON_COMMAND(ID_SECURITY_MOVE_DOWN, OnSecurityMoveDown)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_EXPORT, OnUpdateSecurityExport)
	ON_COMMAND(ID_SECURITY_EXPORT, OnSecurityExport)
	ON_COMMAND(ID_SECURITY_IMPORT, OnSecurityImport)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSecurityWnd construction

CSecurityWnd::CSecurityWnd()
{
	Create( IDR_SECURITYFRAME );
}

CSecurityWnd::~CSecurityWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSecurityWnd message handlers

int CSecurityWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );

	m_wndList.Create( WS_VISIBLE|LVS_ICON|LVS_AUTOARRANGE|LVS_REPORT|LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_RULES );

	m_pSizer.Attach( &m_wndList );
	
	m_wndList.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP,
		LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP );
	
	CoolInterface.LoadIconsTo( m_gdiImageList, nImageID );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	m_wndList.InsertColumn( 0, _T("Address / Content"), LVCFMT_LEFT, 200, -1 );
	m_wndList.InsertColumn( 1, _T("Action"), LVCFMT_CENTER, 100, 0 );
	m_wndList.InsertColumn( 2, _T("Expires"), LVCFMT_CENTER, 100, 1 );
	m_wndList.InsertColumn( 3, _T("Precedence"), LVCFMT_CENTER, 00, 2 );
	m_wndList.InsertColumn( 4, _T("Hits"), LVCFMT_CENTER, 60, 3 );
	m_wndList.InsertColumn( 5, _T("Comment"), LVCFMT_LEFT, 100, 4 );

	m_wndList.SetFont( &theApp.m_gdiFont );
	
	LoadState( _T("CSecurityWnd"), TRUE );

	Update();

	return 0;
}

void CSecurityWnd::OnDestroy() 
{
	Security.Save();

	Settings.SaveList( _T("CSecurityWnd"), &m_wndList );		
	SaveState( _T("CSecurityWnd") );

	CPanelWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CSecurityWnd operations

void CSecurityWnd::Update(int nColumn, BOOL bSort)
{
	Security.Expire();

	CAutoPtr< CLiveList > pLiveList( Security.GetList() );

	if ( nColumn >= 0 )
	{
		SetWindowLongPtr( m_wndList.GetSafeHwnd(), GWLP_USERDATA, 0 - nColumn - 1 );
	}

	pLiveList->Apply( &m_wndList, bSort );

	tLastUpdate = GetTickCount();				// Update time after it's done doing its work
}

CSecureRule* CSecurityWnd::GetItem(int nItem)
{
	if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
	{
		CSecureRule* pRule = (CSecureRule*)m_wndList.GetItemData( nItem );
		if ( Security.Check( pRule ) ) return pRule;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CSecurityWnd message handlers

void CSecurityWnd::OnSize(UINT nType, int cx, int cy) 
{
	CPanelWnd::OnSize( nType, cx, cy );
	SizeListAndBar( &m_wndList, &m_wndToolBar );
	m_wndList.SetWindowPos( NULL, 0, 0, cx, cy - 28, SWP_NOZORDER );
}

void CSecurityWnd::OnTimer(UINT_PTR nIDEvent) 
{
	if ( ( nIDEvent == 1 ) && ( IsPartiallyVisible() ) )
	{
		DWORD tTicks = GetTickCount();
		DWORD tDelay = max( ( 2 * (DWORD)Security.GetCount() ), 1000ul ); // Delay based on size of list

		if ( ( tTicks - tLastUpdate ) > tDelay )
		{
			if ( tDelay < 2000 ) Update();			// Sort if list is under 1000
			else Update( -1, FALSE );				// Otherwise just refresh values
		}
	}
}

void CSecurityWnd::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		LV_ITEM pItem;
		pItem.mask		= LVIF_IMAGE;
		pItem.iItem		= static_cast< int >( pDraw->nmcd.dwItemSpec );
		pItem.iSubItem	= 0;
		m_wndList.GetItem( &pItem );

		switch ( pItem.iImage )
		{
		case CSecureRule::srAccept:
			pDraw->clrText = CoolInterface.m_crSecurityAllow ;
			break;
		case CSecureRule::srDeny:
			pDraw->clrText = CoolInterface.m_crSecurityDeny ;
			break;
		}

		*pResult = CDRF_DODEFAULT;
	}
}

void CSecurityWnd::OnDblClkList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnSecurityEdit();
	*pResult = 0;
}

void CSecurityWnd::OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNotifyStruct;
	CLiveList::Sort( &m_wndList, pNMListView->iSubItem );
	*pResult = 0;
}

void CSecurityWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{
	Skin.TrackPopupMenu( _T("CSecurityWnd"), point, ID_SECURITY_EDIT );
}

void CSecurityWnd::OnUpdateSecurityEdit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() == 1 && m_wndList.GetItemCount() > 1 );
}

void CSecurityWnd::OnSecurityEdit() 
{
	if ( m_wndList.GetSelectedCount() != 1 || m_wndList.GetItemCount() <= 1 )
		return;

	CSecureRule* pEditableRule;
	{
		CQuickLock oLock( Security.m_pSection );
		
		CSecureRule* pRule = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
		if ( ! pRule ) return;
		pEditableRule = new CSecureRule( *pRule ); 
	}

	CSecureRuleDlg dlg( NULL, pEditableRule );
	if ( dlg.DoModal() == IDOK )
	{
		Security.Save();
		Update();
	}
	else
		delete pEditableRule;
}

void CSecurityWnd::OnUpdateSecurityReset(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CSecurityWnd::OnSecurityReset() 
{
	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		CQuickLock oLock( Security.m_pSection );

		if ( CSecureRule* pRule = GetItem( nItem ) )
		{
			pRule->Reset();
		}
	}

	Security.Save();
	Update();
}

void CSecurityWnd::OnUpdateSecurityRemove(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 && m_wndList.GetItemCount() > 1 );
}

void CSecurityWnd::OnSecurityRemove() 
{
	if ( m_wndList.GetSelectedCount() <= 0 || m_wndList.GetItemCount() <= 1 )
		return;

	CString strMessage;
	LoadString( strMessage, IDS_SECURITY_REMOVE_CONFIRM );
	if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;

	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		CQuickLock oLock( Security.m_pSection );

		if ( CSecureRule* pRule = GetItem( nItem ) )
		{
			Security.Remove( pRule );
		}
	}

	Security.Save();
	Update();
}

void CSecurityWnd::OnUpdateSecurityMoveUp(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CSecurityWnd::OnSecurityMoveUp() 
{
	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		CQuickLock oLock( Security.m_pSection );

		if ( CSecureRule* pRule = GetItem( nItem ) )
		{
			Security.MoveUp( pRule );
		}
	}

	Security.Save();
	Update( 3 );
}

void CSecurityWnd::OnUpdateSecurityMoveDown(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CSecurityWnd::OnSecurityMoveDown() 
{
	CQuickLock oLock( Security.m_pSection );

	CList< CSecureRule* > pList;

	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		pList.AddHead( GetItem( nItem ) );
	}

	while ( pList.GetCount() )
	{
		CSecureRule* pRule = pList.RemoveHead();
		if ( pRule ) Security.MoveDown( pRule );
	}

	Security.Save();
	Update( 3 );
}

void CSecurityWnd::OnSecurityAdd() 
{
	CSecureRuleDlg dlg;

	if ( dlg.DoModal() == IDOK )
	{
		Security.Save();
		Update();
	}
}

void CSecurityWnd::OnUpdateSecurityExport(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CSecurityWnd::OnSecurityExport() 
{
	CFileDialog dlg( FALSE, _T("xml"), NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		_T("XML Security Files|*.xml|NET Security Files|*.net|") +
		SchemaCache.GetFilter( CSchema::uriAllFiles ) +
		_T("|"), this );
	
	if ( dlg.DoModal() != IDOK ) return;
	
	CString strText;
	CFile pFile;
	
	if ( ! pFile.Open( dlg.GetPathName(), CFile::modeWrite|CFile::modeCreate ) )
	{
		// TODO: Error
		AfxMessageBox( _T("Error") );
		return;
	}

	CWaitCursor pCursor;
	
	if ( dlg.GetFileExt().CompareNoCase( _T("net") ) == 0 )
	{
		for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
		{
			CQuickLock oLock( Security.m_pSection );

			if ( CSecureRule* pRule = GetItem( nItem ) )
			{
				strText = pRule->ToGnucleusString();

				if ( strText.GetLength() )
				{
					strText += _T("\r\n");

					int nBytes = WideCharToMultiByte( CP_ACP, 0, strText, strText.GetLength(), NULL, 0, NULL, NULL );
					LPSTR pBytes = new CHAR[nBytes];
					WideCharToMultiByte( CP_ACP, 0, strText, strText.GetLength(), pBytes, nBytes, NULL, NULL );
					pFile.Write( pBytes, nBytes );
					delete [] pBytes;

				}
			}
		}
	}
	else
	{
		auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, _T("security") ) );

		pXML->AddAttribute( _T("xmlns"), CSecurity::xmlns );

		for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
		{
			CQuickLock oLock( Security.m_pSection );

			if ( CSecureRule* pRule = GetItem( nItem ) )
			{
				pXML->AddElement( pRule->ToXML() );
			}
		}

		strText = pXML->ToString( TRUE, TRUE );

		int nBytes = WideCharToMultiByte( CP_ACP, 0, strText, strText.GetLength(), NULL, 0, NULL, NULL );
		auto_array< CHAR > pBytes( new CHAR[ nBytes ] );
		WideCharToMultiByte( CP_ACP, 0, strText, strText.GetLength(), pBytes.get(), nBytes, NULL, NULL );
		pFile.Write( pBytes.get(), nBytes );
	}

	pFile.Close();
}

void CSecurityWnd::OnSecurityImport() 
{
	CFileDialog dlg( TRUE, _T("xml"), NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		_T("Security Rules|*.xml;*.net|") +
		SchemaCache.GetFilter( CSchema::uriAllFiles ) +
		_T("|"), this );
	
	if ( dlg.DoModal() != IDOK ) return;

	CWaitCursor pCursor;

	if ( Security.Import( dlg.GetPathName() ) )
	{
		Security.Save();
	}
	else
	{
		// TODO: Error message, unable to import rules
		AfxMessageBox( _T("Error") );
	}
}

void CSecurityWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();

	Settings.LoadList( _T("CSecurityWnd"), &m_wndList, -4 );
	Skin.CreateToolBar( _T("CSecurityWnd"), &m_wndToolBar );

	CoolInterface.LoadIconsTo( m_gdiImageList, nImageID );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
}

void CSecurityWnd::OnUpdateSecurityPolicyAccept(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( Security.m_bDenyPolicy == FALSE );
}

void CSecurityWnd::OnSecurityPolicyAccept() 
{
	Security.m_bDenyPolicy = FALSE;
	Update();
	m_wndList.RedrawItems( 0, m_wndList.GetItemCount() - 1 );
}

void CSecurityWnd::OnUpdateSecurityPolicyDeny(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( Security.m_bDenyPolicy == TRUE );
}

void CSecurityWnd::OnSecurityPolicyDeny() 
{
	Security.m_bDenyPolicy = TRUE;
	Update();
	m_wndList.RedrawItems( 0, m_wndList.GetItemCount() - 1 );
}


BOOL CSecurityWnd::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
		{
			if ( pMsg->wParam == VK_UP )
			{
				PostMessage( WM_COMMAND, ID_SECURITY_MOVE_UP );
				return TRUE;
			}
			else if ( pMsg->wParam == VK_DOWN )
			{
				PostMessage( WM_COMMAND, ID_SECURITY_MOVE_DOWN );
				return TRUE;
			}
		}
		else if ( pMsg->wParam == VK_DELETE )
		{
			PostMessage( WM_COMMAND, ID_SECURITY_REMOVE );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_INSERT )
		{
			PostMessage( WM_COMMAND, ID_SECURITY_ADD );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_RETURN )
		{
			PostMessage( WM_COMMAND, ID_SECURITY_EDIT );
			return TRUE;
		}
	}

	return CPanelWnd::PreTranslateMessage( pMsg );
}
