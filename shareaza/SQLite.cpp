//
// SQLite.cpp
//
// Copyright © Shareaza Development Team, 2002-2009.
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
#include "SQLite.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace SQLite;

//////////////////////////////////////////////////////////////////////////////
// CDatabase

CDatabase::CSQLitePtr::CSQLitePtr(LPCWSTR szDatabasePath) :
	m_db( NULL )
{
	if( sqlite3_open16( szDatabasePath, &m_db ) == SQLITE_OK )
	{
		sqlite3_busy_timeout( m_db, 1000 ); // 1 sec
	}
}

CDatabase::CSQLitePtr::~CSQLitePtr()
{
	if ( m_db )
	{
		sqlite3_close( m_db );
		m_db = NULL;
	}
}

CDatabase::CDatabase(LPCTSTR szDatabasePath)
{
	if ( szDatabasePath )
	{
		Open( szDatabasePath );
	}
}

CDatabase::~CDatabase()
{
}

CDatabase::CSQLiteSharedPtr CDatabase::GetHandle() const throw()
{
	return m_db;
}

CDatabase::operator bool() const throw()
{
	return m_db;
}

bool CDatabase::Open(LPCWSTR szDatabasePath)
{
	// Check if database already opened
	if ( ! m_db )
	{
		// Open database
		CSQLitePtr* pDatabase = new CSQLitePtr( szDatabasePath );
		if ( pDatabase->m_db )
			m_db.reset( pDatabase );
		else
			delete pDatabase;
	}
	return m_db;
}

bool CDatabase::Exec(LPCWSTR szQuery)
{
	CStatement st( *this, szQuery );
	while( st.Step() )
	{
		// Bypass
		st.Finalize();

		if ( ! st.IsPending() )
			// No more SQL commands
			return true;
	}
	return false;
}

LPCWSTR CDatabase::GetLastErrorMessage() const
{
	return m_db ? (LPCWSTR)sqlite3_errmsg16( m_db->m_db ) : NULL;
}


//////////////////////////////////////////////////////////////////////////////
// CStatement

CStatement::CStatement(const CDatabase& db, LPCWSTR pszQuery) :
	m_db( db.GetHandle() ),
	m_query( pszQuery ),
	m_st( NULL ),
	m_prepared( false ),
	m_busy( false )
{
}

CStatement::~CStatement()
{
	Finalize();
}

CStatement::operator bool() const throw()
{
	return ( m_st != NULL );
}

bool CStatement::IsPending() const throw()
{
	return ! m_query.empty();
}

bool CStatement::IsBusy() const throw()
{
	return m_busy;
}

int CStatement::GetCount() const throw()
{
	return sqlite3_data_count( m_st );
}

bool CStatement::Prepare()
{
	if ( m_db && ! m_prepared && m_query.size() )
	{
		m_busy = false;
		for(;;)
		{
			LPCWSTR pszTail = NULL;
			int rc = sqlite3_prepare16( m_db->m_db,
				(LPCVOID)m_query.c_str(), -1, &m_st, (LPCVOID*)&pszTail );
			switch( rc )
			{
			case SQLITE_OK:
				if ( pszTail && *pszTail )
					m_query = pszTail;
				else
				{
					// FIXIT: when SQL command ends with a space in a composite SQL query
					// it enters an infinite loop.
					m_query.clear();
				}

				if ( m_st == NULL )
					// This happens for a comment or white-space
					continue;

				m_prepared = true;
				break;

			case SQLITE_BUSY:
				m_busy = true;
				break;

			default:
				// To get good error message
				sqlite3_reset( m_st );
				break;
			}

			break;
		}
	}
	return m_prepared;
}

void CStatement::Reset()
{
	if ( m_st && m_prepared )
	{
		sqlite3_reset( m_st );
	}
	m_busy = false;
	m_raw.clear();
}

