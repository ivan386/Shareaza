//
// DCStream.h
//
// Copyright (c) Shareaza Development Team, 2010.
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

#pragma once

#include "GProfile.h"


template < class CBase >
class CDCStream : public CBase
{
public:
	CDCStream();
	virtual ~CDCStream();

	CString			m_sRemoteNick;	// Hub/Client nick
	CString			m_sNick;		// My nick

protected:
	std::string		m_strKey;		// Remote client key
	BOOL			m_bExtended;	// Using extended protocol
	CStringList		m_oFeatures;	// Remote client supported features
	BOOL			m_bDownload;	// TRUE - remote client want to download, FALSE - to upload

	virtual BOOL	OnRead();

	// Got unknown command
	virtual BOOL	OnCommand(const std::string& strCommand, const std::string& strParams) = 0;
	// Got $Lock command
	virtual BOOL	OnLock() { return TRUE; };
	// Got $Key command
	virtual BOOL	OnKey() { return TRUE; };
	// Got $Support command
	virtual BOOL	OnSupport() { return TRUE; };
	// Got $Hello command
	virtual BOOL	OnHello() { return TRUE; };
	// Got chat message
	virtual BOOL	OnChat(const std::string& strMessage) = 0;

	// Read single command from input buffer
	BOOL			ReadCommand(std::string& strLine);
	// Processes single command
	BOOL			ProcessCommand(const std::string& strCommand, const std::string& strParams);
	// Generating challenge for this client
	std::string		GenerateLock();

	static std::string MakeKey(const std::string& aLock);
	static std::string KeySubst(const BYTE* aKey, size_t len, size_t n);
	static BOOL IsExtra(BYTE b);
};


template< class CBase >
CDCStream< CBase >::CDCStream()
	: CBase( PROTOCOL_DC )
	, m_sNick( MyProfile.GetNick() )
	, m_bExtended( FALSE )
	, m_bDownload( FALSE )
{
	// Make DC++ safe nick
	m_sNick.Replace( _T(' '), _T('_') );
	m_sNick.Replace( _T('&'), _T('_') );
	m_sNick.Replace( _T('|'), _T('_') );
	m_sNick.Replace( _T('$'), _T('_') );
	m_sNick.Replace( _T('<'), _T('_') );
	m_sNick.Replace( _T('>'), _T('_') );
}

template< class CBase >
CDCStream< CBase >::~CDCStream()
{
}

template< class CBase >
std::string CDCStream< CBase >::GenerateLock()
{
	char cLock[ 256 ] = {};
	sprintf_s( cLock,
		"EXTENDEDPROTOCOL::" CLIENT_NAME "::%s:%u",
		inet_ntoa( m_pHost.sin_addr ), htons( m_pHost.sin_port ) );
	return cLock;
}

template< class CBase >
std::string CDCStream< CBase >::MakeKey(const std::string& aLock)
{
	if ( aLock.size() < 3 )
		return std::string();

	auto_array< BYTE > temp( new BYTE[ aLock.size() ] );
	size_t extra = 0;
	BYTE v1 = (BYTE)( (BYTE)aLock[ 0 ] ^ 5 );
	v1 = (BYTE)( ( ( v1 >> 4 ) | ( v1 << 4 ) ) & 0xff );
	temp[ 0 ] = v1;
	for ( size_t i = 1; i < aLock.size(); i++ )
	{
		v1 = (BYTE)( (BYTE)aLock[ i ] ^ (BYTE)aLock[ i - 1 ] );
		v1 = (BYTE)( ( ( v1 >> 4 ) | ( v1 << 4 ) ) & 0xff );
		temp[ i ] = v1;
		if ( IsExtra( temp[ i ] ) )
			extra++;
	}
	temp[ 0 ] = (BYTE)( temp[ 0 ] ^ temp[ aLock.size() - 1 ] );
	if ( IsExtra( temp[ 0 ] ) )
		extra++;

	return KeySubst( &temp[ 0 ], aLock.size(), extra );
}

template< class CBase >
std::string CDCStream< CBase >::KeySubst(const BYTE* aKey, size_t len, size_t n)
{
	auto_array< BYTE > temp( new BYTE[ len + n * 9 ] );
	size_t j = 0;	
	for ( size_t i = 0; i < len; i++ )
	{
		if ( IsExtra( aKey[ i ] ) )
		{
			temp[ j++ ] = '/'; temp[ j++ ] = '%'; temp[ j++ ] = 'D';
			temp[ j++ ] = 'C'; temp[ j++ ] = 'N';
			switch ( aKey[ i ] )
			{
			case 0:   temp[ j++ ] = '0'; temp[ j++ ] = '0'; temp[ j++ ] = '0'; break;
			case 5:   temp[ j++ ] = '0'; temp[ j++ ] = '0'; temp[ j++ ] = '5'; break;
			case 36:  temp[ j++ ] = '0'; temp[ j++ ] = '3'; temp[ j++ ] = '6'; break;
			case 96:  temp[ j++ ] = '0'; temp[ j++ ] = '9'; temp[ j++ ] = '6'; break;
			case 124: temp[ j++ ] = '1'; temp[ j++ ] = '2'; temp[ j++ ] = '4'; break;
			case 126: temp[ j++ ] = '1'; temp[ j++ ] = '2'; temp[ j++ ] = '6'; break;
			}
			temp[ j++ ] = '%'; temp[ j++ ] = '/';
		}
		else
			temp[ j++ ] = aKey[ i ];
	}
	return std::string( (const char*)&temp[ 0 ], j );
}

