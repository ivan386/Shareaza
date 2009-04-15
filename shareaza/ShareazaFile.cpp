//
// ShareazaFile.cpp
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
#include "Network.h"
#include "ShareazaFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CShareazaFile, CComObject)

BEGIN_INTERFACE_MAP(CShareazaFile, CComObject)
	INTERFACE_PART(CShareazaFile, IID_IShareazaFile, ShareazaFile)
END_INTERFACE_MAP()

CShareazaFile::CShareazaFile() :
	m_nSize( SIZE_UNKNOWN )
{
	EnableDispatch( IID_IShareazaFile );
}

CShareazaFile::CShareazaFile(const CShareazaFile& pFile) :
	m_sName( pFile.m_sName ),
	m_nSize( pFile.m_nSize ),
	m_oSHA1( pFile.m_oSHA1 ),
	m_oTiger( pFile.m_oTiger ),
	m_oED2K( pFile.m_oED2K ),
	m_oBTH( pFile.m_oBTH ),
	m_oMD5( pFile.m_oMD5 ),
	m_sPath( pFile.m_sPath ),
	m_sURL( pFile.m_sURL )
{
	EnableDispatch( IID_IShareazaFile );
}

CShareazaFile& CShareazaFile::operator=(const CShareazaFile& pFile)
{
	m_sName = pFile.m_sName;
	m_nSize = pFile.m_nSize;
	m_oSHA1 = pFile.m_oSHA1;
	m_oTiger = pFile.m_oTiger;
	m_oED2K = pFile.m_oED2K;
	m_oBTH = pFile.m_oBTH;
	m_oMD5 = pFile.m_oMD5;
	m_sPath = pFile.m_sPath;
	m_sURL = pFile.m_sURL;
	return *this;
}

CString CShareazaFile::GetURL(const IN_ADDR& nAddress, WORD nPort) const
{
	CString strURL;
	if ( m_oSHA1 )
	{
		strURL.Format( _T("http://%s:%i/uri-res/N2R?%s"),
			(LPCTSTR)CString( inet_ntoa( nAddress ) ),
			nPort, (LPCTSTR)m_oSHA1.toUrn() );
	}
	else if ( m_oTiger )
	{
		strURL.Format( _T("http://%s:%i/uri-res/N2R?%s"),
			(LPCTSTR)CString( inet_ntoa( nAddress ) ),
			nPort, (LPCTSTR)m_oTiger.toUrn() );
	}
	else if ( m_oED2K )
	{
		strURL.Format( _T("http://%s:%i/uri-res/N2R?%s"),
			(LPCTSTR)CString( inet_ntoa( nAddress ) ),
			nPort, (LPCTSTR)m_oED2K.toUrn() );
	}
	else if ( m_oMD5 )
	{
		strURL.Format( _T("http://%s:%i/uri-res/N2R?%s"),
			(LPCTSTR)CString( inet_ntoa( nAddress ) ),
			nPort, (LPCTSTR)m_oMD5.toUrn() );
	}
	else if ( m_oBTH )
	{
		strURL.Format( _T("http://%s:%i/uri-res/N2R?%s"),
			(LPCTSTR)CString( inet_ntoa( nAddress ) ),
			nPort, (LPCTSTR)m_oBTH.toUrn() );
	}
	return strURL;
}

CString CShareazaFile::GetBitprint() const
{
	if ( m_oSHA1 && m_oTiger )
		return CString( _T("urn:bitprint:") ) + m_oSHA1.toString() + _T(".") + m_oTiger.toString();
	else if ( m_oSHA1 )
		return m_oSHA1.toUrn();
	else if ( m_oTiger )
		return m_oTiger.toUrn();
	else
		return CString();
}

CString CShareazaFile::GetFilename() const
{
	CString sFilename;
	if ( m_oTiger )
		sFilename = CString( _T("ttr_")  ) + m_oTiger.toString();
	else if ( m_oSHA1 )
		sFilename = CString( _T("sha1_") ) + m_oSHA1.toString();
	else if ( m_oED2K )
		sFilename = CString( _T("ed2k_") ) + m_oED2K.toString();
	else if ( m_oMD5 )
		sFilename = CString( _T("md5_")  ) + m_oMD5.toString();
	else if ( m_oBTH )
		sFilename = CString( _T("btih_") ) + m_oBTH.toString();
	else
		sFilename.Format( _T("rand_%2i%2i%2i%2i"),
			GetRandomNum( 0, 99 ), GetRandomNum( 0, 99 ), 
			GetRandomNum( 0, 99 ), GetRandomNum( 0, 99 ) );
	return sFilename;
}

