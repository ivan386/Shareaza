//
// LibraryBuilder.h
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

#if !defined(AFX_LIBRARYBUILDER_H__B2779061_437E_4C10_AC9F_1E2AC7885C40__INCLUDED_)
#define AFX_LIBRARYBUILDER_H__B2779061_437E_4C10_AC9F_1E2AC7885C40__INCLUDED_

#pragma once

class CLibraryFile;
class CXMLElement;
class CLibraryBuilderInternals;
class CLibraryBuilderPlugins;

#include "SHA.h"
#include "TigerTree.h"
#include "MD5.h"
#include "ED2K.h"
#include "asm\common.inc"

class CLibraryBuilder  
{
// Construction
public:
	CLibraryBuilder();
	virtual ~CLibraryBuilder();
	
// Attributes
protected:
	CCriticalSection	m_pSection;
	CPtrList			m_pFiles;
//	CStringList			m_pPriority;	// not used anymore
	HANDLE				m_hThread;
	BOOL				m_bThread;
	BOOL				m_bPriority;
//	DWORD				m_nHashSleep;
	DWORD				m_nIndex;
	CString				m_sPath;
	DWORD				m_tActive;
	BYTE*				m_pBuffer;
	BOOL				m_bRetired;
protected:
	CLibraryBuilderInternals*	m_pInternals;
	CLibraryBuilderPlugins*		m_pPlugins;

// Operations
public:
	void		Add(CLibraryFile* pFile);
	void		Remove(CLibraryFile* pFile);
	int			GetRemaining();
	CString		GetCurrentFile();
	void		RequestPriority(LPCTSTR pszPath);
	void		Clear();
	BOOL		StartThread();
	void		StopThread();
	void		BoostPriority(BOOL bPriority);
	BOOL		GetBoostPriority();
	BOOL		SanityCheck();
protected:
	static UINT	ThreadStart(LPVOID pParam);
	void		OnRun();
	void		HashFile(BOOL bForceHash);
	BOOL		SubmitMetadata(LPCTSTR pszSchemaURI, CXMLElement* pXML);
	BOOL		SubmitCorrupted();
	BOOL		DetectVirtualFile(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	BOOL		DetectVirtualID3v1(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	BOOL		DetectVirtualID3v2(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	
	friend class CLibraryBuilderInternals;
	friend class CLibraryBuilderPlugins;
protected:
	struct
	{
		HANDLE hFile;
		DWORD m_nIndex;
		CString	m_sPath;
		DWORD nSizeHigh;
		DWORD nSizeLow;
		QWORD nFileSize;
		QWORD nFileBase;
		DWORD nStoredBytes;
		QWORD nCount;											// Filelength-Countdown
		BOOL bVirtual;
		CTigerTree* pTiger;
		CED2K* pED2K;
		CSHA1* pSHA1;
		CMD5* pMD5;
		BYTE* m_pBuffer;										// BasePointer
		BYTE* m_pBufferIndex;
	} m_qStack[MAX_PARALLEL], m_qStackN, m_qStackO;
			DWORD	m_nStackTop;
			BOOL	m_bSuspend, m_bSuspended;
			BOOL	m_bTerminate;
			BOOL	m_bListChanged;
	static	DWORD	m_nStackSize;
	static	DWORD	m_nMaxBlock;
	static	void	Init();
			void	DoHash1(), DoHash2(), DoHash3(), DoHash4(), DoHash5(), DoHash6();
	typedef	void	(CLibraryBuilder::*tpDoHash)();
	const static tpDoHash pDoHash[MAX_PARALLEL];
	inline	void	RequestSuspendBuilder();
	inline	void	ContinueBuilder();
	inline	void	RequestTerminateBuilder();
	inline	void	SignalListChanged();
};

extern CLibraryBuilder LibraryBuilder;

inline void CLibraryBuilder::RequestSuspendBuilder()
{
	m_bSuspend = TRUE;
}
inline void CLibraryBuilder::ContinueBuilder()
{
	m_bSuspend = FALSE;
}
inline void	CLibraryBuilder::RequestTerminateBuilder()
{
	m_bTerminate = TRUE;
}
inline void	CLibraryBuilder::SignalListChanged()
{
	m_bListChanged = TRUE;
}

#endif // !defined(AFX_LIBRARYBUILDER_H__B2779061_437E_4C10_AC9F_1E2AC7885C40__INCLUDED_)
