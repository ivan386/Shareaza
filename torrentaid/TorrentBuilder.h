//
// TorrentBuilder.h
//
// Copyright (c) Shareaza Development Team, 2007.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#if !defined(TORRENTBUILDER_H)
#define TORRENTBUILDER_H

#pragma once

#include "SHA1.h"
#include "MD4.h"
#include "ED2K.h"

class CTorrentBuilder : public CWinThread
{
// Construction
public:
	CTorrentBuilder();
	virtual ~CTorrentBuilder();
	
	DECLARE_DYNCREATE(CTorrentBuilder)
	
// Operations
public:
	BOOL	SetName(LPCTSTR pszName);
	BOOL	SetOutputFile(LPCTSTR pszPath);
	void	SetPieceSize(BOOL bAutoPieces);
	BOOL	AddFile(LPCTSTR pszPath);
	BOOL	AddTrackerURL(LPCTSTR pszURL);
	BOOL	SetComment(LPCTSTR pszComment);
public:
	BOOL	Start();
	void	Stop();
	BOOL	SetPriority(int nPriority);
	BOOL	IsRunning();
	BOOL	IsFinished();
	BOOL	GetTotalProgress(DWORD& nPosition, DWORD& nScale);
	BOOL	GetCurrentFile(CString& strFile);
	BOOL	GetMessageString(CString& strMessage);
protected:
	BOOL	ScanFiles();
	BOOL	ProcessFiles();
	BOOL	ProcessFile(LPCTSTR pszFile);
	BOOL	WriteOutput();
	
// Attributes
protected:
	CCriticalSection	m_pSection;
	BOOL				m_bActive;
	BOOL				m_bFinished;
	BOOL				m_bAbort;
	CString				m_sMessage;
protected:
	CString				m_sName;
	CString				m_sOutput;
	CString				m_sTracker;
	CString				m_sComment;
	CStringList			m_pFiles;
protected:
	CString				m_sThisFile;
	QWORD				m_nTotalSize;
	QWORD				m_nTotalPos;
protected:
	CHashSHA1			m_pDataSHA1;
	CHashMD4			m_pDataED2K;
	QWORD*				m_pFileSize;
	CHashSHA1*			m_pFileSHA1;
	CHashMD4*			m_pFileED2K;
protected:
	CHashSHA1*			m_pPieceSHA1;
	DWORD				m_nPieceSize;
	DWORD				m_nPieceCount;
	DWORD				m_nPiecePos;
	DWORD				m_nPieceUsed;
	BOOL				m_bAutoPieces;
protected:
	BYTE*				m_pBuffer;
	DWORD				m_nBuffer;
	CSHA1*				m_phPieceSHA1;
	CSHA1*				m_phFullSHA1;
	CSHA1*				m_phFileSHA1;
	CED2K*				m_phFullED2K;
	CED2K*				m_phFileED2K;
	
// Overrides
public:
	virtual BOOL InitInstance() { return TRUE; }
	virtual int Run();
	
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(ATORRENTBUILDER_H)
