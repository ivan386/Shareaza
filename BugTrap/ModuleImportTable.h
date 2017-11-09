/*
 * This is a part of the BugTrap package.
 * Copyright (c) 2005-2009 IntelleSoft.
 * All rights reserved.
 *
 * Description: Table of modules that import SetUnhandledExceptionFilter() function.
 * Author: Maksim Pyatkovskiy.
 *
 * This source code is only intended as a supplement to the
 * BugTrap package reference and related electronic documentation
 * provided with the product. See these sources for detailed
 * information regarding the BugTrap package.
 */

#pragma once

#include "Hash.h"

/// Macro for adding pointers/DWORDs together without C arithmetic interfering. Taken from Matt Pietrek's book.
#define MakePtr(cast, ptr, offset) ((cast)((DWORD_PTR)(ptr) + (DWORD_PTR)(offset)))

/// Table of modules that import SetUnhandledExceptionFilter() function.
template < typename T >
class CMIT
{
public:
	/// Object constructor.
	inline CMIT(T pDummy, LPCSTR szModule) :
		m_pDummy( pDummy ),
		m_szModule( szModule )
	{
	}

	/// Object destructor.
	inline ~CMIT()
	{
		Restore(NULL);
	}

	/// Override SetUnhandledExceptionFilter().
	inline void Override(HMODULE hModule)
	{
		TraverseImportTables(hModule, TRUE, 0);
	}

	/// Restore original function pointers.
	inline void Restore(HMODULE hModule)
	{
		TraverseImportTables(hModule, FALSE, 0);
	}

	/// Override/restore original function pointers.
	inline void Intercept(HMODULE hModule, BOOL bOverride)
	{
		TraverseImportTables(hModule, bOverride, 0);
	}

	/// Free module entries.
	inline void Clear(void)
	{
		m_mapModuleEntries.DeleteAll(true);
	}

	/// Return original address of T().
	inline T GetOriginalProcAddress(void) const;

private:
	T		m_pDummy;
	LPCSTR	m_szModule;
	/// Map of module entries.
	CHash< HMODULE, T > m_mapModuleEntries;

	/// Protects object from being accidentally copied.
	CMIT(const CMIT& rModuleImportTable);
	/// Protects object from being accidentally copied.
	CMIT& operator=(const CMIT& rModuleImportTable);

	/// Traverse import tables recurrently and replace SetUnhandledExceptionFilter().
	inline void TraverseImportTables(HMODULE hModule, BOOL bOverride, size_t nNestedLevel)
	{
		BOOL bCleanup = FALSE;
		if (nNestedLevel == 0)
		{
			HMODULE hMainModule = GetModuleHandle(NULL);
			if (hModule == NULL)
				hModule = hMainModule;
			if (hModule == hMainModule && ! bOverride)
				bCleanup = TRUE;
		}
		T pfnSavedSetUnhandledExceptionFilter = NULL;
		BOOL bFound = m_mapModuleEntries.Lookup(hModule, pfnSavedSetUnhandledExceptionFilter);
		if (bOverride ? bFound : ! bFound)
			return;
		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;
		if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return;
		PIMAGE_NT_HEADERS pNTHeaders = MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);
		if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE)
			return;
		DWORD dwImportRva = pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if (dwImportRva == 0)
			return;
		if (bOverride)
			m_mapModuleEntries.SetAt(hModule, NULL);
		else
			m_mapModuleEntries.Delete(hModule);
		PIMAGE_IMPORT_DESCRIPTOR pImageImportDesc = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pDOSHeader, dwImportRva);
		while (pImageImportDesc->Name)
		{
			PCSTR pszModuleName = MakePtr(PCSTR, pDOSHeader, pImageImportDesc->Name);
			if (_strcmpi(pszModuleName, "KERNEL32.DLL") == 0)
			{
				__try
				{
					PIMAGE_THUNK_DATA pOriginalThunk = MakePtr(PIMAGE_THUNK_DATA, pDOSHeader, pImageImportDesc->OriginalFirstThunk);
					PIMAGE_THUNK_DATA pActualThink = MakePtr(PIMAGE_THUNK_DATA, pDOSHeader, pImageImportDesc->FirstThunk);
					while (pOriginalThunk->u1.Function)
					{
						if ((pOriginalThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0)
						{
							PIMAGE_IMPORT_BY_NAME pImageImportByName = MakePtr(PIMAGE_IMPORT_BY_NAME, pDOSHeader, pOriginalThunk->u1.AddressOfData);
							PCSTR pszFunctionName = (PCSTR)pImageImportByName->Name;
							if (strcmp(pszFunctionName, m_szModule) == 0)
							{
								T pEntry = (T)pActualThink->u1.Function;
								if (bOverride ? pEntry != m_pDummy : TRUE)
								{
									MEMORY_BASIC_INFORMATION MemBasicInfo = {};
									VirtualQuery(pActualThink, &MemBasicInfo, sizeof(MemBasicInfo));
									DWORD dwOldProtect;
									if (VirtualProtect(MemBasicInfo.BaseAddress, MemBasicInfo.RegionSize, PAGE_READWRITE, &dwOldProtect))
									{
										pActualThink->u1.Function = (DWORD_PTR)(bOverride ? m_pDummy : pEntry);
										VirtualProtect(MemBasicInfo.BaseAddress, MemBasicInfo.RegionSize, MemBasicInfo.Protect, &dwOldProtect);
										if (bOverride)
											m_mapModuleEntries.SetAt(hModule, pEntry);
										break;
									}
								}
							}
						}
						++pOriginalThunk;
						++pActualThink;
					}
				}
				__except( EXCEPTION_EXECUTE_HANDLER )
				{
				}
			}
			else
			{
				HMODULE hNestedModule = GetModuleHandleA(pszModuleName);
				if (hNestedModule != NULL)
					TraverseImportTables(hNestedModule, bOverride, nNestedLevel + 1);
			}
			++pImageImportDesc;
		}
		if (bCleanup)
			Clear();
	}
};
