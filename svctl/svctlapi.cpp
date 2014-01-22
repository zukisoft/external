//---------------------------------------------------------------------------
// svctlapi.cpp
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

#include "stdafx.h"						// Include project pre-compiled headers

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4127)			// "conditional expression is constant"

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Memory Allocation (CRT)
//
// Memory allocation is done through the CRT unless SVCTL_MIN_CRT has been
// specified for the project
//---------------------------------------------------------------------------

#ifndef SVCTL_MIN_CRT

//---------------------------------------------------------------------------
// SVCTL::AllocMem (CRT)
//
// Allocates and initializes a block of heap memory
//
// Arguments :
//
//	cb			- Number of bytes to be allocated from the heap

void* AllocMem(size_t cb)
{
	void* pvMem = malloc(cb);			// Attempt to allocate the buffer
	if(pvMem) memset(pvMem, 0, cb);		// Initialize buffer to all zeros

	return pvMem;						// Return the allocated buffer
}

//---------------------------------------------------------------------------
// SVCTL::ReAllocMem (CRT)
//
// Reallocates a block of memory originally allocated with AllocMem()
//
// Arguments :
//
//	pv			- Pointer to the block of memory allocated by AllocMem()
//	cb			- New size, in bytes, to assign to the heap block

void* ReAllocMem(void *pv, size_t cb)
{
	void*		pvRealloc;				// The reallocated heap buffer pointer
	size_t		oldSize;				// Original heap buffer size
	size_t		newSize;				// Reallocated buffer size
	
	if(!pv) return AllocMem(cb);		// Cannot reallocate a NULL pointer

	oldSize = _msize(pv);				// Retrieve the original buffer size
	pvRealloc = realloc(pv, cb);		// Attempt to reallocate the buffer

	if(pvRealloc) {

		newSize = _msize(pvRealloc);	// Determine the new size of the buffer

		// If the new buffer is larger than the old buffer, zero out the extra

		if(newSize > oldSize) memset(reinterpret_cast<BYTE*>(pvRealloc) 
			+ oldSize, 0, newSize - oldSize);
	}
	
	return pvRealloc;					// Return the reallocated buffer
}

//---------------------------------------------------------------------------
// Memory Allocation (Win32)
//
// Memory allocation is done through the Win32 heap APIs for SVCTL_MIN_CRT
//---------------------------------------------------------------------------

#else	// SVCTL_MIN_CRT

//---------------------------------------------------------------------------
// SVCTL::ReAllocMem (Win32)
//
// Reallocates a block of memory originally allocated with AllocMem()
//
// Arguments :
//
//	pv			- Pointer to the block of memory allocated by AllocMem()
//	cb			- New size, in bytes, to assign to the heap block

void* ReAllocMem(void *pv, size_t cb)
{
	if(!pv) return AllocMem(cb);		// Cannot reallocate a NULL pointer

	// Reallocate the heap buffer, and zero out any newly allocated bytes

	return HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pv, cb);
}

#endif	// SVCTL_MIN_CRT

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)