bool CShareazaFile::SplitStringToURLs(LPCTSTR pszURLs, CMapStringToFILETIME& oUrls) const
{
	CString strURLs( pszURLs );

	// Fix buggy URLs
	strURLs.Replace( _T("Zhttp://"), _T("Z, http://") );
	strURLs.Replace( _T("Z%2C http://"), _T("Z, http://") );

	// Temporary replace quoted commas
	bool bQuote = false;
	for ( int nScan = 0 ; nScan < strURLs.GetLength() ; nScan++ )
	{
		if ( strURLs[ nScan ] == '\"' )
		{
			bQuote = ! bQuote;
			strURLs.SetAt( nScan, ' ' );
		}
		else if ( strURLs[ nScan ] == ',' && bQuote )
		{
			strURLs.SetAt( nScan, '\x1f' );
		}
	}

	int nStart = 0;
	for (;;)
	{
		CString strURL = strURLs.Tokenize( _T(","), nStart );
		if ( strURL.IsEmpty() )
			break;
		strURL.Replace( '\x1f', ',' );	// Restore quoted commas
		strURL.Trim();

		// Get time
		FILETIME tSeen = { 0, 0 };
		int nPos = strURL.ReverseFind( ' ' );
		if ( nPos > 8 && TimeFromString( strURL.Mid( nPos + 1 ).TrimLeft(), &tSeen ) )
		{
			strURL = strURL.Left( nPos ).TrimRight();
		}

		// Convert short "h.o.s.t:port" to full source URL
		nPos = strURL.Find( ':' );
		if ( nPos > 6 && strURL.GetLength() > nPos + 1 &&
			strURL.GetAt( nPos + 1 ) != '/' )
		{
			int nPort = 0;
			_stscanf( strURL.Mid( nPos + 1 ), _T("%i"), &nPort );
			DWORD nAddress = inet_addr( CT2CA( strURL.Left( nPos ) ) );
			if ( nPort > 0 && nPort <= USHRT_MAX && nAddress != INADDR_NONE &&
				! Network.IsFirewalledAddress( &nAddress, TRUE ) &&
				! Network.IsReserved( (IN_ADDR*)&nAddress ) )
			{
				strURL = GetURL( *(IN_ADDR*)&nAddress, static_cast< WORD >( nPort ) );
			}
			else
			{
				strURL.Empty();
			}
		}

		if ( strURL.GetLength() )
		{
			strURL.Replace( _T("%2C"), _T(",") );
			oUrls.SetAt( strURL, tSeen );
		}
	}

	return ! oUrls.IsEmpty();
}
	
//////////////////////////////////////////////////////////////////////
// CShareazaFile automation

IMPLEMENT_DISPATCH(CShareazaFile, ShareazaFile)

STDMETHODIMP CShareazaFile::XShareazaFile::get_Path(BSTR FAR* psPath)
{
	METHOD_PROLOGUE( CShareazaFile, ShareazaFile )
	pThis->m_sPath.SetSysString( psPath );
	return S_OK;
}

STDMETHODIMP CShareazaFile::XShareazaFile::get_Name(BSTR FAR* psName)
{
	METHOD_PROLOGUE( CShareazaFile, ShareazaFile )
	pThis->m_sName.SetSysString( psName );
	return S_OK;
}

STDMETHODIMP CShareazaFile::XShareazaFile::get_Size(ULONGLONG FAR* pnSize)
{
	METHOD_PROLOGUE( CShareazaFile, ShareazaFile )
	*pnSize = pThis->m_nSize;
	return S_OK;
}

STDMETHODIMP CShareazaFile::XShareazaFile::get_URN(BSTR sURN, BSTR FAR* psURN)
{
	METHOD_PROLOGUE( CShareazaFile, ShareazaFile )

	if ( ! psURN )
		return E_POINTER;

	CString strURN( sURN );

	if ( strURN.IsEmpty() )
	{
		if ( pThis->m_oTiger && pThis->m_oSHA1 )
			strURN = _T("urn:bitprint");
		else if ( pThis->m_oTiger )
			strURN = _T("urn:tree:tiger/");
		else if ( pThis->m_oSHA1 )
			strURN = _T("urn:sha1");
		else
			return E_FAIL;
	}

	if ( strURN.CompareNoCase( _T("urn:bitprint") ) == 0 )
	{
		if ( !pThis->m_oSHA1 || ! pThis->m_oTiger ) return E_FAIL;
		strURN	= _T("urn:bitprint:")
				+ pThis->m_oSHA1.toString() + '.'
				+ pThis->m_oTiger.toString();
	}
	else if ( strURN.CompareNoCase( _T("urn:sha1") ) == 0 )
	{
		if ( !pThis->m_oSHA1 ) return E_FAIL;
		strURN = pThis->m_oSHA1.toUrn();
	}
	else if ( strURN.CompareNoCase( _T("urn:tree:tiger/") ) == 0 )
	{
		if ( ! pThis->m_oTiger ) return E_FAIL;
		strURN = pThis->m_oTiger.toUrn();
	}
	else if ( strURN.CompareNoCase( _T("urn:md5") ) == 0 )
	{
		if ( ! pThis->m_oMD5 ) return E_FAIL;
		strURN = pThis->m_oMD5.toUrn();
	}
	else if ( strURN.CompareNoCase( _T("urn:ed2k") ) == 0 )
	{
		if ( ! pThis->m_oED2K ) return E_FAIL;
		strURN = pThis->m_oED2K.toUrn();
	}
	else if ( strURN.CompareNoCase( _T("urn:btih") ) == 0 )
	{
		if ( ! pThis->m_oBTH ) return E_FAIL;
		strURN = pThis->m_oBTH.toUrn();
	}
	else
	{
		return E_FAIL;
	}

	strURN.SetSysString( psURN );

	return S_OK;
}

