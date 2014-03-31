//---------------------------------------------------------------------------
// svctlapi.h
//
// The contents of this file are subject to the Mozilla Public License
// Version 1.1 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is Windows Service Template Library.
//
// The Initial Developer of the Original Code is Michael G. Brehm.
// Portions created by the Initial Developer(s) are Copyright (C)2001-2007
// Michael G. Brehm. All Rights Reserved.
//
// Contributor(s):
//	Michael G. Brehm <michael.brehm@verizon.net> (original author)
//---------------------------------------------------------------------------

#ifndef __SVCTLAPI_H_
#define __SVCTLAPI_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4127)			// "conditional expression is constant"

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Type Declarations

typedef unsigned int (WINAPI * PFN_CRTTHREADPROC)(void*);

//---------------------------------------------------------------------------
// Memory Allocation API
//---------------------------------------------------------------------------

void*	AllocMem(size_t cb);
void	FreeMem(void *pv);
void*	ReAllocMem(void *pv, size_t cb);
size_t	SizeMem(const void *pv);

#ifndef SVCTL_MIN_CRT

//---------------------------------------------------------------------------
// SVCTL::FreeMem (CRT)
//
// Releases memory allocated by the AllocMem() function
//
// Arguments :
//
//	pv			- Pointer to the allocated heap memory.  Can be NULL

inline void FreeMem(void *pv)
{
	if(pv) free(pv);					// Release the allocated buffer
}

//---------------------------------------------------------------------------
// SVCTL::SizeMem (CRT)
//
// Determines the size of a memory block allocated with AllocMem()
//
// Arguments :
//
//	pv			- Pointer to the block of memory allocated by AllocMem()

inline size_t SizeMem(const void *pv)
{
	return (pv) ? _msize(const_cast<void*>(pv)) : 0;	// Get the buffer size
}

#else	// SVCTL_MIN_CRT

//---------------------------------------------------------------------------
// SVCTL::AllocMem (Win32)
//
// Allocates and initializes a block of heap memory
//
// Arguments :
//
//	cb			- Number of bytes to be allocated from the heap

inline void* AllocMem(size_t cb)
{
	// Allocate the buffer from the heap, and initialize it to all zeros

	return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
}

//---------------------------------------------------------------------------
// SVCTL::FreeMem (Win32)
//
// Releases memory allocated by the AllocMem() function
//
// Arguments :
//
//	pv			- Pointer to the allocated heap memory.  Can be NULL

inline void FreeMem(void *pv)
{
	if(pv) HeapFree(GetProcessHeap(), 0, pv);	// Release the allocated buffer
}

//---------------------------------------------------------------------------
// SVCTL::SizeMem (Win32)
//
// Determines the size of a memory block allocated with AllocMem()
//
// Arguments :
//
//	pv			- Pointer to the block of memory allocated by AllocMem()

inline size_t SizeMem(const void *pv)
{
	return (pv) ? HeapSize(GetProcessHeap(), 0, pv) : 0;	// Return size
}

#endif	// SVCTL_MIN_CRT

//---------------------------------------------------------------------------
// Thread Creation API
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// SVCTL::CreateThread
//
// Used as a wrapper around the _beginthreadex() function, using the same
// argument types as the Win32 CreateThread() function
//
// Arguments :
//
//	lpThreadAttributes		- SECURITY_ATTRIBUTES for the thread
//	dwStackSize				- Size of the stack to create for the thread
//	lpStartAddress			- Thread entry point function pointer
//	lpParameter				- Parameter to be passed into lpStartAddress
//	dwCreationFlags			- Special thread creation flags
//	lpThreadId				- On success, receives the thread ID value

inline HANDLE WINAPI CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes,
	DWORD dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, 
	LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
#ifndef SVCTL_NO_CRT

	// If there is any possibility of the CRT, we have to use _beginthreadex(),
	// which as you can see, is an absolute nightmare of typecasting
	
	return reinterpret_cast<HANDLE>(_beginthreadex(
		lpThreadAttributes,
		static_cast<unsigned int>(dwStackSize), 
		reinterpret_cast<PFN_CRTTHREADPROC>(lpStartAddress),
		lpParameter, 
		static_cast<unsigned int>(dwCreationFlags), 
		reinterpret_cast<unsigned int*>(lpThreadId)
		));

#else

	// Otherwise, we can safely use the Win32 CreateThread() API instead

	return ::CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress,
		lpParameter, dwCreationFlags, lpThreadId);

#endif	// SVCTL_NO_CRT

}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif	// __SVCTLAPI_H_