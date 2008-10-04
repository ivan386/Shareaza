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

#pragma once


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
	void	SetPieceSize(int nPieceIndex);
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
	BOOL	ProcessFile(DWORD nFile, LPCTSTR pszFile);
	BOOL	WriteOutput();
	
// Attributes
protected:
	CCriticalSection	m_pSection;
	BOOL				m_bActive;
	BOOL				m_bFinished;
	BOOL				m_bAbort;
	CString				m_sMessage;
	CString				m_sName;
	CString				m_sOutput;
	CString				m_sTracker;
	CString				m_sComment;
	CStringList			m_pFiles;
	CString				m_sThisFile;
	QWORD				m_nTotalSize;
	QWORD				m_nTotalPos;
	CSHA				m_oDataSHA1;	// Total SHA1
	CED2K				m_oDataED2K;	// Total MD4
	CMD5				m_oDataMD5;		// Total MD5
	CSHA				m_oPieceSHA1;	// (temporary)
	QWORD*				m_pFileSize;
	CSHA*				m_pFileSHA1;	// SHA1 per file
	CED2K*				m_pFileED2K;	// MD4 per file
	CMD5*				m_pFileMD5;		// MD5 per file
	CSHA*				m_pPieceSHA1;	// SHA1 per piece
	DWORD				m_nPieceSize;
	DWORD				m_nPieceCount;
	DWORD				m_nPiecePos;
	DWORD				m_nPieceUsed;
	BOOL				m_bAutoPieces;
	BYTE*				m_pBuffer;
	DWORD				m_nBuffer;
	
// Overrides
public:
	virtual BOOL InitInstance() { return TRUE; }
	virtual int Run();
	
	DECLARE_MESSAGE_MAP()
};
