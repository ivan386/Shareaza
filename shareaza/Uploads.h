//
// Uploads.h
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

#pragma once

class CBuffer;
class CConnection;
class CUploadTransfer;


class CUploads  
{
// Construction
public:
	CUploads();
	virtual ~CUploads();
	
// Attributes
public:
	DWORD		m_nCount;			// Active count
	DWORD		m_nBandwidth;		// Total speed
	DWORD		m_nTorrentSpeed;	// BitTorrent clamp
public:
	BOOL		m_bStable;			// Stable flag
	DWORD		m_nBestSpeed;		// Best speed
protected:
	CPtrList	m_pList;

// Operations
public:
	void		Clear(BOOL bMessage = TRUE);
	int			GetCount(CUploadTransfer* pExcept, int nState = -1) const;
public:
	BOOL		AllowMoreTo(IN_ADDR* pAddress) const;
	BOOL		EnforcePerHostLimit(CUploadTransfer* pUpload, BOOL bRequest = FALSE);
public:
	void		SetStable(DWORD nSpeed);
	DWORD		GetBandwidth() const;
	void		OnRun();
	BOOL		OnAccept(CConnection* pConnection, LPCTSTR pszHandshake);
	void		OnRename(LPCTSTR pszSource, LPCTSTR pszTarget = (LPCTSTR)1);
public:
	void		Add(CUploadTransfer* pUpload);
	void		Remove(CUploadTransfer* pUpload);

// List Access
public:
	inline POSITION GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}
	
	inline CUploadTransfer* GetNext(POSITION& pos) const
	{
		return (CUploadTransfer*)m_pList.GetNext( pos );
	}
	
	inline BOOL Check(CUploadTransfer* pUpload) const
	{
		return m_pList.Find( pUpload ) != NULL;
	}
	
	inline int GetTransferCount() const
	{
		return GetCount( NULL, -2 );
	}
	
};

extern CUploads Uploads;
