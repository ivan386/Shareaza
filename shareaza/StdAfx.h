//
// StdAfx.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

//! \file       StdAfx.h
//! \brief      Standard header for prcompiled header feature.
//!
//! Includes MFC header files. Contains several global definitions.

#pragma once

#ifndef RELEASE_BUILD
#define RELEASE_BUILD 0 // Set "1" for release and "0" for alpha, beta, etc. daily builds
#endif

//
// Configuration
//

#if 1

// Warnings that are normally ON by default
#pragma warning ( disable : 4350 )	// (Level 1)	behavior change: 'member1' called instead of 'member2'
#pragma warning ( disable : 4351 )	// (Level 1)	new behavior: elements of array 'array' will be default initialized

#pragma warning ( disable : 4244 )	// (Level 2)	'argument' : conversion from 'type1' to 'type2', possible loss of data

#pragma warning ( disable : 4347 )	// (Level 4)	behavior change: 'function template' is called instead of 'function'
#pragma warning ( disable : 4512 )	// (Level 4)	'class' : assignment operator could not be generated

// Warnings that are normally OFF by default (enabled by using /Wall)
#pragma warning ( disable : 4264 )	// (Level 1)	'virtual_function' : no override available for virtual member function from base 'class'; function is hidden
#pragma warning ( disable : 4555 )	// (Level 1)	expression has no effect; expected expression with side-effect
#pragma warning ( disable : 4711 )	// (Level 1)	function 'function' selected for inline expansion
#pragma warning ( disable : 4548 )	// (Level 1)	expression before comma has no effect; expected expression with side-effect

#pragma warning ( disable : 4191 )	// (Level 3)	'operator/operation' : unsafe conversion from 'type of expression' to 'type required'
#pragma warning ( disable : 4640 )	// (Level 3)	'instance' : construction of local static object is not thread-safe
#pragma warning ( disable : 4738 )	// (Level 3)	storing 32-bit float result in memory, possible loss of performance

#pragma warning ( disable : 4061 )	// (Level 4)	enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning ( disable : 4263 )	// (Level 4)	'function' : member function does not override any base class virtual member function
#pragma warning ( disable : 4266 )	// (Level 4)	'function' : no override available for virtual member function from base 'type'; function is hidden
#pragma warning ( disable : 4365 )	// (Level 4)	'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch
#pragma warning ( disable : 4514 )	// (Level 4)	'function' : unreferenced inline function has been removed
#pragma warning ( disable : 4571 )	// (Level 4)	Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
#pragma warning ( disable : 4625 )	// (Level 4)	'derived class' : copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning ( disable : 4626 )	// (Level 4)	'derived class' : assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning ( disable : 4710 )	// (Level 4)	'function' : function not inlined
#pragma warning ( disable : 4820 )	// (Level 4)	'bytes' bytes padding added after construct 'member_name'

#define _SCL_SECURE_NO_WARNINGS		// For RegExp only
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1

#endif

// Target features available from Windows Vista onwards.
//	To show features that need guards for Windows 2000/XP compatibility use:
//	#define NTDDI_VERSION	NTDDI_WIN2K
//	#define _WIN32_WINNT	0x0500
#define NTDDI_VERSION	NTDDI_LONGHORN	// Minimum build target
#define _WIN32_WINNT	0x0600			// Vista, 2008
#include <sdkddkver.h>					// Setup versioning for windows SDK/DDK

// Add defines missed/messed up when Microsoft converted to NTDDI macros
#define WINXP			0x05010000		// rpcdce.h, rpcdcep.h
#define NTDDI_XP		0x05010000		// ipexport.h, iphlpapi.h
#define NTDDI_WXP		0x05010000		// rpcasync.h
#define NTDDI_XPSP1		0				// 0x05010100	// ipmib.h (leave as 0 due to broken struct)
#define NTDDI_XPSP2		0x05010200		// shellapi.h
#define NTDDI_WIN2K3	0				// 0x05020000	// docobj.h (leave as 0 due to broken enum)
#define NTDDI_WINLH		0x06000000		// objidl.h
#define NTDDK_VERSION	NTDDI_VERSION	// winioctl.h

#define VC_EXTRALEAN

