//---------------------------------------------------------------------------
// svctlcrt.h
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

#ifndef __SVCTLCRT_H_
#define __SVCTLCRT_H_
#pragma once

//---------------------------------------------------------------------------
// Notes :
//
// SVCTL_MIN_CRT is used to activate a bunch of Win32 mappings that replace
// a good portion of the CRT, but still allows the application to be linked
// with the standard Visual C++ runtime library for all the startup code
//
// SVCTL_NO_CRT is used to completely disable the Visual C++ runtime libraries,
// and instead use the code present in the SVCTLCRT static link library.  This
// option cannot be used in debug code builds, or if the application requires
// C++ exception handling.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Configure SVCTL CRT Support

// SVCTL_NO_CRT cannot be used with _DEBUG

#if (defined _DEBUG) && (defined SVCTL_NO_CRT)
#undef SVCTL_NO_CRT
#endif

// SVCTL_NO_CRT cannot be used with _CPPUNWIND

#if (defined _CPPUNWIND) && (defined SVCTL_NO_CRT)
#error SVCTL_NO_CRT cannot be used in conjunction with /GX option
#endif

// SVCTL_NO_CRT implies SVCTL_MIN_CRT

#if (defined SVCTL_NO_CRT) && (!defined SVCTL_MIN_CRT)
#define SVCTL_MIN_CRT
#endif

// SVCTL_NO_CRT requires _CRTIMP to not expand to __declspec(dllimport)

// NOTE : I Changed this from MIN -- figure this out later.
// Perhaps it's best to #undef _DLL instead of doing this

#if (defined SVCTL_NO_CRT) && (defined _DLL)
#undef _CRTIMP
#define _CRTIMP
#endif

// If SVCTL_NO_CRT is not defined, we need to ensure that the program is
// compiled with a multithreaded CRT library, and include some additional headers
// NOTE : I Changed this from MIN -- figure this out later

#ifndef SVCTL_NO_CRT

#ifndef _MT
#error SVCTL requires the use of a multithreaded C runtime library
#endif

#include <malloc.h>				// Include CRT memory allocation declarations
#include <process.h>			// Include CRT process/thread declarations
#include <stdio.h>				// Include CRT standard I/O functions

#endif	// SVCTL_NO_CRT

//---------------------------------------------------------------------------
// SVCTL_NO_CRT

#ifdef SVCTL_NO_CRT

// Manually force the linker to ignore all C runtime library references,
// and use the svctlcrt library instead

#pragma comment(linker, "/nodefaultlib:LIBC")		// Single Threaded
#pragma comment(linker, "/nodefaultlib:LIBCD")		// Single Threaded Debug
#pragma comment(linker, "/nodefaultlib:LIBCMT")		// Multithreaded
#pragma comment(linker, "/nodefaultlib:LIBCMTD")	// Multithreaded Debug
#pragma comment(linker, "/nodefaultlib:MSVCRT")		// Multithreaded DLL
#pragma comment(linker, "/nodefaultlib:MSVCRTD")	// Multithreaded Debug DLL

#pragma comment(lib, "svctlcrt.lib")	// Use the SVCTL runtime library code

#endif	// SVCTL_NO_CRT

//---------------------------------------------------------------------------
// Inlined Win32 CRT Mappings (SVCTL_MIN_CRT)
//---------------------------------------------------------------------------

#ifdef SVCTL_MIN_CRT

// _vsnprintf -> wvsprintfA

inline int __cdecl _vsnprintf(char *buffer, size_t count, const char *format, 
							  va_list argptr)
{
	return wvsprintfA(buffer, format, argptr);
}

// _vsnwprintf -> wvsprintfW

inline int __cdecl _vsnwprintf(wchar_t *buffer, size_t count, const wchar_t *format,
							   va_list argptr)
{
	return wvsprintfW(buffer, format, argptr);
}

// strcat -> lstrcatA

inline char* __cdecl strcat(char *dest, const char *source)
{
	return lstrcatA(dest, source);
}

// strcmp -> lstrcmpA

inline int __cdecl strcmp(const char *string1, const char *string2)
{
	return lstrcmpA(string1, string2);
}

// strcpy ->lstrcpyA

inline char* __cdecl strcpy(char *dest, const char *source)
{
	return lstrcpyA(dest, source);
}

// strncpy -> lstrcpynA

inline char* __cdecl strncpy(char *dest, const char *source, size_t count)
{
	return lstrcpynA(dest, source, count);
}

// tolower -> CharLowerA

inline int __cdecl tolower(int ch)
{
	return reinterpret_cast<int>(CharLowerA(reinterpret_cast<LPSTR>(ch)));
}

// toupper -> CharUpperA

inline int __cdecl toupper(int ch)
{
	return reinterpret_cast<int>(CharUpperA(reinterpret_cast<LPSTR>(ch)));
}

// towlower -> CharLowerW

inline wchar_t __cdecl towlower(wchar_t ch)
{
	return reinterpret_cast<wchar_t>(CharLowerW(reinterpret_cast<LPWSTR>(ch)));
}

// towupper -> CharUpperW

inline wchar_t __cdecl towupper(wchar_t ch)
{
	return reinterpret_cast<wchar_t>(CharUpperW(reinterpret_cast<LPWSTR>(ch)));
}

// wcscat -> lstrcatW

inline wchar_t* __cdecl wcscat(wchar_t *dest, const wchar_t *source)
{
	return lstrcatW(dest, source);
}

// wcscmp -> lstrcmpW

inline int __cdecl wcscmp(const wchar_t *string1, const wchar_t *string2)
{
	return lstrcmpW(string1, string2);
}

// wcscpy -> lstrcpyW

inline wchar_t* __cdecl wcscpy(wchar_t *dest, const wchar_t *source)
{
	return lstrcpyW(dest, source);
}

// wcsncpy -> lstrcpynW

inline wchar_t* __cdecl wcsncpy(wchar_t *dest, const wchar_t *source, size_t count)
{
	return lstrcpynW(dest, source, count);
}

#endif	// SVCTL_MIN_CRT

//---------------------------------------------------------------------------

#endif	// __SVCTLCRT_H_