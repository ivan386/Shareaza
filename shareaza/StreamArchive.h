//
// StreamArchive.h
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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

class CStreamArchive : public CArchive
{
public:
	CStreamArchive(UINT nMode, int nBufSize = 4096, void* lpBuf = NULL) throw() :
		CArchive( ( ( m_pStreamFile = new COleStreamFile ),
			( m_pStreamFile ? m_pStreamFile->CreateMemoryStream() : NULL ), m_pStreamFile ),
			nMode, nBufSize, lpBuf )
	{
	}

	CStreamArchive(IStream* pIStream, UINT nMode, int nBufSize = 4096, void* lpBuf = NULL) throw() :
		CArchive( ( ( m_pStreamFile = new COleStreamFile ),
			( m_pStreamFile ? m_pStreamFile->Attach( pIStream ) : NULL ), m_pStreamFile ),
			nMode, nBufSize, lpBuf )
	{
	}

	virtual ~CStreamArchive() throw()
	{
		if ( m_pFile )
		{
			Close();
		}

		delete m_pStreamFile;
	}

	inline operator LPSTREAM() throw()
	{
		return m_pStreamFile ? m_pStreamFile->m_lpStream : NULL;
	}

	inline LPSTREAM Detach() throw()
	{
		if ( m_pFile )
		{
			Close();
		}
		
		LPSTREAM pStream = m_pStreamFile ? m_pStreamFile->Detach() : NULL;
		
		delete m_pStreamFile;
		m_pStreamFile = NULL;

		return pStream;
	}

	inline bool IsValid() const throw()
	{
		return ( m_pStreamFile && ( m_pStreamFile->m_lpStream != NULL ) );
	}

protected:
	COleStreamFile*	m_pStreamFile;
};
