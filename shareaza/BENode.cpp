//
// BENode.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "BENode.h"
#include "Buffer.h"
#include "SHA.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CBENode construction/destruction

CBENode::CBENode()
{
	m_nType		= beNull;
	m_pValue	= NULL;
	m_nValue	= 0;
}

CBENode::~CBENode()
{
	if ( m_pValue != NULL ) Clear();
}

//////////////////////////////////////////////////////////////////////
// CBENode clear

void CBENode::Clear()
{
	if ( m_pValue != NULL )
	{
		if ( m_nType == beString )
		{
			delete [] (LPSTR)m_pValue;
		}
		else if ( m_nType == beList )
		{
			CBENode** pNode = (CBENode**)m_pValue;
			for ( ; m_nValue-- ; pNode++ ) delete *pNode;
			delete [] (CBENode**)m_pValue;
		}
		else if ( m_nType == beDict )
		{
			CBENode** pNode = (CBENode**)m_pValue;
			for ( ; m_nValue-- ; pNode++ )
			{
				delete *pNode++;
				delete [] (LPBYTE)*pNode;
			}
			delete [] (CBENode**)m_pValue;
		}
	}
	
	m_nType		= beNull;
	m_pValue	= NULL;
	m_nValue	= 0;
}

//////////////////////////////////////////////////////////////////////
// CBENode add a child node

CBENode* CBENode::Add(const LPBYTE pKey, int nKey)
{
	switch ( m_nType )
	{
	case beNull:
		m_nType		= ( pKey != NULL && nKey > 0 ) ? beDict : beList;
		m_pValue	= NULL;
		m_nValue	= 0;
		break;
	case beList:
		ASSERT( pKey == NULL && nKey == 0 );
		break;
	case beDict:
		ASSERT( pKey != NULL && nKey > 0 );
		break;
	default:
		ASSERT( FALSE );
		break;
	}
	
	CBENode* pNew = new CBENode();
	
	if ( m_nType == beList )
	{
		CBENode** pList = new CBENode*[ (DWORD)m_nValue + 1 ];
		
		if ( m_pValue != NULL )
		{
			CopyMemory( pList, m_pValue, 4 * (DWORD)m_nValue );
			delete [] (CBENode**)m_pValue;
		}
		
		pList[ m_nValue++ ] = pNew;
		m_pValue = pList;
	}
	else
	{
		CBENode** pList = new CBENode*[ (DWORD)m_nValue * 2 + 2 ];
		
		if ( m_pValue != NULL )
		{
			CopyMemory( pList, m_pValue, 8 * (DWORD)m_nValue );
			delete [] (CBENode**)m_pValue;
		}
		
		BYTE* pxKey = new BYTE[ nKey + 1 ];
		CopyMemory( pxKey, pKey, nKey );
		pxKey[ nKey ] = 0;
		
		pList[ m_nValue * 2 ]		= pNew;
		pList[ m_nValue * 2 + 1 ]	= (CBENode*)pxKey;
		
		m_pValue = pList;
		m_nValue ++;
	}
	
	return pNew;
}

//////////////////////////////////////////////////////////////////////
// CBENode find a child node

CBENode* CBENode::GetNode(LPCSTR pszKey) const
{
	if ( m_nType != beDict ) return NULL;
	
	CBENode** pNode = (CBENode**)m_pValue;
	
	for ( DWORD nNode = (DWORD)m_nValue ; nNode ; nNode--, pNode += 2 )
	{
		if ( strcmp( pszKey, (LPCSTR)pNode[1] ) == 0 ) return *pNode;
	}
	
	return NULL;
}

