//
// ChatWindows.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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
#include "ChatWindows.h"
#include "ChatSession.h"
#include "CtrlChatFrame.h"
#include "CtrlPrivateChatFrame.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CChatWindows ChatWindows;


//////////////////////////////////////////////////////////////////////
// CChatWindows construction

CChatWindows::CChatWindows()
{
}

CChatWindows::~CChatWindows()
{
}

//////////////////////////////////////////////////////////////////////
// CChatWindows list access

POSITION CChatWindows::GetIterator() const
{
	return m_pList.GetHeadPosition();
}

CChatFrame* CChatWindows::GetNext(POSITION& pos) const
{
	return (CChatFrame*)m_pList.GetNext( pos );
}

int CChatWindows::GetCount() const
{
	return m_pList.GetCount();
}

//////////////////////////////////////////////////////////////////////
// CChatWindows close all

void CChatWindows::Close()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->GetParent()->DestroyWindow();
	}
	
	m_pList.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CChatWindows private chat windows

CPrivateChatFrame* CChatWindows::FindPrivate(CGUID* pGUID)
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPrivateChatFrame* pFrame = reinterpret_cast<CPrivateChatFrame*>( GetNext( pos ) );
		
		if ( pFrame->IsKindOf( RUNTIME_CLASS(CPrivateChatFrame) ) )
		{
			if ( pFrame->m_pSession != NULL &&
				 pFrame->m_pSession->m_bGUID &&
				 pFrame->m_pSession->m_pGUID == *pGUID ) return pFrame;
		}
	}
	
	return NULL;
}

CPrivateChatFrame* CChatWindows::FindPrivate(IN_ADDR* pAddress)
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPrivateChatFrame* pFrame = reinterpret_cast<CPrivateChatFrame*>( GetNext( pos ) );
		
		if ( pFrame->IsKindOf( RUNTIME_CLASS(CPrivateChatFrame) ) )
		{
			if ( pFrame->m_pSession != NULL &&
				 pFrame->m_pSession->m_pHost.sin_addr.S_un.S_addr == pAddress->S_un.S_addr )
				return pFrame;
		}
	}
	
	return NULL;
}

CPrivateChatFrame* CChatWindows::OpenPrivate(CGUID* pGUID, IN_ADDR* pAddress, WORD nPort, BOOL bMustPush)
{
	SOCKADDR_IN pHost;
	
	pHost.sin_family	= PF_INET;
	pHost.sin_addr		= *pAddress;
	pHost.sin_port		= htons( nPort );
	
	return OpenPrivate( pGUID, &pHost, bMustPush );
}

CPrivateChatFrame* CChatWindows::OpenPrivate(CGUID* pGUID, SOCKADDR_IN* pHost, BOOL bMustPush)
{
	CPrivateChatFrame* pFrame = NULL;
	
	if ( ! MyProfile.IsValid() )
	{
		CString strMessage;
		LoadString( strMessage, IDS_CHAT_NEED_PROFILE );
		if ( AfxMessageBox( strMessage, MB_YESNO|MB_ICONQUESTION ) == IDYES )
			AfxGetMainWnd()->PostMessage( WM_COMMAND, ID_TOOLS_PROFILE );
		return NULL;
	}
	
	if ( pGUID != NULL ) pFrame = FindPrivate( pGUID );
	if ( pFrame == NULL ) pFrame = FindPrivate( &pHost->sin_addr );
	
	if ( pFrame == NULL )
	{
		pFrame = new CPrivateChatFrame();
		pFrame->Initiate( pGUID, pHost, bMustPush );
	}
	
	pFrame->PostMessage( WM_COMMAND, ID_CHAT_CONNECT );
	
	CWnd* pParent = pFrame->GetParent();
	if ( pParent->IsIconic() ) pParent->ShowWindow( SW_SHOWNORMAL );
	pParent->BringWindowToTop();
	pParent->SetForegroundWindow();
	
	return pFrame;
}

//////////////////////////////////////////////////////////////////////
// CChatWindows add and remove

void CChatWindows::Add(CChatFrame* pFrame)
{
	if ( m_pList.Find( pFrame ) == NULL ) m_pList.AddTail( pFrame );
}

void CChatWindows::Remove(CChatFrame* pFrame)
{
	if ( POSITION pos = m_pList.Find( pFrame ) ) m_pList.RemoveAt( pos );
}