#define _ATL_NO_COM_SUPPORT
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#define BOOST_USE_WINDOWS_H
#define BOOST_DISABLE_ASSERTS

#pragma warning( push, 0 )			// Suppress Microsoft warnings

//
// MFC
//

#include <afxwin.h>			// MFC core and standard components
#include <afxext.h>			// MFC extensions
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxtempl.h>		// MFC templates
#include <afxmt.h>			// MFC threads
#include <afxole.h>			// MFC OLE
#include <afxocc.h>			// MDC OCC
#include <afxhtml.h>		// MFC HTML
#include <afxpriv.h>
#include <../src/mfc/afximpl.h>

//
// WIN32
//

#include <winsock2.h>		// Windows sockets V2
#include <wininet.h>		// Internet
#include <wincrypt.h>		// Cryptographic API
#include <ddeml.h>			// DDE
#include <math.h>			// Math
#include <winsvc.h>			// Services (excluded by VC_EXTRALEAN)
#include <shlwapi.h>		// Windows Shell API
#include <exdispid.h>		// Internet Explorer DISPIDs
#include <mmsystem.h>		// Multimedia
#include <winioctl.h>		// Sparse files support
#include <atltime.h>		// Time classes
#include <atlenc.h>			// Base64Encode, UUEncode etc.
#include <atlfile.h>		// Thin file classes

// If this header is not found, you'll need to install the Windows XP SP2 Platform SDK (or later)
// from http://www.microsoft.com/msdownload/platformsdk/sdkupdate/

#include <netfw.h>
#include <upnp.h>
#include <natupnp.h>
#include <iphlpapi.h>
#include <MsiQuery.h>
#include <MsiDefs.h>
#include <Powrprof.h>		// The power policy applicator

// Work-around for Microsoft double declaration
#define _interlockedbittestandset _ms_set
#define _interlockedbittestandreset _ms_reset
#define _interlockedbittestandset64 _ms_set64
#define _interlockedbittestandreset64 _ms_reset64
#include <intrin.h>
#undef _interlockedbittestandset
#undef _interlockedbittestandreset
#undef _interlockedbittestandset
#undef _interlockedbittestandreset

//
// STL
//

#include <vector>
#include <list>
#include <deque>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <functional>
#include <algorithm>
#include <memory>
#include <iterator>
#include <limits>
#include <new>

//
// Boost
//

#ifndef _WIN64
	#define BOOST_BIND_ENABLE_STDCALL 1
	#define BOOST_MEM_FN_ENABLE_STDCALL 1
#endif

#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/type_traits.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/array.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/checked_delete.hpp>

//
// Standard headers
//

#include "../zlib/zlib.h"
#include "../bzlib/bzlib.h"

#include "MinMax.hpp"

// Work-around for VC9 where a (pop) is ifdef'd out in stdio.h
#if _MSC_VER >= 1500 && _MSC_VER < 1600
	#pragma warning( pop )
#endif

#pragma warning( pop )				// Restore warnings

#include "augment/augment.hpp"
using augment::implicit_cast;
using augment::auto_ptr;
using augment::auto_array;
using augment::IUnknownImplementation;

#include "../HashLib/HashLib.h"

// BugTrap http://www.intellesoft.net/
#include "../BugTrap/BugTrap.h"

// GeoIP http://geolite.maxmind.com/
#include "../GeoIP/GeoIP.h"

#include "../RegExp/RegExp.h"

typedef CString StringType;

//! \brief Hash function needed for CMap with const CString& as ARG_KEY.
template<> AFX_INLINE UINT AFXAPI HashKey(const CString& key)
{
	return HashKey< LPCTSTR >( key );
}

template<> AFX_INLINE BOOL AFXAPI CompareElements(const IN_ADDR* pElement1, const IN_ADDR* pElement2)
{
	return pElement1->s_addr == pElement2->s_addr;
}

template<> AFX_INLINE UINT AFXAPI HashKey(const IN_ADDR& key)
{
	return key.s_addr;
}

#include "Hashes.hpp"

#undef IDC_HAND		// Defined in Windows.h->WinUser.h and in Resource.h

#include "Resource.h"

#include "ShareazaOM.h"