CBENode* CBENode::GetNode(const LPBYTE pKey, int nKey) const
{
	if ( m_nType != beDict ) return NULL;
	
	CBENode** pNode = (CBENode**)m_pValue;
	
	for ( DWORD nNode = (DWORD)m_nValue ; nNode ; nNode--, pNode += 2 )
	{
		if ( memcmp( pKey, (LPBYTE)pNode[1], nKey ) == 0 ) return *pNode;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CBENode SHA1 computation

void CBENode::GetSHA1(SHA1* pSHA1) const
{
	ASSERT( this != NULL );
	
	CBuffer pBuffer;
	Encode( &pBuffer );
	
	CSHA pSHA;
	pSHA.Add( pBuffer.m_pBuffer, pBuffer.m_nLength );
	pSHA.Finish();
	pSHA.GetHash( pSHA1 );
}

//////////////////////////////////////////////////////////////////////
// CBENode encoding

void CBENode::Encode(CBuffer* pBuffer) const
{
	CHAR szBuffer[64];
	
	ASSERT( this != NULL );
	ASSERT( pBuffer != NULL );
	CString str;
	
	if ( m_nType == beString )
	{
		sprintf( szBuffer, "%u:", (DWORD)m_nValue );
		pBuffer->Print( szBuffer );
		pBuffer->Add( m_pValue, (DWORD)m_nValue );
	}
	else if ( m_nType == beInt )
	{
		sprintf( szBuffer, "i%I64ie", m_nValue );
		pBuffer->Print( szBuffer );
	}
	else if ( m_nType == beList )
	{
		CBENode** pNode = (CBENode**)m_pValue;
		
		pBuffer->Print( "l" );
		
		for ( DWORD nItem = 0 ; nItem < (DWORD)m_nValue ; nItem++, pNode++ )
		{
			(*pNode)->Encode( pBuffer );
		}
		
		pBuffer->Print( "e" );
	}
	else if ( m_nType == beDict )
	{
		CBENode** pNode = (CBENode**)m_pValue;
		
		pBuffer->Print( "d" );
		
		for ( DWORD nItem = 0 ; nItem < m_nValue ; nItem++, pNode += 2 )
		{
			LPCSTR pszKey = (LPCSTR)pNode[1];
			sprintf( szBuffer, "%i:", strlen( pszKey ) );
			pBuffer->Print( szBuffer );
			pBuffer->Print( pszKey );
			(*pNode)->Encode( pBuffer );
		}
		
		pBuffer->Print( "e" );
	}
	else
	{
		ASSERT( FALSE );
	}
}

//////////////////////////////////////////////////////////////////////
// CBENode decoding

CBENode* CBENode::Decode(CBuffer* pBuffer)
{
	ASSERT( pBuffer != NULL );
	
	CBENode* pNode	= new CBENode();
	LPBYTE pInput	= pBuffer->m_pBuffer;
	DWORD nInput	= pBuffer->m_nLength;
	
	try
	{
		pNode->Decode( pInput, nInput );
	}
	catch ( CException* pException )
	{
		pException->Delete();
		delete pNode;
		pNode = NULL;
	}
	
	return pNode;
}

#define INC(x) { pInput += (x); nInput -= (x); }

void CBENode::Decode(LPBYTE& pInput, DWORD& nInput)
{
	ASSERT( m_nType == beNull );
	ASSERT( pInput != NULL );
	
	if ( nInput < 1 ) AfxThrowUserException();
	
	if ( *pInput == 'i' )
	{
		INC( 1 );
		
        DWORD nSeek = 1;
		for ( ; nSeek < 40 ; nSeek++ )
		{
			if ( nSeek >= nInput ) AfxThrowUserException();
			if ( pInput[nSeek] == 'e' ) break;
		}
		
		if ( nSeek >= 40 ) AfxThrowUserException();
		
		pInput[nSeek] = 0;
		if ( sscanf( (LPCSTR)pInput, "%I64i", &m_nValue ) != 1 ) AfxThrowUserException();
		pInput[nSeek] = 'e';
		
		INC( nSeek + 1 );
		m_nType = beInt;
	}
	else if ( *pInput == 'l' )
	{
		m_nType = beList;
		INC( 1 );
		
		while ( TRUE )
		{
			if ( nInput < 1 ) AfxThrowUserException();
			if ( *pInput == 'e' ) break;
			Add()->Decode( pInput, nInput );
		}
		
		INC( 1 );
	}
	else if ( *pInput == 'd' )
	{
		m_nType = beDict;
		INC( 1 );
		
		while ( TRUE )
		{
			if ( nInput < 1 ) AfxThrowUserException();
			if ( *pInput == 'e' ) break;
			
			int nLen = DecodeLen( pInput, nInput );
			LPBYTE pKey = pInput;
			INC( nLen );
			
			Add( pKey, nLen )->Decode( pInput, nInput );
		}
		
		INC( 1 );
	}
	else if ( *pInput >= '0' && *pInput <= '9' )
	{
		m_nType		= beString;
		m_nValue	= DecodeLen( pInput, nInput );
		m_pValue	= new CHAR[ (DWORD)m_nValue + 1 ];
		CopyMemory( m_pValue, pInput, (DWORD)m_nValue );
		((LPBYTE)m_pValue)[ m_nValue ] = 0;
		
		INC( (DWORD)m_nValue );
	}
	else
	{
		AfxThrowUserException();
	}
}

int CBENode::DecodeLen(LPBYTE& pInput, DWORD& nInput)
{
    DWORD nSeek = 1;
    for ( ; nSeek < 32 ; nSeek++ )
	{
		if ( nSeek >= nInput ) AfxThrowUserException();
		if ( pInput[ nSeek ] == ':' ) break;
	}
	
	if ( nSeek >= 32 ) AfxThrowUserException();
	int nLen = 0;
	
	pInput[ nSeek ] = 0;
	if ( sscanf( (LPCSTR)pInput, "%i", &nLen ) != 1 ) AfxThrowUserException();
	pInput[ nSeek ] = ':';
	INC( nSeek + 1 );
	
	if ( nInput < (DWORD)nLen ) AfxThrowUserException();
	
	return nLen;
}