template< class CBase >
BOOL CDCStream< CBase >::IsExtra(BYTE b)
{
	return ( b == 0 || b == 5 || b == 124 || b == 96 || b == 126 || b == 36 );
}

template< class CBase >
BOOL CDCStream< CBase >::ReadCommand(std::string& strLine)
{
	strLine.clear();

	CLockedBuffer pInput( GetInput() );

	if ( ! pInput->m_nLength )
		return FALSE;

	DWORD nLength = 0;
	for ( ; nLength < pInput->m_nLength ; nLength++ )
	{
		if ( pInput->m_pBuffer[ nLength ] == '|' )
			break;
	}

	if ( nLength >= pInput->m_nLength )
		return FALSE;

	strLine.append( (const char*)pInput->m_pBuffer, nLength );

	pInput->Remove( nLength + 1 );

	return TRUE;
}

template< class CBase >
BOOL CDCStream< CBase >::ProcessCommand(const std::string& strCommand, const std::string& strParams)
{
	if ( strCommand.size() == 0 )
	{
		// Ping, i.e. received only "|" message
		return TRUE;
	}
	else if ( strCommand[ 0 ] != '$' )
	{
		return OnChat( strCommand + strParams );
	}
	else if ( strCommand == "$Lock" )
	{
		// $Lock [EXTENDEDPROTOCOL]Challenge Pk=Vendor

		m_bExtended = ( strParams.substr( 0, 16 ) == "EXTENDEDPROTOCOL" );

		std::string strLock;
		std::string::size_type nPos = strParams.find( " Pk=" );
		if ( nPos != std::string::npos )
		{
			// Good way
			strLock = strParams.substr( 0, nPos );
			m_sUserAgent = strParams.substr( nPos + 4 ).c_str();
		}
		else
		{
			// Bad way
			nPos = strParams.find( ' ' );
			if ( nPos != std::string::npos )
				strLock = strParams.substr( 0, nPos );
			else
				// Very bad way
				strLock = strParams;
		}

		m_strKey = MakeKey( strLock );

		return OnLock();
	}
	else if ( strCommand == "$Key" )
	{
		// $Key key

		std::string sKey = MakeKey( GenerateLock() );
		if ( sKey == strParams )
		{
			// Right key
			return OnKey();
		}
		else
		{
			// Wrong key
			return FALSE;
		}
	}
	else if ( strCommand == "$Supports" )
	{
		// $Supports [option1]...[optionN]

		m_bExtended = TRUE;

		m_oFeatures.RemoveAll();
		for ( CString strFeatures( strParams.c_str() ); ! strFeatures.IsEmpty(); )
		{
			CString strFeature = strFeatures.SpanExcluding( _T(" ") );
			strFeatures = strFeatures.Mid( strFeature.GetLength() + 1 );
			if ( strFeature.IsEmpty() )
				continue;
			strFeature.MakeLower();
			if ( m_oFeatures.Find( strFeature ) == NULL )
			{
				m_oFeatures.AddTail( strFeature );
			}				
		}

		return OnSupport();
	}
	else if ( strCommand == "$Hello" )
	{
		// $Hello Nick

		m_sNick = strParams.c_str();

		return OnHello();
	}
	else if ( strCommand == "$MyNick" )
	{
		// $MyNick RemoteNick

		m_sRemoteNick = strParams.c_str();

		return TRUE;
	}

	return OnCommand( strCommand, strParams );
}

template< class CBase >
BOOL CDCStream< CBase >::OnRead()
{
	if ( ! CBase::OnRead() )
		return FALSE;

	std::string strLine;
	while ( ReadCommand( strLine ) )
	{
		theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING,
			_T("%s >> %s|"), (LPCTSTR)m_sAddress, (LPCTSTR)CA2CT( strLine.c_str() ) );

		std::string strCommand, strParams;
		std::string::size_type nPos = strLine.find( ' ' );
		if ( nPos != std::string::npos )
		{
			strCommand = strLine.substr( 0, nPos );
			strParams = strLine.substr( nPos + 1 );
		}
		else
			strCommand = strLine;

		if ( ! ProcessCommand( strCommand, strParams ) )
			return FALSE;
	}

	return TRUE;
}