STDMETHODIMP CShareazaFile::XShareazaFile::get_Hash(URN_TYPE nType, ENCODING nEncoding, BSTR FAR* psURN)
{
	METHOD_PROLOGUE( CShareazaFile, ShareazaFile )

	if ( ! psURN )
		return E_POINTER;

	CString strURN;
	switch( nType )
	{
	case URN_SHA1:
		if ( pThis->m_oSHA1 )
		{
			switch( nEncoding )
			{
			case ENCODING_GUID:
				strURN = pThis->m_oSHA1.toString< Hashes::guidEncoding >();
				break;
			case ENCODING_BASE16:
				strURN = pThis->m_oSHA1.toString< Hashes::base16Encoding >();
				break;
			case ENCODING_BASE32:
				strURN = pThis->m_oSHA1.toString< Hashes::base32Encoding >();
				break;
			default:
				return E_INVALIDARG;
			}
		}
		break;

	case URN_TIGER:
		if ( pThis->m_oTiger )
		{
			switch( nEncoding )
			{
			case ENCODING_GUID:
				strURN = pThis->m_oTiger.toString< Hashes::guidEncoding >();
				break;
			case ENCODING_BASE16:
				strURN = pThis->m_oTiger.toString< Hashes::base16Encoding >();
				break;
			case ENCODING_BASE32:
				strURN = pThis->m_oTiger.toString< Hashes::base32Encoding >();
				break;
			default:
				return E_INVALIDARG;
			}
		}
		break;

	case URN_ED2K:
		if ( pThis->m_oED2K )
		{
			switch( nEncoding )
			{
			case ENCODING_GUID:
				strURN = pThis->m_oED2K.toString< Hashes::guidEncoding >();
				break;
			case ENCODING_BASE16:
				strURN = pThis->m_oED2K.toString< Hashes::base16Encoding >();
				break;
			case ENCODING_BASE32:
				strURN = pThis->m_oED2K.toString< Hashes::base32Encoding >();
				break;
			default:
				return E_INVALIDARG;
			}
		}
		break;

	case URN_MD5:
		if ( pThis->m_oMD5 )
		{
			switch( nEncoding )
			{
			case ENCODING_GUID:
				strURN = pThis->m_oMD5.toString< Hashes::guidEncoding >();
				break;
			case ENCODING_BASE16:
				strURN = pThis->m_oMD5.toString< Hashes::base16Encoding >();
				break;
			case ENCODING_BASE32:
				strURN = pThis->m_oMD5.toString< Hashes::base32Encoding >();
				break;
			default:
				return E_INVALIDARG;
			}
		}
		break;

	case URN_BTIH:
		if ( pThis->m_oBTH )
		{
			switch( nEncoding )
			{
			case ENCODING_GUID:
				strURN = pThis->m_oBTH.toString< Hashes::guidEncoding >();
				break;
			case ENCODING_BASE16:
				strURN = pThis->m_oBTH.toString< Hashes::base16Encoding >();
				break;
			case ENCODING_BASE32:
				strURN = pThis->m_oBTH.toString< Hashes::base32Encoding >();
				break;
			default:
				return E_INVALIDARG;
			}
		}
		break;

	default:
		return E_INVALIDARG;
	}

	strURN.SetSysString( psURN );

	return S_OK;
}

STDMETHODIMP CShareazaFile::XShareazaFile::get_URL(BSTR FAR* psURL)
{
	METHOD_PROLOGUE( CShareazaFile, ShareazaFile )
	pThis->m_sURL.SetSysString( psURL );
	return S_OK;
}
