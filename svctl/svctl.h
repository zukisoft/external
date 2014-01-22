//---------------------------------------------------------------------------
// svctl.h
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

#ifndef __SVCTL_H_
#define __SVCTL_H_
#pragma once

//---------------------------------------------------------------------------
// BUILD OPTIONS
//---------------------------------------------------------------------------

// Ensure C++ compilation has been used

#ifndef __cplusplus
#error SVCTL requires C++ compilation (use a .cpp suffix)
#endif

// Ensure _WIN32_WINNT is set to at least 0x0500 (Windows 2000)

#if (!defined _WIN32_WINNT) || (_WIN32_WINNT < 0x0500)
#error SVCTL requires _WIN32_WINNT to be #defined to at least version 0x0500
#endif

//---------------------------------------------------------------------------
// Macros

#define BEGIN_NAMESPACE(x)		namespace x {
#define END_NAMESPACE(x)		}

//---------------------------------------------------------------------------
// CRT

#include "svctlcrt.h"			// Include SVCTL CRT declarations

#include <tchar.h>				// Include generic text mappings
#include <crtdbg.h>				// Include CRT debugging features

//---------------------------------------------------------------------------
// Win32

#include <windows.h>			// Include Win32 general declarations

#pragma comment(linker, "/defaultlib:kernel32.lib")		// KERNEL32.DLL
#pragma comment(linker, "/defaultlib:user32.lib")		// USER32.DLL
#pragma comment(linker, "/defaultlib:advapi32.lib")		// ADVAPI32.DLL
#pragma comment(linker, "/defaultlib:oleaut32.lib")		// OLEAUT32.DLL

//---------------------------------------------------------------------------
// Windows 2000 Service Template Library

#pragma pack(push, 8)			// Force 8-byte structure boundaries

// General Library --------------------------------------------------

#include "svctlres.h"		// Include SVCTL resource declarations
#include "svctlapi.h"		// Include SVCTL API function declarations
#include "svctllib.h"		// Include SVCTL general library declarations

// User Interface ---------------------------------------------------

#include "svctlui.h"		// Include SVCTL user interface declarations
#include "svctlcui.h"		// <--- goes away
#include "svctlgui.h"		// <--- goes away

// Installer --------------------------------------------------------

#include "svctlctl.h"		// Include ServiceControl class declarations
#include "svctlins.h"		// Include ServiceInstall class declarations

// Service Implementations ------------------------------------------

#include "svctlsts.h"		// Include ServiceStatus class declarations
#include "svctlhlr.h"		// Include Service control handler class decls
#include "svctlsvc.h"		// Include Service classes and templates
#include "svctlreg.h"		// Include ServiceParameters auxiliary class decls
#include "svctlevt.h"		// Include ServiceEvents auxiliary class decls
#include "svctllog.h"		// Include ServiceActivityLog auxiliary class decls

// Management Classes ----------------------------------------------

#include "svctlmap.h"		// Include SERVICE_MAP management class declarations
#include "svctlmgr.h"		// Include ServiceManager class declarations

// COM Implementation -----------------------------------------------

#include "svctlcom.h"		// Include COM Specialization classes

#pragma pack(pop)

//---------------------------------------------------------------------------

#endif	// _SVCTL_H_