void CStatement::Finalize()
{
	if ( m_st )
	{
		sqlite3_finalize( m_st );
		m_st = NULL;
	}
	m_prepared = false;
	m_busy = false;
	m_raw.clear();
}

bool CStatement::Step()
{
	m_busy = false;
	m_raw.clear();

	if ( Prepare() )
	{
		int rc = sqlite3_step( m_st );
		switch ( rc )
		{
		case SQLITE_BUSY:
			m_busy = true;
			break;

		case SQLITE_ROW:
			{
				// Save column names
				int count = sqlite3_data_count( m_st );
				for ( int i = 0; i < count; i++ )
					m_raw.insert( CRaw::value_type(
						(LPCWSTR)sqlite3_column_name16( m_st, i ), i ) );
			}
			return true;

		case SQLITE_DONE:
			Finalize();
			return true;

		case SQLITE_ERROR:
		default:
			// To get good error message
			sqlite3_reset( m_st );
			break;
		}
	}
	return false;
}

int CStatement::GetColumn(LPCWSTR szName) const
{
	if ( szName )
	{
		CRaw::const_iterator i = m_raw.find( szName );
		if ( i != m_raw.end() && IsValidIndex( (*i).second ) )
			// Specified column
			return (*i).second;
	}
	else
	{
		if ( IsValidIndex( 0 ) )
			// First column
			return 0;
	}
	// Error
	return -1;
}

bool CStatement::IsValidIndex(int nIndex) const
{
	return m_st && nIndex >= 0 && nIndex < sqlite3_data_count( m_st );
}

int CStatement::GetType(LPCWSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
	{
		return sqlite3_column_type( m_st, column );
	}
	return 0;
}

int CStatement::GetInt(LPCWSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
	{
		return sqlite3_column_int( m_st, column );
	}
	return 0;
}

__int64 CStatement::GetInt64(LPCWSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
	{
		return sqlite3_column_int64( m_st, column );
	}
	return 0;
}

double CStatement::GetDouble(LPCWSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
	{
		return sqlite3_column_double( m_st, column );
	}
	return 0;
}

LPCWSTR CStatement::GetString(LPCWSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
	{
		return (LPCWSTR)sqlite3_column_text16( m_st, column );
	}
	return NULL;
}

LPCVOID CStatement::GetBlob(LPCWSTR pszName, int* pnLength) const
{
	if ( pnLength )
		*pnLength = 0;
	int column = GetColumn( pszName );
	if ( column != -1 )
	{
		if ( pnLength )
			*pnLength = sqlite3_column_bytes( m_st, column );
		return sqlite3_column_blob( m_st, column );
	}
	return NULL;
}

bool CStatement::Bind(int nNumber, int nData)
{
	if ( Prepare() )
	{
		return sqlite3_bind_int( m_st, nNumber, nData ) == SQLITE_OK;
	}
	return false;
}

bool CStatement::Bind(int nNumber, __int64 nData)
{
	if ( Prepare() )
	{
		return sqlite3_bind_int64( m_st, nNumber, nData ) == SQLITE_OK;
	}
	return false;
}

bool CStatement::Bind(int nNumber, double dData)
{
	if ( Prepare() )
	{
		return sqlite3_bind_double( m_st, nNumber, dData ) == SQLITE_OK;
	}
	return false;
}

bool CStatement::Bind(int nNumber, LPCWSTR szData)
{
	if ( Prepare() )
	{
		return sqlite3_bind_text16( m_st, nNumber, (LPVOID)szData,
			lstrlenW( szData ) * sizeof( WCHAR ), SQLITE_STATIC ) == SQLITE_OK;
	}
	return false;
}

bool CStatement::Bind(int nNumber, LPCVOID pData, int nLength)
{
	if ( Prepare() )
	{
		return sqlite3_bind_blob( m_st, nNumber,
			pData, nLength, SQLITE_STATIC ) == SQLITE_OK;
	}
	return false;
}