//
// Smaller type check fix (/RTCc)
//

#ifdef _DEBUG
	#undef GetRValue
	#define GetRValue(rgb)      ((BYTE)( (rgb)        & 0xff))
	#undef GetGValue
	#define GetGValue(rgb)      ((BYTE)(((rgb) >>  8) & 0xff))
	#undef GetBValue
	#define GetBValue(rgb)      ((BYTE)(((rgb) >> 16) & 0xff))
#endif

//
// 64-bit type
//

typedef uint64 QWORD;
const QWORD SIZE_UNKNOWN = ~0ull;

#define	MAKEDWORD(a,b)	((DWORD) (((a)) | ((DWORD) ((b))) << 16))
#define	MAKEQWORD(a,b)	((QWORD) (((a)) | ((QWORD) ((b))) << 32))

inline CArchive& AFXAPI operator<<(CArchive& ar, const TRISTATE& n)
{
	int tmp = static_cast< int >( n );
	return ar << tmp;
}

inline CArchive& AFXAPI operator>>(CArchive& ar, TRISTATE& n)
{
	int tmp;
	ar >> tmp;
	n = static_cast< TRISTATE >( tmp );
	return ar;
}

#pragma pack( push, 1 )

typedef struct _ICONDIRENTRY
{
	BYTE        bWidth;          // Width, in pixels, of the image
	BYTE        bHeight;         // Height, in pixels, of the image
	BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
	BYTE        bReserved;       // Reserved ( must be 0)
	WORD        wPlanes;         // Color Planes
	WORD        wBitCount;       // Bits per pixel
	DWORD       dwBytesInRes;    // How many bytes in this resource?
	DWORD       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct _GRPICONDIRENTRY
{
	BYTE   bWidth;               // Width, in pixels, of the image
	BYTE   bHeight;              // Height, in pixels, of the image
	BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
	BYTE   bReserved;            // Reserved
	WORD   wPlanes;              // Color Planes
	WORD   wBitCount;            // Bits per pixel
	DWORD  dwBytesInRes;         // how many bytes in this resource?
	WORD   nID;                  // the ID
} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;

typedef struct _ICONDIR
{
	WORD           idReserved;   // Reserved (must be 0)
	WORD           idType;       // Resource Type (1 for icons)
	WORD           idCount;      // How many images?
//	ICONDIRENTRY   idEntries[];  // An entry for each image (idCount of 'em)
} ICONDIR, *LPICONDIR;

#pragma pack( pop )

//
// Protocol IDs
//

enum PROTOCOLID
{
	PROTOCOL_ANY  = -1,
	PROTOCOL_NULL = 0,
	PROTOCOL_G1   = 1,
	PROTOCOL_G2   = 2,
	PROTOCOL_ED2K = 3,
	PROTOCOL_HTTP = 4,
	PROTOCOL_FTP  = 5,
	PROTOCOL_BT   = 6,
	PROTOCOL_KAD  = 7,
	PROTOCOL_DC   = 8,
	PROTOCOL_LAST = 9
};

struct ProtocolCmdIDMapEntry
{
	BYTE	protocol;
	DWORD	commandID;
};

const ProtocolCmdIDMapEntry protocolCmdMap[] =
{
	{ PROTOCOL_NULL,	ID_NETWORK_NULL },
	{ PROTOCOL_G1,		ID_NETWORK_G1 },
	{ PROTOCOL_G2,		ID_NETWORK_G2 },
	{ PROTOCOL_ED2K,	ID_NETWORK_ED2K },
	{ PROTOCOL_HTTP,	ID_NETWORK_HTTP },
	{ PROTOCOL_FTP,		ID_NETWORK_FTP },
	{ PROTOCOL_BT,		ID_NETWORK_BT },
	{ PROTOCOL_KAD,		ID_NETWORK_KAD },
	{ PROTOCOL_DC,		ID_NETWORK_DC }
};

inline PROTOCOLID& operator++(PROTOCOLID& arg)
{
	ASSERT( arg < PROTOCOL_LAST - 1 );
	arg = PROTOCOLID( arg + 1 );
	return arg;
}

inline PROTOCOLID& operator--(PROTOCOLID& arg)
{
	ASSERT( arg > PROTOCOL_ANY );
	arg = PROTOCOLID( arg - 1 );
	return arg;
}

inline CArchive& operator<<(CArchive& ar, const PROTOCOLID& rhs)
{
	int value = rhs;
	return ar << value;
}

inline CArchive& operator>>(CArchive& ar, PROTOCOLID& rhs)
{
	int value;
	ar >> value;
	if ( !( value >= PROTOCOL_ANY && value < PROTOCOL_LAST ) )
		AfxThrowUserException();
	rhs = ( value >= PROTOCOL_ANY && value < PROTOCOL_LAST ) ?
		PROTOCOLID( value ) : PROTOCOL_NULL;
	return ar;
}


class CQuickLock
{
public:
	explicit CQuickLock(CSyncObject& oMutex) : m_oMutex( oMutex ) { oMutex.Lock(); }
	~CQuickLock() { m_oMutex.Unlock(); }
private:
	CSyncObject& m_oMutex;
	CQuickLock(const CQuickLock&);
	CQuickLock& operator=(const CQuickLock&);
	static void* operator new(std::size_t);
	static void* operator new[](std::size_t);
	static void operator delete(void*);
	static void operator delete[](void*);
	CQuickLock* operator&() const;
};

template< class T >
class CGuarded
{
public:
	explicit CGuarded() : m_oSection(), m_oValue() { }
	explicit CGuarded(const CGuarded& other) : m_oSection(), m_oValue( other ) { }
	CGuarded(const T& oValue) : m_oSection(), m_oValue( oValue ) { }
	CGuarded& operator=(const T& oValue)
	{
		CQuickLock oLock( m_oSection );
		m_oValue = oValue;
		return *this;
	}
	operator T() const
	{
		CQuickLock oLock( m_oSection );
		return m_oValue;
	}
private:
	mutable CCriticalSection m_oSection;
	T m_oValue;

	CGuarded* operator&() const; // too unsafe
	CGuarded& operator=(const CGuarded&);
};

typedef boost::shared_ptr< CCriticalSection > CCriticalSectionPtr;

template< typename T, typename L >
class CLocked
{
public:
	CLocked(const CLocked& pGB) :
		m_oValue( pGB.m_oValue ),
		m_oLock( pGB.m_oLock )
	{
		m_oLock->Lock();
	}

	CLocked(T oValue, L oLock) :
		m_oValue( oValue ),
		m_oLock( oLock )
	{
		m_oLock->Lock();
	}

	~CLocked()
	{
		m_oLock->Unlock();
	}
	
	operator T() const throw()
	{
		return m_oValue;
	}

	T operator->() const throw()
	{
		return m_oValue;
	}

private:
	T	m_oValue;
	L	m_oLock;

	CLocked* operator&() const;
	CLocked& operator=(const CLocked&);
};

#ifdef _DEBUG

	// Assume we already entered to this lock
	#define ASSUME_LOCK(lock) \
	if ( (lock).m_nEnterCount < 1 || (lock).m_nThreadId != (LONG)GetCurrentThreadId() ) { \
		static char BUF[1024] = {}; \
		strcpy_s(BUF,1024,THIS_FILE); \
		strcat_s(BUF,1024,"\n\nThis code must be protected by " #lock "!"); \
		if ( ::AfxAssertFailedLine(BUF, __LINE__) ) AfxDebugBreak(); }

	// Assume we already entered to this lock only once
	#define ASSUME_SINGLE_LOCK(lock) \
	if ( (lock).m_nEnterCount != 1 || (lock).m_nThreadId != (LONG)GetCurrentThreadId() ) { \
		static char BUF[1024] = {}; \
		strcpy_s(BUF,1024,THIS_FILE); \
		strcat_s(BUF,1024,"\n\nThis code must be protected by " #lock "!"); \
		if ( ::AfxAssertFailedLine(BUF, __LINE__) ) AfxDebugBreak(); }

	class CMutexEx : public CMutex
	{
	public:
		CMutexEx(BOOL bInitiallyOwn = FALSE, LPCTSTR lpszName = NULL, LPSECURITY_ATTRIBUTES lpsaAttribute = NULL)
			: CMutex( bInitiallyOwn, lpszName, lpsaAttribute )
			, m_nThreadId( 0 )
			, m_nEnterCount( 0 )
		{
		}

		virtual BOOL Lock(DWORD dwTimeout = INFINITE)
		{
			if ( CMutex::Lock( dwTimeout ) )
			{
				InterlockedIncrement( &m_nEnterCount );
				InterlockedCompareExchange( &m_nThreadId, (LONG)GetCurrentThreadId(), 0 );
				return TRUE;
			}
			else
				return FALSE;
		}

		virtual BOOL Unlock()
		{
			if ( m_nThreadId && InterlockedDecrement( &m_nEnterCount ) == 0 )
				InterlockedExchange( &m_nThreadId, 0 );
			return CMutex::Unlock();
		}

		volatile LONG m_nThreadId;		// Owner thread
		volatile LONG m_nEnterCount;	// Re-enter counter

	private:
		CMutexEx(const CMutexEx&);
		CMutexEx& operator=(const CMutexEx&);
	};

#else	// _DEBUG

	#define ASSUME_LOCK(lock) ((void)0)
	#define ASSUME_SINGLE_LOCK(lock) ((void)0)
	typedef CMutex CMutexEx;

#endif	// _DEBUG

#ifdef _DEBUG
	#define VERIFY_FILE_ACCESS(h,f) \
	{ \
		if ( ( h ) == INVALID_HANDLE_VALUE ) \
		{ \
			DWORD err = GetLastError(); \
			theApp.Message( MSG_DEBUG, _T("File error \"%s\": %s (0x%08x)"), \
				LPCTSTR( f ), LPCTSTR( GetErrorString( err ) ), err ); \
		} \
	}
#else
	#define VERIFY_FILE_ACCESS(h,f) ((void)0);
#endif

template<>
struct std::less< CLSID > : public std::binary_function< CLSID, CLSID, bool >
{
	inline bool operator()(const CLSID& _Left, const CLSID& _Right) const throw()
	{
		return _Left.Data1 < _Right.Data1 || ( _Left.Data1 == _Right.Data1 &&
			( _Left.Data2 < _Right.Data2 || ( _Left.Data2 == _Right.Data2 &&
			( _Left.Data3 < _Right.Data3 || ( _Left.Data3 == _Right.Data3 &&
			( memcmp( _Left.Data4, _Right.Data4, 8 ) < 0 ) ) ) ) ) );
	}
};

template<>
struct std::less< CString > : public std::binary_function< CString, CString, bool>
{
	inline bool operator()(const CString& _Left, const CString& _Right) const throw()
	{
		return ( _Left.CompareNoCase( _Right ) < 0 );
	}
};

typedef std::set < CString > string_set;

#define IsIn(x,y) ((x.find((y)))!=(x.end()))

inline UINT ReadArchive(CArchive& ar, void* lpBuf, const UINT nMax)
{
	UINT nReaded = ar.Read( lpBuf, nMax );
	if ( nReaded != nMax )
		AfxThrowArchiveException( CArchiveException::endOfFile );
	return nReaded;
}

// The GetMicroCount function retrieves the number of microseconds that have elapsed
// since the application was started.
__int64 GetMicroCount();

// Produces the best hash table size for CMap::InitHashTable use
UINT GetBestHashTableSize(UINT nCount);

// Encode Unicode text to UTF-8 text
CStringA UTF8Encode(__in const CStringW& strInput);
CStringA UTF8Encode(__in_bcount(nInput) LPCWSTR psInput, __in int nInput);

// Decode UTF-8 text to Unicode text
CStringW UTF8Decode(__in const CStringA& strInput);
CStringW UTF8Decode(__in_bcount(nInput) LPCSTR psInput, __in int nInput);

// Encode and decode URL text, and see if a string starts with a tag
CString URLEncode(LPCTSTR pszInput);                   // Encode "hello world" into "hello%20world"
CString URLDecode(LPCTSTR pszInput);                   // Decode "hello%20world" back to "hello world"
CString URLDecodeANSI(LPCTSTR pszInput);               // Decodes properly encoded URLs
CString URLDecodeUnicode(LPCTSTR pszInput);            // Decodes URLs with extended characters

// StartsWith("hello world", "hello") is true
inline BOOL StartsWith(const CString& sInput, LPCTSTR pszText, const int len)
{
	return ( sInput.GetLength() >= len ) && ! _tcsnicmp( (LPCTSTR)sInput, pszText, len );
}

// Compute average of values collected by specified time
template< class T, DWORD dwMilliseconds >
class CTimeAverage
{
public:
	CTimeAverage()
	{
	}

	inline T operator()(T Val)
	{
		// Add new value
		DWORD dwNow = GetTickCount();
		m_Data.push_back( CAveragePair( Val, dwNow ) );

		// Remove outdated values
		while ( m_Data.size() > 1 )
		{
			if ( dwNow - (*(++m_Data.begin())).second < dwMilliseconds )
				break;
			m_Data.pop_front();
		}

		// Calculate average
		T sum = 0;
		for ( CAverageList::const_iterator i = m_Data.begin(); i != m_Data.end(); ++i )
			sum += (*i).first;
		return sum / (T)m_Data.size();
	}

protected:
	typedef std::pair< T, DWORD > CAveragePair;
	typedef std::list< CAveragePair > CAverageList;
	CAverageList m_Data;

private:
	CTimeAverage(const CTimeAverage&);
	CTimeAverage* operator&() const;
	CTimeAverage& operator=(const CTimeAverage&);
};

template< class T >
inline void SafeRelease(CComPtr< T >& pObj) throw()
{
	__try
	{
		pObj.Release();
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		pObj.Detach();
	}
}

inline bool IsFileNewerThan(LPCTSTR pszFile, const QWORD nMilliseconds)
{
	WIN32_FILE_ATTRIBUTE_DATA fd = {};
	if ( ! GetFileAttributesEx( pszFile, GetFileExInfoStandard, &fd ) )
		return false;

	FILETIME ftNow = {};
	GetSystemTimeAsFileTime( &ftNow );

	if ( ( MAKEQWORD( ftNow.dwLowDateTime, ftNow.dwHighDateTime ) -
		10000ull * nMilliseconds ) > MAKEQWORD( fd.ftLastWriteTime.dwLowDateTime,
		fd.ftLastWriteTime.dwHighDateTime ) )
		return false;

	return true;
}

inline QWORD GetFileSize(LPCTSTR pszFile)
{
	WIN32_FILE_ATTRIBUTE_DATA fd = {};
	if ( GetFileAttributesEx( pszFile, GetFileExInfoStandard, &fd ) )
		return MAKEQWORD( fd.nFileSizeLow, fd.nFileSizeHigh );
	else
		return SIZE_UNKNOWN;
}

// Powered version of AfxMessageBox()
// nType				| *pnDefault
// MB_OK				| 0 - ask, 1 - IDOK
// MB_OKCANCEL			| 0 - ask, 1 - IDOK, 2 - IDCANCEL
// MB_ABORTRETRYIGNORE	| 0 - ask, 1 - IDABORT, 2 - IDRETRY, 3 - IDIGNORE
// MB_YESNOCANCEL		| 0 - ask, 1 - IDNO, 2 - IDYES, 3 - IDCANCEL
// MB_YESNO				| 0 - ask, 1 - IDNO, 2 - IDYES
// MB_RETRYCANCEL		| 0 - ask, 1 - IDRETRY, 2 - IDCANCEL
// MB_CANCELTRYCONTINUE	| 0 - ask, 1 - IDCANCEL, 2 - IDTRYAGAIN, 3 - IDCONTINUE
INT_PTR MsgBox(LPCTSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0, DWORD* pnDefault = NULL);
INT_PTR MsgBox(UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = 0, DWORD* pnDefault = NULL);
#undef AfxMessageBox
#define AfxMessageBox MsgBox

#undef  _stscanf
#define _stscanf _stscanf_s	// Don't forget that %s, %c and [ requires buffer size parameter.

#define SERVERLOST(hr) \
	(((hr)==MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,RPC_S_SERVER_UNAVAILABLE))||\
	((hr)==CO_E_OBJNOTCONNECTED)||\
	((hr)==RPC_E_SERVERFAULT)||\
	((hr)==RPC_E_INVALID_OBJECT))
