//
// DlgGraphList.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
#include "GraphLine.h"
#include "GraphItem.h"
#include "DlgGraphList.h"
#include "DlgGraphItem.h"
#include "LiveList.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CGraphListDlg, CSkinDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_GRAPH_ITEMS, OnItemChangedGraphItems)
	ON_BN_CLICKED(IDC_GRAPH_ADD, OnGraphAdd)
	ON_BN_CLICKED(IDC_GRAPH_EDIT, OnGraphEdit)
	ON_BN_CLICKED(IDC_GRAPH_REMOVE, OnGraphRemove)
	ON_NOTIFY(NM_DBLCLK, IDC_GRAPH_ITEMS, OnDblClkGraphItems)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_GRAPH_ITEMS, OnCustomDrawItems)
END_MESSAGE_MAP()

#define LIST_COLUMNS	1
#define SPEED_MINIMUM	10ul

/////////////////////////////////////////////////////////////////////////////
// CGraphListDlg dialog

CGraphListDlg::CGraphListDlg(CWnd* pParent, CLineGraph* pGraph) : CSkinDialog(CGraphListDlg::IDD, pParent)
, m_nSpeed( SPEED_MINIMUM )
, m_bShowGrid( FALSE )
, m_bShowAxis( FALSE )
, m_bShowLegend( FALSE )
, m_pGraph( pGraph )
{
}

void CGraphListDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_GRAPH_SPEED_SPIN, m_wndSpeed);
	DDX_Control(pDX, IDC_GRAPH_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_GRAPH_EDIT, m_wndEdit);
	DDX_Control(pDX, IDC_GRAPH_ITEMS, m_wndList);
	DDX_Text(pDX, IDC_GRAPH_SPEED, m_nSpeed);
	DDX_Check(pDX, IDC_GRAPH_GRID, m_bShowGrid);
	DDX_Check(pDX, IDC_GRAPH_AXIS, m_bShowAxis);
	DDX_Check(pDX, IDC_GRAPH_LEGEND, m_bShowLegend);
	DDX_Text(pDX, IDC_NAME, m_sName);
}

/////////////////////////////////////////////////////////////////////////////
// CGraphListDlg message handlers

BOOL CGraphListDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CGraphListDlg"), IDR_TRAFFICFRAME );

	m_gdiImageList.Create( IDB_COLOURDOT, 16, 0, RGB(0,255,0) );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	m_wndSpeed.SendMessage( UDM_SETRANGE32, SPEED_MINIMUM, 120000 );

	m_bShowAxis		= m_pGraph->m_bShowAxis;
	m_bShowGrid		= m_pGraph->m_bShowGrid;
	m_bShowLegend	= m_pGraph->m_bShowLegend;
	m_nSpeed		= max( m_pGraph->m_nSpeed, SPEED_MINIMUM );

	UpdateData( FALSE );

	for ( POSITION pos = m_pGraph->GetItemIterator() ; pos ; )
	{
		CGraphItem* pItem = m_pGraph->GetNextItem( pos );
		CLiveItem* pLive = PrepareItem( pItem );
		pLive->Add( &m_wndList, -1, LIST_COLUMNS );
		delete pLive;
	}

	OnItemChangedGraphItems( NULL, NULL );

	return TRUE;
}

CLiveItem* CGraphListDlg::PrepareItem(CGraphItem* pItem)
{
	CLiveItem* pLive = new CLiveItem( LIST_COLUMNS, reinterpret_cast< DWORD_PTR>( pItem ) );
	pLive->SetImage( 0, I_IMAGECALLBACK );

	pLive->Set( 0, pItem->m_sName );

	return pLive;
}

void CGraphListDlg::OnCustomDrawItems(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == (CDDS_ITEM|CDDS_PREPAINT) )
	{
		CGraphItem* pItem = (CGraphItem*)pDraw->nmcd.lItemlParam;
		CDC* pDC = CDC::FromHandle( pDraw->nmcd.hdc );

		CRect rc;
		m_wndList.GetItemRect( static_cast< int >( pDraw->nmcd.dwItemSpec ), &rc, LVIR_ICON );

		pDC->SetTextColor( pItem->m_nColour );
		m_gdiImageList.Draw( pDC, 1, rc.TopLeft(), ILD_NORMAL );
		m_gdiImageList.Draw( pDC, 0, rc.TopLeft(), ILD_MASK|ILD_TRANSPARENT );

		*pResult = CDRF_DODEFAULT;
	}
}

void CGraphListDlg::OnItemChangedGraphItems(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if ( pResult ) *pResult = 0;
	int nSelected = m_wndList.GetSelectedCount();
	m_wndEdit.EnableWindow( nSelected == 1 );
	m_wndRemove.EnableWindow( nSelected > 0 );
}

void CGraphListDlg::OnDblClkGraphItems(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnGraphEdit();
	*pResult = 0;
}

void CGraphListDlg::OnGraphAdd()
{
	CGraphItemDlg dlg( this, new CGraphItem() );

	if ( dlg.DoModal() == IDOK )
	{
		m_pGraph->AddItem( dlg.m_pItem );
		CLiveItem* pLive = PrepareItem( dlg.m_pItem );
		pLive->Add( &m_wndList, -1, LIST_COLUMNS );
		delete pLive;
		SetModified();
	}
	else
	{
		delete dlg.m_pItem;
	}
}

void CGraphListDlg::OnGraphEdit()
{
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem < 0 ) return;

	CGraphItemDlg dlg( this, (CGraphItem*)m_wndList.GetItemData( nItem ) );

	if ( dlg.DoModal() == IDOK )
	{
		CLiveItem* pLive = PrepareItem( dlg.m_pItem );
		pLive->Update( &m_wndList, nItem, LIST_COLUMNS );
		delete pLive;
		SetModified();
	}
}

void CGraphListDlg::OnGraphRemove()
{
	for ( int nItem = m_wndList.GetItemCount() - 1 ; nItem >= 0 ; nItem -- )
	{
		if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
		{
			CGraphItem* pItem = (CGraphItem*)m_wndList.GetItemData( nItem );
			m_wndList.DeleteItem( nItem );
			m_pGraph->RemoveItem( pItem );
			m_pGraph->ResetMaximum();
			SetModified();
		}
	}
}

void CGraphListDlg::SetModified()
{
	CString sText;
	if ( m_wndCancel.IsWindowEnabled() )
	{
		m_wndCancel.EnableWindow( FALSE );
		LoadString( sText, IDS_GENERAL_CLOSE );
		m_wndOK.SetWindowText( sText );
	}
}

void CGraphListDlg::OnOK()
{
	UpdateData();

	m_pGraph->m_bShowAxis		= m_bShowAxis;
	m_pGraph->m_bShowGrid		= m_bShowGrid;
	m_pGraph->m_bShowLegend		= m_bShowLegend;
	m_pGraph->m_nSpeed			= max( m_nSpeed, SPEED_MINIMUM );

	m_pGraph->ResetMaximum();

	CSkinDialog::OnOK();
}
