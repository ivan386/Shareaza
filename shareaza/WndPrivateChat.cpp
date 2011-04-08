//
// WndPrivateChat.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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
#include "ChatCore.h"
#include "ChatSession.h"
#include "CoolInterface.h"
#include "GProfile.h"
#include "RichElement.h"
#include "Skin.h"
#include "Transfers.h"
#include "UploadTransfer.h"
#include "Uploads.h"
#include "WndBrowseHost.h"
#include "WndPrivateChat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CPrivateChatWnd, CChatWnd)

BEGIN_MESSAGE_MAP(CPrivateChatWnd, CChatWnd)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_CHAT_CONNECT, &CPrivateChatWnd::OnUpdateChatConnect)
	ON_COMMAND(ID_CHAT_CONNECT, &CPrivateChatWnd::OnChatConnect)
	ON_UPDATE_COMMAND_UI(ID_CHAT_DISCONNECT, &CPrivateChatWnd::OnUpdateChatDisconnect)
	ON_COMMAND(ID_CHAT_DISCONNECT, &CPrivateChatWnd::OnChatDisconnect)
	ON_UPDATE_COMMAND_UI(ID_CHAT_BROWSE, &CPrivateChatWnd::OnUpdateChatBrowse)
	ON_COMMAND(ID_CHAT_BROWSE, &CPrivateChatWnd::OnChatBrowse)
	ON_UPDATE_COMMAND_UI(ID_CHAT_PRIORITY, &CPrivateChatWnd::OnUpdateChatPriority)
	ON_COMMAND(ID_CHAT_PRIORITY, &CPrivateChatWnd::OnChatPriority)
	ON_MESSAGE(WM_CHAT_MESSAGE, &CPrivateChatWnd::OnRemoteMessage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrivateChatWnd construction

CPrivateChatWnd::CPrivateChatWnd()
	: m_pSession	( NULL )
{
	Create( IDR_CHATFRAME, TRUE );
}

CPrivateChatWnd::~CPrivateChatWnd()
{
	ASSERT( m_pSession == NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CPrivateChatWnd operations

void CPrivateChatWnd::Setup(LPCTSTR szNick)
{
	m_sNick = szNick;

	// Open window
	Open();

	// Put a 'connecting' message in the window
	CString strMessage;
	strMessage.Format( LoadString( IDS_CHAT_CONNECTING_TO ), m_sNick );
	CChatWnd::OnStatusMessage( 0, strMessage );
}

void CPrivateChatWnd::Setup(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush, PROTOCOLID nProtocol)
{
	ASSERT( m_pSession == NULL );

	m_pSession = new CChatSession( nProtocol, this );
	m_pSession->m_oGUID		= oGUID;
	m_pSession->m_pHost		= *pHost;
	m_pSession->m_bMustPush	= bMustPush;
	m_pSession->m_sNick = HostToString( pHost );
}

BOOL CPrivateChatWnd::Accept(CChatSession* pSession)
{
	if ( m_pSession )
	{
		if ( m_pSession->m_nState > cssConnecting )
			return FALSE;

		m_pSession->OnCloseWindow();
	}

	m_pSession = pSession;

	return TRUE;
}

BOOL CPrivateChatWnd::Find(const IN_ADDR* pAddress) const
{
	if ( m_pSession )
	{
		// Regular chat window that matches
		return ( m_pSession->m_pHost.sin_addr.s_addr == pAddress->s_addr ) ||
		// ED2K Low ID chat window that matches
			( m_pSession->m_bMustPush &&
			  m_pSession->m_nProtocol == PROTOCOL_ED2K &&
			  m_pSession->m_nClientID == pAddress->s_addr );
	}
	return FALSE;
}

BOOL CPrivateChatWnd::Find(const Hashes::Guid& oGUID, bool bLive) const
{
	if ( m_pSession && validAndEqual( m_pSession->m_oGUID, oGUID ) )
	{
		if ( bLive )
			return ( m_pSession->m_nState > cssConnecting );
		else
			return ( m_pSession->m_nState <= cssConnecting );
	}
	return FALSE;
}

BOOL CPrivateChatWnd::Find(const CString& sNick) const
{
	return ( sNick == m_sNick && m_pSession == NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CPrivateChatWnd event handlers

void CPrivateChatWnd::OnDestroy()
{
	{
		CQuickLock pLock( ChatCore.m_pSection );

		if ( m_pSession != NULL )
		{
			m_pSession->OnCloseWindow();
			m_pSession = NULL;
		}
	}

	CChatWnd::OnDestroy();
}

BOOL CPrivateChatWnd::OnLocalMessage(bool bAction, const CString& sText)
{
	CString sChatID;
	if ( m_pSession )
	{
		m_sNick = m_pSession->m_sNick;
		sChatID = HostToString( &m_pSession->m_pHost );
	}
	else
		sChatID = _T("Private:") + m_sNick;

	CChatWnd::OnMessage( bAction, sChatID, true, MyProfile.GetNick(), m_sNick, sText );

	if ( ! m_pSession )
		return FALSE;

	return m_pSession->SendPrivateMessage( bAction, sText );
}

BOOL CPrivateChatWnd::OnLocalCommand(const CString& sCommand, const CString& sArgs)
{
	if ( sCommand.CompareNoCase( _T("/connect") ) == 0 )
	{
		PostMessage( WM_COMMAND, ID_CHAT_CONNECT );
	}
	else if ( sCommand.CompareNoCase( _T("/disconnect") ) == 0 )
	{
		PostMessage( WM_COMMAND, ID_CHAT_DISCONNECT );
	}
	else if ( sCommand.CompareNoCase( _T("/browse") ) == 0 )
	{
		PostMessage( WM_COMMAND, ID_CHAT_BROWSE );
	}
	else
	{
		return CChatWnd::OnLocalCommand( sCommand, sArgs );
	}
	return TRUE;
}


LRESULT CPrivateChatWnd::OnRemoteMessage(WPARAM /*wParam*/, LPARAM lParam)
{
	CAutoPtr< CChatMessage > pMsg( (CChatMessage*)lParam );

	if ( pMsg->m_bType == cmtProfile )
	{
		m_sNick = pMsg->m_sFrom;

		CChatWnd::AddLogin( m_sNick );

		CString strCaption;
		LoadString( strCaption, IDR_CHATFRAME );
		if ( Settings.General.LanguageRTL ) strCaption = _T("\x200F") + strCaption + _T("\x202E");
		strCaption += _T(" : ");
		if ( Settings.General.LanguageRTL ) strCaption += _T("\x202B");
		strCaption += m_sNick;

		if ( m_pSession )
		{
			CString strAddress;
			strAddress.Format( _T(" (%s)"),
				(LPCTSTR)HostToString( &m_pSession->m_pHost ) );
			if ( Settings.General.LanguageRTL ) strCaption += _T("\x200F");
			strCaption += strAddress;
			if ( ! m_pSession->m_sUserAgent.IsEmpty() )
			{
				if ( Settings.General.LanguageRTL ) strCaption += _T("\x200F");
				strCaption = strCaption + _T(" - ") + m_pSession->m_sUserAgent;
			}
		}

		SetWindowText( strCaption );

		Open();

		SetAlert();

		return 0;
	}

	if ( pMsg->m_hBitmap )
	{
		CChatWnd::AddBitmap( pMsg->m_hBitmap );
		return 0;
	}

	if ( pMsg->m_bType == cmtStatus ||
		 pMsg->m_bType == cmtError ||
		 pMsg->m_bType == cmtInfo )
	{
		CChatWnd::OnStatusMessage( (int)pMsg->m_bType - 3, pMsg->m_sMessage );
		return 0;
	}

	CString sChatID;
	if ( m_pSession )
		sChatID = HostToString( &m_pSession->m_pHost );
	else
		sChatID = _T("@") + m_sNick;

	CChatWnd::OnMessage( ( pMsg->m_bType == cmtAction ), sChatID, false,
		pMsg->m_sFrom, MyProfile.GetNick(), pMsg->m_sMessage );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CPrivateChatWnd commands

void CPrivateChatWnd::OnUpdateChatConnect(CCmdUI* pCmdUI)
{
	BOOL bState = m_pSession && m_pSession->GetConnectedState() == TRI_FALSE;
	if ( CCoolBarItem* pItem = CCoolBarItem::FromCmdUI( pCmdUI ) ) pItem->Show( bState );
	pCmdUI->Enable( bState );
}

void CPrivateChatWnd::OnChatConnect()
{
	if ( m_pSession && m_pSession->GetConnectedState() == TRI_FALSE )
	{
		Open();

		m_pSession->Connect();
	}
}

void CPrivateChatWnd::OnUpdateChatDisconnect(CCmdUI* pCmdUI)
{
	BOOL bState = m_pSession && m_pSession->GetConnectedState() != TRI_FALSE;
	if ( CCoolBarItem* pItem = CCoolBarItem::FromCmdUI( pCmdUI ) ) pItem->Show( bState );
	pCmdUI->Enable( bState );
}

void CPrivateChatWnd::OnChatDisconnect()
{
	if ( m_pSession )
	{
		m_pSession->Close();
	}
}

void CPrivateChatWnd::OnUpdateChatBrowse(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_pSession != NULL );
}

void CPrivateChatWnd::OnChatBrowse()
{
	if ( m_pSession )
	{
		new CBrowseHostWnd( m_pSession->m_nProtocol,
			&m_pSession->m_pHost, FALSE, m_pSession->m_oGUID, m_pSession->m_sNick );
	}
}

void CPrivateChatWnd::OnUpdateChatPriority(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_pSession && m_pSession->GetConnectedState() == TRI_TRUE );
}

void CPrivateChatWnd::OnChatPriority()
{
	if ( ! m_pSession )
		return;

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 500 ) )
		return;

	DWORD nAddress = m_pSession->m_pHost.sin_addr.s_addr;

	for ( POSITION pos = Uploads.GetIterator() ; pos ; )
	{
		CUploadTransfer* pUpload = Uploads.GetNext( pos );

		if ( pUpload->m_pHost.sin_addr.s_addr == nAddress &&
			 pUpload->m_nState == upsQueued )
		{
			pUpload->Promote();
		}
	}

	CString strMessage;
	strMessage.Format( LoadString( IDS_CHAT_PRIORITY_GRANTED ), (LPCTSTR)HostToString( &m_pSession->m_pHost ) );
	CChatWnd::OnStatusMessage( 2, strMessage );
}
