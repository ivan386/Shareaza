//
// Uploads.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

class CShareazaFile;
class CBuffer;
class CConnection;
class CUploadTransfer;


class CUploads
{
// Construction
public:
	CUploads();
	~CUploads();

// Attributes
public:
	DWORD		m_nCount;			// Active count
	DWORD		m_nBandwidth;		// Total speed
	DWORD		m_nTorrentSpeed;	// BitTorrent clamp
public:
	BOOL		m_bStable;			// Stable flag
	DWORD		m_nBestSpeed;		// Best speed
protected:
	CList< CUploadTransfer* >	m_pList;

// Operations
public:
	void		Clear(BOOL bMessage = TRUE);
	DWORD		GetCount(CUploadTransfer* pExcept, int nState = -1) const;
	DWORD		GetTorrentCount(int nState) const;
public:
	BOOL		AllowMoreTo(const IN_ADDR* pAddress) const;
	BOOL		CanUploadFileTo(const IN_ADDR* pAddress, const CShareazaFile* pFile) const;
	BOOL		EnforcePerHostLimit(CUploadTransfer* pUpload, BOOL bRequest = FALSE);
public:
	void		SetStable(DWORD nSpeed);
	DWORD		GetBandwidth() const;
	// Calculate upload limit (Bytes/s)
	DWORD		GetBandwidthLimit() const;
	void		OnRun();
	BOOL		OnAccept(CConnection* pConnection);
	// Rename, delete or release uploading file.
	// pszTarget == 0 - delete file; pszTarget == 1 - release file.
	void		OnRename(LPCTSTR pszSource, LPCTSTR pszTarget);
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
		return m_pList.GetNext( pos );
	}

	inline BOOL Check(CUploadTransfer* pUpload) const
	{
		return m_pList.Find( pUpload ) != NULL;
	}

	inline INT_PTR GetTransferCount() const
	{
		return GetCount( NULL, -2 );
	}

	inline DWORD GetTorrentTransferCount() const
	{
		return GetTorrentCount( -2 );
	}

	inline DWORD GetTorrentUploadCount() const
	{
		return GetTorrentCount( -3 );
	}

};

extern CUploads Uploads;
