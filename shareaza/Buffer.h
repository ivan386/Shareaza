//
// Buffer.h
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

// Only include the lines beneath this one once
#pragma once

// A buffer of memory that takes care of allocating and freeing itself, and has methods for compression and encoding
class CBuffer
{

// Construction
public:

	// Make a new CBuffer object, and delete one
	CBuffer(DWORD* pLimit = NULL); // The argument is not used (do)
	virtual ~CBuffer();            // The virtual keyword indicates a class that inherits from this one may override this

// Attributes
public:

	// Memory pointers and byte counts
	CBuffer* m_pNext;   // A pointer to the next CBuffer object, letting them be linked together in a list
	BYTE*    m_pBuffer; // The block of allocated memory
	DWORD    m_nLength; // The number of bytes we have written into the block
	DWORD    m_nBuffer; // The size of the allocated block

// Operations
public:

	// Add and remove data from the memory block in the CBuffer object
	void  Add(const void* pData, DWORD nLength);                   // Add data to the end of the block already in this object
	void  Insert(DWORD nOffset, const void* pData, DWORD nLength); // Insert the data in the middle somewhere
	void  Remove(DWORD nLength);                                   // Delete the first nLength bytes from the memory block
	void  Clear();                                                 // Mark the entire buffer empty, not changing the size of the allocated block

	// Add text to the buffer, does not add a null terminator
	void  Print(LPCSTR pszText);                           // Add ASCII text to the buffer
	void  Print(LPCWSTR pszText, UINT nCodePage = CP_ACP); // Convert Unicode text to ASCII and add it to the buffer

	// Copy all or part of the data in another CBuffer object into this one
	DWORD AddBuffer(CBuffer* pBuffer, DWORD nLength = 0xFFFFFFFF); // Length is -1 by default to copy the whole thing
	void  AddReversed(const void* pData, DWORD nLength);           // Add data to this buffer, but with the bytes in reverse order
	void  Prefix(LPCSTR pszText);                                  // Add ASCII text to the start of this buffer, shifting everything else forward
	void  EnsureBuffer(DWORD nLength);                             // Tell the buffer to prepare to recieve this number of additional bytes

public:

	// Read the data in the buffer as text
	CString ReadString(DWORD nBytes, UINT nCodePage = CP_ACP);                       // Reads nBytes of ASCII characters as a string
	BOOL    ReadLine(CString& strLine, BOOL bPeek = FALSE, UINT nCodePage = CP_ACP); // Reads until "\r\n"
	BOOL    StartsWith(LPCSTR pszString, BOOL bRemove = FALSE);                      // Returns true if the buffer starts with this text

	// Use the buffer with a socket
	DWORD Receive(SOCKET hSocket); // Move incoming data from the socket to this buffer
	DWORD Send(SOCKET hSocket);    // Send the contents of this buffer to the computer on the far end of the socket

	// Use the buffer with the ZLib compression library
	BOOL Deflate(BOOL bIfSmaller = FALSE); // Compress the data in this buffer
	BOOL Inflate(DWORD nSuggest = 0);      // Remove the compression on the data in this buffer
	BOOL Ungzip();                         // Delete the gzip header and then remove the compression

	// Read and write a DIME message in the buffer
	void WriteDIME(DWORD nFlags, LPCSTR pszID, LPCSTR pszType, LPCVOID pBody, DWORD nBody);
	BOOL ReadDIME(DWORD* pnFlags, CString* psID, CString* psType, DWORD* pnBody);

public:

	// Static means you can call CBuffer::ReverseBuffer without having a CBuffer object at all
	static void ReverseBuffer(const void* pInput, void* pOutput, DWORD nLength);
};
